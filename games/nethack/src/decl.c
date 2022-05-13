/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

int NDECL((*afternmv));
int NDECL((*occupation));

/* from xxxmain.c */
const char *hname = 0;      /* name of the game (argv[0] of main) */
int hackpid = 0;        /* current process id */
#if defined(UNIX) || defined(VMS)
int locknum = 0;        /* max num of simultaneous users */
#endif
#ifdef DEF_PAGER
char *catmore = 0;      /* default pager */
#endif

NEARDATA int bases[MAXOCLASSES] = DUMMY;

NEARDATA int multi = 0;
char multi_txt[BUFSZ] = DUMMY;
const char *multi_reason = NULL;
#if 0
NEARDATA int warnlevel = 0; /* used by movemon and dochugw */
#endif
NEARDATA int nroom = 0;
NEARDATA int nsubroom = 0;
NEARDATA int occtime = 0;

/* maze limits must be even; masking off lowest bit guarantees that */
int x_maze_max = (COLNO - 1) & ~1, y_maze_max = (ROWNO - 1) & ~1;
int otg_temp; /* used by object_to_glyph() [otg] */

#ifdef REDO
NEARDATA int in_doagain = 0;
#endif

/*
 *  The following structure will be initialized at startup time with
 *  the level numbers of some "important" things in the game.
 */
struct dgn_topology dungeon_topology = { 0 };

#include "quest.h"
struct q_score quest_status = DUMMY;

#include "qtext.h"
NEARDATA char pl_tutorial[QT_T_MAX-QT_T_FIRST+1] = {0};

NEARDATA int smeq[MAXNROFROOMS+1] = DUMMY;
NEARDATA int doorindex = 0;

NEARDATA int warn_obj_cnt = 0;
NEARDATA char *save_cm = 0;

NEARDATA struct kinfo killer = DUMMY;
NEARDATA long done_money = 0;

long killer_flags = 0L;

const char *nomovemsg = 0;
const char nul[40] = DUMMY;         /* contains zeros */
NEARDATA char plname[PL_NSIZ] = DUMMY;      /* player name */
NEARDATA char pl_character[PL_CSIZ] = DUMMY;
NEARDATA char pl_race = '\0';

NEARDATA char pl_fruit[PL_FSIZ] = DUMMY;
NEARDATA int current_fruit = 0;
NEARDATA struct fruit *ffruit = (struct fruit *)0;

NEARDATA char tune[6] = DUMMY;
NEARDATA boolean ransacked = 0;
boolean zombify = FALSE;

const char *occtxt = DUMMY;
const char quitchars[] = " \r\n\033";
const char vowels[] = "aeiouAEIOU";
const char ynchars[] = "yn";
const char ynqchars[] = "ynq";
const char ynaqchars[] = "ynaq";
const char ynNaqchars[] = "yn#aq";
NEARDATA long yn_number = 0L;

const char disclosure_options[] = "iavgc";

#if defined(MICRO) || defined(WIN32)
char hackdir[PATHLEN];      /* where rumors, help, record are */
# ifdef MICRO
char levels[PATHLEN];       /* where levels are */
# endif
#endif /* MICRO || WIN32 */


#ifdef MFLOPPY
char permbones[PATHLEN];    /* where permanent copy of bones go */
int ramdisk = FALSE;        /* whether to copy bones to levels or not */
int saveprompt = TRUE;
const char *alllevels = "levels.*";
const char *allbones = "bones*.*";
#endif

struct linfo level_info[MAXLINFO];

NEARDATA struct sinfo program_state;

/* 'rogue'-like direction commands (cmd.c) */
#ifdef QWERTZ
const char qykbd_dir[] = "hykulnjb><";       /* qwerty layout */
const char qzkbd_dir[] = "hzkulnjb><";       /* qwertz layout */
char const *sdir=qykbd_dir;
#else
const char sdir[] = "hykulnjb><";
#endif
const char ndir[] = "47896321><";   /* number pad mode */
const schar xdir[10] = { -1, -1, 0, 1, 1, 1, 0, -1, 0, 0 };
const schar ydir[10] = {  0, -1, -1, -1, 0, 1, 1, 1, 0, 0 };
const schar zdir[10] = {  0, 0, 0, 0, 0, 0, 0, 0, 1, -1 };

NEARDATA schar tbx = 0, tby = 0;    /* mthrowu: target */

/* for xname handling of multiple shot missile volleys:
   number of shots, index of current one, validity check, shoot vs throw */
NEARDATA struct multishot m_shot = { 0, 0, STRANGE_OBJECT, FALSE };

NEARDATA struct dig_info digging;

NEARDATA dungeon dungeons[MAXDUNGEON];  /* ini'ed by init_dungeon() */
NEARDATA s_level *sp_levchn;
NEARDATA stairway upstair = { 0 }, dnstair = { 0 };
NEARDATA stairway upladder = { 0 }, dnladder = { 0 };
NEARDATA stairway sstairs = { 0 };
NEARDATA dest_area updest = { 0 };
NEARDATA dest_area dndest = { 0 };
NEARDATA coord inv_pos = { 0 };

NEARDATA boolean defer_see_monsters = FALSE;
NEARDATA boolean in_mklev = FALSE;
NEARDATA boolean in_mk_rndvault = FALSE;
NEARDATA boolean rndvault_failed = FALSE;
NEARDATA boolean stoned = FALSE;    /* done to monsters hit by 'c' */
int unweapon = FALSE;
NEARDATA boolean mrg_to_wielded = FALSE;
/* weapon picked is merged with wielded one */
/* some objects need special handling during destruction or placement */
NEARDATA struct obj *current_wand = 0; /* wand currently zapped/applied */
NEARDATA struct obj *thrownobj = 0;    /* object in flight due to throwing */
NEARDATA struct obj *kickedobj = 0;    /* object in flight due to kicking */

NEARDATA boolean in_steed_dismounting = FALSE;

NEARDATA coord bhitpos = DUMMY;
NEARDATA coord doors[DOORMAX] = {DUMMY};

NEARDATA struct mkroom rooms[(MAXNROFROOMS+1)*2] = {DUMMY};
NEARDATA struct mkroom* subrooms = &rooms[MAXNROFROOMS+1];
struct mkroom *upstairs_room, *dnstairs_room, *sstairs_room;

struct engr *head_engr;

dlevel_t level;     /* level map */
struct trap *ftrap = (struct trap *)0;
NEARDATA struct monst youmonst = DUMMY;
NEARDATA struct permonst upermonst = DUMMY;
/* > to be replaced with context */
NEARDATA struct polearm_info polearm;
/* < to be replaced with context */
NEARDATA struct obj_split context_objsplit = DUMMY;
NEARDATA struct flag flags = DUMMY;
NEARDATA struct instance_flags iflags = DUMMY;
NEARDATA struct you u = DUMMY;

NEARDATA struct obj *invent = (struct obj *)0,
                    *uwep = (struct obj *)0, *uarm = (struct obj *)0,
                    *uswapwep = (struct obj *)0,
                    *uquiver = (struct obj *)0, /* quiver */
#ifdef TOURIST
                    *uarmu = (struct obj *)0, /* under-wear, so to speak */
#endif
*uskin = (struct obj *)0,     /* dragon armor, if a dragon */
                    *uarmc = (struct obj *)0, *uarmh = (struct obj *)0,
                    *uarms = (struct obj *)0, *uarmg = (struct obj *)0,
                    *uarmf = (struct obj *)0, *uamul = (struct obj *)0,
                    *uright = (struct obj *)0,
                    *uleft = (struct obj *)0,
                    *ublindf = (struct obj *)0,
                    *uchain = (struct obj *)0,
                    *uball = (struct obj *)0;

#ifdef TEXTCOLOR
/*
 *  This must be the same order as used for buzz() in zap.c.
 */
const int zapcolors[NUM_ZAP] = {
    HI_ZAP,         /* 0 - missile */
    CLR_ORANGE,     /* 1 - fire */
    CLR_WHITE,      /* 2 - frost */
    HI_ZAP,         /* 3 - sleep */
    CLR_BLACK,      /* 4 - death */
    CLR_WHITE,      /* 5 - lightning */
    /* NH 3.6.3: poison gas zap used to be yellow and acid zap was green,
       which conflicted with the corresponding dragon colors */
    CLR_GREEN,      /* 6 - poison gas */
    CLR_RED,        /* 7 - lava */
    CLR_YELLOW,     /* 8 - acid */
};
#endif /* text color */

const int shield_static[SHIELD_COUNT] = {
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,    /* 7 per row */
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,
};

NEARDATA struct spell spl_book[MAXSPELL + 1] = {DUMMY};

long game_loop_counter = 1L;
NEARDATA long moves = 1L, monstermoves = 1L;
/* These diverge when player is Fast */
NEARDATA long wailmsg = 0L;

/* objects that are moving to another dungeon level */
NEARDATA struct obj *migrating_objs = (struct obj *)0;
/* objects not yet paid for */
NEARDATA struct obj *billobjs = (struct obj *)0;

/* used to zero all elements of a struct obj */
struct obj zeroobj = DUMMY;           /* used to zero out a struct obj */
const struct monst zeromonst = DUMMY; /* used to zero out a struct monst */
const anything zeroany = DUMMY;       /* used to zero out union any */

/* originally from dog.c */
NEARDATA char dogname[PL_PSIZ] = DUMMY;
NEARDATA char catname[PL_PSIZ] = DUMMY;
NEARDATA char horsename[PL_PSIZ] = DUMMY;

#ifdef EXOTIC_PETS
NEARDATA char monkeyname[PL_PSIZ] = DUMMY;
NEARDATA char wolfname[PL_PSIZ] = DUMMY;
NEARDATA char crocodilename[PL_PSIZ] = DUMMY;
/* ...and 'preferred_pet', just below, can now be 'e'. */
#endif
#ifdef CONVICT
NEARDATA char ratname[PL_PSIZ] = DUMMY;
#endif /* CONVICT */

char preferred_pet; /* '\0', 'c', 'd', 'n' (none) */
/* monsters that went down/up together with @ */
NEARDATA struct monst *mydogs = (struct monst *)0;
/* monsters that are moving to another dungeon level */
NEARDATA struct monst *migrating_mons = (struct monst *)0;
/* autopickup exception list */
struct autopickup_exception *apelist = (struct autopickup_exception *)0;

NEARDATA struct mvitals mvitals[NUMMONS];

/* originally from end.c */
#ifdef DUMP_LOG
#ifdef DUMP_FN
char dump_fn[] = DUMP_FN;
#else
char dump_fn[PL_PSIZ] = DUMMY;
#endif
#endif /* DUMP_LOG */

NEARDATA struct c_color_names c_color_names = {
    "black", "amber", "golden",
    "light blue", "red", "orange", "green",
    "silver", "blue", "purple",
    "white"
};

const char *c_obj_colors[] = {
    "black",        /* CLR_BLACK */
    "red",          /* CLR_RED */
    "green",        /* CLR_GREEN */
    "brown",        /* CLR_BROWN */
    "blue",         /* CLR_BLUE */
    "magenta",      /* CLR_MAGENTA */
    "cyan",         /* CLR_CYAN */
    "gray",         /* CLR_GRAY */
    "transparent",      /* no_color */
    "orange",       /* CLR_ORANGE */
    "bright green",     /* CLR_BRIGHT_GREEN */
    "yellow",       /* CLR_YELLOW */
    "bright blue",      /* CLR_BRIGHT_BLUE */
    "bright magenta",   /* CLR_BRIGHT_MAGENTA */
    "bright cyan",      /* CLR_BRIGHT_CYAN */
    "white",        /* CLR_WHITE */
};

struct menucoloring *menu_colorings = 0;

struct c_common_strings c_common_strings = {
    "Nothing happens.",     "That's enough tries!",
    "That is a silly thing to %s.", "shudder for a moment.",
    "something", "Something", "You can move again.", "Never mind.",
    "vision quickly clears.", {"the", "your"}
};

/* NOTE: the order of these words exactly corresponds to the
   order of oc_material values #define'd in objclass.h. */
const char *materialnm[] = {
    "mysterious", "liquid", "wax", "organic", "flesh",
    "paper", "cloth", "leather", "wooden", "bone", "dragonhide",
    "iron", "metal", "copper", "silver", "gold", "platinum", "mithril",
    "plastic", "glass", "gemstone", "stone"
};

/* Vision */
NEARDATA boolean vision_full_recalc = 0;
NEARDATA char    **viz_array = 0;/* used in cansee() and couldsee() macros */

/* Global windowing data, defined here for multi-window-system support */
NEARDATA winid WIN_MESSAGE = WIN_ERR, WIN_STATUS = WIN_ERR;
NEARDATA winid WIN_MAP = WIN_ERR, WIN_INVEN = WIN_ERR;
char toplines[TBUFSZ];
/* Windowing stuff that's really tty oriented, but present for all ports */
struct tc_gbl_data tc_gbl_data = { 0, 0, 0, 0 };  /* AS,AE, LI,CO */

char *fqn_prefix[PREFIX_COUNT] = { (char *)0, (char *)0, (char *)0, (char *)0,
                                   (char *)0, (char *)0, (char *)0, (char *)0, (char *)0 };

#ifdef PREFIXES_IN_USE
char *fqn_prefix_names[PREFIX_COUNT] = { "hackdir", "leveldir", "savedir",
                                         "bonesdir", "datadir", "scoredir",
                                         "lockdir", "configdir", "troubledir" };
#endif

#ifdef RECORD_ACHIEVE
struct u_achieve achieve = DUMMY;
#endif

#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)
struct u_realtime urealtime = { 0 };
#endif

struct _plinemsg *pline_msg = NULL;

/* FIXME: The curses windowport requires this stupid hack, in the
   case where a game is in progress and the user is asked if he
   wants to destroy old game.
   Without this, curses tries to show the yn() question with pline()
   ...but the message window isn't up yet.
 */
boolean curses_stupid_hack = 1;

/* dummy routine used to force linkage */
void
decl_init()
{
    return;
}

/*decl.c*/
