# Hologram Generator

A simple C tool to generate synthetic off-axis holograms using FFTW and libpng.

## Requirements
- `fftw3` library
- `libpng` library
- GCC compiler

## Setup & Run
```bash
# Install dependencies
sudo apt-get update && sudo apt-get install -y libfftw3-dev libpng-dev

# Compile and run
gcc hologram_tool.c -o hologram_tool -lfftw3 -lpng -lm && ./hologram_tool 512 512 0.05 0.03 0 hologram.png
