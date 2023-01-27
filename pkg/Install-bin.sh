#!/bin/bash

usage() {
  echo "Usage: sudo ./Install-bin.sh /path/to/Asciiville_<version>-<release>.<arch>.<suffix>"
  echo "Where:"
  printf "\n\t<suffix> must be a gzip'd tar archive with filename suffix 'tgz'\n"
  echo "Download the latest gzip'd distribution archive at"
  echo "https://github.com/doctorfree/Asciiville/releases"
  exit 1
}

user=`id -u -n`

[ "${user}" == "root" ] || {
  echo "Install-bin.sh must be run as the root user."
  echo "Use 'sudo ./Install-bin.sh ...'"
  echo "Exiting"
  exit 1
}

[ "$1" ] || {
  echo "Install-bin.sh requires a pathname argument specifying"
  echo "the distribution archive to extract and install."
  echo "No argument provided."
  usage
}

[ -f "$1" ] || {
  echo "Install-bin.sh requires a pathname argument specifying"
  echo "the distribution archive to extract and install."
  echo "$1 does not exist or is not a plain file."
  usage
}

PATH_TO_ARCHIVE="$1"
ARCHIVE=`basename "${PATH_TO_ARCHIVE}"`
pkgname=`echo ${ARCHIVE} | awk -F '_' '{ print $1 }'`

[ "${pkgname}" == "Asciiville" ] || {
  echo "Distribution archive filename must be of the format:"
  printf "\n\tAsciiville_<version>-<release>.<arch>.<suffix>\n"
  echo "Distribution archive filename ${ARCHIVE} does not begin with 'Asciiville_'"
  usage
}

pkgvra=`echo ${ARCHIVE} | awk -F '-' '{ print $1 }'`
pkgver=`echo ${pkgvra} | awk -F '_' '{ print $2 }'`
pkgrag=`echo ${ARCHIVE} | awk -F '-' '{ print $2 }'`
pkgrel=`echo ${pkgrag} | awk -F '.' '{ print $1 }'`
pkgarc=`echo ${pkgrag} | awk -F '.' '{ print $2 }'`
pkgsuf=`echo ${pkgrag} | awk -F '.' '{ print $3 }'`

echo "Preparing to install ${pkgname} Version ${pkgver} Release ${pkgrel}"
echo "for architecture ${pkgarc}"
echo ""
echo "This installation method is recommended only if your system is not"
echo "supported by one of the packaged installation formats (Debian/RPM)"
echo "available at https://github.com/doctorfree/Asciiville/releases"
echo ""
while true
do
  read -p "Proceed with installation? (y/n) " answer
  case ${answer} in
    [Yy]* )
      printf "\nProceeding with installation.\n"
      break
      ;;
    [Nn]* )
      printf "\nSkipping installation.\n"
      exit 1
      ;;
    * ) echo "Please answer 'y' to install, or 'n' to exit."
      ;;
  esac
done

if [ "${pkgsuf}" == "tgz" ]
then
  tar -mxzf ${PATH_TO_ARCHIVE} -C /
else
  echo "Unrecognized filename suffix '${pkgsuf}'"
  echo "Install-bin.sh requires a filename suffix of 'tgz'"
  usage
fi

export PATH=${PATH}:/usr/local/bin:/snap/bin
python3_inst=`type -p python3`
if [ "${python3_inst}" ]
then
  PYTHON="python3"
else
  PYTHON="python"
fi

lol_inst=`type -p lolcat`
if [ ! "${lol_inst}" ]
then
  gem_inst=`type -p gem`
  if [ "${gem_inst}" ]
  then
    wget_inst=`type -p wget`
    if [ "${wget_inst}" ]
    then
      if [ "${unzip_inst}" ]
      then
        wget --quiet https://github.com/busyloop/lolcat/archive/master.zip
        unzip -qq master.zip
        cd lolcat-master/bin
        gem install lolcat
        cd ../..
        rm -f master.zip
        rm -rf lolcat-master
      fi
    fi
  fi
fi

FIGLET_DIR="/usr/local/share/figlet-fonts"
FIGLET_ZIP="figlet-fonts.zip"
zip_inst=`type -p zip`
if [ "${zip_inst}" ]
then
  ${PYTHON} -m pip install setuptools > /dev/null 2>&1
  pyfig_inst=`type -p pyfiglet`
  [ "${pyfig_inst}" ] || {
    ${PYTHON} -m pip install pyfiglet > /dev/null 2>&1
    pyfig_inst=`type -p pyfiglet`
  }
  if [ "${pyfig_inst}" ]
  then
    if [ -d ${FIGLET_DIR} ]
    then
      cd ${FIGLET_DIR}
      zip -q ${FIGLET_ZIP} *.flf
      pyfiglet -L ${FIGLET_ZIP}
      rm -f ${FIGLET_ZIP}
    fi
  fi
fi

# Add /usr/local/bin and /usr/local/games to PATH
if [ -d /etc/profile.d ]
then
  if [ -f /etc/profile.d/asciiville.sh ]
  then
    echo 'export PATH="$PATH:/usr/local/bin:/usr/local/games"' >> /etc/profile.d/asciiville.sh
  else
    echo 'export PATH="$PATH:/usr/local/bin:/usr/local/games"' > /etc/profile.d/asciiville.sh
  fi
else
  [ -f /etc/profile ] && {
    echo 'export PATH="$PATH:/usr/local/bin:/usr/local/games"' >> /etc/profile
  }
fi
if [ -d /etc/zsh ]
then
  if [ -f /etc/zsh/zshenv ]
  then
    echo 'export PATH="$PATH:/usr/local/bin:/usr/local/games"' >> /etc/zsh/zshenv
  else
    echo 'export PATH="$PATH:/usr/local/bin:/usr/bin:/bin:/usr/games:/usr/local/games"' > /etc/zsh/zshenv
  fi
else
  [ -f /etc/zshrc ] && {
    echo 'export PATH="$PATH:/usr/local/bin:/usr/local/games"' >> /etc/zshrc
  }
fi

echo "Installation of ${pkgname} Version ${pkgver} Release ${pkgrel}"
echo "for architecture ${pkgarc} complete. Add '/usr/local/bin' to PATH."
echo ""
echo "Initialize the Asciiville environment with 'ascinit':"
echo ""
echo "As a normal user with sudo privilege, execute the Asciiville"
echo "initialization command '/usr/local/bin/ascinit'"

exit 0
