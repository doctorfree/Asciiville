#!/bin/bash

AVILLE_DIRS="/usr/local/games/share/doc/nethack \
/usr/local/games/share/doc/tetris \
/usr/local/games/share/nethack \
/usr/local/games/var/nethack \
/usr/local/share/asciiville \
/usr/local/share/btop \
/usr/local/share/doc/asciiville \
/usr/local/share/figlet-fonts"

AVILLE_FILES="/usr/local/bin/asciiart \
/usr/local/bin/asciijulia \
/usr/local/bin/asciimpplus \
/usr/local/bin/asciiplasma \
/usr/local/bin/asciisplash \
/usr/local/bin/asciiville \
/usr/local/bin/ascinit \
/usr/local/bin/btop \
/usr/local/bin/cbftp \
/usr/local/bin/cbftp-debug \
/usr/local/bin/datafilecat \
/usr/local/bin/datafilewrite \
/usr/local/bin/make_ascii_art \
/usr/local/bin/set_xfce_font \
/usr/local/bin/set_xfce_trans \
/usr/local/bin/show_ascii_art \
/usr/local/bin/show_moon \
/usr/local/bin/show_weather \
/usr/local/bin/termprofset \
/usr/local/games/bin/gameserver \
/usr/local/games/bin/nethack \
/usr/local/games/bin/tetris \
/usr/local/games/gameserver \
/usr/local/games/nethack \
/usr/local/games/share/applications/tetris.desktop \
/usr/local/games/share/pixmaps/tetris.xpm \
/usr/local/games/tetris \
/usr/local/games/var/tetris-hiscores \
/usr/local/share/applications/asciiville.desktop \
/usr/local/share/man/man1/asciiart.1 \
/usr/local/share/man/man1/asciijulia.1 \
/usr/local/share/man/man1/asciimpplus.1 \
/usr/local/share/man/man1/asciiplasma.1 \
/usr/local/share/man/man1/asciisplash.1 \
/usr/local/share/man/man1/asciisplash-tmux.1 \
/usr/local/share/man/man1/asciiville.1 \
/usr/local/share/man/man1/ascinit.1 \
/usr/local/share/man/man1/btop.1 \
/usr/local/share/man/man1/cbftp.1 \
/usr/local/share/man/man1/show_ascii_art.1 \
/usr/local/share/man/man1/show_moon.1 \
/usr/local/share/man/man1/show_weather.1 \
/usr/local/share/man/man1/termprofset.1 \
/usr/local/share/man/man6/dgn_comp.6 \
/usr/local/share/man/man6/dlb.6 \
/usr/local/share/man/man6/lev_comp.6 \
/usr/local/share/man/man6/nethack.6 \
/usr/local/share/man/man6/recover.6 \
/usr/local/share/menu/asciiville"

user=`id -u -n`

[ "${user}" == "root" ] || {
  echo "Uninstall-bin.sh must be run as the root user."
  echo "Use 'sudo ./Uninstall-bin.sh ...'"
  echo "Exiting"
  exit 1
}

rm -f ${AVILLE_FILES}
rm -rf ${AVILLE_DIRS}
