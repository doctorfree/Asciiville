#!/bin/bash
#
# shellcheck disable=SC2001

platform=$(uname -s)
if [ "${platform}" == "Darwin" ]; then
  USRDIR="/usr/local"
else
  USRDIR="/usr"
fi

AVILLE_DIRS="${USRDIR}/share/asciiville
${USRDIR}/share/doc/asciiville
${USRDIR}/share/figlet-fonts
${USRDIR}/share/neofetch-themes"

AVILLE_FILES="${USRDIR}/bin/any2ascii
${USRDIR}/bin/asciiart
${USRDIR}/bin/asciibrow
${USRDIR}/bin/asciijulia
${USRDIR}/bin/asciimpplus
${USRDIR}/bin/asciiplasma
${USRDIR}/bin/asciisplash
${USRDIR}/bin/asciiville
${USRDIR}/bin/ascinit
${USRDIR}/bin/make_ascii_art
${USRDIR}/bin/set_xfce_font
${USRDIR}/bin/set_xfce_trans
${USRDIR}/bin/show_ascii_art
${USRDIR}/bin/show_moon
${USRDIR}/bin/show_pokemon
${USRDIR}/bin/show_weather
${USRDIR}/bin/termprofset
${USRDIR}/share/applications/asciiville.desktop
${USRDIR}/share/man/man1/asciiart.1
${USRDIR}/share/man/man1/asciijulia.1
${USRDIR}/share/man/man1/asciimpplus.1
${USRDIR}/share/man/man1/asciiplasma.1
${USRDIR}/share/man/man1/asciisplash-tmux.1
${USRDIR}/share/man/man1/asciisplash.1
${USRDIR}/share/man/man1/asciiville.1
${USRDIR}/share/man/man1/ascinit.1
${USRDIR}/share/man/man1/show_ascii_art.1
${USRDIR}/share/man/man1/show_moon.1
${USRDIR}/share/man/man1/show_pokemon.1
${USRDIR}/share/man/man1/show_weather.1
${USRDIR}/share/man/man1/termprofset.1
${USRDIR}/share/menu/asciiville"

user=$(id -u -n)

[ "${user}" == "root" ] || {
  echo "Uninstall-bin.sh must be run as the root user."
  echo "Use 'sudo ./Uninstall-bin.sh ...'"
  echo "Exiting"
  exit 1
}

for aville_file in ${AVILLE_FILES}; do
  rm -f "${aville_file}"
done
for aville_dir in ${AVILLE_DIRS}; do
  rm -rf "${aville_dir}"
done
if [ -f /etc/profile.d/asciiville.sh ]; then
  rm -f /etc/profile.d/asciiville.sh
fi

[ "$1" == "all" ] && {
  ANY_FILES="${USRDIR}/bin/any2a
  ${USRDIR}/bin/jp2a
  ${USRDIR}/share/man/man1/jp2a.1"
  ANY_DIRS="${USRDIR}/share/any2ascii
  ${USRDIR}/share/doc/any2ascii"
  AEWAN_FILES="${USRDIR}/bin/aecat
  ${USRDIR}/bin/aemakeflic
  ${USRDIR}/bin/aewan
  ${USRDIR}/share/man/man5/aewan.5
  ${USRDIR}/share/man/man1/aecat.1
  ${USRDIR}/share/man/man1/aemakeflic.1
  ${USRDIR}/share/man/man1/aewan.1"
  AEWAN_DIRS="${USRDIR}/share/asciiville-aewan
  ${USRDIR}/share/doc/asciiville-aewan"
  CBTP_DIRS="${USRDIR}/share/cbftp"
  CBTP_FILES="${USRDIR}/bin/cbftp \
              ${USRDIR}/bin/cbftp-debug \
              ${USRDIR}/bin/datafilecat \
              ${USRDIR}/bin/datafilewrite \
              ${USRDIR}/share/man/man1/cbftp.1"
  ENDO_DIRS="${USRDIR}/share/endoh1"
  ENDO_FILES="${USRDIR}/bin/show_endo"

  PROJECTFILES="${ANY_FILES} ${AEWAN_FILES} ${CBTP_FILES} ${ENDO_FILES}"
  PROJECTDIRS="${ANY_DIRS} ${AEWAN_DIRS} ${CBTP_DIRS} ${ENDO_DIRS}"

  for project_file in ${PROJECTFILES}; do
    rm -f "${project_file}"
  done
  for project_dir in ${PROJECTDIRS}; do
    rm -rf "${project_dir}"
  done
}
