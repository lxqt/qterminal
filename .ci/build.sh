set -ex

source shared-ci/prepare-archlinux.sh

# See *depends in https://github.com/archlinuxcn/repo/blob/master/archlinuxcn/qterminal-git/PKGBUILD
pacman -S --noconfirm --needed git cmake lxqt-build-tools-git qt5-tools qtermwidget-git qt5-x11extras qt5-translations

cmake -B build -S .
make -C build
ARGS="-V" make -C build test
