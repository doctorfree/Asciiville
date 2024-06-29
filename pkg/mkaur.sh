#!/bin/bash
#
# mkaur.sh - front end for Arch Linux PKGBUILD distribution creation
#
# shellcheck disable=SC2001

PKG="asciiville"
PKG_NAME="Asciiville"
SUDO=sudo
GCI=

have_makepkg=$(type -p makepkg)
[ "${have_makepkg}" ] || {
  echo "Arch Linux packaging tools do not appear to be installed"
  echo "Are you on the appropriate Linux system with packaging requirements ?"
  echo "Exiting"
  exit 1
}

export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu

if [ "${__ASC_SRC__}" ]; then
  SRC="${__ASC_SRC__}"
else
  SRC="${HOME}/src/${PKG_NAME}"
fi

[ -f "${SRC}/VERSION" ] || {
  [ -f "/builds/doctorfree/${PKG_NAME}/VERSION" ] || {
    echo "${SRC}/VERSION does not exist. Exiting."
    exit 1
  }
  SRC="/builds/doctorfree/${PKG_NAME}"
  GCI=1
  # SUDO=
}

. "${SRC}/VERSION"
PKG_VER=${VERSION}
PKG_REL=${RELEASE}

umask 0022

[ -d "${SRC}" ] || {
  echo "$SRC does not exist or is not a directory. Exiting."
  exit 1
}

cd "${SRC}" || echo "Cannot enter $SRC"

${SUDO} rm -rf ${PKG}
mkdir ${PKG}
cp VERSION ${PKG}/VERSION
cp pkg/aur/PKGBUILD ${PKG}/PKGBUILD
cp pkg/aur/${PKG}.install ${PKG}/${PKG}.install
cp pkg/aur/.SRCINFO ${PKG}/.SRCINFO
cp pkg/aur/makepkg.conf ${PKG}/makepkg.conf

echo "Creating ${PKG_NAME} PKGBUILD distribution archive"
tar cf - ${PKG} \
  | gzip -9 >${PKG_NAME}-pkgbuild-${PKG_VER}-${PKG_REL}.tar.gz
mv ${PKG_NAME}-pkgbuild-${PKG_VER}-${PKG_REL}.tar.gz ${PKG}

echo "Building ${PKG_NAME}_${PKG_VER} AUR package"
cd "${SRC}/${PKG}" || echo "Cannot enter $SRC/$PKG"

# On Arch invoke with 'mkaur.sh sync'
dep_arg="--nodeps"
[ "$1" == "sync" ] && dep_arg="--syncdeps"
makepkg --force --log --cleanbuild --noconfirm ${dep_arg}

have_zstd=$(type -p zstd)
if [ "${have_zstd}" ]; then
  for pkg in *any.pkg.tar.gz
  do
    [ "${pkg}" == "*any.pkg.tar.gz" ] && continue
    gunzip ${pkg}
    ptar=$(echo "${pkg}" | sed -e "s/.gz//")
    zstd --rm ${ptar}
  done
else
  echo "ERROR: cannot locate zstd compression utility"
fi

# Rename package if necessary
for zstfile in *.zst; do
  [ "${zstfile}" == "*.zst" ] && continue
  newnam=$(echo "${zstfile}" | sed -e "s/${PKG}-${PKG_VER}-${PKG_REL}/${PKG_NAME}_${PKG_VER}-${PKG_REL}/")
  newnam=$(echo "${newnam}" | sed -e "s/asciiville/Asciiville/")
  [ "${zstfile}" == "${newnam}" ] && continue
  mv "${zstfile}" "${newnam}"
done

[ "${GCI}" ] || {
  [ -d ../releases ] || mkdir ../releases
  [ -d ../releases/${PKG_VER} ] || mkdir ../releases/${PKG_VER}
  ${SUDO} cp ./*.zst ./*.tar.gz ../releases/${PKG_VER}
}
