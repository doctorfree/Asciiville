/*  SCCS Id: @(#)decl.h 3.4 2001/12/10  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DECL_H
#define DECL_H

#define E extern

E int NDECL((*occupation));
E int NDECL((*afternmv));

E const char *hname;
E int hackpid;
#if defined(UNIX) || defined(VMS)
E int locknum;
#endif
#ifdef DEF_PAGER
E char *catmore;
#endif  /* DEF_PAGER */

E char SAVEF[];
#ifdef MICRO
E char SAVEP[];
#endif

E NEARDATA int bases[MAXOCLASSES];

E NEARDATA int multi;
E char multi_txt[BUFSZ];
E const char *multi_reason;
#if 0
E NEARDATA int warnlevel;
#endif
E NEARDATA int nroom;
E NEARDATA int nsubroom;
E NEARDATA int occtime;

#define WARNCOUNT 6         /* number of different warning levels */
E uchar warnsyms[WARNCOUNT];
E NEARDATA int warn_obj_cnt; /* count of monsters meeting criteria */

E int x_maze_max, y_maze_max;
E int otg_temp;

#ifdef REDO
E NEARDATA int in_doagain;
#endif

E struct dgn_topology {     /* special dungeon levels for speed */
    d_level d_oracle_level;
    d_level d_bigroom_level;    /* unused */
#ifdef REINCARNATION
    d_level d_rogue_level;
#endif
    d_level d_medusa_level;
    d_level d_stronghold_level;
    d_level d_valley_level;
    d_level d_wiz1_level;
    d_level d_wiz2_level;
    d_level d_wiz3_level;
    d_level d_juiblex_level;
    d_level d_orcus_level;
    d_level d_baalzebub_level;  /* unused */
    d_level d_asmodeus_level;   /* unused */
    d_level d_portal_level;     /* only in goto_level() [do.c] */
    d_level d_sanctum_level;
    d_level d_earth_level;
    d_level d_water_level;
    d_level d_fire_level;
    d_level d_air_level;
    d_level d_astral_level;
    xchar d_tower_dnum;
    xchar d_sokoban_dnum;
    xchar d_mines_dnum, d_quest_dnum;
    xchar d_mall_dnum;
    xchar d_sheol_dnum;
    xchar d_moria_dnum;
    xchar d_dragon_caves_dnum;
    d_level d_qstart_level, d_qlocate_level, d_nemesis_level;
    d_level d_knox_level;
    d_level d_nymph_level;
#ifdef ADVENT_CALENDAR
    d_level d_advcal_level;
#endif
#ifdef RECORD_ACHIEVE
    d_level d_mineend_level;
    d_level d_sokoend_level;
#endif
#ifdef BLACKMARKET
    d_level d_blackmarket_level;
#endif /* BLACKMARKET */
    d_level d_minetown_level;
    d_level d_town_level;
    d_level d_moria_level;
} dungeon_topology;
/* macros for accesing the dungeon levels by their old names */
#define oracle_level        (dungeon_topology.d_oracle_level)
#define bigroom_level       (dungeon_topology.d_bigroom_level)
#ifdef REINCARNATION
#define rogue_level     (dungeon_topology.d_rogue_level)
#endif
#define medusa_level        (dungeon_topology.d_medusa_level)
#define stronghold_level    (dungeon_topology.d_stronghold_level)
#define valley_level        (dungeon_topology.d_valley_level)
#define wiz1_level      (dungeon_topology.d_wiz1_level)
#define wiz2_level      (dungeon_topology.d_wiz2_level)
#define wiz3_level      (dungeon_topology.d_wiz3_level)
#define juiblex_level       (dungeon_topology.d_juiblex_level)
#define orcus_level     (dungeon_topology.d_orcus_level)
#define baalzebub_level     (dungeon_topology.d_baalzebub_level)
#define asmodeus_level      (dungeon_topology.d_asmodeus_level)
#define portal_level        (dungeon_topology.d_portal_level)
#define sanctum_level       (dungeon_topology.d_sanctum_level)
#define earth_level     (dungeon_topology.d_earth_level)
#define water_level     (dungeon_topology.d_water_level)
#define fire_level      (dungeon_topology.d_fire_level)
#define air_level       (dungeon_topology.d_air_level)
#define astral_level        (dungeon_topology.d_astral_level)
#define tower_dnum      (dungeon_topology.d_tower_dnum)
#define sokoban_dnum        (dungeon_topology.d_sokoban_dnum)
#define mines_dnum      (dungeon_topology.d_mines_dnum)
#define quest_dnum      (dungeon_topology.d_quest_dnum)
#define mall_dnum       (dungeon_topology.d_mall_dnum)
#define sheol_dnum      (dungeon_topology.d_sheol_dnum)
#define moria_dnum      (dungeon_topology.d_moria_dnum)
#define dragon_caves_dnum (dungeon_topology.d_dragon_caves_dnum)
#define qstart_level        (dungeon_topology.d_qstart_level)
#define qlocate_level       (dungeon_topology.d_qlocate_level)
#define nemesis_level       (dungeon_topology.d_nemesis_level)
#define knox_level      (dungeon_topology.d_knox_level)
#define nymph_level     (dungeon_topology.d_nymph_level)
#ifdef ADVENT_CALENDAR
# define advcal_level       (dungeon_topology.d_advcal_level)
#endif
#ifdef RECORD_ACHIEVE
#define mineend_level           (dungeon_topology.d_mineend_level)
#define sokoend_level           (dungeon_topology.d_sokoend_level)
#endif
#ifdef BLACKMARKET
#define blackmarket_level   (dungeon_topology.d_blackmarket_level)
#endif /* BLACKMARKET */
#define minetown_level      (dungeon_topology.d_minetown_level)
#define town_level      (dungeon_topology.d_town_level)
#define moria_level     (dungeon_topology.d_moria_level)

E NEARDATA stairway dnstair, upstair;       /* stairs up and down */
#define xdnstair    (dnstair.sx)
#define ydnstair    (dnstair.sy)
#define xupstair    (upstair.sx)
#define yupstair    (upstair.sy)

E NEARDATA stairway dnladder, upladder;     /* ladders up and down */
#define xdnladder   (dnladder.sx)
#define ydnladder   (dnladder.sy)
#define xupladder   (upladder.sx)
#define yupladder   (upladder.sy)

E NEARDATA stairway sstairs;

E NEARDATA dest_area updest, dndest;    /* level-change destination areas */

E NEARDATA coord inv_pos;
E NEARDATA dungeon dungeons[];
E NEARDATA s_level *sp_levchn;
#define dunlev_reached(x)   (dungeons[(x)->dnum].dunlev_ureached)

#include "quest.h"
E struct q_score quest_status;

#include "qtext.h"
E NEARDATA char pl_tutorial[QT_T_MAX-QT_T_FIRST+1];

E NEARDATA char pl_character[PL_CSIZ];
E NEARDATA char pl_race;        /* character's race */

E NEARDATA char pl_fruit[PL_FSIZ];
E NEARDATA int current_fruit;
E NEARDATA struct fruit *ffruit;

E NEARDATA char tune[6];

#define MAXLINFO (MAXDUNGEON * MAXLEVEL)
E struct linfo level_info[MAXLINFO];

E NEARDATA struct sinfo {
    int gameover;       /* self explanatory? */
    int stopprint;      /* inhibit further end of game disclosure */
#if defined(UNIX) || defined(VMS) || defined (__EMX__) || defined(WIN32)
    int done_hup;       /* SIGHUP or moral equivalent received
                         * -- no more screen output */
#endif
    int something_worth_saving; /* in case of panic */
    int panicking;      /* `panic' is in progress */
#if defined(VMS) || defined(WIN32)
    int exiting;        /* an exit handler is executing */
#endif
    int in_impossible;
#ifdef PANICLOG
    int in_paniclog;
#endif
    int wizkit_wishing;
} program_state;

E boolean restoring;
E boolean ransacked;
extern boolean zombify;

E const char quitchars[];
E const char vowels[];
E const char ynchars[];
E const char ynqchars[];
E const char ynaqchars[];
E const char ynNaqchars[];
E NEARDATA long yn_number;

E const char disclosure_options[];

E NEARDATA int smeq[];
E NEARDATA int doorindex;
E NEARDATA char *save_cm;

E NEARDATA struct kinfo {
    struct kinfo *next; /* chain of delayed killers */
    int id;             /* uprop keys to ID a delayed killer */
    int format;         /* one of the killer formats */
#define KILLED_BY_AN     0
#define KILLED_BY        1
#define NO_KILLER_PREFIX 2
    char name[BUFSZ]; /* actual killer name */
} killer;

E long done_money;

E long killer_flags;

#ifdef DUMP_LOG
E char dump_fn[];       /* dumpfile name (dump patch) */
#endif
#ifdef WHEREIS_FILE
E char whereis_real_path[255];
#endif
E const char *configfile;
E NEARDATA char plname[PL_NSIZ];
E NEARDATA char dogname[];
E NEARDATA char catname[];
E NEARDATA char horsename[];
#ifdef EXOTIC_PETS
E NEARDATA char monkeyname[];
E NEARDATA char wolfname[];
E NEARDATA char crocodilename[];
#endif
#ifdef CONVICT
E NEARDATA char ratname[];
#endif /* CONVICT */
E char preferred_pet;
E const char *occtxt;           /* defined when occupation != NULL */
E const char *nomovemsg;
E const char nul[];
E char lock[];

#ifdef QWERTZ
E const char qykbd_dir[], qzkbd_dir[], ndir[];
E char const *sdir;
#else
E const char sdir[], ndir[];
#endif
E const schar xdir[], ydir[], zdir[];

#define DOESCAPE '\033'

E NEARDATA schar tbx, tby;      /* set in mthrowu.c */

E NEARDATA struct multishot { int n, i; short o; boolean s; } m_shot;

E NEARDATA struct dig_info {        /* apply.c, hack.c */
    int effort;
    d_level level;
    coord pos;
    long lastdigtime;
    boolean down, chew, warned, quiet;
} digging;

extern long game_loop_counter;
E NEARDATA long moves, monstermoves;
E NEARDATA long wailmsg;

E NEARDATA boolean in_mklev;
E NEARDATA boolean in_mk_rndvault;
E NEARDATA boolean rndvault_failed;
E NEARDATA boolean stoned;
extern int unweapon;
E NEARDATA boolean mrg_to_wielded;
E NEARDATA struct obj *current_wand, *thrownobj, *kickedobj;

E NEARDATA boolean defer_see_monsters;

E NEARDATA boolean in_steed_dismounting;

E const int shield_static[];

#include "spell.h"
E NEARDATA struct spell spl_book[]; /* sized in decl.c */

#include "color.h"
#ifdef TEXTCOLOR
E const int zapcolors[];
#endif

E const char def_oc_syms[MAXOCLASSES];  /* default class symbols */
E uchar oc_syms[MAXOCLASSES];       /* current class symbols */
E const char def_monsyms[MAXMCLASSES];  /* default class symbols */
E uchar monsyms[MAXMCLASSES];       /* current class symbols */

#include "obj.h"
E NEARDATA struct obj *invent,
  *uarm, *uarmc, *uarmh, *uarms, *uarmg, *uarmf,
#ifdef TOURIST
  *uarmu,               /* under-wear, so to speak */
#endif
*uskin, *uamul, *uleft, *uright, *ublindf,
  *uwep, *uswapwep, *uquiver;

E NEARDATA struct obj *uchain;      /* defined only when punished */
E NEARDATA struct obj *uball;
E NEARDATA struct obj *migrating_objs;
E NEARDATA struct obj *billobjs;
E NEARDATA struct obj zeroobj;      /* init'd and defined in decl.c */
E NEARDATA const anything zeroany;

#include "engrave.h"
E struct engr *head_engr;

#include "you.h"
E NEARDATA struct you u;

#include "onames.h"
#ifndef PM_H        /* (pm.h has already been included via youprop.h) */
#include "pm.h"
#endif


#define PM_BABY_GRAY_DRAGON   PM_BABY_TATZELWORM
#define PM_BABY_SILVER_DRAGON PM_BABY_AMPHITERE
#define PM_BABY_RED_DRAGON    PM_BABY_DRAKEN
#define PM_BABY_WHITE_DRAGON  PM_BABY_LINDWORM
#define PM_BABY_ORANGE_DRAGON PM_BABY_SARKANY
#define PM_BABY_BLACK_DRAGON  PM_BABY_SIRRUSH
#define PM_BABY_BLUE_DRAGON   PM_BABY_LEVIATHAN
#define PM_BABY_GREEN_DRAGON  PM_BABY_WYVERN
/*#define PM_BABY_GLOWING_DRAGON   PM_BABY_GLOWING_DRAGON*/
#define PM_BABY_YELLOW_DRAGON PM_BABY_GUIVRE
#define PM_GRAY_DRAGON        PM_TATZELWORM
#define PM_SILVER_DRAGON      PM_AMPHITERE
#define PM_RED_DRAGON         PM_DRAKEN
#define PM_WHITE_DRAGON       PM_LINDWORM
#define PM_ORANGE_DRAGON      PM_SARKANY
#define PM_BLACK_DRAGON       PM_SIRRUSH
#define PM_BLUE_DRAGON        PM_LEVIATHAN
#define PM_GREEN_DRAGON       PM_WYVERN
/*#define PM_GLOWING_DRAGON   PM_GLOWING_DRAGON*/
#define PM_YELLOW_DRAGON      PM_GUIVRE

#define GRAY_DRAGON_SCALE_MAIL       MAGIC_DRAGON_SCALE_MAIL
#define SILVER_DRAGON_SCALE_MAIL     REFLECTING_DRAGON_SCALE_MAIL
#define RED_DRAGON_SCALE_MAIL        FIRE_DRAGON_SCALE_MAIL
#define WHITE_DRAGON_SCALE_MAIL      ICE_DRAGON_SCALE_MAIL
#define ORANGE_DRAGON_SCALE_MAIL     SLEEP_DRAGON_SCALE_MAIL
#define BLACK_DRAGON_SCALE_MAIL      DISINTEGRATION_DRAGON_SCALE_MA
#define BLUE_DRAGON_SCALE_MAIL       ELECTRIC_DRAGON_SCALE_MAIL
#define GREEN_DRAGON_SCALE_MAIL      POISON_DRAGON_SCALE_MAIL
#define GLOWING_DRAGON_SCALE_MAIL    STONE_DRAGON_SCALE_MAIL
#define YELLOW_DRAGON_SCALE_MAIL     ACID_DRAGON_SCALE_MAIL

#define GRAY_DRAGON_SCALES       MAGIC_DRAGON_SCALES
#define SILVER_DRAGON_SCALES     REFLECTING_DRAGON_SCALES
#define RED_DRAGON_SCALES        FIRE_DRAGON_SCALES
#define WHITE_DRAGON_SCALES      ICE_DRAGON_SCALES
#define ORANGE_DRAGON_SCALES     SLEEP_DRAGON_SCALES
#define BLACK_DRAGON_SCALES      DISINTEGRATION_DRAGON_SCALES
#define BLUE_DRAGON_SCALES       ELECTRIC_DRAGON_SCALES
#define GREEN_DRAGON_SCALES      POISON_DRAGON_SCALES
#define GLOWING_DRAGON_SCALES    STONE_DRAGON_SCALES
#define YELLOW_DRAGON_SCALES     ACID_DRAGON_SCALES


E NEARDATA const struct monst zeromonst; /* for init of new or temp monsters */
E NEARDATA struct monst youmonst;   /* init'd and defined in decl.c */
E NEARDATA struct monst *mydogs, *migrating_mons;

E NEARDATA struct permonst upermonst;   /* init'd in decl.c,
                                         * defined in polyself.c
                                         */

E NEARDATA struct mvitals {
    uchar born;
    uchar died;
    uchar mvflags;
} mvitals[NUMMONS];

E NEARDATA struct c_color_names {
    const char  *const c_black, *const c_amber, *const c_golden,
    *const c_light_blue, *const c_red, *const c_orange,
    *const c_green, *const c_silver, *const c_blue,
    *const c_purple, *const c_white;
} c_color_names;
#define NH_BLACK        c_color_names.c_black
#define NH_AMBER        c_color_names.c_amber
#define NH_GOLDEN       c_color_names.c_golden
#define NH_LIGHT_BLUE       c_color_names.c_light_blue
#define NH_RED          c_color_names.c_red
#define NH_ORANGE       c_color_names.c_orange
#define NH_GREEN        c_color_names.c_green
#define NH_SILVER       c_color_names.c_silver
#define NH_BLUE         c_color_names.c_blue
#define NH_PURPLE       c_color_names.c_purple
#define NH_WHITE        c_color_names.c_white

/* The names of the colors used for gems, etc. */
E const char *c_obj_colors[];

E struct c_common_strings {
    const char  *const c_nothing_happens, *const c_thats_enough_tries,
    *const c_silly_thing_to, *const c_shudder_for_moment,
    *const c_something, *const c_Something,
    *const c_You_can_move_again,
    *const c_Never_mind, *c_vision_clears,
    *const c_the_your[2];
} c_common_strings;
#define nothing_happens    c_common_strings.c_nothing_happens
#define thats_enough_tries c_common_strings.c_thats_enough_tries
#define silly_thing_to     c_common_strings.c_silly_thing_to
#define shudder_for_moment c_common_strings.c_shudder_for_moment
#define something      c_common_strings.c_something
#define Something      c_common_strings.c_Something
#define You_can_move_again c_common_strings.c_You_can_move_again
#define Never_mind     c_common_strings.c_Never_mind
#define vision_clears      c_common_strings.c_vision_clears
#define the_your       c_common_strings.c_the_your

/* material strings */
E const char *materialnm[];

/* Monster name articles */
#define ARTICLE_NONE    0
#define ARTICLE_THE 1
#define ARTICLE_A   2
#define ARTICLE_YOUR    3

/* Monster name suppress masks */
#define SUPPRESS_IT            0x01
#define SUPPRESS_INVISIBLE     0x02
#define SUPPRESS_HALLUCINATION 0x04
#define SUPPRESS_SADDLE        0x08
#define EXACT_NAME             0x0F
#define SUPPRESS_NAME          0x10

/* Vision */
E NEARDATA boolean vision_full_recalc;  /* TRUE if need vision recalc */
E NEARDATA char **viz_array;        /* could see/in sight row pointers */

/* Window system stuff */
E NEARDATA winid WIN_MESSAGE, WIN_STATUS;
E NEARDATA winid WIN_MAP, WIN_INVEN;
E char toplines[];
#ifndef TCAP_H
E struct tc_gbl_data {  /* also declared in tcap.h */
    char *tc_AS, *tc_AE;    /* graphics start and end (tty font swapping) */
    int tc_LI,  tc_CO;      /* lines and columns */
} tc_gbl_data;
#define AS tc_gbl_data.tc_AS
#define AE tc_gbl_data.tc_AE
#define LI tc_gbl_data.tc_LI
#define CO tc_gbl_data.tc_CO
#endif

/* xxxexplain[] is in drawing.c */
E const char * const monexplain[], invisexplain[], * const objexplain[], * const oclass_names[];

/* Some systems want to use full pathnames for some subsets of file names,
 * rather than assuming that they're all in the current directory.  This
 * provides all the subclasses that seem reasonable, and sets up for all
 * prefixes being null.  Port code can set those that it wants.
 */
#define HACKPREFIX  0
#define LEVELPREFIX 1
#define SAVEPREFIX  2
#define BONESPREFIX 3
#define DATAPREFIX  4   /* this one must match hardcoded value in dlb.c */
#define SCOREPREFIX 5
#define LOCKPREFIX  6
#define CONFIGPREFIX    7
#define TROUBLEPREFIX   8
#define PREFIX_COUNT    9
/* used in files.c; xxconf.h can override if needed */
# ifndef FQN_MAX_FILENAME
#define FQN_MAX_FILENAME 512
# endif

#if defined(NOCWD_ASSUMPTIONS) || defined(VAR_PLAYGROUND)
/* the bare-bones stuff is unconditional above to simplify coding; for
 * ports that actually use prefixes, add some more localized things
 */
#define PREFIXES_IN_USE
#endif

E char *fqn_prefix[PREFIX_COUNT];
#ifdef PREFIXES_IN_USE
E char *fqn_prefix_names[PREFIX_COUNT];
#endif

struct opvar {
    xchar spovartyp; /* one of SPOVAR_foo */
    union {
        char *str;
        long l;
    } vardata;
};

#ifdef AUTOPICKUP_EXCEPTIONS
struct autopickup_exception {
    struct nhregex *regex;
    char *pattern;
    boolean grab;
    struct autopickup_exception *next;
};
extern struct autopickup_exception *apelist;
#endif /* AUTOPICKUP_EXCEPTIONS */

struct _plinemsg {
    xchar msgtype;
    char *pattern;
    struct _plinemsg *next;
};

E struct _plinemsg *pline_msg;

#define MSGTYP_NORMAL   0
#define MSGTYP_NOREP    1
#define MSGTYP_NOSHOW   2
#define MSGTYP_STOP 3

enum bcargs { override_restriction = -1 };
struct breadcrumbs {
    const char *funcnm;
    int linenum;
    boolean in_effect;
};

#ifdef RECORD_ACHIEVE
struct u_achieve {
    Bitfield(get_bell, 1);           /* You have obtained the bell of
                                      * opening */
    Bitfield(get_candelabrum, 1);    /* You have obtained the candelabrum */
    Bitfield(get_book, 1);           /* You have obtained the book of
                                      * the dead */
    Bitfield(enter_gehennom, 1);     /* Entered Gehennom (including the
                                      * Valley) by any means */
    Bitfield(perform_invocation, 1);    /* You have performed the invocation
                                         * ritual */
    Bitfield(get_amulet, 1);         /* You have obtained the amulet
                                      * of Yendor */
    Bitfield(ascended, 1);           /* You ascended to demigod[dess]hood.
                                      * Not quite the same as
                                      * u.uevent.ascended. */
    Bitfield(get_luckstone, 1);      /* You obtained the luckstone at the
                                      * end of the mines. */
    Bitfield(solved_sokoban, 1);     /* Finished the last Sokoban puzzle. */
    Bitfield(finish_sokoban, 1);     /* You obtained the sokoban prize. */
    Bitfield(killed_medusa, 1);      /* You defeated Medusa. */
};

E struct u_achieve achieve;
#endif

#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)
extern struct u_realtime {
    time_t realtime;     /**< Accumulated playing time in seconds */
    time_t start_timing; /**< Time game was started or restored or 'realtime'
                              was last updated (savegamestate for checkpoint) */
    time_t last_displayed_time; /**< Last time displayed on the status line */
} urealtime;
#endif /* RECORD_REALTIME || REALTIME_ON_BOTL */

#ifdef SIMPLE_MAIL
E int mailckfreq;
#endif

E int use_mon_rng;

E boolean curses_stupid_hack;

/* special key functions */
enum nh_keyfunc {
    NHKF_ESC = 0,
    NHKF_DOAGAIN,

    NHKF_REQMENU,

    /* run ... clicklook need to be in a continuous block */
    NHKF_RUN,
    NHKF_RUN2,
    NHKF_RUSH,
    NHKF_FIGHT,
    NHKF_FIGHT2,
    NHKF_NOPICKUP,
    NHKF_RUN_NOPICKUP,
    NHKF_DOINV,
    NHKF_TRAVEL,
    NHKF_CLICKLOOK,

    NHKF_REDRAW,
    NHKF_REDRAW2,
    NHKF_GETDIR_SELF,
    NHKF_GETDIR_SELF2,
    NHKF_GETDIR_HELP,
    NHKF_COUNT,
    NHKF_GETPOS_SELF,
    NHKF_GETPOS_PICK,
    NHKF_GETPOS_PICK_Q,  /* quick */
    NHKF_GETPOS_PICK_O,  /* once */
    NHKF_GETPOS_PICK_V,  /* verbose */
    NHKF_GETPOS_SHOWVALID,
    NHKF_GETPOS_AUTODESC,
    NHKF_GETPOS_MON_NEXT,
    NHKF_GETPOS_MON_PREV,
    NHKF_GETPOS_OBJ_NEXT,
    NHKF_GETPOS_OBJ_PREV,
    NHKF_GETPOS_DUNGEON_FEATURE_NEXT,
    NHKF_GETPOS_DUNGEON_FEATURE_PREV,
    NHKF_GETPOS_DOOR_NEXT,
    NHKF_GETPOS_DOOR_PREV,
    NHKF_GETPOS_UNEX_NEXT,
    NHKF_GETPOS_UNEX_PREV,
    NHKF_GETPOS_INTERESTING_NEXT,
    NHKF_GETPOS_INTERESTING_PREV,
    NHKF_GETPOS_VALID_NEXT,
    NHKF_GETPOS_VALID_PREV,
    NHKF_GETPOS_HELP,
    NHKF_GETPOS_MENU,
    NHKF_GETPOS_LIMITVIEW,
    NHKF_GETPOS_MOVESKIP,

    NUM_NHKF
};

/* commands[] is used to directly access cmdlist[] instead of looping
   through it to find the entry for a given input character;
   move_X is the character used for moving one step in direction X;
   alphadirchars corresponds to old sdir,
   dirchars corresponds to ``iflags.num_pad ? ndir : sdir'';
   pcHack_compat and phone_layout only matter when num_pad is on,
   swap_yz only matters when it's off */
struct cmd {
    unsigned serialno;     /* incremented after each update */
    boolean num_pad;       /* same as iflags.num_pad except during updates */
    boolean pcHack_compat; /* for numpad:  affects 5, M-5, and M-0 */
    boolean phone_layout;  /* inverted keypad:  1,2,3 above, 7,8,9 below */
    boolean swap_yz;       /* QWERTZ keyboards; use z to move NW, y to zap */
    char move_W, move_NW, move_N, move_NE, move_E, move_SE, move_S, move_SW;
    const char *dirchars;      /* current movement/direction characters */
    const char *alphadirchars; /* same as dirchars if !numpad */
    const struct ext_func_tab *commands[256]; /* indexed by input character */
    char spkeys[NUM_NHKF];
};

extern NEARDATA struct cmd Cmd;

#undef E

#endif /* DECL_H */
