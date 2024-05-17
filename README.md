# QTerminal

## Overview

QTerminal is a lightweight Qt terminal emulator based on [QTermWidget](https://github.com/lxqt/qtermwidget).

It is maintained by the LXQt project but can be used independently from this desktop environment. The only bonds are [lxqt-build-tools](https://github.com/lxqt/lxqt-build-tools) representing a build dependency.

This project is licensed under the terms of the [GPLv2](https://www.gnu.org/licenses/gpl-2.0.en.html) or any later version. See the LICENSE file for the full text of the license.

## Installation

### Compiling sources

Dependencies Qt ≥ 6.6.0 and [QTermWidget](https://github.com/lxqt/qtermwidget).
Optional dependencies include [libcanberra](https://0pointer.net/lennart/projects/libcanberra/) for playing bells.
In order to build CMake ≥ 3.18.0 and [lxqt-build-tools](https://github.com/lxqt/lxqt-build-tools) are needed as well as optionally Git to pull latest VCS checkouts.

Code configuration is handled by CMake. Building out of source is required. CMake variable `CMAKE_INSTALL_PREFIX` will normally have to be set to `/usr`.

To build run `make`, to install `make install` which accepts variable `DESTDIR` as usual.

### Binary packages

Official binary packages are provided by all major Linux and BSD distributions.
Just use your package manager to search for string `qterminal`.


### Translation

Translations can be done in [LXQt-Weblate](https://translate.lxqt-project.org/projects/lxqt-desktop/qterminal/)

<a href="https://translate.lxqt-project.org/projects/lxqt-desktop/qterminal/">
<img src="https://translate.lxqt-project.org/widgets/lxqt-desktop/-/qterminal/multi-auto.svg" alt="Translation status" />
</a>

