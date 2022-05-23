Name: Asciiville
Version:    %{_version}
Release:    %{_release}
BuildArch:  x86_64
Requires: util-linux, python3, python3-pip, tmux, mplayer, w3m, ranger, jq, asciinema, speedtest-cli, ncurses, neofetch, newsboat, figlet, ImageMagick, dconf, libjpeg-turbo, libpng, cmatrix, gnupg, pandoc, urlscan, khard
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

%preun

%files
%defattr(-,root,root)
%attr(4755, games, games) /usr/games/bin/*
%attr(0644, games, games) /usr/games/lib/ninvaders/*
%attr(0664, games, games) /usr/games/var/tetris-hiscores
%attr(0755, games, games) /usr/games/var/nethack/bones
%attr(0755, games, games) /usr/games/var/nethack/level
%attr(0755, games, games) /usr/games/var/nethack/saves
%attr(0644, games, games) /usr/games/var/nethack/logfile
%attr(0644, games, games) /usr/games/var/nethack/perm
%attr(0644, games, games) /usr/games/var/nethack/record
%attr(0644, games, games) /usr/games/var/nethack/xlogfile
%exclude %dir /usr/share/man/man1
%exclude %dir /usr/share/man/man6
%exclude %dir /usr/share/man
%exclude %dir /usr/share/doc
%exclude %dir /usr/share/menu
%exclude %dir /usr/share
%exclude %dir /usr/bin
%exclude %dir /usr/games
%exclude %dir /usr/games/bin
%exclude %dir /usr/games/lib
%exclude %dir /usr/games/share
%exclude %dir /usr/games/var
/usr/bin/*
/usr/share/*
/usr/games/gameserver
/usr/games/nethack
/usr/games/ninvaders
/usr/games/tetris
/usr/games/share/*

%changelog
