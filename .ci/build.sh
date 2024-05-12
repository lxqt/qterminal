set -ex

source shared-ci/prepare-archlinux.sh

# See *depends in https://github.com/archlinuxcn/repo/blob/master/archlinuxcn/qterminal-git/PKGBUILD
pacman -S --noconfirm --needed git cmake lxqt-build-tools-git qt6-tools qtermwidget-git qt6-translations libcanberra

cmake -B build -S .
make -C build
ARGS="-V" make -C build test
