Name: Asciiville
Version:    %{_version}
Release:    %{_release}
BuildArch:  x86_64
Requires: python3, python3-pip, tilix, xfce4-terminal, cool-retro-term, gnome-terminal, tmux, mplayer, lynx, mutt, ranger, jq, asciinema, speedtest-cli, caca-utils, neofetch, figlet, ruby
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
  zip_inst=`type -p unzip`
  gem_inst=`type -p gem`
  if [ "${gem_inst}" ]
  then
    if [ "${wget_inst}" ]
    then
      if [ "${zip_inst}" ]
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

%preun

%files
/usr
%exclude %dir /usr/share/man/man1
%exclude %dir /usr/share/man
%exclude %dir /usr/share/doc
%exclude %dir /usr/share/menu
%exclude %dir /usr/share
%exclude %dir /usr/bin

%changelog
