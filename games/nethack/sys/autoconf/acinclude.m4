# $Id: acinclude.m4,v 1.8 2007/01/06 16:00:55 j_ali Exp $

# Front-end of AC_ARG_ENABLE
# Usage:
#	NETHACK_ARG([feature], [description], [pre-processor switch], [default],
#	  [action-if-auto])
# Note that the action-if-auto argument is only used if the default is auto
# (and the user doesn't specify). The action should set the shell variable
# enable_feature, with any '-' characters changed into '_'.
AC_DEFUN([NETHACK_ARG],
  [AC_MSG_CHECKING(whether to enable $1)
   AC_ARG_ENABLE([$1], AC_HELP_STRING([--enable-$1],[$2 (default=$4)]),
     [],m4_if([$4], [auto], [$5], [enable_]m4_bpatsubst([$1], -, _)="[$4]"))
   AC_MSG_RESULT([$enable_]m4_bpatsubst([$1], -, _))
   if test "[$enable_]m4_bpatsubst([$1], -, _)" != "no" ; then
      AC_DEFINE_UNQUOTED([$3], [], [$2])
   fi
  ])

# NETHACK_ARG_WITH_PARAM([feature], [description], [pre-processor switch],
#                        [default], [type], [default-value])
# ------------------------------------------------------------------------
AC_DEFUN([NETHACK_ARG_WITH_PARAM],
  [AC_MSG_CHECKING([whether to enable m4_bpatsubst([$1], -, [ ])])
   AC_ARG_ENABLE([$1], AC_HELP_STRING([--enable-$1=$5],
        [$2 (default=$4, default m4_tolower($5)=[$6])]),
        if test "[$enableval]" != no; then [enable_]m4_bpatsubst([$1], -, _)=yes;
         if test "[$enableval]" != yes; then m4_bpatsubst([$1], -, _)=[$enableval];
         else m4_bpatsubst([$1], -, _)=[$6]; fi;
        else [enable_]m4_bpatsubst([$1], -, _)=no; fi,
        [enable_]m4_bpatsubst([$1], -, _)=$4;m4_bpatsubst([$1], -, _)=[$6])
   if test "[$enable_]m4_bpatsubst([$1], -, _)" != "no" ; then
      AC_MSG_RESULT([yes (m4_tolower($5) [$]m4_bpatsubst([$1], -, _))])
      AC_DEFINE_UNQUOTED([$3],
       m4_case([NUMBER], $5, [$]m4_bpatsubst([$1], -, _), dnl NUMBER without quotes
               "[$]m4_bpatsubst([$1], -, _)"),
       [$2])
      AC_SUBST([$3],["[$]m4_bpatsubst([$1], -, _)"])
   else
      AC_MSG_RESULT([no])
   fi
  ])

# NETHACK_WIN_ENABLE(win, default, help-text)
# -------------------------------------------
AC_DEFUN([NETHACK_WIN_ENABLE],[AC_ARG_ENABLE($1-graphics,
  [AC_HELP_STRING([--enable-$1-graphics],
    m4_ifval([$3],[$3],[use $1-graphics]) [(default=$2)])],
  [enable_$1_graphics="$enableval"], [enable_$1_graphics="$2"])
  AC_MSG_CHECKING([$1-graphics])
  if test "$enable_$1_graphics" != "no"; then
    nethack_wins="$1 $nethack_wins"
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi
  ])

# NETHACK_AUTO_WIN_DISABLE(win)
# -----------------------------
# Disable a windowing system previously set to auto.
AC_DEFUN([NETHACK_AUTO_WIN_DISABLE],[enable_$1_graphics="no"
    nethack_wins=`echo $nethack_wins | sed 's/$1 //'`
])

# NETHACK__LINK_ADD(DEST, [SOURCE])
# ---------------------------------
# Update if there is a newer source
AC_DEFUN([NETHACK__LINK_ADD],
	[m4_ifval([$2],[if test ! -f $1 -o ${srcdir}/$2 -nt $1 ; then
	   NETHACK_LINKS="${NETHACK_LINKS} $1:$2"
	fi],[NETHACK__LINK_ADD([$1],[$1])])])
AC_DEFUN([NETHACK_LINKS_ADD],
	[AC_FOREACH([NETHACK_File],[$1],
	    [NETHACK__LINK_ADD(m4_bpatsubst(NETHACK_File,[:],[,]))])])

# Autoconf's standard AC_CONFIG_HEADERS() avoids undefining switches because
# of concern that it might undefine system switches (eg., POSIX_SOURCE).
# However, we need to override the settings in config.h etc., so we post-process
# the generated headers and force an explicit define and/or undefine.
AC_DEFUN([NETHACK_CONFIG_HEADER],
    [AC_CONFIG_HEADERS([$1],
      [sed -e 's,^/\* \(#undef .*\) \*/,\1,' \
        -e '/#define [^ ]*/ { h; s,#define \([^ ]*\).*,#undef \1,p; g; }'] \
        < m4_bpatsubst([$1], [:.*]) > m4_bpatsubst([$1], [[^.]*:.*], [new])
      [mv] m4_bpatsubst([$1], [[^.]*:.*], [new]) \
        m4_bpatsubst([$1], [:.*]),[])])

# NETHACK__ADD_FLAG(output-variable, existing-flags, flag)
# ---------------------------------
# Add a compiler flag to a variable. A number of flags are recognized and
# handled specially (specifically, -I, -L and -l).
AC_DEFUN([NETHACK__ADD_FLAG],[[
    case "$3" in
	-I*|-L*|-l*)
	    for nethack__eflag in $2; do
		if test "$nethack__eflag" = "$3"; then
		    break
		fi
	    done
	    if test "$nethack__eflag" = "$3"; then
		$1="$2"
	    else
		case "$3" in
		    -L*)$1="$3 $2";;
		    *)  $1="$2 $3";;
		esac
	    fi
	    ;;
	*)  $1="$2 $3";;
    esac
]])

# NETHACK__ADD_FLAGS(output-variable, existing-flags, new-flags)
# ---------------------------------
# Front-end to NETHACK__ADD_FLAG. Add compiler flags to a variable.
AC_DEFUN([NETHACK__ADD_FLAGS],[
    $1="$2"
    for nethack__flag in $3; do
	NETHACK__ADD_FLAG([$1], [${$1}], [$nethack__flag])
    done
])

# NETHACK__TRY_LINK(STUFF, library, cflags, libs, includes, body, action-if,
#	action-if-not)
# ---------------------------------
# Try linking using the given cflags and libs.
AC_DEFUN([NETHACK__TRY_LINK],[
    nethack__save_cflags=$CFLAGS
    nethack__save_libs=$LIBS
    CFLAGS="$CFLAGS $3"
    LIBS="$LIBS $4"
    AC_MSG_CHECKING([if $2 is useable with $CFLAGS and $LIBS])
    AC_TRY_LINK([$5], [$6],
      [$1_CFLAGS="$3"; $1_LIBS="$4"; AC_MSG_RESULT([yes]); $7],
      [AC_MSG_RESULT([no]); $8])
    CFLAGS=$nethack__save_cflags
    LIBS=$nethack__save_libs])

# NETHACK__TRY_LIBXAW(STUFF, cflags, libs, action-if, action-if-not)
# ---------------------------------
# Try linking with libXaw using the given cflags and libs.
AC_DEFUN([NETHACK__TRY_LIBXAW], [NETHACK__TRY_LINK([$1], [libXaw], [$2], [$3],
    [#include <X11/Xaw/List.h>], [XawListUnhighlight((Widget)0);], [$4], [$5])])

# NETHACK_CHECK_LIBXAW(stuff, action-if, action-if-not)
# ---------------------------------
# Check if libXaw can be used
AC_DEFUN([NETHACK_CHECK_LIBXAW],[
    AC_ARG_VAR([LIBXAW_CFLAGS], [Compiler flags for libXaw, overriding pkg-config])
    AC_ARG_VAR([LIBXAW_LIBS], [Linker flags for libXaw, overriding pkg-config])
    # If the user has supplied flags, then believe them.
    if test -n "$LIBXAW_CFLAGS" -o -n "$LIBXAW_LIBS"; then
	NETHACK__ADD_FLAGS(nethack__cflags, [$NETHACK__LIBXAW_CFLAGS],
	  [$LIBXAW_CFLAGS])
	NETHACK__ADD_FLAGS(nethack__libs, [$NETHACK__LIBXAW_LIBS],
	  [$LIBXAW_LIBS])
	NETHACK__TRY_LIBXAW($1, [$nethack__cflags], [$nethack__libs],
	  [nethack__succeeded=yes], [nethack__succeeded=no])
    else
	nethack__succeeded=maybe
    fi
    if test $nethack__succeeded = maybe; then
	PKG_CHECK_EXISTS(xaw8, [nethack__succeeded=yes], [])
    fi
    if test $nethack__succeeded = maybe; then
	PKG_CHECK_EXISTS(xaw7, [nethack__succeeded=yes], [])
    fi
    if test $nethack__succeeded = maybe; then
	PKG_CHECK_EXISTS(xaw6, [nethack__succeeded=yes], [])
    fi
    if test $nethack__succeeded = yes; then
	NETHACK__ADD_FLAGS(nethack__cflags, [$NETHACK__LIBXAW_CFLAGS],
	  [-I/usr/X11R6/include])
	NETHACK__ADD_FLAGS(nethack__libs, [$NETHACK__LIBXAW_LIBS],
	  [-L/usr/X11R6/lib -lXaw -lXmu -lXext -lXt -lX11])
	NETHACK__TRY_LIBXAW($1, [$nethack__cflags], [$nethack__libs],
	  [nethack__succeeded=yes], [nethack__succeeded=no])
    fi
    if test $nethack__succeeded = yes; then
	AC_MSG_CHECKING([$1_CFLAGS])
	AC_MSG_RESULT([${$1_CFLAGS}])
	AC_SUBST($1_CFLAGS)
	AC_MSG_CHECKING([$1_LIBS])
	AC_MSG_RESULT([${$1_LIBS}])
	AC_SUBST($1_LIBS)
	$2
    m4_ifval([$3],[else $3],[])
    fi
])

# NETHACK__TRY_LIBPNG(STUFF, cflags, libs, action-if, action-if-not)
# ---------------------------------
# Try linking with libpng using the given cflags and libs.
AC_DEFUN([NETHACK__TRY_LIBPNG], [NETHACK__TRY_LINK([$1], [libpng], [$2], [$3],
    [#include <png.h>], [png_structp ptr=
      png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);],
    [$4], [$5])])

# NETHACK_CHECK_LIBPNG(stuff, action-if, action-if-not)
# ---------------------------------
# Check if libpng can be used
AC_DEFUN([NETHACK_CHECK_LIBPNG],[
    # Slash'EM may well work with earlier versions,
    # but 1.2.4 is the earliest tested to date.
    PKG_CHECK_MODULES(NETHACK__LIBPNG, libpng >= 1.2.4,
      [nethack__succeeded=maybe], [nethack__succeeded=no])
    # Slash'EM doesn't use zlib directly, but libpng does. This should
    # all be handled in libpng.pc, but isn't in many versions of libpng.
    if test $nethack__succeeded = maybe; then
	NETHACK__ADD_FLAGS(nethack__libs, [$NETHACK__LIBPNG_LIBS], [-lz])
	NETHACK__TRY_LIBPNG($1, $NETHACK__LIBPNG_CFLAGS, $nethack__libs,
	  [nethack__succeeded=yes], [])
    fi
    # If the user has supplied flags, then believe them.
    if test $nethack__succeeded = maybe -a \
      \( -n "$ZLIB_CFLAGS" -o -n "$ZLIB_LIBS" \); then
	NETHACK__ADD_FLAGS(nethack__cflags, [$NETHACK__LIBPNG_CFLAGS],
	  [$ZLIB_CFLAGS])
	NETHACK__ADD_FLAGS(nethack__libs, [$NETHACK__LIBPNG_LIBS],
	  [$ZLIB_LIBS])
	NETHACK__TRY_LIBPNG($1, [$nethack__cflags], [$nethack__libs],
	  [nethack__succeeded=yes], [])
    fi
    # Otherwise, try pkg-config.
    if test $nethack__succeeded = maybe; then
	AC_MSG_CHECKING([for zlib])
	if $PKG_CONFIG --exists zlib; then
	    AC_MSG_RESULT([yes])
	    NETHACK__ADD_FLAGS(nethack__cflags, [$NETHACK__LIBPNG_CFLAGS],
	      [`$PKG_CONFIG --cflags zlib`])
	    NETHACK__ADD_FLAGS(nethack__libs, [$NETHACK__LIBPNG_LIBS],
	      [`$PKG_CONFIG --libs zlib`])
	    NETHACK__TRY_LIBPNG($1, [$nethack__cflags], [$nethack__libs],
	      [nethack__succeeded=yes], [])
	else
	    AC_MSG_RESULT([not found by pkg-config])
	fi
    fi
    # Try the same prefix as libpng since zlib is
    # likely to be installed in the same place.
    if test $nethack__succeeded = maybe; then
        nethack__libpng_prefix=`$PKG_CONFIG --variable=prefix libpng`
	NETHACK__ADD_FLAG(nethack__cflags, [$NETHACK__LIBPNG_CFLAGS],
	  [-I$nethack__libpng_prefix/include])
	NETHACK__ADD_FLAG(nethack__libs, [$NETHACK__LIBPNG_LIBS],
	  [-L$nethack__libpng_prefix/lib])
	NETHACK__ADD_FLAG(nethack__libs, [$nethack__libs], [-lz])
        NETHACK__TRY_LIBPNG($1, [$nethack__cflags], [$nethack__libs],
          [nethack__succeeded=yes], [])
    fi
    if test $nethack__succeeded = yes; then
	AC_MSG_CHECKING([$1_CFLAGS])
	AC_MSG_RESULT([${$1_CFLAGS}])
	AC_SUBST($1_CFLAGS)
	AC_MSG_CHECKING([$1_LIBS])
	AC_MSG_RESULT([${$1_LIBS}])
	AC_SUBST($1_LIBS)
	$2
    m4_ifval([$3],[else $3],[])
    fi
])

# NETHACK_COMMAND_IFELSE(command, action-if, action-if-not)
# ---------------------------------
# If command returns an exit status of 0 when executed, run shell commands
# action-if. Otherwise, run shell commands action-if-not.
AC_DEFUN([NETHACK_COMMAND_IFELSE], [
    AS_IF([AC_TRY_COMMAND([$1])], [$2],
    [echo "$as_me: program exited with status $ac_status" >&AS_MESSAGE_LOG_FD
    m4_ifvaln([$3], [( exit $ac_status )
	    $3])])])

# NETHACK_COMPRESS_EXT
# ---------------------------------
# Check the extension used for compressed files by the compressor.
AC_DEFUN([NETHACK_COMPRESS_EXT], [
    AC_MSG_CHECKING([for suffix of compressed files])
    rm -f conftest conftest.*
    echo "Test" > conftest
    nethack__extension=""
    NETHACK_COMMAND_IFELSE([$COMPRESS $COMPRESS_OPTIONS conftest], [
	for nethack__file in \
	  `(ls conftest.bz2 conftest.gz conftest.Z conftest.*) 2>/dev/null`; do
	    case $nethack__file in
		*)  nethack__extension=`expr "$nethack__file" : '.*\(\..*\)'`
		    break
		    ;;
	    esac
	done
	if test -z "$nethack__extension"; then
	    AC_MSG_FAILURE([test failed: cannot find compressed output])
	fi
	rm -f conftest conftest.*
	AC_DEFINE_UNQUOTED(COMPRESS_EXTENSION, ["$nethack__extension"],
	  [Extension of compressed files])
	AC_MSG_RESULT([$nethack__extension])
	], [AC_MSG_FAILURE([test failed: cannot compress])])])

# NETHACK_PROG_COMPRESS
# ---------------------------------
# Configure the compress program.
AC_DEFUN([NETHACK_PROG_COMPRESS], [
    AC_ARG_WITH([compression], [AS_HELP_STRING([--with-compression=PROGRAM],
	    [Set the program to compress data files (default=auto)])],
	    [], [with_compression=auto])
    case $with_compression in
	auto)
	    AC_PATH_PROGS(COMPRESS, [bzip2 gzip compress], [:])
	    ;;
	no)
	    COMPRESS=":"
	    ;;
	*)
	    set dummy $with_compression
	    AC_PATH_PROG(COMPRESS, [$[2]], [:])
	    ;;
    esac
    AC_MSG_CHECKING([for compression support])
    if test "$with_compression" = "auto" ; then
	AC_DEFINE_UNQUOTED(COMPRESS, ["$COMPRESS"], [Compression program])
	COMPRESS_OPTIONS=""
	if test "$COMPRESS" = ":"; then
	    AC_DEFINE(INTERNAL_COMP, [1], [Use UnNetHack's compression routines])
	fi
    elif test "$with_compression" = "no" ; then
	COMPRESS=":"
	AC_DEFINE(INTERNAL_COMP, [1], [Use UnNetHack's compression routines])
    else
	AC_DEFINE_UNQUOTED(COMPRESS, ["$COMPRESS"], [Compression program])
	set dummy $with_compression
	shift
	shift
	if test -n "$[1]"; then
	    AC_DEFINE_UNQUOTED(COMPRESS_OPTIONS, ["$[@]"],
	      [Options to pass to the compression program, if any])
	fi
	COMPRESS_OPTIONS="$[@]"
    fi
    if test "$COMPRESS" = ":"; then
	AC_MSG_RESULT([internal])
    else
	AC_MSG_RESULT([$COMPRESS $COMPRESS_OPTIONS])
	NETHACK_COMPRESS_EXT
    fi])
