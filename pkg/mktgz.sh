#!/bin/bash
#
# shellcheck disable=SC2001

PKG="asciiville"
PKG_NAME="Asciiville"
ARCH=$(uname -s)
SUDO=sudo
HERE=$(pwd)
USER=$(id -u -n)

[ "${USER}" == "root" ] && {
  echo "pkg/mktgz.sh must be run as a non-root user with sudo privilege"
  echo "Exiting"
  exit 1
}

[ -f VERSION ] || {
  echo "pkg/mktgz.sh must be run in the root of the Asciiville source folder"
  echo "Exiting"
  exit 1
}

if [ "${ARCH}" == "Darwin" ]; then
  DESTDIR="usr/local"
  group="wheel"
else
  DESTDIR="usr"
  ARCH="Linux"
  group="root"
fi

. ./VERSION
PKG_VER=${VERSION}
PKG_REL=${RELEASE}

umask 0022

# Subdirectory in which to create the distribution files
OUT_DIR="${HERE}/dist/${PKG_NAME}_${PKG_VER}"

${SUDO} rm -rf dist
mkdir dist

[ -d "${OUT_DIR}" ] && rm -rf "${OUT_DIR}"
mkdir "${OUT_DIR}"

for dir in "usr" "${DESTDIR}" "${DESTDIR}/share" "${DESTDIR}/share/man" \
  "${DESTDIR}/share/applications" "${DESTDIR}/share/doc" \
  "${DESTDIR}/share/doc/${PKG}" \
  "${DESTDIR}/share/${PKG}"; do
  [ -d "${OUT_DIR}"/"${dir}" ] || ${SUDO} mkdir "${OUT_DIR}"/"${dir}"
  ${SUDO} chown root:${group} "${OUT_DIR}"/"${dir}"
done

[ -d "${OUT_DIR}"/"${DESTDIR}"/bin ] && ${SUDO} rm -rf "${OUT_DIR}"/"${DESTDIR}"/bin

${SUDO} cp -a bin "${OUT_DIR}"/"${DESTDIR}"/bin

${SUDO} cp ./*.desktop "${OUT_DIR}/${DESTDIR}/share/applications"
${SUDO} cp -a conf/btop "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/btop
${SUDO} cp -a conf/console "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/console
${SUDO} cp -a conf/got "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/got
${SUDO} cp -a conf/jrnl "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/jrnl
${SUDO} cp -a conf/khard "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/khard
${SUDO} cp -a conf/kitty "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/kitty
${SUDO} cp -a conf/mutt "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/mutt
${SUDO} cp -a conf/mutt_multiple "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/mutt_multiple
${SUDO} cp -a conf/neomutt "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/neomutt
${SUDO} cp -a conf/newsboat "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/newsboat
${SUDO} cp -a conf/w3m "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/w3m
${SUDO} cp -a conf/tuir "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/tuir
${SUDO} cp conf/tmux.conf "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"
${SUDO} cp conf/rifle.conf "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"
${SUDO} cp copyright "${OUT_DIR}"/"${DESTDIR}"/share/doc/"${PKG}"
${SUDO} cp LICENSE "${OUT_DIR}"/"${DESTDIR}"/share/doc/"${PKG}"
${SUDO} cp CHANGELOG.md "${OUT_DIR}"/"${DESTDIR}"/share/doc/"${PKG}"
${SUDO} cp README.md "${OUT_DIR}"/"${DESTDIR}"/share/doc/"${PKG}"
${SUDO} cp VERSION "${OUT_DIR}"/"${DESTDIR}"/share/doc/"${PKG}"
${SUDO} gzip -9 "${OUT_DIR}"/"${DESTDIR}"/share/doc/"${PKG}"/CHANGELOG.md

${SUDO} cp -a art "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/art
${SUDO} mv "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/art/pics \
  "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/pics
${SUDO} cp -a music "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/music
${SUDO} cp -a tools "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/tools

${SUDO} gzip "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/art/*/*.asc

${SUDO} cp -a man/man1 "${OUT_DIR}"/"${DESTDIR}"/share/man/man1

${SUDO} cp -a share/menu "${OUT_DIR}"/"${DESTDIR}"/share/menu
${SUDO} cp -a share/figlet-fonts "${OUT_DIR}"/"${DESTDIR}"/share/figlet-fonts
${SUDO} cp -a share/neofetch-themes "${OUT_DIR}"/"${DESTDIR}"/share/neofetch-themes

${SUDO} chmod 644 "${OUT_DIR}"/"${DESTDIR}"/share/man/*/*
${SUDO} chmod 644 "${OUT_DIR}"/"${DESTDIR}"/share/menu/*
${SUDO} chmod 644 "${OUT_DIR}"/"${DESTDIR}"/share/figlet-fonts/*
find "${OUT_DIR}"/"${DESTDIR}"/share/doc/"${PKG}" -type d | while read -r dir; do
  ${SUDO} chmod 755 "${dir}"
done
find "${OUT_DIR}"/"${DESTDIR}"/share/doc/"${PKG}" -type f | while read -r f; do
  ${SUDO} chmod 644 "${f}"
done
find "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}" -type d | while read -r dir; do
  ${SUDO} chmod 755 "${dir}"
done
find "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}" -type f | while read -r f; do
  ${SUDO} chmod 644 "${f}"
done
${SUDO} chmod 755 "${OUT_DIR}"/"${DESTDIR}"/bin/* \
  "${OUT_DIR}"/"${DESTDIR}"/bin \
  "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/newsboat/*.sh \
  "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/newsboat/scripts/* \
  "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/newsboat/scripts/*/* \
  "${OUT_DIR}"/"${DESTDIR}"/share/menu \
  "${OUT_DIR}"/"${DESTDIR}"/share/figlet-fonts \
  "${OUT_DIR}"/"${DESTDIR}"/share/man \
  "${OUT_DIR}"/"${DESTDIR}"/share/man/*
${SUDO} chmod 755 "${OUT_DIR}"/"${DESTDIR}"/share/"${PKG}"/tools/bin/*
${SUDO} chown -R root:${group} "${OUT_DIR}"/"${DESTDIR}"

cd "${OUT_DIR}" || echo "Unable to enter ${OUT_DIR}"
echo "Creating compressed tar archive of ${PKG_NAME} ${PKG_VER} distribution"
${SUDO} tar cf - "${DESTDIR}"/*/* | gzip -9 >../"${PKG_NAME}"_"${PKG_VER}"-"${PKG_REL}"."${ARCH}".tgz

cd ..
[ -d ../releases ] || mkdir ../releases
[ -d ../releases/${PKG_VER} ] || mkdir ../releases/${PKG_VER}
${SUDO} cp ./*.tgz ../releases/"${PKG_VER}"
