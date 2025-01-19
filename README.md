# libtref

libtref is a library for working with .tref (tre Bitmap Font) files.

This repository also provides some tools for working with .tref files if TREF_BUILD_TOOLS is enabled. This includes:

- trefc (CLI .tref compiler)
- gtref (graphical .tref editor)

## Documentation

Documentation can be built with Doxygen, or can be viewed [here](https://trdario.github.io/libtref/).

The HTML theme uses [Doxygen Awesome](https://github.com/jothepro/doxygen-awesome-css).

## Building

The following is required to build libtref:

- A C++20 compiler.
- CMake 3.23.0 or higher.

libtref depends on the following external libraries:

- [qoi](https://github.com/phoboslab/qoi) (vendored)
- [lz4](https://github.com/lz4/lz4)

trefc depends on the following external libraries:

- [stb_image](https://github.com/nothings/stb) (vendored)

gtref depends on the following external libraries:

- [libtr](https://github.com/TRDario/libtr)
- [Dear ImGui](https://github.com/ocornut/imgui) (handled with FetchContent)
- [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/) (handled with FetchContent)

## License

libtref, trefc and gtref are licensed under the MIT license, see [here](https://github.com/TRDario/libtr/blob/main/LICENSE).
Vendored external libraries used by libtref and its tools are distributed under their respective licenses.
