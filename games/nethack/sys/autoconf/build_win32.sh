#!/bin/bash -xe

# packages for Debian
# dpkg --add-architecture i386 && apt-get update
# sudo apt-get install gcc-mingw-w64-i686 wine wine32 wine-binfmt imagemagick
#
# Arch:
# mingw-w64-gcc wine imagemagick

INSTALL=/tmp/nethack_win32
DESTDIR=/tmp/nethack_destdir
mkdir -p $DESTDIR $INSTALL

function compile_nethack {
	env CFLAGS='-O2 -Wall -Wno-unused' ./configure \
		--host i686-w64-mingw32 \
		--prefix=$INSTALL \
		--with-owner="`id -un`" \
		--with-group="`id -gn`" \
		--build=i686-pc-mingw32 \
		--without-compression --disable-file-areas \
		--disable-status-color --enable-score-on-botl --enable-realtime-on-botl \
		$GRAPHICS \
		&& make --trace install
}

rm -rf $INSTALL/share/nethack $DESTDIR/nethack-win32-*

GRAPHICS="--disable-mswin-graphics --enable-tty-graphics"
compile_nethack
mv $INSTALL/share/nethack/nethack.exe $INSTALL/share/nethack/NetHack.exe

GRAPHICS="--enable-mswin-graphics --disable-tty-graphics"
compile_nethack
mv $INSTALL/share/nethack/nethack.exe $INSTALL/share/nethack/NetHackW.exe

rm -f $INSTALL/share/nethack/nethack.exe.old

make win32_release
make release_archive
