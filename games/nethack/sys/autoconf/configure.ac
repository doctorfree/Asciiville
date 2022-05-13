dnl	NETHACK autoconf template
AC_INIT([NETHACK],[6.0.3],[bhaak@gmx.net])

AC_CONFIG_SRCDIR(README.configure)
AC_SUBST(SRCDIR,[../$srcdir/src])
AC_CONFIG_AUX_DIR(sys/autoconf)

dnl  ----- Precious variables
AC_ARG_VAR(CC, [C compiler])
AC_ARG_VAR(CHGRP, [sets GAMEUID (set this to true as a no op)])
AC_ARG_VAR(CHOWN, [sets GAMEGRP (set this to true as a no op)])
AC_ARG_VAR(CHMOD, [sets file modes (set this to true as a no op)])
AC_ARG_VAR(ZLIB_CFLAGS, [Compiler flags to find zlib header files])
AC_ARG_VAR(ZLIB_LIBS, [Linker flags to find zlib library files])

AC_PROG_LN_S
AC_PROG_AWK

dnl  ----- Check options
dnl  ----- Window systems

# NETHACK_WIN_ENABLE(proxy, auto)
nethack_default_win=""
# Note: Order is important: The last listed enabled port will be the default.
NETHACK_WIN_ENABLE(dummy, no, [dummy graphics for unit tests])
NETHACK_WIN_ENABLE(lisp, no, [enable playing NetHack in Emacs])
NETHACK_WIN_ENABLE(curses, no, [use curses interface])
NETHACK_WIN_ENABLE(tty, auto)
NETHACK_WIN_ENABLE(x11, no)
NETHACK_WIN_ENABLE(mswin, no, [use Microsoft Window's native graphics])
dnl deactivate X11 and Win32 tiles version
dnl enable_x11_graphics="no"

dnl  ----- Figure out a C compiler to use; set @CC@
AC_PROG_CC
AC_SUBST(LFLAGS)

dnl  ----- Check OS
AC_CANONICAL_HOST

dnl  ----- Early checks for libraries, as needed

dnl PKG_PROG_PKG_CONFIG([])
if test "$enable_x11_graphics" != "no"; then
    # We have to check for libXaw before we define SYSV,
    # since the X11 include files change their behaviour
    # depending on whether the compiler defines it or not.
    NETHACK_CHECK_LIBXAW([LIBXAW], have_libXaw=yes, have_libXaw=no)
fi

dnl  ------ Main OS types
case $host_os in
     mingw32 )		AC_DEFINE(WIN32,[],[An MS-Windows system])
			AC_SUBST(HOST_MSWIN,[])
			AC_SUBST(HOST_UNIX,[#])
			SYSTEM="Syswinnt"
			SYSSRC="\${SYSMSWINSRC}"
			SYSOBJ="\${SYSMSWINOBJ}"
			;;
     *bsd* )		AC_DEFINE(BSD,[],[A BSD like system])
			AC_SUBST(HOST_MSWIN,[#])
			AC_SUBST(HOST_UNIX,[])
			SYSTEM="Sysunix"
			SYSSRC="\${SYSUNIXSRC}"
			SYSOBJ="\${SYSUNIXOBJ}"
			;;
     * )		AC_DEFINE(SYSV,[],[A System V like system])
			AC_SUBST(HOST_MSWIN,[#])
			AC_SUBST(HOST_UNIX,[])
			SYSTEM="Sysunix"
			SYSSRC="\${SYSUNIXSRC}"
			SYSOBJ="\${SYSUNIXOBJ}"
			;;
esac
AC_SUBST(SYSTEM)
AC_SUBST(SYSSRC)
AC_SUBST(SYSOBJ)

if test $host_os = mingw32; then
    AC_MSG_CHECKING([for how to build NetHack key dynamic libraries])
    if test "$GCC" = yes; then
	AC_MSG_RESULT([gcc])
	AC_SUBST(NHKEYDLL_GCC,[])
    else
	AC_MSG_RESULT([unknown])
	if test "$enable_tty_graphics" = yes; then
	    AC_MSG_ERROR([Don't know how to build NetHack key DLLs with $CC])
	else
	    AC_MSG_WARN([Can't enable tty graphics (key dynamic libraries)])
	    NETHACK_AUTO_WIN_DISABLE(tty)
	fi
    fi
fi


dnl ------ Common subtypes
case $host_os in
     *cygwin* )		AC_DEFINE(CYGWIN32,[],[Unix on win32]);;
     *linux* )		AC_DEFINE(LINUX,[],[GNU Linux]);;
esac

dnl  ----- Check for programs
AC_PROG_LEX(noyywrap)
if test "$LEX" = ":"; then
	AC_MSG_ERROR([cannot find flex or lex])
fi
AC_CHECK_PROGS(YACC, ['bison -y' byacc yacc])
if test -z "$YACC"; then
	AC_MSG_ERROR([cannot find bison, byacc or yacc])
fi
AC_CHECK_TOOLS(CHOWN,[chown true])
AC_CHECK_TOOLS(CHGRP,[chgrp true])
AC_CHECK_TOOLS(CHMOD,[chmod true])
AC_CHECK_TOOL(WINDRES,windres,no)
NETHACK_PROG_COMPRESS

dnl  ----- Check for libraries

dnl NETHACK_CHECK_LIBPNG(NETHACK_LIBPNG, have_libpng=yes, have_libpng=no)

dnl  ----- Select window systems

WINSRC=
WINOBJ=
WINLIB=
CNF_SHARE_DATND=

OPTIONS_FILE=
OPTIONS_FILES=

WINTTYLIB=
if test "$enable_tty_graphics" != "no"; then
    if test $host_os = mingw32; then
        AC_SUBST(EXTRA_UNSHARE,["nhdefkey.dll nh340key.dll nhraykey.dll"])
        CFLAGS="$CFLAGS -DWIN32CON"
        WINTTYLIB=""
        WINSRC="${WINSRC} \$(WINTTYSRC)"
        WINOBJ="${WINOBJ} \$(WINTTYOBJ)"
        WINLIB="${WINLIB} \$(WINTTYLIB)"
        AC_DEFINE(TTY_GRAPHICS, [], [Console output])
    else
        dnl  ----- Check for tparm in curses
        LIBS=
        AC_CHECK_HEADERS(ncurses.h curses.h termcap.h)
        AC_SEARCH_LIBS([tparm], [ncursesw ncurses curses tinfow tinfo], AC_DEFINE(HAVE_TPARM, 1, [Define this if you have the tparm function in an included lib.]))
        TERMLIB_VARIANTS="ncursesw ncurses curses tinfow tinfo termlib termcap terminfo"

        AC_SEARCH_LIBS([has_colors], ${TERMLIB_VARIANTS},
                     [WINTTYLIB="${LIBS}"
                      WINSRC="${WINSRC} \$(WINTTYSRC)"
                      WINOBJ="${WINOBJ} \$(WINTTYOBJ)"
                      WINLIB="${WINLIB} \$(WINTTYLIB)"
                      AC_DEFINE(TTY_GRAPHICS, [], [Console output])],
                      AC_MSG_ERROR([Can't find working curses library (needed for tty graphics)]))
    fi

    OPTIONS_FILE="options"
    OPTIONS_FILES="options"
fi

if test "$enable_tty_graphics" = "no" -a $host_os = mingw32; then
    LFLAGS="$LFLAGS -mwindows"
fi
AC_SUBST(WINTTYLIB)


WINCURSESLIB=
if test "$enable_curses_graphics" != "no"; then
    # For curses, determine the curses lib
    LIBS=
    AC_CHECK_HEADERS(ncurses.h curses.h termcap.h locale.h)
    AC_SEARCH_LIBS([tparm], [ncursesw ncurses curses tinfow tinfo], AC_DEFINE(HAVE_TPARM, 1, [Define this if you have the tparm function in an included lib.]))
    CURSES_VARIANTS="ncursesw ncurses curses tinfow tinfo"
    AC_SEARCH_LIBS([has_colors], ${CURSES_VARIANTS},
                 [WINCURSESLIB="${LIBS}"
                  WINSRC="${WINSRC} \$(WINCURSESSRC)"
                  WINOBJ="${WINOBJ} \$(WINCURSESOBJ)"
                  WINLIB="${WINLIB} \$(WINCURSESLIB)"
                  AC_DEFINE(CURSES_GRAPHICS, [], [Console output])],
                  AC_MSG_ERROR([Can't find working curses library (needed for curses graphics)]))

    OPTIONS_FILE="options"
    OPTIONS_FILES="options"
fi
AC_SUBST(WINCURSESLIB)

dnl if test "$enable_proxy_graphics" != "no"; then
dnl     if test "$have_libpng" = "no"; then
dnl 	if test "$enable_proxy_graphics" = "yes"; then
dnl 	    AC_MSG_ERROR([Need libpng for proxy graphics])
dnl 	else
dnl 	    AC_MSG_WARN([Can't enable proxy graphics (no useable libpng)])
dnl 	fi
dnl     else
dnl 	if test "$enable_proxy_graphics" = "yes"; then
dnl 	    PKG_CHECK_MODULES(NETHACK_PROXY, nhproxy >= 0.9.0)
dnl 	else
dnl 	    PKG_CHECK_MODULES(NETHACK_PROXY, nhproxy >= 0.9.0, :, :)
dnl 	fi
dnl 	if test -n "$NETHACK_PROXY_LIBS"; then
dnl 	    CFLAGS="${CFLAGS} ${NETHACK_LIBPNG_CFLAGS}"
dnl 	    LFLAGS="${LFLAGS} ${NETHACK_LIBPNG_LFLAGS}"
dnl 	    WINSRC="${WINSRC} \$(WINPROXYSRC)"
dnl 	    WINOBJ="${WINOBJ} \$(WINPROXYOBJ)"
dnl 	    WINLIB="${WINLIB} ${NETHACK_PROXY_LIBS}"
dnl 	    CNF_SHARE_DATND="${CNF_SHARE_DATND} \$(CNF_SHARE_PROXYND)"
dnl 	    AC_DEFINE(PROXY_GRAPHICS, [], [proxy graphics])
dnl 	fi
dnl     fi
dnl fi
dnl AC_SUBST(NETHACK_PROXY_CFLAGS)

if test "$enable_x11_graphics" != "no"; then
    if test $have_libXaw != yes; then
      AC_MSG_ERROR([Need X Athena Widgets Library for X11 graphics])
    fi
    AC_SEARCH_LIBS([XpmReadFileToImage], [Xpm], [USE_XPM="1"], [AC_MSG_ERROR([Need libXpm for X11 graphics])])

    CFLAGS="${CFLAGS} ${LIBXAW_CFLAGS}"
    WINSRC="${WINSRC} \$(WINX11SRC)"
    WINOBJ="${WINOBJ} \$(WINX11OBJ)"
    WINLIB="${WINLIB} ${LIBXAW_LIBS}"
    CNF_SHARE_DATND="${CNF_SHARE_DATND} \$(CNF_SHARE_X11ND)"
    AC_DEFINE(X11_GRAPHICS, [], [X11 graphics])

    OPTIONS_FILE="options"
    OPTIONS_FILES="options"
fi

if test "$enable_dummy_graphics" != "no"; then
    WINSRC="${WINSRC} \$(WINDUMMYSRC)"
    WINOBJ="${WINOBJ} \$(WINDUMMYOBJ)"
    AC_DEFINE(DUMMY_GRAPHICS, [], [Dummy graphics])

    OPTIONS_FILE="options"
    OPTIONS_FILES="options"
fi

if test "$enable_mswin_graphics" != "no"; then
    if test "$WINDRES" = "no"; then
        AC_MSG_ERROR([Need windres for Microsoft Window's native graphics])
    fi
    WINSRC="${WINSRC} \$(WINMSSRC)"
    WINOBJ="${WINOBJ} \$(WINMSOBJ)"
    WINLIB="${WINLIB} \$(WINMSLIB)"
    CNF_SHARE_DATND="${CNF_SHARE_DATND} \$(CNF_SHARE_MSND)"
    AC_DEFINE(MSWIN_GRAPHICS, [], [Microsoft Window's native graphics])

    OPTIONS_FILE="guioptions"
    OPTIONS_FILES="$OPTIONS_FILES guioptions"
fi

LISPLIB=
if test "$enable_lisp_graphics" != "no"; then
    WINSRC="${WINSRC} \$(WINLISPSRC)"
    WINOBJ="${WINOBJ} \$(WINLISPOBJ)"
    WINLIB="${WINLIB} \$(WINLISPLIB)"
    AC_DEFINE(LISP_GRAPHICS, [], [Lisp interface])

    OPTIONS_FILE="options"
    OPTIONS_FILES="options"
fi
AC_SUBST(LISPLIB)

AC_SUBST(WINSRC)
AC_SUBST(WINOBJ)
AC_SUBST(WINLIB)
AC_SUBST(CNF_SHARE_DATND)
AC_SUBST(OPTIONS_FILE)
AC_SUBST(OPTIONS_FILES)

AC_MSG_CHECKING(for valid window system configuration)
if test -n "$nethack_wins"; then
    AC_MSG_RESULT([yes])
    for win in $nethack_wins; do
	AC_DEFINE_UNQUOTED(DEFAULT_WINDOW_SYS,["$win"],
	  [The default windowing system])
	break
    done
else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([need to enable at least one window system])
fi

# Due to what may be a bug in autoconf, we have to call AC_CHECK_HEADERS()
# to get the various HAVE_xxx_H preprocessor variables detected by
# AC_HEADER_STDC active. These are needed for AC_CHECK_DECLS(), below.
AC_CHECK_HEADERS([inttypes.h stdint.h unistd.h string.h utime.h])
AC_CHECK_HEADER(regex.h, [have_regex_h=yes],[have_regex_h=no],/* TEST_REGEX */)

dnl ------ Check for data types
dnl -- Regex-Check for MENU_COLOR
if test "$have_regex_h" = "yes"; then
    AC_CHECK_DECL(regcomp,
    [AC_DEFINE(MENU_COLOR_REGEX, [], [Use regular expressions])
     AC_SUBST(REGEXOBJ,[posixregex.o])],
    [AC_SUBST(REGEXOBJ,[pmatchregex.o])],
     #include <regex.h>
    )
else
    AC_SUBST(REGEXOBJ,[pmatchregex.o])
fi

dnl  ----- Check for library functions
AC_CHECK_DECLS([strncmpi, strnicmp], AC_DEFINE(STRNCMPI, [], [Use strncmpi]))

dnl ------ Check needed functions for TIMED_DELAY
AC_CHECK_FUNC(msleep, cf_result=yes, cf_result=no)
if test "$cf_result" = no; then
	AC_CHECK_FUNC(usleep, AC_DEFINE(TIMED_DELAY, [], [Using usleep]), [])
else
	AC_DEFINE(TIMED_DELAY, [], [Using msleep])
fi

dnl ------ Check fcntl for USE_FCNTL
AC_CHECK_FUNCS(fcntl)

dnl ------ Check for 32 bit datatype
AX_DEFINE_INTEGER_BITS(uint32_t, unsigned int, unsigned long, unsigned long long)
AX_DEFINE_INTEGER_BITS(uint64_t, unsigned int, unsigned long, unsigned long long)

dnl  ----- Check for program needed to build Guidebook
AC_CHECK_PROG(HAVE_NROFF, nroff, "yes", "no")
AC_CHECK_PROG(HAVE_TBL, tbl, "yes", "no")
if test "$HAVE_NROFF" != "no" ; then
if test "$HAVE_TBL" != "no" ; then
	AC_SUBST(GUIDEBOOK,[Guidebook])
	AC_SUBST(GUIDEBOOK_TXT,[Guidebook.txt])
fi
fi
AC_CHECK_PROG(HAVE_COL, col, "yes", "no")
if test "$HAVE_COL" != "yes" ; then
        if test $host_os != mingw32; then
                AC_MSG_ERROR([Couldn't find 'col'])
        else
                AC_MSG_ERROR([Couldn't find 'col' (install 'util-linux' in Cygwin)])
        fi
fi

if test $host_os != mingw32; then
  dnl searching for non blocking random device
  AC_MSG_CHECKING(for pseudo random number device)
  if test -r "/dev/urandom"; then
	AC_DEFINE(DEV_RANDOM, ["/dev/urandom"], [Pseudo random number device])
	AC_MSG_RESULT(/dev/urandom)
  elif test -r "/dev/random"; then
	AC_DEFINE(DEV_RANDOM, ["/dev/random"], [Pseudo random number device])
	AC_MSG_RESULT(/dev/random)
  else
	AC_MSG_RESULT(not found);
  fi
fi

dnl  ----- Check for compile-time options
# We use auto for data librarian to avoid breaking the rule that autoconf
# should use the same defaults as the manual configuration, but all platforms
# that support autoconf also support DLB, so the automatic choice is always yes.
NETHACK_ARG(data-librarian,[Data librarian],DLB,auto,
  [enable_data_librarian=yes])
NETHACK_ARG(sinks,[Kitchen sinks],SINKS,yes)
# NETHACK_ARG(light-src-spell,[Spells are light sources],LIGHT_SRC_SPELL,yes)
NETHACK_ARG(wallified-maze,[Fancy mazes],WALLIFIED_MAZE,yes)
NETHACK_ARG(reincarnation,[Special Rogue-like levels],REINCARNATION,yes)
NETHACK_ARG(blackmarket,[Blackmarket],BLACKMARKET,yes)
NETHACK_ARG(kops,[Keystone Kops],KOPS,yes)
NETHACK_ARG(seduce,[Succubi/incubi seduction],SEDUCE,yes)
# NETHACK_ARG(invisible-objects,[Invisible objects],INVISIBLE_OBJECTS,yes)
# NETHACK_ARG(unpolypile,[Object unpolymorphing],UNPOLYPILE,yes)
# NETHACK_ARG(wallet-o-p,[Perseus' Wallet],WALLET_O_P,yes)
# NETHACK_ARG(lightsabers,[Lightsabers],LIGHTSABERS,yes)
# NETHACK_ARG(d-saber,[Dimsabers],LIGHTSABERS,yes)
# NETHACK_ARG(p-spoon,[Spoon],P_SPOON,yes)
# NETHACK_ARG(firearms,[Guns and Bullets],FIREARMS,yes)
# NETHACK_ARG(eaten-memory,[Remember which monsters have been eaten],
#  EATEN_MEMORY,yes)
NETHACK_ARG(randomized-planes,[Shuffling order of Elemental Planes],RANDOMIZED_PLANES,yes)
NETHACK_ARG(steed,[Ability to ride monsters],STEED,yes)
# NETHACK_ARG(dwarf,[Dwarf race],DWARF,yes)
NETHACK_ARG(tourist,[Tourist class],TOURIST,yes)
NETHACK_ARG(convict,[Convict class],CONVICT,yes)
# NETHACK_ARG(yeoman,[Yeoman class],YEOMAN,yes)
# NETHACK_ARG(zouthern,[Zoutherner class],ZOUTHERN,no)
NETHACK_ARG(redo,[Redo last command],REDO,yes)
NETHACK_ARG(clipping,[Allow smaller screens],CLIPPING,yes)
NETHACK_ARG(menu-color,[TTY graphics menu color],MENU_COLOR,yes)
if test "$enable_tty_graphics" != "no"; then
NETHACK_ARG(status-color,[TTY graphics status color],STATUS_COLORS,yes)
fi
NETHACK_ARG(auto-open,[Open doors by walking into them],AUTO_OPEN,yes)
NETHACK_ARG(elbereth,[Engraving the E-word repels monsters],ELBERETH,yes)
# NETHACK_ARG(noartifactwish,[Prevent wishing for special artifacts],
#  NOARTIFACTWISH,no)
# NETHACK_ARG(no-bones,[Disable loading and saving bones levels],NO_BONES,no)
if test $host_os != mingw32; then
 NETHACK_ARG(bones-pool,[Multiple bones per level],BONES_POOL,yes)
fi
NETHACK_ARG(user-sounds,
  [Allow user-defined mapping from messages to sounds)],
  USER_SOUNDS,no)
NETHACK_ARG(exp-on-botl,[Show experience on bottom line],EXP_ON_BOTL,yes)
NETHACK_ARG(score-on-botl,[Show score on bottom line],SCORE_ON_BOTL,no)
NETHACK_ARG(realtime-on-botl,[Show elapsed time on bottom line],REALTIME_ON_BOTL,no)
NETHACK_ARG(high-score-comparison-by-uid,[Compare High Score entries by user id instead of user name],PERS_IS_UID,no)
#NETHACK_ARG(borg,[Borg player (only under DOS)],BORG,no)
# NETHACK_ARG(keep-save,[Keep save files after restore],KEEP_SAVE,no)
# NETHACK_ARG(charon,[Charon's boat, enables Cerebus (not implemented)],CHARON,no)
# NETHACK_ARG(show-dmg,[Display damage reports],SHOW_DMG,yes)
# NETHACK_ARG(show-weight,[Display object weight],SHOW_WEIGHT,yes)
# NETHACK_ARG(other-services,[Shopkeeper services],OTHER_SERVICES,yes)
NETHACK_ARG(paranoid,[Require some questions to be answered with 'yes' instead of 'y'],PARANOID,yes)
NETHACK_ARG(shell,[Start a shell from within the game],SHELL,no)
NETHACK_ARG(show-born,[Display number of created monsters at the end of game],SHOW_BORN,yes)
NETHACK_ARG(show-extinct,[Display extinct monsters at the end of game],SHOW_EXTINCT,yes)
NETHACK_ARG(sortloot,[Sort item lists alphabetically by description.],SORTLOOT,yes)
NETHACK_ARG(dungeon-growth,[Dungeon growth],DUNGEON_GROWTH,yes)
#NETHACK_ARG(shout,[Shouting and pet commands (not implemented)],SHOUT,no)
# NETHACK_ARG(display-layers,[Support transparent tilesets],DISPLAY_LAYERS,yes)
NETHACK_ARG(autopickup-exceptions,[Exceptions to autopickup],
  AUTOPICKUP_EXCEPTIONS,yes)
NETHACK_ARG(qwertz,[QWERTZ keyboard layout support],QWERTZ,yes)
NETHACK_ARG(simple-mail,[simple mail dgamelaunch support],SIMPLE_MAIL,no)

NETHACK_ARG_WITH_PARAM(xlogfile,[More verbose logfile],XLOGFILE,yes,FILE,[xlogfile])

NETHACK_ARG_WITH_PARAM(livelog,[Live game progress logfile],LIVELOGFILE,no,FILE,[livelog])
if test "$enable_livelog" != "no" ; then
	NETHACK_ARG(livelog-shout,[Shoutting into the live game logfile],LIVELOG_SHOUT,no)
	NETHACK_ARG(livelog-killing,[Reporting killing of uniques and ghosts of former players],LIVELOG_BONES_KILLER,no)
fi

NETHACK_ARG(dump,[Dumping character stats],DUMP_LOG,yes)
if test "$enable_dump" != "no" ; then
	NETHACK_ARG_WITH_PARAM(dump-file,[Name of character dump file],DUMP_FN,no,FILE,"/tmp/%n.nh")
	NETHACK_ARG_WITH_PARAM(dump-messages,[Number of dumped messages],DUMPMSGS,yes,NUMBER,30)
	NETHACK_ARG(dump-text,[Dump a text file],DUMP_TEXT_LOG,yes)
	NETHACK_ARG(dump-html,[Dump an HTML file],DUMP_HTML_LOG,no)
	if test "$enable_dump_html" != "no" ; then
		NETHACK_ARG_WITH_PARAM(dump-html-css-file,[Path to the css dump file],DUMP_HTML_CSS_FILE,yes,FILE,"nethack_dump.css")
		NETHACK_ARG(dump-html-css-embedded,[CSS file should be included in the HTML dump],DUMP_HTML_CSS_EMBEDDED,no)
	fi
fi

NETHACK_ARG_WITH_PARAM(sysconf,[Enable public server global configuration file],SYSCF_FILE,no,FILE,[sysconf])
if test "$enable_sysconf" != "no" ; then
	AC_DEFINE([SYSCF], [], [Use configuration file for public server])
	AC_SUBST(SYSCF_CP,[])
	AC_SUBST(SYSCF_COMMENT,[])
else
	AC_SUBST(SYSCF_CP,[@#])
	AC_SUBST(SYSCF_COMMENT,[#])
fi

if test "$enable_tty_graphics" != "no" -a "$host_os" != mingw32; then
	NETHACK_ARG(utf8-glyphs,[Enable UTF-8 glyphs in TTY],UTF8_GLYPHS,yes)
	if test "$enable_utf8_glyphs" != "no" ; then
		dnl check for uint32_t
		if test "$uint32_t" = no; then
			AC_MSG_ERROR([unable to continue without a 4-bytes datatype that could hold unicode codepoints])
		fi
	fi
fi


NETHACK_ARG_WITH_PARAM(whereis-file,[Log information about running games],WHEREIS_FILE,no,FILE,[%n.whereis])

NETHACK_ARG_WITH_PARAM(wizmode,[enable wizard mode for the specified user],WIZARD,yes,USER,[wizard])

if test "$CHOWN" != "true"; then
    AC_MSG_CHECKING([for owner of installed files])
    AC_ARG_WITH([owner], [AS_HELP_STRING([--with-owner=USER],
	    [Set the owner of installed files (default=games)])],
	    [], [with_owner=games])
    if test "$with_owner" != "no" ; then
	NETHACK_COMMAND_IFELSE(
		[find . -user "$with_owner" -prune -print > /dev/null],
		[AC_MSG_RESULT([$with_owner])
		 AC_SUBST(OWNER,[$with_owner])],
		[AC_MSG_FAILURE([test for user $with_owner failed])])
    else
	CHOWN=true
	AC_MSG_RESULT([not set])
    fi
fi

if test "$CHGRP" != "true"; then
    AC_MSG_CHECKING([for group of installed files])
    AC_ARG_WITH([group], [AS_HELP_STRING([--with-group=GROUP],
	    [Set the group of installed files (default=bin)])],
	    [], [with_group=bin])
    if test "$with_group" != "no" ; then
	NETHACK_COMMAND_IFELSE(
		[find . -group "$with_group" -prune -print > /dev/null],
		[AC_MSG_RESULT([$with_group])
		 AC_SUBST(GROUP,[$with_group])],
		[AC_MSG_FAILURE([test for group $with_group failed])])
    else
	CHGRP=true
	AC_MSG_RESULT([not set])
    fi
fi


NETHACK_ARG(file-areas,[Place files in different directories],FILE_AREAS,yes)
NETHACK_ARG(debug-fuzzer,[Automatic fuzzing for debugging],DEBUG_FUZZER,no)

AC_DEFUN([NETHACK_DIR_SET],
  [
   AC_MSG_CHECKING([which directory is $1])
   AC_ARG_WITH([$1], [AS_HELP_STRING([--with-$1=DIR],
    [$7 (default=$3)])],
    [autoconf_val='"'$withval'"'], [withval=$4;autoconf_val='$5'])
   if test "$enable_file_areas" != "no" ; then
     AC_SUBST($6,[$withval])
     AC_DEFINE_UNQUOTED($6, [$autoconf_val], [$7])
     AC_MSG_RESULT([$withval])
   else
     # set default value if FILE_AREAS is not defined
     if test "$6" != "FILE_AREA_SAVE" ; then
       tmp_installdir=[${datadir}/nethack]
     else
       tmp_installdir=[${datadir}/nethack/save]
     fi
     AC_SUBST($6,[${tmp_installdir}])
     AC_MSG_RESULT([default value: ${tmp_installdir}])
   fi
  ])

# set doc directory manually
if test "$enable_file_areas" != "no" ; then
  AC_SUBST(FILE_AREA_DOC,[${docdir}/nethack])
  AC_DEFINE_UNQUOTED(FILE_AREA_DOC, [AUTOCONF_DOCDIR"/nethack"],
    [Location of human-readable documentation])
else
  # ignore ${docdir} if FILE_AREAS is not defined
  AC_SUBST(FILE_AREA_DOC,[${datadir}/nethack])
fi

NETHACK_DIR_SET(gamesdir, gamesdir, [LOCALSTATEDIR/nethack],
		[${localstatedir}/nethack],
		[AUTOCONF_LOCALSTATEDIR"/nethack"],
		FILE_AREA_VAR, [Location of other files which may be modified])

NETHACK_DIR_SET(bonesdir, bones, [GAMESDIR/bones],
		[${FILE_AREA_VAR}/bones],
		[FILE_AREA_VAR"/bones"],
		FILE_AREA_BONES, [Location of bones files (rw)])

NETHACK_DIR_SET(savesdir, saves, [GAMESDIR/saves],
		[${FILE_AREA_VAR}/saves],
		[FILE_AREA_VAR"/saves"],
		FILE_AREA_SAVE, [Location of save files (rw)])

NETHACK_DIR_SET(leveldir, level, [GAMESDIR/level],
		[${FILE_AREA_VAR}/level],
		[FILE_AREA_VAR"/level"],
		FILE_AREA_LEVL, [Location of level files (rw)])

NETHACK_DIR_SET(sharedir, share, [DATADIR/nethack],
		[${datadir}/nethack],
		[AUTOCONF_DATADIR"/nethack"],
		FILE_AREA_SHARE, [For read-only, architecture independent files])

NETHACK_DIR_SET(unsharedir, unshare, [DATADIR/nethack],
		[${datadir}/nethack],
		[AUTOCONF_DATADIR"/nethack"],
		FILE_AREA_UNSHARE, [For read-only, architecture dependent files])


NETHACK_LINKS=
NETHACK_LINKS_ADD([dat/data.base])
NETHACK_LINKS_ADD([dat/rumors.tru])
NETHACK_LINKS_ADD([dat/rumors.fal])
NETHACK_LINKS_ADD([dat/quest.txt])
NETHACK_LINKS_ADD([dat/oracles.txt])
NETHACK_LINKS_ADD([dat/dungeon.def])
NETHACK_LINKS_ADD([dat/help])
NETHACK_LINKS_ADD([dat/hh])
NETHACK_LINKS_ADD([dat/cmdhelp])
NETHACK_LINKS_ADD([dat/history])
NETHACK_LINKS_ADD([dat/opthelp])
NETHACK_LINKS_ADD([dat/wizhelp])
NETHACK_LINKS_ADD([include/win32api.h:sys/winnt/win32api.h])

AC_CONFIG_LINKS(${NETHACK_LINKS})

dnl  ----- Process the Makefiles
AC_CONFIG_FILES([Makefile:sys/autoconf/Makefile.top]
	[src/Makefile:sys/autoconf/Makefile.src]
	[doc/Makefile:sys/autoconf/Makefile.doc]
	[dat/Makefile:sys/autoconf/Makefile.dat]
	[util/Makefile:sys/autoconf/Makefile.utl]
	[sys/autoconf/depend.awk:sys/autoconf/depend.in])

AH_TOP([#ifndef AUTOCONF_H
#define AUTOCONF_H])
AH_BOTTOM([#endif /* AUTOCONF_H */])

NETHACK_CONFIG_HEADER([include/autoconf.h:sys/autoconf/autoconf.hin])

AC_OUTPUT
