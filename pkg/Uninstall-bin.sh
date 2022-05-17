#!/bin/bash

AVILLE_DIRS="/usr/share/doc/asciiville \
/usr/share/btop \
/usr/share/asciiville \
/usr/share/figlet-fonts \
/usr/games/lib/ninvaders \
/usr/games/share/doc/tetris \
/usr/games/share/doc/nethack \
/usr/games/share/nethack \
/usr/games/var/nethack"

AVILLE_FILES="/usr/share/man/man6/nethack.6 \
/usr/share/man/man6/lev_comp.6 \
/usr/share/man/man6/dgn_comp.6 \
/usr/share/man/man6/recover.6 \
/usr/share/man/man6/dlb.6 \
/usr/share/man/man1/asciisplash-tmux.1 \
/usr/share/man/man1/asciisplash.1 \
/usr/share/man/man1/cbftp.1 \
/usr/share/man/man1/btop.1 \
/usr/share/man/man1/show_moon.1 \
/usr/share/man/man1/show_weather.1 \
/usr/share/man/man1/asciiart.1 \
/usr/share/man/man1/asciijulia.1 \
/usr/share/man/man1/asciimpplus.1 \
/usr/share/man/man1/asciiplasma.1 \
/usr/share/man/man1/diagon.1 \
/usr/share/man/man1/ascinit.1 \
/usr/share/man/man1/asciiville.1 \
/usr/share/man/man1/termprofset.1 \
/usr/share/man/man1/show_ascii_art.1 \
/usr/share/man/man1/ddgr.1 \
/usr/share/man/man1/jp2a.1 \
/usr/share/applications/asciiville.desktop \
/usr/share/menu/asciiville \
/usr/games/bin/nethack \
/usr/games/bin/ninvaders \
/usr/games/bin/tetris \
/usr/games/bin/gameserver \
/usr/games/share/pixmaps/tetris.xpm \
/usr/games/share/applications/tetris.desktop \
/usr/games/var/tetris-hiscores \
/usr/games/nethack \
/usr/games/ninvaders \
/usr/games/tetris \
/usr/games/gameserver \
/usr/bin/btop-tmux \
/usr/bin/chktermcolor \
/usr/bin/show_figlet_fonts \
/usr/bin/asciijulia \
/usr/bin/asciimpplus \
/usr/bin/asciiplasma \
/usr/bin/show_moon \
/usr/bin/show_weather \
/usr/bin/asciiart \
/usr/bin/asciisplash \
/usr/bin/asciisplash-tmux \
/usr/bin/bb-tmux \
/usr/bin/make_ascii_art \
/usr/bin/ascinit \
/usr/bin/set_xfce_font \
/usr/bin/set_xfce_trans \
/usr/bin/termprofset \
/usr/bin/asciiville \
/usr/bin/show_ascii_art \
/usr/bin/btop \
/usr/bin/jp2a \
/usr/bin/cbftp \
/usr/bin/cbftp-debug \
/usr/bin/datafilecat \
/usr/bin/datafilewrite \
/usr/bin/ddgr"

rm -f ${AVILLE_FILES}
rm -rf ${AVILLE_DIRS}
