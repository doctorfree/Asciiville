/*	SCCS Id: @(#)config.h	3.4	2003/12/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CONFIG_H /* make sure the compiler does not see the typedefs twice */
#define CONFIG_H


/*
 * Section 1:	Operating and window systems selection.
 *		Select the version of the OS you are using.
 *		For "UNIX" select BSD, ULTRIX, SYSV, or HPUX in unixconf.h.
 *		A "VMS" option is not needed since the VMS C-compilers
 *		provide it (no need to change sec#1, vmsconf.h handles it).
 */

#define UNIX		/* delete if no fork(), exec() available */

/* #define MSDOS */	/* in case it's not auto-detected */

/* #define OS2 */	/* define for OS/2 */

/* #define TOS */	/* define for Atari ST/TT */

/* #define STUPID */	/* avoid some complicated expressions if
			   your C compiler chokes on them */
/* #define MINIMAL_TERM */
			/* if a terminal handles highlighting or tabs poorly,
			   try this define, used in pager.c and termcap.c */
/* #define ULTRIX_CC20 */
			/* define only if using cc v2.0 on a DECstation */
/* #define ULTRIX_PROTO */
			/* define for Ultrix 4.0 (or higher) on a DECstation;
			 * if you get compiler errors, don't define this. */
			/* Hint: if you're not developing code, don't define
			   ULTRIX_PROTO. */

#include "config1.h"	/* should auto-detect MSDOS, MAC, AMIGA, and WIN32 */

#ifdef AUTOCONF
# include "autoconf_paths.h"
# include "autoconf.h"
#endif


/* Windowing systems...
 * Define all of those you want supported in your binary.
 * Some combinations make no sense.  See the installation document.
 */
/* #define TTY_GRAPHICS */	/* good old tty based graphics */
/* #define CURSES_GRAPHICS */	/* Proper curses interface */
/* #define X11_GRAPHICS */	/* X11 interface */
/* #define QT_GRAPHICS */	/* Qt interface */
/* #define GNOME_GRAPHICS */	/* Gnome interface */
/* #define MSWIN_GRAPHICS */	/* Windows NT, CE, Graphics */
/* #define LISP_GRAPHICS */	/* lisp interface */

/*
 * Define the default window system.  This should be one that is compiled
 * into your system (see defines above).  Known window systems are:
 *
 *	tty, X11, mac, amii, BeOS, Qt, Gem, Gnome, lisp
 */

/* MAC also means MAC windows */
#ifdef MAC
# ifndef	AUX
#  define DEFAULT_WINDOW_SYS "mac"
# endif
#endif

/* Amiga supports AMII_GRAPHICS and/or TTY_GRAPHICS */
#ifdef AMIGA
# define AMII_GRAPHICS			/* (optional) */
# define DEFAULT_WINDOW_SYS "amii"	/* "amii", "amitile" or "tty" */
#endif

/* Atari supports GEM_GRAPHICS and/or TTY_GRAPHICS */
#ifdef TOS
# define GEM_GRAPHICS			/* Atari GEM interface (optional) */
# define DEFAULT_WINDOW_SYS "Gem"	/* "Gem" or "tty" */
#endif

#ifdef __BEOS__
#define BEOS_GRAPHICS /* (optional) */
#define DEFAULT_WINDOW_SYS "BeOS"  /* "tty" */
#ifndef HACKDIR	/* override the default hackdir below */
# define HACKDIR "/boot/apps/UnNetHack"
#endif
#endif

#ifdef QT_GRAPHICS
# define DEFAULT_WC_TILED_MAP   /* Default to tiles if users doesn't say wc_ascii_map */
# define USER_SOUNDS		/* Use sounds */
# ifndef __APPLE__
#  define USER_SOUNDS_REGEX
# endif
# define USE_XPM		/* Use XPM format for images (required) */
# define GRAPHIC_TOMBSTONE	/* Use graphical tombstone (rip.ppm) */
# ifndef DEFAULT_WINDOW_SYS
#  define DEFAULT_WINDOW_SYS "Qt"
# endif
#endif

#ifdef GNOME_GRAPHICS
# define USE_XPM		/* Use XPM format for images (required) */
# define GRAPHIC_TOMBSTONE	/* Use graphical tombstone (rip.ppm) */
# ifndef DEFAULT_WINDOW_SYS
#  define DEFAULT_WINDOW_SYS "Gnome"
# endif
#endif

#ifdef MSWIN_GRAPHICS
# ifdef TTY_GRAPHICS
# undef TTY_GRAPHICS
# endif
# ifndef DEFAULT_WINDOW_SYS
#  define DEFAULT_WINDOW_SYS "mswin"
# endif
# define HACKDIR "\\unnethack"
#endif

#ifdef CURSES_GRAPHICS
# ifndef DEFAULT_WINDOW_SYS
#  define DEFAULT_WINDOW_SYS "curses"
# endif
#endif

#ifndef DEFAULT_WINDOW_SYS
# define DEFAULT_WINDOW_SYS "tty"
#endif

#ifdef X11_GRAPHICS
/*
 * There are two ways that X11 tiles may be defined.  (1) using a custom
 * format loaded by NetHack code, or (2) using the XPM format loaded by
 * the free XPM library.  The second option allows you to then use other
 * programs to generate tiles files.  For example, the PBMPlus tools
 * would allow:
 *  xpmtoppm <x11tiles.xpm | pnmscale 1.25 | ppmquant 90 >x11tiles_big.xpm
 */
/* # define USE_XPM */		/* Disable if you do not have the XPM library */
# ifdef USE_XPM
#  define GRAPHIC_TOMBSTONE	/* Use graphical tombstone (rip.xpm) */
# endif
#endif


/*
 * Section 2:	Some global parameters and filenames.
 *		Commenting out WIZARD, LOGFILE, NEWS or PANICLOG removes that
 *		feature from the game; otherwise set the appropriate wizard
 *		name.  LOGFILE, NEWS and PANICLOG refer to files in the
 *		playground.
 */

#ifndef WIZARD		/* allow for compile-time or Makefile changes */
# ifndef KR1ED
#  define WIZARD  "wizard" /* the person allowed to use the -D option */
# else
#  define WIZARD
#  define WIZARD_NAME "wizard"
# endif
#endif

#define LOGFILE "logfile"	/* larger file for debugging purposes */
#define LOGAREA FILE_AREA_VAR
/* #define XLOGFILE "xlogfile" */ /* even larger logfile */
#define NEWS "news"		/* the file containing the latest hack news */
#define NEWS_AREA FILE_AREA_SHARE
#define PANICLOG "paniclog"	/* log of panic and impossible events */
/* #define LIVELOGFILE "livelog" */ /* live game progress log file */

/* #define LIVELOG_SHOUT */

/*
 *	If COMPRESS is defined, it should contain the full path name of your
 *	'compress' program.  Defining INTERNAL_COMP causes NetHack to do
 *	simpler byte-stream compression internally.  Both COMPRESS and
 *	INTERNAL_COMP create smaller bones/level/save files, but require
 *	additional code and time.  Currently, only UNIX fully implements
 *	COMPRESS; other ports should be able to uncompress save files a
 *	la unixmain.c if so inclined.
 *	If you define COMPRESS, you must also define COMPRESS_EXTENSION
 *	as the extension your compressor appends to filenames after
 *	compression.
 */

#ifndef AUTOCONF
#ifdef UNIX
/* path and file name extension for compression program */
/* #define COMPRESS "/usr/bin/compress" */	/* Lempel-Ziv compression */
/* #define COMPRESS_EXTENSION ".Z"	*/	/* compress's extension */
/* An example of one alternative you might want to use: */
#define COMPRESS "/bin/gzip"	/* FSF gzip compression */
#define COMPRESS_EXTENSION ".gz"		/* normal gzip extension */
#endif

#ifndef COMPRESS
# define INTERNAL_COMP	/* control use of NetHack's compression routines */
#endif
#endif

/*
 *	Data librarian.  Defining DLB places most of the support files into
 *	a tar-like file, thus making a neater installation.  See *conf.h
 *	for detailed configuration.
 */
/* #define DLB */	/* not supported on all platforms */

/*
 *	Defining INSURANCE slows down level changes, but allows games that
 *	died due to program or system crashes to be resumed from the point
 *	of the last level change, after running a utility program.
 */
#define INSURANCE	/* allow crashed game recovery */

#ifndef MAC
# define CHDIR		/* delete if no chdir() available */
#endif

#ifdef CHDIR
/*
 * If you define HACKDIR, then this will be the default playground;
 * otherwise it will be the current directory.
 */
# ifndef HACKDIR
#  define HACKDIR "/usr/games/lib/unnethackdir"
# endif

/*
 * Some system administrators are stupid enough to make Hack suid root
 * or suid daemon, where daemon has other powers besides that of reading or
 * writing Hack files.	In such cases one should be careful with chdir's
 * since the user might create files in a directory of his choice.
 * Of course SECURE is meaningful only if HACKDIR is defined.
 */
/* #define SECURE */	/* do setuid(getuid()) after chdir() */

/*
 * If it is desirable to limit the number of people that can play Hack
 * simultaneously, define HACKDIR, SECURE and MAX_NR_OF_PLAYERS.
 * #define MAX_NR_OF_PLAYERS 6
 */
#endif /* CHDIR */



/*
 * Section 3:	Definitions that may vary with system type.
 *		For example, both schar and uchar should be short ints on
 *		the AT&T 3B2/3B5/etc. family.
 */

/*
 * Uncomment the following line if your compiler doesn't understand the
 * 'void' type (and thus would give all sorts of compile errors without
 * this definition).
 */
/* #define NOVOID */			/* define if no "void" data type. */

/*
 * Uncomment the following line if your compiler falsely claims to be
 * a standard C compiler (i.e., defines __STDC__ without cause).
 * Examples are Apollo's cc (in some versions) and possibly SCO UNIX's rcc.
 */
/* #define NOTSTDC */			/* define for lying compilers */

#include "tradstdc.h"

/*
 * type schar: small signed integers (8 bits suffice) (eg. TOS)
 *
 *	typedef char	schar;
 *
 *	will do when you have signed characters; otherwise use
 *
 *	typedef short int schar;
 */
#ifdef AZTEC
# define schar	char
#else
typedef signed char	schar;
#endif

/*
 * type uchar: small unsigned integers (8 bits suffice - but 7 bits do not)
 *
 *	typedef unsigned char	uchar;
 *
 *	will be satisfactory if you have an "unsigned char" type;
 *	otherwise use
 *
 *	typedef unsigned short int uchar;
 */
#ifndef _AIX32		/* identical typedef in system file causes trouble */
typedef unsigned char	uchar;
#endif

/* Type used for outputting DECgraphics and IBMgraphics characters into
 * HTML dumps or for holding unicode codepoints. */
#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif
#ifdef UINT32_MAX
typedef uint32_t glyph_t;
#else
/* Fallback that should work on most systems */
typedef long glyph_t;
#endif

/*
 * Various structures have the option of using bitfields to save space.
 * If your C compiler handles bitfields well (e.g., it can initialize structs
 * containing bitfields), you can define BITFIELDS.  Otherwise, the game will
 * allocate a separate character for each bitfield.  (The bitfields used never
 * have more than 7 bits, and most are only 1 bit.)
 */
#define BITFIELDS	/* Good bitfield handling */

/* #define STRNCMPI */	/* compiler/library has the strncmpi function */

/*
 * There are various choices for the NetHack vision system.  There is a
 * choice of two algorithms with the same behavior.  Defining VISION_TABLES
 * creates huge (60K) tables at compile time, drastically increasing data
 * size, but runs slightly faster than the alternate algorithm.  (MSDOS in
 * particular cannot tolerate the increase in data size; other systems can
 * flip a coin weighted to local conditions.)
 *
 * If VISION_TABLES is not defined, things will be faster if you can use
 * MACRO_CPATH.  Some cpps, however, cannot deal with the size of the
 * functions that have been macroized.
 */

/* #define VISION_TABLES */ /* use vision tables generated at compile time */
#ifndef VISION_TABLES
# ifndef NO_MACRO_CPATH
#  define MACRO_CPATH	/* use clear_path macros instead of functions */
# endif
#endif

#define EXOTIC_PETS      /* Rob Ellwood  June 2002 */

/*
 * Section 4:  THE FUN STUFF!!!
 *
 * Conditional compilation of special options are controlled here.
 * If you define the following flags, you will add not only to the
 * complexity of the game but also to the size of the load module.
 */

#ifndef AUTOCONF
/* dungeon features */
#define SINKS		/* Kitchen sinks - Janet Walz */
/* dungeon levels */
#define WALLIFIED_MAZE	/* Fancy mazes - Jean-Christophe Collet */
#define REINCARNATION	/* Special Rogue-like levels */
/* monsters & objects */
#define KOPS		/* Keystone Kops by Scott R. Turner */
#define SEDUCE		/* Succubi/incubi seduction, by KAA, suggested by IM */
#define STEED		/* Riding steeds */
#define TOURIST		/* Tourist players with cameras and Hawaiian shirts */
#define CONVICT		/* Convict player with heavy iron ball */

/* difficulty */
#define ELBERETH	/* Engraving the E-word repels monsters */
/* I/O */
#define REDO		/* support for redoing last command - DGK */
#if !defined(MAC)
# define CLIPPING	/* allow smaller screens -- ERS */
#endif

#define EXP_ON_BOTL	/* Show experience on bottom line */
/* #define SCORE_ON_BOTL */	/* added by Gary Erickson (erickson@ucivax) */
#endif /* AUTOCONF */

#ifdef REDO
# define DOAGAIN '\001' /* ^A, the "redo" key used in cmd.c and getline.c */
#endif

/* #define REALTIME_ON_BOTL */  /* Show elapsed time on bottom line.  Note:
                                 * this breaks savefile compatibility. */

/* The options in this section require the extended logfile support */
#ifdef XLOGFILE
#define RECORD_CONDUCT  /* Record conducts kept in logfile */
#define RECORD_TURNS    /* Record turns elapsed in logfile */
#define RECORD_ACHIEVE  /* Record certain notable achievements in the
                         * logfile.  Note: this breaks savefile compatibility
                         * due to the addition of the u_achieve struct. */
#define RECORD_REALTIME /* Record the amount of actual playing time (in
                         * seconds) in the record file.  Note: this breaks
                         * savefile compatibility. */
#define RECORD_START_END_TIME /* Record to-the-second starting and ending
                               * times; stored as 32-bit values obtained
                               * from time(2) (seconds since the Epoch.) */
#define RECORD_GENDER0   /* Record initial gender in logfile */
#define RECORD_ALIGN0   /* Record initial alignment in logfile */
#endif

/*
 * Section 5:  EXPERIMENTAL STUFF
 *
 * Conditional compilation of new or experimental options are controlled here.
 * Enable any of these at your own risk -- there are almost certainly
 * bugs left here.
 */

#ifndef AUTOCONF

#define RANDOMIZED_PLANES /* Elemental Planes order is randomized - Patric Mueller (4 Jan 2009) */
#define BLACKMARKET	/* Massimo Campostrini (campo@sunthpi3.difi.unipi.it) */


#if defined(TTY_GRAPHICS) || defined(MSWIN_GRAPHICS) || \
 defined(CURSES_GRAPHICS)
# define MENU_COLOR
/*# define MENU_COLOR_REGEX*/
/*# define MENU_COLOR_REGEX_POSIX */
/* if MENU_COLOR_REGEX is defined, use regular expressions (regex.h,
 * GNU specific functions by default, POSIX functions with
 * MENU_COLOR_REGEX_POSIX).
 * otherwise use pmatch() to match menu color lines.
 * pmatch() provides basic globbing: '*' and '?' wildcards.
 */
#endif

#define STATUS_COLORS /* Shachaf & Oren Ben-Kiki */

#define DUNGEON_GROWTH /* Makes the dungeons feel a bit more living - Pasi Kallinen*/
/*#define GOLDOBJ */	/* Gold is kept on obj chains - Helge Hafting */
/*#define AUTOPICKUP_EXCEPTIONS */ /* exceptions to autopickup */
#define DUMP_LOG        /* Dump game end information to a file */
/* #define DUMP_FN "/tmp/%n.nh" */      /* Fixed dumpfile name, if you want
                                         * to prevent definition by users */
#define DUMP_TEXT_LOG   /* Dump game end information in a plain text form */
/*#define DUMP_HTML_LOG*/   /* Dump game end information to a html file */
#define DUMPMSGS 30     /* Number of latest messages in the dump file  */

#define AUTO_OPEN	/* open doors by walking into them - Stefano Busti */

/* #define WHEREIS_FILE "./whereis/%n.whereis" */ /* Write out player's current location to player.whereis */

#endif /* AUTOCONF */

#ifdef TTY_GRAPHICS
# define WIN_EDGE	/* windows aligned left&top */
# define VIDEOSHADES    /* Slash'Em like colors */
#endif

#define ADVENT_CALENDAR

/* End of Section 5 */

/*
 * Section 6:  UNCODITIONAL DEFINES
 *
 * These defines must be defined.
 * They come from patches that have been unconditionally incorporated
 * into UnNetHack.
 */
#define WEBB_DISINT /* Disintegrator - Nicholas Webb */
#define ASTRAL_ESCAPE /* Allow escape from Astral plane (with the Amulet) - CWC */
/* #define LIVELOG_BONES_KILLER */ /* Report if a ghost of a former player is
                                    * killed - Patric Mueller (15 Aug 2009) */
#define ADJSPLIT /* splittable #adjust - Sam Dennis, conditionalized by Jukka Lahtinen */
#define TUTORIAL_MODE /* Alex Smith */
#define ELBERETH_CONDUCT /* Track the number of times the player engraves Elbereth. - Ray Kulhanek */
#define SHOW_WEIGHT
/* End of Section 6 */

#include "global.h"	/* Define everything else according to choices above */

#endif /* CONFIG_H */
