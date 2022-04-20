Name: Asciiville
Version:    %{_version}
Release:    %{_release}
BuildArch:  x86_64
Requires: python3, python3-pip, tilix, xfce4-terminal, cool-retro-term, gnome-terminal, tmux, mplayer, lynx, neomutt, ranger, jq, asciinema, speedtest-cli, caca-utils, libaa-bin, libaa1, bb, aview, neofetch, lolcat
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
python3 -m pip install asciimatics
python3 -m pip install rainbowstream

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
