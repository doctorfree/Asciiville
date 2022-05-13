/*  SCCS Id: @(#)options.c  3.4 2003/11/14  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef OPTION_LISTS_ONLY    /* (AMIGA) external program for opt lists */
#include "config.h"
#include "objclass.h"
#include "flag.h"
NEARDATA struct flag flags; /* provide linkage */
NEARDATA struct instance_flags iflags;  /* provide linkage */
#define static
#else
#include "hack.h"
#include "tcap.h"
#include <ctype.h>
#endif
#include <errno.h>

#define WINTYPELEN 16

#ifdef DEFAULT_WC_TILED_MAP
#define PREFER_TILED TRUE
#else
#define PREFER_TILED FALSE
#endif

static boolean illegal_menu_cmd_key(CHAR_P);

#ifdef CURSES_GRAPHICS
extern int curses_read_attrs(char *attrs);
#endif

#define config_error_add raw_printf

enum { optn_silenterr = -1, optn_err = 0, optn_ok = 1 };

#ifdef REALTIME_ON_BOTL
static const char* realtime_type_strings[] = {
    "disabled",
    "play time",
    "wallclock time",
};
static const char* realtime_format_strings[] = {
    "seconds",
    "condensed",
    "units",
};
#endif

/*
 *  NOTE:  If you add (or delete) an option, please update the short
 *  options help (option_help()), the long options help (dat/opthelp),
 *  and the current options setting display function (doset()),
 *  and also the Guidebooks.
 *
 *  The order matters.  If an option is a an initial substring of another
 *  option (e.g. time and timed_delay) the shorter one must come first.
 */

static struct Bool_Opt
{
    const char *name;
    boolean *addr, initvalue;
    int optflags;
} boolopt[] = {
    { "acoustics", &flags.acoustics, TRUE, SET_IN_FILE },
#ifdef AMIGA
    {"altmeta", &flags.altmeta, TRUE, DISP_IN_GAME},
#else
    {"altmeta", (boolean *)0, TRUE, DISP_IN_GAME},
#endif
    {"ascii_map",     &iflags.wc_ascii_map, !PREFER_TILED, SET_IN_GAME},    /*WC*/
#ifdef MFLOPPY
    {"asksavedisk", &flags.asksavedisk, FALSE, SET_IN_GAME},
#else
    {"asksavedisk", (boolean *)0, FALSE, SET_IN_FILE},
#endif
    {"autodescribe", &iflags.autodescribe, TRUE, SET_IN_FILE },
    {"autodig", &flags.autodig, FALSE, SET_IN_GAME},
#ifdef AUTO_OPEN
    {"autoopen", &iflags.autoopen, TRUE, SET_IN_GAME},
#endif /* AUTO_OPEN */
    {"autopickup", &flags.pickup, TRUE, SET_IN_GAME},
    {"autoquiver", &flags.autoquiver, FALSE, SET_IN_GAME},
    {"autounlock", &flags.autounlock, TRUE, SET_IN_GAME},
#if defined(MICRO) && !defined(AMIGA)
    {"BIOS", &iflags.BIOS, FALSE, SET_IN_FILE},
#else
    {"BIOS", (boolean *)0, FALSE, SET_IN_FILE},
#endif
    {"bones", &flags.bones, TRUE, SET_IN_FILE},
#ifdef INSURANCE
    {"checkpoint", &flags.ins_chkpt, TRUE, SET_IN_FILE},
#else
    {"checkpoint", (boolean *)0, FALSE, SET_IN_FILE},
#endif
#ifdef MFLOPPY
    {"checkspace", &iflags.checkspace, TRUE, SET_IN_GAME},
#else
    {"checkspace", (boolean *)0, FALSE, SET_IN_FILE},
#endif
#ifdef CURSES_GRAPHICS
    {"classic_status", &iflags.classic_status, TRUE, SET_IN_FILE},
#endif
    {"clicklook", &iflags.clicklook, FALSE, SET_IN_FILE },
    {"cmdassist", &iflags.cmdassist, TRUE, SET_IN_GAME},
    {"color", &iflags.wc_color, TRUE, SET_IN_GAME},     /*WC*/
    {"confirm", &flags.confirm, TRUE, SET_IN_GAME},
#ifdef CURSES_GRAPHICS
    {"cursesgraphics", &iflags.cursesgraphics, TRUE, SET_IN_GAME},
#else
    {"cursesgraphics", (boolean *)0, FALSE, SET_IN_FILE},
#endif
    {"dark_room", &iflags.dark_room, TRUE, SET_IN_FILE},
#ifdef DEBUG_FUZZER
    {"debug_fuzzer", &iflags.debug_fuzzer, FALSE, SET_IN_FILE},
#endif
    {"deathdropless", &flags.deathdropless, FALSE, SET_IN_FILE},
#if defined(TERMLIB) && !defined(MAC_GRAPHICS_ENV)
    {"DECgraphics", &iflags.DECgraphics, FALSE, SET_IN_GAME},
#else
    {"DECgraphics", (boolean *)0, FALSE, SET_IN_FILE},
#endif
#ifdef ELBERETH
    {"elberethignore", &flags.elberethignore, FALSE, SET_IN_FILE},
#endif
    {"eight_bit_tty", &iflags.wc_eight_bit_input, FALSE, SET_IN_FILE},  /*WC*/
#if defined(TTY_GRAPHICS) || defined(CURSES_GRAPHICS) || defined(X11_GRAPHICS)
    {"extmenu", &iflags.extmenu, FALSE, SET_IN_GAME},
#else
    {"extmenu", (boolean *)0, FALSE, SET_IN_FILE},
#endif
#ifdef OPT_DISPMAP
    {"fast_map", &flags.fast_map, TRUE, SET_IN_GAME},
#else
    {"fast_map", (boolean *)0, TRUE, SET_IN_FILE},
#endif
    {"female", &flags.female, FALSE, DISP_IN_GAME},
    {"fixinv", &flags.invlet_constant, TRUE, SET_IN_GAME},
#ifdef AMIFLUSH
    {"flush", &flags.amiflush, FALSE, SET_IN_GAME},
#else
    {"flush", (boolean *)0, FALSE, SET_IN_FILE},
#endif
    {"force_invmenu", &iflags.force_invmenu, FALSE, SET_IN_GAME },
    {"fullscreen", &iflags.wc2_fullscreen, FALSE, SET_IN_FILE},
    {"goldX", &flags.goldX, FALSE, SET_IN_FILE },
    {"guicolor", &iflags.wc2_guicolor, TRUE, SET_IN_GAME},
    {"heaven_or_hell", &flags.heaven_or_hell, FALSE, SET_IN_FILE},
    {"hell_and_hell", &flags.hell_and_hell, FALSE, SET_IN_FILE},
    {"help", &flags.help, TRUE, SET_IN_GAME},
    {"hilite_pet",    &iflags.wc_hilite_pet, TRUE, SET_IN_GAME},    /*WC*/
    {"hint", &flags.hint, TRUE, SET_IN_FILE},
    {"hitpointbar", &flags.hitpointbar, TRUE, SET_IN_GAME},
    {"hp_notify", &iflags.hp_notify, FALSE, SET_IN_GAME},
#ifdef ASCIIGRAPH
    {"IBMgraphics", &iflags.IBMgraphics, FALSE, SET_IN_GAME},
#else
    {"IBMgraphics", (boolean *)0, FALSE, SET_IN_FILE},
#endif
#ifndef MAC
    {"ignintr", &flags.ignintr, FALSE, SET_IN_GAME},
#else
    {"ignintr", (boolean *)0, FALSE, SET_IN_FILE},
#endif
#ifdef SHOW_WEIGHT
    {"invweight", &flags.invweight, FALSE, SET_IN_GAME},
#else
    {"invweight", (boolean *)0, FALSE, SET_IN_FILE},
#endif
    {"large_font", &iflags.obsolete, FALSE, SET_IN_FILE},   /* OBSOLETE */
    {"legacy", &flags.legacy, TRUE, SET_IN_FILE},
    {"lit_corridor", &flags.lit_corridor, TRUE, SET_IN_FILE},
    {"lootabc", &iflags.lootabc, FALSE, SET_IN_GAME},
#ifdef MAC_GRAPHICS_ENV
    {"Macgraphics", &iflags.MACgraphics, TRUE, SET_IN_GAME},
#else
    {"Macgraphics", (boolean *)0, FALSE, SET_IN_FILE},
#endif
#ifdef MAIL
    {"mail", &flags.biff, TRUE, SET_IN_GAME},
#else
    {"mail", (boolean *)0, TRUE, SET_IN_FILE},
#endif
    {"marathon", &flags.marathon, FALSE, DISP_IN_GAME },
    {"mention_walls", &iflags.mention_walls, FALSE, SET_IN_GAME },
    {"menucolors", &iflags.use_menu_color, TRUE,  SET_IN_FILE},
#ifdef WIZARD
    /* for menu debugging only*/
    {"menu_tab_sep", &iflags.menu_tab_sep, FALSE, SET_IN_GAME},
#else
    {"menu_tab_sep", (boolean *)0, FALSE, SET_IN_FILE},
#endif
#ifdef CURSES_GRAPHICS
    {"mouse_support", &iflags.wc_mouse_support, FALSE, DISP_IN_GAME},   /*WC*/
#else
    {"mouse_support", &iflags.wc_mouse_support, TRUE, DISP_IN_GAME},    /*WC*/
#endif
    {"newcolors", &iflags.wc2_newcolors, TRUE, SET_IN_FILE},
#ifdef NEWS
    {"news", &iflags.news, TRUE, DISP_IN_GAME},
#else
    {"news", (boolean *)0, FALSE, SET_IN_FILE},
#endif
    {"conducts_ascet", &flags.ascet, FALSE, SET_IN_FILE },
    {"conducts_atheist", &flags.atheist, FALSE, SET_IN_FILE },
    {"conducts_blindfolded", &flags.blindfolded, FALSE, SET_IN_FILE },
    {"conducts_illiterate", &flags.illiterate, FALSE, SET_IN_FILE },
    {"conducts_pacifist", &flags.pacifist, FALSE, SET_IN_FILE },
    {"conducts_nudist", &flags.nudist, FALSE, SET_IN_FILE },
    {"conducts_vegan", &flags.vegan, FALSE, SET_IN_FILE },
    {"conducts_vegetarian", &flags.vegetarian, FALSE, SET_IN_FILE },
    {"null", &flags.null, TRUE, SET_IN_FILE},
#ifdef MAC
    {"page_wait", &flags.page_wait, TRUE, SET_IN_GAME},
#else
    {"page_wait", (boolean *)0, FALSE, SET_IN_FILE},
#endif
#ifdef PARANOID
    {"paranoid_hit", &iflags.paranoid_hit, TRUE, SET_IN_FILE},
    {"paranoid_quit", &iflags.paranoid_quit, TRUE, SET_IN_FILE},
    {"paranoid_remove", &iflags.paranoid_remove, FALSE, SET_IN_FILE},
    {"paranoid_trap", &iflags.paranoid_trap, FALSE, SET_IN_FILE},
    {"paranoid_lava", &iflags.paranoid_lava, TRUE, SET_IN_FILE},
    {"paranoid_water", &iflags.paranoid_water, TRUE, SET_IN_FILE},
#endif
    {"perm_invent", &iflags.perm_invent, TRUE, SET_IN_GAME},
    {"perma_hallu", &flags.perma_hallu, FALSE, SET_IN_FILE},
    {"pickup_dropped", &flags.pickup_dropped, FALSE, SET_IN_GAME},
    {"pickup_thrown", &flags.pickup_thrown, TRUE, SET_IN_GAME},
#ifdef CURSES_GRAPHICS
    {"popup_dialog",  &iflags.wc_popup_dialog, TRUE, SET_IN_GAME},  /*WC*/
#else
    {"popup_dialog",  &iflags.wc_popup_dialog, FALSE, SET_IN_GAME}, /*WC*/
#endif  /* CURSES_GRAPHICS */
    {"prayconfirm", &flags.prayconfirm, TRUE, SET_IN_GAME},
    {"preload_tiles", &iflags.wc_preload_tiles, TRUE, DISP_IN_GAME},    /*WC*/
    {"pushweapon", &flags.pushweapon, FALSE, SET_IN_FILE},
    {"quiver_fired", (boolean *)0, TRUE, SET_IN_FILE},
#ifdef QWERTZ
    {"qwertz_layout", &iflags.qwertz_layout, FALSE, SET_IN_GAME},
#endif
#if defined(MICRO) && !defined(AMIGA)
    {"rawio", &iflags.rawio, FALSE, DISP_IN_GAME},
#else
    {"rawio", (boolean *)0, FALSE, SET_IN_FILE},
#endif
    {"rest_on_space", &iflags.rest_on_space, FALSE, SET_IN_FILE},
    {"safe_pet", &flags.safe_dog, TRUE, SET_IN_GAME},
#ifdef WIZARD
    {"sanity_check", &iflags.sanity_check, FALSE, SET_IN_GAME},
#else
    {"sanity_check", (boolean *)0, FALSE, SET_IN_FILE},
#endif
    {"showannotation", &iflags.show_annotation, TRUE, SET_IN_FILE},
#ifdef SHOW_BORN
    {"showborn", &iflags.show_born, TRUE, SET_IN_GAME},
#endif
    {"showbuc", &iflags.show_buc, TRUE, SET_IN_GAME},
    {"showdmg", &iflags.showdmg, FALSE, SET_IN_GAME},
    {"show_dgn_name", &iflags.show_dgn_name, TRUE, SET_IN_GAME},
#ifdef EXP_ON_BOTL
    {"showexp", &flags.showexp, FALSE, SET_IN_GAME},
#else
    {"showexp", (boolean *)0, FALSE, SET_IN_FILE},
#endif
    {"showrace", &iflags.showrace, FALSE, SET_IN_GAME},
#ifdef SCORE_ON_BOTL
    {"showscore", &flags.showscore, FALSE, SET_IN_GAME},
#else
    {"showscore", (boolean *)0, FALSE, SET_IN_FILE},
#endif
#ifdef SHOW_WEIGHT
    {"showweight", &flags.showweight, FALSE, SET_IN_GAME},
#else
    {"showweight", (boolean *)0, FALSE, SET_IN_FILE},
#endif
    {"silent", &flags.silent, TRUE, SET_IN_GAME},
    {"softkeyboard", &iflags.wc2_softkeyboard, FALSE, SET_IN_FILE},
    {"sortpack", &flags.sortpack, TRUE, SET_IN_GAME},
    {"sound", &flags.soundok, TRUE, SET_IN_GAME},
    {"sparkle", &flags.sparkle, TRUE, SET_IN_GAME},
    /* not removed for backwards compatibilty */
    {"standout", &flags.standout, TRUE, SET_IN_FILE},
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
    {"statuscolors", &iflags.use_status_colors, TRUE, SET_IN_GAME},
#else
    {"statuscolors", (boolean *)0, TRUE, SET_IN_GAME},
#endif
    {"splash_screen",     &iflags.wc_splash_screen, TRUE, DISP_IN_GAME},    /*WC*/
    {"tiled_map",     &iflags.wc_tiled_map, PREFER_TILED, DISP_IN_GAME},    /*WC*/
    {"time", &flags.time, TRUE, SET_IN_GAME},
#ifdef TIMED_DELAY
    {"timed_delay", &flags.nap, TRUE, SET_IN_GAME},
#else
    {"timed_delay", (boolean *)0, FALSE, SET_IN_GAME},
#endif
    {"tombstone", &flags.tombstone, TRUE, SET_IN_GAME},
    {"toptenwin", &flags.toptenwin, FALSE, SET_IN_GAME},
    {"travel", &iflags.travelcmd, TRUE, SET_IN_GAME},
#ifdef UTF8_GLYPHS
    {"UTF8graphics", &iflags.UTF8graphics, FALSE, SET_IN_GAME},
#else
    {"UTF8graphics", (boolean *)0, FALSE, SET_IN_FILE},
#endif
    {"use_inverse",   &iflags.wc_inverse, TRUE, SET_IN_GAME},       /*WC*/
#ifdef WIN_EDGE
    {"win_edge", &iflags.win_edge, FALSE, SET_IN_GAME},
#else
    {"win_edge", (boolean *)0, TRUE, SET_IN_FILE},
#endif
    {"wound_message", &flags.wounds, TRUE, SET_IN_GAME},
    {"vanilla_ui_behavior", &iflags.vanilla_ui_behavior, FALSE, SET_IN_FILE},
    {"verbose", &flags.verbose, TRUE, SET_IN_GAME},
#ifdef USE_TILES
    {"vt_tiledata", &iflags.vt_nethack, FALSE, SET_IN_GAME},
#else
    {"vt_tiledata", (boolean *)0, FALSE, SET_IN_FILE},
#endif
    { "whatis_menu", &iflags.getloc_usemenu, TRUE, SET_IN_GAME },
    { "whatis_moveskip", &iflags.getloc_moveskip, FALSE, SET_IN_GAME },
    { "wraptext", &iflags.wc2_wraptext, FALSE, SET_IN_GAME },
    {(char *)0, (boolean *)0, FALSE, 0}
};

/* compound options, for option_help() and external programs like Amiga
 * frontend */
static struct Comp_Opt
{
    const char *name, *descr;
    int size;   /* for frontends and such allocating space --
                 * usually allowed size of data in game, but
                 * occasionally maximum reasonable size for
                 * typing when game maintains information in
                 * a different format */
    int optflags;
} compopt[] = {
    { "align",    "your starting alignment (lawful, neutral, or chaotic)",
      8, DISP_IN_GAME },
    { "align_message", "message window alignment", 20, DISP_IN_GAME },  /*WC*/
    { "align_status", "status window alignment", 20, DISP_IN_GAME },    /*WC*/
    { "altkeyhandler", "alternate key handler", 20, DISP_IN_GAME },
    { "boulder",  "the symbol to use for displaying boulders",
      1, SET_IN_GAME },
    { "catname",  "the name of your (first) cat (e.g., catname:Tabby)",
      PL_PSIZ, DISP_IN_GAME },
    { "conducts", "the kind of conducts you want to adhere to",
      1,                   /* not needed */
      DISP_IN_GAME },
#ifdef EXOTIC_PETS
    { "crocodilename", "the name of your (first) crocodile (e.g., crocodilename:TickTock)",
      PL_PSIZ, DISP_IN_GAME },
#endif
    { "disclose", "the kinds of information to disclose at end of game",
      sizeof(flags.end_disclose) * 2,
      SET_IN_GAME },
    { "dogname",  "the name of your (first) dog (e.g., dogname:Fang)",
      PL_PSIZ, DISP_IN_GAME },
#ifdef DUMP_LOG
    { "dumpfile", "where to dump data (e.g., dumpfile:/tmp/dump.nh)",
#ifdef DUMP_FN
      PL_PSIZ, DISP_IN_GAME },
#else
      PL_PSIZ, SET_IN_GAME },
#endif
#endif
    { "dungeon",  "the symbols to use in drawing the dungeon map",
      MAXDCHARS+1, SET_IN_FILE },
    { "effects",  "the symbols to use in drawing special effects",
      MAXECHARS+1, SET_IN_FILE },
    { "font_map", "the font to use in the map window", 40, DISP_IN_GAME },  /*WC*/
    { "font_menu", "the font to use in menus", 40, DISP_IN_GAME },      /*WC*/
    { "font_message", "the font to use in the message window",
      40, DISP_IN_GAME },                       /*WC*/
    { "font_size_map", "the size of the map font", 20, DISP_IN_GAME },  /*WC*/
    { "font_size_menu", "the size of the menu font", 20, DISP_IN_GAME },    /*WC*/
    { "font_size_message", "the size of the message font", 20, DISP_IN_GAME },  /*WC*/
    { "font_size_status", "the size of the status font", 20, DISP_IN_GAME },    /*WC*/
    { "font_size_text", "the size of the text font", 20, DISP_IN_GAME },    /*WC*/
    { "font_status", "the font to use in status window", 40, DISP_IN_GAME }, /*WC*/
    { "font_text", "the font to use in text windows", 40, DISP_IN_GAME },   /*WC*/
    { "fruit",    "the name of a fruit you enjoy eating",
      PL_FSIZ, SET_IN_GAME },
    { "gender",   "your starting gender (male or female)",
      8, DISP_IN_GAME },
    { "horsename", "the name of your (first) horse (e.g., horsename:Silver)",
      PL_PSIZ, DISP_IN_GAME },
    { "hp_notify_fmt", "hp_notify format string", 20, SET_IN_GAME },
    { "map_mode", "map display mode under Windows", 20, DISP_IN_GAME }, /*WC*/
    { "menucolor", "set menu colors", PL_PSIZ, SET_IN_FILE },
    { "menustyle", "user interface for object selection",
      MENUTYPELEN, SET_IN_GAME },
    { "menu_deselect_all", "deselect all items in a menu", 4, SET_IN_FILE },
    { "menu_deselect_page", "deselect all items on this page of a menu",
      4, SET_IN_FILE },
    { "menu_first_page", "jump to the first page in a menu",
      4, SET_IN_FILE },
    { "menu_headings", "bold, inverse, or underline headings", 9, SET_IN_GAME },
    { "menu_invert_all", "invert all items in a menu", 4, SET_IN_FILE },
    { "menu_invert_page", "invert all items on this page of a menu",
      4, SET_IN_FILE },
    { "menu_last_page", "jump to the last page in a menu", 4, SET_IN_FILE },
    { "menu_next_page", "goto the next menu page", 4, SET_IN_FILE },
    { "menu_previous_page", "goto the previous menu page", 4, SET_IN_FILE },
    { "menu_search", "search for a menu item", 4, SET_IN_FILE },
    { "menu_select_all", "select all items in a menu", 4, SET_IN_FILE },
    { "menu_select_page", "select all items on this page of a menu",
      4, SET_IN_FILE },
#ifdef EXOTIC_PETS
    { "monkeyname", "the name of your (first) monkey (e.g., monkeyname:Bonzo)",
      PL_PSIZ, DISP_IN_GAME },
#endif
    { "monsters", "the symbols to use for monsters",
      MAXMCLASSES, SET_IN_FILE },
    { "msghistory", "number of top line messages to save",
      5, DISP_IN_GAME },
# ifdef TTY_GRAPHICS
    {"msg_window", "the type of message window required", 1, SET_IN_GAME},
# else
    {"msg_window", "the type of message window required", 1, SET_IN_FILE},
# endif
    { "name",     "your character's name (e.g., name:Merlin-W)",
      PL_NSIZ, DISP_IN_GAME },
    { "nameempty",  "the automatic name given to known empty wands (e.g., nameempty:empty)",
      20, DISP_IN_GAME },
    { "number_pad", "use the number pad", 1, SET_IN_GAME},
    { "objects",  "the symbols to use for objects",
      MAXOCLASSES, SET_IN_FILE },
    { "packorder", "the inventory order of the items in your pack",
      MAXOCLASSES, SET_IN_GAME },
#ifdef CHANGE_COLOR
    { "palette",  "palette (00c/880/-fff is blue/yellow/reverse white)",
      15, SET_IN_GAME },
# if defined(MAC)
    { "hicolor",  "same as palette, only order is reversed",
      15, SET_IN_FILE },
# endif
#endif
    { "petattr",  "attributes for highlighting pets", 12, SET_IN_FILE },
#ifdef PARANOID
    { "paranoid", "the kind of actions you want to be paranoid about",
      1,                   /* not needed */
      SET_IN_GAME },
#endif
    { "pettype",  "your preferred initial pet type", 4, DISP_IN_GAME },
    { "pickup_burden",  "maximum burden picked up before prompt",
      20, SET_IN_GAME },
    { "pickup_types", "types of objects to pick up automatically",
      MAXOCLASSES, SET_IN_GAME },
    { "pilesize", "maximum number of items on floor to list automatically",
      20, SET_IN_GAME },
    { "player_selection", "choose character via dialog or prompts",
      12, DISP_IN_GAME },
    { "race",     "your starting race (e.g., Human, Elf)",
      PL_CSIZ, DISP_IN_GAME },
#ifdef CONVICT
    { "ratname",  "the name of your (first) rat (e.g., ratname:Squeak)",
      PL_PSIZ, DISP_IN_GAME },
#endif /* CONVICT */
    { "role",     "your starting role (e.g., Barbarian, Valkyrie)",
      PL_CSIZ, DISP_IN_GAME },
#ifdef REALTIME_ON_BOTL
    {"realtime", "show realtime in status line", 1, SET_IN_GAME},
    {"realtime_format", "format of realtime in status line", 1, SET_IN_GAME},
#endif
    { "runmode", "display frequency when `running' or `travelling'",
      sizeof "teleport", SET_IN_GAME },
    { "scores",   "the parts of the score list you wish to see",
      32, SET_IN_GAME },
    { "scroll_amount", "amount to scroll map when scroll_margin is reached",
      20, DISP_IN_GAME },                   /*WC*/
    { "scroll_margin", "scroll map when this far from the edge", 20, DISP_IN_GAME }, /*WC*/
    { "seed", "game seed for repeatable dungeon layout",
      20, DISP_IN_GAME },
#ifdef SORTLOOT
    { "sortloot", "sort object selection lists by description", 4, SET_IN_GAME },
#endif
#ifdef MSDOS
    { "soundcard", "type of sound card to use", 20, SET_IN_FILE },
#endif
    { "statuscolor", "set status colors", PL_PSIZ, SET_IN_FILE },
#ifdef TTY_GRAPHICS
    { "statuslines", "set number of status lines (2 or 3)", 20, SET_IN_GAME },
#endif
    { "suppress_alert", "suppress alerts about version-specific features",
      8, SET_IN_FILE },
    { "tile_width", "width of tiles", 20, DISP_IN_GAME},    /*WC*/
    { "tile_height", "height of tiles", 20, DISP_IN_GAME},  /*WC*/
    { "tile_file", "name of tile file", 70, DISP_IN_GAME},  /*WC*/
    { "traps",    "the symbols to use in drawing traps",
      MAXTCHARS+1, SET_IN_FILE },
    { "vary_msgcount", "show more old messages at a time", 20, DISP_IN_GAME }, /*WC*/
#ifdef MSDOS
    { "video",    "method of video updating", 20, SET_IN_FILE },
#endif
#ifdef VIDEOSHADES
    { "videocolors", "color mappings for internal screen routines",
      40, DISP_IN_GAME },
#ifdef MSDOS
    { "videoshades", "gray shades to map to black/gray/white",
      32, DISP_IN_GAME },
#endif
#endif
#ifdef WIN32CON
    {"subkeyvalue", "override keystroke value", 7, SET_IN_FILE},
#endif
    { "whatis_coord", "show coordinates when auto-describing cursor position", 1, SET_IN_GAME },
    { "whatis_filter", "filter coordinate locations when targeting", 1, SET_IN_GAME },
    { "windowcolors",  "the foreground/background colors of windows",   /*WC*/
      80, DISP_IN_GAME },
    { "windowtype", "windowing system to use", WINTYPELEN, DISP_IN_GAME },
#ifdef EXOTIC_PETS
    { "wolfname", "the name of your (first) wolf (e.g., wolfname:Gnasher)",
      PL_PSIZ, DISP_IN_GAME },
#endif
#ifdef TTY_GRAPHICS
    { "truecolor_separator", "the character separator for truecolor escape sequences",
      2, SET_IN_FILE },
#endif
    { (char *)0, (char *)0, 0, 0 }
};

#ifdef OPTION_LISTS_ONLY
#undef static

#else   /* use rest of file */

static boolean need_redraw; /* for doset() */

#if defined(TOS) && defined(TEXTCOLOR)
extern boolean colors_changed;  /* in tos.c */
#endif

#ifdef VIDEOSHADES
extern char *shade[3];        /* in sys/msdos/video.c */
extern int ttycolors[CLR_MAX];    /* in sys/msdos/video.c, win/tty/termcap.c */
#endif

static char def_inv_order[MAXOCLASSES] = {
    COIN_CLASS, AMULET_CLASS, WEAPON_CLASS, ARMOR_CLASS, FOOD_CLASS,
    SCROLL_CLASS, SPBOOK_CLASS, POTION_CLASS, RING_CLASS, WAND_CLASS,
    TOOL_CLASS, GEM_CLASS, ROCK_CLASS, BALL_CLASS, CHAIN_CLASS, 0,
};

/*
 * Default menu manipulation command accelerators.  These may _not_ be:
 *
 *  + a number - reserved for counts
 *  + an upper or lower case US ASCII letter - used for accelerators
 *  + ESC - reserved for escaping the menu
 *  + NULL, CR or LF - reserved for commiting the selection(s).  NULL
 *    is kind of odd, but the tty's xwaitforspace() will return it if
 *    someone hits a <ret>.
 *  + a default object class symbol - used for object class accelerators
 *
 * Standard letters (for now) are:
 *
 *      <  back 1 page
 *      >  forward 1 page
 *      ^  first page
 *      |  last page
 *      :  search
 *
 *      page        all
 *       ,    select     .
 *       \    deselect   -
 *       ~    invert     @
 *
 * The command name list is duplicated in the compopt array.
 */
typedef struct {
    const char *name;
    char cmd;
    const char *desc;
} menu_cmd_t;

#define NUM_MENU_CMDS 11
static const menu_cmd_t default_menu_cmd_info[NUM_MENU_CMDS] = {
 { "menu_first_page",    MENU_FIRST_PAGE,    "Go to first page" },
 { "menu_last_page",     MENU_LAST_PAGE,     "Go to last page" },
 { "menu_next_page",     MENU_NEXT_PAGE,     "Go to next page" },
 { "menu_previous_page", MENU_PREVIOUS_PAGE, "Go to previous page" },
 { "menu_select_all",    MENU_SELECT_ALL,    "Select all items" },
 { "menu_deselect_all",  MENU_UNSELECT_ALL,  "Unselect all items" },
 { "menu_invert_all",    MENU_INVERT_ALL,    "Invert selection" },
 { "menu_select_page",   MENU_SELECT_PAGE,   "Select items in current page" },
 { "menu_deselect_page", MENU_UNSELECT_PAGE, "Unselect items in current page" },
 { "menu_invert_page",   MENU_INVERT_PAGE,   "Invert current page selection" },
 { "menu_search",        MENU_SEARCH,        "Search and toggle matching items" },
};

/*
 * Allow the user to map incoming characters to various menu commands.
 * The accelerator list must be a valid C string.
 */
#define MAX_MENU_MAPPED_CMDS 32 /* some number */
char mapped_menu_cmds[MAX_MENU_MAPPED_CMDS+1]; /* exported */
static char mapped_menu_op[MAX_MENU_MAPPED_CMDS+1];
static short n_menu_mapped = 0;


static boolean initial, from_file;

STATIC_DCL void FDECL(doset_add_menu, (winid, const char *, int));
STATIC_DCL void FDECL(nmcpy, (char *, const char *, int));
STATIC_DCL void FDECL(escapes, (const char *, char *));
STATIC_DCL void FDECL(rejectoption, (const char *));
STATIC_DCL void FDECL(badoption, (const char *));
STATIC_DCL char *FDECL(string_for_opt, (char *, BOOLEAN_P));
STATIC_DCL char *FDECL(string_for_env_opt, (const char *, char *, BOOLEAN_P));
STATIC_DCL void FDECL(bad_negation, (const char *, BOOLEAN_P));
STATIC_DCL int FDECL(change_inv_order, (char *));
STATIC_DCL void FDECL(oc_to_str, (char *, char *));
STATIC_DCL void FDECL(graphics_opts, (char *, const char *, int, int));
STATIC_DCL int FDECL(feature_alert_opts, (char *, const char *));
STATIC_DCL const char *FDECL(get_compopt_value, (const char *, char *));
STATIC_DCL boolean FDECL(special_handling, (const char *, BOOLEAN_P, BOOLEAN_P));
STATIC_DCL void FDECL(warning_opts, (char *, const char *));
STATIC_DCL void FDECL(duplicate_opt_detection, (const char *, int));

STATIC_OVL void FDECL(wc_set_font_name, (int, char *));
STATIC_OVL int FDECL(wc_set_window_colors, (char *));
STATIC_OVL boolean FDECL(is_wc_option, (const char *));
STATIC_OVL boolean FDECL(wc_supported, (const char *));
STATIC_OVL boolean FDECL(is_wc2_option, (const char *));
STATIC_OVL boolean FDECL(wc2_supported, (const char *));

static int FDECL(handle_add_list_remove, (const char *, int));

static const char *FDECL(attr2attrname, (int));
static boolean FDECL(test_regex_pattern, (const char *, const char *));

/* menu coloring */
extern struct menucoloring *menu_colorings;
static boolean FDECL(add_menu_coloring_parsed, (char *, int, int));
static void FDECL(free_one_menu_coloring, (int));
static int NDECL(count_menucolors);

#ifdef AUTOPICKUP_EXCEPTIONS
static int NDECL(handler_autopickup_exception);
static void FDECL(remove_autopickup_exception, (struct autopickup_exception *));
static int NDECL(count_apes);
#endif

#ifdef AUTOPICKUP_EXCEPTIONS
static int
handler_autopickup_exception()
{
    winid tmpwin;
    anything any;
    int i;
    int opt_idx, numapes = 0;
    char apebuf[2 + BUFSZ]; /* so &apebuf[1] is BUFSZ long for getlin() */
    struct autopickup_exception *ape;

 ape_again:
    numapes = count_apes();
    opt_idx = handle_add_list_remove("autopickup exception", numapes);
    if (opt_idx == 3) {
        /* done */
        return TRUE;
    } else if (opt_idx == 0) { /* add new */
        /* EDIT_GETLIN:  assume user doesn't user want previous
           exception used as default input string for this one... */
        apebuf[0] = apebuf[1] = '\0';
        getlin("What new autopickup exception pattern?", &apebuf[1]);
        mungspaces(&apebuf[1]); /* regularize whitespace */
        if (apebuf[1] == '\033') {
            return TRUE;
        }
        if (apebuf[1]) {
            apebuf[0] = '\"';
            /* guarantee room for \" prefix and \"\0 suffix;
               -2 is good enough for apebuf[] but -3 makes
               sure the whole thing fits within normal BUFSZ */
            apebuf[sizeof apebuf - 2] = '\0';
            Strcat(apebuf, "\"");
            add_autopickup_exception(apebuf);
        }
        goto ape_again;
    } else { /* list (1) or remove (2) */
        int pick_idx, pick_cnt;
        menu_item *pick_list = (menu_item *) 0;

        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        if (numapes) {
            ape = apelist;
            any = zeroany;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings,
                     "Always pickup '<'; never pickup '>'",
                     MENU_ITEMFLAGS_NONE);
            for (i = 0; i < numapes && ape; i++) {
                any.a_void = (opt_idx == 1) ? 0 : ape;
                /* length of pattern plus quotes (plus '<'/'>') is
                   less than BUFSZ */
                Sprintf(apebuf, "\"%c%s\"", ape->grab ? '<' : '>', ape->pattern);
                add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, apebuf, MENU_ITEMFLAGS_NONE);
                ape = ape->next;
            }
        }
        Sprintf(apebuf, "%s autopickup exceptions",
                (opt_idx == 1) ? "List of" : "Remove which");
        end_menu(tmpwin, apebuf);
        pick_cnt = select_menu(tmpwin,
                               (opt_idx == 1) ? PICK_NONE : PICK_ANY,
                               &pick_list);
        if (pick_cnt > 0) {
            for (pick_idx = 0; pick_idx < pick_cnt; ++pick_idx) {
                remove_autopickup_exception((struct autopickup_exception *) pick_list[pick_idx].item.a_void);
            }
            free(pick_list), pick_list = (menu_item *) 0;
        }
        destroy_nhwindow(tmpwin);
        if (pick_cnt >= 0) {
            goto ape_again;
        }
    }
    return optn_ok;
}
#endif /* AUTOPICKUP_EXCEPTIONS */

#ifdef MENU_COLOR
static int
handler_menu_colors(void)
{
    winid tmpwin;
    anything any;
    char buf[BUFSZ];
    int opt_idx, nmc, mcclr, mcattr;
    char mcbuf[BUFSZ];

menucolors_again:
    nmc = count_menucolors();
    opt_idx = handle_add_list_remove("menucolor", nmc);
    if (opt_idx == 3) {
        /* done */

menucolors_done:
        /* in case we've made a change which impacts current persistent
           inventory window; we don't track whether an actual changed
           occurred, so just assume there was one and that it matters;
           if we're wrong, a redundant update is cheap... */
        if (nmc > 0) {
            iflags.use_menu_color = TRUE;
        }
        update_inventory();

        return optn_ok;

    } else if (opt_idx == 0) { /* add new */
        mcbuf[0] = '\0';
        getlin("What new menucolor pattern?", mcbuf);
        if (*mcbuf == '\033') {
            goto menucolors_done;
        }
        if (*mcbuf &&
             test_regex_pattern(mcbuf, "MENUCOLORS regex") &&
             (mcclr = query_color((char *) 0)) != -1 &&
             (mcattr = query_attr((char *) 0)) != -1 &&
             !add_menu_coloring_parsed(mcbuf, mcclr, mcattr)) {
            pline("Error adding the menu color.");
            wait_synch();
        }
        goto menucolors_again;

    } else {
        /* list (1) or remove (2) */
        int pick_idx, pick_cnt;
        int mc_idx;
        unsigned ln;
        const char *sattr, *sclr;
        menu_item *pick_list = (menu_item *) 0;
        struct menucoloring *tmp = menu_colorings;
        char clrbuf[QBUFSZ];

        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        any = zeroany;
        mc_idx = 0;
        while (tmp) {
            sattr = attr2attrname(tmp->attr);
            sclr = strcpy(clrbuf, clr2colorname(tmp->color));
            (void) strNsubst(clrbuf, " ", "-", 0);
            any.a_int = ++mc_idx;
            /* construct suffix */
            Sprintf(buf, "\"\"=%s%s%s", sclr,
                    (tmp->attr != ATR_NONE && tmp->attr != ATR_UNDEFINED) ? "&" : "",
                    (tmp->attr != ATR_NONE && tmp->attr != ATR_UNDEFINED) ? sattr : "");
            /* now main string */
            ln = sizeof buf - strlen(buf) - 1; /* length available */
            Strcpy(mcbuf, "\"");
            if (strlen(tmp->origstr) > ln) {
                Strcat(strncat(mcbuf, tmp->origstr, ln - 3), "...");
            } else {
                Strcat(mcbuf, tmp->origstr);
            }
            /* combine main string and suffix */
            Strcat(mcbuf, &buf[1]); /* skip buf[]'s initial quote */
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, mcbuf, MENU_ITEMFLAGS_NONE);
            tmp = tmp->next;
        }
        Sprintf(mcbuf, "%s menu colors", (opt_idx == 1) ? "List of" : "Remove which");
        end_menu(tmpwin, mcbuf);
        pick_cnt = select_menu(tmpwin, (opt_idx == 1) ? PICK_NONE : PICK_ANY, &pick_list);
        if (pick_cnt > 0) {
            for (pick_idx = 0; pick_idx < pick_cnt; ++pick_idx) {
                free_one_menu_coloring(pick_list[pick_idx].item.a_int - 1 - pick_idx);
            }
            free(pick_list), pick_list = (menu_item *) 0;
        }
        destroy_nhwindow(tmpwin);
        if (pick_cnt >= 0) {
            goto menucolors_again;
        }
    }
    return optn_ok;
}
#endif

/* check whether a user-supplied option string is a proper leading
   substring of a particular option name; option string might have
   a colon or equals sign and arbitrary value appended to it */
boolean
match_optname(user_string, opt_name, min_length, val_allowed)
const char *user_string, *opt_name;
int min_length;
boolean val_allowed;
{
    int len = (int)strlen(user_string);

    if (val_allowed) {
        const char *p = index(user_string, ':'),
                   *q = index(user_string, '=');

        if (!p || (q && q < p)) {
            p = q;
        }
        if (p) {
            /* 'user_string' hasn't necessarily been through mungspaces()
               so might have tabs or consecutive spaces */
            while (p > user_string && isspace(*(p-1))) {
                p--;
            }
            len = (int)(p - user_string);
        }
    }

    return (len >= min_length) && !strncmpi(opt_name, user_string, len);
}

/* most environment variables will eventually be printed in an error
 * message if they don't work, and most error message paths go through
 * BUFSZ buffers, which could be overflowed by a maliciously long
 * environment variable.  If a variable can legitimately be long, or
 * if it's put in a smaller buffer, the responsible code will have to
 * bounds-check itself.
 */
char *
nh_getenv(ev)
const char *ev;
{
    char *getev = getenv(ev);

    if (getev && strlen(getev) <= (BUFSZ / 2))
        return getev;
    else
        return (char *)0;
}

/* process options */
void
initoptions()
{
#ifndef MAC
    char *opts;
#endif
    int i;

    /* initialize the random number generator */
    init_random(0);

    /* set up the command parsing */
    reset_commands(TRUE); /* init */

    /* for detection of configfile options specified multiple times */
    iflags.opt_booldup = iflags.opt_compdup = (int *)0;

    for (i = 0; boolopt[i].name; i++) {
        if (boolopt[i].addr)
            *(boolopt[i].addr) = boolopt[i].initvalue;
    }
    flags.end_own = FALSE;
    flags.end_top = 3;
    flags.end_around = 2;
    iflags.runmode = RUN_LEAP;
    iflags.msg_history = 20;
#ifdef TTY_GRAPHICS
    iflags.prevmsg_window = 'f';
#endif
    iflags.menu_headings = ATR_INVERSE;
    iflags.pilesize = 5;
    iflags.getpos_coords = GPCOORDS_NONE;

#ifdef REALTIME_ON_BOTL
    iflags.showrealtime = REALTIME_NONE;
    iflags.realtime_format = REALTIME_FORMAT_UNITS;
#endif

    /* Set the default monster and object class symbols. */
    flags.initrole = ROLE_NONE;
    flags.initrace = ROLE_NONE;
    flags.initgend = ROLE_NONE;
    flags.initalign = ROLE_NONE;

    /* Set the default monster and object class symbols.  Don't use */
    /* memcpy() --- sizeof char != sizeof uchar on some machines.   */
    for (i = 0; i < MAXOCLASSES; i++)
        oc_syms[i] = (uchar) def_oc_syms[i];
    for (i = 0; i < MAXMCLASSES; i++)
        monsyms[i] = (uchar) def_monsyms[i];
    for (i = 0; i < WARNCOUNT; i++)
        warnsyms[i] = def_warnsyms[i].sym;
    iflags.bouldersym = 0;
    iflags.travelcc.x = iflags.travelcc.y = -1;
    flags.warnlevel = 1;
    flags.warntype = 0L;

    /* assert( sizeof flags.inv_order == sizeof def_inv_order ); */
    (void)memcpy((genericptr_t)flags.inv_order,
                 (genericptr_t)def_inv_order, sizeof flags.inv_order);
    flags.pickup_types[0] = '\0';
    flags.pickup_burden = SLT_ENCUMBER;

#ifdef SORTLOOT
    /* sort only loot by default */
    iflags.sortloot = 'l';
#endif

    for (i = 0; i < NUM_DISCLOSURE_OPTIONS; i++)
        flags.end_disclose[i] = DISCLOSE_PROMPT_DEFAULT_NO;

    /* set default characters */
#ifdef UTF8_GLYPHS
    if (iflags.supports_utf8) {
        switch_graphics(UTF8_GRAPHICS);
    } else {
        switch_graphics(ASCII_GRAPHICS);
    }
#else
    switch_graphics(ASCII_GRAPHICS);
#endif

#if defined(UNIX) && defined(TTY_GRAPHICS)
    /*
     * Set defaults for some options depending on what we can
     * detect about the environment's capabilities.
     * This has to be done after the global initialization above
     * and before reading user-specific initialization via
     * config file/environment variable below.
     */
    /* this detects the IBM-compatible console on most 386 boxes */
    if ((opts = nh_getenv("TERM")) && !strncmp(opts, "AT", 2)) {
        switch_graphics(IBM_GRAPHICS);
# ifdef TEXTCOLOR
        iflags.use_color = TRUE;
# endif
    }
#endif /* UNIX && TTY_GRAPHICS */
#if defined(UNIX) || defined(VMS)
# ifdef TTY_GRAPHICS
    /* detect whether a "vt" terminal can handle alternate charsets */
    if ((opts = nh_getenv("TERM")) &&
        !strncmpi(opts, "vt", 2) && AS && AE &&
        index(AS, '\016') && index(AE, '\017')) {
        switch_graphics(DEC_GRAPHICS);
    }

    iflags.truecolor_separator = ';';

    /* Set color default to false if env variable NO_COLOR is present.
     * see https://no-color.org/ */
    if (nh_getenv("NO_COLOR")) {
        iflags.use_color = FALSE;
    }
# endif
#endif /* UNIX || VMS */

#ifdef MAC_GRAPHICS_ENV
    switch_graphics(MAC_GRAPHICS);
#endif /* MAC_GRAPHICS_ENV */
    flags.menu_style = MENU_FULL;

    iflags.wc_align_message = ALIGN_TOP;
    iflags.wc_align_status = ALIGN_BOTTOM;
    /* only used by curses */
    iflags.wc2_windowborders = 2; /* 'Auto' */

    /* since this is done before init_objects(), do partial init here */
    objects[SLIME_MOLD].oc_name_idx = SLIME_MOLD;
    nmcpy(pl_fruit, OBJ_NAME(objects[SLIME_MOLD]), PL_FSIZ);

#ifdef SYSCF
/* someday there may be other SYSCF alternatives besides text file */
#ifdef SYSCF_FILE
    /* If SYSCF_FILE is specified, it _must_ exist... */
    assure_syscf_file();

    /* ... and _must_ parse correctly. */
    if (!read_config_file(SYSCF_FILE, set_in_sysconf)) {
        nh_terminate(EXIT_FAILURE);
    }
    /*
     * TODO [maybe]: parse the sysopt entries which are space-separated
     * lists of usernames into arrays with one name per element.
     */
#endif
#endif /* SYSCF */

#ifndef MAC
    opts = getenv("NETHACKOPTIONS");
    if (!opts) opts = getenv("HACKOPTIONS");
    if (opts) {
        if (*opts == '/' || *opts == '\\' || *opts == '@') {
            if (*opts == '@') opts++;   /* @filename */
            /* looks like a filename */
            if (strlen(opts) < BUFSZ/2)
                read_config_file(opts, set_in_config);
        } else {
            read_config_file((char *) 0, set_in_config);
            /* let the total length of options be long;
             * parseoptions() will check each individually
             */
            parseoptions(opts, TRUE, FALSE);
        }
    } else
#endif
    read_config_file((char *) 0, set_in_config);

    (void)fruitadd(pl_fruit);
    /* Remove "slime mold" from list of object names; this will */
    /* prevent it from being wished unless it's actually present    */
    /* as a named (or default) fruit.  Wishing for "fruit" will */
    /* result in the player's preferred fruit [better than "\033"]. */
    obj_descr[SLIME_MOLD].oc_name = "fruit";
}

STATIC_OVL void
nmcpy(dest, src, maxlen)
char    *dest;
const char *src;
int maxlen;
{
    int count;

    for(count = 1; count < maxlen; count++) {
        if(*src == ',' || *src == '\0') break; /*exit on \0 terminator*/
        *dest++ = *src++;
    }
    *dest = 0;
}

/*
 * escapes(): escape expansion for showsyms.  C-style escapes understood
 * include \n, \b, \t, \r, \xnnn (hex), \onnn (octal), \nnn (decimal).
 * (Note: unlike in C, leading digit 0 is not used to indicate octal;
 * the letter o (either upper or lower case) is used for that.
 * The ^-prefix for control characters is also understood, and \[mM]
 * has the effect of 'meta'-ing the value which follows (so that the
 * alternate character set will be enabled).
 *
 * X     normal key X
 * ^X    control-X
 * \mX   meta-X
 *
 * For 3.4.3 and earlier, input ending with "\M", backslash, or caret
 * prior to terminating '\0' would pull that '\0' into the output and then
 * keep processing past it, potentially overflowing the output buffer.
 * Now, trailing \ or ^ will act like \\ or \^ and add '\\' or '^' to the
 * output and stop there; trailing \M will fall through to \<other> and
 * yield 'M', then stop.  Any \X or \O followed by something other than
 * an appropriate digit will also fall through to \<other> and yield 'X'
 * or 'O', plus stop if the non-digit is end-of-string.
 */
STATIC_OVL void
escapes(cp, tp)
const char *cp; /* might be 'tp', updating in place */
char *tp; /* result is never longer than 'cp' */
{
    static NEARDATA const char oct[] = "01234567", dec[] = "0123456789",
                               hex[] = "00112233445566778899aAbBcCdDeEfF";
    const char *dp;
    int cval, meta, dcount;

    while (*cp) {
        /* \M has to be followed by something to do meta conversion,
           otherwise it will just be \M which ultimately yields 'M' */
        meta = (*cp == '\\' && (cp[1] == 'm' || cp[1] == 'M') && cp[2]);
        if (meta) {
            cp += 2;
        }

        cval = dcount = 0; /* for decimal, octal, hexadecimal cases */
        if ((*cp != '\\' && *cp != '^') || !cp[1]) {
            /* simple character, or nothing left for \ or ^ to escape */
            cval = *cp++;
        } else if (*cp == '^') { /* expand control-character syntax */
            cval = (*++cp & 0x1f);
            ++cp;

        /* remaining cases are all for backslash; we know cp[1] is not \0 */
        } else if (index(dec, cp[1])) {
            ++cp; /* move past backslash to first digit */
            do {
                cval = (cval * 10) + (*cp - '0');
            } while (*++cp && index(dec, *cp) && ++dcount < 3);
        } else if ((cp[1] == 'o' || cp[1] == 'O') && cp[2] && index(oct, cp[2])) {
            cp += 2; /* move past backslash and 'O' */
            do {
                cval = (cval * 8) + (*cp - '0');
            } while (*++cp && index(oct, *cp) && ++dcount < 3);
        } else if ((cp[1] == 'x' || cp[1] == 'X') && cp[2] && (dp = index(hex, cp[2])) != 0) {
            cp += 2; /* move past backslash and 'X' */
            do {
                cval = (cval * 16) + ((int) (dp - hex) / 2);
            } while (*++cp && (dp = index(hex, *cp)) != 0 && ++dcount < 2);
        } else { /* C-style character escapes */
            switch (*++cp) {
                case '\\': cval = '\\'; break;
                case 'n':  cval = '\n'; break;
                case 't':  cval = '\t'; break;
                case 'b':  cval = '\b'; break;
                case 'r':  cval = '\r'; break;
                default:   cval = *cp;
            }
            ++cp;
        }

        if (meta) {
            cval |= 0x80;
        }
        *tp++ = (char) cval;
    }
    *tp = '\0';
}

STATIC_OVL void
rejectoption(optname)
const char *optname;
{
#ifdef MICRO
    pline("\"%s\" settable only from %s.", optname, configfile);
#else
    pline("%s can be set only from NETHACKOPTIONS or %s.", optname,
          configfile);
#endif
}

STATIC_OVL void
badoption(opts)
const char *opts;
{
    if (!initial) {
        if (!strncmp(opts, "h", 1) || !strncmp(opts, "?", 1))
            option_help();
        else
            pline("Bad syntax: %s.  Enter \"?g\" for help.", opts);
        return;
    }
#ifdef MAC
    else return;
#endif

    if(from_file)
        raw_printf("Bad syntax in OPTIONS in %s: %s.", configfile, opts);
    else
        raw_printf("Bad syntax in NETHACKOPTIONS: %s.", opts);

    wait_synch();
}

STATIC_OVL char *
string_for_opt(opts, val_optional)
char *opts;
boolean val_optional;
{
    char *colon, *equals;

    colon = index(opts, ':');
    equals = index(opts, '=');
    if (!colon || (equals && equals < colon)) colon = equals;

    if (!colon || !*++colon) {
        if (!val_optional) badoption(opts);
        return (char *)0;
    }
    return colon;
}

STATIC_OVL char *
string_for_env_opt(optname, opts, val_optional)
const char *optname;
char *opts;
boolean val_optional;
{
    if(!initial) {
        rejectoption(optname);
        return (char *)0;
    }
    return string_for_opt(opts, val_optional);
}

STATIC_OVL void
bad_negation(optname, with_parameter)
const char *optname;
boolean with_parameter;
{
    pline_The("%s option may not %sbe negated.",
              optname,
              with_parameter ? "both have a value and " : "");
}

/*
 * Change the inventory order, using the given string as the new order.
 * Missing characters in the new order are filled in at the end from
 * the current inv_order, except for gold, which is forced to be first
 * if not explicitly present.
 *
 * This routine returns 1 unless there is a duplicate or bad char in
 * the string.
 */
STATIC_OVL int
change_inv_order(op)
char *op;
{
    int oc_sym, num;
    char *sp, buf[BUFSZ];

    num = 0;
    if (!index(op, GOLD_SYM)) {
        buf[num++] = COIN_CLASS;
    }

    for (sp = op; *sp; sp++) {
        oc_sym = def_char_to_objclass(*sp);
        /* reject bad or duplicate entries */
        if (oc_sym == MAXOCLASSES ||
            oc_sym == RANDOM_CLASS || oc_sym == ILLOBJ_CLASS ||
            !index(flags.inv_order, oc_sym) || index(sp+1, *sp))
            return 0;
        /* retain good ones */
        buf[num++] = (char) oc_sym;
    }
    buf[num] = '\0';

    /* fill in any omitted classes, using previous ordering */
    for (sp = flags.inv_order; *sp; sp++)
        if (!index(buf, *sp)) {
            buf[num++] = *sp;
            buf[num] = '\0'; /* explicitly terminate for next index() */
        }

    Strcpy(flags.inv_order, buf);
    return 1;
}

STATIC_OVL void
graphics_opts(opts, optype, maxlen, offset)
register char *opts;
const char *optype;
int maxlen, offset;
{
    glyph_t translate[MAXPCHARS+1];
    int length, i;

    if (!(opts = string_for_env_opt(optype, opts, FALSE)))
        return;
    escapes(opts, opts);

    length = strlen(opts);
    if (length > maxlen) length = maxlen;
    /* match the form obtained from PC configuration files */
    for (i = 0; i < length; i++)
        translate[i] = (glyph_t) opts[i];
    assign_graphics(translate, length, maxlen, offset);
}

STATIC_OVL void
warning_opts(opts, optype)
register char *opts;
const char *optype;
{
    uchar translate[MAXPCHARS+1];
    int length, i;

    if (!(opts = string_for_env_opt(optype, opts, FALSE)))
        return;
    escapes(opts, opts);

    length = strlen(opts);
    if (length > WARNCOUNT) length = WARNCOUNT;
    /* match the form obtained from PC configuration files */
    for (i = 0; i < length; i++)
        translate[i] = (((i < WARNCOUNT) && opts[i]) ?
                        (uchar) opts[i] : def_warnsyms[i].sym);
    assign_warnings(translate);
}

void
assign_warnings(graph_chars)
register uchar *graph_chars;
{
    int i;
    for (i = 0; i < WARNCOUNT; i++)
        if (graph_chars[i]) warnsyms[i] = graph_chars[i];
}

STATIC_OVL int
feature_alert_opts(op, optn)
char *op;
const char *optn;
{
    char buf[BUFSZ];
    boolean rejectver = FALSE;
    unsigned long fnv = get_feature_notice_ver(op);     /* version.c */
    if (fnv == 0L) return 0;
    if (fnv > get_current_feature_ver())
        rejectver = TRUE;
    else
        flags.suppress_alert = fnv;
    if (rejectver) {
        if (!initial)
            You_cant("disable new feature alerts for future versions.");
        else {
            Sprintf(buf,
                    "\n%s=%s Invalid reference to a future version ignored",
                    optn, op);
            badoption(buf);
        }
        return 0;
    }
    if (!initial) {
        Sprintf(buf, "%lu.%lu.%lu", FEATURE_NOTICE_VER_MAJ,
                FEATURE_NOTICE_VER_MIN, FEATURE_NOTICE_VER_PATCH);
        pline("Feature change alerts disabled for NetHack %s features and prior.",
              buf);
    }
    return 1;
}

/*
 * This is used by parse_config_line() in files.c
 *
 */

/* parse key:command */
boolean
parsebindings(bindings)
char* bindings;
{
    char *bind;
    char key;
    int i;
    boolean ret = FALSE;

    /* break off first binding from the rest; parse the rest */
    if ((bind = index(bindings, ',')) != 0) {
        *bind++ = 0;
        ret |= parsebindings(bind);
    }

    /* parse a single binding: first split around : */
    if (! (bind = index(bindings, ':'))) {
        return FALSE; /* it's not a binding */
    }
    *bind++ = 0;

    /* read the key to be bound */
    key = txt2key(bindings);
    if (!key) {
        config_error_add("Unknown key binding key '%s'", bindings);
        return FALSE;
    }

    bind = trimspaces(bind);

    /* is it a special key? */
    if (bind_specialkey(key, bind)) {
        return TRUE;
    }

    /* is it a menu command? */
    for (i = 0; i < SIZE(default_menu_cmd_info); i++) {
        if (!strcmp(default_menu_cmd_info[i].name, bind)) {
            if (illegal_menu_cmd_key(key)) {
                config_error_add("Bad menu key %s:%s", visctrl(key), bind);
                return FALSE;
            } else
                add_menu_cmd_alias(key, default_menu_cmd_info[i].cmd);
            return TRUE;
        }
    }

    /* extended command? */
    if (!bind_key(key, bind)) {
        config_error_add("Unknown key binding command '%s'", bind);
        return FALSE;
    }
    return TRUE;
}

#if defined(STATUS_COLORS) && defined(TEXTCOLOR)

int match_str2clr(char*);
int match_str2attr(const char *, boolean);

struct color_option
parse_color_option(start)
char *start;
{
    struct color_option result = { NO_COLOR, 0 };
    char *end = NULL;
    int attr = 0;
    char *end_of_string = start + strlen(start);

    if ((end = index(start, '&')) != 0) {
        *end = '\0';
    }
    result.color = match_str2clr(start);

    while (end && end < end_of_string) {
        start = end + 1;
        if ((end = index(start, '&')) != 0) {
            *end = '\0';
        }
        attr = match_str2attr(start, FALSE);
        if (attr >= 0) {
            result.attr_bits |= 1 << attr;
        }
    }

    return result;
}

const struct percent_color_option *hp_colors = NULL;
const struct percent_color_option *pw_colors = NULL;
const struct text_color_option *text_colors = NULL;

struct percent_color_option *
add_percent_option(new_option, list_head)
struct percent_color_option *new_option;
struct percent_color_option *list_head;
{
    if (list_head == NULL)
        return new_option;
    if (new_option->percentage <= list_head->percentage) {
        new_option->next = list_head;
        return new_option;
    }
    list_head->next = add_percent_option(new_option, list_head->next);
    return list_head;
}

boolean
parse_status_color_option(start)
char *start;
{
    char *middle;

    while (*start && isspace(*start)) start++;
    for (middle = start; *middle != ':' && *middle != '=' && *middle != '\0'; ++middle);
    *middle++ = '\0';
    if (middle - start > 2 && (start[2] == '%' || start[2] == '.' || start[2] == '<' || start[2] == '>')) {
        struct percent_color_option *percent_color_option =
            (struct percent_color_option *)alloc(sizeof(*percent_color_option));
        percent_color_option->next = NULL;
        percent_color_option->percentage = atoi(start + 3);
        switch (start[2]) {
        default:
        case '%': percent_color_option->statclrtype = STATCLR_TYPE_PERCENT; break;
        case '.': percent_color_option->statclrtype = STATCLR_TYPE_NUMBER_EQ; break;
        case '>': percent_color_option->statclrtype = STATCLR_TYPE_NUMBER_GT; break;
        case '<': percent_color_option->statclrtype = STATCLR_TYPE_NUMBER_LT; break;
        }
        percent_color_option->color_option = parse_color_option(middle);
        start[2] = '\0';
        if (percent_color_option->color_option.color >= 0
            && percent_color_option->color_option.attr_bits >= 0) {
            if (!strcmpi(start, "hp")) {
                hp_colors = add_percent_option(percent_color_option, hp_colors);
                return TRUE;
            }
            if (!strcmpi(start, "pw")) {
                pw_colors = add_percent_option(percent_color_option, pw_colors);
                return TRUE;
            }
        }
        free(percent_color_option);
        return FALSE;
    } else {
        int length = strlen(start) + 1;
        struct text_color_option *text_color_option =
            (struct text_color_option *)alloc(sizeof(*text_color_option));
        text_color_option->next = NULL;
        text_color_option->text = (char *)alloc(length);
        memcpy((char *)text_color_option->text, start, length);
        text_color_option->color_option = parse_color_option(middle);
        if (text_color_option->color_option.color >= 0 &&
             text_color_option->color_option.color < CLR_MAX &&
             text_color_option->color_option.attr_bits >= 0) {
            text_color_option->next = text_colors;
            text_colors = text_color_option;
            return TRUE;
        }
        free((genericptr_t)text_color_option->text);
        free(text_color_option);
        return FALSE;
    }
}

boolean
parse_status_color_options(start)
char *start;
{
    char last = ',';
    char *end = start - 1;
    boolean ok = TRUE;
    while (last == ',') {
        for (start = ++end; *end != ',' && *end != '\0'; ++end);
        last = *end;
        *end = '\0';
        ok = parse_status_color_option(start) && ok;
    }
    return ok;
}


#endif /* STATUS_COLORS */

void
set_duplicate_opt_detection(on_or_off)
int on_or_off;
{
    int k, *optptr;
    if (on_or_off != 0) {
        /*-- ON --*/
        if (iflags.opt_booldup)
            impossible("iflags.opt_booldup already on (memory leak)");
        iflags.opt_booldup = (int *)alloc(SIZE(boolopt) * sizeof(int));
        optptr = iflags.opt_booldup;
        for (k = 0; k < SIZE(boolopt); ++k)
            *optptr++ = 0;

        if (iflags.opt_compdup)
            impossible("iflags.opt_compdup already on (memory leak)");
        iflags.opt_compdup = (int *)alloc(SIZE(compopt) * sizeof(int));
        optptr = iflags.opt_compdup;
        for (k = 0; k < SIZE(compopt); ++k)
            *optptr++ = 0;
    } else {
        /*-- OFF --*/
        if (iflags.opt_booldup) free((genericptr_t) iflags.opt_booldup);
        iflags.opt_booldup = (int *)0;
        if (iflags.opt_compdup) free((genericptr_t) iflags.opt_compdup);
        iflags.opt_compdup = (int *)0;
    }
}

STATIC_OVL void
duplicate_opt_detection(opts, bool_or_comp)
const char *opts;
int bool_or_comp;   /* 0 == boolean option, 1 == compound */
{
    int i, *optptr;
#if defined(MAC)
    /* the Mac has trouble dealing with the output of messages while
     * processing the config file.  That should get fixed one day.
     * For now just return.
     */
    return;
#endif
    if ((bool_or_comp == 0) && iflags.opt_booldup && initial && from_file) {
        for (i = 0; boolopt[i].name; i++) {
            if (match_optname(opts, boolopt[i].name, 3, FALSE)) {
                optptr = iflags.opt_booldup + i;
                if (*optptr == 1) {
                    raw_printf(
                        "\nWarning - Boolean option specified multiple times: %s.\n",
                        opts);
                    wait_synch();
                }
                *optptr += 1;
                break; /* don't match multiple options */
            }
        }
    } else if ((bool_or_comp == 1) && iflags.opt_compdup && initial && from_file) {
        for (i = 0; compopt[i].name; i++) {
            if (match_optname(opts, compopt[i].name, strlen(compopt[i].name), TRUE)) {
                optptr = iflags.opt_compdup + i;
                if (*optptr == 1) {
                    raw_printf(
                        "\nWarning - compound option specified multiple times: %s.\n",
                        compopt[i].name);
                    wait_synch();
                }
                *optptr += 1;
                break; /* don't match multiple options */
            }
        }
    }
}

static const struct {
    const char *name;
    const int color;
} colornames[] = {
    { "black", CLR_BLACK },
    { "red", CLR_RED },
    { "green", CLR_GREEN },
    { "brown", CLR_BROWN },
    { "blue", CLR_BLUE },
    { "magenta", CLR_MAGENTA },
    { "cyan", CLR_CYAN },
    { "gray", CLR_GRAY },
    { "orange", CLR_ORANGE },
    { "light green", CLR_BRIGHT_GREEN },
    { "yellow", CLR_YELLOW },
    { "light blue", CLR_BRIGHT_BLUE },
    { "light magenta", CLR_BRIGHT_MAGENTA },
    { "light cyan", CLR_BRIGHT_CYAN },
    { "white", CLR_WHITE },
#ifdef MENU_COLOR
    {"none", NO_COLOR},
#endif
    { "no color", NO_COLOR },
    { NULL, CLR_BLACK }, /* everything after this is an alias */
    { "transparent", NO_COLOR },
    { "purple", CLR_MAGENTA },
    { "light purple", CLR_BRIGHT_MAGENTA },
    { "bright purple", CLR_BRIGHT_MAGENTA },
    { "grey", CLR_GRAY },
    { "bright red", CLR_ORANGE },
    { "bright green", CLR_BRIGHT_GREEN },
    { "bright blue", CLR_BRIGHT_BLUE },
    { "bright magenta", CLR_BRIGHT_MAGENTA },
    { "bright cyan", CLR_BRIGHT_CYAN }
};

static const struct {
    const char *name;
    const int attr;
} attrnames[] = {
    { "none", ATR_NONE },
    { "bold", ATR_BOLD },
    { "dim", ATR_DIM },
    { "underline", ATR_ULINE },
    { "blink", ATR_BLINK },
    { "inverse", ATR_INVERSE },
    { NULL, ATR_NONE }, /* everything after this is an alias */
    { "normal", ATR_NONE },
    { "uline", ATR_ULINE },
    { "reverse", ATR_INVERSE },
};

const char *
clr2colorname(clr)
int clr;
{
    int i;

    for (i = 0; i < SIZE(colornames); i++)
        if (colornames[i].name && colornames[i].color == clr)
            return colornames[i].name;
    return (char *) 0;
}

int
match_str2clr(str)
char *str;
{
    int i, c = CLR_MAX;

    /* allow "lightblue", "light blue", and "light-blue" to match "light blue"
       (also junk like "_l i-gh_t---b l u e" but we won't worry about that);
       also copes with trailing space; mungspaces removed any leading space */
    for (i = 0; i < SIZE(colornames); i++)
        if (colornames[i].name
            && fuzzymatch(str, colornames[i].name, " -_", TRUE)) {
            c = colornames[i].color;
            break;
        }

    if (c < 0 || c >= CLR_MAX) {
        raw_printf("Unknown color '%s'", str);
        c = CLR_MAX; /* "none of the above" */
    }

    return c;
}

static const char *
attr2attrname(attr)
int attr;
{
    int i;
    for (i = 0; i < SIZE(attrnames); i++) {
        if (attrnames[i].attr == attr) {
            return attrnames[i].name;
        }
    }
    return (char *) 0;
}

int
match_str2attr(const char *str, boolean complain)
{
    int i, a = -1;

    for (i = 0; i < SIZE(attrnames); i++) {
        if (attrnames[i].name && fuzzymatch(str, attrnames[i].name, " -_", TRUE)) {
            a = attrnames[i].attr;
            break;
        }
    }

    if (a == -1 && complain) {
        config_error_add("Unknown text attribute '%.50s'", str);
    }

    return a;
}

int
query_color(prompt)
const char *prompt;
{
    winid tmpwin;
    anything any;
    int i, pick_cnt;
    menu_item *picks = (menu_item *) 0;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);
    any = zeroany;
    for (i = 0; i < SIZE(colornames); i++) {
        if (!colornames[i].name) {
            break;
        }
        any.a_int = i + 1;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, colornames[i].name,
                 (colornames[i].color == NO_COLOR) ? MENU_ITEMFLAGS_SELECTED
                                                   : MENU_ITEMFLAGS_NONE);
    }
    end_menu(tmpwin, (prompt && *prompt) ? prompt : "Pick a color");
    pick_cnt = select_menu(tmpwin, PICK_ONE, &picks);
    destroy_nhwindow(tmpwin);
    if (pick_cnt > 0) {
        i = colornames[picks[0].item.a_int - 1].color;
        /* pick_cnt==2: explicitly picked something other than the
           preselected entry */
        if (pick_cnt == 2 && i == NO_COLOR) {
            i = colornames[picks[1].item.a_int - 1].color;
        }
        free(picks);
        return i;
    } else if (pick_cnt == 0) {
        /* pick_cnt==0: explicitly picking preselected entry toggled it off */
        return NO_COLOR;
    }
    return -1;
}

/* ask about highlighting attribute; for menu headers and menu
   coloring patterns, only one attribute at a time is allowed;
   for status highlighting, multiple attributes are allowed [overkill;
   life would be much simpler if that were restricted to one also...] */
int
query_attr(prompt)
const char *prompt;
{
    winid tmpwin;
    anything any;
    int i, pick_cnt;
    menu_item *picks = (menu_item *) 0;
    boolean allow_many = (prompt && !strncmpi(prompt, "Choose", 6));
    int default_attr = ATR_NONE;

    if (prompt && strstri(prompt, "menu headings")) {
        default_attr = iflags.menu_headings;
    }
    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);
    any = zeroany;
    for (i = 0; i < SIZE(attrnames); i++) {
        if (!attrnames[i].name) {
            break;
        }
        any.a_int = i + 1;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, attrnames[i].attr,
                 attrnames[i].name,
                 (attrnames[i].attr == default_attr) ? MENU_ITEMFLAGS_SELECTED
                                                     : MENU_ITEMFLAGS_NONE);
    }
    end_menu(tmpwin, (prompt && *prompt) ? prompt : "Pick an attribute");
    pick_cnt = select_menu(tmpwin, allow_many ? PICK_ANY : PICK_ONE, &picks);
    destroy_nhwindow(tmpwin);
    if (pick_cnt > 0) {
        int j, k = 0;

        if (allow_many) {
            /* PICK_ANY, with one preselected entry (ATR_NONE) which
               should be excluded if any other choices were picked */
            for (i = 0; i < pick_cnt; ++i) {
                j = picks[i].item.a_int - 1;
                if (attrnames[j].attr != ATR_NONE || pick_cnt == 1) {
                    switch (attrnames[j].attr) {
                    case ATR_DIM:     k |= HL_DIM; break;
                    case ATR_BLINK:   k |= HL_BLINK; break;
                    case ATR_ULINE:   k |= HL_ULINE; break;
                    case ATR_INVERSE: k |= HL_INVERSE; break;
                    case ATR_BOLD:    k |= HL_BOLD; break;
                    case ATR_NONE:    k =  HL_NONE; break;
                    }
                }
            }
        } else {
            /* PICK_ONE, but might get 0 or 2 due to preselected entry */
            j = picks[0].item.a_int - 1;
            /* pick_cnt==2: explicitly picked something other than the
               preselected entry */
            if (pick_cnt == 2 && attrnames[j].attr == default_attr) {
                j = picks[1].item.a_int - 1;
            }
            k = attrnames[j].attr;
        }
        free(picks);
        return k;
    } else if (pick_cnt == 0 && !allow_many) {
        /* PICK_ONE, preselected entry explicitly chosen */
        return default_attr;
    }
    /* either ESC to explicitly cancel (pick_cnt==-1) or
       PICK_ANY with preselected entry toggled off and nothing chosen */
    return -1;
}

#ifdef MENU_COLOR
/* parse 'str' as a regular expression to check whether it's valid;
   compiled regexp gets thrown away regardless of the outcome */
static boolean
test_regex_pattern(str, errmsg)
const char *str;
const char *errmsg;
{
    static const char def_errmsg[] = "NHregex error";
    struct nhregex *match;
    const char *re_error_desc;
    boolean retval;

    if (!str) {
        return FALSE;
    }
    if (!errmsg) {
        errmsg = def_errmsg;
    }

    match = regex_init();
    if (!match) {
        config_error_add("%s", errmsg);
        return FALSE;
    }

    retval = regex_compile(str, match);
    /* get potential error message before freeing regexp and free regexp
       before issuing message in case the error is "ran out of memory"
       since message delivery might need to allocate some memory */
    re_error_desc = !retval ? regex_error_desc(match) : 0;
    /* discard regexp; caller will re-parse it after validating other stuff */
    regex_free(match);
    /* if returning failure, tell player */
    if (!retval) {
        config_error_add("%s: %s", errmsg, re_error_desc);
    }

    return retval;
}

/* parse '"regex_string"=color&attr' and add it to menucoloring */
boolean
add_menu_coloring(char *tmpstr) /* never NULL but could be empty */
{
    int c = NO_COLOR, a = ATR_NONE;
    char *tmps, *cs, *amp;
    char str[BUFSZ];

    (void) strncpy(str, tmpstr, sizeof str - 1);
    str[sizeof str - 1] = '\0';

    if ((cs = index(str, '=')) == 0) {
        config_error_add("Malformed MENUCOLOR");
        return FALSE;
    }

    tmps = cs + 1; /* advance past '=' */
    mungspaces(tmps);
    if ((amp = index(tmps, '&')) != 0) {
        *amp = '\0';
    }

    c = match_str2clr(tmps);
    if (c >= CLR_MAX) {
        return FALSE;
    }

    if (amp) {
        tmps = amp + 1; /* advance past '&' */
        a = match_str2attr(tmps, TRUE);
        if (a == -1) {
            return FALSE;
        }
    }

    /* the regexp portion here has not been condensed by mungspaces() */
    *cs = '\0';
    tmps = str;
    if (*tmps == '"' || *tmps == '\'') {
        cs--;
        while (isspace((uchar) *cs)) {
            cs--;
        }
        if (*cs == *tmps) {
            *cs = '\0';
            tmps++;
        }
    }

    return add_menu_coloring_parsed(tmps, c, a);
}

static boolean
add_menu_coloring_parsed(str, c, a)
char *str;
int c, a;
{
    static const char re_error[] = "Menucolor regex error";
    struct menucoloring *tmp;

    if (!str) {
        return FALSE;
    }
    tmp = (struct menucoloring *) alloc(sizeof *tmp);
    tmp->match = regex_init();
    /* test_regex_pattern() has already validated this regexp but parsing
       it again could conceivably run out of memory */
    if (!regex_compile(str, tmp->match)) {
        const char *re_error_desc = regex_error_desc(tmp->match);

        /* free first in case reason for regcomp failure was out-of-memory */
        regex_free(tmp->match);
        free(tmp);
        config_error_add("%s: %s", re_error, re_error_desc);
        return FALSE;
    }
    tmp->next = menu_colorings;
    tmp->origstr = dupstr(str);
    tmp->color = c;
    tmp->attr = a;
    menu_colorings = tmp;
    return TRUE;
}

void
free_menu_coloring()
{
    struct menucoloring *tmp, *tmp2;

    for (tmp = menu_colorings; tmp; tmp = tmp2) {
        tmp2 = tmp->next;
        regex_free(tmp->match);
        free(tmp->origstr);
        free(tmp);
    }
    menu_colorings = (struct menucoloring *) 0;
}

static void
free_one_menu_coloring(idx)
int idx; /* 0 .. */
{
    struct menucoloring *tmp = menu_colorings;
    struct menucoloring *prev = NULL;

    while (tmp) {
        if (idx == 0) {
            struct menucoloring *next = tmp->next;

            regex_free(tmp->match);
            free(tmp->origstr);
            free(tmp);
            if (prev) {
                prev->next = next;
            } else {
                menu_colorings = next;
            }
            return;
        }
        idx--;
        prev = tmp;
        tmp = tmp->next;
    }
}

static int
count_menucolors()
{
    struct menucoloring *tmp;
    int count = 0;

    for (tmp = menu_colorings; tmp; tmp = tmp->next) {
        count++;
    }
    return count;
}
#endif

/* parse '"monster name":color' and change monster info accordingly */
boolean
parse_monster_color(str)
char *str;
{
    int i, c = NO_COLOR;
    char *tmps, *cs = strchr(str, ':');
    char buf[BUFSZ];
    int monster;

    if (!str) return FALSE;

    strncpy(buf, str, BUFSZ);
    cs = strchr(buf, ':');
    if (!cs) return FALSE;

    tmps = cs;
    tmps++;
    /* skip whitespace at start of string */
    while (*tmps && isspace(*tmps)) tmps++;

    /* determine color */
    for (i = 0; i < SIZE(colornames); i++)
        if (colornames[i].name && strstri(tmps, colornames[i].name) == tmps) {
            c = colornames[i].color;
            break;
        }

    if (c > 15) return FALSE;

    /* determine monster name */
    *cs = '\0';
    tmps = buf;
    if ((*tmps == '"') || (*tmps == '\'')) {
        cs--;
        while (isspace(*cs)) cs--;
        if (*cs == *tmps) {
            *cs = '\0';
            tmps++;
        }
    }

    monster = name_to_mon(tmps);
    if (monster > -1) {
        mons[monster].mcolor = c;
        /* give all Riders the same color */
        if (monster == PM_DEATH || monster == PM_FAMINE || monster == PM_PESTILENCE) {
            mons[PM_DEATH].mcolor = c;
            mons[PM_FAMINE].mcolor = c;
            mons[PM_PESTILENCE].mcolor = c;
        }
        return TRUE;
    } else {
        return FALSE;
    }
}

/** Split up a string that matches name:value or 'name':value and
 * return name and value separately. */
static boolean
parse_extended_option(str, option_name, option_value)
const char *str;
char *option_name;  /**< Output string buffer for option name */
char *option_value; /**< Output string buffer for option value */
{
    int i;
    char *tmps, *cs;
    char buf[BUFSZ];

    if (!str) return FALSE;

    strncpy(buf, str, BUFSZ);

    /* remove comment*/
    cs = strrchr(buf, '#');
    if (cs) *cs = '\0';

    /* trim whitespace at end of string */
    i = strlen(buf)-1;
    while (i>=0 && isspace(buf[i])) {
        buf[i--] = '\0';
    }

    /* extract value */
    cs = strchr(buf, ':');
    if (!cs) return FALSE;

    tmps = cs;
    tmps++;
    /* skip whitespace at start of string */
    while (*tmps && isspace(*tmps)) tmps++;

    strncpy(option_value, tmps, BUFSZ);

    /* extract option name */
    *cs = '\0';
    tmps = buf;
    if ((*tmps == '"') || (*tmps == '\'')) {
        cs--;
        while (isspace(*cs)) cs--;
        if (*cs == *tmps) {
            *cs = '\0';
            tmps++;
        }
    }

    strncpy(option_name, tmps, BUFSZ);

    return TRUE;
}

/** Parse a string as Unicode codepoint and return the numerical codepoint.
 * Valid codepoints are decimal numbers or U+FFFF and 0xFFFF for hexadecimal
 * values. */
static int
parse_codepoint(codepoint)
char *codepoint;
{
    char *ptr, *endptr;
    int num=0, base;

    /* parse codepoint */
    if (!strncmpi(codepoint, "u+", 2) ||
        !strncmpi(codepoint, "0x", 2)) {
        /* hexadecimal */
        ptr = &codepoint[2];
        base = 16;
    } else {
        /* decimal */
        ptr = &codepoint[0];
        base = 10;
    }
    errno = 0;
    num = strtol(ptr, &endptr, base);
    if (errno != 0 || *endptr != 0 || endptr == ptr) {
        return FALSE;
    }
    return num;
}

/** Parse '"monster name":unicode_codepoint' and change symbol in
 * monster list. */
boolean
parse_monster_symbol(str)
const char *str;
{
    char monster[BUFSZ];
    char codepoint[BUFSZ];
    int i, num=0;

    if (!parse_extended_option(str, monster, codepoint)) {
        return FALSE;
    }

    num = parse_codepoint(codepoint);
    if (num < 0) {
        return FALSE;
    }

    /* find monster */
    for (i=0; mons[i].mlet != 0; i++) {
        if (!strcmpi(monster, mons[i].mname)) {
            mons[i].unicode_codepoint = num;
            return TRUE;
        }
    }

    return FALSE;
}

/** Parse '"dungeon feature":unicode_codepoint' and change symbol in
 * UTF8graphics. */
boolean
parse_symbol(str)
const char *str;
{
    char feature[BUFSZ];
    char codepoint[BUFSZ];
    int i, num;

    if (!parse_extended_option(str, feature, codepoint)) {
        return FALSE;
    }

    num = parse_codepoint(codepoint);
    if (num < 0) {
        return FALSE;
    }

    /* find dungeon feature */
    for (i=0; i < MAXPCHARS; i++) {
        if (!strcmpi(feature, defsyms[i].explanation)) {
            assign_utf8graphics_symbol(i, num);
            return TRUE;
        }
    }

    return FALSE;
}

/** Parse '"object name":unicode_codepoint' and change symbol in
 * object list. */
boolean
parse_object_symbol(str)
const char *str;
{
    char object[BUFSZ];
    char codepoint[BUFSZ];
    int i, num=0;

    if (!parse_extended_option(str, object, codepoint)) {
        return FALSE;
    }

    num = parse_codepoint(codepoint);
    if (num < 0) {
        return FALSE;
    }

    /* find object */
    for (i=0; obj_descr[i].oc_name || obj_descr[i].oc_descr; i++) {
        if ((obj_descr[i].oc_name && obj_descr[i].oc_descr) ||
            (obj_descr[i].oc_descr)) {
            /* Items with both descriptive and actual name or only
             * descriptive name. */
            if (!strcmpi(object, obj_descr[i].oc_descr)) {
                objects[i].unicode_codepoint = num;
                return TRUE;
            }
        } else if (obj_descr[i].oc_name) {
            /* items with only actual name like "carrot" */
            if (!strcmpi(object, obj_descr[i].oc_name)) {
                objects[i].unicode_codepoint = num;
                return TRUE;
            }
        }
    }

    return FALSE;
}

/** Parse '"color_name":rgb_in_hex' and set color value in instance flags. */
boolean
parse_color_definition(str)
const char *str;
{
    char color_name[BUFSZ];
    char color_value[BUFSZ];

    if (!parse_extended_option(str, color_name, color_value)) {
        return FALSE;
    }

    int color = match_str2clr(color_name);
    if (color >= CLR_MAX) {
        return FALSE;
    }

    if (strlen(color_value) != 6) {
        return FALSE;
    }

    char *ptr = color_value;
    iflags.color_definitions[color] = strtol(ptr, (char **)(ptr+6), 16);

    return TRUE;
}

void
common_prefix_options_parser(fullname, opts, negated)
const char *fullname;
char *opts;
boolean negated;
{
    boolean badopt = FALSE;
    char *op;
    size_t fullname_len = strlen(fullname);

    op = string_for_opt(opts, TRUE);
    if (op && negated) {
        bad_negation(fullname, TRUE);
        return;
    }
    /* "fullname" without a value means "all"
       and negated means "none" */
    if (!op || !strcmpi(op, "all") || !strcmpi(op, "none")) {
        int i;
        if (op && !strcmpi(op, "none")) negated = TRUE;
        boolean value = negated ? FALSE : TRUE;
        /* set all boolean options starting with fullname */
        for (i = 0; boolopt[i].name; i++) {
            if (!strncmp(boolopt[i].name, fullname, fullname_len)) {
                *boolopt[i].addr = value;
            }
        }
        return;
    }

    /* check for "+option1 -option2" */
    while (*op) {
        boolean check = FALSE, value = FALSE;
        register char c;
        c = *op;
        if (c == '+') {
            check = TRUE;
            value = TRUE;
        } else if (c == '-' || c == '!') {
            check = TRUE;
            value = FALSE;
        } else if (c == ' ') {
            /* do nothing */
            check = FALSE;
        } else {
            badopt = TRUE;
        }
        op++;
        if (check) {
            int i;
            for (i = 0; boolopt[i].name; i++) {
                int name_len = strlen(boolopt[i].name)-fullname_len-1;
                if ((strlen(boolopt[i].name) > fullname_len) &&
                    /* name starts with fullname */
                    !strncmp(boolopt[i].name, fullname, fullname_len) &&
                    boolopt[i].name[fullname_len] == '_' &&
                    /* name ends with user supplied option name */
                    !strncmp(boolopt[i].name+fullname_len+1, op, name_len)) {
                    op += name_len;
                    *boolopt[i].addr = value;
                }
            }
        }
    }

    if (badopt) badoption(opts);
}

boolean
parseoptions(opts, tinitial, tfrom_file)
register char *opts;
boolean tinitial, tfrom_file;
{
    register char *op;
    unsigned num;
    boolean negated;
    int i;
    const char *fullname;
    boolean retval = TRUE;

    initial = tinitial;
    from_file = tfrom_file;
    if ((op = index(opts, ',')) != 0) {
        *op++ = 0;
        if (!parseoptions(op, initial, from_file)) {
            retval = FALSE;
        }
    }
    if (strlen(opts) > BUFSZ/2) {
        badoption("option too long");
        return FALSE;
    }

    /* strip leading and trailing white space */
    while (isspace(*opts)) opts++;
    op = eos(opts);
    while (--op >= opts && isspace(*op)) *op = '\0';

    if (!*opts) {
        config_error_add("Empty statement");
        return FALSE;
    }
    negated = FALSE;
    while ((*opts == '!') || !strncmpi(opts, "no", 2)) {
        if (*opts == '!') opts++; else opts += 2;
        negated = !negated;
    }

    /* variant spelling */

    if (match_optname(opts, "colour", 5, FALSE))
        Strcpy(opts, "color");  /* fortunately this isn't longer */

    if (!match_optname(opts, "subkeyvalue", 11, TRUE)) /* allow multiple */
        duplicate_opt_detection(opts, 1); /* 1 means compound opts */

    /* special boolean options */

    if (match_optname(opts, "female", 3, FALSE)) {
        if(!initial && flags.female == negated)
            pline("That is not anatomically possible.");
        else
            flags.initgend = flags.female = !negated;
        return retval;
    }

    if (match_optname(opts, "male", 4, FALSE)) {
        if (!initial && flags.female != negated) {
            config_error_add("That is not anatomically possible.");
            return FALSE;
        } else {
            flags.initgend = flags.female = negated;
        }
        return retval;
    }

    /* heaven or hell */
    if (match_optname(opts, "heaven_or_hell", 14, FALSE)) {
        if (!initial)
            flags.heaven_or_hell = !negated;
    }
    /* hell or hell */
    if (match_optname(opts, "hell_and_hell", 13, FALSE)) {
        if (!initial)
            flags.hell_and_hell = !negated;
    }
    if (flags.hell_and_hell)
        flags.heaven_or_hell = TRUE;

    if (match_optname(opts, "marathon", 8, FALSE)) {
        if (!initial)
            marathon_mode = !negated;
    }

#if defined(MICRO) && !defined(AMIGA)
    /* included for compatibility with old NetHack.cnf files */
    if (match_optname(opts, "IBM_", 4, FALSE)) {
        iflags.BIOS = !negated;
        return retval;
    }
#endif /* MICRO */

    /* compound options */

    fullname = "pettype";
    if (match_optname(opts, fullname, 3, TRUE)) {
        if ((op = string_for_env_opt(fullname, opts, negated)) != 0) {
            if (negated) {
                bad_negation(fullname, TRUE);
                return FALSE;
            } else {
                switch (*op) {
                case 'd': /* dog */
                case 'D':
                    preferred_pet = 'd';
                    break;
                case 'c': /* cat */
                case 'C':
                case 'f': /* feline */
                case 'F':
                    preferred_pet = 'c';
                    break;
#ifdef EXOTIC_PETS
                case 'e': /* exotic */
                case 'E':
                    preferred_pet = 'e';
                    break;
#endif
                case 'n': /* no pet */
                case 'N':
                    preferred_pet = 'n';
                    break;
                default:
                    pline("Unrecognized pet type '%s'.", op);
                    break;
                }
            }
        } else if (negated) {
            preferred_pet = 'n';
        }
        return retval;
    }

    fullname = "catname";
    if (match_optname(opts, fullname, 3, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0) {
            nmcpy(catname, op, PL_PSIZ);
        }
        sanitizestr(catname);
        return retval;
    }

    fullname = "dogname";
    if (match_optname(opts, fullname, 3, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0)
            nmcpy(dogname, op, PL_PSIZ);
        sanitizestr(dogname);
        return retval;
    }

#ifdef DUMP_LOG
    fullname = "dumpfile";
    if (match_optname(opts, fullname, 3, TRUE)) {
#ifndef DUMP_FN
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_opt(opts, !tfrom_file)) != 0
                 && strlen(op) > 1)
            nmcpy(dump_fn, op, PL_PSIZ);
#endif
        return retval;
    }
#endif

    fullname = "horsename";
    if (match_optname(opts, fullname, 5, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0)
            nmcpy(horsename, op, PL_PSIZ);
        sanitizestr(horsename);
        return retval;
    }

#ifdef CONVICT
    fullname = "ratname";
    if (match_optname(opts, fullname, 3, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0)
            nmcpy(ratname, op, PL_PSIZ);
        return retval;
    }
#endif /* CONVICT */

    fullname = "nameempty";
    if (match_optname(opts, fullname, sizeof("nameempty")-1, TRUE)) {
        if ((op = string_for_opt(opts, FALSE)) != 0) {
            if (iflags.nameempty) free(iflags.nameempty);
            iflags.nameempty = (char *)alloc(strlen(op) + 1);
            Strcpy(iflags.nameempty, op);
        }
        return retval;
    }

    fullname = "number_pad";
    if (match_optname(opts, fullname, 10, TRUE)) {
        boolean compat = (strlen(opts) <= 10);
        op = string_for_opt(opts, (compat || !initial));
        if (!op) {
            if (compat || negated || initial) {
                /* for backwards compatibility, "number_pad" without a
                   value is a synonym for number_pad:1 */
                Cmd.num_pad = iflags.num_pad = !negated;
                if (iflags.num_pad) iflags.num_pad_mode = 0;
            }
        } else if (negated) {
            bad_negation("number_pad", TRUE);
            return FALSE;
        } else {
            if (*op == '1' || *op == '2') {
                Cmd.num_pad = iflags.num_pad = 1;
                if (*op == '2') iflags.num_pad_mode = 1;
                else iflags.num_pad_mode = 0;
            } else if (*op == '0') {
                Cmd.num_pad = iflags.num_pad = 0;
                iflags.num_pad_mode = 0;
            } else {
                badoption(opts);
                return FALSE;
            }
        }
        reset_commands(FALSE);
        number_pad(iflags.num_pad ? 1 : 0);
        return retval;
    }

#ifdef QWERTZ
    fullname = "qwertz_layout";
    if (match_optname(opts, fullname, 6, FALSE)) {
        if (negated)
            sdir=qykbd_dir;
        else
            sdir=qzkbd_dir;
        iflags.qwertz_layout=!negated;
        return retval;
    }
#endif

    fullname = "runmode";
    if (match_optname(opts, fullname, 4, TRUE)) {
        if (negated) {
            iflags.runmode = RUN_TPORT;
        } else if ((op = string_for_opt(opts, FALSE)) != 0) {
            if (!strncmpi(op, "teleport", strlen(op)))
                iflags.runmode = RUN_TPORT;
            else if (!strncmpi(op, "run", strlen(op)))
                iflags.runmode = RUN_LEAP;
            else if (!strncmpi(op, "walk", strlen(op)))
                iflags.runmode = RUN_STEP;
            else if (!strncmpi(op, "crawl", strlen(op)))
                iflags.runmode = RUN_CRAWL;
            else {
                badoption(opts);
                return FALSE;
            }
        }
        return retval;
    }

    /* menucolor:"regex_string"=color */
    fullname = "menucolor";
    if (match_optname(opts, fullname, 9, TRUE)) {
#ifdef MENU_COLOR
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0) {
            if (!add_menu_coloring(op)) {
                badoption(opts);
                return FALSE;
            }
        }
#endif
        return retval;
    }

    fullname = "hp_notify_fmt";
    if (match_optname(opts, fullname, sizeof("hp_notify_fmt")-1, TRUE)) {
        if ((op = string_for_opt(opts, FALSE)) != 0) {
            if (iflags.hp_notify_fmt) free(iflags.hp_notify_fmt);
            iflags.hp_notify_fmt = (char *)alloc(strlen(op) + 1);
            Strcpy(iflags.hp_notify_fmt, op);
        }
        return retval;
    }

#ifdef EXOTIC_PETS
    fullname = "monkeyname";
    if (match_optname(opts, fullname, 6, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0)
            nmcpy(monkeyname, op, PL_PSIZ);
        return retval;
    }

    fullname = "wolfname";
    if (match_optname(opts, fullname, 4, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0)
            nmcpy(wolfname, op, PL_PSIZ);
        return retval;
    }

    fullname = "crocodilename";
    if (match_optname(opts, fullname, 9, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0)
            nmcpy(crocodilename, op, PL_PSIZ);
        return retval;
    }
#endif

    fullname = "msghistory";
    if (match_optname(opts, fullname, 3, TRUE)) {
        op = string_for_env_opt(fullname, opts, negated);
        if ((negated && !op) || (!negated && op)) {
            iflags.msg_history = negated ? 0 : atoi(op);
        } else if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        return retval;
    }

    fullname="msg_window";
    /* msg_window:single, combo, full or reversed */
    if (match_optname(opts, fullname, 4, TRUE)) {
        /* allow option to be silently ignored by non-tty ports */
#ifdef TTY_GRAPHICS
        int tmp;
        if (!(op = string_for_opt(opts, TRUE))) {
            tmp = negated ? 's' : 'f';
        } else {
            if (negated) {
                bad_negation(fullname, TRUE);
                return FALSE;
            }
            tmp = tolower(*op);
        }
        switch (tmp) {
        case 's':       /* single message history cycle (default if negated) */
            iflags.prevmsg_window = 's';
            break;
        case 'c':       /* combination: two singles, then full page reversed */
            iflags.prevmsg_window = 'c';
            break;
        case 'f':       /* full page (default if no opts) */
            iflags.prevmsg_window = 'f';
            break;
        case 'r':       /* full page (reversed) */
            iflags.prevmsg_window = 'r';
            break;
        default:
            badoption(opts);
            return FALSE;
        }
#endif
        return retval;
    }

    /* WINCAP
     * setting font options  */
    fullname = "font";
    if (!strncmpi(opts, fullname, 4))
    {
        int wintype = -1;
        char *fontopts = opts + 4;

        if (!strncmpi(fontopts, "map", 3) ||
            !strncmpi(fontopts, "_map", 4))
            wintype = NHW_MAP;
        else if (!strncmpi(fontopts, "message", 7) ||
                 !strncmpi(fontopts, "_message", 8))
            wintype = NHW_MESSAGE;
        else if (!strncmpi(fontopts, "text", 4) ||
                 !strncmpi(fontopts, "_text", 5))
            wintype = NHW_TEXT;
        else if (!strncmpi(fontopts, "menu", 4) ||
                 !strncmpi(fontopts, "_menu", 5))
            wintype = NHW_MENU;
        else if (!strncmpi(fontopts, "status", 6) ||
                 !strncmpi(fontopts, "_status", 7))
            wintype = NHW_STATUS;
        else if (!strncmpi(fontopts, "_size", 5)) {
            if (!strncmpi(fontopts, "_size_map", 8))
                wintype = NHW_MAP;
            else if (!strncmpi(fontopts, "_size_message", 12))
                wintype = NHW_MESSAGE;
            else if (!strncmpi(fontopts, "_size_text", 9))
                wintype = NHW_TEXT;
            else if (!strncmpi(fontopts, "_size_menu", 9))
                wintype = NHW_MENU;
            else if (!strncmpi(fontopts, "_size_status", 11))
                wintype = NHW_STATUS;
            else {
                badoption(opts);
                return FALSE;
            }
            if (wintype > 0 && !negated &&
                (op = string_for_opt(opts, FALSE)) != 0) {
                switch(wintype)  {
                case NHW_MAP:
                    iflags.wc_fontsiz_map = atoi(op);
                    break;
                case NHW_MESSAGE:
                    iflags.wc_fontsiz_message = atoi(op);
                    break;
                case NHW_TEXT:
                    iflags.wc_fontsiz_text = atoi(op);
                    break;
                case NHW_MENU:
                    iflags.wc_fontsiz_menu = atoi(op);
                    break;
                case NHW_STATUS:
                    iflags.wc_fontsiz_status = atoi(op);
                    break;
                }
            }
            return retval;
        } else {
            badoption(opts);
            return FALSE;
        }
        if (wintype > 0 &&
            (op = string_for_opt(opts, FALSE)) != 0) {
            wc_set_font_name(wintype, op);
#ifdef MAC
            set_font_name (wintype, op);
#endif
            return retval;
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }
#ifdef CHANGE_COLOR
    if (match_optname(opts, "palette", 3, TRUE)
# ifdef MAC
        || match_optname(opts, "hicolor", 3, TRUE)
# endif
        ) {
        int color_number, color_incr;

# ifdef MAC
        if (match_optname(opts, "hicolor", 3, TRUE)) {
            if (negated) {
                bad_negation("hicolor", FALSE);
                return FALSE;
            }
            color_number = CLR_MAX + 4; /* HARDCODED inverse number */
            color_incr = -1;
        } else {
# endif
        if (negated) {
            bad_negation("palette", FALSE);
            return FALSE;
        }
        color_number = 0;
        color_incr = 1;
# ifdef MAC
    }
# endif
        if ((op = string_for_opt(opts, FALSE)) != (char *)0) {
            char *pt = op;
            int cnt, tmp, reverse;
            long rgb;

            while (*pt && color_number >= 0) {
                cnt = 3;
                rgb = 0L;
                if (*pt == '-') {
                    reverse = 1;
                    pt++;
                } else {
                    reverse = 0;
                }
                while (cnt-- > 0) {
                    if (*pt && *pt != '/') {
# ifdef AMIGA
                        rgb <<= 4;
# else
                        rgb <<= 8;
# endif
                        tmp = *(pt++);
                        if (isalpha(tmp)) {
                            tmp = (tmp + 9) & 0xf; /* Assumes ASCII... */
                        } else {
                            tmp &= 0xf; /* Digits in ASCII too... */
                        }
# ifndef AMIGA
                        /* Add an extra so we fill f -> ff and 0 -> 00 */
                        rgb += tmp << 4;
# endif
                        rgb += tmp;
                    }
                }
                if (*pt == '/') {
                    pt++;
                }
                change_color(color_number, rgb, reverse);
                color_number += color_incr;
            }
        }
        if (!initial) {
            need_redraw = TRUE;
        }
        return retval;
    }
#endif /* CHANGE_COLOR */

    if (match_optname(opts, "fruit", 2, TRUE)) {
        char empty_str = '\0';
        op = string_for_opt(opts, negated);
        if (negated) {
            if (op) {
                bad_negation("fruit", TRUE);
                return FALSE;
            }
            op = &empty_str;
            goto goodfruit;
        }
        if (!op) {
            return FALSE;
        }
        if (!initial) {
            struct fruit *f;

            num = 0;
            for(f=ffruit; f; f=f->nextf) {
                if (!strcmp(op, f->fname)) goto goodfruit;
                num++;
            }
            if (num >= 100) {
                pline("Doing that so many times isn't very fruitful.");
                return retval;
            }
        }
goodfruit:
        nmcpy(pl_fruit, op, PL_FSIZ);
        sanitizestr(pl_fruit);
        /* OBJ_NAME(objects[SLIME_MOLD]) won't work after initialization */
        if (!*pl_fruit)
            nmcpy(pl_fruit, "slime mold", PL_FSIZ);
        if (!initial)
            (void)fruitadd(pl_fruit);
        /* If initial, then initoptions is allowed to do it instead
         * of here (initoptions always has to do it even if there's
         * no fruit option at all.  Also, we don't want people
         * setting multiple fruits in their options.)
         */
        return retval;
    }

    /* graphics:string */
    fullname = "graphics";
    if (match_optname(opts, fullname, 2, TRUE)) {
        if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        } else {
            graphics_opts(opts, fullname, MAXPCHARS, 0);
        }
        return retval;
    }
    fullname = "dungeon";
    if (match_optname(opts, fullname, 2, TRUE)) {
        if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        } else {
            graphics_opts(opts, fullname, MAXDCHARS, 0);
        }
        return retval;
    }
    fullname = "traps";
    if (match_optname(opts, fullname, 2, TRUE)) {
        if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        } else {
            graphics_opts(opts, fullname, MAXTCHARS, MAXDCHARS);
        }
        return retval;
    }
    fullname = "effects";
    if (match_optname(opts, fullname, 2, TRUE)) {
        if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        } else {
            graphics_opts(opts, fullname, MAXECHARS, MAXDCHARS+MAXTCHARS);
        }
        return retval;
    }

    /* objects:string */
    fullname = "objects";
    if (match_optname(opts, fullname, 7, TRUE)) {
        int length;

        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        if (!(opts = string_for_env_opt(fullname, opts, FALSE))) {
            return FALSE;
        }
        escapes(opts, opts);

        /*
         * Override the default object class symbols.  The first
         * object in the object class is the "random object".  I
         * don't want to use 0 as an object class, so the "random
         * object" is basically a place holder.
         *
         * The object class symbols have already been initialized in
         * initoptions().
         */
        length = strlen(opts);
        if (length >= MAXOCLASSES)
            length = MAXOCLASSES-1; /* don't count RANDOM_OBJECT */

        for (i = 0; i < length; i++)
            oc_syms[i+1] = (uchar) opts[i];
        return retval;
    }

    /* monsters:string */
    fullname = "monsters";
    if (match_optname(opts, fullname, 8, TRUE)) {
        int length;

        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        if (!(opts = string_for_env_opt(fullname, opts, FALSE))) {
            return FALSE;
        }
        escapes(opts, opts);

        /* Override default mon class symbols set in initoptions(). */
        length = strlen(opts);
        if (length >= MAXMCLASSES)
            length = MAXMCLASSES-1; /* mon class 0 unused */

        for (i = 0; i < length; i++)
            monsyms[i+1] = (uchar) opts[i];
        return retval;
    }

    fullname = "whatis_coord";
    if (match_optname(opts, fullname, 8, TRUE)) {
        if (negated) {
            iflags.getpos_coords = GPCOORDS_NONE;
            return retval;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0) {
            static char gpcoords[] = {
                GPCOORDS_NONE, GPCOORDS_COMPASS,
                GPCOORDS_COMFULL, GPCOORDS_MAP,
                GPCOORDS_SCREEN, '\0'
            };
            char c = lowc(*op);

            if (c && index(gpcoords, c)) {
                iflags.getpos_coords = c;
            } else {
                config_error_add("Unknown %s parameter '%s'", fullname, op);
                return FALSE;
            }
        } else {
            return FALSE;
        }
        return retval;
    }

    fullname = "whatis_filter";
    if (match_optname(opts, fullname, 8, TRUE)) {
        if (negated) {
            iflags.getloc_filter = GFILTER_NONE;
            return retval;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0) {
            char c = lowc(*op);

            switch (c) {
                case 'n':
                    iflags.getloc_filter = GFILTER_NONE;
                    break;

                case 'v':
                    iflags.getloc_filter = GFILTER_VIEW;
                    break;

                case 'a':
                    iflags.getloc_filter = GFILTER_AREA;
                    break;

                default:
                    config_error_add("Unknown %s parameter '%s'", fullname, op);
                    return FALSE;
            }
        } else {
            return FALSE;
        }
        return retval;
    }

    fullname = "warnings";
    if (match_optname(opts, fullname, 5, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else {
            warning_opts(opts, fullname);
        }
        return retval;
    }
    /* boulder:symbol */
    fullname = "boulder";
    if (match_optname(opts, fullname, 7, TRUE)) {
        int clash = 0;
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
/*      if (!(opts = string_for_env_opt(fullname, opts, FALSE))) */
        if (!(opts = string_for_opt(opts, FALSE))) {
            return FALSE;
        }
        escapes(opts, opts);
        if (def_char_to_monclass(opts[0]) != MAXMCLASSES)
            clash = 1;
        else if (opts[0] >= '1' && opts[0] <= '5')
            clash = 2;
        if (clash) {
            /* symbol chosen matches a used monster or warning
               symbol which is not good - reject it*/
            pline(
                "Badoption - boulder symbol '%c' conflicts with a %s symbol.",
                opts[0], (clash == 1) ? "monster" : "warning");
            return FALSE;
        } else {
            /*
             * Override the default boulder symbol.
             */
            iflags.bouldersym = (uchar) opts[0];
        }
        if (!initial) need_redraw = TRUE;
        return retval;
    }

    /* name:string */
    fullname = "name";
    if (match_optname(opts, fullname, 4, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0)
            nmcpy(plname, op, PL_NSIZ);
        return retval;
    }

    /* role:string or character:string */
    fullname = "role";
    if (match_optname(opts, fullname, 4, TRUE) ||
        match_optname(opts, (fullname = "character"), 4, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0) {
            if ((flags.initrole = str2role(op)) == ROLE_NONE) {
                badoption(opts);
                return FALSE;
            } else {
                /* Backwards compatibility */
                nmcpy(pl_character, op, PL_NSIZ);
            }
        }
        return retval;
    }

    /* race:string */
    fullname = "race";
    if (match_optname(opts, fullname, 4, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0) {
            if ((flags.initrace = str2race(op)) == ROLE_NONE) {
                badoption(opts);
                return FALSE;
            }
            else /* Backwards compatibility */
                pl_race = *op;
        }
        return retval;
    }

    /* gender:string */
    fullname = "gender";
    if (match_optname(opts, fullname, 4, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0) {
            if ((flags.initgend = str2gend(op)) == ROLE_NONE) {
                badoption(opts);
                return FALSE;
            } else {
                flags.female = flags.initgend;
            }
        }
        return retval;
    }

    /* altkeyhandler:string */
    fullname = "altkeyhandler";
    if (match_optname(opts, fullname, 4, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        else if ((op = string_for_opt(opts, negated))) {
#ifdef WIN32CON
            (void)strncpy(iflags.altkeyhandler, op, MAX_ALTKEYHANDLER - 5);
            load_keyboard_handler();
#endif
        }
        return retval;
    }

    /* WINCAP
     * align_status:[left|top|right|bottom] */
    fullname = "align_status";
    if (match_optname(opts, fullname, sizeof("align_status")-1, TRUE)) {
        op = string_for_opt(opts, negated);
        if (op && !negated) {
            if (!strncmpi (op, "left", sizeof("left")-1))
                iflags.wc_align_status = ALIGN_LEFT;
            else if (!strncmpi (op, "top", sizeof("top")-1))
                iflags.wc_align_status = ALIGN_TOP;
            else if (!strncmpi (op, "right", sizeof("right")-1))
                iflags.wc_align_status = ALIGN_RIGHT;
            else if (!strncmpi (op, "bottom", sizeof("bottom")-1))
                iflags.wc_align_status = ALIGN_BOTTOM;
            else {
                badoption(opts);
                return FALSE;
            }
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }
    /* WINCAP
     * align_message:[left|top|right|bottom] */
    fullname = "align_message";
    if (match_optname(opts, fullname, sizeof("align_message")-1, TRUE)) {
        op = string_for_opt(opts, negated);
        if (op && !negated) {
            if (!strncmpi (op, "left", sizeof("left")-1))
                iflags.wc_align_message = ALIGN_LEFT;
            else if (!strncmpi (op, "top", sizeof("top")-1))
                iflags.wc_align_message = ALIGN_TOP;
            else if (!strncmpi (op, "right", sizeof("right")-1))
                iflags.wc_align_message = ALIGN_RIGHT;
            else if (!strncmpi (op, "bottom", sizeof("bottom")-1))
                iflags.wc_align_message = ALIGN_BOTTOM;
            else {
                badoption(opts);
                return FALSE;
            }
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }
    /* align:string */
    fullname = "align";
    if (match_optname(opts, fullname, sizeof("align")-1, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0)
            if ((flags.initalign = str2align(op)) == ROLE_NONE) {
                badoption(opts);
                return FALSE;
            }
        return retval;
    }

    /* the order to list the pack */
    fullname = "packorder";
    if (match_optname(opts, fullname, 4, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if (!(op = string_for_opt(opts, FALSE))) {
            return FALSE;
        }

        if (!change_inv_order(op)) {
            badoption(opts);
            return FALSE;
        }
        return retval;
    }

    /* maximum burden picked up before prompt (Warren Cheung) */
    fullname = "pickup_burden";
    if (match_optname(opts, fullname, 8, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0) {
            switch (tolower(*op)) {
            /* Unencumbered */
            case 'u':
                flags.pickup_burden = UNENCUMBERED;
                break;
            /* Burdened (slight encumbrance) */
            case 'b':
                flags.pickup_burden = SLT_ENCUMBER;
                break;
            /* streSsed (moderate encumbrance) */
            case 's':
                flags.pickup_burden = MOD_ENCUMBER;
                break;
            /* straiNed (heavy encumbrance) */
            case 'n':
                flags.pickup_burden = HVY_ENCUMBER;
                break;
            /* OverTaxed (extreme encumbrance) */
            case 'o':
            case 't':
                flags.pickup_burden = EXT_ENCUMBER;
                break;
            /* overLoaded */
            case 'l':
                flags.pickup_burden = OVERLOADED;
                break;
            default:
                badoption(opts);
                return FALSE;
            }
        }
        return retval;
    }

    /* types of objects to pick up automatically */
    if (match_optname(opts, "pickup_types", 8, TRUE)) {
        char ocl[MAXOCLASSES + 1], tbuf[MAXOCLASSES + 1],
             qbuf[QBUFSZ], abuf[BUFSZ];
        int oc_sym;
        boolean badopt = FALSE, compat = (strlen(opts) <= 6), use_menu;

        oc_to_str(flags.pickup_types, tbuf);
        flags.pickup_types[0] = '\0';   /* all */
        op = string_for_opt(opts, (compat || !initial));
        if (!op) {
            if (compat || negated || initial) {
                /* for backwards compatibility, "pickup" without a
                   value is a synonym for autopickup of all types
                   (and during initialization, we can't prompt yet) */
                flags.pickup = !negated;
                return retval;
            }
            oc_to_str(flags.inv_order, ocl);
            use_menu = TRUE;
            if (flags.menu_style == MENU_TRADITIONAL ||
                flags.menu_style == MENU_COMBINATION) {
                use_menu = FALSE;
                Sprintf(qbuf, "New pickup_types: [%s am] (%s)",
                        ocl, *tbuf ? tbuf : "all");
                getlin(qbuf, abuf);
                op = mungspaces(abuf);
                if (abuf[0] == '\0' || abuf[0] == '\033')
                    op = tbuf;  /* restore */
                else if (abuf[0] == 'm')
                    use_menu = TRUE;
            }
            if (use_menu) {
                (void) choose_classes_menu("Auto-Pickup what?", 1,
                                           TRUE, ocl, tbuf);
                op = tbuf;
            }
        }
        if (negated) {
            bad_negation("pickup_types", TRUE);
            return FALSE;
        }
        while (*op == ' ') op++;
        if (*op != 'a' && *op != 'A') {
            num = 0;
            while (*op) {
                oc_sym = def_char_to_objclass(*op);
                /* make sure all are valid obj symbols occuring once */
                if (oc_sym != MAXOCLASSES &&
                    !index(flags.pickup_types, oc_sym)) {
                    flags.pickup_types[num] = (char)oc_sym;
                    flags.pickup_types[++num] = '\0';
                } else
                    badopt = TRUE;
                op++;
            }
            if (badopt) {
                badoption(opts);
                return FALSE;
            }
        }
        return retval;
    }

    fullname = "pilesize";
    if (match_optname(opts, fullname, sizeof("pilesize")-1, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if (!(op = string_for_opt(opts, FALSE))) {
            return FALSE;
        }
        iflags.pilesize = atoi(op);
        if (iflags.pilesize < 1) iflags.pilesize = 1;
        return retval;
    }

    /* WINCAP
     * player_selection: dialog | prompts */
    fullname = "player_selection";
    if (match_optname(opts, fullname, sizeof("player_selection")-1, TRUE)) {
        op = string_for_opt(opts, negated);
        if (op && !negated) {
            if (!strncmpi (op, "dialog", sizeof("dialog")-1))
                iflags.wc_player_selection = VIA_DIALOG;
            else if (!strncmpi (op, "prompt", sizeof("prompt")-1))
                iflags.wc_player_selection = VIA_PROMPTS;
            else {
                badoption(opts);
                return FALSE;
            }

        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }

#ifdef REALTIME_ON_BOTL
    /* backwards compatibility */
    fullname = "showrealtime";
    if (match_optname(opts, fullname, 12, FALSE)) {
        iflags.showrealtime = REALTIME_PLAYTIME;
        return retval;
    }

    fullname = "realtime";
    if (match_optname(opts, fullname, 8, TRUE)) {
        op = string_for_opt(opts, negated);
        if (op && !negated) {
            iflags.showrealtime = REALTIME_PLAYTIME;
            if (!strncmpi(op, realtime_type_strings[REALTIME_NONE], 1)) {
                iflags.showrealtime = REALTIME_NONE;
            } else if (!strncmpi(op, realtime_type_strings[REALTIME_PLAYTIME], 1)) {
                iflags.showrealtime = REALTIME_PLAYTIME;
            } else if (!strncmpi(op, realtime_type_strings[REALTIME_WALLTIME], 1)) {
                iflags.showrealtime = REALTIME_WALLTIME;
            } else {
                badoption(opts);
                return FALSE;
            }
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }

    fullname = "realtime_format";
    if (match_optname(opts, fullname, 15, TRUE)) {
        op = string_for_opt(opts, negated);
        if (op && !negated) {
            if (!strncmpi(op, realtime_format_strings[REALTIME_FORMAT_SECONDS], 1)) {
                iflags.realtime_format = REALTIME_FORMAT_SECONDS;
            } else if (!strncmpi(op, realtime_format_strings[REALTIME_FORMAT_CONDENSED], 1)) {
                iflags.realtime_format = REALTIME_FORMAT_CONDENSED;
            } else if (!strncmpi(op, realtime_format_strings[REALTIME_FORMAT_UNITS], 1)) {
                iflags.realtime_format = REALTIME_FORMAT_UNITS;
            } else {
                badoption(opts);
                return FALSE;
            }
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }
#endif

    /* things to disclose at end of game */
    if (match_optname(opts, "disclose", 7, TRUE)) {
        /*
         * The order that the end_disclore options are stored:
         * inventory, attribs, vanquished, genocided, conduct
         * There is an array in flags:
         *  end_disclose[NUM_DISCLOSURE_OPT];
         * with option settings for the each of the following:
         * iagvc [see disclosure_options in decl.c]:
         * Legal setting values in that array are:
         *  DISCLOSE_PROMPT_DEFAULT_YES  ask with default answer yes
         *  DISCLOSE_PROMPT_DEFAULT_NO   ask with default answer no
         *  DISCLOSE_YES_WITHOUT_PROMPT  always disclose and don't ask
         *  DISCLOSE_NO_WITHOUT_PROMPT   never disclose and don't ask
         *
         * Those setting values can be used in the option
         * string as a prefix to get the desired behaviour.
         *
         * For backward compatibility, no prefix is required,
         * and the presence of a i,a,g,v, or c without a prefix
         * sets the corresponding value to DISCLOSE_YES_WITHOUT_PROMPT.
         */
        boolean badopt = FALSE;
        int idx, prefix_val;

        op = string_for_opt(opts, TRUE);
        if (op && negated) {
            bad_negation("disclose", TRUE);
            return FALSE;
        }
        /* "disclose" without a value means "all with prompting"
           and negated means "none without prompting" */
        if (!op || !strcmpi(op, "all") || !strcmpi(op, "none")) {
            if (op && !strcmpi(op, "none")) negated = TRUE;
            for (num = 0; num < NUM_DISCLOSURE_OPTIONS; num++)
                flags.end_disclose[num] = negated ?
                                          DISCLOSE_NO_WITHOUT_PROMPT :
                                          DISCLOSE_PROMPT_DEFAULT_YES;
            return retval;
        }

        num = 0;
        prefix_val = -1;
        while (*op && num < sizeof flags.end_disclose - 1) {
            register char c, *dop;
            static char valid_settings[] = {
                DISCLOSE_PROMPT_DEFAULT_YES,
                DISCLOSE_PROMPT_DEFAULT_NO,
                DISCLOSE_YES_WITHOUT_PROMPT,
                DISCLOSE_NO_WITHOUT_PROMPT,
                '\0'
            };
            c = lowc(*op);
            if (c == 'k') c = 'v';  /* killed -> vanquished */
            dop = index(disclosure_options, c);
            if (dop) {
                idx = dop - disclosure_options;
                if (idx < 0 || idx > NUM_DISCLOSURE_OPTIONS - 1) {
                    impossible("bad disclosure index %d %c",
                               idx, c);
                    continue;
                }
                if (prefix_val != -1) {
                    flags.end_disclose[idx] = prefix_val;
                    prefix_val = -1;
                } else
                    flags.end_disclose[idx] = DISCLOSE_YES_WITHOUT_PROMPT;
            } else if (index(valid_settings, c)) {
                prefix_val = c;
            } else if (c == ' ') {
                /* do nothing */
            } else
                badopt = TRUE;
            op++;
        }
        if (badopt) {
            badoption(opts);
            return FALSE;
        }
        return retval;
    }

    fullname = "seed";
    if (match_optname(opts, fullname, sizeof("seed")-1, TRUE)) {
#ifdef SYSCF
        if (sysopt.disable_user_seed) {
            return TRUE;
        }
#endif

        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if (!(op = string_for_opt(opts, FALSE))) {
            return FALSE;
        }
        unsigned int seed;
        /* base32 is marked by a leading u */
        if (*op == 'u') {
            seed = decode_base32(op);
        } else {
            seed = atoi(op);
        }
        init_random(seed);
        return retval;
    }

#ifdef PARANOID
    fullname = "conducts";
    if (match_optname(opts, fullname, 8, TRUE)) {
        common_prefix_options_parser(fullname, opts, negated);
        return retval;
    }
    /* things the player want an extended yes/no answer to */
    fullname = "paranoid";
    if (match_optname(opts, fullname, 8, TRUE)) {
        common_prefix_options_parser(fullname, opts, negated);
        return retval;
    }
#endif

    /* scores:5t[op] 5a[round] o[wn] */
    if (match_optname(opts, "scores", 4, TRUE)) {
        if (negated) {
            bad_negation("scores", FALSE);
            return FALSE;
        }
        if (!(op = string_for_opt(opts, FALSE))) {
            return FALSE;
        }

        while (*op) {
            int inum = 1;

            if (digit(*op)) {
                inum = atoi(op);
                while (digit(*op)) op++;
            } else if (*op == '!') {
                negated = !negated;
                op++;
            }
            while (*op == ' ') op++;

            switch (*op) {
            case 't':
            case 'T':  flags.end_top = inum;
                break;
            case 'a':
            case 'A':  flags.end_around = inum;
                break;
            case 'o':
            case 'O':  flags.end_own = !negated;
                break;

            default:
                badoption(opts);
                return FALSE;
            }
            while (letter(*++op) || *op == ' ') continue;
            if (*op == '/') op++;
        }
        return retval;
    }

    fullname = "statuscolor";
    if (match_optname(opts, fullname, 11, TRUE)) {
#if defined(STATUS_COLORS) && defined(TEXTCOLOR)
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0)
            if (!parse_status_color_options(op)) {
                badoption(opts);
                return FALSE;
            }
#endif
        return retval;
    }

#ifdef TTY_GRAPHICS
    fullname = "statuslines";
    if (match_optname(opts, fullname, 11, TRUE)) {
        op = string_for_opt(opts, negated);
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if (op && strlen(op) == 1 && (*op == '2' || *op == '3')) {
            /* LI isn't initialized yet on startup */
            int max_statuslines = (LI == 0 || LI > ROWNO+3) ? 3 : 2;
            iflags.statuslines = (*op == '2') ? 2 : max_statuslines;
            docrt();
        } else {
            badoption(opts);
            return FALSE;
        }
        return retval;
    }
#endif

#ifdef SORTLOOT
    fullname = "sortloot";
    if (match_optname(opts, fullname, 4, TRUE)) {
        op = string_for_env_opt(fullname, opts, FALSE);
        if (op) {
            switch (tolower(*op)) {
            case 'n':
            case 'l':
            case 'f': iflags.sortloot = tolower(*op);
                break;

            default:
                badoption(opts);
                return FALSE;
            }
        }
        return retval;
    }
#endif /* SORTLOOT */

#ifdef TTY_GRAPHICS
    fullname = "truecolor_separator";
    if (match_optname(opts, fullname, strlen(fullname), TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if (!(op = string_for_opt(opts, FALSE))) {
            return FALSE;
        }
        if (strlen(op) > 1 || ((*op) != ':' && (*op) != ';')) {
            badoption(opts);
            return FALSE;
        } else {
            iflags.truecolor_separator = (*op);
        }
        return retval;
    }
#endif

    fullname = "suppress_alert";
    if (match_optname(opts, fullname, 4, TRUE)) {
        op = string_for_opt(opts, negated);
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        else if (op) (void) feature_alert_opts(op, fullname);
        return retval;
    }

#ifdef VIDEOSHADES
    /* videocolors:string */
    fullname = "videocolors";
    if (match_optname(opts, fullname, 6, TRUE) ||
        match_optname(opts, "videocolours", 10, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        else if (!(opts = string_for_env_opt(fullname, opts, FALSE))) {
            return FALSE;
        }
        if (!assign_videocolors(opts)) {
            badoption(opts);
            return FALSE;
        }
        return retval;
    }
# ifdef MSDOS
    /* videoshades:string */
    fullname = "videoshades";
    if (match_optname(opts, fullname, 6, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        else if (!(opts = string_for_env_opt(fullname, opts, FALSE))) {
            return FALSE;
        }
        if (!assign_videoshades(opts)) {
            badoption(opts);
            return FALSE;
        }
        return retval;
    }
# endif
#endif /* VIDEOSHADES */
#ifdef MSDOS
# ifdef NO_TERMS
    /* video:string -- must be after longer tests */
    fullname = "video";
    if (match_optname(opts, fullname, 5, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        else if (!(opts = string_for_env_opt(fullname, opts, FALSE))) {
            return FALSE;
        }
        if (!assign_video(opts)) {
            badoption(opts);
            return FALSE;
        }

        return retval;
    }
# endif /* NO_TERMS */
    /* soundcard:string -- careful not to match boolean 'sound' */
    fullname = "soundcard";
    if (match_optname(opts, fullname, 6, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        else if (!(opts = string_for_env_opt(fullname, opts, FALSE))) {
            return FALSE;
        }
        if (!assign_soundcard(opts)) {
            badoption(opts);
            return FALSE;
        }
        return retval;
    }
#endif /* MSDOS */

    /* WINCAP
     * map_mode:[tiles|ascii4x6|ascii6x8|ascii8x8|ascii16x8|ascii7x12|ascii8x12|
            ascii16x12|ascii12x16|ascii10x18|fit_to_screen] */
    fullname = "map_mode";
    if (match_optname(opts, fullname, sizeof("map_mode")-1, TRUE)) {
        op = string_for_opt(opts, negated);
        if (op && !negated) {
            if (!strncmpi (op, "tiles", sizeof("tiles")-1))
                iflags.wc_map_mode = MAP_MODE_TILES;
            else if (!strncmpi (op, "ascii4x6", sizeof("ascii4x6")-1))
                iflags.wc_map_mode = MAP_MODE_ASCII4x6;
            else if (!strncmpi (op, "ascii6x8", sizeof("ascii6x8")-1))
                iflags.wc_map_mode = MAP_MODE_ASCII6x8;
            else if (!strncmpi (op, "ascii8x8", sizeof("ascii8x8")-1))
                iflags.wc_map_mode = MAP_MODE_ASCII8x8;
            else if (!strncmpi (op, "ascii16x8", sizeof("ascii16x8")-1))
                iflags.wc_map_mode = MAP_MODE_ASCII16x8;
            else if (!strncmpi (op, "ascii7x12", sizeof("ascii7x12")-1))
                iflags.wc_map_mode = MAP_MODE_ASCII7x12;
            else if (!strncmpi (op, "ascii8x12", sizeof("ascii8x12")-1))
                iflags.wc_map_mode = MAP_MODE_ASCII8x12;
            else if (!strncmpi (op, "ascii16x12", sizeof("ascii16x12")-1))
                iflags.wc_map_mode = MAP_MODE_ASCII16x12;
            else if (!strncmpi (op, "ascii12x16", sizeof("ascii12x16")-1))
                iflags.wc_map_mode = MAP_MODE_ASCII12x16;
            else if (!strncmpi (op, "ascii10x18", sizeof("ascii10x18")-1))
                iflags.wc_map_mode = MAP_MODE_ASCII10x18;
            else if (!strncmpi (op, "fit_to_screen", sizeof("fit_to_screen")-1))
                iflags.wc_map_mode = MAP_MODE_ASCII_FIT_TO_SCREEN;
            else {
                badoption(opts);
                return FALSE;
            }
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }
    /* WINCAP
     * scroll_amount:nn */
    fullname = "scroll_amount";
    if (match_optname(opts, fullname, sizeof("scroll_amount")-1, TRUE)) {
        op = string_for_opt(opts, negated);
        if ((negated && !op) || (!negated && op)) {
            iflags.wc_scroll_amount = negated ? 1 : atoi(op);
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }
    /* WINCAP
     * scroll_margin:nn */
    fullname = "scroll_margin";
    if (match_optname(opts, fullname, sizeof("scroll_margin")-1, TRUE)) {
        op = string_for_opt(opts, negated);
        if ((negated && !op) || (!negated && op)) {
            iflags.wc_scroll_margin = negated ? 5 : atoi(op);
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }
    fullname = "subkeyvalue";
    if (match_optname(opts, fullname, 5, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else {
#if defined(WIN32CON)
            op = string_for_opt(opts, 0);
            map_subkeyvalue(op);
#endif
        }
        return retval;
    }
    /* WINCAP
     * tile_width:nn */
    fullname = "tile_width";
    if (match_optname(opts, fullname, sizeof("tile_width")-1, TRUE)) {
        op = string_for_opt(opts, negated);
        if ((negated && !op) || (!negated && op)) {
            iflags.wc_tile_width = negated ? 0 : atoi(op);
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }
    /* WINCAP
     * tile_file:name */
    fullname = "tile_file";
    if (match_optname(opts, fullname, sizeof("tile_file")-1, TRUE)) {
        if ((op = string_for_opt(opts, FALSE)) != 0) {
            if (iflags.wc_tile_file) free(iflags.wc_tile_file);
            iflags.wc_tile_file = (char *)alloc(strlen(op) + 1);
            Strcpy(iflags.wc_tile_file, op);
        }
        return retval;
    }
    /* WINCAP
     * tile_height:nn */
    fullname = "tile_height";
    if (match_optname(opts, fullname, sizeof("tile_height")-1, TRUE)) {
        op = string_for_opt(opts, negated);
        if ((negated && !op) || (!negated && op)) {
            iflags.wc_tile_height = negated ? 0 : atoi(op);
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }
    /* WINCAP
     * vary_msgcount:nn */
    fullname = "vary_msgcount";
    if (match_optname(opts, fullname, sizeof("vary_msgcount")-1, TRUE)) {
        op = string_for_opt(opts, negated);
        if ((negated && !op) || (!negated && op)) {
            iflags.wc_vary_msgcount = negated ? 0 : atoi(op);
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }
    fullname = "windowtype";
    if (match_optname(opts, fullname, 3, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE)) != 0) {
            char buf[WINTYPELEN];
            nmcpy(buf, op, WINTYPELEN);
            choose_windows(buf);
        }
        return retval;
    }

    /* WINCAP
     * setting window colors
     * syntax: windowcolors=menu foregrnd/backgrnd text foregrnd/backgrnd
     */
    fullname = "windowcolors";
    if (match_optname(opts, fullname, 7, TRUE)) {
        if ((op = string_for_opt(opts, FALSE)) != 0) {
            if (!wc_set_window_colors(op)) {
                badoption(opts);
                return FALSE;
            }
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }

    /* WINCAP2
     * term_cols:amount */
    fullname = "term_cols";
    if (match_optname(opts, fullname, sizeof("term_cols")-1, TRUE)) {
        op = string_for_opt(opts, negated);
        iflags.wc2_term_cols = atoi(op);
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        return retval;
    }

    /* WINCAP2
     * term_rows:amount */
    fullname = "term_rows";
    if (match_optname(opts, fullname, sizeof("term_rows")-1, TRUE)) {
        op = string_for_opt(opts, negated);
        iflags.wc2_term_rows = atoi(op);
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        return retval;
    }

#ifdef CURSES_GRAPHICS
    /* WINCAP2
     * petattr:string */
    fullname = "petattr";
    if (match_optname(opts, fullname, sizeof("petattr")-1, TRUE)) {
        op = string_for_opt(opts, negated);
        if (op && !negated) {
            iflags.wc2_petattr = curses_read_attrs(op);
            if (!curses_read_attrs(op)) {
                badoption(opts);
                return FALSE;
            }
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        }
        return retval;
    }
#endif

    /* WINCAP2
     * windowborders:n */
    fullname = "windowborders";
    if (match_optname(opts, fullname, sizeof("windowborders")-1, TRUE)) {
        op = string_for_opt(opts, negated);
        if (negated && op) {
            bad_negation(fullname, TRUE);
            return FALSE;
        } else {
            if (negated)
                iflags.wc2_windowborders = 2; /* Off */
            else if (!op)
                iflags.wc2_windowborders = 1; /* On */
            else    /* Value supplied */
                iflags.wc2_windowborders = atoi(op);
            if ((iflags.wc2_windowborders > 3) ||
                (iflags.wc2_windowborders < 1)) {
                iflags.wc2_windowborders = 0;
                badoption(opts);
                return FALSE;
            }
        }
        return retval;
    }

    /* menustyle:traditional or combo or full or partial */
    if (match_optname(opts, "menustyle", 4, TRUE)) {
        int tmp;
        boolean val_required = (strlen(opts) > 5 && !negated);

        if (!(op = string_for_opt(opts, !val_required))) {
            if (val_required) {
                return FALSE; /* string_for_opt gave feedback */
            }
            tmp = negated ? 'n' : 'f';
        } else {
            tmp = tolower(*op);
        }
        switch (tmp) {
        case 'n':       /* none */
        case 't':       /* traditional */
            flags.menu_style = MENU_TRADITIONAL;
            break;
        case 'c':       /* combo: trad.class sel+menu */
            flags.menu_style = MENU_COMBINATION;
            break;
        case 'p':       /* partial: no class menu */
            flags.menu_style = MENU_PARTIAL;
            break;
        case 'f':       /* full: class menu + menu */
            flags.menu_style = MENU_FULL;
            break;
        default:
            badoption(opts);
            return FALSE;
        }
        return retval;
    }

    fullname = "menu_headings";
    if (match_optname(opts, fullname, 12, TRUE)) {
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        }
        else if (!(opts = string_for_env_opt(fullname, opts, FALSE))) {
            return FALSE;
        }
        if (!strcmpi(opts, "bold"))
            iflags.menu_headings = ATR_BOLD;
        else if (!strcmpi(opts, "inverse"))
            iflags.menu_headings = ATR_INVERSE;
        else if (!strcmpi(opts, "underline"))
            iflags.menu_headings = ATR_ULINE;
        else {
            badoption(opts);
            return FALSE;
        }
        return retval;
    }

    /* check for menu command mapping */
    for (i = 0; i < NUM_MENU_CMDS; i++) {
        fullname = default_menu_cmd_info[i].name;
        if (match_optname(opts, fullname, (int)strlen(fullname), TRUE)) {
            if (negated) {
                bad_negation(fullname, FALSE);
                return FALSE;
            } else if ((op = string_for_opt(opts, FALSE)) != 0) {
                int j;
                char c, op_buf[BUFSZ];
                boolean isbad = FALSE;

                escapes(op, op_buf);
                c = *op_buf;

                if (c == 0 || c == '\r' || c == '\n' || c == '\033' ||
                    c == ' ' || digit(c) || (letter(c) && c != '@'))
                    isbad = TRUE;
                else /* reject default object class symbols */
                    for (j = 1; j < MAXOCLASSES; j++)
                        if (c == def_oc_syms[i]) {
                            isbad = TRUE;
                            break;
                        }

                if (isbad) {
                    badoption(opts);
                    return FALSE;
                } else {
                    add_menu_cmd_alias(c, default_menu_cmd_info[i].cmd);
                }
            }
            return retval;
        }
    }

    /* OK, if we still haven't recognized the option, check the boolean
     * options list
     */
    for (i = 0; boolopt[i].name; i++) {
        if (match_optname(opts, boolopt[i].name, 3, FALSE)) {
            /* options that don't exist */
            if (!boolopt[i].addr) {
                if (!initial && !negated)
                    pline_The("\"%s\" option is not available.",
                              boolopt[i].name);
                return FALSE;
            }
            /* options that must come from config file */
            if (!initial && (boolopt[i].optflags == SET_IN_FILE)) {
                rejectoption(boolopt[i].name);
                return FALSE;
            }

            *(boolopt[i].addr) = !negated;

            duplicate_opt_detection(boolopt[i].name, 0);

#if defined(TERMLIB) || defined(ASCIIGRAPH) || defined(MAC_GRAPHICS_ENV) || defined(CURSES_GRAPHICS)
            if (FALSE
# ifdef TERMLIB
                || (boolopt[i].addr) == &iflags.DECgraphics
# endif
# ifdef ASCIIGRAPH
                || (boolopt[i].addr) == &iflags.IBMgraphics
# endif
# ifdef MAC_GRAPHICS_ENV
                || (boolopt[i].addr) == &iflags.MACgraphics
# endif
# ifdef CURSES_GRAPHICS
                || (boolopt[i].addr) == &iflags.cursesgraphics
# endif
# ifdef UTF8_GLYPHS
                || (boolopt[i].addr) == &iflags.UTF8graphics
# endif
                ) {
# ifdef REINCARNATION
                if (!initial && Is_rogue_level(&u.uz))
                    assign_rogue_graphics(FALSE);
# endif
                if (!initial && Is_moria_level(&u.uz)) {
                    assign_moria_graphics(FALSE);
                }

                need_redraw = TRUE;
# ifdef TERMLIB
                if ((boolopt[i].addr) == &iflags.DECgraphics)
                    switch_graphics(iflags.DECgraphics ?
                                    DEC_GRAPHICS : ASCII_GRAPHICS);
# endif
# ifdef ASCIIGRAPH
                if ((boolopt[i].addr) == &iflags.IBMgraphics)
                    switch_graphics(iflags.IBMgraphics ?
                                    IBM_GRAPHICS : ASCII_GRAPHICS);
# endif
# ifdef MAC_GRAPHICS_ENV
                if ((boolopt[i].addr) == &iflags.MACgraphics)
                    switch_graphics(iflags.MACgraphics ?
                                    MAC_GRAPHICS : ASCII_GRAPHICS);
# endif
# ifdef CURSES_GRAPHICS
                if ((boolopt[i].addr) == &iflags.cursesgraphics)
                    switch_graphics(iflags.cursesgraphics ?
                                    CURS_GRAPHICS : ASCII_GRAPHICS);
# endif
# ifdef UTF8_GLYPHS
                if ((boolopt[i].addr) == &iflags.UTF8graphics)
                    switch_graphics(iflags.UTF8graphics ?
                                    UTF8_GRAPHICS : ASCII_GRAPHICS);
# endif
# ifdef REINCARNATION
                if (!initial && Is_rogue_level(&u.uz))
                    assign_rogue_graphics(TRUE);
# endif
                if (!initial && Is_moria_level(&u.uz)) {
                    assign_moria_graphics(TRUE);
                }
            }
#endif /* TERMLIB || ASCIIGRAPH || MAC_GRAPHICS_ENV */

            /* only do processing below if setting with doset() */
            if (initial) {
                return FALSE;
            }

            if ((boolopt[i].addr) == &flags.time
#ifdef EXP_ON_BOTL
                || (boolopt[i].addr) == &flags.showexp
#endif
#ifdef SCORE_ON_BOTL
                || (boolopt[i].addr) == &flags.showscore
#endif
#ifdef SHOW_WEIGHT
                || (boolopt[i].addr) == &flags.showweight
#endif
                )
                flags.botl = TRUE;

            else if ((boolopt[i].addr) == &flags.invlet_constant) {
                if (flags.invlet_constant) reassign();
            }
#ifdef LAN_MAIL
            else if ((boolopt[i].addr) == &flags.biff) {
                if (flags.biff) lan_mail_init();
                else lan_mail_finish();
            }
#endif
            else if (((boolopt[i].addr) == &flags.lit_corridor) ||
                     ((boolopt[i].addr) == &iflags.dark_room)) {
                /*
                 * All corridor squares seen via night vision or
                 * candles & lamps change.  Update them by calling
                 * newsym() on them.  Don't do this if we are
                 * initializing the options --- the vision system
                 * isn't set up yet.
                 */
                vision_recalc(2);       /* shut down vision */
                vision_full_recalc = 1; /* delayed recalc */
            }
            else if ((boolopt[i].addr) == &iflags.use_inverse ||
                     (boolopt[i].addr) == &iflags.showrace ||
                     (boolopt[i].addr) == &iflags.hilite_pet ||
                     (boolopt[i].addr) == &iflags.wc2_guicolor) {
                need_redraw = TRUE;
            }
#ifdef CURSES_GRAPHICS
            else if ((boolopt[i].addr) == &iflags.cursesgraphics) {
                need_redraw = TRUE;
            }
#endif
#ifdef TEXTCOLOR
            else if ((boolopt[i].addr) == &iflags.use_color) {
                need_redraw = TRUE;
# ifdef TOS
                if ((boolopt[i].addr) == &iflags.use_color
                    && iflags.BIOS) {
                    if (colors_changed)
                        restore_colors();
                    else
                        set_colors();
                }
# endif
            }
#endif

            return retval;
        }
    }

    /* out of valid options */
    badoption(opts);
    return FALSE;
}


static NEARDATA const char *menutype[] = {
    "traditional", "combination", "partial", "full"
};

static NEARDATA const char *burdentype[] = {
    "unencumbered", "burdened", "stressed",
    "strained", "overtaxed", "overloaded"
};

static NEARDATA const char *runmodes[] = {
    "teleport", "run", "walk", "crawl"
};

#ifdef SORTLOOT
static NEARDATA const char *sortltype[] = {
    "none", "loot", "full"
};
#endif

/*
 * Convert the given string of object classes to a string of default object
 * symbols.
 */
STATIC_OVL void
oc_to_str(src, dest)
char *src, *dest;
{
    int i;

    while ((i = (int) *src++) != 0) {
        if (i < 0 || i >= MAXOCLASSES)
            impossible("oc_to_str:  illegal object class %d", i);
        else
            *dest++ = def_oc_syms[i];
    }
    *dest = '\0';
}

/*
 * Add the given mapping to the menu command map list.  Always keep the
 * maps valid C strings.
 */
void
add_menu_cmd_alias(from_ch, to_ch)
char from_ch, to_ch;
{
    if (n_menu_mapped >= MAX_MENU_MAPPED_CMDS)
        pline("out of menu map space.");
    else {
        mapped_menu_cmds[n_menu_mapped] = from_ch;
        mapped_menu_op[n_menu_mapped] = to_ch;
        n_menu_mapped++;
        mapped_menu_cmds[n_menu_mapped] = 0;
        mapped_menu_op[n_menu_mapped] = 0;
    }
}

/*
 * Map the given character to its corresponding menu command.  If it
 * doesn't match anything, just return the original.
 */
char
map_menu_cmd(ch)
char ch;
{
    char *found = index(mapped_menu_cmds, ch);
    if (found) {
        int idx = found - mapped_menu_cmds;
        ch = mapped_menu_op[idx];
    }
    return ch;
}


#if defined(MICRO) || defined(MAC) || defined(WIN32)
# define OPTIONS_HEADING "OPTIONS"
#else
# define OPTIONS_HEADING "NETHACKOPTIONS"
#endif

static char fmtstr_doset_add_menu[] = "%s%-15s [%s]   ";
static char fmtstr_doset_add_menu_tab[] = "%s\t[%s]";

STATIC_OVL void
doset_add_menu(win, option, indexoffset)
winid win;              /* window to add to */
const char *option;         /* option name */
int indexoffset;            /* value to add to index in compopt[], or zero
                               if option cannot be changed */
{
    const char *value = "unknown";      /* current value */
    char buf[BUFSZ], buf2[BUFSZ];
    anything any;
    int i;

    any.a_void = 0;
    if (indexoffset == 0) {
        any.a_int = 0;
        value = get_compopt_value(option, buf2);
    } else {
        for (i=0; compopt[i].name; i++)
            if (strcmp(option, compopt[i].name) == 0) break;

        if (compopt[i].name) {
            any.a_int = i + 1 + indexoffset;
            value = get_compopt_value(option, buf2);
        } else {
            /* We are trying to add an option not found in compopt[].
               This is almost certainly bad, but we'll let it through anyway
               (with a zero value, so it can't be selected). */
            any.a_int = 0;
        }
    }
    /* "    " replaces "a - " -- assumes menus follow that style */
    if (!iflags.menu_tab_sep)
        Sprintf(buf, fmtstr_doset_add_menu, any.a_int ? "" : "    ", option, value);
    else
        Sprintf(buf, fmtstr_doset_add_menu_tab, option, value);
    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
}

/* Changing options via menu by Per Liboriussen */
int
doset()
{
    char buf[BUFSZ*2], buf2[BUFSZ];
    int i, pass, boolcount, pick_cnt, pick_idx, opt_indx;
    boolean *bool_p;
    winid tmpwin;
    anything any;
    menu_item *pick_list;
    int indexoffset, startpass, endpass;
    boolean setinitial = FALSE, fromfile = FALSE;
    int biggest_name = 0;
    boolean istty = !strncmpi(windowprocs.name, "tty", 3);
    boolean ismswin = !strncmpi(windowprocs.name, "mswin", 5);

    if (iflags.debug_fuzzer) {
        return 0;
    }

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);

    any.a_void = 0;
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings,
             "Booleans (selecting will toggle value):", MENU_UNSELECTED);
    any.a_int = 0;
    /* first list any other non-modifiable booleans, then modifiable ones */
    for (pass = 0; pass <= 1; pass++)
        for (i = 0; boolopt[i].name; i++)
            if ((bool_p = boolopt[i].addr) != 0 &&
                ((boolopt[i].optflags == DISP_IN_GAME && pass == 0) ||
                 (boolopt[i].optflags == SET_IN_GAME && pass == 1))) {
                if (bool_p == &flags.female) continue; /* obsolete */
#ifdef WIZARD
                if (bool_p == &iflags.sanity_check && !wizard) continue;
                if (bool_p == &iflags.menu_tab_sep && !wizard) continue;
                if (bool_p == &iflags.showdmg && !wizard) continue;
#endif
                /* hide options that are useless in tty*/
                if (istty) {
                    if (bool_p == &iflags.perm_invent) {
                        continue;
                    }
#ifdef CURSES_GRAPHICS
                    if (bool_p == &iflags.cursesgraphics) continue;
#endif
                    if (bool_p == &iflags.wc_popup_dialog) continue;
                } else {
                    /* only implemented for tty */
                    if (bool_p == &flags.hitpointbar) continue;
                }
                if (ismswin) {
                    if (bool_p == &iflags.use_inverse) continue;
                    /* nobody would want to disable these in mswin */
                    if (bool_p == &iflags.hilite_pet) continue;
                }

                if (is_wc_option(boolopt[i].name) &&
                    !wc_supported(boolopt[i].name)) continue;
                if (is_wc2_option(boolopt[i].name) &&
                    !wc2_supported(boolopt[i].name)) continue;
                any.a_int = (pass == 0) ? 0 : i + 1;
                if (!iflags.menu_tab_sep)
                    Sprintf(buf, "%s%-14s [%s]",
                            pass == 0 ? "    " : "",
                            boolopt[i].name, *bool_p ? "true" : "false");
                else
                    Sprintf(buf, "%s\t[%s]",
                            boolopt[i].name, *bool_p ? "true" : "false");
                add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0,
                         ATR_NONE, buf, MENU_UNSELECTED);
            }

    boolcount = i;
    indexoffset = boolcount;
    any.a_void = 0;
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings,
             "Compounds (selecting will prompt for new value):",
             MENU_UNSELECTED);

    startpass = DISP_IN_GAME;
    endpass = SET_IN_GAME;

    /* spin through the options to find the biggest name
           and adjust the format string accordingly if needed */
    biggest_name = 0;
    for (i = 0; compopt[i].name; i++)
        if (compopt[i].optflags >= startpass && compopt[i].optflags <= endpass &&
            strlen(compopt[i].name) > (unsigned) biggest_name)
            biggest_name = (int) strlen(compopt[i].name);
    if (biggest_name > 30) biggest_name = 30;
    if (!iflags.menu_tab_sep)
        Sprintf(fmtstr_doset_add_menu, "%%s%%-%us [%%s]", biggest_name);

    /* deliberately put `name', `role', `race', `gender' first */
    doset_add_menu(tmpwin, "name", 0);
    doset_add_menu(tmpwin, "role", 0);
    doset_add_menu(tmpwin, "race", 0);
    doset_add_menu(tmpwin, "gender", 0);

    for (pass = startpass; pass <= endpass; pass++) {
        for (i = 0; compopt[i].name; i++) {
            if (compopt[i].optflags == pass) {
                if (!strcmp(compopt[i].name, "name") ||
                    !strcmp(compopt[i].name, "role") ||
                    !strcmp(compopt[i].name, "race") ||
                    !strcmp(compopt[i].name, "gender"))
                    continue;
                else if (is_wc_option(compopt[i].name) &&
                         !wc_supported(compopt[i].name))
                    continue;
                else if (is_wc2_option(compopt[i].name) &&
                         !wc2_supported(compopt[i].name))
                    continue;
                else
                    doset_add_menu(tmpwin, compopt[i].name,
                                   (pass == DISP_IN_GAME) ? 0 : indexoffset);
            }
        }
    }

    any = zeroany;
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", MENU_ITEMFLAGS_NONE);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings,
             "Other settings:", MENU_ITEMFLAGS_NONE);

#ifdef AUTOPICKUP_EXCEPTIONS
    any.a_int = -1;
    Sprintf(buf, "autopickup exceptions (%d currently set)", count_apes());
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
#endif /* AUTOPICKUP_EXCEPTIONS */

#ifdef MENU_COLOR
    any.a_int = -2;
    Sprintf(buf, "message colors        (%d currently set)", count_menucolors());
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
#endif /* AUTOPICKUP_EXCEPTIONS */

#ifdef PREFIXES_IN_USE
    any.a_void = 0;
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings,
             "Variable playground locations:", MENU_UNSELECTED);
    for (i = 0; i < PREFIX_COUNT; i++)
        doset_add_menu(tmpwin, fqn_prefix_names[i], 0);
#endif
    end_menu(tmpwin, "Set what options?");
    need_redraw = FALSE;
    if ((pick_cnt = select_menu(tmpwin, PICK_ANY, &pick_list)) > 0) {
        /*
         * Walk down the selection list and either invert the booleans
         * or prompt for new values. In most cases, call parseoptions()
         * to take care of options that require special attention, like
         * redraws.
         */
        for (pick_idx = 0; pick_idx < pick_cnt; ++pick_idx) {
            opt_indx = pick_list[pick_idx].item.a_int - 1;
#ifdef AUTOPICKUP_EXCEPTIONS
            if (opt_indx == -2) {
                special_handling("autopickup_exception",
                                 setinitial, fromfile);
            } else
#endif
#ifdef MENU_COLOR
            if (opt_indx == -3) {
                special_handling("menu colors", setinitial, fromfile);
            } else
#endif
            if (opt_indx < boolcount) {
                /* boolean option */
                Sprintf(buf, "%s%s", *boolopt[opt_indx].addr ? "!" : "",
                        boolopt[opt_indx].name);
                parseoptions(buf, setinitial, fromfile);
                if (wc_supported(boolopt[opt_indx].name) ||
                    wc2_supported(boolopt[opt_indx].name))
                    preference_update(boolopt[opt_indx].name);
            } else {
                /* compound option */
                opt_indx -= boolcount;

                if (!special_handling(compopt[opt_indx].name,
                                      setinitial, fromfile)) {
                    Sprintf(buf, "Set %s to what?", compopt[opt_indx].name);
                    getlin(buf, buf2);
                    if (buf2[0] == '\033')
                        continue;
                    Sprintf(buf, "%s:%s", compopt[opt_indx].name, buf2);
                    /* pass the buck */
                    parseoptions(buf, setinitial, fromfile);
                }
                if (wc_supported(compopt[opt_indx].name) ||
                    wc2_supported(compopt[opt_indx].name))
                    preference_update(compopt[opt_indx].name);
            }
        }
        free((genericptr_t)pick_list);
        pick_list = (menu_item *)0;
    }

    destroy_nhwindow(tmpwin);
    if (need_redraw) {
        (void) doredraw();
    }
    return 0;
}

STATIC_OVL boolean
special_handling(optname, setinitial, setfromfile)
const char *optname;
boolean setinitial, setfromfile;
{
    winid tmpwin;
    anything any;
    int i;
    char buf[BUFSZ];
    boolean retval = FALSE;

    /* Special handling of menustyle, pickup_burden, pickup_types,
     * disclose, runmode, msg_window, menu_headings, number_pad and sortloot
     #ifdef AUTOPICKUP_EXCEPTIONS
     * Also takes care of interactive autopickup_exception_handling changes.
     #endif
     */
    if (!strcmp("menustyle", optname)) {
        const char *style_name;
        menu_item *style_pick = (menu_item *)0;
        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < SIZE(menutype); i++) {
            style_name = menutype[i];
            /* note: separate `style_name' variable used
               to avoid an optimizer bug in VAX C V2.3 */
            any.a_int = i + 1;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, *style_name, 0,
                     ATR_NONE, style_name, MENU_UNSELECTED);
        }
        end_menu(tmpwin, "Select menustyle:");
        if (select_menu(tmpwin, PICK_ONE, &style_pick) > 0) {
            flags.menu_style = style_pick->item.a_int - 1;
            free((genericptr_t)style_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = TRUE;
    } else if (!strcmp("pickup_burden", optname)) {
        const char *burden_name, *burden_letters = "ubsntl";
        menu_item *burden_pick = (menu_item *)0;
        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < SIZE(burdentype); i++) {
            burden_name = burdentype[i];
            any.a_int = i + 1;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, burden_letters[i],
                     0, ATR_NONE, burden_name, MENU_UNSELECTED);
        }
        end_menu(tmpwin, "Select encumbrance level:");
        if (select_menu(tmpwin, PICK_ONE, &burden_pick) > 0) {
            flags.pickup_burden = burden_pick->item.a_int - 1;
            free((genericptr_t)burden_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = TRUE;
    } else if (!strcmp("pickup_types", optname)) {
        /* parseoptions will prompt for the list of types */
        parseoptions(strcpy(buf, "pickup_types"), setinitial, setfromfile);
        retval = TRUE;
#ifdef PARANOID
    } else if (!strcmp("paranoid", optname)) {
        int pick_cnt, pick_idx, opt_idx;
        menu_item *paranoid_category_pick = (menu_item *)0;

        static const char *paranoid_names[] = {
            "hit", "quit", "remove", "trap", "lava", "water"
        };
    #define NUM_PARANOID_OPTIONS SIZE(paranoid_names)
        static boolean *paranoid_bools[NUM_PARANOID_OPTIONS];
        paranoid_bools[0] = &iflags.paranoid_hit;
        paranoid_bools[1] = &iflags.paranoid_quit;
        paranoid_bools[2] = &iflags.paranoid_remove;
        paranoid_bools[3] = &iflags.paranoid_trap;
        paranoid_bools[4] = &iflags.paranoid_lava;
        paranoid_bools[5] = &iflags.paranoid_water;
        int paranoid_settings[NUM_PARANOID_OPTIONS];

        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < NUM_PARANOID_OPTIONS; i++) {
            any.a_int = i + 1;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, paranoid_names[i][0], 0,
                     ATR_NONE, paranoid_names[i],
                     *paranoid_bools[i] ? MENU_SELECTED : MENU_UNSELECTED);
            paranoid_settings[i] = 0;
        }
        end_menu(tmpwin, "Change which paranoid settings:");
        if ((pick_cnt = select_menu(tmpwin, PICK_ANY, &paranoid_category_pick)) > 0) {
            for (pick_idx = 0; pick_idx < pick_cnt; ++pick_idx) {
                opt_idx = paranoid_category_pick[pick_idx].item.a_int - 1;
                paranoid_settings[opt_idx] = 1;
            }
            free((genericptr_t)paranoid_category_pick);
            paranoid_category_pick = (menu_item *)0;
        }
        destroy_nhwindow(tmpwin);

        iflags.paranoid_hit = paranoid_settings[0];
        iflags.paranoid_quit = paranoid_settings[1];
        iflags.paranoid_remove = paranoid_settings[2];
        iflags.paranoid_trap = paranoid_settings[3];
        iflags.paranoid_lava = paranoid_settings[4];
        iflags.paranoid_water = paranoid_settings[5];

        retval = TRUE;
#endif
    } else if (!strcmp("disclose", optname)) {
        int pick_cnt, pick_idx, opt_idx;
        menu_item *disclosure_category_pick = (menu_item *)0;
        /*
         * The order of disclose_names[]
         * must correspond to disclosure_options in decl.h
         */
        static const char *disclosure_names[] = {
            "inventory", "attributes", "vanquished", "genocides", "conduct"
        };
        int disc_cat[NUM_DISCLOSURE_OPTIONS];
        const char *disclosure_name;

        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < NUM_DISCLOSURE_OPTIONS; i++) {
            disclosure_name = disclosure_names[i];
            any.a_int = i + 1;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, disclosure_options[i], 0,
                     ATR_NONE, disclosure_name, MENU_UNSELECTED);
            disc_cat[i] = 0;
        }
        end_menu(tmpwin, "Change which disclosure options categories:");
        if ((pick_cnt = select_menu(tmpwin, PICK_ANY, &disclosure_category_pick)) > 0) {
            for (pick_idx = 0; pick_idx < pick_cnt; ++pick_idx) {
                opt_idx = disclosure_category_pick[pick_idx].item.a_int - 1;
                disc_cat[opt_idx] = 1;
            }
            free((genericptr_t)disclosure_category_pick);
            disclosure_category_pick = (menu_item *)0;
        }
        destroy_nhwindow(tmpwin);

        for (i = 0; i < NUM_DISCLOSURE_OPTIONS; i++) {
            if (disc_cat[i]) {
                char dbuf[BUFSZ];
                menu_item *disclosure_option_pick = (menu_item *)0;
                Sprintf(dbuf, "Disclosure options for %s:", disclosure_names[i]);
                tmpwin = create_nhwindow(NHW_MENU);
                start_menu(tmpwin);
                any.a_char = DISCLOSE_NO_WITHOUT_PROMPT;
                add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'a', 0,
                         ATR_NONE, "Never disclose and don't prompt", MENU_UNSELECTED);
                any.a_void = 0;
                any.a_char = DISCLOSE_YES_WITHOUT_PROMPT;
                add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'b', 0,
                         ATR_NONE, "Always disclose and don't prompt", MENU_UNSELECTED);
                any.a_void = 0;
                any.a_char = DISCLOSE_PROMPT_DEFAULT_NO;
                add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'c', 0,
                         ATR_NONE, "Prompt and default answer to \"No\"", MENU_UNSELECTED);
                any.a_void = 0;
                any.a_char = DISCLOSE_PROMPT_DEFAULT_YES;
                add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'd', 0,
                         ATR_NONE, "Prompt and default answer to \"Yes\"", MENU_UNSELECTED);
                end_menu(tmpwin, dbuf);
                if (select_menu(tmpwin, PICK_ONE, &disclosure_option_pick) > 0) {
                    flags.end_disclose[i] = disclosure_option_pick->item.a_char;
                    free((genericptr_t)disclosure_option_pick);
                }
                destroy_nhwindow(tmpwin);
            }
        }
        retval = TRUE;
#ifdef REALTIME_ON_BOTL
    } else if (!strcmp("realtime", optname)) {
        const char *mode_name;
        menu_item *mode_pick = (menu_item *)0;
        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < SIZE(realtime_type_strings); i++) {
            mode_name = realtime_type_strings[i];
            any.a_int = i + 1;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, *mode_name, 0,
                     ATR_NONE, mode_name, MENU_UNSELECTED);
        }
        end_menu(tmpwin, "Type of time to show on status bar:");
        if (select_menu(tmpwin, PICK_ONE, &mode_pick) > 0) {
            iflags.showrealtime = mode_pick->item.a_int - 1;
            free(mode_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = TRUE;
    } else if (!strcmp("realtime_format", optname)) {
        const char *mode_name;
        menu_item *mode_pick = (menu_item *)0;
        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < SIZE(realtime_format_strings); i++) {
            mode_name = realtime_format_strings[i];
            any.a_int = i + 1;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, *mode_name, 0,
                     ATR_NONE, mode_name, MENU_UNSELECTED);
        }
        end_menu(tmpwin, "Format used by realtime display:");
        if (select_menu(tmpwin, PICK_ONE, &mode_pick) > 0) {
            iflags.realtime_format = mode_pick->item.a_int - 1;
            free(mode_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = TRUE;
#endif
    } else if (!strcmp("runmode", optname)) {
        const char *mode_name;
        menu_item *mode_pick = (menu_item *)0;
        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < SIZE(runmodes); i++) {
            mode_name = runmodes[i];
            any.a_int = i + 1;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, *mode_name, 0,
                     ATR_NONE, mode_name, MENU_UNSELECTED);
        }
        end_menu(tmpwin, "Select run/travel display mode:");
        if (select_menu(tmpwin, PICK_ONE, &mode_pick) > 0) {
            iflags.runmode = mode_pick->item.a_int - 1;
            free((genericptr_t)mode_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = TRUE;
    } else if (!strcmp("whatis_coord", optname)) {
        menu_item *window_pick = (menu_item *) 0;
        int pick_cnt;
        char gp = iflags.getpos_coords;

        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        any = zeroany;
        any.a_char = GPCOORDS_COMPASS;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, GPCOORDS_COMPASS, 0, ATR_NONE,
                 "compass ('east' or '3s' or '2n,4w')",
                 (gp == GPCOORDS_COMPASS) ? MENU_ITEMFLAGS_SELECTED : MENU_ITEMFLAGS_NONE);
        any.a_char = GPCOORDS_COMFULL;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, GPCOORDS_COMFULL, 0, ATR_NONE,
                 "full compass ('east' or '3south' or '2north,4west')",
                 (gp == GPCOORDS_COMFULL) ? MENU_ITEMFLAGS_SELECTED : MENU_ITEMFLAGS_NONE);
        any.a_char = GPCOORDS_MAP;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, GPCOORDS_MAP, 0, ATR_NONE,
                 "map <x,y>",
                 (gp == GPCOORDS_MAP) ? MENU_ITEMFLAGS_SELECTED : MENU_ITEMFLAGS_NONE);
        any.a_char = GPCOORDS_SCREEN;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, GPCOORDS_SCREEN, 0, ATR_NONE,
                 "screen [row,column]",
                 (gp == GPCOORDS_SCREEN) ? MENU_ITEMFLAGS_SELECTED : MENU_ITEMFLAGS_NONE);
        any.a_char = GPCOORDS_NONE;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, GPCOORDS_NONE, 0, ATR_NONE,
                 "none (no coordinates displayed)",
                 (gp == GPCOORDS_NONE) ? MENU_ITEMFLAGS_SELECTED : MENU_ITEMFLAGS_NONE);
        any.a_long = 0L;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", MENU_ITEMFLAGS_NONE);
        Sprintf(buf, "map: upper-left: <%d,%d>, lower-right: <%d,%d>%s",
                1, 0, COLNO - 1, ROWNO - 1,
                flags.verbose ? "; column 0 unused, off left edge" : "");
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, MENU_ITEMFLAGS_NONE);
        if (strcmp(windowprocs.name, "tty")) {
            /* only show for non-tty */
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                     "screen: row is offset to accommodate tty interface's use of top line",
                     MENU_ITEMFLAGS_NONE);
        }
#if COLNO == 80
#define COL80ARG flags.verbose ? "; column 80 is not used" : ""
#else
#define COL80ARG ""
#endif
        Sprintf(buf, "screen: upper-left: [%02d,%02d], lower-right: [%d,%d]%s",
                0 + 2, 1, ROWNO - 1 + 2, COLNO - 1, COL80ARG);
#undef COL80ARG
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, MENU_ITEMFLAGS_NONE);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", MENU_ITEMFLAGS_NONE);
        end_menu(tmpwin,
            "Select coordinate display when auto-describing a map position:");
        if ((pick_cnt = select_menu(tmpwin, PICK_ONE, &window_pick)) > 0) {
            iflags.getpos_coords = window_pick[0].item.a_char;
            /* PICK_ONE doesn't unselect preselected entry when
               selecting another one */
            if (pick_cnt > 1 && iflags.getpos_coords == gp) {
                iflags.getpos_coords = window_pick[1].item.a_char;
            }
            free(window_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = TRUE;
    } else if (!strcmp("whatis_filter", optname)) {
        menu_item *window_pick = (menu_item *) 0;
        int pick_cnt;
        char gf = iflags.getloc_filter;

        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        any = zeroany;
        any.a_char = (GFILTER_NONE + 1);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'n', 0, ATR_NONE, "no filtering",
                 (gf == GFILTER_NONE) ? MENU_ITEMFLAGS_SELECTED : MENU_ITEMFLAGS_NONE);
        any.a_char = (GFILTER_VIEW + 1);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'v', 0, ATR_NONE, "in view only",
                 (gf == GFILTER_VIEW) ? MENU_ITEMFLAGS_SELECTED : MENU_ITEMFLAGS_NONE);
        any.a_char = (GFILTER_AREA + 1);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'a', 0, ATR_NONE, "in same area",
                 (gf == GFILTER_AREA) ? MENU_ITEMFLAGS_SELECTED : MENU_ITEMFLAGS_NONE);
        end_menu(tmpwin, "Select location filtering when going for next/previous map position:");
        if ((pick_cnt = select_menu(tmpwin, PICK_ONE, &window_pick)) > 0) {
            iflags.getloc_filter = (window_pick[0].item.a_char - 1);
            /* PICK_ONE doesn't unselect preselected entry when
               selecting another one */
            if (pick_cnt > 1 && iflags.getloc_filter == gf) {
                iflags.getloc_filter = (window_pick[1].item.a_char - 1);
            }
            free(window_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = TRUE;
    }
#ifdef TTY_GRAPHICS
    else if (!strcmp("msg_window", optname)) {
        /* by Christian W. Cooper */
        menu_item *window_pick = (menu_item *)0;
        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        any.a_char = 's';
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 's', 0,
                 ATR_NONE, "single", MENU_UNSELECTED);
        any.a_char = 'c';
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'c', 0,
                 ATR_NONE, "combination", MENU_UNSELECTED);
        any.a_char = 'f';
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'f', 0,
                 ATR_NONE, "full", MENU_UNSELECTED);
        any.a_char = 'r';
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'r', 0,
                 ATR_NONE, "reversed", MENU_UNSELECTED);
        end_menu(tmpwin, "Select message history display type:");
        if (select_menu(tmpwin, PICK_ONE, &window_pick) > 0) {
            iflags.prevmsg_window = window_pick->item.a_char;
            free((genericptr_t)window_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = TRUE;
#ifdef SORTLOOT
    } else if (!strcmp("sortloot", optname)) {
        const char *sortl_name;
        menu_item *sortl_pick = (menu_item *)0;
        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < SIZE(sortltype); i++) {
            sortl_name = sortltype[i];
            any.a_char = *sortl_name;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, *sortl_name, 0,
                     ATR_NONE, sortl_name, MENU_UNSELECTED);
        }
        end_menu(tmpwin, "Select loot sorting type:");
        if (select_menu(tmpwin, PICK_ONE, &sortl_pick) > 0) {
            iflags.sortloot = sortl_pick->item.a_char;
            free((genericptr_t)sortl_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = TRUE;
#endif
    }
#endif
    else if (!strcmp("align_message", optname) ||
             !strcmp("align_status", optname)) {
        menu_item *window_pick = (menu_item *)0;
        char abuf[BUFSZ];
        boolean msg = (*(optname+6) == 'm');

        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        any.a_int = ALIGN_TOP;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 't', 0,
                 ATR_NONE, "top", MENU_UNSELECTED);
        any.a_int = ALIGN_BOTTOM;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'b', 0,
                 ATR_NONE, "bottom", MENU_UNSELECTED);
        any.a_int = ALIGN_LEFT;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'l', 0,
                 ATR_NONE, "left", MENU_UNSELECTED);
        any.a_int = ALIGN_RIGHT;
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 'r', 0,
                 ATR_NONE, "right", MENU_UNSELECTED);
        Sprintf(abuf, "Select %s window placement relative to the map:",
                msg ? "message" : "status");
        end_menu(tmpwin, abuf);
        if (select_menu(tmpwin, PICK_ONE, &window_pick) > 0) {
            if (msg) iflags.wc_align_message = window_pick->item.a_int;
            else iflags.wc_align_status = window_pick->item.a_int;
            free((genericptr_t)window_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = TRUE;
    } else if (!strcmp("number_pad", optname)) {
        static const char *npchoices[3] =
        {"0 (off)", "1 (on)", "2 (on, DOS compatible)"};
        const char *npletters = "abc";
        menu_item *mode_pick = (menu_item *)0;

        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < SIZE(npchoices); i++) {
            any.a_int = i + 1;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, npletters[i], 0,
                     ATR_NONE, npchoices[i], MENU_UNSELECTED);
        }
        end_menu(tmpwin, "Select number_pad mode:");
        if (select_menu(tmpwin, PICK_ONE, &mode_pick) > 0) {
            int mode = mode_pick->item.a_int - 1;
            switch(mode) {
            case 2:
                Cmd.num_pad = iflags.num_pad = 1;
                iflags.num_pad_mode = 1;
                break;
            case 1:
                Cmd.num_pad = iflags.num_pad = 1;
                iflags.num_pad_mode = 0;
                break;
            case 0:
            default:
                Cmd.num_pad = iflags.num_pad = 0;
                iflags.num_pad_mode = 0;
            }
            reset_commands(FALSE);
            number_pad(iflags.num_pad ? 1 : 0);
            free((genericptr_t)mode_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = TRUE;
    } else if (!strcmp("menu_headings", optname)) {
        static const char *mhchoices[3] = {"bold", "inverse", "underline"};
        const char *npletters = "biu";
        menu_item *mode_pick = (menu_item *)0;

        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
        for (i = 0; i < SIZE(mhchoices); i++) {
            any.a_int = i + 1;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, npletters[i], 0,
                     ATR_NONE, mhchoices[i], MENU_UNSELECTED);
        }
        end_menu(tmpwin, "How to highlight menu headings:");
        if (select_menu(tmpwin, PICK_ONE, &mode_pick) > 0) {
            int mode = mode_pick->item.a_int - 1;
            switch(mode) {
            case 2:
                iflags.menu_headings = ATR_ULINE;
                break;
            case 0:
                iflags.menu_headings = ATR_BOLD;
                break;
            case 1:
            default:
                iflags.menu_headings = ATR_INVERSE;
            }
            free((genericptr_t)mode_pick);
        }
        destroy_nhwindow(tmpwin);
        retval = TRUE;
#ifdef AUTOPICKUP_EXCEPTIONS
    } else if (!strcmp("autopickup_exception", optname)) {
        handler_autopickup_exception();
#endif
#ifdef MENU_COLOR
    } else if (!strcmp("menu colors", optname)) {
        handler_menu_colors();
#endif
    }
    return retval;
}

#define rolestring(val, array, field) ((val >= 0) ? array[val].field : \
                                       (val == ROLE_RANDOM) ? randomrole : none)

/* This is ugly. We have all the option names in the compopt[] array,
   but we need to look at each option individually to get the value. */
STATIC_OVL const char *
get_compopt_value(optname, buf)
const char *optname;
char *buf;
{
    char ocl[MAXOCLASSES+1];
    static const char none[] = "(none)", randomrole[] = "random",
                      to_be_done[] = "(to be done)",
                      defopt[] = "default",
                      defbrief[] = "def";
    int i;

    buf[0] = '\0';
    if (!strcmp(optname, "align_message"))
        Sprintf(buf, "%s", iflags.wc_align_message == ALIGN_TOP     ? "top" :
                iflags.wc_align_message == ALIGN_LEFT    ? "left" :
                iflags.wc_align_message == ALIGN_BOTTOM  ? "bottom" :
                iflags.wc_align_message == ALIGN_RIGHT   ? "right" :
                defopt);
    else if (!strcmp(optname, "align_status"))
        Sprintf(buf, "%s", iflags.wc_align_status == ALIGN_TOP     ? "top" :
                iflags.wc_align_status == ALIGN_LEFT    ? "left" :
                iflags.wc_align_status == ALIGN_BOTTOM  ? "bottom" :
                iflags.wc_align_status == ALIGN_RIGHT   ? "right" :
                defopt);
    else if (!strcmp(optname, "align"))
        Sprintf(buf, "%s", rolestring(flags.initalign, aligns, adj));
#ifdef WIN32CON
    else if (!strcmp(optname, "altkeyhandler"))
        Sprintf(buf, "%s", iflags.altkeyhandler[0] ?
                iflags.altkeyhandler : "default");
#endif
    else if (!strcmp(optname, "boulder"))
        Sprintf(buf, "%c", iflags.bouldersym ?
                iflags.bouldersym : oc_syms[(int)objects[BOULDER].oc_class]);
    else if (!strcmp(optname, "catname"))
        Sprintf(buf, "%s", catname[0] ? catname : none );
    else if (!strcmp(optname, "conducts"))
        Sprintf(buf, "%s%s %s%s %s%s %s%s %s%s %s%s %s%s %s%s",
                flags.ascet ? "+" : "-", "ascet",
                flags.atheist ? "+" : "-", "atheist",
                flags.blindfolded ? "+" : "-", "blindfolded",
                flags.illiterate ? "+" : "-", "illiterate",
                flags.nudist ? "+" : "-", "nudist",
                flags.pacifist ? "+" : "-", "pacifist",
                flags.vegan ? "+" : "-", "vegan",
                flags.vegetarian ? "+" : "-", "vegetarian");
#ifdef EXOTIC_PETS
    else if (!strcmp(optname, "crocodilename"))
        Sprintf(buf, "%s", crocodilename[0] ? crocodilename : none);
#endif
    else if (!strcmp(optname, "disclose")) {
        for (i = 0; i < NUM_DISCLOSURE_OPTIONS; i++) {
            char topt[2];
            if (i) Strcat(buf, " ");
            topt[1] = '\0';
            topt[0] = flags.end_disclose[i];
            Strcat(buf, topt);
            topt[0] = disclosure_options[i];
            Strcat(buf, topt);
        }
    }
    else if (!strcmp(optname, "dogname"))
        Sprintf(buf, "%s", dogname[0] ? dogname : none );
#ifdef DUMP_LOG
    else if (!strcmp(optname, "dumpfile"))
        Sprintf(buf, "%s", dump_fn[0] ? dump_fn : none );
#endif
    else if (!strcmp(optname, "hp_notify_fmt"))
        Sprintf(buf, "%s", iflags.hp_notify_fmt ? iflags.hp_notify_fmt : "[HP%c%a=%h]" );
    else if (!strcmp(optname, "dungeon"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "effects"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "font_map"))
        Sprintf(buf, "%s", iflags.wc_font_map ? iflags.wc_font_map : defopt);
    else if (!strcmp(optname, "font_message"))
        Sprintf(buf, "%s", iflags.wc_font_message ? iflags.wc_font_message : defopt);
    else if (!strcmp(optname, "font_status"))
        Sprintf(buf, "%s", iflags.wc_font_status ? iflags.wc_font_status : defopt);
    else if (!strcmp(optname, "font_menu"))
        Sprintf(buf, "%s", iflags.wc_font_menu ? iflags.wc_font_menu : defopt);
    else if (!strcmp(optname, "font_text"))
        Sprintf(buf, "%s", iflags.wc_font_text ? iflags.wc_font_text : defopt);
    else if (!strcmp(optname, "font_size_map")) {
        if (iflags.wc_fontsiz_map) Sprintf(buf, "%d", iflags.wc_fontsiz_map);
        else Strcpy(buf, defopt);
    }
    else if (!strcmp(optname, "font_size_message")) {
        if (iflags.wc_fontsiz_message) Sprintf(buf, "%d",
                                               iflags.wc_fontsiz_message);
        else Strcpy(buf, defopt);
    }
    else if (!strcmp(optname, "font_size_status")) {
        if (iflags.wc_fontsiz_status) Sprintf(buf, "%d", iflags.wc_fontsiz_status);
        else Strcpy(buf, defopt);
    }
    else if (!strcmp(optname, "font_size_menu")) {
        if (iflags.wc_fontsiz_menu) Sprintf(buf, "%d", iflags.wc_fontsiz_menu);
        else Strcpy(buf, defopt);
    }
    else if (!strcmp(optname, "font_size_text")) {
        if (iflags.wc_fontsiz_text) Sprintf(buf, "%d", iflags.wc_fontsiz_text);
        else Strcpy(buf, defopt);
    }
    else if (!strcmp(optname, "fruit"))
        Sprintf(buf, "%s", pl_fruit);
    else if (!strcmp(optname, "gender"))
        Sprintf(buf, "%s", rolestring(flags.initgend, genders, adj));
    else if (!strcmp(optname, "horsename"))
        Sprintf(buf, "%s", horsename[0] ? horsename : none);
    else if (!strcmp(optname, "map_mode"))
        Sprintf(buf, "%s",
                iflags.wc_map_mode == MAP_MODE_TILES      ? "tiles" :
                iflags.wc_map_mode == MAP_MODE_ASCII4x6   ? "ascii4x6" :
                iflags.wc_map_mode == MAP_MODE_ASCII6x8   ? "ascii6x8" :
                iflags.wc_map_mode == MAP_MODE_ASCII8x8   ? "ascii8x8" :
                iflags.wc_map_mode == MAP_MODE_ASCII16x8  ? "ascii16x8" :
                iflags.wc_map_mode == MAP_MODE_ASCII7x12  ? "ascii7x12" :
                iflags.wc_map_mode == MAP_MODE_ASCII8x12  ? "ascii8x12" :
                iflags.wc_map_mode == MAP_MODE_ASCII16x12 ? "ascii16x12" :
                iflags.wc_map_mode == MAP_MODE_ASCII12x16 ? "ascii12x16" :
                iflags.wc_map_mode == MAP_MODE_ASCII10x18 ? "ascii10x18" :
                iflags.wc_map_mode == MAP_MODE_ASCII_FIT_TO_SCREEN ?
                "fit_to_screen" : defopt);
    else if (!strcmp(optname, "menustyle"))
        Sprintf(buf, "%s", menutype[(int)flags.menu_style] );
    else if (!strcmp(optname, "menu_deselect_all"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_deselect_page"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_first_page"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_invert_all"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_headings")) {
        Sprintf(buf, "%s", (iflags.menu_headings == ATR_BOLD) ?
                "bold" :   (iflags.menu_headings == ATR_INVERSE) ?
                "inverse" :   (iflags.menu_headings == ATR_ULINE) ?
                "underline" : "unknown");
    }
    else if (!strcmp(optname, "menu_invert_page"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_last_page"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_next_page"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_previous_page"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_search"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_select_all"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "menu_select_page"))
        Sprintf(buf, "%s", to_be_done);
#ifdef EXOTIC_PETS
    else if (!strcmp(optname, "monkeyname"))
        Sprintf(buf, "%s", monkeyname[0] ? monkeyname : none);
#endif
    else if (!strcmp(optname, "monsters"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "msghistory"))
        Sprintf(buf, "%u", iflags.msg_history);
#ifdef TTY_GRAPHICS
    else if (!strcmp(optname, "msg_window"))
        Sprintf(buf, "%s", (iflags.prevmsg_window=='s') ? "single" :
                (iflags.prevmsg_window=='c') ? "combination" :
                (iflags.prevmsg_window=='f') ? "full" : "reversed");
#endif
    else if (!strcmp(optname, "name"))
        Sprintf(buf, "%s", plname);
    else if (!strcmp(optname, "nameempty"))
        Sprintf(buf, "%s", iflags.nameempty ? iflags.nameempty : none );
    else if (!strcmp(optname, "number_pad"))
        Sprintf(buf, "%s",
                (!iflags.num_pad) ? "0=off" :
                (iflags.num_pad_mode) ? "2=on, DOS compatible" : "1=on");
    else if (!strcmp(optname, "objects"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "packorder")) {
        oc_to_str(flags.inv_order, ocl);
        Sprintf(buf, "%s", ocl);
    }
#ifdef CHANGE_COLOR
    else if (!strcmp(optname, "palette"))
        Sprintf(buf, "%s", get_color_string());
#endif
#ifdef PARANOID
    else if (!strcmp(optname, "paranoid"))
        Sprintf(buf, "%s%s %s%s %s%s %s%s %s%s %s%s",
                iflags.paranoid_hit ? "+" : "-", "hit",
                iflags.paranoid_quit ? "+" : "-", "quit",
                iflags.paranoid_remove ? "+" : "-", "remove",
                iflags.paranoid_trap ? "+" : "-", "trap",
                iflags.paranoid_lava ? "+" : "-", "lava",
                iflags.paranoid_water ? "+" : "-", "water");
#endif
    else if (!strcmp(optname, "pettype"))
        Sprintf(buf, "%s", (preferred_pet == 'c') ? "cat" :
                (preferred_pet == 'd') ? "dog" :
                (preferred_pet == 'e') ? "exotic" :
                (preferred_pet == 'n') ? "none" : "random");
    else if (!strcmp(optname, "pickup_burden"))
        Sprintf(buf, "%s", burdentype[flags.pickup_burden] );
    else if (!strcmp(optname, "pickup_types")) {
        oc_to_str(flags.pickup_types, ocl);
        Sprintf(buf, "%s", ocl[0] ? ocl : "all" );
    }
    else if (!strcmp(optname, "pilesize")) {
        Sprintf(buf, "%u", iflags.pilesize);
    }
    else if (!strcmp(optname, "race"))
        Sprintf(buf, "%s", rolestring(flags.initrace, races, noun));
#ifdef CONVICT
    else if (!strcmp(optname, "ratname"))
        Sprintf(buf, "%s", ratname[0] ? catname : none );
#endif /* CONVICT */
#ifdef REALTIME_ON_BOTL
    else if (!strcmp(optname, "realtime")) {
        Sprintf(buf, "%s", realtime_type_strings[iflags.showrealtime]);
    } else if (!strcmp(optname, "realtime_format")) {
        Sprintf(buf, "%s", realtime_format_strings[iflags.realtime_format]);
    }
#endif
    else if (!strcmp(optname, "role"))
        Sprintf(buf, "%s", rolestring(flags.initrole, roles, name.m));
    else if (!strcmp(optname, "runmode"))
        Sprintf(buf, "%s", runmodes[iflags.runmode]);
    else if (!strcmp(optname, "whatis_coord")) {
        Sprintf(buf, "%s",
                (iflags.getpos_coords == GPCOORDS_MAP) ? "map" :
                (iflags.getpos_coords == GPCOORDS_COMPASS) ? "compass" :
                (iflags.getpos_coords == GPCOORDS_COMFULL) ? "full compass" :
                (iflags.getpos_coords == GPCOORDS_SCREEN) ? "screen" : "none");
    } else if (!strcmp(optname, "whatis_filter")) {
        Sprintf(buf, "%s",
                (iflags.getloc_filter == GFILTER_VIEW) ? "view" :
                (iflags.getloc_filter == GFILTER_AREA) ? "area" : "none");
    }
    else if (!strcmp(optname, "scores")) {
        Sprintf(buf, "%d top/%d around%s", flags.end_top,
                flags.end_around, flags.end_own ? "/own" : "");
    }
    else if (!strcmp(optname, "scroll_amount")) {
        if (iflags.wc_scroll_amount) Sprintf(buf, "%d", iflags.wc_scroll_amount);
        else Strcpy(buf, defopt);
    }
    else if (!strcmp(optname, "scroll_margin")) {
        if (iflags.wc_scroll_margin) Sprintf(buf, "%d", iflags.wc_scroll_margin);
        else Strcpy(buf, defopt);
    }
#ifdef SORTLOOT
    else if (!strcmp(optname, "sortloot")) {
        char *sortname = (char *)NULL;
        for (i=0; i < SIZE(sortltype) && sortname==(char *)NULL; i++) {
            if (iflags.sortloot == sortltype[i][0])
                sortname = (char *)sortltype[i];
        }
        if (sortname != (char *)NULL)
            Sprintf(buf, "%s", sortname);
    }
#endif
    else if (!strcmp(optname, "player_selection"))
        Sprintf(buf, "%s", iflags.wc_player_selection ? "prompts" : "dialog");
#ifdef MSDOS
    else if (!strcmp(optname, "soundcard"))
        Sprintf(buf, "%s", to_be_done);
#endif
#ifdef TTY_GRAPHICS
    else if (!strcmp(optname, "statuslines"))
        Sprintf(buf, "%d", iflags.statuslines);
#endif
    else if (!strcmp(optname, "suppress_alert")) {
        if (flags.suppress_alert == 0L)
            Strcpy(buf, none);
        else
            Sprintf(buf, "%lu.%lu.%lu",
                    FEATURE_NOTICE_VER_MAJ,
                    FEATURE_NOTICE_VER_MIN,
                    FEATURE_NOTICE_VER_PATCH);
    }
    else if (!strcmp(optname, "term_cols")) {
        if (iflags.wc2_term_cols) Sprintf(buf, "%d", iflags.wc2_term_cols);
        else Strcpy(buf, defopt);
    }
    else if (!strcmp(optname, "term_rows")) {
        if (iflags.wc2_term_rows) Sprintf(buf, "%d", iflags.wc2_term_rows);
        else Strcpy(buf, defopt);
    }
    else if (!strcmp(optname, "tile_file"))
        Sprintf(buf, "%s", iflags.wc_tile_file ? iflags.wc_tile_file : defopt);
    else if (!strcmp(optname, "tile_height")) {
        if (iflags.wc_tile_height) Sprintf(buf, "%d", iflags.wc_tile_height);
        else Strcpy(buf, defopt);
    }
    else if (!strcmp(optname, "tile_width")) {
        if (iflags.wc_tile_width) Sprintf(buf, "%d", iflags.wc_tile_width);
        else Strcpy(buf, defopt);
    }
    else if (!strcmp(optname, "traps"))
        Sprintf(buf, "%s", to_be_done);
    else if (!strcmp(optname, "vary_msgcount")) {
        if (iflags.wc_vary_msgcount) Sprintf(buf, "%d", iflags.wc_vary_msgcount);
        else Strcpy(buf, defopt);
    }
#ifdef MSDOS
    else if (!strcmp(optname, "video"))
        Sprintf(buf, "%s", to_be_done);
#endif
#ifdef VIDEOSHADES
# ifdef MSDOS
    else if (!strcmp(optname, "videoshades"))
        Sprintf(buf, "%s-%s-%s", shade[0], shade[1], shade[2]);
    else if (!strcmp(optname, "videocolors"))
        Sprintf(buf, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",
                ttycolors[CLR_RED], ttycolors[CLR_GREEN],
                ttycolors[CLR_BROWN], ttycolors[CLR_BLUE],
                ttycolors[CLR_MAGENTA], ttycolors[CLR_CYAN],
                ttycolors[CLR_ORANGE], ttycolors[CLR_BRIGHT_GREEN],
                ttycolors[CLR_YELLOW], ttycolors[CLR_BRIGHT_BLUE],
                ttycolors[CLR_BRIGHT_MAGENTA],
                ttycolors[CLR_BRIGHT_CYAN]);
# else
    else if (!strcmp(optname, "videocolors"))
        Sprintf(buf, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",
                ttycolors[CLR_RED], ttycolors[CLR_GREEN],
                ttycolors[CLR_BROWN], ttycolors[CLR_BLUE],
                ttycolors[CLR_MAGENTA], ttycolors[CLR_CYAN],
                ttycolors[CLR_GRAY], ttycolors[CLR_BLACK],
                ttycolors[CLR_ORANGE], ttycolors[CLR_BRIGHT_GREEN],
                ttycolors[CLR_YELLOW], ttycolors[CLR_BRIGHT_BLUE],
                ttycolors[CLR_BRIGHT_MAGENTA],
                ttycolors[CLR_BRIGHT_CYAN], ttycolors[CLR_WHITE]);
# endif /* MSDOS */
#endif /* VIDEOSHADES */
    else if (!strcmp(optname, "windowborders"))
        Sprintf(buf, "%s", iflags.wc2_windowborders == 1     ? "1=on" :
                iflags.wc2_windowborders == 2             ? "2=off" :
                iflags.wc2_windowborders == 3             ? "3=auto" :
                defopt);
    else if (!strcmp(optname, "windowtype"))
        Sprintf(buf, "%s", windowprocs.name);
    else if (!strcmp(optname, "windowcolors"))
        Sprintf(buf, "%s/%s %s/%s %s/%s %s/%s",
                iflags.wc_foregrnd_menu    ? iflags.wc_foregrnd_menu : defbrief,
                iflags.wc_backgrnd_menu    ? iflags.wc_backgrnd_menu : defbrief,
                iflags.wc_foregrnd_message ? iflags.wc_foregrnd_message : defbrief,
                iflags.wc_backgrnd_message ? iflags.wc_backgrnd_message : defbrief,
                iflags.wc_foregrnd_status  ? iflags.wc_foregrnd_status : defbrief,
                iflags.wc_backgrnd_status  ? iflags.wc_backgrnd_status : defbrief,
                iflags.wc_foregrnd_text    ? iflags.wc_foregrnd_text : defbrief,
                iflags.wc_backgrnd_text    ? iflags.wc_backgrnd_text : defbrief);
#ifdef PREFIXES_IN_USE
#ifdef EXOTIC_PETS
    else if (!strcmp(optname, "wolfname"))
        Sprintf(buf, "%s", wolfname[0] ? wolfname : none);
#endif
    else {
        for (i = 0; i < PREFIX_COUNT; ++i)
            if (!strcmp(optname, fqn_prefix_names[i]) && fqn_prefix[i])
                Sprintf(buf, "%s", fqn_prefix[i]);
    }
#endif

    if (buf[0]) return buf;
    else return "unknown";
}

/* common to msg-types, menu-colors, autopickup-exceptions */
static int
handle_add_list_remove(optname, numtotal)
const char *optname;
int numtotal;
{
    winid tmpwin;
    anything any;
    int i, pick_cnt, opt_idx;
    menu_item *pick_list = (menu_item *) 0;
    static const struct action {
        char letr;
        const char *desc;
    } action_titles[] = {
        { 'a', "add new %s" },         /* [0] */
        { 'l', "list %s" },            /* [1] */
        { 'r', "remove existing %s" }, /* [2] */
        { 'x', "exit this menu" },     /* [3] */
    };

    opt_idx = 0;
    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);
    any = zeroany;
    for (i = 0; i < SIZE(action_titles); i++) {
        char tmpbuf[BUFSZ];

        any.a_int++;
        /* omit list and remove if there aren't any yet */
        if (!numtotal && (i == 1 || i == 2)) {
            continue;
        }
        Sprintf(tmpbuf, action_titles[i].desc, (i == 1) ? makeplural(optname) : optname);

        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, action_titles[i].letr, 0, ATR_NONE,
                 tmpbuf, (i == 3) ? MENU_ITEMFLAGS_SELECTED : MENU_ITEMFLAGS_NONE);
    }
    end_menu(tmpwin, "Do what?");
    if ((pick_cnt = select_menu(tmpwin, PICK_ONE, &pick_list)) > 0) {
        opt_idx = pick_list[0].item.a_int - 1;
        if (pick_cnt > 1 && opt_idx == 3) {
            opt_idx = pick_list[1].item.a_int - 1;
        }
        free(pick_list);
    } else {
        opt_idx = 3; /* none selected, exit menu */
    }
    destroy_nhwindow(tmpwin);
    return opt_idx;
}

int
dotogglepickup()
{
    char buf[BUFSZ], ocl[MAXOCLASSES+1];

    flags.pickup = !flags.pickup;
    if (flags.pickup) {
        oc_to_str(flags.pickup_types, ocl);
        Sprintf(buf, "ON, for %s objects%s", ocl[0] ? ocl : "all",
#ifdef AUTOPICKUP_EXCEPTIONS
                (apelist) ? ((count_apes() == 1) ? ", with one exception" : ", with some exceptions") :
#endif
                "");
    } else {
        Strcpy(buf, "OFF");
    }
    pline("Autopickup: %s.", buf);
    return 0;
}

#ifdef AUTOPICKUP_EXCEPTIONS
int
add_autopickup_exception(mapping)
const char *mapping;
{
    static const char APE_regex_error[] = "regex error in AUTOPICKUP_EXCEPTION";
    static const char APE_syntax_error[] = "syntax error in AUTOPICKUP_EXCEPTION";

    char text[256], end;
    int n;
    boolean grab = FALSE;

    /* scan length limit used to be 255, but smaller size allows the
       quoted value to fit within BUFSZ, simplifying formatting elsewhere;
       this used to ignore the possibility of trailing junk but now checks
       for it, accepting whitespace but rejecting anything else unless it
       starts with '#" for a comment */
    end = '\0';
    if (((n = sscanf(mapping, "\"<%253[^\"]\" %c", text, &end)) == 1) ||
         (n == 2 && end == '#')) {
        grab = TRUE;
    } else if (((n = sscanf(mapping, "\">%253[^\"]\" %c", text, &end)) == 1) ||
               ((n = sscanf(mapping, "\"%253[^\"]\" %c", text, &end)) == 1) ||
                (n == 2 && end == '#')) {
        grab = FALSE;
    } else {
        config_error_add("%s", APE_syntax_error);
        return 0;
    }

    struct autopickup_exception *ape = (struct autopickup_exception *) alloc(sizeof *ape);
    ape->regex = regex_init();
    if (!regex_compile(text, ape->regex)) {
        const char *re_error_desc = regex_error_desc(ape->regex);

        /* free first in case reason for failure was insufficient memory */
        regex_free(ape->regex);
        free(ape);
        config_error_add("%s: %s", APE_regex_error, re_error_desc);
        return 0;
    }
    ape->pattern = dupstr(text);
    ape->grab = grab;
    ape->next = apelist;
    apelist = ape;
    return 1;
}

static void
remove_autopickup_exception(whichape)
struct autopickup_exception *whichape;
{
    struct autopickup_exception *ape, *freeape, *prev = 0;

    for (ape = apelist; ape;) {
        if (ape == whichape) {
            freeape = ape;
            ape = ape->next;
            if (prev) {
                prev->next = ape;
            } else {
                apelist = ape;
            }
            regex_free(freeape->regex);
            free(freeape->pattern);
            free(freeape);
        } else {
            prev = ape;
            ape = ape->next;
        }
    }
}

void
free_autopickup_exceptions()
{
    struct autopickup_exception *ape;

    while ((ape = apelist) != 0) {
        free(ape->pattern);
        regex_free(ape->regex);
        apelist = ape->next;
        free(ape);
    }
}

int
count_apes()
{
    int numapes = 0;
    struct autopickup_exception *ape = apelist;

    while (ape) {
      numapes++;
      ape = ape->next;
    }

    return numapes;
}

#endif /* AUTOPICKUP_EXCEPTIONS */

/* data for option_help() */
static const char *opt_intro[] = {
    "",
    "                 NetHack Options Help:",
    "",
#define CONFIG_SLOT 3   /* fill in next value at run-time */
    (char *)0,
#if !defined(MICRO) && !defined(MAC)
    "or use `NETHACKOPTIONS=\"<options>\"' in your environment",
#endif
    "(<options> is a list of options separated by commas)",
#ifdef VMS
    "-- for example, $ DEFINE NETHACKOPTIONS \"noautopickup,fruit:kumquat\"",
#endif
    "or press \"O\" while playing and use the menu.",
    "",
    "Boolean options (which can be negated by prefixing them with '!' or \"no\"):",
    (char *)0
};

static const char *opt_epilog[] = {
    "",
    "Some of the options can be set only before the game is started; those",
    "items will not be selectable in the 'O' command's menu.",
    (char *)0
};

void
option_help()
{
    char buf[BUFSZ], buf2[BUFSZ];
    register int i;
    winid datawin;

    datawin = create_nhwindow(NHW_TEXT);
    Sprintf(buf, "Set options as OPTIONS=<options> in %s", configfile);
    opt_intro[CONFIG_SLOT] = (const char *) buf;
    for (i = 0; opt_intro[i]; i++)
        putstr(datawin, 0, opt_intro[i]);

    /* Boolean options */
    for (i = 0; boolopt[i].name; i++) {
        if (boolopt[i].addr) {
#ifdef WIZARD
            if (boolopt[i].addr == &iflags.sanity_check && !wizard) continue;
            if (boolopt[i].addr == &iflags.menu_tab_sep && !wizard) continue;
#endif
            next_opt(datawin, boolopt[i].name);
        }
    }
    next_opt(datawin, "");

    /* Compound options */
    putstr(datawin, 0, "Compound options:");
    for (i = 0; compopt[i].name; i++) {
        Sprintf(buf2, "`%s'", compopt[i].name);
        Snprintf(buf, sizeof(buf), "%-20s - %s%c", buf2, compopt[i].descr,
                 compopt[i + 1].name ? ',' : '.');
        putstr(datawin, 0, buf);
    }

    for (i = 0; opt_epilog[i]; i++)
        putstr(datawin, 0, opt_epilog[i]);

    display_nhwindow(datawin, FALSE);
    destroy_nhwindow(datawin);
    return;
}

/*
 * prints the next boolean option, on the same line if possible, on a new
 * line if not. End with next_opt("").
 */
void
next_opt(datawin, str)
winid datawin;
const char *str;
{
    static char *buf = 0;
    int i;
    char *s;

    if (!buf) *(buf = (char *)alloc(BUFSZ)) = '\0';

    if (!*str) {
        s = eos(buf);
        if (s > &buf[1] && s[-2] == ',')
            Strcpy(s - 2, "."); /* replace last ", " */
        i = COLNO;  /* (greater than COLNO - 2) */
    } else {
        i = strlen(buf) + strlen(str) + 2;
    }

    if (i > COLNO - 2) { /* rule of thumb */
        putstr(datawin, 0, buf);
        buf[0] = 0;
    }
    if (*str) {
        Strcat(buf, str);
        Strcat(buf, ", ");
    } else {
        putstr(datawin, 0, str);
        free(buf),  buf = 0;
    }
    return;
}

/** Check if character c is illegal as a menu command key */
boolean
illegal_menu_cmd_key(c)
char c;
{
    if (c == 0 || c == '\r' || c == '\n' || c == '\033' || c == ' ' ||
         digit(c) || (letter(c) && c != '@')) {
        config_error_add("Reserved menu command key '%s'", visctrl(c));
        return TRUE;
    } else { /* reject default object class symbols */
        int j;
        for (j = 1; j < MAXOCLASSES; j++) {
            if (c == def_oc_syms[j]) {
                config_error_add("Menu command key '%s' is an object class", visctrl(c));
                return TRUE;
            }
        }
    }
    return FALSE;
}

/* Returns the fid of the fruit type; if that type already exists, it
 * returns the fid of that one; if it does not exist, it adds a new fruit
 * type to the chain and returns the new one.
 */
int
fruitadd(str)
char *str;
{
    register int i;
    register struct fruit *f;
    struct fruit *lastf = 0;
    int highest_fruit_id = 0;
    char buf[PL_FSIZ];
    boolean user_specified = (str == pl_fruit);
    /* if not user-specified, then it's a fruit name for a fruit on
     * a bones level or from orctown raider's loot...
     */

    /* Note: every fruit has an id (spe for fruit objects) of at least
     * 1; 0 is an error.
     */
    if (user_specified) {
        /* disallow naming after other foods (since it'd be impossible
         * to tell the difference)
         */

        boolean found = FALSE, numeric = FALSE;

        for (i = bases[FOOD_CLASS]; objects[i].oc_class == FOOD_CLASS;
             i++) {
            if (!strcmp(OBJ_NAME(objects[i]), pl_fruit)) {
                found = TRUE;
                break;
            }
        }
        {
            char *c;

            c = pl_fruit;

            for(c = pl_fruit; *c >= '0' && *c <= '9'; c++)
                ;
            if (isspace(*c) || *c == 0) numeric = TRUE;
        }
        if (found || numeric ||
            !strncmp(str, "cursed ", 7) ||
            !strncmp(str, "uncursed ", 9) ||
            !strncmp(str, "blessed ", 8) ||
            !strncmp(str, "partly eaten ", 13) ||
            (!strncmp(str, "tin of ", 7) &&
             (!strcmp(str+7, "spinach") ||
              name_to_mon(str+7) >= LOW_PM)) ||
            !strcmp(str, "empty tin") ||
            ((str_end_is(pl_fruit, " corpse") ||
              str_end_is(pl_fruit, " egg")) &&
             name_to_mon(str) >= LOW_PM))
        {
            Strcpy(buf, pl_fruit);
            Strcpy(pl_fruit, "candied ");
            nmcpy(pl_fruit+8, buf, PL_FSIZ-8);
        }
    }
    for(f=ffruit; f; f = f->nextf) {
        lastf = f;
        if(f->fid > highest_fruit_id) highest_fruit_id = f->fid;
        if(!strncmp(str, f->fname, PL_FSIZ))
            goto nonew;
    }
    /* if adding another fruit would overflow spe, use a random
       fruit instead... we've got a lot to choose from. */
    if (highest_fruit_id >= 127) return rnd(127);
    highest_fruit_id++;
    f = newfruit();
    if (ffruit) lastf->nextf = f;
    else ffruit = f;
    Strcpy(f->fname, str);
    f->fid = highest_fruit_id;
    f->nextf = 0;
nonew:
    if (user_specified) current_fruit = highest_fruit_id;
    return f->fid;
}

/*
 * This is a somewhat generic menu for taking a list of NetHack style
 * class choices and presenting them via a description
 * rather than the traditional NetHack characters.
 * (Benefits users whose first exposure to NetHack is via tiles).
 *
 * prompt
 *       The title at the top of the menu.
 *
 * category: 0 = monster class
 *           1 = object  class
 *
 * way
 *       FALSE = PICK_ONE, TRUE = PICK_ANY
 *
 * class_list
 *       a null terminated string containing the list of choices.
 *
 * class_selection
 *       a null terminated string containing the selected characters.
 *
 * Returns number selected.
 */
int
choose_classes_menu(prompt, category, way, class_list, class_select)
const char *prompt;
int category;
boolean way;
char *class_list;
char *class_select;
{
    menu_item *pick_list = (menu_item *)0;
    winid win;
    anything any;
    char buf[BUFSZ];
    int i, n;
    int ret;
    int next_accelerator, accelerator;

    if (class_list == (char *)0 || class_select == (char *)0) return 0;
    accelerator = 0;
    next_accelerator = 'a';
    any.a_void = 0;
    win = create_nhwindow(NHW_MENU);
    start_menu(win);
    while (*class_list) {
        const char *text;
        boolean selected;

        text = (char *)0;
        selected = FALSE;
        switch (category) {
        case 0:
            text = monexplain[def_char_to_monclass(*class_list)];
            accelerator = *class_list;
            Sprintf(buf, "%s", text);
            break;
        case 1:
            text = objexplain[def_char_to_objclass(*class_list)];
            accelerator = next_accelerator;
            Sprintf(buf, "%c  %s", *class_list, text);
            break;
        default:
            impossible("choose_classes_menu: invalid category %d",
                       category);
        }
        if (way && *class_select) { /* Selections there already */
            if (index(class_select, *class_list)) {
                selected = TRUE;
            }
        }
        any.a_int = *class_list;
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, accelerator,
                 category ? *class_list : 0,
                 ATR_NONE, buf, selected);
        ++class_list;
        if (category > 0) {
            ++next_accelerator;
            if (next_accelerator == ('z' + 1)) next_accelerator = 'A';
            if (next_accelerator == ('Z' + 1)) break;
        }
    }
    end_menu(win, prompt);
    n = select_menu(win, way ? PICK_ANY : PICK_ONE, &pick_list);
    destroy_nhwindow(win);
    if (n > 0) {
        for (i = 0; i < n; ++i)
            *class_select++ = (char)pick_list[i].item.a_int;
        free((genericptr_t)pick_list);
        ret = n;
    } else if (n == -1) {
        class_select = eos(class_select);
        ret = -1;
    } else
        ret = 0;
    *class_select = '\0';
    return ret;
}

struct wc_Opt wc_options[] = {
    {"ascii_map", WC_ASCII_MAP},
    {"color", WC_COLOR},
    {"eight_bit_tty", WC_EIGHT_BIT_IN},
    {"hilite_pet", WC_HILITE_PET},
    {"popup_dialog", WC_POPUP_DIALOG},
    {"player_selection", WC_PLAYER_SELECTION},
    {"preload_tiles", WC_PRELOAD_TILES},
    {"tiled_map", WC_TILED_MAP},
    {"tile_file", WC_TILE_FILE},
    {"tile_width", WC_TILE_WIDTH},
    {"tile_height", WC_TILE_HEIGHT},
    {"use_inverse", WC_INVERSE},
    {"align_message", WC_ALIGN_MESSAGE},
    {"align_status", WC_ALIGN_STATUS},
    {"font_map", WC_FONT_MAP},
    {"font_menu", WC_FONT_MENU},
    {"font_message", WC_FONT_MESSAGE},
#if 0
    {"perm_invent", WC_PERM_INVENT},
#endif
    {"font_size_map", WC_FONTSIZ_MAP},
    {"font_size_menu", WC_FONTSIZ_MENU},
    {"font_size_message", WC_FONTSIZ_MESSAGE},
    {"font_size_status", WC_FONTSIZ_STATUS},
    {"font_size_text", WC_FONTSIZ_TEXT},
    {"font_status", WC_FONT_STATUS},
    {"font_text", WC_FONT_TEXT},
    {"map_mode", WC_MAP_MODE},
    {"scroll_amount", WC_SCROLL_AMOUNT},
    {"scroll_margin", WC_SCROLL_MARGIN},
    {"splash_screen", WC_SPLASH_SCREEN},
    {"vary_msgcount", WC_VARY_MSGCOUNT},
    {"windowcolors", WC_WINDOWCOLORS},
    {"mouse_support", WC_MOUSE_SUPPORT},
    {(char *)0, 0L}
};

struct wc_Opt wc2_options[] = {
    {"fullscreen", WC2_FULLSCREEN},
    {"newcolors", WC2_NEWCOLORS},
    {"softkeyboard", WC2_SOFTKEYBOARD},
    {"wraptext", WC2_WRAPTEXT},
    {"term_cols", WC2_TERM_COLS},
    {"term_rows", WC2_TERM_ROWS},
    {"windowborders", WC2_WINDOWBORDERS},
    {"petattr", WC2_PETATTR},
    {"guicolor", WC2_GUICOLOR},
    {(char *)0, 0L}
};


/*
 * If a port wants to change or ensure that the
 * SET_IN_FILE, DISP_IN_GAME, or SET_IN_GAME status of an option is
 * correct (for controlling its display in the option menu) call
 * set_option_mod_status()
 * with the second argument of 0,2, or 3 respectively.
 */
void
set_option_mod_status(optnam, status)
const char *optnam;
int status;
{
    int k;
    if (status < SET_IN_FILE || status > SET_IN_GAME) {
        impossible("set_option_mod_status: status out of range %d.",
                   status);
        return;
    }
    for (k = 0; boolopt[k].name; k++) {
        if (!strncmpi(boolopt[k].name, optnam, strlen(optnam))) {
            boolopt[k].optflags = status;
            return;
        }
    }
    for (k = 0; compopt[k].name; k++) {
        if (!strncmpi(compopt[k].name, optnam, strlen(optnam))) {
            compopt[k].optflags = status;
            return;
        }
    }
}

/*
 * You can set several wc_options in one call to
 * set_wc_option_mod_status() by setting
 * the appropriate bits for each option that you
 * are setting in the optmask argument
 * prior to calling.
 *    example: set_wc_option_mod_status(WC_COLOR|WC_SCROLL_MARGIN, SET_IN_GAME);
 */
void
set_wc_option_mod_status(optmask, status)
unsigned long optmask;
int status;
{
    int k = 0;
    if (status < SET_IN_FILE || status > SET_IN_GAME) {
        impossible("set_wc_option_mod_status: status out of range %d.",
                   status);
        return;
    }
    while (wc_options[k].wc_name) {
        if (optmask & wc_options[k].wc_bit) {
            set_option_mod_status(wc_options[k].wc_name, status);
        }
        k++;
    }
}

STATIC_OVL boolean
is_wc_option(optnam)
const char *optnam;
{
    int k = 0;
    while (wc_options[k].wc_name) {
        if (strcmp(wc_options[k].wc_name, optnam) == 0)
            return TRUE;
        k++;
    }
    return FALSE;
}

STATIC_OVL boolean
wc_supported(optnam)
const char *optnam;
{
    int k = 0;
    while (wc_options[k].wc_name) {
        if (!strcmp(wc_options[k].wc_name, optnam) &&
            (windowprocs.wincap & wc_options[k].wc_bit))
            return TRUE;
        k++;
    }
    return FALSE;
}


/*
 * You can set several wc2_options in one call to
 * set_wc2_option_mod_status() by setting
 * the appropriate bits for each option that you
 * are setting in the optmask argument
 * prior to calling.
 *    example: set_wc2_option_mod_status(WC2_FULLSCREEN|WC2_SOFTKEYBOARD|WC2_WRAPTEXT, SET_IN_FILE);
 */

void
set_wc2_option_mod_status(optmask, status)
unsigned long optmask;
int status;
{
    int k = 0;
    if (status < SET_IN_FILE || status > SET_IN_GAME) {
        impossible("set_wc2_option_mod_status: status out of range %d.",
                   status);
        return;
    }
    while (wc2_options[k].wc_name) {
        if (optmask & wc2_options[k].wc_bit) {
            set_option_mod_status(wc2_options[k].wc_name, status);
        }
        k++;
    }
}

STATIC_OVL boolean
is_wc2_option(optnam)
const char *optnam;
{
    int k = 0;
    while (wc2_options[k].wc_name) {
        if (strcmp(wc2_options[k].wc_name, optnam) == 0)
            return TRUE;
        k++;
    }
    return FALSE;
}

STATIC_OVL boolean
wc2_supported(optnam)
const char *optnam;
{
    int k = 0;
    while (wc2_options[k].wc_name) {
        if (!strcmp(wc2_options[k].wc_name, optnam) &&
            (windowprocs.wincap2 & wc2_options[k].wc_bit))
            return TRUE;
        k++;
    }
    return FALSE;
}


STATIC_OVL void
wc_set_font_name(wtype, fontname)
int wtype;
char *fontname;
{
    char **fn = (char **)0;
    if (!fontname) return;
    switch(wtype) {
    case NHW_MAP:
        fn = &iflags.wc_font_map;
        break;
    case NHW_MESSAGE:
        fn = &iflags.wc_font_message;
        break;
    case NHW_TEXT:
        fn = &iflags.wc_font_text;
        break;
    case NHW_MENU:
        fn = &iflags.wc_font_menu;
        break;
    case NHW_STATUS:
        fn = &iflags.wc_font_status;
        break;
    default:
        return;
    }
    if (fn) {
        if (*fn) free(*fn);
        *fn = (char *)alloc(strlen(fontname) + 1);
        Strcpy(*fn, fontname);
    }
    return;
}

STATIC_OVL int
wc_set_window_colors(op)
char *op;
{
    /* syntax:
     *  menu white/black message green/yellow status white/blue text white/black
     */

    int j;
    char buf[BUFSZ];
    char *wn, *tfg, *tbg, *newop;
    static const char *wnames[] = { "menu", "message", "status", "text" };
    static const char *shortnames[] = { "mnu", "msg", "sts", "txt" };
    static char **fgp[] = {
        &iflags.wc_foregrnd_menu,
        &iflags.wc_foregrnd_message,
        &iflags.wc_foregrnd_status,
        &iflags.wc_foregrnd_text
    };
    static char **bgp[] = {
        &iflags.wc_backgrnd_menu,
        &iflags.wc_backgrnd_message,
        &iflags.wc_backgrnd_status,
        &iflags.wc_backgrnd_text
    };

    Strcpy(buf, op);
    newop = mungspaces(buf);
    while (newop && *newop) {

        wn = tfg = tbg = (char *)0;

        /* until first non-space in case there's leading spaces - before colorname*/
        while(*newop && isspace(*newop)) newop++;
        if (*newop) wn = newop;
        else return 0;

        /* until first space - colorname*/
        while(*newop && !isspace(*newop)) newop++;
        if (*newop) *newop = '\0';
        else return 0;
        newop++;

        /* until first non-space - before foreground*/
        while(*newop && isspace(*newop)) newop++;
        if (*newop) tfg = newop;
        else return 0;

        /* until slash - foreground */
        while(*newop && *newop != '/') newop++;
        if (*newop) *newop = '\0';
        else return 0;
        newop++;

        /* until first non-space (in case there's leading space after slash) - before background */
        while(*newop && isspace(*newop)) newop++;
        if (*newop) tbg = newop;
        else return 0;

        /* until first space - background */
        while(*newop && !isspace(*newop)) newop++;
        if (*newop) *newop++ = '\0';

        for (j = 0; j < 4; ++j) {
            if (!strcmpi(wn, wnames[j]) ||
                !strcmpi(wn, shortnames[j])) {
                if (tfg && !strstri(tfg, " ")) {
                    if (*fgp[j]) free(*fgp[j]);
                    *fgp[j] = (char *)alloc(strlen(tfg) + 1);
                    Strcpy(*fgp[j], tfg);
                }
                if (tbg && !strstri(tbg, " ")) {
                    if (*bgp[j]) free(*bgp[j]);
                    *bgp[j] = (char *)alloc(strlen(tbg) + 1);
                    Strcpy(*bgp[j], tbg);
                }
                break;
            }
        }
    }
    return 1;
}

#endif  /* OPTION_LISTS_ONLY */

/*options.c*/
