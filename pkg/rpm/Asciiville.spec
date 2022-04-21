Name: Asciiville
Version:    %{_version}
Release:    %{_release}
BuildArch:  x86_64
Requires: python3, python3-pip, tilix, xfce4-terminal, cool-retro-term, gnome-terminal, tmux, mplayer, lynx, mutt, ranger, jq, asciinema, speedtest-cli, caca-utils, neofetch, lolcat
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
