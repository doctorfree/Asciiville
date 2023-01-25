#!/bin/bash

AVILLE_DIRS="/usr/local/share/asciiville \
/usr/local/share/doc/asciiville \
/usr/local/share/figlet-fonts"

AVILLE_FILES="/usr/local/bin/asciiart \
/usr/local/bin/any2ascii \
/usr/local/bin/asciibrow \
/usr/local/bin/show_endo \
/usr/local/bin/show_pokemon \
/usr/local/bin/asciijulia \
/usr/local/bin/asciimpplus \
/usr/local/bin/asciiplasma \
/usr/local/bin/asciisplash \
/usr/local/bin/asciiville \
/usr/local/bin/ascinit \
/usr/local/bin/make_ascii_art \
/usr/local/bin/set_xfce_font \
/usr/local/bin/set_xfce_trans \
/usr/local/bin/show_ascii_art \
/usr/local/bin/show_moon \
/usr/local/bin/show_weather \
/usr/local/bin/termprofset \
/usr/local/share/applications/asciiville.desktop \
/usr/local/share/man/man1/asciiart.1 \
/usr/local/share/man/man1/asciijulia.1 \
/usr/local/share/man/man1/asciimpplus.1 \
/usr/local/share/man/man1/asciiplasma.1 \
/usr/local/share/man/man1/asciisplash.1 \
/usr/local/share/man/man1/asciisplash-tmux.1 \
/usr/local/share/man/man1/asciiville.1 \
/usr/local/share/man/man1/ascinit.1 \
/usr/local/share/man/man1/show_ascii_art.1 \
/usr/local/share/man/man1/show_moon.1 \
/usr/local/share/man/man1/show_pokemon.1 \
/usr/local/share/man/man1/show_weather.1 \
/usr/local/share/man/man1/termprofset.1 \
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
if [ -f /etc/profile.d/asciiville.sh ]
then
  rm -f /etc/profile.d/asciiville.sh
fi
