#!/bin/bash

usage() {
  echo "Usage: sudo ./Install-bin.sh /path/to/Asciiville_<version>-<release>.<arch>.<suffix>"
  echo "Where:"
  printf "\n\t<suffix> can be a gzip'd tar archive with filename suffix 'tgz'"
  printf "\n\t\tor a zip archive with filename suffix 'zip'\n"
  echo "Download the latest gzip'd or zip'd binary distribution archive at"
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

platform=`uname -s`
have_brew=`type -p brew`
have_btop=`type -p btop`
[ "${platform}" == "Darwin" ] && [ "${have_brew}" ] && {
  [ "${have_btop}" ] || {
    echo "Installing btop with Brew"
    brew install btop
  }
}

unzip_inst=`type -p unzip`
if [ "${pkgsuf}" == "tgz" ]
then
  sudo tar -mxzf ${PATH_TO_ARCHIVE} -C /
else
  if [ "${pkgsuf}" == "zip" ]
  then
    [ "${unzip_inst}" ] || {
      echo "The 'unzip' utility is not installed or not in the execution path"
      echo "Install-bin.sh requires 'unzip' to install a 'zip' archive"
      usage
    }
    sudo unzip ${PATH_TO_ARCHIVE} –d /
  else
    echo "Unrecognized filename suffix '${pkgsuf}'"
    echo "Install-bin.sh requires a filename suffix of 'tgz' or 'zip'"
    usage
  fi
fi

export PATH=${PATH}:/usr/local/bin:/snap/bin
python3_inst=`type -p python3`
if [ "${python3_inst}" ]
then
  PYTHON="python3"
else
  PYTHON="python"
fi
${PYTHON} -m pip install setuptools
${PYTHON} -m pip install asciimatics
${PYTHON} -m pip install rainbowstream
npm_inst=`type -p npm`
if [ "${npm_inst}" ]
then
  npm install -g mapscii
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

FIGLET_DIR="/usr/share/figlet-fonts"
FIGLET_ZIP="figlet-fonts.zip"
zip_inst=`type -p zip`
if [ "${zip_inst}" ]
then
  pyfig_inst=`type -p pyfiglet`
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

echo "Installation of ${pkgname} Version ${pkgver} Release ${pkgrel}"
echo "for architecture ${pkgarc} complete."
echo ""
echo "The manual installation of ${pkgname} does not automatically"
echo "install dependencies. Package names vary from platform to platform."
echo "It may be necessary to install one or more of the following:"
echo ""
echo "uuid-runtime, python3, python3-pip, python3-venv, tmux, mplayer, w3m,"
echo "neomutt, newsboat, ranger, jq, asciinema, speedtest-cli, libaa-bin,"
echo "libaa1, bb, neofetch, figlet, ImageMagick/imagemagick, dconf/dconf-cli,"
echo "libncurses-dev/ncurses, libjpeg-dev/libjpeg-turbo, libpng/libpng-dev,"
echo "cmatrix, gnupg, pandoc, util-linux"
echo ""
echo "After installing dependencies, initialize the Asciiville environment."
echo "As a normal user with sudo privilege, execute the Asciiville"
echo "initialization command 'ascinit' or 'ascinit -c' for headless systems"
echo "(systems on which there is no X11 windowing system)"

exit 0
