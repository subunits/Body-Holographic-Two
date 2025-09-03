#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fftw3.h>
#include <png.h>

// --- Simple object: three Gaussians ---
double object(double x, double y) {
    double r1 = exp(-((x+30)*(x+30) + (y+20)*(y+20)) / 200.0);
    double r2 = exp(-((x-40)*(x-40) + (y-25)*(y-25)) / 300.0);
    double r3 = exp(-(x*x + y*y) / 500.0);
    return r1 + r2 + r3;
}

// --- Save grayscale PNG ---
int save_png(const char *filename, unsigned char *data, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) { perror("fopen"); return 1; }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) { fclose(fp); return 1; }
    png_infop info = png_create_info_struct(png);
    if (!info) { png_destroy_write_struct(&png, NULL); fclose(fp); return 1; }
    if (setjmp(png_jmpbuf(png))) { png_destroy_write_struct(&png, &info); fclose(fp); return 1; }

    png_init_io(png, fp);
    png_set_IHDR(png, info, width, height,
                 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    png_bytep *row_ptrs = (png_bytep*) malloc(sizeof(png_bytep) * height);
    if (!row_ptrs) { png_destroy_write_struct(&png, &info); fclose(fp); return 1; }
    for (int y = 0; y < height; y++) row_ptrs[y] = data + y * width;
    png_write_image(png, row_ptrs);
    png_write_end(png, NULL);

    free(row_ptrs);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 7) {
        fprintf(stderr,
            "Usage: %s NX NY kx ky mode output.png\n"
            "  mode=0 -> intensity hologram (|field|^2)\n"
            "  mode=1 -> amplitude hologram (|field|)\n"
            "  mode=2 -> phase hologram (arg(field))\n"
            "Example: %s 512 512 0.05 0.03 0 hologram.png\n",
            argv[0], argv[0]);
        return 1;
    }

    int NX = atoi(argv[1]);
    int NY = atoi(argv[2]);
    double kx = atof(argv[3]);
    double ky = atof(argv[4]);
    int mode = atoi(argv[5]);
    const char *outfile = argv[6];

    printf("Generating hologram %dx%d, kx=%.4f, ky=%.4f, mode=%d -> %s\n",
           NX, NY, kx, ky, mode, outfile);

    fftw_complex *U_obj = fftw_malloc(sizeof(fftw_complex) * NX * NY);
    fftw_complex *U_fft = fftw_malloc(sizeof(fftw_complex) * NX * NY);
    if (!U_obj || !U_fft) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    fftw_plan plan_fwd = fftw_plan_dft_2d(NY, NX, U_obj, U_fft, FFTW_FORWARD, FFTW_ESTIMATE);

    // Build object field
    for (int y = 0; y < NY; y++) {
        for (int x = 0; x < NX; x++) {
            double xx = x - NX/2.0;
            double yy = y - NY/2.0;
            double amp = object(xx, yy);
            U_obj[y*NX + x][0] = amp;
            U_obj[y*NX + x][1] = 0.0;
        }
    }

    fftw_execute(plan_fwd);

    unsigned char *holo = (unsigned char*) malloc((size_t)NX * (size_t)NY);
    if (!holo) {
        fprintf(stderr, "Memory allocation failed\n");
        fftw_destroy_plan(plan_fwd);
        fftw_free(U_obj);
        fftw_free(U_fft);
        return 1;
    }
    double maxVal = 0.0;

    // Compute hologram according to mode
    for (int y = 0; y < NY; y++) {
        for (int x = 0; x < NX; x++) {
            double re = U_fft[y*NX + x][0];
            double im = U_fft[y*NX + x][1];

            double phase = kx * x + ky * y;
            double ref_re = cos(phase);
            double ref_im = sin(phase);

            double field_re = re + ref_re;
            double field_im = im + ref_im;

            double val;
            if (mode == 0) { // intensity
                val = field_re*field_re + field_im*field_im;
            } else if (mode == 1) { // amplitude
                val = sqrt(field_re*field_re + field_im*field_im);
            } else { // phase
                val = atan2(field_im, field_re); // [-pi,pi]
                val = (val + M_PI) / (2*M_PI);   // normalize 0..1
            }

            if (val > maxVal) maxVal = val;
            U_fft[y*NX + x][0] = val;
        }
    }

    // Normalize to 8-bit
    for (int y = 0; y < NY; y++) {
        for (int x = 0; x < NX; x++) {
            double val = U_fft[y*NX + x][0];
            unsigned char pix;
            if (mode == 2) {
                // phase already normalized 0..1
                pix = (unsigned char)(255.0 * val);
            } else {
                if (maxVal <= 0.0) pix = 0;
                else pix = (unsigned char)(255.0 * val / maxVal);
            }
            holo[y*NX + x] = pix;
        }
    }

    if (save_png(outfile, holo, NX, NY) == 0) {
        printf("Wrote %s\n", outfile);
    } else {
        fprintf(stderr, "Failed to save PNG\n");
    }

    free(holo);
    fftw_destroy_plan(plan_fwd);
    fftw_free(U_obj);
    fftw_free(U_fft);
    return 0;
}
