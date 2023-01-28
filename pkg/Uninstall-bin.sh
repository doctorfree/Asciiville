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

[ "$1" == "all" ] && {
  ANY_FILES="usr/local/bin/any2a
             usr/local/bin/jp2a
             usr/local/share/man/man1/jp2a.1"
  ANY_DIRS="usr/local/share/any2ascii
            usr/local/share/doc/any2ascii"
  AEWAN_FILES="usr/local/bin/aecat
               usr/local/bin/aemakeflic
               usr/local/bin/aewan
               usr/local/share/man/man5/aewan.5
               usr/local/share/man/man1/aecat.1
               usr/local/share/man/man1/aemakeflic.1
               usr/local/share/man/man1/aewan.1"
  AEWAN_DIRS="usr/local/share/asciiville-aewan
              usr/local/share/doc/asciiville-aewan"
  CBTP_DIRS="/usr/local/share/cbftp"
  CBTP_FILES="/usr/local/bin/cbftp \
              /usr/local/bin/cbftp-debug \
              /usr/local/bin/datafilecat \
              /usr/local/bin/datafilewrite \
              /usr/local/share/man/man1/cbftp.1"
  ENDO_DIRS="/usr/local/share/endoh1"
  ENDO_FILES="/usr/local/bin/show_endo"

  PROJECTFILES="${ANY_FILES} ${AEWAN_FILES} ${CBTP_FILES} ${ENDO_FILES}"
  PROJECTDIRS="${ANY_DIRS} ${AEWAN_DIRS} ${CBTP_DIRS} ${ENDO_DIRS}"

  rm -f ${PROJECTFILES}
  rm -rf ${PROJECTDIRS}
}
