#!/bin/bash
#
# shellcheck disable=SC1090,SC2001

PKG="asciiville"
SRC_NAME="Asciiville"
PKG_NAME="Asciiville"
DESTDIR="usr"
SRC=${HOME}/src
SUDO=sudo
GCI=

[ -f "${SRC}/${SRC_NAME}/VERSION" ] || {
  [ -f "/builds/doctorfree/${SRC_NAME}/VERSION" ] || {
    echo "$SRC/$SRC_NAME/VERSION does not exist. Exiting."
    exit 1
  }
  SRC="/builds/doctorfree"
  SUDO=
  GCI=1
}

. "${SRC}/${SRC_NAME}/VERSION"
PKG_VER=${VERSION}
PKG_REL=${RELEASE}

umask 0022

# Subdirectory in which to create the distribution files
OUT_DIR="${SRC}/${SRC_NAME}/dist/${PKG_NAME}_${PKG_VER}"

[ -d "${SRC}/${SRC_NAME}" ] || {
  echo "$SRC/$SRC_NAME does not exist or is not a directory. Exiting."
  exit 1
}

cd "${SRC}/${SRC_NAME}" || echo "Cannot enter $SRC/$SRC_NAME"

${SUDO} rm -rf dist
mkdir dist

[ -d "${OUT_DIR}" ] && rm -rf "${OUT_DIR}"
mkdir "${OUT_DIR}"

for dir in "usr" "${DESTDIR}" "${DESTDIR}/share" "${DESTDIR}/share/man" \
  "${DESTDIR}/share/applications" "${DESTDIR}/share/doc" \
  "${DESTDIR}/share/doc/${PKG}" \
  "${DESTDIR}/share/${PKG}"; do
  [ -d "${OUT_DIR}/${dir}" ] || ${SUDO} mkdir "${OUT_DIR}/${dir}"
  ${SUDO} chown root:root "${OUT_DIR}/${dir}"
done

[ -d "${OUT_DIR}/${DESTDIR}"/bin ] && ${SUDO} rm -rf "${OUT_DIR}/${DESTDIR}"/bin

${SUDO} cp -a bin "${OUT_DIR}/${DESTDIR}"/bin

${SUDO} cp ./*.desktop "${OUT_DIR}/${DESTDIR}/share/applications"
${SUDO} cp -a conf/btop "${OUT_DIR}/${DESTDIR}"/share/${PKG}/btop
${SUDO} cp -a conf/console "${OUT_DIR}/${DESTDIR}"/share/${PKG}/console
${SUDO} cp -a conf/got "${OUT_DIR}/${DESTDIR}"/share/${PKG}/got
${SUDO} cp -a conf/jrnl "${OUT_DIR}/${DESTDIR}"/share/${PKG}/jrnl
${SUDO} cp -a conf/khard "${OUT_DIR}/${DESTDIR}"/share/${PKG}/khard
${SUDO} cp -a conf/kitty "${OUT_DIR}/${DESTDIR}"/share/${PKG}/kitty
${SUDO} cp -a conf/mutt "${OUT_DIR}/${DESTDIR}"/share/${PKG}/mutt
${SUDO} cp -a conf/mutt_multiple "${OUT_DIR}/${DESTDIR}"/share/${PKG}/mutt_multiple
${SUDO} cp -a conf/neomutt "${OUT_DIR}/${DESTDIR}"/share/${PKG}/neomutt
${SUDO} cp -a conf/newsboat "${OUT_DIR}/${DESTDIR}"/share/${PKG}/newsboat
${SUDO} cp -a conf/w3m "${OUT_DIR}/${DESTDIR}"/share/${PKG}/w3m
${SUDO} cp -a conf/tuir "${OUT_DIR}/${DESTDIR}"/share/${PKG}/tuir
${SUDO} cp conf/tmux.conf "${OUT_DIR}/${DESTDIR}"/share/${PKG}
${SUDO} cp conf/rifle.conf "${OUT_DIR}/${DESTDIR}"/share/${PKG}
${SUDO} cp conf/rainbow_config.json "${OUT_DIR}/${DESTDIR}"/share/${PKG}
${SUDO} cp copyright "${OUT_DIR}/${DESTDIR}"/share/doc/${PKG}
${SUDO} cp LICENSE "${OUT_DIR}/${DESTDIR}"/share/doc/${PKG}
${SUDO} cp CHANGELOG.md "${OUT_DIR}/${DESTDIR}"/share/doc/${PKG}
${SUDO} cp README.md "${OUT_DIR}/${DESTDIR}"/share/doc/${PKG}
${SUDO} cp VERSION "${OUT_DIR}/${DESTDIR}"/share/doc/${PKG}
${SUDO} gzip -9 "${OUT_DIR}/${DESTDIR}"/share/doc/${PKG}/CHANGELOG.md

${SUDO} cp -a art "${OUT_DIR}/${DESTDIR}/share/${PKG}/art"
${SUDO} mv "${OUT_DIR}/${DESTDIR}/share/${PKG}/art/pics" \
  "${OUT_DIR}/${DESTDIR}/share/${PKG}/pics"
${SUDO} cp -a music "${OUT_DIR}/${DESTDIR}/share/${PKG}/music"
${SUDO} cp -a tools "${OUT_DIR}/${DESTDIR}/share/${PKG}/tools"

${SUDO} gzip "${OUT_DIR}/${DESTDIR}"/share/${PKG}/art/*/*.asc

${SUDO} cp -a man/man1 "${OUT_DIR}/${DESTDIR}"/share/man/man1

${SUDO} cp -a share/menu "${OUT_DIR}/${DESTDIR}/share/menu"
${SUDO} cp -a share/figlet-fonts "${OUT_DIR}/${DESTDIR}/share/figlet-fonts"
${SUDO} cp -a share/neofetch-themes "${OUT_DIR}/${DESTDIR}/share/neofetch-themes"

${SUDO} chmod 644 "${OUT_DIR}/${DESTDIR}"/share/man/*/*
${SUDO} chmod 644 "${OUT_DIR}/${DESTDIR}"/share/menu/*
${SUDO} chmod 644 "${OUT_DIR}/${DESTDIR}"/share/figlet-fonts/*
find "${OUT_DIR}/${DESTDIR}"/share/doc/${PKG} -type d | while read -r dir; do
  ${SUDO} chmod 755 "${dir}"
done
find "${OUT_DIR}/${DESTDIR}"/share/doc/${PKG} -type f | while read -r f; do
  ${SUDO} chmod 644 "${f}"
done
find "${OUT_DIR}/${DESTDIR}"/share/${PKG} -type d | while read -r dir; do
  ${SUDO} chmod 755 "${dir}"
done
find "${OUT_DIR}/${DESTDIR}"/share/${PKG} -type f | while read -r f; do
  ${SUDO} chmod 644 "${f}"
done
${SUDO} chmod 755 "${OUT_DIR}/${DESTDIR}"/bin/* \
  "${OUT_DIR}/${DESTDIR}"/bin \
  "${OUT_DIR}/${DESTDIR}"/share/${PKG}/newsboat/*.sh \
  "${OUT_DIR}/${DESTDIR}"/share/${PKG}/newsboat/scripts/* \
  "${OUT_DIR}/${DESTDIR}"/share/${PKG}/newsboat/scripts/*/* \
  "${OUT_DIR}/${DESTDIR}"/share/menu \
  "${OUT_DIR}/${DESTDIR}"/share/figlet-fonts \
  "${OUT_DIR}/${DESTDIR}"/share/man \
  "${OUT_DIR}/${DESTDIR}"/share/man/*
${SUDO} chmod 755 "${OUT_DIR}/${DESTDIR}"/share/${PKG}/tools/bin/*
${SUDO} chown -R root:root "${OUT_DIR}/${DESTDIR}"/share
${SUDO} chown -R root:root "${OUT_DIR}/${DESTDIR}"/bin

echo "Building ${PKG_NAME}_${PKG_VER} rpm package"

[ -d pkg/rpm ] && cp -a pkg/rpm "${OUT_DIR}"/rpm
[ -d "${OUT_DIR}"/rpm ] || mkdir "${OUT_DIR}"/rpm

have_rpm=$(type -p rpmbuild)
[ "${have_rpm}" ] || {
  have_dnf=$(type -p dnf)
  if [ "${have_dnf}" ]; then
    ${SUDO} dnf install rpm-build
  else
    ${SUDO} apt-get update
    export DEBIAN_FRONTEND=noninteractive
    ${SUDO} ln -fs /usr/share/zoneinfo/America/Los_Angeles /etc/localtime
    ${SUDO} apt-get install rpm -y
    ${SUDO} dpkg-reconfigure --frontend noninteractive tzdata
  fi
}

ARCH=$(rpm --eval '%{_arch}')

rpmbuild -ba --build-in-place \
  --define "_topdir ${OUT_DIR}" \
  --define "_sourcedir ${OUT_DIR}" \
  --define "_version ${PKG_VER}" \
  --define "_release ${PKG_REL}" \
  --buildroot "${SRC}/${SRC_NAME}/${OUT_DIR}"/BUILDROOT \
  "${OUT_DIR}"/rpm/${PKG_NAME}.spec

# Rename RPMs if necessary
for rpmfile in "${OUT_DIR}"/RPMS/*/*.rpm; do
  [ "${rpmfile}" == "${OUT_DIR}/RPMS/*/*.rpm" ] && continue
  rpmbas=$(basename "${rpmfile}")
  rpmdir=$(dirname "${rpmfile}")
  newnam=$(echo "${rpmbas}" | sed -e "s/${PKG_NAME}-${PKG_VER}-${PKG_REL}/${PKG_NAME}_${PKG_VER}-${PKG_REL}/")
  [ "${rpmbas}" == "${newnam}" ] && continue
  mv "${rpmdir}/${rpmbas}" "${rpmdir}/${newnam}"
done

${SUDO} cp "${OUT_DIR}"/RPMS/*/*.rpm dist

cd "${OUT_DIR}" || echo "Unable to enter $OUT_DIR"
echo "Creating compressed tar archive of ${PKG_NAME} ${PKG_VER} distribution"
${SUDO} tar cf - usr/*/* | gzip -9 >../"${PKG_NAME}_${PKG_VER}-${PKG_REL}.${ARCH}".tgz

cd ../..

[ "${GCI}" ] || {
  [ -d releases ] || mkdir releases
  [ -d releases/"${PKG_VER}" ] || mkdir releases/"${PKG_VER}"
  ${SUDO} cp "${OUT_DIR}"/RPMS/*/*.rpm releases/"${PKG_VER}"
  ${SUDO} cp dist/*.tgz releases/"${PKG_VER}"
}
