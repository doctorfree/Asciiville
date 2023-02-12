Name: Asciiville
Version:    %{_version}
Release:    %{_release}
BuildArch:  noarch
Requires: util-linux, ncurses, dconf, libjpeg-turbo, libpng, khard, mplayer, git
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
