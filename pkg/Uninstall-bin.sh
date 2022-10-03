#!/bin/bash

AVILLE_DIRS="/usr/games/lib/ninvaders \
/usr/games/share/doc/nethack \
/usr/games/share/doc/tetris \
/usr/games/share/nethack \
/usr/games/var/nethack \
/usr/share/asciiville \
/usr/share/btop \
/usr/share/doc/asciiville \
/usr/share/figlet-fonts"

AVILLE_FILES="/usr/bin/asciiart \
/usr/bin/asciijulia \
/usr/bin/asciimpplus \
/usr/bin/asciiplasma \
/usr/bin/asciisplash \
/usr/bin/asciiville \
/usr/bin/ascinit \
/usr/bin/btop \
/usr/bin/cbftp \
/usr/bin/cbftp-debug \
/usr/bin/datafilecat \
/usr/bin/datafilewrite \
/usr/bin/jp2a \
/usr/bin/make_ascii_art \
/usr/bin/set_xfce_font \
/usr/bin/set_xfce_trans \
/usr/bin/show_ascii_art \
/usr/bin/show_moon \
/usr/bin/show_weather \
/usr/bin/termprofset \
/usr/games/bin/gameserver \
/usr/games/bin/nethack \
/usr/games/bin/ninvaders \
/usr/games/bin/tetris \
/usr/games/gameserver \
/usr/games/nethack \
/usr/games/ninvaders \
/usr/games/share/applications/tetris.desktop \
/usr/games/share/pixmaps/tetris.xpm \
/usr/games/tetris \
/usr/games/var/tetris-hiscores \
/usr/share/applications/asciiville.desktop \
/usr/share/man/man1/asciiart.1 \
/usr/share/man/man1/asciijulia.1 \
/usr/share/man/man1/asciimpplus.1 \
/usr/share/man/man1/asciiplasma.1 \
/usr/share/man/man1/asciisplash.1 \
/usr/share/man/man1/asciisplash-tmux.1 \
/usr/share/man/man1/asciiville.1 \
/usr/share/man/man1/ascinit.1 \
/usr/share/man/man1/btop.1 \
/usr/share/man/man1/cbftp.1 \
/usr/share/man/man1/jp2a.1 \
/usr/share/man/man1/show_ascii_art.1 \
/usr/share/man/man1/show_moon.1 \
/usr/share/man/man1/show_weather.1 \
/usr/share/man/man1/termprofset.1 \
/usr/share/man/man6/dgn_comp.6 \
/usr/share/man/man6/dlb.6 \
/usr/share/man/man6/lev_comp.6 \
/usr/share/man/man6/nethack.6 \
/usr/share/man/man6/recover.6 \
/usr/share/menu/asciiville"

user=`id -u -n`

[ "${user}" == "root" ] || {
  echo "Uninstall-bin.sh must be run as the root user."
  echo "Use 'sudo ./Uninstall-bin.sh ...'"
  echo "Exiting"
  exit 1
}

rm -f ${AVILLE_FILES}
rm -rf ${AVILLE_DIRS}
