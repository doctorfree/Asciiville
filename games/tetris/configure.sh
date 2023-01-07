#! /bin/sh

# I hope this script will save you the task of editing config.mk manually.
# It should work with most sh-compatible shells (perhaps not some very old).
# 

# You may comment out/uncomment any of these lines to change defaults.
PREFIX=/usr/local/games
ENABLE_2P=y		# 2-player
ENABLE_JS=y		# joystick
ENABLE_NETWORK=y
WITH_CURSES=y
#WITH_X=y
#WITH_ALLEGRO=y

ENABLE_TERM_RESIZING=y
ENABLE_MENU=y
ENABLE_BLOCKSTYLES=y

# DOS only
PCTIMER=y

# defaults are assigned later
BINDIR=
DATAROOTDIR=
DOCDIR=
PIXMAPDIR=z
DESKTOPDIR=z

default_prefix=$PREFIX
while [ $# -ge 1 ]; do
	arg=$1
	shift
	arg=${arg#-}
	arg=${arg#-}
	if [ "$arg" = h ] || [ "$arg" = help ]
  then
	echo 'Usage: ./configure.sh [OPTION]... [VAR=VALUE]...'
	echo "
Installation directories:
  --prefix=PREFIX	  install in PREFIX [$PREFIX]
  --bindir=DIR		  executable        [PREFIX/bin]
  --datarootdir=DIR	  data root	    [PREFIX/share]
  --docdir=DIR		  documentation     [DATAROOTDIR/doc/tetris]
  --pixmapdir=DIR	  tetris.xpm	    [DATAROOTDIR/pixmaps]
  --desktopdir=DIR	  tetris.desktop  [DATAROOTDIR/applications]

Build options:
  --disable-FEATURE	  do not include FEATURE (same as --enable-FEATURE=no)
  --enable-FEATURE[=ARG]  include FEATURE [ARG=yes]
  FEATURE=		  do not include FEATURE (same as FEATURE=no)
  FEATURE[=ARG]		  include FEATURE [ARG=yes]
  curses=LIB		  link with filename LIB
  curses=-lLIB		  link with library named LIB

--with- and --without-FEATURE can also be used.

Default features:"
	FEATURES=
	for feat in "2player=$ENABLE_2P" "joystick=$ENABLE_JS" \
		    "network=$ENABLE_NETWORK" "curses=$WITH_CURSES" \
		    "xlib=$WITH_X" "allegro=$WITH_ALLEGRO" nl \
		    "term_resizing=$ENABLE_TERM_RESIZING" "menu=$ENABLE_MENU" \
		    "blockstyles=$ENABLE_BLOCKSTYLES" \
		    " pctimer=$PCTIMER" nl
	do
		if [ "$feat" = nl ]; then
			echo "  $FEATURES"
			FEATURES=
		else
			if [ "$feat" != "${feat%=}" ]
			then feat="$feat"no
			else feat=${feat%=*}
			fi
			FEATURES="$FEATURES $feat"
		fi
	done
	echo
	echo These variables are used from the environment:
	echo '  CC CFLAGS CPPFLAGS LDFLAGS LIBS'
	echo
	echo See INSTALL for examples and more details.
	exit
  fi
	case "${arg%%=*}" in
	prefix) PREFIX=${arg#prefix=}; continue ;;
	bindir) BINDIR=${arg#bindir=}; continue ;;
	datarootdir)
		DATAROOTDIR=${arg#datarootdir=}; continue ;;
	docdir) DOCDIR=${arg#docdir=}; continue ;;
	pixmapdir)
		PIXMAPDIR=${arg#pixmapdir=}; continue ;;
	desktopdir)
		DESKTOPDIR=${arg#desktopdir=}; continue ;;
	CC)
		export CC=${arg#CC=}
		continue ;;
	CFLAGS)
		export CFLAGS=${arg#CFLAGS=}
		continue ;;
	CPPFLAGS)
		export CPPFLAGS=${arg#CPPFLAGS=}
		continue ;;
	LDFLAGS)
		LDFLAGS=${arg#LDFLAGS=}
		continue ;;
	LIBS)
		LIBS=${arg#LIBS=}
		continue
	esac

	arg=${arg%=yes}
	with=y
	if [ -n "${arg##*=}" ]; then
		with=${arg##*=}
		if [ "$with" = "${with#-l}" ] && [ "$with" = "${with%.*}" ]
		then with=y
		fi
	fi
	case "${arg%%-*}" in
	enable|disable|with|without)
		feat=${arg#*-} ;;
	*)
		feat=$arg
	esac
	if [ "$feat" = "$arg" ]; then
		feat=${feat%no}
		feat=${feat%n}
		if [ "$feat" != "${feat%=}" ]; then
			with=
		fi
		feat=${feat%=*}
	elif [ "$feat" != "${feat%=no}" ]; then
		feat=${feat%=no}
		case "${arg%%-*}" in
		enable|with)
			with=
		esac
	else
		case "${arg%%-*}" in
		disable|without)
			with=
		esac
	fi
	if [ "$feat" != "${feat%=*}" ]; then
		echo "unrecognized option \"$arg\""
		exit 1
	fi
	case "$feat" in
	2p*|twoplayer)
		ENABLE_2P=$with ;;
	js|joystick)
		ENABLE_JS=$with ;;
	net*)
		ENABLE_NETWORK=$with ;;
	curses|ncurses)
		WITH_CURSES=$with ;;
	x*)
		WITH_X=$with ;;
	alleg*)
		WITH_ALLEGRO=$with ;;
	term_resiz*)
		ENABLE_TERM_RESIZING=$with ;;
	menu)
		ENABLE_MENU=$with ;;
	blockstyle*)
		ENABLE_BLOCKSTYLES=$with ;;
	pctimer)
		PCTIMER=$with ;;
	*)
		echo "unrecognized option \"$arg\""
		exit 1
	esac
done
if [ -z "$ENABLE_2P" ]; then
	ENABLE_NETWORK=
fi
if [ -n "$WITH_ALLEGRO" ]; then
	WITH_CURSES=
	WITH_X=
	ENABLE_TERM_RESIZING=
fi

# change directory
arg="$0"
if [ "$arg" != "${arg#/}" ]; then
	dir=${arg%/*}
	echo cd $dir
	cd $dir
	arg=
fi
while [ "$arg" != "${arg#*/}" ]; do
	dir=${arg%%/*}
	if [ -n "$dir" -a "$dir" != . ]; then
		echo cd $dir
		cd $dir
	fi
	arg=${arg#*/}
done

# echo -n fix
if [ "x`echo -n`" = x-n ]; then
	n=; c='\c'
else
	n=-n; c=
fi
if [ -n "$ZSH_VERSION" ]; then
	emulate sh
fi


DPREFIX=/usr
if [ -n "$DJDIR" ]; then
	echo "DJDIR is set to $DJDIR"
	DPREFIX=$DJDIR
	OS=msdos
elif [ -n "$MINGDIR" ]; then
	echo "MINGDIR is set to $MINGDIR"
	DPREFIX=$MINGDIR
	CPPFLAGS="$CPPFLAGS -I$DPREFIX/include"
	LDFLAGS="$LDFLAGS -L$DPREFIX/lib"
	OS=win32
fi
if [ "$OS" = win32 ]; then
	LDLIBS=-lwinmm
	for flag in $CFLAGS x; do
		if [ x$flag = x-mno-cygwin ]; then
			if [ -z "$CC" ]; then CC=gcc; fi
			CC="$CC -mno-cygwin"
			CFLAGS="${CFLAGS%-mno-cygwin*} ${CFLAGS#*no-cygwin}"
		fi
	done
elif [ "$OS" != msdos ]; then
	OS=unix
fi
if [ $OS != unix ]; then
	if [ "$PREFIX" = "$default_prefix" ]; then
		PREFIX=c:/tetris
	fi
	WITH_X=
fi  

if [ -n "$PCTIMER" ] && [ $OS = msdos ]; then
	echo -n 'checking for pctimer... '
	PCTIMER=
	for dir in pctime14/ "" src/; do
		if [ -f ${dir}gccint8.c ]; then
			PCTIMER=${dir}gccint8
			echo $PCTIMER.c
			break
		fi
	done
	if [ -z "$PCTIMER" ]; then
		echo no
	fi
else
	PCTIMER=
fi

file_exists() {
	for file in $@; do
		if [ -f $file ]; then
			return 0
		fi
	done
	return 1
}

check_dynlib() {
	file_exists $1/lib$2.so* $1/lib$2.dll.a $1/lib$2.dylib
}

if [ $OS != msdos ]
then check_lib() {
	check_dynlib $1 $2 || test -f $1/lib$2.a
}
else check_lib() {
	test -f $1/lib$2.a
}
fi

LIB_SEARCH=
for flag in $LDFLAGS ignore-me; do
	if [ -d "${flag#-L}" ]; then
		LIB_SEARCH="${flag#-L} $LIB_SEARCH"
	fi
done
INC_SEARCH=
I=
for flag in $CPPFLAGS $CFLAGS ignore-me; do
	if [ x"$flag" = x-I ]; then
		I=1
	else
		path=
		if [ -n "$I" ]; then
			path="$flag"
		elif [ x"$flag" != x"${flag#-I}" ]; then
			path="${flag#-I}"
		fi
		I=
		if [ -d "$path" ]; then
			INC_SEARCH="$path $INC_SEARCH"
		fi
	fi
done

in_inc_search() {
	if [ $DPREFIX = /usr ] && [ $path = /usr/include ]; then
		return 0
	fi
	for path in $INC_SEARCH 1; do
		if [ $1 = $path ]; then return 0; fi
	done
	return 1
}

add_path_ldflags() {
	if [ $DPREFIX = /usr ] && [ ${1#/usr} = /lib ]; then
		return 1
	fi
	libpath=-L$1
	for flag in $LDFLAGS 1; do
		if [ x"$flag" = x$libpath ]; then
			libpath=
			break
		fi
	done
	if [ -n "$libpath" ]; then
		LDFLAGS="$LDFLAGS $libpath -Wl,-R${libpath#-L}"
	fi
}

if [ -n "$ENABLE_JS" ] && [ -z "$WITH_ALLEGRO" ]; then
	if [ "$OS" != unix ]; then
		echo joystick not supported
		ENABLE_JS=
	else
		echo $n "checking for linux/joystick.h... $c"
		linux_inc=
		for path in $INC_SEARCH /usr/include; do
			if [ -d $path/linux ]; then
				linux_inc=$path/linux
				break
			fi
		done
		if [ -n "$linux_inc" ] && [ -f $linux_inc/joystick.h ]; then
			echo yes
			echo $n "checking for linux/input.h... $c"
			if [ -f $linux_inc/input.h ]; then
				echo yes
			else
				echo no
				ENABLE_JS=
			fi
		else
			echo no
			ENABLE_JS=
		fi
	fi
fi

CHECK_CURSES=
if [ -n "$WITH_CURSES" ]; then
	CHECK_CURSES=y
	# don't check for curses if given curses=-lLIB or curses=LIB.
	if [ "x$WITH_CURSES" != "x${WITH_CURSES#-l}" ]; then
		CHECK_CURSES=
	elif [ "x$WITH_CURSES" != "x${WITH_CURSES%.*}" ] && \
	     [ -e "$WITH_CURSES" ]; then
		CHECK_CURSES=
	fi
fi
for lib in $LIBS x; do
	if [ x$lib != x${lib%curses*} ]; then
		WITH_CURSES=$lib
		CHECK_CURSES=
		LIBS="${LIBS%$lib*} ${LIBS#*$lib}"
	fi
done
libpath=
if [ -n "$CHECK_CURSES" ]; then
	echo $n "checking for curses... $c"
	WITH_CURSES=
	if [ $OS = unix ]; then
		search="/lib /usr/lib /usr/local/lib"
	else
		search=$DPREFIX/lib
	fi
	for path in $LIB_SEARCH $search; do
		for lib in ncurses curses pdcurses; do
			if check_lib $path $lib; then
				echo found $lib in $path
				WITH_CURSES=-l$lib
				break
			fi
		done
		if [ -n "$WITH_CURSES" ]; then
			libpath=$path
			break
		fi
	done
	if [ -z "$WITH_CURSES" ]; then
		echo no
	fi
fi
if [ -n "$WITH_CURSES" ]; then
	echo $n "checking for curses.h... $c"
	CURSES_INC=
	for path in $INC_SEARCH $DPREFIX/include /usr/local/include; do
		if [ -f $path/ncurses/curses.h ]; then
			path=$path/ncurses
		fi
		if [ -f $path/curses.h ]; then
			echo found in $path
			CURSES_INC=$path
			break
		fi
	done
	if [ -z "$CURSES_INC" ]; then
		echo no
		WITH_CURSES=
	else
		if in_inc_search $CURSES_INC; then
			CURSES_INC=
		else
			CURSES_INC=-I$CURSES_INC
		fi
		if [ -n "$libpath" ]; then add_path_ldflags $libpath ; fi
	fi
fi

if [ -n "$WITH_X" ]; then
	echo $n "checking for Xlib... $c"
	WITH_X=
	xpath='/usr /usr/X11R6 /usr/X11 /usr/local /usr/X11R7'
	search=$LIB_SEARCH
	for path in $xpath; do
		search="$search $path/lib"
	done
	libpath=
	for path in $search; do
		if check_dynlib $path X11; then
			echo found in $path
			WITH_X=-lX11
			if [ $path != /usr ]; then
				libpath=$path
			fi
			break
		fi
	done
	if [ -z "$WITH_X" ]; then
		echo no
	else
		echo $n "checking for X11/Xlib.h... $c"
		XLIB_INC=
		search=$INC_SEARCH
		for path in $xpath; do
			search="$search $path/include"
		done
		for path in $search; do
			if [ -f $path/X11/Xlib.h ]; then
				echo found in $path
				XLIB_INC=$path
				break
			fi
		done
		if [ -z "$XLIB_INC" ]; then
			echo no
			WITH_X=
		else
			if in_inc_search $XLIB_INC; then
				XLIB_INC=
			else
				XLIB_INC=-I$XLIB_INC
			fi
			add_path_ldflags $libpath
		fi
	fi
	xpath=
fi

if [ -n "$WITH_ALLEGRO" ] && [ "x$WITH_ALLEGRO" = "x${WITH_ALLEGRO#-l}" ]
then
	if [ $OS = unix ]; then
		WITH_ALLEGRO='`allegro-config --libs`'
	elif [ $OS = win32 ]; then
		echo -n 'checking for Allegro... '
		WITH_ALLEGRO=
		for path in $LIB_SEARCH $DPREFIX/lib; do
			for lib in alleg alleg_s; do
				if check_lib $path $lib; then
					echo -l$lib
					WITH_ALLEGRO=-l$lib
					break
				fi
			done
			if [ -n "$WITH_ALLEGRO" ]; then break ; fi
		done
		if [ -z "$WITH_ALLEGRO" ]; then
			echo not found!
			echo --- FAILED
			exit 1
		fi
	else
		WITH_ALLEGRO=-lalleg
	fi
fi

CONIO_H=
GETTEXTINFO=
echo $n "checking for conio.h... $c"
if [ -f $DPREFIX/include/conio.h ]; then
	echo yes
	CONIO_H=y
	echo $n "checking for gettextinfo... $c"
	if grep gettextinfo $DPREFIX/include/conio.h >/dev/null
	then echo yes ; GETTEXTINFO=y
	else echo no
	fi
else
	echo no
fi

STDINT_H=
INTTYPES_H=
SYS_TYPES_H=
UINT_LEAST32_T=
U_INT32_T=

echo $n "checking for stdint.h... $c"
if [ -f $DPREFIX/include/stdint.h ]; then
	echo yes
	STDINT_H=y
	UINT_LEAST32_T=y
else
	echo no
	echo $n "checking for inttypes.h... $c"
	if [ -f $DPREFIX/include/inttypes.h ]; then
		INTTYPES_H=y
		if grep uint_least32_t $DPREFIX/include/inttypes.h >/dev/null
		then echo yes ; UINT_LEAST32_T=y
		else echo no uint_least32_t
		fi
	else
		echo no
	fi
fi
echo $n "checking for sys/types.h... $c"
if [ -f $DPREFIX/include/sys/types.h ]; then
	SYS_TYPES_H=y
	if [ -n "$UINT_LEAST32_T" ]
	then echo yes
	elif grep u_int32_t $DPREFIX/include/sys/types.h >/dev/null
	then echo yes ; U_INT32_T=y
	else echo no u_int32_t
	fi
else
	echo no
fi

SYS_SELECT_H=
if [ $OS != win32 ]; then
	echo $n "checking for sys/select.h... $c"
	if [ -f $DPREFIX/include/sys/select.h ]
	then echo yes ; SYS_SELECT_H=y
	else echo no
	fi
fi

TTY_SOCKET=
if [ $OS = unix ] && [ -n "$ENABLE_NETWORK" ]; then
	echo $n "checking for sys/un.h... $c"
	if [ -f /usr/include/sys/un.h ]
	then echo yes ; TTY_SOCKET=y
	else echo no
	fi
fi

if [ $OS = win32 ] && [ -n "$ENABLE_NETWORK" ]; then
	echo -n 'checking for wsock32... '
	if check_lib $DPREFIX/lib wsock32
	then echo yes ; LDLIBS="-lwsock32 $LDLIBS"
	else echo no
	fi
fi

echo $n "updating src/config.h  $c"

echo '/* Generated by configure.sh */
#ifndef config_h
#define config_h
' > config_h.tmp

for def in CONIO_H-$CONIO_H  GETTEXTINFO-$GETTEXTINFO \
	   STDINT_H-$STDINT_H  INTTYPES_H-$INTTYPES_H \
	   SYS_TYPES_H-$SYS_TYPES_H  UINT_LEAST32_T-$UINT_LEAST32_T \
	   U_INT32_T-$U_INT32_T  SYS_SELECT_H-$SYS_SELECT_H; do
if [ -n "${def#*-}" ]
then echo "#define HAVE_${def%-*} 1" >> config_h.tmp
fi
done

if [ "x$WITH_ALLEGRO" != "x${WITH_ALLEGRO%_s}" ]; then
	echo >> config_h.tmp
	echo '#define ALLEGRO_STATICLINK' >> config_h.tmp
fi

echo '
#include "config2.h"

#endif' >> config_h.tmp

if cmp -s src/config.h config_h.tmp; then
	rm -f config_h.tmp
	echo '(no update needed, leaving unchanged)'
else
	echo
	mv -f config_h.tmp src/config.h
	if [ -f config_h.tmp ]; then
		echo --- ERROR! Could not update src/config.h
		exit 1
	fi
fi

# Write config.mk

echo $n "updating config.mk  $c"

echo '# config.mk.  Generated by configure.sh.' > configmk.tmp
echo '# This file is included in Makefile and src/Makefile.' >> configmk.tmp
echo >> configmk.tmp

# Default installation directories:
if [ $OS = unix ]; then
	if [ -z "$BINDIR" ]
	then BINDIR='$(prefix)/bin'
	fi
	if [ -z "$DATAROOTDIR" ]
	then DATAROOTDIR='$(prefix)/share'
	fi
	if [ -z "$DOCDIR" ]
	then DOCDIR='$(datarootdir)/doc/tetris'
	fi
	if [ z = "$PIXMAPDIR" ]
	then PIXMAPDIR='$(datarootdir)/pixmaps'
	fi
	if [ z = "$DESKTOPDIR" ]
	then DESKTOPDIR='$(datarootdir)/applications'
	fi
else
	if [ -z "$BINDIR" ]; then BINDIR='$(prefix)'; fi
	if [ -z "$DOCDIR" ]; then DOCDIR='$(prefix)'; fi
	PIXMAPDIR=
	DESKTOPDIR=
fi

echo prefix = $PREFIX >> configmk.tmp
echo bindir = $BINDIR >> configmk.tmp
echo datarootdir = $DATAROOTDIR >> configmk.tmp
echo "docdir     = $DOCDIR" >> configmk.tmp
echo "pixmapdir  = $PIXMAPDIR" >> configmk.tmp
echo "desktopdir = $DESKTOPDIR" >> configmk.tmp
if [ $OS = unix ]; then
	echo >> configmk.tmp
	echo 'datadir = $(datarootdir)/allegro' >> configmk.tmp
	echo '#datadir = $(datarootdir)/allegro/tetris' >> configmk.tmp
else
	echo 'datadir = $(prefix)' >> configmk.tmp
fi
echo >> configmk.tmp

if [ $OS = msdos ]; then
	echo SHELL = sh >> configmk.tmp
fi
if [ $OS = unix ]; then
	echo UNIX = y >> configmk.tmp
else
	echo EXE = .exe >> configmk.tmp
fi
if [ -n "$CC" ]; then
	echo "CC = $CC" >> configmk.tmp
fi
if [ -n "$CFLAGS" ]; then
	echo "CFLAGS = $CFLAGS" >> configmk.tmp
fi
if [ -n "$CPPFLAGS" ]; then
	echo "CPPFLAGS = $CPPFLAGS" >> configmk.tmp
fi
echo >> configmk.tmp

comment_if_empty() {
	if [ -z "$1" ]; then
		echo $n "#$c" >> configmk.tmp
	fi
}

comment_if_empty $ENABLE_2P
echo TWOPLAYER = y >> configmk.tmp
comment_if_empty $ENABLE_JS
echo JOYSTICK = y >> configmk.tmp
comment_if_empty $ENABLE_NETWORK
echo NETWORK = y >> configmk.tmp
comment_if_empty $TTY_SOCKET
echo TTY_SOCKET = y >> configmk.tmp
comment_if_empty $ENABLE_TERM_RESIZING
echo TERM_RESIZING = y >> configmk.tmp
comment_if_empty $ENABLE_MENU
echo MENU = y >> configmk.tmp
comment_if_empty $ENABLE_BLOCKSTYLES
echo BLOCKSTYLES = y >> configmk.tmp
echo >> configmk.tmp

if [ $OS = unix ]; then
	if [ -z "${WITH_CURSES}${WITH_ALLEGRO}" ]
	then echo INPUT_SYS = unixterm >> configmk.tmp
	else echo '#INPUT_SYS = unixterm' >> configmk.tmp
	fi
elif [ -n "$CONIO_H" ]; then
	if [ -z "${WITH_CURSES}${WITH_ALLEGRO}" ]
	then echo INPUT_SYS = conio >> configmk.tmp
	else echo '#INPUT_SYS = conio' >> configmk.tmp
	fi
fi

comment_if_empty $WITH_CURSES
echo CURSES = y >> configmk.tmp
if [ -n "$WITH_CURSES" ]; then
	echo CURSES_INC = $CURSES_INC >> configmk.tmp
fi

comment_if_empty "$WITH_ALLEGRO"
echo ALLEGRO = y >> configmk.tmp

comment_if_empty $WITH_X
echo XLIB = y >> configmk.tmp
if [ -n "$WITH_X" ]
then echo XLIB_INC = $XLIB_INC >> configmk.tmp
else echo '#XLIB_INC =' >> configmk.tmp
fi

echo LDFLAGS = $LDFLAGS >> configmk.tmp
if [ "x$WITH_ALLEGRO" != "x${WITH_ALLEGRO%_s}" ]
then
	echo 'LDLIBS  = -lalleg_s -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lole32 -ldinput -lddraw -ldxguid -ldsound'	   >> configmk.tmp
	echo 'LDLIBS +=' $LDLIBS '$(LIBS)' >> configmk.tmp
else
echo 'LDLIBS  =' "$WITH_ALLEGRO" $LDLIBS $WITH_CURSES $WITH_X '$(LIBS)' \
>> configmk.tmp
fi
if [ -n "$LIBS" ]; then
	echo LIBS = $LIBS >> configmk.tmp
fi
echo '
# DOS millisecond granularity' >> configmk.tmp
if [ -n "$PCTIMER" ]; then
	echo PCTIMER = $PCTIMER >> configmk.tmp
	if [ $PCTIMER = ${PCTIMER#*/} ]; then
		echo PCTIMER_INC = -I.. >> configmk.tmp
	elif [ $PCTIMER = ${PCTIMER#src/} ]; then
		echo PCTIMER_INC = -I../${PCTIMER%/*} >> configmk.tmp
	fi
else
	echo '#PCTIMER = pctime14/gccint8' >> configmk.tmp
	echo '#PCTIMER_INC = -I../pctime14' >> configmk.tmp
fi
echo >> configmk.tmp

if cmp -s config.mk configmk.tmp; then
	rm -f configmk.tmp
	echo '(no update needed, leaving unchanged)'
else
	echo
	mv -f configmk.tmp config.mk
	if [ -f configmk.tmp ]; then
		echo --- ERROR! Could not update config.mk
		rm -f configmk.tmp
		exit 1
	fi
fi

echo
echo INSTALLATION DIRECTORIES:
echo "  prefix = $PREFIX"
echo "  bindir = $BINDIR"
echo "  datarootdir = $DATAROOTDIR"
echo "  docdir     = $DOCDIR"
echo "  pixmapdir  = $PIXMAPDIR"
echo "  desktopdir = $DESKTOPDIR"
echo
echo FEATURES:
echo $n "  $c"
for feat in "2player=$ENABLE_2P" "joystick=$ENABLE_JS" \
	    "network=$ENABLE_NETWORK" "curses=$WITH_CURSES" \
	    "xlib=$WITH_X" "allegro=$WITH_ALLEGRO"  \
	    "term_resizing=$ENABLE_TERM_RESIZING" "menu=$ENABLE_MENU" \
	    "blockstyles=$ENABLE_BLOCKSTYLES" pctimer=$PCTIMER
do
	if [ "$feat" = "${feat%=}" ]
	then echo $n "${feat%=*} $c"
	fi
done

echo
echo
echo '(If you wish to change anything, run "./configure.sh help" for options.)'
echo Now run make
