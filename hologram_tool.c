#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <png.h>

double object(double x, double y) {
    double r1 = exp(-((x+30)*(x+30) + (y+20)*(y+20)) / 200.0);
    double r2 = exp(-((x-40)*(x-40) + (y-25)*(y-25)) / 300.0);
    double r3 = exp(-(x*x + y*y) / 500.0);
    return r1 + r2 + r3;
}

int save_png(const char *filename, unsigned char *data, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) return 1;
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    if (setjmp(png_jmpbuf(png))) { png_destroy_write_struct(&png, &info); fclose(fp); return 1; }
    png_init_io(png, fp);
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    png_bytep *row_ptrs = (png_bytep*) malloc(sizeof(png_bytep) * height);
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
        printf("Usage: %s NX NY kx ky mode output.png\n", argv[0]);
        return 1;
    }
    int NX = atoi(argv[1]);
    int NY = atoi(argv[2]);
    if (NX <= 0 || NY <= 0) { fprintf(stderr, "Invalid dimensions\n"); return 1; }
    
    double kx = atof(argv[3]), ky = atof(argv[4]);
    int mode = atoi(argv[5]);

    unsigned char *holo = (unsigned char*) malloc((size_t)NX * (size_t)NY);
    double minVal = 1e9, maxVal = -1e9;
    double *raw_vals = malloc(sizeof(double) * NX * NY);

    for (int y = 0; y < NY; y++) {
        for (int x = 0; x < NX; x++) {
            double X = x - NX / 2.0;
            double Y = y - NY / 2.0;

            double O = object(X, Y);

            double phase = kx * X + ky * Y;
            double ref_re = cos(phase);
            double ref_im = sin(phase);

            double field_re = O + ref_re;
            double field_im = ref_im;

            double val;
            if (mode == 0) {
                val = field_re * field_re + field_im * field_im;
            } else if (mode == 1) {
                val = sqrt(field_re * field_re + field_im * field_im);
            } else {
                val = (atan2(field_im, field_re) + M_PI) / (2.0 * M_PI);
            }

            raw_vals[y * NX + x] = val;
            if (val < minVal) minVal = val;
            if (val > maxVal) maxVal = val;
        }
    }

    for (int i = 0; i < NX * NY; i++) {
        double range = maxVal - minVal;
        if (range > 0) {
            double normalized = (raw_vals[i] - minVal) / range;
            holo[i] = (unsigned char)(255.0 * normalized);
        } else {
            holo[i] = 0;
        }
    }

    save_png(argv[6], holo, NX, NY);
    free(raw_vals);
    free(holo);
    return 0;
}
