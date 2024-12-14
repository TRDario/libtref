# libtref

libtref is a library for working with .tref (tre Bitmap Font) files.

This repository also provides some tools for working with .tref files if TREF_BUILD_TOOLS is enabled. As of now, this includes:

- trefc (CLI .tref compiler)

## Documentation

Documentation can be built with Doxygen, or can be viewed [here](https://trdario.github.io/libtref/).

## Dependencies

libtref depends on the following external libraries:

- [qoi](https://github.com/phoboslab/qoi) (vendored)
- [lz4](https://github.com/lz4/lz4)

trefc depends on the following external libraries:

- [stb_image](https://github.com/nothings/stb) (vendored)

## Building

The following is required to build libtref:

- A C++20 compiler.
- CMake 3.23.0 or higher.
