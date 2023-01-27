Name: Asciiville
Version:    %{_version}
Release:    %{_release}
BuildArch:  noarch
Requires: util-linux, python3, python3-pip, tmux, mplayer, w3m, ranger, curl, jq, wget, asciinema, speedtest-cli, ncurses, neofetch, newsboat, figlet, ImageMagick, dconf, libjpeg-turbo, libpng, cmatrix, gnupg, urlscan, khard, git
URL:        https://github.com/doctorfree/Asciiville
Vendor:     Doctorwhen's Bodacious Laboratory
Packager:   ronaldrecord@gmail.com
License     : MIT
Summary     : ASCII Art and Utilities

%global __os_install_post %{nil}

%description

%prep

%build

%install
cp -a %{_sourcedir}/usr %{buildroot}/usr

%pre

%post
export PATH=${PATH}:/usr/local/bin
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
      unzip_inst=`type -p unzip`
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
  ${PYTHON} -m pip install setuptools 2> /dev/null
  pyfig_inst=`type -p pyfiglet`
  [ "${pyfig_inst}" ] || {
    ${PYTHON} -m pip install pyfiglet 2> /dev/null
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

%preun
if [ -f /etc/profile.d/asciiville.sh ]
then
  rm -f /etc/profile.d/asciiville.sh
fi

%files
%defattr(-,root,root)
%exclude %dir /usr/share/man/man1
%exclude %dir /usr/share/man
%exclude %dir /usr/share/doc
%exclude %dir /usr/share/menu
%exclude %dir /usr/share
%exclude %dir /usr/bin
/usr/bin/*
/usr/share/*

%changelog
