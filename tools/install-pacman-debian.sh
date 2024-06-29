#!/bin/bash
#
# Install pacman, makepkg, and Arch packaging utilities on Ubuntu Linux
#
# Written 2024-06-27 by Ronald Joe Record <ronaldrecord@gmail.com>

sudo apt -q -y install --no-install-recommends \
  git pkg-config meson gcc libtool libgpgme-dev libarchive-dev ninja-build \
  libcurl4-openssl-dev libssl-dev curl gettext python3 python3-setuptools \
  dash gawk ca-certificates fakeroot fakechroot libarchive-tools

# Need a more recent version of Meson
sudo mkdir -p /usr/local/meson
sudo git clone https://github.com/mesonbuild/meson.git /usr/local/meson
[ -f /usr/bin/meson ] && {
  mver=$(/usr/bin/meson --version)
  sudo mv /usr/bin/meson /usr/bin/meson-${mver}
}
if [ -f /usr/local/meson/meson.py ]; then
  sudo ln -s /usr/local/meson/meson.py /usr/bin/meson
else
  echo "ERROR: cannot locate /usr/local/meson/meson.py"
  exit 1
fi

sudo rm -rf pacman
git clone https://gitlab.archlinux.org/pacman/pacman.git
cd pacman
meson setup -Ddoc=disabled build
ninja -C build

[ -d build ] || {
  echo "Directory pacman/build does not exist. Exiting."
  exit 1
}
cd build
grep mirror scripts/libmakepkg/source/git.sh > /dev/null && {
  cat scripts/libmakepkg/source/git.sh | sed -e "s/---mirror//" > /tmp/git$$
  cp /tmp/git$$ scripts/libmakepkg/source/git.sh
  rm -f /tmp/git$$
}

sudo meson install --no-rebuild --quiet
