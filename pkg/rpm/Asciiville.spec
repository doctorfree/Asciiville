Name: Asciiville
Version:    %{_version}
Release:    %{_release}
BuildArch:  x86_64
Requires: util-linux, python3, python3-pip, tmux, mplayer, w3m, ranger, curl, jq, wget, asciinema, speedtest-cli, ncurses, neofetch, newsboat, figlet, ImageMagick, dconf, libjpeg-turbo, libpng, cmatrix, gnupg, pandoc, urlscan, khard, git
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

FIGLET_DIR="/usr/local/share/figlet-fonts"
FIGLET_ZIP="figlet-fonts.zip"
zip_inst=`type -p zip`
if [ "${zip_inst}" ]
then
  pyfig_inst=`type -p pyfiglet`
  [ "${pyfig_inst}" ] || {
    ${PYTHON} -m pip install pyfiglet
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

%preun
if [ -f /etc/profile.d/asciiville.sh ]
then
  rm -f /etc/profile.d/asciiville.sh
fi

%files
%defattr(-,root,root)
%attr(4755, games, games) /usr/local/games/bin/*
%attr(0664, games, games) /usr/local/games/var/tetris-hiscores
%attr(0755, games, games) /usr/local/games/var/nethack/bones
%attr(0755, games, games) /usr/local/games/var/nethack/level
%attr(0755, games, games) /usr/local/games/var/nethack/saves
%attr(0644, games, games) /usr/local/games/var/nethack/logfile
%attr(0644, games, games) /usr/local/games/var/nethack/perm
%attr(0644, games, games) /usr/local/games/var/nethack/record
%attr(0644, games, games) /usr/local/games/var/nethack/xlogfile
%exclude %dir /usr/local/share/man/man1
%exclude %dir /usr/local/share/man/man6
%exclude %dir /usr/local/share/man
%exclude %dir /usr/local/share/doc
%exclude %dir /usr/local/share/menu
%exclude %dir /usr/local/share
%exclude %dir /usr/local/bin
%exclude %dir /usr/local/games
%exclude %dir /usr/local/games/bin
%exclude %dir /usr/local/games/lib
%exclude %dir /usr/local/games/share
%exclude %dir /usr/local/games/var
/usr/local/bin/*
/usr/local/share/*
/usr/local/games/gameserver
/usr/local/games/nethack
/usr/local/games/tetris
/usr/local/games/share/*

%changelog
