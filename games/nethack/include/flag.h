/*  SCCS Id: @(#)flag.h 3.4 2002/08/22  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* If you change the flag structure make sure you increment EDITLEVEL in   */
/* patchlevel.h if needed.  Changing the instance_flags structure does     */
/* not require incrementing EDITLEVEL.                     */

#ifndef FLAG_H
#define FLAG_H

#include "color.h"

/*
 * Persistent flags that are saved and restored with the game.
 *
 */

struct flag {
    boolean acoustics;  /* allow dungeon sound messages */
#ifdef AMIFLUSH
    boolean altmeta;    /* use ALT keys as META */
    boolean amiflush;   /* kill typeahead */
#endif
#ifdef  MFLOPPY
    boolean asksavedisk;
#endif
    boolean autodig;        /* MRKR: Automatically dig */
    boolean autoquiver;     /* Automatically fill quiver */
    boolean autounlock;     /* Automatically apply unlocking tool */
    boolean beginner;
#ifdef MAIL
    boolean biff;       /* enable checking for mail */
#endif
    boolean hint;       /* enable Unclippy the helpful hint daemon */
    boolean botl;       /* partially redo status line */
    boolean botlx;      /* print an entirely new bottom line */
    boolean confirm;    /* confirm before hitting tame monsters */
    boolean deathdropless;  /* disable monsters dropping random items */
    boolean debug;      /* in debugging mode */
#define wizard   flags.debug
#ifdef ELBERETH
    boolean elberethignore;  /* Elbereth ignores requests for protection */
#endif
    boolean end_own;    /* list all own scores */
    boolean explore;    /* in exploration mode */
    boolean tutorial;       /* in tutorial mode */
#ifdef OPT_DISPMAP
    boolean fast_map;   /* use optimized, less flexible map display */
#endif
#define discover flags.explore
    boolean female;
    boolean forcefight;
    boolean friday13;   /* it's Friday the 13th */
    boolean help;       /* look in data file for info about stuff */
    boolean hitpointbar;    /* colourful hit point status bar */
    boolean ignintr;    /* ignore interrupts */
#ifdef INSURANCE
    boolean ins_chkpt;  /* checkpoint as appropriate */
#endif
    boolean invlet_constant;  /* let objects keep their inventory symbol */
    boolean legacy;     /* print game entry "story" */
    boolean lit_corridor;   /* show a dark corr as lit if it is in sight */
    boolean made_amulet;
    boolean mon_moving;     /* monsters' turn to move */
    boolean move;
    boolean mv;
    boolean bypasses;   /* bypass flag is set on at least one fobj */
    boolean nap;        /* `timed_delay' option for display effects */
    boolean nopick;     /* do not pickup objects (as when running) */
    boolean null;       /* OK to send nulls to the terminal */
#ifdef MAC
    boolean page_wait;  /* put up a --More-- after a page of messages */
#endif
    boolean pickup;     /* whether you pickup or move and look */
    boolean pickup_dropped;     /* don't auto-pickup items you dropped */
    boolean pickup_thrown;      /* auto-pickup items you threw */

    boolean pushweapon;     /* When wielding, push old weapon into second slot */
    boolean safe_dog;   /* give complete protection to the dog */
#ifdef EXP_ON_BOTL
    boolean showexp;    /* show experience points */
#endif
    boolean invweight;
    boolean showweight;
#ifdef SCORE_ON_BOTL
    boolean showscore;  /* show score */
#endif
    boolean silent;     /* whether the bell rings or not */
    boolean sortpack;   /* sorted inventory */
    boolean soundok;    /* ok to tell about sounds heard */
    boolean sparkle;    /* show "resisting" special FX (Scott Bigham) */
    boolean standout;   /* use standout for --More-- */
    boolean time;       /* display elapsed 'time' */
    boolean tombstone;  /* print tombstone */
    boolean toptenwin;  /* ending list in window instead of stdout */
    boolean verbose;    /* max battle info */
    boolean wounds;     /* healers can see how badly monsters are damaged */
    boolean prayconfirm;    /* confirm before praying */
    int end_top, end_around;    /* describe desired score list */
    unsigned ident;     /* social security number for each monster */
    unsigned moonphase;
    unsigned long suppress_alert;
#define NEW_MOON    0
#define FULL_MOON   4
    unsigned no_of_wizards; /* 0, 1 or 2 (wizard and his shadow) */
    boolean travel;     /* find way automatically to u.tx,u.ty */
    unsigned run;       /* 0: h (etc), 1: H (etc), 2: fh (etc) */
    /* 3: FH, 4: ff+, 5: ff-, 6: FF+, 7: FF- */
    /* 8: travel */
    unsigned long warntype; /* warn_of_mon monster type M2 */
    unsigned startingpet_mid;
    int rndencode;          /* randomized escape sequence introducer */
    int warnlevel;
    int djinni_count, ghost_count;  /* potion effect tuning */
    int pickup_burden;      /* maximum burden before prompt */
    char inv_order[MAXOCLASSES];
    char pickup_types[MAXOCLASSES];
#define NUM_DISCLOSURE_OPTIONS      5
#define DISCLOSE_PROMPT_DEFAULT_YES 'y'
#define DISCLOSE_PROMPT_DEFAULT_NO  'n'
#define DISCLOSE_YES_WITHOUT_PROMPT '+'
#define DISCLOSE_NO_WITHOUT_PROMPT  '-'
    char end_disclose[NUM_DISCLOSURE_OPTIONS + 1];      /* disclose various info
                                                           upon exit */
    char menu_style;        /* User interface style setting */
    boolean made_fruit; /* don't easily let the user overflow the number of
                           fruits */
#ifdef AMII_GRAPHICS
    int numcols;
    unsigned short amii_dripens[ 20 ]; /* DrawInfo Pens currently there are 13 in v39 */
    AMII_COLOR_TYPE amii_curmap[ AMII_MAXCOLORS ]; /* colormap */
#endif
    boolean bones;      /* allow loading bones */
    boolean perma_hallu;    /* let the player permanently hallucinate */

    int last_broken_otyp;    /* from last object that got broken */
    int last_picked_up_otyp; /* from last object that got picked up */

    /* KMH, role patch -- Variables used during startup.
     *
     * If the user wishes to select a role, race, gender, and/or alignment
     * during startup, the choices should be recorded here.  This
     * might be specified through command-line options, environmental
     * variables, a popup dialog box, menus, etc.
     *
     * These values are each an index into an array.  They are not
     * characters or letters, because that limits us to 26 roles.
     * They are not booleans, because someday someone may need a neuter
     * gender.  Negative values are used to indicate that the user
     * hasn't yet specified that particular value.  If you determine
     * that the user wants a random choice, then you should set an
     * appropriate random value; if you just left the negative value,
     * the user would be asked again!
     *
     * These variables are stored here because the u structure is
     * cleared during character initialization, and because the
     * flags structure is restored for saved games.  Thus, we can
     * use the same parameters to build the role entry for both
     * new and restored games.
     *
     * These variables should not be referred to after the character
     * is initialized or restored (specifically, after role_init()
     * is called).
     */
    int initrole;   /* starting role      (index into roles[])   */
    int initrace;   /* starting race      (index into races[])   */
    int initgend;   /* starting gender    (index into genders[]) */
    int initalign;  /* starting alignment (index into aligns[])  */
    int randomall;  /* randomly assign everything not specified */
    int pantheon;   /* deity selection for priest character */

    /* --- initial roleplay flags ---
     * These flags represent the player's conduct/roleplay
     * intention at character creation.
     *
     * First the player can sets some of these at character
     * creation. (via configuration-file, ..)
     * Then role_init() may set/prevent certain combinations,
     * e.g. Monks get the vegetarian flag, vegans should also be
     * vegetarians, ..
     *
     * After that the initial flags shouldn't be modified.
     * In u_init() the flags can be used to put some
     * roleplay-intrinsics into the u structure. Only those
     * should be modified during gameplay.
     */
    boolean ascet;
    boolean atheist;
    boolean blindfolded;
    boolean illiterate;
    boolean pacifist;
    boolean nudist;
    boolean vegan;
    boolean vegetarian;

    /* Heaven or hell modes */
    boolean heaven_or_hell; /* player and monsters have max 1 HP and player has 3 lives */
    boolean hell_and_hell; /* like heaven_or_hell but only player has max 1 HP */

    boolean marathon;   /* cannot heal, fixed high max hp */

    boolean goldX;      /* for BUCX filtering, whether gold is X or U */
};

#define marathon_mode flags.marathon

/*
 * Flags that are set each time the game is started.
 * These are not saved with the game.
 *
 */
/* values for iflags.getpos_coords */
#define GPCOORDS_NONE    'n'
#define GPCOORDS_MAP     'm'
#define GPCOORDS_COMPASS 'c'
#define GPCOORDS_COMFULL 'f'
#define GPCOORDS_SCREEN  's'

enum getloc_filters {
    GFILTER_NONE = 0,
    GFILTER_VIEW,
    GFILTER_AREA,

    NUM_GFILTER
};

struct instance_flags {
    /* stuff that really isn't option or platform related. They are
     * set and cleared during the game to control the internal
     * behaviour of various NetHack functions and probably warrant
     * a structure of their own elsewhere some day.
     */
    boolean debug_fuzzer;  /* fuzz testing */
    boolean defer_plname;  /* X11 hack: askname() might not set g.plname */
    boolean herecmd_menu;  /* use menu when mouseclick on yourself */
    boolean invis_goldsym; /* gold symbol is ' '? */
    int at_midnight;       /* only valid during end of game disclosure */
    int at_night;          /* also only valid during end of game disclosure */
    int failing_untrap;    /* move_into_trap() -> spoteffects() -> dotrap() */
    int in_lava_effects;   /* hack for Boots_off() */
    int last_msg;          /* indicator of last message player saw */
    int override_ID;       /* true to force full identification of objects */
    int parse_config_file_src;  /* hack for parse_config_line() */
    int suppress_price;    /* controls doname() for unpaid objects */
    int terrainmode; /* for getpos()'s autodescribe when #terrain is active */
#define TER_MAP    0x01
#define TER_TRP    0x02
#define TER_OBJ    0x04
#define TER_MON    0x08
#define TER_DETECT 0x10    /* detect_foo magic rather than #terrain */
    boolean getloc_travelmode;
    int getloc_filter;     /* GFILTER_foo */
    boolean getloc_usemenu;
    boolean getloc_moveskip;

    /* stuff that is related to options and/or user or platform preferences
     */
    boolean mention_walls;    /* give feedback when bumping walls */
    boolean cbreak;     /* in cbreak mode, rogue format */
#ifdef CURSES_GRAPHICS
    boolean classic_status;     /* What kind of horizontal statusbar to use */
    boolean cursesgraphics;     /* Use portable curses extended characters */
#endif
    boolean DECgraphics;    /* use DEC VT-xxx extended character set */
    boolean echo;           /* 1 to echo characters */
    boolean IBMgraphics;    /* use IBM extended character set */
    boolean UTF8graphics;   /* use UTF-8 characters */
    int supports_utf8;      /* if the terminal supports utf8 */
    unsigned msg_history;   /* hint: # of top lines to save */
    int getpos_coords;      /* show coordinates when getting cursor position */
    boolean num_pad;        /* use numbers for movement commands */
    boolean news;           /* print news */
    boolean perm_invent;    /* keep full inventories up until dismissed */
    boolean window_inited;  /* true if init_nhwindows() completed */
    boolean vision_inited;  /* true if vision is ready */
    boolean menu_tab_sep;   /* Use tabs to separate option menu fields */
    boolean menu_requested; /* Flag for overloaded use of 'm' prefix
                             * on some non-move commands */
    uchar num_pad_mode;
    int menu_headings;  /* ATR for menu headings */
    int purge_monsters;         /* # of dead monsters still on fmon list */
    int *opt_booldup;   /* for duplication of boolean opts in config file */
    int *opt_compdup;   /* for duplication of compound opts in config file */
    uchar bouldersym;   /* symbol for boulder display */
    boolean autoexplore;    /* in autoexplore */
    boolean travel1;    /* first travel step */
    coord travelcc;     /* coordinates for travel_cache */
#ifdef QWERTZ
    boolean qwertz_layout;  /* swap y/z for this key layout */
#endif
#ifdef SIMPLE_MAIL
    boolean simplemail; /* simple mail format $NAME:$MESSAGE */
#endif
#ifdef WIZARD
    boolean sanity_check;   /* run sanity checks */
    boolean mon_polycontrol;    /* debug: control monster polymorphs */
    boolean in_parse;      /* is a command being parsed? */
#endif
#ifdef TTY_GRAPHICS
    char prevmsg_window;    /* type of old message window to use */
#endif
#if defined(TTY_GRAPHICS) || defined(CURSES_GRAPHICS)
    boolean extmenu;    /* extended commands use menu interface */
#endif
    boolean use_background_glyph; /* use background glyph when appropriate */
    boolean use_menu_color; /* use color in menus; only if wc_color */
#ifdef WIN_EDGE
    boolean win_edge;   /* are the menus aligned left&top */
#endif
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
    boolean use_status_colors; /* use color in status line; only if wc_color */
#endif
#ifdef MFLOPPY
    boolean checkspace;     /* check disk space before writing files */
    /* (in iflags to allow restore after moving
     * to >2GB partition) */
#endif
#ifdef MICRO
    boolean BIOS;       /* use IBM or ST BIOS calls when appropriate */
#endif
#if defined(MICRO) || defined(WIN32)
    boolean rawio;      /* whether can use rawio (IOCTL call) */
#endif
#ifdef MAC_GRAPHICS_ENV
    boolean MACgraphics;    /* use Macintosh extended character set, as
                               as defined in the special font HackFont */
    unsigned use_stone;         /* use the stone ppats */
#endif
#if defined(MSDOS) || defined(WIN32)
    boolean hassound;   /* has a sound card */
    boolean usesound;   /* use the sound card */
    boolean usepcspeaker;   /* use the pc speaker */
    boolean tile_view;
    boolean over_view;
    boolean traditional_view;
#endif
#ifdef MSDOS
    boolean hasvga;     /* has a vga adapter */
    boolean usevga;     /* use the vga adapter */
    boolean grmode;     /* currently in graphics mode */
#endif
#ifdef LAN_FEATURES
    boolean lan_mail;   /* mail is initialized */
    boolean lan_mail_fetched; /* mail is awaiting display */
#endif
#ifdef SORTLOOT
    char sortloot;      /* sort items to loot alphabetically */
#endif
#ifdef SHOW_BORN
    boolean show_born;  /* show numbers of created monsters */
#endif
    boolean showdmg;    /* show damage */
    /* only set when PARANOID is defined */
    boolean paranoid_hit;   /* Ask for 'yes' when hitting peacefuls */
    boolean paranoid_quit;  /* Ask for 'yes' when quitting */
    boolean paranoid_remove; /* Always show menu for 'T' and 'R' */
    boolean paranoid_trap; /* Ask for 'yes' before walking into known traps */
    boolean paranoid_lava; /* Ask for 'yes' before walking into lava */
    boolean paranoid_water; /* Ask for 'yes' before walking into water */
#ifdef USE_TILES
    boolean vt_nethack;
#endif
/*
 * Window capability support.
 */
    boolean wc_color;       /* use color graphics                  */
    boolean wc_hilite_pet;      /* hilight pets                        */
    boolean wc_ascii_map;       /* show map using traditional ascii    */
    boolean wc_tiled_map;       /* show map using tiles                */
    boolean wc_preload_tiles;   /* preload tiles into memory           */
    int wc_tile_width;      /* tile width                          */
    int wc_tile_height;     /* tile height                         */
    char    *wc_tile_file;      /* name of tile file;overrides default */
    boolean wc_inverse;     /* use inverse video for some things   */
    int wc_align_status;    /*  status win at top|bot|right|left   */
    int wc_align_message;   /* message win at top|bot|right|left   */
    int wc_vary_msgcount;       /* show more old messages at a time    */
    char    *wc_foregrnd_menu;  /* points to foregrnd color name for menu win   */
    char    *wc_backgrnd_menu;  /* points to backgrnd color name for menu win   */
    char    *wc_foregrnd_message;   /* points to foregrnd color name for msg win    */
    char    *wc_backgrnd_message;   /* points to backgrnd color name for msg win    */
    char    *wc_foregrnd_status;    /* points to foregrnd color name for status win */
    char    *wc_backgrnd_status;    /* points to backgrnd color name for status win */
    char    *wc_foregrnd_text;  /* points to foregrnd color name for text win   */
    char    *wc_backgrnd_text;  /* points to backgrnd color name for text win   */
    char    *wc_font_map;       /* points to font name for the map win */
    char    *wc_font_message;   /* points to font name for message win */
    char    *wc_font_status;    /* points to font name for status win  */
    char    *wc_font_menu;      /* points to font name for menu win    */
    char    *wc_font_text;      /* points to font name for text win    */
    int wc_fontsiz_map;         /* font size for the map win           */
    int wc_fontsiz_message;     /* font size for the message window    */
    int wc_fontsiz_status;      /* font size for the status window     */
    int wc_fontsiz_menu;        /* font size for the menu window       */
    int wc_fontsiz_text;        /* font size for text windows          */
    int wc_scroll_amount;   /* scroll this amount at scroll_margin */
    int wc_scroll_margin;   /* scroll map when this far from
                               the edge */
    int wc_map_mode;        /* specify map viewing options, mostly
                               for backward compatibility */
    int wc_player_selection;    /* method of choosing character */
    boolean wc_splash_screen;   /* display an opening splash screen or not */
    boolean wc_popup_dialog;    /* put queries in pop up dialogs instead of
                                   in the message window */
    boolean wc_eight_bit_input; /* allow eight bit input               */
    boolean wc_mouse_support;   /* allow mouse support */
    boolean wc2_fullscreen;     /* run fullscreen */
    boolean wc2_softkeyboard;   /* use software keyboard */
    boolean wc2_wraptext;       /* wrap text */
    int wc2_term_cols;          /* terminal width, in characters */
    int wc2_term_rows;          /* terminal height, in characters */
    int wc2_windowborders;      /* display borders on NetHack windows */
    int wc2_petattr;            /* points to text attributes for pet */
    boolean wc2_guicolor;       /* allow colors in GUI (outside map) */
    boolean wc2_newcolors;      /* try to use slashem like colors including
                                 * dark-gray to represent black object */

    boolean hp_notify;
    char *hp_notify_fmt;
    char *nameempty;    /* what to automatically name known empty wands */
    boolean show_buc;   /* always show BUC status */
    boolean clicklook;  /* allow right-clicking for look */
    boolean cmdassist;  /* provide detailed assistance for some commands */
    boolean obsolete;   /* obsolete options can point at this, it isn't used */
    boolean rest_on_space;  /* space means rest */
    /* Items which belong in flags, but are here to allow save compatibility */
    boolean lootabc;    /* use "a/b/c" rather than "o/i/b" when looting */
    boolean showrace;   /* show hero glyph by race rather than by role */
    boolean travelcmd;  /* allow travel command */
    boolean show_dgn_name;  /* show dungeon names instead of Dlvl: on bottom line */
    int runmode;    /* update screen display during run moves */
    int pilesize;   /* how many items to list automatically */
#ifdef WIN32CON
#define MAX_ALTKEYHANDLER 25
    char altkeyhandler[MAX_ALTKEYHANDLER];
#endif
    /* copies of values in struct u, used during detection when the
       originals are temporarily cleared; kept here rather than
       locally so that they can be restored during a hangup save */
    Bitfield(save_uswallow, 1);
    Bitfield(save_uinwater, 1);
    Bitfield(save_uburied, 1);
    /* REALTIME_ON_BOTL */
    int showrealtime;    /* show elapsed time */
    int realtime_format; /* how elapsed time is presented */

#ifdef AUTO_OPEN
    boolean autoopen;   /* open doors by walking into them */
#endif
    boolean dark_room;  /* show shadows in lit rooms */
    boolean vanilla_ui_behavior;    /* fall back to vanilla behavior */
    boolean show_annotation;    /* level annotation when entering level */
    int statuslines;    /* number of status lines */
#ifdef TEXTCOLOR
    int color_mode; /* number of available colors */
    uint64_t color_definitions[CLR_MAX];
    char truecolor_separator;
#endif
    genericptr_t returning_missile; /* 'struct obj *'; Mjollnir or aklys */

    boolean force_invmenu;    /* always menu when handling inventory */
    boolean autodescribe;     /* autodescribe mode in getpos() */

    boolean renameallowed;    /* can change hero name during role selection */
    boolean renameinprogress; /* we are changing hero name */
};

/*
 * Old deprecated names
 */
#ifdef TTY_GRAPHICS
#define eight_bit_tty wc_eight_bit_input
#endif
#ifdef TEXTCOLOR
#define use_color wc_color
#endif
#define hilite_pet wc_hilite_pet
#define use_inverse wc_inverse
#ifdef MAC_GRAPHICS_ENV
#define large_font obsolete
#endif
#ifdef MAC
#define popup_dialog wc_popup_dialog
#endif
#define preload_tiles wc_preload_tiles

extern NEARDATA struct flag flags;
extern NEARDATA struct instance_flags iflags;

/* last_msg values
 * Usage:
 *  pline("some message");
 *    pline: vsprintf + putstr + iflags.last_msg = PLNMSG_UNKNOWN;
 *  iflags.last_msg = PLNMSG_some_message;
 * and subsequent code can adjust the next message if it is affected
 * by some_message.  The next message will clear iflags.last_msg.
 */
enum plnmsg_types {
    PLNMSG_UNKNOWN = 0,         /* arbitrary */
    PLNMSG_ONE_ITEM_HERE,       /* "you see <single item> here" */
    PLNMSG_TOWER_OF_FLAME,      /* scroll of fire */
    PLNMSG_CAUGHT_IN_EXPLOSION, /* explode() feedback */
    PLNMSG_OBJ_GLOWS,           /* "the <obj> glows <color>" */
    PLNMSG_OBJNAM_ONLY,         /* xname/doname only, for #tip */
    PLNMSG_OK_DONT_DIE          /* overriding death in explore/wizard mode */
};

/* runmode options */
#define RUN_TPORT   0   /* don't update display until movement stops */
#define RUN_LEAP    1   /* update display every 7 steps */
#define RUN_STEP    2   /* update display every single step */
#define RUN_CRAWL   3   /* walk w/ extra delay after each update */

enum gloctypes {
    GLOC_MONS = 0,
    GLOC_OBJS,
    GLOC_DUNGEON_FEATURE,
    GLOC_DOOR,
    GLOC_EXPLORE,
    GLOC_INTERESTING,
    GLOC_VALID,

    NUM_GLOCS
};

enum realtime_types {
    REALTIME_NONE = 0,
    REALTIME_PLAYTIME,
    REALTIME_WALLTIME,

    NUM_REALTIME_TYPES
};

enum realtime_formats {
    REALTIME_FORMAT_SECONDS = 0,
    REALTIME_FORMAT_CONDENSED,
    REALTIME_FORMAT_UNITS,

    NUM_REALTIME_FORMATS
};

#endif /* FLAG_H */
