# QTerminal

The lightweight Qt terminal emulator

## Installation

### Requirements

* Qt >= 5.2
* CMake >= 2.8
* [qtermwidget](https://github.com/lxde/qtermwidget)


### Building

A shadow build (out of source) is strongly recommended
http://www.cmake.org/Wiki/CMake_FAQ#Out-of-source_build_trees

1. `mkdir -p build && cd build`
2. `cmake <path/to/source>`
3. `make`
4. (to install) `make install`

Read cmake docs to fine tune the build process (CMAKE_INSTALL_PREFIX, etc...)

## Translations

* Edit `src/CMakeLists.txt` to add a new ts file.
* `make lupdate` updates ts files to be translated (lrelease is called automatically in make)
