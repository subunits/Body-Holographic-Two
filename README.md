FFT Hologram — Forward Generator (C)
===================================

Contents
--------
- fft_hologram_forward_modes.c : C program that generates holograms (intensity, amplitude, phase)
- README.md                    : this file

Overview
--------
This package contains a hard-coded, easy-to-run C program that:
- Builds a simple synthetic object (three Gaussian spots),
- Computes its FFT (object -> frequency domain),
- Interferes the object field with a tilted reference wave,
- Produces one of three hologram types (intensity, amplitude, phase),
- Writes the hologram to an 8-bit grayscale PNG.

Requirements
------------
- GCC (or compatible C compiler)
- FFTW3 development library (libfftw3-dev)
- libpng development library (libpng-dev)

On Debian/Ubuntu:
    sudo apt-get update
    sudo apt-get install build-essential libfftw3-dev libpng-dev

On macOS (Homebrew):
    brew install fftw libpng

Build
-----
Compile the program with:

    gcc fft_hologram_forward_modes.c -o fft_hologram_forward_modes -lfftw3 -lpng -lm

Usage
-----
Run with arguments:

    ./fft_hologram_forward_modes NX NY kx ky mode output.png

- NX NY : image size in pixels (e.g. 512 512)
- kx ky : reference tilt parameters (radians per pixel) controlling fringe spacing and orientation
- mode  : hologram mode
    0 -> intensity hologram (|field|^2)
    1 -> amplitude hologram (|field|)
    2 -> phase hologram (arg(field) mapped to grayscale)
- output.png : output filename

Examples
--------
    ./fft_hologram_forward_modes 512 512 0.05 0.03 0 hologram_intensity.png
    ./fft_hologram_forward_modes 512 512 0.05 0.03 1 hologram_amplitude.png
    ./fft_hologram_forward_modes 512 512 0.05 0.03 2 hologram_phase.png

Quick tuning tips
-----------------
- Fringe spacing (pixels) ≈ 2π / sqrt(kx^2 + ky^2).
  Example: for ~50-pixel spacing, sqrt(kx^2+ky^2) ≈ 2π/50 ≈ 0.125.
- Fringe angle = atan2(ky, kx).
- Typical kx,ky values for NX=512: 0.02–0.08 for visible fringes; increase to 0.1+ for stronger carrier separation.

Notes
-----
- The program is intended for forward hologram generation only (no numerical reconstruction).
- The object function is synthetic and hard-coded; feel free to replace it with a custom object.
- The program uses FFTW's 2D complex-to-complex forward transform, then interprets the FFT output as the object field in frequency domain (consistent with the earlier walkthrough).

License
-------
MIT License — feel free to reuse and adapt.

