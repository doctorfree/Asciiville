#!/bin/bash
#
# mkaur.sh - front end for Arch Linux PKGBUILD distribution creation

PKG="asciiville"
PKG_NAME="Asciiville"
ARCH=x86_64
SUDO=sudo
GCI=

have_makepkg=`type -p makepkg`
[ "${have_makepkg}" ] || {
  echo "Arch Linux packaging tools do not appear to be installed"
  echo "Are you on the appropriate Linux system with packaging requirements ?"
  echo "Exiting"
  exit 1
}

arch_arch=`uname -m`
[ "${arch_arch}" == "${ARCH}" ] || ARCH=${arch_arch}

if [ "${__ASC_SRC__}" ]
then
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

# Subdirectory in which to create the distribution files
OUT_DIR="dist/${PKG_NAME}_${PKG_VER}"

[ -d "${SRC}" ] || {
    echo "$SRC does not exist or is not a directory. Exiting."
    exit 1
}

cd "${SRC}"

${SUDO} rm -rf ${PKG}
mkdir ${PKG}
cp VERSION ${PKG}/VERSION
cp pkg/aur/PKGBUILD ${PKG}/PKGBUILD
cp pkg/aur/${PKG}.install ${PKG}/${PKG}.install
cp pkg/aur/.SRCINFO ${PKG}/.SRCINFO
cp pkg/aur/makepkg.conf ${PKG}/makepkg.conf

echo "Creating ${PKG_NAME} PKGBUILD distribution archive"
tar cf - ${PKG} | \
    gzip -9 > ${PKG_NAME}-pkgbuild-${PKG_VER}-${PKG_REL}.tar.gz
mv ${PKG_NAME}-pkgbuild-${PKG_VER}-${PKG_REL}.tar.gz ${PKG}

echo "Building ${PKG_NAME}_${PKG_VER} AUR package"
cd "${SRC}/${PKG}"
makepkg --force --log --cleanbuild --noconfirm --syncdeps

# Rename package if necessary
for zstfile in *.zst
do
  [ "${zstfile}" == "*.zst" ] && continue
  newnam=`echo ${zstfile} | sed -e "s/${PKG}-${PKG_VER}-${PKG_REL}/${PKG_NAME}_${PKG_VER}-${PKG_REL}/"`
  [ "${zstfile}" == "${newnam}" ] && continue
  mv ${zstfile} ${newnam}
done

[ "${GCI}" ] || {
  [ -d ../releases ] || mkdir ../releases
  [ -d ../releases/${PKG_VER} ] || mkdir ../releases/${PKG_VER}
  ${SUDO} cp *.zst *.tar.gz ../releases/${PKG_VER}
}
