#!/bin/bash
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

cd "${SRC}/${SRC_NAME}"

# Build btop
if [ -x build ]
then
  ./build btop
else
  cd btop
  make distclean
  make STATIC=true STRIP=true
  chmod +x bin/btop
  cd ..
fi

# Build cbftp
if [ -x build ]
then
  ./build cbftp
else
  cd cbftp
  make clean
  make
  chmod +x bin/*
  cd ..
fi

# Build endoh1
if [ -x build ]
then
  ./build endo
else
  cd endoh1
  make clobber
  make everything
  chmod +x endoh1 endoh1_color
  cd ..
fi

# Build nethack
if [ -x build ]
then
  ./build nethack
else
  cd games/nethack
  ./configure --prefix=/usr/games \
              --with-owner=games \
              --with-group=games \
              --enable-wizmode=doctorwhen
  make
  cd ../..
fi

# Build tetris
if [ -x build ]
then
  ./build tetris
else
  cd games/tetris
  [ -f tetris ] || {
    ./configure.sh --prefix=/usr/games --enable-xlib=no --enable-curses=yes
    make
    make gameserver
  }
  cd ../..
fi

${SUDO} rm -rf dist
mkdir dist

[ -d ${OUT_DIR} ] && rm -rf ${OUT_DIR}
mkdir ${OUT_DIR}

for dir in "${DESTDIR}" "${DESTDIR}/share" "${DESTDIR}/share/man" \
           "${DESTDIR}/share/applications" "${DESTDIR}/share/doc" \
           "${DESTDIR}/share/doc/${PKG}" "${DESTDIR}/share/btop" \
           "${DESTDIR}/share/${PKG}" "${DESTDIR}/games" "${DESTDIR}/games/bin" \
           "${DESTDIR}/games/lib" \
           "${DESTDIR}/games/share" "${DESTDIR}/games/share/doc" \
           "${DESTDIR}/games/share/doc/tetris" \
           "${DESTDIR}/games/share/pixmaps" \
           "${DESTDIR}/games/share/applications"
do
    [ -d ${OUT_DIR}/${dir} ] || ${SUDO} mkdir ${OUT_DIR}/${dir}
    ${SUDO} chown root:root ${OUT_DIR}/${dir}
done

for dir in bin
do
    [ -d ${OUT_DIR}/${DESTDIR}/${dir} ] && ${SUDO} rm -rf ${OUT_DIR}/${DESTDIR}/${dir}
done

${SUDO} cp -a bin ${OUT_DIR}/${DESTDIR}/bin
${SUDO} cp btop/bin/btop ${OUT_DIR}/${DESTDIR}/bin/btop
${SUDO} cp cbftp/bin/* ${OUT_DIR}/${DESTDIR}/bin

${SUDO} cp -a endoh1 ${OUT_DIR}/${DESTDIR}/share/${PKG}/endoh1
${SUDO} chmod 755 ${OUT_DIR}/${DESTDIR}/share/${PKG}/endoh1/endoh1
${SUDO} chmod 755 ${OUT_DIR}/${DESTDIR}/share/${PKG}/endoh1/endoh1_color

# Revised NetHack install using UnNetHack mods
cd games/nethack
${SUDO} make DESTDIR=${OUT_DIR} install
${SUDO} cp dat/license ${OUT_DIR}/${DESTDIR}/games/share/nethack/license
cd doc
${SUDO} make DESTDIR=${OUT_DIR} manpages
${SUDO} chown games ${OUT_DIR}/${DESTDIR}/games/bin/nethack
${SUDO} chgrp games ${OUT_DIR}/${DESTDIR}/games/bin/nethack
${SUDO} chmod 04755 ${OUT_DIR}/${DESTDIR}/games/bin/nethack
${SUDO} ln -r -s ${OUT_DIR}/${DESTDIR}/games/bin/nethack ${OUT_DIR}/${DESTDIR}/games/nethack
cd ../..

# Tetris
${SUDO} cp tetris/tetris ${OUT_DIR}/${DESTDIR}/games/bin
${SUDO} chown games ${OUT_DIR}/${DESTDIR}/games/bin/tetris
${SUDO} chgrp games ${OUT_DIR}/${DESTDIR}/games/bin/tetris
${SUDO} chmod 04755 ${OUT_DIR}/${DESTDIR}/games/bin/tetris
${SUDO} ln -r -s ${OUT_DIR}/${DESTDIR}/games/bin/tetris ${OUT_DIR}/${DESTDIR}/games/tetris
${SUDO} cp tetris/gameserver ${OUT_DIR}/${DESTDIR}/games/bin
${SUDO} chown games ${OUT_DIR}/${DESTDIR}/games/bin/gameserver
${SUDO} chgrp games ${OUT_DIR}/${DESTDIR}/games/bin/gameserver
${SUDO} chmod 04755 ${OUT_DIR}/${DESTDIR}/games/bin/gameserver
${SUDO} ln -r -s ${OUT_DIR}/${DESTDIR}/games/bin/gameserver ${OUT_DIR}/${DESTDIR}/games/gameserver

${SUDO} cp tetris/licence.txt ${OUT_DIR}/${DESTDIR}/games/share/doc/tetris
${SUDO} cp tetris/README ${OUT_DIR}/${DESTDIR}/games/share/doc/tetris
${SUDO} cp tetris/INSTALL ${OUT_DIR}/${DESTDIR}/games/share/doc/tetris
${SUDO} cp tetris/tetris.xpm ${OUT_DIR}/${DESTDIR}/games/share/pixmaps
${SUDO} cp tetris/tetris.desktop ${OUT_DIR}/${DESTDIR}/games/share/applications
${SUDO} touch ${OUT_DIR}/${DESTDIR}/games/var/tetris-hiscores
${SUDO} chown games ${OUT_DIR}/${DESTDIR}/games/var/tetris-hiscores
${SUDO} chgrp games ${OUT_DIR}/${DESTDIR}/games/var/tetris-hiscores
${SUDO} chmod 0664 ${OUT_DIR}/${DESTDIR}/games/var/tetris-hiscores
cd ..

${SUDO} cp *.desktop "${OUT_DIR}/${DESTDIR}/share/applications"
${SUDO} cp -a conf/console ${OUT_DIR}/${DESTDIR}/share/${PKG}/console
${SUDO} cp -a conf/got ${OUT_DIR}/${DESTDIR}/share/${PKG}/got
${SUDO} cp -a conf/jrnl ${OUT_DIR}/${DESTDIR}/share/${PKG}/jrnl
${SUDO} cp -a conf/khard ${OUT_DIR}/${DESTDIR}/share/${PKG}/khard
${SUDO} cp -a conf/kitty ${OUT_DIR}/${DESTDIR}/share/${PKG}/kitty
${SUDO} cp -a conf/mutt ${OUT_DIR}/${DESTDIR}/share/${PKG}/mutt
${SUDO} cp -a conf/mutt_multiple ${OUT_DIR}/${DESTDIR}/share/${PKG}/mutt_multiple
${SUDO} cp -a conf/neomutt ${OUT_DIR}/${DESTDIR}/share/${PKG}/neomutt
${SUDO} cp -a conf/newsboat ${OUT_DIR}/${DESTDIR}/share/${PKG}/newsboat
${SUDO} cp -a conf/w3m ${OUT_DIR}/${DESTDIR}/share/${PKG}/w3m
${SUDO} cp -a conf/tuir ${OUT_DIR}/${DESTDIR}/share/${PKG}/tuir
${SUDO} cp conf/tmux.conf ${OUT_DIR}/${DESTDIR}/share/${PKG}
${SUDO} cp conf/rifle.conf ${OUT_DIR}/${DESTDIR}/share/${PKG}
${SUDO} cp conf/rainbow_config.json ${OUT_DIR}/${DESTDIR}/share/${PKG}
${SUDO} cp copyright ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}
${SUDO} cp LICENSE ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}
${SUDO} cp NOTICE ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}
${SUDO} cp CHANGELOG.md ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}
${SUDO} cp README.md ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}
${SUDO} pandoc -f gfm README.md | ${SUDO} tee ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}/README.html > /dev/null
${SUDO} cp VERSION ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}
${SUDO} cp btop/README.md ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}/README-btop.md
${SUDO} cp btop/LICENSE ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}/LICENSE-btop
${SUDO} cp btop/README.md ${OUT_DIR}/${DESTDIR}/share/btop/README.md
${SUDO} cp btop/LICENSE ${OUT_DIR}/${DESTDIR}/share/btop/LICENSE
${SUDO} cp -a btop/themes ${OUT_DIR}/${DESTDIR}/share/btop/themes
${SUDO} cp btop/btop.desktop "${OUT_DIR}/${DESTDIR}/share/applications"
${SUDO} mkdir -p ${OUT_DIR}/${DESTDIR}/share/icons
${SUDO} mkdir -p ${OUT_DIR}/${DESTDIR}/share/icons/hicolor
${SUDO} mkdir -p ${OUT_DIR}/${DESTDIR}/share/icons/hicolor/48x48
${SUDO} mkdir -p ${OUT_DIR}/${DESTDIR}/share/icons/hicolor/48x48/apps
${SUDO} mkdir -p ${OUT_DIR}/${DESTDIR}/share/icons/hicolor/scalable
${SUDO} mkdir -p ${OUT_DIR}/${DESTDIR}/share/icons/hicolor/scalable/apps
${SUDO} cp btop/Img/icon.png "${OUT_DIR}/${DESTDIR}/share/icons/hicolor/48x48/apps/btop.png"
${SUDO} cp btop/Img/icon.svg "${OUT_DIR}/${DESTDIR}/share/icons/hicolor/scalable/apps/btop.svg"
${SUDO} cp cbftp/README ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}/README-cbftp
${SUDO} cp cbftp/LICENSE ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}/LICENSE-cbftp
${SUDO} cp games/tetris/licence.txt ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}/license-tetris
${SUDO} cp games/tetris/README ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}/README-tetris
${SUDO} gzip -9 ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}/CHANGELOG.md

${SUDO} cp -a art "${OUT_DIR}/${DESTDIR}/share/${PKG}/art"
${SUDO} mv "${OUT_DIR}/${DESTDIR}/share/${PKG}/art/pics" \
           "${OUT_DIR}/${DESTDIR}/share/${PKG}/pics"
${SUDO} cp -a music "${OUT_DIR}/${DESTDIR}/share/${PKG}/music"
${SUDO} cp -a tools "${OUT_DIR}/${DESTDIR}/share/${PKG}/tools"

${SUDO} gzip ${OUT_DIR}/${DESTDIR}/share/${PKG}/art/*/*.asc

${SUDO} cp -a man/man1 ${OUT_DIR}/${DESTDIR}/share/man/man1
[ -d ${OUT_DIR}/${DESTDIR}/share/man/man5 ] || {
  ${SUDO} mkdir -p ${OUT_DIR}/${DESTDIR}/share/man/man5
}

[ -d ${OUT_DIR}/${DESTDIR}/share/man/man6 ] || {
  ${SUDO} mkdir -p ${OUT_DIR}/${DESTDIR}/share/man/man6
}
${SUDO} cp games/nethack/dat/license ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}/LICENSE-nethack
${SUDO} cp games/nethack/README ${OUT_DIR}/${DESTDIR}/share/doc/${PKG}/README-nethack

${SUDO} cp -a share/menu "${OUT_DIR}/${DESTDIR}/share/menu"
${SUDO} cp -a share/figlet-fonts "${OUT_DIR}/${DESTDIR}/share/figlet-fonts"
${SUDO} cp -a share/neofetch-themes "${OUT_DIR}/${DESTDIR}/share/neofetch-themes"

${SUDO} chmod 644 ${OUT_DIR}/${DESTDIR}/share/man/*/*
${SUDO} chmod 644 ${OUT_DIR}/${DESTDIR}/share/menu/*
${SUDO} chmod 644 ${OUT_DIR}/${DESTDIR}/share/figlet-fonts/*
${SUDO} chmod 755 ${OUT_DIR}/${DESTDIR}/bin/* \
                  ${OUT_DIR}/${DESTDIR}/bin \
                  ${OUT_DIR}/${DESTDIR}/share/menu \
                  ${OUT_DIR}/${DESTDIR}/share/figlet-fonts \
                  ${OUT_DIR}/${DESTDIR}/share/man \
                  ${OUT_DIR}/${DESTDIR}/share/man/*
find ${OUT_DIR}/${DESTDIR}/share/doc/${PKG} -type d | while read dir
do
  ${SUDO} chmod 755 "${dir}"
done
find ${OUT_DIR}/${DESTDIR}/share/doc/${PKG} -type f | while read f
do
  ${SUDO} chmod 644 "${f}"
done
find ${OUT_DIR}/${DESTDIR}/share/${PKG} -type d | while read dir
do
  ${SUDO} chmod 755 "${dir}"
done
find ${OUT_DIR}/${DESTDIR}/share/${PKG} -type f | while read f
do
  ${SUDO} chmod 644 "${f}"
done
${SUDO} chmod 755 ${OUT_DIR}/${DESTDIR}/share/${PKG}/tools/bin/*
${SUDO} chown -R root:root ${OUT_DIR}/${DESTDIR}/share
${SUDO} chown -R root:root ${OUT_DIR}/${DESTDIR}/bin
${SUDO} chown -R games:games ${OUT_DIR}/${DESTDIR}/games/var

echo "Building ${PKG_NAME}_${PKG_VER} rpm package"

[ -d pkg/rpm ] && cp -a pkg/rpm ${OUT_DIR}/rpm
[ -d ${OUT_DIR}/rpm ] || mkdir ${OUT_DIR}/rpm

have_rpm=`type -p rpmbuild`
[ "${have_rpm}" ] || {
  have_dnf=`type -p dnf`
  if [ "${have_dnf}" ]
  then
    ${SUDO} dnf install rpm-build
  else
    ${SUDO} apt-get update
    export DEBIAN_FRONTEND=noninteractive
    ${SUDO} ln -fs /usr/share/zoneinfo/America/Los_Angeles /etc/localtime
    ${SUDO} apt-get install rpm -y
    ${SUDO} dpkg-reconfigure --frontend noninteractive tzdata
  fi
}

ARCH=`rpm --eval '%{_arch}'`

rpmbuild -ba --build-in-place \
         --define "_topdir ${OUT_DIR}" \
         --define "_sourcedir ${OUT_DIR}" \
         --define "_version ${PKG_VER}" \
         --define "_release ${PKG_REL}" \
         --buildroot ${SRC}/${SRC_NAME}/${OUT_DIR}/BUILDROOT \
         ${OUT_DIR}/rpm/${PKG_NAME}.spec

# Rename RPMs if necessary
for rpmfile in ${OUT_DIR}/RPMS/*/*.rpm
do
  [ "${rpmfile}" == "${OUT_DIR}/RPMS/*/*.rpm" ] && continue
  rpmbas=`basename ${rpmfile}`
  rpmdir=`dirname ${rpmfile}`
  newnam=`echo ${rpmbas} | sed -e "s/${PKG_NAME}-${PKG_VER}-${PKG_REL}/${PKG_NAME}_${PKG_VER}-${PKG_REL}/"`
  [ "${rpmbas}" == "${newnam}" ] && continue
  mv ${rpmdir}/${rpmbas} ${rpmdir}/${newnam}
done

${SUDO} cp ${OUT_DIR}/RPMS/*/*.rpm dist

cd ${OUT_DIR}
echo "Creating compressed tar archive of ${PKG_NAME} ${PKG_VER} distribution"
${SUDO} tar cf - usr | gzip -9 > ../${PKG_NAME}_${PKG_VER}-${PKG_REL}.${ARCH}.tgz

have_zip=`type -p zip`
[ "${have_zip}" ] || {
  ${SUDO} yum install zip -y
}
echo "Creating zip archive of ${PKG_NAME} ${PKG_VER} distribution"
${SUDO} zip -q -r ../${PKG_NAME}_${PKG_VER}-${PKG_REL}.${ARCH}.zip usr
cd ../..

[ "${GCI}" ] || {
  [ -d releases ] || mkdir releases
  [ -d releases/${PKG_VER} ] || mkdir releases/${PKG_VER}
  ${SUDO} cp ${OUT_DIR}/RPMS/*/*.rpm releases/${PKG_VER}
  ${SUDO} cp dist/*.tgz dist/*.zip releases/${PKG_VER}
}
