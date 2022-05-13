/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"
#include "func_tab.h"
/* #define DEBUG */ /* uncomment for debugging */

/* Macros for meta and ctrl modifiers:
 *   M and C return the meta/ctrl code for the given character;
 *     e.g., (C('c') is ctrl-c
 */
#ifndef M
#ifndef NHSTDC
#define M(c) (0x80 | (c))
#else
#define M(c) ((c) - 128)
#endif /* NHSTDC */
#endif

#ifndef C
#define C(c) (0x1f & (c))
#endif

#define unctrl(c) ((c) <= C('z') ? (0x60 | (c)) : (c))
#define unmeta(c) (0x7f & (c))

#ifdef ALTMETA
static boolean alt_esc = FALSE;
#endif

/*
 * Some systems may have getchar() return EOF for various reasons, and
 * we should not quit before seeing at least NR_OF_EOFS consecutive EOFs.
 */
#if defined(SYSV) || defined(DGUX) || defined(HPUX)
#define NR_OF_EOFS  20
#endif

#define CMD_TRAVEL (char)0x90

#ifdef DEBUG
/*
 * only one "wiz_debug_cmd" routine should be available (in whatever
 * module you are trying to debug) or things are going to get rather
 * hard to link :-)
 */
extern int NDECL(wiz_debug_cmd);
#endif

struct cmd Cmd = { 0 }; /* flag.h */

#ifdef DUMB /* stuff commented out in extern.h, but needed here */
extern int NDECL(doapply); /**/
extern int NDECL(dorub); /**/
extern int NDECL(dojump); /**/
extern int NDECL(doextlist); /**/
extern int NDECL(dodrop); /**/
extern int NDECL(doddrop); /**/
extern int NDECL(dodown); /**/
extern int NDECL(doup); /**/
extern int NDECL(donull); /**/
extern int NDECL(dowipe); /**/
extern int NDECL(do_mname); /**/
extern int NDECL(ddocall); /**/
extern int NDECL(dotakeoff); /**/
extern int NDECL(doremring); /**/
extern int NDECL(dowear); /**/
extern int NDECL(doputon); /**/
extern int NDECL(doddoremarm); /**/
extern int NDECL(dokick); /**/
extern int NDECL(dofire); /**/
extern int NDECL(dothrow); /**/
extern int NDECL(doeat); /**/
extern int NDECL(done2); /**/
extern int NDECL(doengrave); /**/
#ifdef ELBERETH
extern int NDECL(doengrave_elbereth); /**/
#endif
extern int NDECL(dopickup); /**/
extern int NDECL(ddoinv); /**/
extern int NDECL(dotypeinv); /**/
extern int NDECL(dolook); /**/
extern int NDECL(doprgold); /**/
extern int NDECL(doprwep); /**/
extern int NDECL(doprarm); /**/
extern int NDECL(doprring); /**/
extern int NDECL(dopramulet); /**/
extern int NDECL(doprtool); /**/
extern int NDECL(dosuspend); /**/
extern int NDECL(doforce); /**/
extern int NDECL(doopen); /**/
extern int NDECL(doclose); /**/
extern int NDECL(dodiscovered); /**/
extern int NDECL(doset); /**/
extern int NDECL(dotogglepickup); /**/
extern int NDECL(dowhatis); /**/
extern int NDECL(doquickwhatis); /**/
extern int NDECL(dowhatdoes); /**/
extern int NDECL(dohelp); /**/
extern int NDECL(dohistory); /**/
extern int NDECL(doloot); /**/
extern int NDECL(dodrink); /**/
extern int NDECL(dodip); /**/
extern int NDECL(dosacrifice); /**/
extern int NDECL(dopray); /**/
extern int NDECL(doturn); /**/
extern int NDECL(doredraw); /**/
extern int NDECL(doread); /**/
extern int NDECL(dosave); /**/
extern int NDECL(dosearch); /**/
extern int NDECL(doidtrap); /**/
extern int NDECL(dopay); /**/
extern int NDECL(dosit); /**/
extern int NDECL(dotalk); /**/
extern int NDECL(docast); /**/
extern int NDECL(dovspell); /**/
extern int NDECL(dotelecmd); /**/
extern int NDECL(dotele); /**/
extern int NDECL(dountrap); /**/
extern int NDECL(doversion); /**/
extern int NDECL(doextversion); /**/
extern int NDECL(doswapweapon); /**/
extern int NDECL(dowield); /**/
extern int NDECL(dowieldquiver); /**/
extern int NDECL(dozap); /**/
extern int NDECL(doorganize); /**/
extern int NDECL(dovanquished); /**/

#ifdef DUMP_LOG
extern int NDECL(dump_screenshot); /**/
#endif
#ifdef LIVELOG_SHOUT
extern int NDECL(doshout); /**/
#endif

#endif /* DUMB */

static int NDECL((*timed_occ_fn));

STATIC_PTR int NDECL(doprev_message);
STATIC_PTR int NDECL(timed_occupation);
STATIC_PTR int NDECL(doextcmd);
STATIC_PTR int NDECL(domonability);
STATIC_PTR int NDECL(dooverview_or_wiz_where);
STATIC_PTR int NDECL(dotravel);
STATIC_PTR int NDECL(doautoexplore);
STATIC_PTR int NDECL(doautofight);
# ifdef WIZARD
int NDECL(wiz_show_rooms);
STATIC_PTR int NDECL(wiz_wish);
STATIC_PTR int NDECL(wiz_identify);
STATIC_PTR int NDECL(wiz_map);
STATIC_PTR int NDECL(wiz_genesis);
STATIC_PTR int NDECL(wiz_where);
STATIC_PTR int NDECL(wiz_detect);
STATIC_PTR int NDECL(wiz_panic);
STATIC_PTR int NDECL(wiz_polyself);
STATIC_PTR int NDECL(wiz_level_tele);
STATIC_PTR int NDECL(wiz_level_change);
STATIC_PTR int NDECL(wiz_show_seenv);
STATIC_PTR int NDECL(wiz_show_vision);
STATIC_PTR int NDECL(wiz_mon_polycontrol);
STATIC_PTR int NDECL(wiz_show_wmodes);
STATIC_PTR int NDECL(wiz_mazewalkmap);
extern char SpLev_Map[COLNO][ROWNO];
STATIC_PTR int NDECL(wiz_showkills);    /* showborn patch */
static void wiz_map_levltyp();
static void wiz_levltyp_legend();
#if defined(__BORLANDC__) && !defined(_WIN32)
extern void FDECL(show_borlandc_stats, (winid));
#endif
#ifdef DEBUG_MIGRATING_MONS
STATIC_PTR int NDECL(wiz_migrate_mons);
#endif
STATIC_DCL int FDECL(size_monst, (struct monst *, BOOLEAN_P));
STATIC_DCL int FDECL(size_obj, (struct obj *));
STATIC_DCL void FDECL(count_obj, (struct obj *, long *, long *, BOOLEAN_P, BOOLEAN_P));
STATIC_DCL void FDECL(obj_chain, (winid, const char *, struct obj *, BOOLEAN_P, long *, long *));
STATIC_DCL void FDECL(mon_invent_chain, (winid, const char *, struct monst *, long *, long *));
STATIC_DCL void FDECL(mon_chain, (winid, const char *, struct monst *, BOOLEAN_P, long *, long *));
STATIC_DCL void FDECL(contained_stats, (winid, const char *, long *, long *));
STATIC_DCL void FDECL(misc_stats, (winid, long *, long *));
STATIC_PTR int NDECL(wiz_show_stats);
static boolean FDECL(accept_menu_prefix, (int NDECL((*))));
#  ifdef PORT_DEBUG
STATIC_DCL int NDECL(wiz_port_debug);
#  endif
# else
extern int NDECL(tutorial_redisplay);
# endif
STATIC_PTR int NDECL(enter_explore_mode);
STATIC_PTR int NDECL(doattributes);
STATIC_PTR int NDECL(doconduct); /**/
STATIC_PTR boolean NDECL(minimal_enlightenment);

STATIC_DCL void FDECL(enlght_line, (const char *, const char *, const char *));
STATIC_DCL char *FDECL(enlght_combatinc, (const char *, int, int, char *));
#ifdef UNIX
static void NDECL(end_of_input);
#endif

static const char* readchar_queue="";
static coord clicklook_cc;
static char last_cmd_char='\0';
/* for rejecting attempts to use wizard mode commands */
static const char unavailcmd[] = "Unavailable command '%s'.";
/* for rejecting #if !SHELL, !SUSPEND */
static const char cmdnotavail[] = "'%s' command not available.";

static char FDECL(here_cmd_menu, (BOOLEAN_P));
static char FDECL(there_cmd_menu, (BOOLEAN_P, int, int));
STATIC_DCL char *NDECL(parse);
static boolean FDECL(help_dir, (CHAR_P, int, const char *));
static int NDECL(dosuspend_core);

/* allmain.c */
extern int monclock;

static int debug_show_colors();

STATIC_PTR int
doprev_message()
{
    return nh_doprev_message();
}

/* Count down by decrementing multi */
STATIC_PTR int
timed_occupation()
{
    (*timed_occ_fn)();
    if (multi > 0)
        multi--;
    return multi > 0;
}

/* If you have moved since initially setting some occupations, they
 * now shouldn't be able to restart.
 *
 * The basic rule is that if you are carrying it, you can continue
 * since it is with you.  If you are acting on something at a distance,
 * your orientation to it must have changed when you moved.
 *
 * The exception to this is taking off items, since they can be taken
 * off in a number of ways in the intervening time, screwing up ordering.
 *
 *  Currently:  Take off all armor.
 *              Picking Locks / Forcing Chests.
 *              Setting traps.
 */
void
reset_occupations()
{
    reset_remarm();
    reset_pick();
    reset_trapset();
}

/* If a time is given, use it to timeout this function, otherwise the
 * function times out by its own means.
 */
void
set_occupation(fn, txt, xtime)
int NDECL((*fn));
const char *txt;
int xtime;
{
    if (xtime) {
        occupation = timed_occupation;
        timed_occ_fn = fn;
    } else
        occupation = fn;
    occtxt = txt;
    occtime = 0;
    return;
}

#ifdef REDO

static char NDECL(popch);

/* Provide a means to redo the last command.  The flag `in_doagain' is set
 * to true while redoing the command.  This flag is tested in commands that
 * require additional input (like `throw' which requires a thing and a
 * direction), and the input prompt is not shown.  Also, while in_doagain is
 * TRUE, no keystrokes can be saved into the saveq.
 */
#define BSIZE 20
static char pushq[BSIZE], saveq[BSIZE];
static NEARDATA int phead, ptail, shead, stail;

static char
popch() {
    /* If occupied, return '\0', letting tgetch know a character should
     * be read from the keyboard.  If the character read is not the
     * ABORT character (as checked in pcmain.c), that character will be
     * pushed back on the pushq.
     */
    if (occupation) return '\0';
    if (in_doagain) return (char)((shead != stail) ? saveq[stail++] : '\0');
    else return (char)((phead != ptail) ? pushq[ptail++] : '\0');
}

char
pgetchar() {        /* curtesy of aeb@cwi.nl */
    register int ch;

    if (iflags.debug_fuzzer) {
        return randomkey();
    }
    if(!(ch = popch()))
        ch = nhgetch();
    return((char)ch);
}

/* A ch == 0 resets the pushq */
void
pushch(ch)
char ch;
{
    if (!ch)
        phead = ptail = 0;
    if (phead < BSIZE)
        pushq[phead++] = ch;
    return;
}

/* A ch == 0 resets the saveq.  Only save keystrokes when not
 * replaying a previous command.
 */
void
savech(ch)
char ch;
{
    if (!in_doagain) {
        if (!ch)
            phead = ptail = shead = stail = 0;
        else if (shead < BSIZE)
            saveq[shead++] = ch;
    }
    return;
}
#endif /* REDO */

/** here after # - now read a full-word command */
STATIC_PTR int
doextcmd()
{
    int idx, retval;
    int NDECL((*func));

    /* keep repeating until we don't run help or quit */
    do {
        idx = get_ext_cmd();
        if (idx < 0) {
            return 0; /* quit */
        }
        func = extcmdlist[idx].ef_funct;
        if (!wizard && (extcmdlist[idx].flags & WIZMODECMD)) {
            You("can't do that.");
            return 0;
        }
        if (iflags.menu_requested && !accept_menu_prefix(func)) {
            pline("'%s' prefix has no effect for the %s command.",
                  visctrl(Cmd.spkeys[NHKF_REQMENU]),
                  extcmdlist[idx].ef_txt);
            iflags.menu_requested = FALSE;
        }
        retval = (*func)();
    } while (func == doextlist);

    return retval;
}

/** here after #? - now list all full-word commands and provid
   some navigation capability through the long list */
int
doextlist()
{
    const struct ext_func_tab *efp;
    char buf[BUFSZ], searchbuf[BUFSZ], promptbuf[QBUFSZ];
    winid menuwin;
    anything any;
    menu_item *selected;
    int n, pass;
    int menumode = 1, menushown[2], onelist = 0;
    boolean redisplay = TRUE, search = FALSE;
    static const char *headings[] = {
        "Extended commands",
        "Debugging Extended Commands"
    };
#ifdef WIN_EDGE
    boolean saved_win_edge = iflags.win_edge;
    iflags.win_edge = TRUE;
#endif

    searchbuf[0] = '\0';
    menuwin = create_nhwindow(NHW_MENU);

    while (redisplay) {
        redisplay = FALSE;
        any = zeroany;
        start_menu(menuwin);
        add_menu(menuwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "Extended Commands List",
                 MENU_UNSELECTED);
        add_menu(menuwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);

        Strcpy(buf, menumode ? "Show" : "Hide");
        Strcat(buf, " commands that don't autocomplete");
        if (!menumode) {
            Strcat(buf, " (those not marked with [A])");
        }
        any.a_int = 1;
        add_menu(menuwin, NO_GLYPH, MENU_DEFCNT, &any, 'a', 0, ATR_NONE, buf, MENU_UNSELECTED);

        if (!*searchbuf) {
            any.a_int = 2;
            /* was 's', but then using ':' handling within the interface
               would only examine the two or three meta entries, not the
               actual list of extended commands shown via separator lines;
               having ':' as an explicit selector overrides the default
               menu behavior for it; we retain 's' as a group accelerator */
            add_menu(menuwin, NO_GLYPH, MENU_DEFCNT, &any, ':', 's', ATR_NONE, "Search extended commands",
                     MENU_UNSELECTED);
        } else {
            Strcpy(buf, "Show all, clear search");
            if (strlen(buf) + strlen(searchbuf) + strlen(" (\"\")") < QBUFSZ) {
                Sprintf(eos(buf), " (\"%s\")", searchbuf);
            }
            any.a_int = 3;
            /* specifying ':' as a group accelerator here is mostly a
               statement of intent (we'd like to accept it as a synonym but
               also want to hide it from general menu use) because it won't
               work for interfaces which support ':' to search; use as a
               general menu command takes precedence over group accelerator */
            add_menu(menuwin, NO_GLYPH, MENU_DEFCNT, &any, 's', ':', ATR_NONE, buf, MENU_UNSELECTED);
        }
        if (wizard) {
            any.a_int = 4;
            add_menu(menuwin, NO_GLYPH, MENU_DEFCNT, &any, 'z', 0, ATR_NONE,
                     onelist ? "Show debugging commands in separate section"
                     : "Show all alphabetically, including debugging commands",
                     MENU_UNSELECTED);
        }
        any = zeroany;
        add_menu(menuwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);
        menushown[0] = menushown[1] = 0;
        n = 0;
        for (pass = 0; pass <= 1; ++pass) {
            /* skip second pass if not in wizard mode or wizard mode
               commands are being integrated into a single list */
            if (pass == 1 && (onelist || !wizard)) {
                break;
            }
            for (efp = extcmdlist; efp->ef_txt; efp++) {
                int wizc;

                if ((efp->flags & CMD_NOT_AVAILABLE) != 0) {
                    continue;
                }
                /* if hiding non-autocomplete commands, skip such */
                if (menumode == 1 && (efp->flags & AUTOCOMPLETE) == 0) {
                    continue;
                }
                /* if searching, skip this command if it doesn't match */
                if (*searchbuf &&
                    /* first try case-insensitive substring match */
                     !strstri(efp->ef_txt, searchbuf) &&
                     !strstri(efp->ef_desc, searchbuf) &&
                    /* wildcard support; most interfaces use case-insensitve
                       pmatch rather than regexp for menu searching */
                     !pmatchi(searchbuf, efp->ef_txt) &&
                     !pmatchi(searchbuf, efp->ef_desc)) {
                    continue;
                }
                /* skip wizard mode commands if not in wizard mode;
                   when showing two sections, skip wizard mode commands
                   in pass==0 and skip other commands in pass==1 */
                wizc = (efp->flags & WIZMODECMD) != 0;
                if (wizc && !wizard) {
                    continue;
                }
                if (!onelist && pass != wizc) {
                    continue;
                }

                /* We're about to show an item, have we shown the menu yet?
                   Doing menu in inner loop like this on demand avoids a
                   heading with no subordinate entries on the search
                   results menu. */
                if (!menushown[pass]) {
                    Strcpy(buf, headings[pass]);
                    add_menu(menuwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings, buf, MENU_UNSELECTED);
                    menushown[pass] = 1;
                }
                Sprintf(buf, " %-15s %-3s %s",
                        efp->ef_txt,
                        (efp->flags & AUTOCOMPLETE) ? "[A]" : " ",
                        efp->ef_desc);
                add_menu(menuwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
                ++n;
            }
            if (n) {
                add_menu(menuwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);
            }
        }
        if (*searchbuf && !n) {
            add_menu(menuwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "no matches", MENU_UNSELECTED);
        }

        end_menu(menuwin, (char *) 0);
        n = select_menu(menuwin, PICK_ONE, &selected);
        if (n > 0) {
            switch (selected[0].item.a_int) {
            case 1: /* 'a': toggle show/hide non-autocomplete */
                menumode = 1 - menumode;  /* toggle 0 -> 1, 1 -> 0 */
                redisplay = TRUE;
                break;

            case 2: /* ':' when not searching yet: enable search */
                search = TRUE;
                break;

            case 3: /* 's' when already searching: disable search */
                search = FALSE;
                searchbuf[0] = '\0';
                redisplay = TRUE;
                break;

            case 4: /* 'z': toggle showing wizard mode commands separately */
                search = FALSE;
                searchbuf[0] = '\0';
                onelist = 1 - onelist;  /* toggle 0 -> 1, 1 -> 0 */
                redisplay = TRUE;
            }
            free(selected);
        } else {
            search = FALSE;
            searchbuf[0] = '\0';
        }
        if (search) {
            Strcpy(promptbuf, "Extended command list search phrase");
            Strcat(promptbuf, "?");
            getlin(promptbuf, searchbuf);
            (void) mungspaces(searchbuf);
            if (searchbuf[0] == '\033') {
                searchbuf[0] = '\0';
            }
            if (*searchbuf) {
                redisplay = TRUE;
            }
            search = FALSE;
        }
    }
    destroy_nhwindow(menuwin);
#ifdef WIN_EDGE
    iflags.win_edge = saved_win_edge;
#endif
    return 0;
}

#if defined(TTY_GRAPHICS) || defined(CURSES_GRAPHICS)
#define MAX_EXT_CMD 200 /* Change if we ever have more ext cmds */
/*
 * This is currently used only by the tty interface and is
 * controlled via runtime option 'extmenu'.  (Most other interfaces
 * already use a menu all the time for extended commands.)
 *
 * ``# ?'' is counted towards the limit of the number of commands,
 * so we actually support MAX_EXT_CMD-1 "real" extended commands.
 *
 * Here after # - now show pick-list of possible commands.
 */
int
extcmd_via_menu()   /* here after # - now show pick-list of possible commands */
{
    const struct ext_func_tab *efp;
    menu_item *pick_list = (menu_item *)0;
    winid win;
    anything any = { 0 };
    const struct ext_func_tab *choices[MAX_EXT_CMD + 1];
    char buf[BUFSZ];
    char cbuf[QBUFSZ], prompt[QBUFSZ], fmtstr[20];
    int i, n, nchoices, acount;
    int ret, len, biggest;
    int accelerator, prevaccelerator;
    int matchlevel = 0;
    boolean wastoolong, one_per_line;

    ret = 0;
    cbuf[0] = '\0';
    biggest = 0;
    while (!ret) {
        i = n = 0;
        any.a_void = 0;
        /* populate choices */
        for (efp = extcmdlist; efp->ef_txt; efp++) {
            if ((efp->flags & CMD_NOT_AVAILABLE) ||
                !(efp->flags & AUTOCOMPLETE) ||
                (!wizard && (efp->flags & WIZMODECMD))) {
                continue;
            }
            if (!matchlevel || !strncmp(efp->ef_txt, cbuf, matchlevel)) {
                choices[i] = efp;
                if ((len = (int) strlen(efp->ef_desc)) > biggest) {
                    biggest = len;
                }
                if (++i > MAX_EXT_CMD) {
                    warning("Exceeded %d extended commands in doextcmd() menu; 'extmenu' disabled.",
                            MAX_EXT_CMD);
                    iflags.extmenu = 0;
                    return -1;
                }
            }
        }
        choices[i] = (struct ext_func_tab *)0;
        nchoices = i;
        /* if we're down to one, we have our selection so get out of here */
        if (nchoices <= 1) {
            ret = (nchoices == 1) ? (int) (choices[0] - extcmdlist) : -1;
            break;
        }

        /* otherwise... */
        win = create_nhwindow(NHW_MENU);
        start_menu(win);
        Sprintf(fmtstr, "%%-%ds", biggest + 15);
        prompt[0] = '\0';
        wastoolong = FALSE; /* True => had to wrap due to line width
                             * ('w' in wizard mode) */
        /* -3: two line menu header, 1 line menu footer (for prompt) */
        one_per_line = (nchoices < ROWNO - 3);
        accelerator = prevaccelerator = 0;
        acount = 0;
        for (i = 0; choices[i]; ++i) {
            accelerator = choices[i]->ef_txt[matchlevel];
            if (accelerator != prevaccelerator || one_per_line) {
                wastoolong = FALSE;
            }
            if (accelerator != prevaccelerator ||
                one_per_line ||
                (acount >= 2 &&
                    /* +4: + sizeof " or " - sizeof "" */
                     (strlen(prompt) + 4 + strlen(choices[i]->ef_txt)
                      /* -6: enough room for 1 space left margin
                       *   + "%c - " menu selector + 1 space right margin */
                      >= min(sizeof prompt, COLNO - 6)))) {
                if (acount) {
                    /* flush the extended commands for that letter already in buf */
                    Sprintf(buf, fmtstr, prompt);
                    any.a_char = prevaccelerator;
                    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, any.a_char, 0,
                             ATR_NONE, buf, FALSE);
                    acount = 0;
                    if (!(accelerator != prevaccelerator || one_per_line)) {
                        wastoolong = TRUE;
                    }
                }
            }
            prevaccelerator = accelerator;
            if (!acount || one_per_line) {
                Sprintf(prompt, "%s%s [%s]",
                        wastoolong ? "or " : "",
                        choices[i]->ef_txt,
                        choices[i]->ef_desc);
            } else if (acount == 1) {
                Sprintf(prompt, "%s%s or %s",
                        wastoolong ? "or " : "",
                        choices[i - 1]->ef_txt,
                        choices[i]->ef_txt);
            } else {
                Strcat(prompt, " or ");
                Strcat(prompt, choices[i]->ef_txt);
            }
            ++acount;
        }
        if (acount) {
            /* flush buf */
            Sprintf(buf, fmtstr, prompt);
            any.a_char = prevaccelerator;
            add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, any.a_char, 0, ATR_NONE, buf, FALSE);
        }
        Sprintf(prompt, "Extended Command: %s", cbuf);
        end_menu(win, prompt);
        n = select_menu(win, PICK_ONE, &pick_list);
        destroy_nhwindow(win);
        if (n == 1) {
            if (matchlevel > (QBUFSZ - 2)) {
                free((genericptr_t)pick_list);
#ifdef DEBUG
                warning("Too many characters (%d) entered in extcmd_via_menu()",
                        matchlevel);
#endif
                ret = -1;
            } else {
                cbuf[matchlevel++] = pick_list[0].item.a_char;
                cbuf[matchlevel] = '\0';
                free((genericptr_t)pick_list);
            }
        } else {
            if (matchlevel) {
                ret = 0;
                matchlevel = 0;
            } else {
                ret = -1;
            }
        }
    }
    return ret;
}
#endif

/* #monster command - use special monster ability while polymorphed */
STATIC_PTR int
domonability()
{
    if (can_breathe(youmonst.data)) return dobreathe();
    else if (attacktype(youmonst.data, AT_SPIT)) return dospit();
    else if (youmonst.data->mlet == S_NYMPH) return doremove();
    else if (attacktype(youmonst.data, AT_GAZE)) return dogaze();
    else if (is_were(youmonst.data)) return dosummon();
    else if (webmaker(youmonst.data)) return dospinweb();
    else if (is_hider(youmonst.data)) return dohide();
    else if (is_mind_flayer(youmonst.data)) return domindblast();
    else if (u.umonnum == PM_GREMLIN) {
        if(IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
            if (split_mon(&youmonst, (struct monst *)0))
                dryup(u.ux, u.uy, TRUE);
        } else There("is no fountain here.");
    } else if (is_unicorn(youmonst.data)) {
        fix_attributes_and_properties((struct obj *)0, 0);
        return 1;
    } else if (youmonst.data->msound == MS_SHRIEK) {
        You("shriek.");
        if(u.uburied)
            pline("Unfortunately sound does not carry well through rock.");
        else aggravate();
    } else if (Upolyd)
        pline("Any special ability you may have is purely reflexive.");
    else You("don't have a special ability in your normal form!");
    return 0;
}

STATIC_PTR int
enter_explore_mode()
{
    if(!discover && !wizard) {
        pline("Beware!  From explore mode there will be no return to normal game.");
        if (paranoid_yn("Do you want to enter explore mode?", iflags.paranoid_quit) == 'y') {
            clear_nhwindow(WIN_MESSAGE);
            You("are now in non-scoring explore mode.");
            discover = TRUE;
        }
        else {
            clear_nhwindow(WIN_MESSAGE);
            pline("Resuming normal game.");
        }
    }
    return 0;
}

/* #vanquished command */
STATIC_PTR int dovanquished()
{
    list_vanquishedonly();
    return 0;
}

STATIC_PTR int
dooverview_or_wiz_where()
{
/*
 #ifdef WIZARD
    if (wizard) return wiz_where();
    else
 #endif
 */
    dooverview();
    return 0;
}

#ifdef WIZARD

int
wiz_show_rooms()
{
    winid win;
    int x, y;
    char row[COLNO+1];

    win = create_nhwindow(NHW_TEXT);
    for (y = 0; y < ROWNO; y++) {
        for (x = 0; x < COLNO; x++) {
            int rno = levl[x][y].roomno;
            if (rno == NO_ROOM)
                row[x] = '.';
            else if (rno == SHARED)
                row[x] = '+';
            else if (rno == SHARED_PLUS)
                row[x] = '*';
            else {
                int i = (rno - ROOMOFFSET) % 52;
                row[x] = (i < 26) ? ('a'+i) : ('A'+i-26);
            }
        }
        row[COLNO] = '\0';
        putstr(win, 0, row);
    }
    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
    return 0;
}

/* ^W command - wish for something */
STATIC_PTR int
wiz_wish()  /* Unlimited wishes for debug mode by Paul Polderman */
{
    if (wizard) {
        boolean save_verbose = flags.verbose;

        flags.verbose = FALSE;
        makewish(TRUE);
        flags.verbose = save_verbose;
        (void) encumber_msg();
    } else
        pline("Unavailable command '^W'.");
    return 0;
}

/* ^I command - identify hero's inventory */
STATIC_PTR int
wiz_identify()
{
    if (wizard) {
        identify_pack(0, FALSE);
    } else {
        pline("Unavailable command '^I'.");
    }
    return 0;
}

/* ^F command - reveal the level map and any traps on it */
STATIC_PTR int
wiz_map()
{
    if (wizard) {
        struct trap *t;
        long save_Hconf = HConfusion,
             save_Hhallu = HHallucination;

        HConfusion = HHallucination = 0L;
        for (t = ftrap; t != 0; t = t->ntrap) {
            t->tseen = 1;
            map_trap(t, TRUE);
        }
        do_mapping();
        HConfusion = save_Hconf;
        HHallucination = save_Hhallu;
    } else
        pline("Unavailable command '^F'.");
    return 0;
}

/* ^G command - generate monster(s); a count prefix will be honored */
STATIC_PTR int
wiz_genesis()
{
    if (wizard) (void) create_particular();
    else pline("Unavailable command '^G'.");
    return 0;
}

/* ^O command - display dungeon layout */
STATIC_PTR int
wiz_where()
{
    if (wizard) {
        (void) print_dungeon(FALSE, (schar *)0, (xchar *)0);
    } else {
        pline(unavailcmd, visctrl((int) cmd_from_func(wiz_where)));
    }
    return 0;
}

/* ^E command - detect unseen (secret doors, traps, hidden monsters) */
STATIC_PTR int
wiz_detect()
{
    if (wizard) {
        (void) findit();
    } else {
        pline(unavailcmd, visctrl((int) cmd_from_func(wiz_detect)));
    }
    return 0;
}

/* ^V command - level teleport, or tutorial review */
STATIC_PTR int
wiz_level_tele()
{
    if (wizard) level_tele();
    else if(flags.tutorial)
        tutorial_redisplay();
    return 0;
}

/* #monpolycontrol command - choose new form for shapechangers, polymorphees */
STATIC_PTR int
wiz_mon_polycontrol()
{
    iflags.mon_polycontrol = !iflags.mon_polycontrol;
    pline("Monster polymorph control is %s.",
          iflags.mon_polycontrol ? "on" : "off");
    return 0;
}

/* #levelchange command - adjust hero's experience level */
STATIC_PTR int
wiz_level_change()
{
    char buf[BUFSZ];
    int newlevel;
    int ret;

    getlin("To what experience level do you want to be set?", buf);
    (void)mungspaces(buf);
    if (buf[0] == '\033' || buf[0] == '\0') ret = 0;
    else ret = sscanf(buf, "%d", &newlevel);

    if (ret != 1) {
        pline("%s", Never_mind);
        return 0;
    }
    if (newlevel == u.ulevel) {
        You("are already that experienced.");
    } else if (newlevel < u.ulevel) {
        if (u.ulevel == 1) {
            You("are already as inexperienced as you can get.");
            return 0;
        }
        if (newlevel < 1) newlevel = 1;
        while (u.ulevel > newlevel)
            losexp("#levelchange");
    } else {
        if (u.ulevel >= MAXULEV) {
            You("are already as experienced as you can get.");
            return 0;
        }
        if (newlevel > MAXULEV) newlevel = MAXULEV;
        while (u.ulevel < newlevel)
            pluslvl(FALSE);
    }
    u.ulevelmax = u.ulevel;
    return 0;
}

/* #panic command - test program's panic handling */
STATIC_PTR int
wiz_panic()
{
    if (iflags.debug_fuzzer) {
        u.uhp = u.uhpmax = 1000;
        u.uen = u.uenmax = 1000;
        return 0;
    }

    if (yn("Do you want to call panic() and end your game?") == 'y')
        panic("crash test.");
    return 0;
}

/* #polyself command - change hero's form */
STATIC_PTR int
wiz_polyself()
{
    polyself(TRUE);
    return 0;
}

/* #seenv command */
STATIC_PTR int
wiz_show_seenv()
{
    winid win;
    int x, y, v, startx, stopx, curx;
    char row[COLNO+1];

    win = create_nhwindow(NHW_TEXT);
    /*
     * Each seenv description takes up 2 characters, so center
     * the seenv display around the hero.
     */
    startx = max(1, u.ux-(COLNO/4));
    stopx = min(startx+(COLNO/2), COLNO);
    /* can't have a line exactly 80 chars long */
    if (stopx - startx == COLNO/2) startx++;

    for (y = 0; y < ROWNO; y++) {
        for (x = startx, curx = 0; x < stopx; x++, curx += 2) {
            if (x == u.ux && y == u.uy) {
                row[curx] = row[curx+1] = '@';
            } else {
                v = levl[x][y].seenv & 0xff;
                if (v == 0)
                    row[curx] = row[curx+1] = ' ';
                else
                    Sprintf(&row[curx], "%02x", v);
            }
        }
        /* remove trailing spaces */
        for (x = curx-1; x >= 0; x--)
            if (row[x] != ' ') break;
        row[x+1] = '\0';

        putstr(win, 0, row);
    }
    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
    return 0;
}

/* #vision command */
STATIC_PTR int
wiz_show_vision()
{
    winid win;
    int x, y, v;
    char row[COLNO+1];

    win = create_nhwindow(NHW_TEXT);
    Sprintf(row, "Flags: 0x%x could see, 0x%x in sight, 0x%x temp lit",
            COULD_SEE, IN_SIGHT, TEMP_LIT);
    putstr(win, 0, row);
    putstr(win, 0, "");
    for (y = 0; y < ROWNO; y++) {
        for (x = 1; x < COLNO; x++) {
            if (x == u.ux && y == u.uy)
                row[x] = '@';
            else {
                v = viz_array[y][x]; /* data access should be hidden */
                if (v == 0)
                    row[x] = ' ';
                else
                    row[x] = '0' + viz_array[y][x];
            }
        }
        /* remove trailing spaces */
        for (x = COLNO-1; x >= 1; x--)
            if (row[x] != ' ') break;
        row[x+1] = '\0';

        putstr(win, 0, &row[1]);
    }
    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
    return 0;
}

/* #wmode command */
STATIC_PTR int
wiz_show_wmodes()
{
    winid win;
    int x, y;
    char row[COLNO+1];
    struct rm *lev;

    win = create_nhwindow(NHW_TEXT);
    for (y = 0; y < ROWNO; y++) {
        for (x = 0; x < COLNO; x++) {
            lev = &levl[x][y];
            if (x == u.ux && y == u.uy)
                row[x] = '@';
            else if (IS_WALL(lev->typ) || lev->typ == SDOOR)
                row[x] = '0' + (lev->wall_info & WM_MASK);
            else if (lev->typ == CORR)
                row[x] = '#';
            else if (IS_ROOM(lev->typ) || IS_DOOR(lev->typ))
                row[x] = '.';
            else
                row[x] = 'x';
        }
        row[COLNO] = '\0';
        putstr(win, 0, row);
    }
    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
    return 0;
}

/* #showkills command */
STATIC_PTR int wiz_showkills()      /* showborn patch */
{
    list_vanquished('y', FALSE);
    return 0;
}

#endif /* WIZARD */

/* wizard mode variant of #terrain; internal levl[][].typ values in base-36 */
static void
wiz_map_levltyp(void)
{
    winid win;
    int x, y, terrain;
    char row[COLNO + 1];
    boolean istty = !strcmp(windowprocs.name, "tty");

    win = create_nhwindow(NHW_TEXT);
    /* map row 0, levl[][0], is drawn on the second line of tty screen */
    if (istty) {
        putstr(win, 0, ""); /* tty only: blank top line */
    }
    for (y = 0; y < ROWNO; y++) {
        /* map column 0, levl[0][], is off the left edge of the screen;
           it should always have terrain type "undiggable stone" */
        for (x = 1; x < COLNO; x++) {
            terrain = levl[x][y].typ;
            /* assumes there aren't more than 10+26+26 terrain types */
            row[x - 1] = (char) ((terrain == STONE && !may_dig(x, y)) ? '*' :
                                (terrain < 10) ? '0' + terrain :
                                (terrain < 36) ? 'a' + terrain - 10 :
                                'A' + terrain - 36);
        }
        x--;
        if (levl[0][y].typ != STONE || may_dig(0, y)) {
            row[x++] = '!';
        }
        row[x] = '\0';
        putstr(win, 0, row);
    }

    char dsc[BUFSZ];
    s_level *slev = Is_special(&u.uz);

    Sprintf(dsc, "D:%d,L:%d", u.uz.dnum, u.uz.dlevel);
    /* [dungeon branch features currently omitted] */
    /* special level features */
    if (slev) {
        Sprintf(eos(dsc), " \"%s\"", slev->proto);
        /* special level flags (note: dungeon.def doesn't set `maze'
            or `hell' for any specific levels so those never show up) */
        if (slev->flags.maze_like) {
            Strcat(dsc, " mazelike");
        }
        if (slev->flags.hellish) {
            Strcat(dsc, " hellish");
        }
        if (slev->flags.town) {
            Strcat(dsc, " town");
        }
        if (slev->flags.rogue_like) {
            Strcat(dsc, " roguelike");
        }
        /* alignment currently omitted to save space */
    }
    /* level features */
    if (level.flags.nfountains) {
        Sprintf(eos(dsc), " %c:%d", defsyms[S_fountain].sym, (int) level.flags.nfountains);
    }
    if (level.flags.nsinks) {
        Sprintf(eos(dsc), " %c:%d", defsyms[S_sink].sym, (int) level.flags.nsinks);
    }
    if (level.flags.has_vault) {
        Strcat(dsc, " vault");
    }
    if (level.flags.has_shop) {
        Strcat(dsc, " shop");
    }
    if (level.flags.has_temple) {
        Strcat(dsc, " temple");
    }
    if (level.flags.has_court) {
        Strcat(dsc, " throne");
    }
    if (level.flags.has_zoo) {
        Strcat(dsc, " zoo");
    }
    if (level.flags.has_morgue) {
        Strcat(dsc, " morgue");
    }
    if (level.flags.has_barracks) {
        Strcat(dsc, " barracks");
    }
    if (level.flags.has_beehive) {
        Strcat(dsc, " hive");
    }
    if (level.flags.has_swamp) {
        Strcat(dsc, " swamp");
    }
    /* level flags */
    if (level.flags.noteleport) {
        Strcat(dsc, " noTport");
    }
    if (level.flags.hardfloor) {
        Strcat(dsc, " noDig");
    }
    if (level.flags.nommap) {
        Strcat(dsc, " noMMap");
    }
    if (!level.flags.hero_memory) {
        Strcat(dsc, " noMem");
    }
    if (level.flags.shortsighted) {
        Strcat(dsc, " shortsight");
    }
    if (level.flags.graveyard) {
        Strcat(dsc, " graveyard");
    }
    if (level.flags.is_maze_lev) {
        Strcat(dsc, " maze");
    }
    if (level.flags.is_cavernous_lev) {
        Strcat(dsc, " cave");
    }
    if (level.flags.arboreal) {
        Strcat(dsc, " tree");
    }
    if (Sokoban) {
        Strcat(dsc, " sokoban-rules");
    }
    /* non-flag info; probably should include dungeon branching
        checks (extra stairs and magic portals) here */
    if (Invocation_lev(&u.uz)) {
        Strcat(dsc, " invoke");
    }
    if (On_W_tower_level(&u.uz)) {
        Strcat(dsc, " tower");
    }
    /* append a branch identifier for completeness' sake */
    if (u.uz.dnum == 0) {
        Strcat(dsc, " dungeon");
    } else if (u.uz.dnum == mines_dnum) {
        Strcat(dsc, " mines");
    } else if (In_sokoban(&u.uz)) {
        Strcat(dsc, " sokoban");
    } else if (u.uz.dnum == quest_dnum) {
        Strcat(dsc, " quest");
    } else if (Is_knox(&u.uz)) {
        Strcat(dsc, " ludios");
    } else if (u.uz.dnum == 1) {
        Strcat(dsc, " gehennom");
    } else if (u.uz.dnum == tower_dnum) {
        Strcat(dsc, " vlad");
    } else if (In_endgame(&u.uz)) {
        Strcat(dsc, " endgame");
    } else {
        /* somebody's added a dungeon branch we're not expecting */
        const char *brname = dungeons[u.uz.dnum].dname;

        if (!brname || !*brname) {
            brname = "unknown";
        }
        if (!strncmpi(brname, "the ", 4)) {
            brname += 4;
        }
        Sprintf(eos(dsc), " %s", brname);
    }
    /* limit the line length to map width */
    if (strlen(dsc) >= COLNO) {
        dsc[COLNO - 1] = '\0'; /* truncate */
    }
    putstr(win, 0, dsc);

    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
    return;
}

/* temporary? hack, since level type codes aren't the same as screen
   symbols and only the latter have easily accessible descriptions */
static const char *levltyp[] = {
    "stone", "vertical wall", "horizontal wall", "top-left corner wall",
    "top-right corner wall", "bottom-left corner wall",
    "bottom-right corner wall", "cross wall", "tee-up wall", "tee-down wall",
    "tee-left wall", "tee-right wall", "drawbridge wall", "tree", "dead tree",
    "ice wall", "crystal ice wall",
    "secret door", "secret corridor", "pool", "moat", "water",
    "drawbridge up", "lava pool", "iron bars", "door", "corridor", "room",
    "stairs", "ladder", "fountain", "throne", "sink", "grave", "altar", "ice",
    "swamp",
    "drawbridge down", "air", "cloud",
    /* not a real terrain type, but used for undiggable stone
       by wiz_map_levltyp() */
    "unreachable/undiggable",
    /* padding in case the number of entries above is odd */
    ""
};

/* explanation of base-36 output from wiz_map_levltyp() */
static void
wiz_levltyp_legend(void)
{
    winid win;
    int i, j, last, c;
    const char *dsc, *fmt;
    char buf[BUFSZ];

    win = create_nhwindow(NHW_TEXT);
    putstr(win, 0, "#terrain encodings:");
    putstr(win, 0, "");
    fmt = " %c - %-28s"; /* TODO: include tab-separated variant for win32 */
    *buf = '\0';
    /* output in pairs, left hand column holds [0],[1],...,[N/2-1]
       and right hand column holds [N/2],[N/2+1],...,[N-1];
       N ('last') will always be even, and may or may not include
       the empty string entry to pad out the final pair, depending
       upon how many other entries are present in levltyp[] */
    last = SIZE(levltyp) & ~1;
    for (i = 0; i < last / 2; ++i) {
        for (j = i; j < last; j += last / 2) {
            dsc = levltyp[j];
            c = !*dsc ? ' ' :
                !strncmp(dsc, "unreachable", 11) ? '*' :
                /* same int-to-char conversion as wiz_map_levltyp() */
                (j < 10) ? '0' + j :
                (j < 36) ? 'a' + j - 10 :
                'A' + j - 36;
            Sprintf(eos(buf), fmt, c, dsc);
            if (j > i) {
                putstr(win, 0, buf);
                *buf = '\0';
            }
        }
    }
    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
    return;
}

/* #terrain command -- show known map, inspired by crawl's '|' command */
static int
doterrain(void)
{
    winid men;
    menu_item *sel;
    anything any;
    int n;
    int which;

    /*
     * normal play: choose between known map without mons, obj, and traps
     *  (to see underlying terrain only), or
     *  known map without mons and objs (to see traps under mons and objs), or
     *  known map without mons (to see objects under monsters);
     * explore mode: normal choices plus full map (w/o mons, objs, traps);
     * wizard mode: normal and explore choices plus
     *  a dump of the internal levl[][].typ codes w/ level flags, or
     *  a legend for the levl[][].typ codes dump
     */
    men = create_nhwindow(NHW_MENU);
    start_menu(men);
    any = zeroany;
    any.a_int = 1;
    add_menu(men, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
             "known map without monsters, objects, and traps", MENU_SELECTED);
    any.a_int = 2;
    add_menu(men, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
             "known map without monsters and objects", MENU_UNSELECTED);
    any.a_int = 3;
    add_menu(men, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
             "known map without monsters", MENU_UNSELECTED);
    if (discover || wizard) {
        any.a_int = 4;
        add_menu(men, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                 "full map without monsters, objects, and traps", MENU_UNSELECTED);
        if (wizard) {
            any.a_int = 5;
            add_menu(men, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                     "internal levl[][].typ codes in base-36", MENU_UNSELECTED);
            any.a_int = 6;
            add_menu(men, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                     "legend of base-36 levl[][].typ codes", MENU_UNSELECTED);
        }
    }
    end_menu(men, "View which?");

    n = select_menu(men, PICK_ONE, &sel);
    destroy_nhwindow(men);
    /*
     * n <  0: player used ESC to cancel;
     * n == 0: preselected entry was explicitly chosen and got toggled off;
     * n == 1: preselected entry was implicitly chosen via <space>|<enter>;
     * n == 2: another entry was explicitly chosen, so skip preselected one.
     */
    which = (n < 0) ? -1 : (n == 0) ? 1 : sel[0].item.a_int;
    if (n > 1 && which == 1) {
        which = sel[1].item.a_int;
    }
    if (n > 0) {
        free(sel);
    }

    switch (which) {
    case 1: /* known map */
        reveal_terrain(0, TER_MAP);
        break;

    case 2: /* known map with known traps */
        reveal_terrain(0, TER_MAP | TER_TRP);
        break;

    case 3: /* known map with known traps and objects */
        reveal_terrain(0, TER_MAP | TER_TRP | TER_OBJ);
        break;

    case 4: /* full map */
        reveal_terrain(1, TER_MAP);
        break;

    case 5: /* map internals */
        wiz_map_levltyp();
        break;

    case 6: /* internal details */
        wiz_levltyp_legend();
        break;

    default:
        break;
    }
    return 0; /* no time elapses */
}

/* -enlightenment and conduct- */
static winid en_win;
static const char
    You_[] = "You ",
    are[]  = "are ",  were[]  = "were ",
    have[] = "have ", had[]   = "had ",
    can[]  = "can ",  could[] = "could ";
static const char
    have_been[]  = "have been ",
    have_never[] = "have never ", never[] = "never ";

#define enl_msg(prefix, present, past, suffix) \
    enlght_line(prefix, final ? past : present, suffix)
#define you_are(attr)   enl_msg(You_, are, were, attr)
#define you_have(attr)  enl_msg(You_, have, had, attr)
#define you_can(attr)   enl_msg(You_, can, could, attr)
#define you_have_been(goodthing) enl_msg(You_, have_been, were, goodthing)
#define you_have_never(badthing) enl_msg(You_, have_never, never, badthing)
#define you_have_X(something)   enl_msg(You_, have, (const char *)"", something)

static int want_display = FALSE;
static void
enlght_line(start, middle, end)
const char *start, *middle, *end;
{
    char buf[BUFSZ];

    Sprintf(buf, "%s%s%s.", start, middle, end);
    if (want_display) {
        putstr(en_win, 0, buf);
    }
    dump_list_item(buf);
}

/* format increased damage or chance to hit */
static char *
enlght_combatinc(inctyp, incamt, final, outbuf)
const char *inctyp;
int incamt, final;
char *outbuf;
{
    char numbuf[24];
    const char *modif, *bonus;

    if (final
#ifdef WIZARD
        || wizard
#endif
        ) {
        Sprintf(numbuf, "%s%d",
                (incamt > 0) ? "+" : "", incamt);
        modif = (const char *) numbuf;
    } else {
        int absamt = abs(incamt);

        if (absamt <= 3) modif = "small";
        else if (absamt <= 6) modif = "moderate";
        else if (absamt <= 12) modif = "large";
        else modif = "huge";
    }
    bonus = (incamt > 0) ? "bonus" : "penalty";
    /* "bonus to hit" vs "damage bonus" */
    if (!strcmp(inctyp, "damage")) {
        const char *ctmp = inctyp;
        inctyp = bonus;
        bonus = ctmp;
    }
    Sprintf(outbuf, "%s %s %s", an(modif), bonus, inctyp);
    return outbuf;
}

void
enlightenment(final, want_disp)
int final;  /* 0 => still in progress; 1 => over, survived; 2 => dead */
boolean want_disp;
{
    int ltmp;
    char buf[BUFSZ];

    want_display = want_disp;

    Sprintf(buf, final ? "Final Attributes:" : "Current Attributes:");
    if (want_display) {
        en_win = create_nhwindow(NHW_MENU);
        putstr(en_win, 0, buf);
        putstr(en_win, 0, "");
    }
    dump_title(buf);
    dump_list_start();

#ifdef ELBERETH
    if (u.uevent.uhand_of_elbereth) {
        static const char * const hofe_titles[3] = {
            "the Hand of Elbereth",
            "the Envoy of Balance",
            "the Glory of Arioch"
        };
        you_are(hofe_titles[u.uevent.uhand_of_elbereth - 1]);
    }
#endif

    /* heaven or hell modes */
    if (heaven_or_hell_mode) {
        if (u.ulives > 1) {
            Sprintf(buf, "%ld lives left", u.ulives);
        } else if (u.ulives == 0) {
            Sprintf(buf, "no lives left");
        } else {
            Sprintf(buf, "%ld life left", u.ulives);
        }
        you_have(buf);
    }

    /* note: piousness 3 matches MIN_QUEST_ALIGN (quest.h) */
    if (u.ualign.record >= 20) you_are("piously aligned");
    else if (u.ualign.record > 13) you_are("devoutly aligned");
    else if (u.ualign.record > 8) you_are("fervently aligned");
    else if (u.ualign.record > 3) you_are("stridently aligned");
    else if (u.ualign.record == 3) you_are("aligned");
    else if (u.ualign.record > 0) you_are("haltingly aligned");
    else if (u.ualign.record == 0) you_are("nominally aligned");
    else if (u.ualign.record >= -3) you_have("strayed");
    else if (u.ualign.record >= -8) you_have("sinned");
    else you_have("transgressed");
#ifdef WIZARD
    if (wizard || final) {
        Sprintf(buf, " %d", u.uhunger);
        enl_msg("Hunger level ", "is", "was", buf);
        Sprintf(buf, " %d / %ld", u.ualign.record, ALIGNLIM);
        enl_msg("Your alignment ", "is", "was", buf);
        Sprintf(buf, " %d - %d",
                min_monster_difficulty(), max_monster_difficulty());
        enl_msg("Monster difficulty range ", "is", "was", buf);
        Sprintf(buf, " %d", level_difficulty());
        enl_msg("Level difficulty ", "is", "was", buf);
    }
    if (wizard || final) {
        Sprintf(buf, " u%s", encode_base32(level_info[0].seed));
        enl_msg("Seed ", "is", "was", buf);
        if (is_game_pre_seeded) {
            enl_msg("You ", "are playing", "played", " a pre-seeded game");
        }
    }
    if ((wizard || final) && (monclock > 0)) {
        Sprintf(buf, "%2.2fx", (float)MIN_MONGEN_RATE/monclock);

        enl_msg("Monster generation rate ", "is ", "was ", buf);
    }
    if (wizard) {
        Snprintf(buf, sizeof(buf), "Current process id is %d", getpid());
        enl_msg("","","",buf);
    }
#endif

    /*** Resistances to troubles ***/
    if (Fire_resistance) you_are("fire resistant");
    if (Cold_resistance) you_are("cold resistant");
    if (Sleep_resistance) you_are("sleep resistant");
    if (Disint_resistance) you_are("disintegration-resistant");
    if (Shock_resistance) you_are("shock resistant");
    if (Poison_resistance) you_are("poison resistant");
    if (Drain_resistance) you_are("level-drain resistant");
    if (Sick_resistance) you_are("immune to sickness");
    if (Antimagic) you_are("magic-protected");
    if (Acid_resistance) you_are("acid resistant");
    if (Stone_resistance)
        you_are("petrification resistant");
    if (Invulnerable) you_are("invulnerable");
    if (u.uedibility) you_can("recognize detrimental food");

    /*** Troubles ***/
    if (!flags.perma_hallu && Halluc_resistance)
        enl_msg("You resist", "", "ed", " hallucinations");
    if (flags.perma_hallu) you_are("permanently hallucinating");
    if (final) {
        if (!flags.perma_hallu && Hallucination) you_are("hallucinating");
        if (Stunned) you_are("stunned");
        if (Confusion) you_are("confused");
        if (Blinded) you_are("blinded");
        if (Sick) {
            if (u.usick_type & SICK_VOMITABLE)
                you_are("sick from food poisoning");
            if (u.usick_type & SICK_NONVOMITABLE)
                you_are("sick from illness");
        }
#ifdef CONVICT
        if (Punished) {
            you_are("punished");
        }
#endif /* CONVICT */
    }
    if (Stoned) you_are("turning to stone");
    if (Slimed) you_are("turning into slime");
    if (Strangled) you_are((u.uburied) ? "buried" : "being strangled");
    if (Glib) {
        Sprintf(buf, "slippery %s", makeplural(body_part(FINGER)));
        you_have(buf);
    }
    if (Fumbling) enl_msg("You fumble", "", "d", "");
    if (Wounded_legs
#ifdef STEED
        && !u.usteed
#endif
        ) {
        Sprintf(buf, "wounded %s", makeplural(body_part(LEG)));
        you_have(buf);
    }
#if defined(WIZARD) && defined(STEED)
    if (Wounded_legs && u.usteed && (wizard || final)) {
        Strcpy(buf, x_monnam(u.usteed, ARTICLE_YOUR, (char *)0,
                             SUPPRESS_SADDLE | SUPPRESS_HALLUCINATION, FALSE));
        *buf = highc(*buf);
        enl_msg(buf, " has", " had", " wounded legs");
    }
#endif
    if (Sleeping) enl_msg("You ", "fall", "fell", " asleep");
    if (Hunger) enl_msg("You hunger", "", "ed", " rapidly");

    /*** Vision and senses ***/
    if (See_invisible) enl_msg(You_, "see", "saw", " invisible");
    if (Blind_telepat) you_are("telepathic");
    if (Warning) you_are("warned");
    if (Warn_of_mon && flags.warntype) {
        Sprintf(buf, "aware of the presence of %s",
                (flags.warntype & M2_ORC) ? "orcs" :
                (flags.warntype & M2_DEMON) ? "demons" :
                (flags.warntype & M2_GIANT) ? "giants" :
                (flags.warntype & M2_WERE) ? "lycanthropes" :
                (flags.warntype & M2_UNDEAD) ? "undead" :
                something);
        you_are(buf);
    } else if (Warn_of_mon && uwep) {
        const char *monster_name = get_warned_of_monster(uwep);
        if (monster_name) {
            Sprintf(buf, "aware of the presence of %s", monster_name);
            you_are(buf);
        }
    }
    if (Undead_warning) you_are("warned of undead");
    if (Searching) you_have("automatic searching");
    if (Clairvoyant) you_are("clairvoyant");
    if (Infravision) you_have("infravision");
    if (Detect_monsters) you_are("sensing the presence of monsters");
    if (u.umconf) you_are("going to confuse monsters");

    /*** Appearance and behavior ***/
    if (Adornment) {
        int adorn = 0;

        if(uleft && uleft->otyp == RIN_ADORNMENT) adorn += uleft->spe;
        if(uright && uright->otyp == RIN_ADORNMENT) adorn += uright->spe;
        if (adorn < 0)
            you_are("poorly adorned");
        else
            you_are("adorned");
    }
    if (Invisible) you_are("invisible");
    else if (Invis) you_are("invisible to others");
    /* ordinarily "visible" is redundant; this is a special case for
       the situation when invisibility would be an expected attribute */
    else if ((HInvis || EInvis || pm_invisible(youmonst.data)) && BInvis)
        you_are("visible");
    if (Displaced) you_are("displaced");
    if (Stealth) you_are("stealthy");
    if (Aggravate_monster) enl_msg("You aggravate", "", "d", " monsters");
    if (Conflict) enl_msg("You cause", "", "d", " conflict");

    /*** Transportation ***/
    if (Jumping) you_can("jump");
    if (Teleportation) you_can("teleport");
    if (Teleport_control) you_have("teleport control");
    if (Lev_at_will) you_are("levitating, at will");
    else if (Levitation) you_are("levitating"); /* without control */
    else if (Flying) you_can("fly");
    if (Wwalking) you_can("walk on water");
    if (Swimming) you_can("swim");
    if (Breathless) you_can("survive without air");
    else if (Amphibious) you_can("breathe water");
    if (Passes_walls) you_can("walk through walls");
#ifdef STEED
    /* If you die while dismounting, u.usteed is still set.  Since several
     * places in the done() sequence depend on u.usteed, just detect this
     * special case. */
    if (u.usteed && (final < 2 || strcmp(killer.name, "riding accident"))) {
        Sprintf(buf, "riding %s", y_monnam(u.usteed));
        you_are(buf);
    }
#endif
    if (u.uswallow) {
        Sprintf(buf, "swallowed by %s", a_monnam(u.ustuck));
#ifdef WIZARD
        if (wizard || final) Sprintf(eos(buf), " (%u)", u.uswldtim);
#endif
        you_are(buf);
    } else if (u.ustuck) {
        Sprintf(buf, "%s %s",
                (Upolyd && sticks(youmonst.data)) ? "holding" : "held by",
                a_monnam(u.ustuck));
        you_are(buf);
    }

    /*** Physical attributes ***/
    if (u.uhitinc)
        you_have(enlght_combatinc("to hit", u.uhitinc, final, buf));
    if (u.udaminc)
        you_have(enlght_combatinc("damage", u.udaminc, final, buf));
    if (Slow_digestion) you_have("slower digestion");
    if (Regeneration && can_regenerate()) enl_msg("You regenerate", "", "d", "");
    if (!can_regenerate()) {
        if (is_elf(youmonst.data))
            you_are("in direct contact with cold iron");
        else if (is_vampiric(youmonst.data))
            you_are("in direct contact with silver");
    }
    if (u.uspellprot || Protection) {
        int prot = 0;

        if(uleft && uleft->otyp == RIN_PROTECTION) prot += uleft->spe;
        if(uright && uright->otyp == RIN_PROTECTION) prot += uright->spe;
        if (HProtection & INTRINSIC) prot += u.ublessed;
        prot += u.uspellprot;

        if (prot < 0)
            you_are("ineffectively protected");
        else if (prot > 0)
            you_are("protected");
    }
    if (Half_gas_damage) {
        enl_msg(You_, "take", "took", " reduced poison gas damage");
    }
    /* polymorph and other shape change */
    if (Protection_from_shape_changers)
        you_are("protected from shape changers");
    if (Polymorph) you_are("polymorphing");
    if (Polymorph_control) you_have("polymorph control");
    if (u.ulycn >= LOW_PM) {
        Strcpy(buf, an(mons[u.ulycn].mname));
        you_are(buf);
    }
    if (Upolyd) {
        if (u.umonnum == u.ulycn) Strcpy(buf, "in beast form");
        else Sprintf(buf, "polymorphed into %s", an(youmonst.data->mname));
#ifdef WIZARD
        if (wizard || final) Sprintf(eos(buf), " (%d)", u.mtimedone);
#endif
        you_are(buf);
    }
    if (Unchanging) you_can("not change from your current form");
    if (Fast) you_are(Very_fast ? "very fast" : "fast");
    if (Reflecting) you_have("reflection");
    if (Free_action) you_have("free action");
    if (Fixed_abil) you_have("fixed abilities");
    if (Lifesaved)
        enl_msg("Your life ", "will be", "would have been", " saved");
    if (u.twoweap) you_are("wielding two weapons at once");

    /*** Miscellany ***/
    if (Luck) {
        ltmp = abs((int)Luck);
        Sprintf(buf, "%s%slucky",
                ltmp >= 10 ? "extremely " : ltmp >= 5 ? "very " : "",
                Luck < 0 ? "un" : "");
#ifdef WIZARD
        if (wizard || final) Sprintf(eos(buf), " (%d)", Luck);
#endif
        you_are(buf);
    }
#ifdef WIZARD
    else if (wizard || final) enl_msg("Your luck ", "is", "was", " zero");
#endif
    if (u.moreluck > 0) you_have("extra luck");
    else if (u.moreluck < 0) you_have("reduced luck");
    if (has_luckitem()) {
        ltmp = stone_luck(FALSE);
        if (ltmp <= 0)
            enl_msg("Bad luck ", "times", "timed", " out slowly for you");
        if (ltmp >= 0)
            enl_msg("Good luck ", "times", "timed", " out slowly for you");
    }

    if (u.ugangr) {
        Sprintf(buf, " %sangry with you",
                u.ugangr > 6 ? "extremely " : u.ugangr > 3 ? "very " : "");
#ifdef WIZARD
        if (wizard || final) Sprintf(eos(buf), " (%d)", u.ugangr);
#endif
        enl_msg(u_gname(), " is", " was", buf);
    } else
    /*
     * We need to suppress this when the game is over, because death
     * can change the value calculated by can_pray(), potentially
     * resulting in a false claim that you could have prayed safely.
     */
    if (!final) {
#if 0
        /* "can [not] safely pray" vs "could [not] have safely prayed" */
        Sprintf(buf, "%s%ssafely pray%s", can_pray(FALSE) ? "" : "not ",
                final ? "have " : "", final ? "ed" : "");
#else
        Sprintf(buf, "%ssafely pray", can_pray(FALSE) ? "" : "not ");
#endif
#ifdef WIZARD
        if (wizard || final) Sprintf(eos(buf), " (%d)", u.ublesscnt);
#endif
        you_can(buf);
    }

    {
        const char *p;

        buf[0] = '\0';
        if (final < 2) { /* still in progress, or quit/escaped/ascended */
            p = "survived after being killed ";
            switch (u.umortality) {
            case 0:  p = !final ? (char *)0 : "survived";  break;
            case 1:  Strcpy(buf, "once");  break;
            case 2:  Strcpy(buf, "twice");  break;
            case 3:  Strcpy(buf, "thrice");  break;
            default: Sprintf(buf, "%d times", u.umortality);
                break;
            }
        } else {    /* game ended in character's death */
            p = "are dead";
            switch (u.umortality) {
            case 0:  warning("dead without dying?");
            case 1:  break;     /* just "are dead" */
            default: Sprintf(buf, " (%d%s time!)", u.umortality,
                             ordin(u.umortality));
                break;
            }
        }
        if (p) enl_msg(You_, "have been killed ", p, buf);
    }
    dump_list_end();
    dump("", "");

    if (want_display) {
        display_nhwindow(en_win, TRUE);
        destroy_nhwindow(en_win);
    }
}

/*
 * Courtesy function for non-debug, non-explorer mode players
 * to help refresh them about who/what they are.
 * Returns FALSE if menu cancelled (dismissed with ESC), TRUE otherwise.
 */
STATIC_OVL boolean
minimal_enlightenment()
{
    winid tmpwin;
    menu_item *selected;
    anything any;
    int genidx, n;
    char buf[BUFSZ], buf2[BUFSZ];
    static const char untabbed_fmtstr[] = "%-15s: %-12s";
    static const char untabbed_deity_fmtstr[] = "%-17s%s";
    static const char tabbed_fmtstr[] = "%s:\t%-12s";
    static const char tabbed_deity_fmtstr[] = "%s\t%s";
    static const char *fmtstr;
    static const char *deity_fmtstr;

    fmtstr = iflags.menu_tab_sep ? tabbed_fmtstr : untabbed_fmtstr;
    deity_fmtstr = iflags.menu_tab_sep ?
                   tabbed_deity_fmtstr : untabbed_deity_fmtstr;
    any.a_void = 0;
    buf[0] = buf2[0] = '\0';
    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings, "Starting", FALSE);

    /* Starting name, race, role, gender */
    Sprintf(buf, fmtstr, "name", plname);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    Sprintf(buf, fmtstr, "race", urace.noun);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    Sprintf(buf, fmtstr, "role",
            (flags.initgend && urole.name.f) ? urole.name.f : urole.name.m);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    Sprintf(buf, fmtstr, "gender", genders[flags.initgend].adj);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

    /* Starting alignment */
    Sprintf(buf, fmtstr, "alignment", align_str(u.ualignbase[A_ORIGINAL]));
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

    /* Current name, race, role, gender */
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", FALSE);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings, "Current", FALSE);
    Sprintf(buf, fmtstr, "race", Upolyd ? youmonst.data->mname : urace.noun);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    if (Upolyd) {
        Sprintf(buf, fmtstr, "role (base)",
                (u.mfemale && urole.name.f) ? urole.name.f : urole.name.m);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    } else {
        Sprintf(buf, fmtstr, "role",
                (flags.female && urole.name.f) ? urole.name.f : urole.name.m);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    }
    /* don't want poly_gender() here; it forces `2' for non-humanoids */
    genidx = is_neuter(youmonst.data) ? 2 : flags.female;
    Sprintf(buf, fmtstr, "gender", genders[genidx].adj);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    if (Upolyd && (int)u.mfemale != genidx) {
        Sprintf(buf, fmtstr, "gender (base)", genders[u.mfemale].adj);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    }

    /* Current alignment */
    Sprintf(buf, fmtstr, "alignment", align_str(u.ualign.type));
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

    /* Current position of hero */
    if (wizard) {
        Sprintf(buf2, "(%2d,%2d)", u.ux, u.uy);
        Sprintf(buf, fmtstr, "position", buf2);
        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);
    }

    /* Deity list */
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", FALSE);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings, "Deities", FALSE);
    Sprintf(buf2, deity_fmtstr, align_gname(A_CHAOTIC),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type
             && u.ualign.type == A_CHAOTIC) ? " (s,c)" :
            (u.ualignbase[A_ORIGINAL] == A_CHAOTIC)       ? " (s)" :
            (u.ualign.type   == A_CHAOTIC)       ? " (c)" : "");
    Sprintf(buf, fmtstr, "Chaotic", buf2);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

    Sprintf(buf2, deity_fmtstr, align_gname(A_NEUTRAL),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type
             && u.ualign.type == A_NEUTRAL) ? " (s,c)" :
            (u.ualignbase[A_ORIGINAL] == A_NEUTRAL)       ? " (s)" :
            (u.ualign.type   == A_NEUTRAL)       ? " (c)" : "");
    Sprintf(buf, fmtstr, "Neutral", buf2);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

    Sprintf(buf2, deity_fmtstr, align_gname(A_LAWFUL),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type &&
             u.ualign.type == A_LAWFUL)  ? " (s,c)" :
            (u.ualignbase[A_ORIGINAL] == A_LAWFUL)        ? " (s)" :
            (u.ualign.type   == A_LAWFUL)        ? " (c)" : "");
    Sprintf(buf, fmtstr, "Lawful", buf2);
    add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, buf, FALSE);

    end_menu(tmpwin, "Base Attributes");
    n = select_menu(tmpwin, PICK_NONE, &selected);
    destroy_nhwindow(tmpwin);
    return (n != -1);
}

int
do_naming(typ)
int typ;
{
    winid win;
    anything any;
    menu_item *pick_list = NULL;
    struct obj *obj;
    char allowall[2];
    /* if player wants a,b,c instead of i,o when looting, do that here too */
    boolean abc = iflags.lootabc;

    static NEARDATA const char callable[] = {
        SCROLL_CLASS, POTION_CLASS, WAND_CLASS, RING_CLASS, AMULET_CLASS,
        GEM_CLASS, SPBOOK_CLASS, ARMOR_CLASS, TOOL_CLASS, 0
    };

    char ch = 'q';
    if (!typ) {
        any.a_void = 0;
        win = create_nhwindow(NHW_MENU);
        start_menu(win);

        /* the accelerator keys are chosen to be compatible with NAO and
         * where possible similar to AceHack's keys. That explains the
         * choice for the group accelerators. */
        any.a_char = 'm'; /* group accelerator 'C' */
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, abc ? 0 : any.a_char, 'C', ATR_NONE,
                 "Name a monster", MENU_UNSELECTED);

        if (invent) {
            /* we use y and n as accelerators so that we can accept user's
            response keyed to old "name an individual object?" prompt */
            any.a_char = 'i'; /* group accelerator 'y' */
            add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, abc ? 0 : any.a_char, 'y', ATR_NONE,
                     "Name an individual item", MENU_UNSELECTED);

            any.a_char = 'o'; /* group accelerator 'n' */
            add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, abc ? 0 : any.a_char, 'n', ATR_NONE,
                     "Name all items of a certain type", MENU_UNSELECTED);
        }

        any.a_char = 'f'; /* group accelerator ',' (or ':' instead?) */
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, abc ? 0 : any.a_char, ',', ATR_NONE,
                 "Name the type of an item on the floor", MENU_UNSELECTED);

        any.a_char = 'a'; /* group accelerator 'l' */
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, abc ? 0 : any.a_char, 'l', ATR_NONE,
                 "Annotate the current level", MENU_UNSELECTED);

        if ((flags.last_broken_otyp != STRANGE_OBJECT) &&
            (objects[flags.last_broken_otyp].oc_uname == NULL) &&
            (!objects[flags.last_broken_otyp].oc_name_known)) {
            char buf[BUFSZ];
            Sprintf(buf, "Name %s (last broken object)",
                    an(obj_typename(flags.last_broken_otyp)));
            any.a_char = 'b'; /* group accelerator 'V' */
            add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, abc ? 0 : any.a_char, 'V', ATR_NONE, buf,
                     MENU_UNSELECTED);
        }

        if ((flags.last_picked_up_otyp != STRANGE_OBJECT) &&
            (objects[flags.last_picked_up_otyp].oc_uname == NULL) &&
            (!objects[flags.last_picked_up_otyp].oc_name_known)) {
            char buf[BUFSZ];
            Sprintf(buf, "Name %s (last picked up object)",
                    an(obj_typename(flags.last_picked_up_otyp)));
            any.a_char = 'p'; /* group accelerator 'P' */
            add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, abc ? 0 : any.a_char, 'P', ATR_NONE, buf,
                     MENU_UNSELECTED);
        }

        any.a_int = 0;
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, 0, 'q', ATR_NONE, "", MENU_UNSELECTED);

        end_menu(win, "What do you wish to name?");
        if (select_menu(win, PICK_ONE, &pick_list) > 0) {
            ch = pick_list[0].item.a_char;
            free(pick_list);
        }
        destroy_nhwindow(win);
    }

    switch (ch) {
        default:
        case 'q':
            break;

        case 'm':
            do_mname();
            break;
            /* cases 1 & 2 duplicated from ddocall() */

        case 'i':
            allowall[0] = ALL_CLASSES; allowall[1] = '\0';
            obj = getobj(allowall, "name");
            if (obj) {
                do_oname(obj);
            }
            break;

        case 'o':
            obj = getobj(callable, "call");
            if (obj) {
                /* behave as if examining it in inventory;
                   this might set dknown if it was picked up
                   while blind and the hero can now see */
                (void) xname(obj);

                if (!obj->dknown) {
                    You("would never recognize another one.");
                    return 0;
                }
                docall(obj);
            }
            break;

        case 'f':
            namefloorobj();
            break;

        case 'a':
            donamelevel();
            break;

        case 'b':
            docall_input(flags.last_broken_otyp);
            break;

        case 'p':
            docall_input(flags.last_picked_up_otyp);
            break;
    }
    return 0;
}

int
do_naming_mname()
{
    if (iflags.vanilla_ui_behavior) return do_naming(1);
    return do_naming(0);
}

int
do_naming_ddocall()
{
    return do_naming(0);
}

STATIC_PTR int
doattributes()
{
    if (!minimal_enlightenment())
        return 0;
    if (wizard || discover)
        enlightenment(0, TRUE);
    return 0;
}

/* KMH, #conduct
 * (shares enlightenment's tense handling)
 */
STATIC_PTR int
doconduct()
{
    show_conduct(0, TRUE);
    return 0;
}

void
show_conduct(final, want_disp)
int final;
boolean want_disp;
{
    char buf[BUFSZ];
    int ngenocided;
    int cdt;

    want_display = want_disp;

    /* Create the conduct window */
    Sprintf(buf, "Voluntary challenges:");
    if (want_display) {
        en_win = create_nhwindow(NHW_MENU);
        putstr(en_win, 0, buf);
        putstr(en_win, 0, "");
    }
    dump_title(buf);
    dump_list_start();

    if (heaven_or_hell_mode) {
        if (hell_and_hell_mode) {
            you_have_been("following the path of Hell and Hell");
        } else {
            you_have_been("following the path of Heaven or Hell");
        }
    }

    if (marathon_mode) {
        you_have_been("playing in marathon mode");
    }

    if (flags.deathdropless)
        you_have_been("ignoring all death drops");

    /* list all major conducts */

    for(cdt=FIRST_CONDUCT; cdt<=LAST_CONDUCT; cdt++) {
        if(successful_cdt(cdt)) {
            if (!superfluous_cdt(cdt))
                enl_msg(conducts[cdt].prefix, /* "You " */
                        conducts[cdt].presenttxt, /* "have been"    */
                        conducts[cdt].pasttxt, /* "were"    */
                        conducts[cdt].suffix); /* "a pacifist"  */
        } else if(intended_cdt(cdt)) {
            you_have_X(conducts[cdt].failtxt); /* "pretended to be a pacifist" */
        }
    }

    if (failed_cdt(CONDUCT_PACIFISM) || failed_cdt(CONDUCT_SADISM)) {
        if (u.uconduct.killer == 0) {
            you_have_never("killed a creature");
        } else {
            Sprintf(buf, "killed %ld creature%s", u.uconduct.killer,
                    plur(u.uconduct.killer));
            you_have_X(buf);
        }
    }

    /* now list the remaining statistical details */

    if (!u.uconduct.weaphit)
        you_have_never("hit with a wielded weapon");
    else if (wizard || final) {
        Sprintf(buf, "used a wielded weapon %ld time%s",
                u.uconduct.weaphit, plur(u.uconduct.weaphit));
        you_have_X(buf);
    }

#ifdef WIZARD
    if ((wizard || final) && u.uconduct.literate) {
        Sprintf(buf, "read items or engraved %ld time%s",
                u.uconduct.literate, plur(u.uconduct.literate));
        you_have_X(buf);
    }
    if ((wizard || final) && u.uconduct.armoruses) {
        Sprintf(buf, "put on armor %ld time%s",
                u.uconduct.armoruses, plur(u.uconduct.armoruses));
        you_have_X(buf);
    }
#endif
    if (!u.uconduct.non_racial_armor &&
        /* only show when armor was worn at all */
        u.uconduct.armoruses > 0) {
        Sprintf(buf, "wearing only %s armor", urace.adj);
        you_have_been(buf);
    }

    ngenocided = num_genocides();
    if (ngenocided == 0) {
        you_have_never("genocided any monsters");
    } else {
        Sprintf(buf, "genocided %d type%s of monster%s",
                ngenocided, plur(ngenocided), plur(ngenocided));
        you_have_X(buf);
    }

    if (!u.uconduct.polypiles)
        you_have_never("polymorphed an object");
#ifdef WIZARD
    else if (wizard || final) {
        Sprintf(buf, "polymorphed %ld item%s",
                u.uconduct.polypiles, plur(u.uconduct.polypiles));
        you_have_X(buf);
    }
#endif

    if (!u.uconduct.polyselfs)
        you_have_never("changed form");
#ifdef WIZARD
    else if (wizard || final) {
        Sprintf(buf, "changed form %ld time%s",
                u.uconduct.polyselfs, plur(u.uconduct.polyselfs));
        you_have_X(buf);
    }
#endif

    if (!u.uconduct.wishes)
        you_have_X("used no wishes");
    else {
        Sprintf(buf, "used %ld wish%s",
                u.uconduct.wishes, (u.uconduct.wishes > 1L) ? "es" : "");
        you_have_X(buf);

        if (u.uconduct.wishmagic) {
            Sprintf(buf, "used %ld wish%s for magical items",
                    u.uconduct.wishmagic, (u.uconduct.wishmagic > 1L) ? "es" : "");
            you_have_X(buf);
        }

        if (u.uconduct.wisharti) {
            Sprintf(buf, "used %ld wish%s for %s",
                    u.uconduct.wisharti, (u.uconduct.wisharti > 1L) ? "es" : "",
                    (u.uconduct.wisharti == 1L) ? "an artifact" : "artifacts");
            you_have_X(buf);
        }

        if (!u.uconduct.wisharti)
            enl_msg(You_, "have not wished", "did not wish",
                    " for any artifacts");
    }

#ifdef ELBERETH_CONDUCT
#ifdef ELBERETH
    /* no point displaying the conduct if Elbereth doesn't do anything */
    if (flags.elberethignore) {
        you_have_been("ignored by Elbereth");
    } else {
        if (u.uconduct.elbereths) {
            Sprintf(buf, "engraved Elbereth %ld time%s",
                    u.uconduct.elbereths, plur(u.uconduct.elbereths));
            you_have_X(buf);
        } else {
            you_have_never("engraved Elbereth");
        }
    }
#endif /* ELBERETH */
#endif /* ELBERETH_CONDUCT */

    if (wizard || discover || final) {
        if (!flags.bones) {
            you_have_X("disabled loading of bones levels");
        } else if (!u.uconduct.bones) {
            you_have_never("encountered a bones level");
        } else {
            Sprintf(buf, "encountered %ld bones level%s",
                    u.uconduct.bones, plur(u.uconduct.bones));
            you_have_X(buf);
        }
    }

#ifdef RECORD_ACHIEVE
    if ((wizard || final) && !u.uconduct.sokoban) {
        you_have_never("used any Sokoban shortcuts");
    } else if (wizard || final) {
        Sprintf(buf, "used Sokoban shortcuts %ld time%s",
                u.uconduct.sokoban, plur(u.uconduct.sokoban));
        you_have_X(buf);
    }
#endif

    dump_list_end();
    dump("", "");

    /* Pop up the window and wait for a key */
    if (want_display) {
        display_nhwindow(en_win, TRUE);
        destroy_nhwindow(en_win);
    }
}

#ifndef M
# ifndef NHSTDC
#  define M(c)      (0x80 | (c))
# else
#  define M(c)      ((c) - 128)
# endif /* NHSTDC */
#endif
#ifndef C
#define C(c)        (0x1f & (c))
#endif

#if 0
    {C('o'), TRUE, dooverview_or_wiz_where, NULL}, /* depending on wizard status */
    {C('v'), TRUE, tutorial_redisplay, NULL},
    {M('n'), TRUE, ddocall, NULL},
    {M('N'), TRUE, ddocall, NULL},
    {'\'', TRUE, doautofight, NULL},
    {0, 0, 0, 0}
#endif

struct ext_func_tab extcmdlist[] = {
    { '#',    "#", "perform an extended command", doextcmd, IFBURIED | GENERALCMD, NULL },
    { M('?'), "?", "list all extended commands", doextlist, IFBURIED | AUTOCOMPLETE | GENERALCMD, NULL },
    { M('a'), "adjust", "adjust inventory letters", doorganize, IFBURIED | AUTOCOMPLETE, NULL },
    { M('A'), "annotate", "name current level", donamelevel, IFBURIED | AUTOCOMPLETE, NULL },
    {   'a',  "apply", "apply (use) a tool (pick-axe, key, lamp...)", doapply, 0, NULL },
    { C('x'), "attributes", "show your attributes", doattributes, IFBURIED, NULL },
    {   'v',  "autoexplore", "automatic exploration of the dungeon", doautoexplore, AUTOCOMPLETE, NULL },
    {   '@',  "autopickup", "toggle the pickup option on/off", dotogglepickup, IFBURIED, NULL },
    {   'C',  "call", "call (name) something", do_naming_ddocall, IFBURIED, NULL },
    {   'Z',  "cast", "zap (cast) a spell", docast, IFBURIED, NULL },
    { M('c'), "chat", "talk to someone", dotalk, IFBURIED | AUTOCOMPLETE, NULL },
    {   'c',  "close", "close a door", doclose, 0, NULL },
    { M('C'), "conduct", "list voluntary challenges you have adhered to", doconduct, IFBURIED | AUTOCOMPLETE,
               NULL },
    { M('d'), "dip", "dip an object into something", dodip, AUTOCOMPLETE, NULL },
    {   '>',  "down", "go down a staircase", dodown, 0, NULL },
    {   'd',  "drop", "drop an item", dodrop, 0, NULL },
    {   'D',  "droptype", "drop specific item types", doddrop, 0, NULL },
    {   'e',  "eat", "eat something", doeat, 0, NULL },
    {   'E',  "engrave", "engrave writing on the floor", doengrave, 0, NULL },
#ifdef ELBERETH
    { C('e'), "engraveelbereth", "engrave \"Elbereth\" on the floor", doengrave_elbereth, 0, NULL },
#endif
    { M('e'), "enhance", "advance or check weapon and spell skills", enhance_weapon_skill,
              IFBURIED | AUTOCOMPLETE, NULL },
    {  '\0',  "exploremode", "enter explore (discovery) mode", enter_explore_mode, IFBURIED, NULL },
    {   'f',  "fire", "fire ammunition from quiver", dofire, 0, NULL },
    { M('f'), "force", "force a lock", doforce, AUTOCOMPLETE, NULL },
    {   ';',  "glance", "show what type of thing a map symbol corresponds to", doquickwhatis,
              IFBURIED | GENERALCMD, NULL },
    {   '?',  "help", "give a help message", dohelp, IFBURIED | GENERALCMD, NULL },
#if NEXT_VERSION
    {  '\0',  "herecmdmenu", "show menu of commands you can do here", doherecmdmenu, IFBURIED, NULL },
#endif
    {  '\0',  "history", "show long version and game history", dohistory, IFBURIED | GENERALCMD, NULL },
    {   'i',  "inventory", "show your inventory", ddoinv, IFBURIED, NULL },
    {   'I',  "inventtype", "inventory specific item types", dotypeinv, IFBURIED, NULL },
    { M('i'), "invoke", "invoke an object's special powers", doinvoke, IFBURIED | AUTOCOMPLETE, NULL },
    { M('j'), "jump", "jump to another location", dojump, AUTOCOMPLETE, NULL },
    { C('d'), "kick", "kick something", dokick, 0, NULL },
    {  '\\',  "known", "show what object types have been discovered", dodiscovered, IFBURIED | GENERALCMD,
               NULL },
    {   '`',  "knownclass", "show discovered types for one class of objects", doclassdisco,
              IFBURIED | GENERALCMD, NULL },
    {  '\0',  "levelchange", "change experience level", wiz_level_change,
              IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
    {  '\0',  "lightsources", "show mobile light sources", wiz_light_sources,
              IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
    {   ':',  "look", "look at what is here", dolook, IFBURIED, NULL },
    { M('l'), "loot", "loot a box on the floor", doloot, AUTOCOMPLETE, NULL },
#ifdef DEBUG_MIGRATING_MONS
    {  '\0',  "migratemons", "migrate N random monsters", wiz_migrate_mons,
              IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
#endif
    {  '\0',  "monpolycontrol", "control monster polymorphs", wiz_mon_polycontrol, AUTOCOMPLETE | WIZMODECMD,
              NULL },
    { M('m'), "monster", "use monster's special ability", domonability, IFBURIED | AUTOCOMPLETE, NULL },
    {   'N',  "name", "name a monster or an object", do_naming_ddocall, IFBURIED | AUTOCOMPLETE, NULL },
    { M('o'), "offer", "offer a sacrifice to the gods", dosacrifice, AUTOCOMPLETE, NULL },
    {   'o',  "open", "open a door", doopen, 0, NULL },
    {   'O',  "options", "show option settings, possibly change them", doset, IFBURIED | GENERALCMD, NULL },
    { C('o'), "overview", "show a summary of the explored dungeon", dooverview, IFBURIED | AUTOCOMPLETE,
              NULL },
    {  '\0',  "panic", "test panic routine (fatal to game)", wiz_panic, IFBURIED | AUTOCOMPLETE | WIZMODECMD,
              NULL },
    {   'p',  "pay", "pay your shopping bill", dopay, 0, NULL },
    {   ',',  "pickup", "pick up things at the current location", dopickup, 0, NULL },
    {  '\0',  "polyself", "polymorph self", wiz_polyself, IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
    { M('p'), "pray", "pray to the gods for help", dopray, IFBURIED | AUTOCOMPLETE, NULL },
    { C('p'), "prevmsg", "view recent game messages", doprev_message, IFBURIED | GENERALCMD, NULL },
    {   'P',  "puton", "put on an accessory (ring, amulet, etc)", doputon, 0, NULL },
    {   'q',  "quaff", "quaff (drink) something", dodrink, 0, NULL },
    { M('q'), "quit", "exit without saving current game", done2, IFBURIED | AUTOCOMPLETE | GENERALCMD, NULL },

    {   'Q',  "quiver", "select ammunition for quiver", dowieldquiver, 0, NULL },
    {   'r',  "read", "read a scroll or spellbook", doread, 0, NULL },
    { C('r'), "redraw", "redraw screen", doredraw, IFBURIED | GENERALCMD, NULL },
    {   'R',  "remove", "remove an accessory (ring, amulet, etc)", doremring, 0, NULL },
#ifdef STEED
    { M('R'), "ride", "mount or dismount a saddled steed", doride, AUTOCOMPLETE, NULL },
#endif
    {  '\0',  "rooms", "show room numbers", wiz_show_rooms, AUTOCOMPLETE | WIZMODECMD, NULL },
    { M('r'), "rub", "rub a lamp or a stone", dorub, AUTOCOMPLETE, NULL },
    {   'S',  "save", "save the game and exit", dosave, IFBURIED | GENERALCMD, NULL },

#ifdef DUMP_LOG
    {  '\0',  "screenshot", "output current map to a html file", dump_screenshot, AUTOCOMPLETE, NULL },
#endif
    {   's',  "search", "search for traps and secret doors", dosearch, IFBURIED, "searching" },
    {   '*',  "seeall", "show all equipment in use", doprinuse, IFBURIED, NULL },
    {   '^',  "seetrap", "show the type of adjacent trap", doidtrap, IFBURIED, NULL },
    { AMULET_SYM, "seeamulet", "show the amulet currently worn", dopramulet, IFBURIED, NULL },
    { ARMOR_SYM, "seearmor", "show the armor currently worn", doprarm, IFBURIED, NULL },
    { GOLD_SYM, "seegold", "count your gold", doprgold, IFBURIED, NULL },
    { RING_SYM, "seerings", "show the ring(s) currently worn", doprring, IFBURIED, NULL },
    { SPBOOK_SYM, "seespells", "list and reorder known spells", dovspell, IFBURIED, NULL },
    { TOOL_SYM, "seetools", "show the tools currently in use", doprtool, IFBURIED, NULL },
    { WEAPON_SYM, "seeweapon", "show the weapon currently wielded", doprwep, IFBURIED, NULL },
    {  '\0',  "seenv", "show seen vectors", wiz_show_seenv, IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
#ifdef LIVELOG_SHOUT
    {  '\0',  "shout", "shout something", doshout, AUTOCOMPLETE, NULL },
#endif
    {  '\0',  "showcolors", "show available colors", debug_show_colors, AUTOCOMPLETE, NULL },
    {  '\0',  "showkills", "show list of monsters killed", wiz_showkills, AUTOCOMPLETE, NULL },
    { M('s'), "sit", "sit down", dosit, AUTOCOMPLETE, NULL },
    {  '\0',  "stats", "show memory statistics", wiz_show_stats, IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
    { C('z'), "suspend", "suspend the game", dosuspend_core, IFBURIED | GENERALCMD
#ifndef SUSPEND
                            | CMD_NOT_AVAILABLE
#endif /* SUSPEND */
    , NULL},
    {   'x', "swap", "swap wielded and secondary weapons", doswapweapon, 0, NULL },
    {   'T', "takeoff", "take off one piece of armor", dotakeoff, 0, NULL },
    {   'A', "takeoffall", "remove all armor", doddoremarm, 0, NULL },
    { C('t'), "teleport", "teleport around the level", dotelecmd, IFBURIED, NULL },
    /* \177 == <del> aka <delete> aka <rubout>; some terminals have an
       option to swap it with <backspace> so if there's a key labeled
       <delete> it may or may not actually invoke the #terrain command */
    { '\177', "terrain", "view map without monsters or objects obstructing it",
              doterrain, IFBURIED | AUTOCOMPLETE, NULL },
#if NEXT_VERSION
    {  '\0', "therecmdmenu", "menu of commands you can do from here to adjacent spot", dotherecmdmenu },
#endif
    {   't',  "throw", "throw something", dothrow, 0, NULL },
    {  '\0',  "timeout", "look at timeout queue and hero's timed intrinsics", wiz_timeout_queue,
              IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
    { M('T'), "tip", "empty a container", dotip, AUTOCOMPLETE, NULL },
    {   '_',  "travel", "travel to a specific location on the map", dotravel, 0, NULL },

    { M('t'), "turn", "turn undead away", doturn, IFBURIED | AUTOCOMPLETE, NULL },
    {   'X',  "twoweapon", "toggle two-weapon combat", dotwoweapon, AUTOCOMPLETE, NULL },
    { M('u'), "untrap", "untrap something", dountrap, AUTOCOMPLETE, 0 },
    {   '<',  "up", "go up a staircase", doup, 0, NULL },

    {  '\0',  "vanquished", "list vanquished monsters", dovanquished,
              IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
    {   'V',  "versionshort", "show version", doversion, IFBURIED | GENERALCMD, NULL },
    {  '\0',  "version", "list compile time options for this version of NetHack", doextversion,
              IFBURIED | AUTOCOMPLETE | GENERALCMD, NULL },
    {  '\0',  "vision", "show vision array", wiz_show_vision, IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },

    {   '.',  "wait", "rest one move while doing nothing", donull, IFBURIED, "waiting" },
    {   'W',  "wear", "wear a piece of armor", dowear, 0, NULL },
    {   '&',  "whatdoes", "tell what a command does", dowhatdoes, IFBURIED, NULL },
    {   '/',  "whatis", "show what type of thing a symbol corresponds to", dowhatis, IFBURIED | GENERALCMD,
              NULL },
    {   'w',  "wield", "wield (put in use) a weapon", dowield, 0, NULL },
    { M('w'), "wipe", "wipe off your face", dowipe, AUTOCOMPLETE, NULL },
#ifdef DEBUG
    {  '\0',  "wizbury", "bury objs under and around you", wiz_debug_cmd_bury,
              IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
#endif
    {  '\0',  "wizdetect", "reveal hidden things within a small radius", wiz_detect,
              IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
#ifdef DEBUG
    {  '\0',  "wizdebug", "wizard debug command", wiz_debug_cmd, AUTOCOMPLETE | WIZMODECMD, NULL },
#endif
    { C('g'), "wizgenesis", "create a monster", wiz_genesis, IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
    { C('i'), "wizidentify", "identify all items in inventory", wiz_identify,
              IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
#if NEXT_VERSION
    {  '\0',  "wizintrinsic", "set an intrinsic", wiz_intrinsic, IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
#endif
    { C('v'), "wizlevelport", "teleport to another level", wiz_level_tele,
              IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
#if NEXT_VERSION
    {  '\0',  "wizmakemap", "recreate the current level", wiz_makemap, IFBURIED | WIZMODECMD, NULL },
#endif
    { C('f'), "wizmap", "map the level", wiz_map, IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
#if 0
    {  '\0',  "wizrumorcheck", "verify rumor boundaries", wiz_rumor_check,
              IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
    {  '\0',  "wizsmell", "smell monster", wiz_smell, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#endif
    {  '\0',  "wizwhere", "show locations of special levels", wiz_where, IFBURIED | AUTOCOMPLETE | WIZMODECMD,
              NULL },
    { C('w'), "wizwish", "wish for something", wiz_wish, IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
    {  '\0',  "wmode", "show wall modes", wiz_show_wmodes, IFBURIED | AUTOCOMPLETE | WIZMODECMD, NULL },
    {   'z',  "zap", "zap a wand", dozap, 0, NULL },
    {  '\0',  (char *) 0, (char *) 0, donull, 0, (char *) 0 } /* sentinel */
};

const char *
key2extcmddesc(key)
uchar key;
{
    static char key2cmdbuf[48];
    const struct movcmd *mov;
    int k, c;
    uchar M_5 = (uchar) M('5'), M_0 = (uchar) M('0');

    /* need to check for movement commands before checking the extended
       commands table because it contains entries for number_pad commands
       that match !number_pad movement (like 'j' for "jump") */
    key2cmdbuf[0] = '\0';
    if (movecmd(k = key)) {
        Strcpy(key2cmdbuf, "move"); /* "move or attack"? */
    } else if (movecmd(k = unctrl(key))) {
        Strcpy(key2cmdbuf, "rush");
    } else if (movecmd(k = (Cmd.num_pad ? unmeta(key) : lowc(key)))) {
        Strcpy(key2cmdbuf, "run");
    }
#if NEXT_VERSION
    if (*key2cmdbuf) {
        for (mov = &movtab[0]; mov->k1; ++mov) {
            c = !Cmd.num_pad ? (!Cmd.swap_yz ? mov->k1 : mov->k2)
                             : (!Cmd.phone_layout ? mov->k3 : mov->k4);
            if (c == k) {
                Sprintf(eos(key2cmdbuf), " %s (screen %s)",
                        mov->txt, mov->alt);
                return key2cmdbuf;
            }
        }
    } else if (digit(key) || (Cmd.num_pad && digit(unmeta(key)))) {
        key2cmdbuf[0] = '\0';
        if (!Cmd.num_pad) {
            Strcpy(key2cmdbuf, "start of, or continuation of, a count");
        } else if (key == '5' || key == M_5) {
            Sprintf(key2cmdbuf, "%s prefix",
                    (!!Cmd.pcHack_compat ^ (key == M_5)) ? "run" : "rush");
        } else if (key == '0' || (Cmd.pcHack_compat && key == M_0)) {
            Strcpy(key2cmdbuf, "synonym for 'i'");
        }
        if (*key2cmdbuf) {
            return key2cmdbuf;
        }
    }
#endif
    if (Cmd.commands[key]) {
        if (Cmd.commands[key]->ef_txt) {
            return Cmd.commands[key]->ef_desc;
        }

    }
    return (char *) 0;
}

boolean
bind_key(key, command)
uchar key;
const char *command;
{
    struct ext_func_tab *extcmd;

    /* special case: "nothing" is reserved for unbinding */
    if (!strcmp(command, "nothing")) {
        Cmd.commands[key] = (struct ext_func_tab *) 0;
        return TRUE;
    }

    for (extcmd = extcmdlist; extcmd->ef_txt; extcmd++) {
        if (strcmp(command, extcmd->ef_txt)) {
            continue;
        }
        Cmd.commands[key] = extcmd;
#if 0 /* silently accept key binding for unavailable command (!SHELL,&c) */
        if ((extcmd->flags & CMD_NOT_AVAILABLE) != 0) {
            char buf[BUFSZ];

            Sprintf(buf, cmdnotavail, extcmd->ef_txt);
            config_error_add("%s", buf);
        }
#endif
        return TRUE;
    }

    return FALSE;
}

/* initialize all keyboard commands */
void
commands_init()
{
    struct ext_func_tab *extcmd;

    for (extcmd = extcmdlist; extcmd->ef_txt; extcmd++) {
        if (extcmd->key) {
            Cmd.commands[extcmd->key] = extcmd;
        }
    }

    (void) bind_key(C('l'), "redraw"); /* if number_pad is set */
    /*       'b', 'B' : go sw */
    /*       'F' : fight (one time) */
    /*       'g', 'G' : multiple go */
    /*       'h', 'H' : go west */
    (void) bind_key('h',    "help"); /* if number_pad is set */
    (void) bind_key('j',    "jump"); /* if number_pad is on */
    /*       'j', 'J', 'k', 'K', 'l', 'L', 'm', 'M', 'n', 'N' move commands */
    (void) bind_key('k',    "kick"); /* if number_pad is on */
    (void) bind_key('l',    "loot"); /* if number_pad is on */
    (void) bind_key(C('n'), "annotate"); /* if number_pad is on */
    (void) bind_key(M('n'), "name");
    (void) bind_key(M('N'), "name");
    (void) bind_key('u',    "untrap"); /* if number_pad is on */

    /* alt keys: */
    (void) bind_key(M('O'), "overview");
    (void) bind_key(M('2'), "twoweapon");

    /* wait_on_space */
    (void) bind_key(' ',    "wait");
}

char
cmd_from_func(fn)
int NDECL((*fn));
{
    int i;

    for (i = 0; i < 256; ++i) {
        if (Cmd.commands[i] && Cmd.commands[i]->ef_funct == fn) {
            return (char) i;
        }
    }

    return '\0';
}

/*
 * wizard mode sanity_check code
 */

static const char template[] = "%-27s  %4ld  %6ld";
static const char stats_hdr[] = "                             count  bytes";
static const char stats_sep[] = "---------------------------  ----- -------";

STATIC_OVL int
size_obj(otmp)
struct obj *otmp;
{
    int sz = (int) sizeof (struct obj);

    if (otmp->oextra) {
        sz += (int) sizeof (struct oextra);
        if (ONAME(otmp))
            sz += (int) strlen(ONAME(otmp)) + 1;
        if (OMONST(otmp))
            sz += size_monst(OMONST(otmp), FALSE);
        if (OMID(otmp))
            sz += (int) sizeof (unsigned);
        if (OLONG(otmp))
            sz += (int) sizeof (long);
        if (OMAILCMD(otmp))
            sz += (int) strlen(OMAILCMD(otmp)) + 1;
    }
    return sz;
}

STATIC_OVL void
count_obj(chain, total_count, total_size, top, recurse)
struct obj *chain;
long *total_count;
long *total_size;
boolean top;
boolean recurse;
{
    long count, size;
    struct obj *obj;

    for (count = size = 0, obj = chain; obj; obj = obj->nobj) {
        if (top) {
            count++;
            size += size_obj(obj);
        }
        if (recurse && obj->cobj) {
            count_obj(obj->cobj, total_count, total_size, TRUE, TRUE);
        }
    }
    *total_count += count;
    *total_size += size;
}

STATIC_OVL void
obj_chain(win, src, chain, force, total_count, total_size)
winid win;
const char *src;
struct obj *chain;
boolean force;
long *total_count;
long *total_size;
{
    char buf[BUFSZ];
    long count = 0L, size = 0L;

    count_obj(chain, &count, &size, TRUE, FALSE);

    if (count || size || force) {
        *total_count += count;
        *total_size += size;
        Sprintf(buf, template, src, count, size);
        putstr(win, 0, buf);
    }
}

STATIC_OVL void
mon_invent_chain(win, src, chain, total_count, total_size)
winid win;
const char *src;
struct monst *chain;
long *total_count;
long *total_size;
{
    char buf[BUFSZ];
    long count = 0, size = 0;
    struct monst *mon;

    for (mon = chain; mon; mon = mon->nmon)
        count_obj(mon->minvent, &count, &size, TRUE, FALSE);
    *total_count += count;
    *total_size += size;
    Sprintf(buf, template, src, count, size);
    putstr(win, 0, buf);
}

STATIC_OVL void
contained_stats(win, src, total_count, total_size)
winid win;
const char *src;
long *total_count;
long *total_size;
{
    char buf[BUFSZ];
    long count = 0, size = 0;
    struct monst *mon;

    count_obj(invent, &count, &size, FALSE, TRUE);
    count_obj(fobj, &count, &size, FALSE, TRUE);
    count_obj(level.buriedobjlist, &count, &size, FALSE, TRUE);
    count_obj(migrating_objs, &count, &size, FALSE, TRUE);
    /* DEADMONSTER check not required in this loop since they have no
     * inventory */
    for (mon = fmon; mon; mon = mon->nmon)
        count_obj(mon->minvent, &count, &size, FALSE, TRUE);
    for (mon = migrating_mons; mon; mon = mon->nmon)
        count_obj(mon->minvent, &count, &size, FALSE, TRUE);

    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(buf, template, src, count, size);
        putstr(win, 0, buf);
    }
}

STATIC_OVL int
size_monst(mtmp, incl_wsegs)
struct monst *mtmp;
boolean incl_wsegs;
{
    int sz = (int) sizeof (struct monst);

    if (mtmp->wormno && incl_wsegs)
        sz += size_wseg(mtmp);

    if (mtmp->mextra) {
        sz += (int) sizeof (struct mextra);
        if (MNAME(mtmp))
            sz += (int) strlen(MNAME(mtmp)) + 1;
        if (EGD(mtmp))
            sz += (int) sizeof (struct egd);
        if (EPRI(mtmp))
            sz += (int) sizeof (struct epri);
        if (ESHK(mtmp))
            sz += (int) sizeof (struct eshk);
        if (EMIN(mtmp))
            sz += (int) sizeof (struct emin);
        if (EDOG(mtmp))
            sz += (int) sizeof (struct edog);
        /* mextra->mcorpsenm doesn't point to more memory */
    }
    return sz;
}

STATIC_OVL void
mon_chain(win, src, chain, force, total_count, total_size)
winid win;
const char *src;
struct monst *chain;
boolean force;
long *total_count;
long *total_size;
{
    char buf[BUFSZ];
    long count, size;
    struct monst *mon;
    /* mon->wormno means something different for migrating_mons and mydogs */
    boolean incl_wsegs = !strcmpi(src, "fmon");

    count = size = 0L;
    for (mon = chain; mon; mon = mon->nmon) {
        count++;
        size += size_monst(mon, incl_wsegs);
    }
    if (count || size || force) {
        *total_count += count;
        *total_size += size;
        Sprintf(buf, template, src, count, size);
        putstr(win, 0, buf);
    }
}

STATIC_OVL void
misc_stats(win, total_count, total_size)
winid win;
long *total_count;
long *total_size;
{
    char buf[BUFSZ], hdrbuf[QBUFSZ];
    long count, size;
    int idx;
    struct trap *tt;
    struct damage *sd; /* shop damage */
    struct kinfo *k; /* delayed killer */
    struct cemetery *bi; /* bones info */

    /* traps and engravings are output unconditionally;
     * others only if nonzero
     */
    count = size = 0L;
    for (tt = ftrap; tt; tt = tt->ntrap) {
        ++count;
        size += (long) sizeof *tt;
    }
    *total_count += count;
    *total_size += size;
    Sprintf(hdrbuf, "traps, size %ld", (long) sizeof (struct trap));
    Sprintf(buf, template, hdrbuf, count, size);
    putstr(win, 0, buf);

    count = size = 0L;
    engr_stats("engravings, size %ld+text", hdrbuf, &count, &size);
    *total_count += count;
    *total_size += size;
    Sprintf(buf, template, hdrbuf, count, size);
    putstr(win, 0, buf);

#if NEXT_VERSION
    count = size = 0L;
    light_stats("light sources, size %ld", hdrbuf, &count, &size);
    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(buf, template, hdrbuf, count, size);
        putstr(win, 0, buf);
    }

    count = size = 0L;
    timer_stats("timers, size %ld", hdrbuf, &count, &size);
    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(buf, template, hdrbuf, count, size);
        putstr(win, 0, buf);
    }

    count = size = 0L;
    for (sd = level.damagelist; sd; sd = sd->next) {
        ++count;
        size += (long) sizeof *sd;
    }
    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(hdrbuf, "shop damage, size %ld",
                (long) sizeof (struct damage));
        Sprintf(buf, template, hdrbuf, count, size);
        putstr(win, 0, buf);
    }

    count = size = 0L;
    region_stats("regions, size %ld+%ld*rect+N", hdrbuf, &count, &size);
    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(buf, template, hdrbuf, count, size);
        putstr(win, 0, buf);
    }

    count = size = 0L;
    for (k = killer.next; k; k = k->next) {
        ++count;
        size += (long) sizeof *k;
    }
    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(hdrbuf, "delayed killer%s, size %ld",
                plur(count), (long) sizeof (struct kinfo));
        Sprintf(buf, template, hdrbuf, count, size);
        putstr(win, 0, buf);
    }

    count = size = 0L;
    for (bi = level.bonesinfo; bi; bi = bi->next) {
        ++count;
        size += (long) sizeof *bi;
    }
    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(hdrbuf, "bones history, size %ld",
                (long) sizeof (struct cemetery));
        Sprintf(buf, template, hdrbuf, count, size);
        putstr(win, 0, buf);
    }
#endif

    count = size = 0L;
    for (idx = 0; idx < NUM_OBJECTS; ++idx)
        if (objects[idx].oc_uname) {
            ++count;
            size += (long) (strlen(objects[idx].oc_uname) + 1);
        }
    if (count || size) {
        *total_count += count;
        *total_size += size;
        Strcpy(hdrbuf, "object type names, text");
        Sprintf(buf, template, hdrbuf, count, size);
        putstr(win, 0, buf);
    }
}

static int
wiz_mazewalkmap()
{
    winid win;
    int x, y;
    char row[COLNO+1];

    win = create_nhwindow(NHW_TEXT);

    for (y = 0; y < ROWNO; y++) {
        for (x = 0; x < COLNO; x++)
            row[x] = SpLev_Map[x][y] ? '1' : '.';
        if (y == u.uy)
            row[u.ux] = '@';
        row[x] = '\0';
        putstr(win, 0, row);
    }
    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
    return 0;
}


/*
 * Display memory usage of all monsters and objects on the level.
 */
static int
wiz_show_stats()
{
    char buf[BUFSZ];
    winid win;
    long total_obj_size, total_obj_count,
         total_mon_size, total_mon_count,
         total_ovr_size, total_ovr_count,
         total_misc_size, total_misc_count;

    win = create_nhwindow(NHW_TEXT);
    putstr(win, 0, "Current memory statistics:");

    total_obj_count = total_obj_size = 0L;
    putstr(win, 0, stats_hdr);
    Sprintf(buf, "  Objects, base size %ld", (long) sizeof (struct obj));
    putstr(win, 0, buf);
    obj_chain(win, "invent", invent, TRUE, &total_obj_count, &total_obj_size);
    obj_chain(win, "fobj", fobj, TRUE, &total_obj_count, &total_obj_size);
    obj_chain(win, "buried", level.buriedobjlist, FALSE,
              &total_obj_count, &total_obj_size);
    obj_chain(win, "migrating obj", migrating_objs, FALSE,
              &total_obj_count, &total_obj_size);
    obj_chain(win, "billobjs", billobjs, FALSE,
              &total_obj_count, &total_obj_size);
    mon_invent_chain(win, "minvent", fmon, &total_obj_count, &total_obj_size);
    mon_invent_chain(win, "migrating minvent", migrating_mons,
                     &total_obj_count, &total_obj_size);
    contained_stats(win, "contained", &total_obj_count, &total_obj_size);
    putstr(win, 0, stats_sep);
    Sprintf(buf, template, "  Obj total", total_obj_count, total_obj_size);
    putstr(win, 0, buf);

    total_mon_count = total_mon_size = 0L;
    putstr(win, 0, "");
    Sprintf(buf, "  Monsters, base size %ld", (long) sizeof (struct monst));
    putstr(win, 0, buf);
    mon_chain(win, "fmon", fmon, TRUE, &total_mon_count, &total_mon_size);
    mon_chain(win, "migrating", migrating_mons, FALSE,
              &total_mon_count, &total_mon_size);
    /* 'mydogs' is only valid during level change or end of game disclosure,
       but conceivably we've been called from within debugger at such time */
    if (mydogs) /* monsters accompanying hero */
        mon_chain(win, "mydogs", mydogs, FALSE,
                  &total_mon_count, &total_mon_size);
    putstr(win, 0, stats_sep);
    Sprintf(buf, template, "  Mon total", total_mon_count, total_mon_size);
    putstr(win, 0, buf);

    total_ovr_count = total_ovr_size = 0L;
    putstr(win, 0, "");
    putstr(win, 0, "  Overview");
    overview_stats(win, template, &total_ovr_count, &total_ovr_size);
    putstr(win, 0, stats_sep);
    Sprintf(buf, template, "  Over total", total_ovr_count, total_ovr_size);
    putstr(win, 0, buf);

    total_misc_count = total_misc_size = 0L;
    putstr(win, 0, "");
    putstr(win, 0, "  Miscellaneous");
    misc_stats(win, &total_misc_count, &total_misc_size);
    putstr(win, 0, stats_sep);
    Sprintf(buf, template, "  Misc total", total_misc_count, total_misc_size);
    putstr(win, 0, buf);

    putstr(win, 0, "");
    putstr(win, 0, stats_sep);
    Sprintf(buf, template, "  Grand total",
            (total_obj_count + total_mon_count
             + total_ovr_count + total_misc_count),
            (total_obj_size + total_mon_size
             + total_ovr_size + total_misc_size));
    putstr(win, 0, buf);

    display_nhwindow(win, FALSE);
    destroy_nhwindow(win);
    return 0;
}

void
trap_sanity_check()
{
    struct trap *trap = ftrap;
    while (trap) {
        if ((trap->tx < 0) || (trap->ty < 0)) {
            impossible("trap out of bound: ttyp: %d, %dx%d", trap->ttyp, trap->tx, trap->ty);
        }
        if (trap->ttyp == NO_TRAP) {
            impossible("trap typ NO_TRAP: ttyp: %d, %dx%d", trap->ttyp, trap->tx, trap->ty);
        }
        trap = trap->ntrap;
    }
}

void
sanity_check()
{
    obj_sanity_check();
    timer_sanity_check();
    mon_sanity_check();
    light_sources_sanity_check();
    bc_sanity_check();
    trap_sanity_check();
    sanity_check_shopkeepers();
}

#ifdef DEBUG_MIGRATING_MONS
static int
wiz_migrate_mons()
{
    int mcount = 0;
    char inbuf[BUFSZ];
    struct permonst *ptr;
    struct monst *mtmp;
    d_level tolevel;
    getlin("How many random monsters to migrate? [0]", inbuf);
    if (*inbuf == '\033') return 0;
    mcount = atoi(inbuf);
    if (mcount < 0 || mcount > (COLNO * ROWNO) || Is_botlevel(&u.uz))
        return 0;
    while (mcount > 0) {
        if (Is_stronghold(&u.uz))
            assign_level(&tolevel, &valley_level);
        else
            get_level(&tolevel, depth(&u.uz) + 1);
        ptr = rndmonst();
        mtmp = makemon(ptr, 0, 0, NO_MM_FLAGS);
        if (mtmp) migrate_to_level(mtmp, ledger_no(&tolevel),
                                   MIGR_RANDOM, (coord *)0);
        mcount--;
    }
    return 0;
}
#endif

struct {
    int nhkf;
    char key;
    const char *name;
} const spkeys_binds[] = {
    { NHKF_ESC,              '\033', (char *) 0 }, /* no binding */
    { NHKF_DOAGAIN,          DOAGAIN, "repeat" },
    { NHKF_REQMENU,          'm', "reqmenu" },
    { NHKF_RUN,              'G', "run" },
    { NHKF_RUN2,             '5', "run.numpad" },
    { NHKF_RUSH,             'g', "rush" },
    { NHKF_FIGHT,            'F', "fight" },
    { NHKF_FIGHT2,           '-', "fight.numpad" },
    { NHKF_NOPICKUP,         'm', "nopickup" },
    { NHKF_RUN_NOPICKUP,     'M', "run.nopickup" },
    { NHKF_DOINV,            '0', "doinv" },
    { NHKF_TRAVEL,           CMD_TRAVEL, (char *) 0 }, /* no binding */
#if NEXT_VERSION
    { NHKF_CLICKLOOK,        CMD_CLICKLOOK, (char *) 0 }, /* no binding */
#endif
    { NHKF_REDRAW,           C('r'), "redraw" },
    { NHKF_REDRAW2,          C('l'), "redraw.numpad" },
    { NHKF_GETDIR_SELF,      '.', "getdir.self" },
    { NHKF_GETDIR_SELF2,     's', "getdir.self2" },
    { NHKF_GETDIR_HELP,      '?', "getdir.help" },
    { NHKF_COUNT,            'n', "count" },
    { NHKF_GETPOS_SELF,      '@', "getpos.self" },
    { NHKF_GETPOS_PICK,      '.', "getpos.pick" },
    { NHKF_GETPOS_PICK_Q,    ',', "getpos.pick.quick" },
    { NHKF_GETPOS_PICK_O,    ';', "getpos.pick.once" },
    { NHKF_GETPOS_PICK_V,    ':', "getpos.pick.verbose" },
    { NHKF_GETPOS_SHOWVALID, '$', "getpos.valid" },
    { NHKF_GETPOS_AUTODESC,  '#', "getpos.autodescribe" },
    { NHKF_GETPOS_MON_NEXT,  'm', "getpos.mon.next" },
    { NHKF_GETPOS_MON_PREV,  'M', "getpos.mon.prev" },
    { NHKF_GETPOS_OBJ_NEXT,  'o', "getpos.obj.next" },
    { NHKF_GETPOS_OBJ_PREV,  'O', "getpos.obj.prev" },
    { NHKF_GETPOS_DUNGEON_FEATURE_NEXT, 'f', "getpos.dungeon_feature.next" },
    { NHKF_GETPOS_DUNGEON_FEATURE_PREV, 'F', "getpos.dungeon_feature.prev" },
    { NHKF_GETPOS_DOOR_NEXT, 'd', "getpos.door.next" },
    { NHKF_GETPOS_DOOR_PREV, 'D', "getpos.door.prev" },
    { NHKF_GETPOS_UNEX_NEXT, 'x', "getpos.unexplored.next" },
    { NHKF_GETPOS_UNEX_PREV, 'X', "getpos.unexplored.prev" },
    { NHKF_GETPOS_VALID_NEXT, 'z', "getpos.valid.next" },
    { NHKF_GETPOS_VALID_PREV, 'Z', "getpos.valid.prev" },
    { NHKF_GETPOS_INTERESTING_NEXT, 'a', "getpos.all.next" },
    { NHKF_GETPOS_INTERESTING_PREV, 'A', "getpos.all.prev" },
    { NHKF_GETPOS_HELP,      '?', "getpos.help" },
    { NHKF_GETPOS_LIMITVIEW, '"', "getpos.filter" },
    { NHKF_GETPOS_MOVESKIP,  '*', "getpos.moveskip" },
    { NHKF_GETPOS_MENU,      '!', "getpos.menu" }
};

boolean
bind_specialkey(key, command)
uchar key;
const char *command;
{
    int i;
    for (i = 0; i < SIZE(spkeys_binds); i++) {
        if (!spkeys_binds[i].name || strcmp(command, spkeys_binds[i].name)) {
            continue;
        }
        Cmd.spkeys[spkeys_binds[i].nhkf] = key;
        return TRUE;
    }
    return FALSE;
}

/* returns a one-byte character from the text (it may massacre the txt
 * buffer) */
char
txt2key(txt)
char *txt;
{
    txt = trimspaces(txt);
    if (!*txt) {
        return '\0';
    }

    /* simple character */
    if (!txt[1]) {
        return txt[0];
    }

    /* a few special entries */
    if (!strcmp(txt, "<enter>")) {
        return '\n';
    }
    if (!strcmp(txt, "<space>")) {
        return ' ';
    }
    if (!strcmp(txt, "<esc>")) {
        return '\033';
    }

    /* control and meta keys */
    switch (*txt) {
    case 'm': /* can be mx, Mx, m-x, M-x */
    case 'M':
        txt++;
        if (*txt == '-' && txt[1]) {
            txt++;
        }
        if (txt[1]) {
            return '\0';
        }
        return M(*txt);
    case 'c': /* can be cx, Cx, ^x, c-x, C-x, ^-x */
    case 'C':
    case '^':
        txt++;
        if (*txt == '-' && txt[1]) {
            txt++;
        }
        if (txt[1]) {
            return '\0';
        }
        return C(*txt);
    }

    /* ascii codes: must be three-digit decimal */
    if (*txt >= '0' && *txt <= '9') {
        uchar key = 0;
        int i;

        for (i = 0; i < 3; i++) {
            if (txt[i] < '0' || txt[i] > '9') {
                return '\0';
            }
            key = 10 * key + txt[i] - '0';
        }
        return key;
    }

    return '\0';
}

/* returns the text for a one-byte encoding;
 * must be shorter than a tab for proper formatting */
char *
key2txt(c, txt)
uchar c;
char *txt; /* sufficiently long buffer */
{
    /* should probably switch to "SPC", "ESC", "RET"
       since nethack's documentation uses ESC for <escape> */
    if (c == ' ') {
        Sprintf(txt, "<space>");
    } else if (c == '\033') {
        Sprintf(txt, "<esc>");
    } else if (c == '\n') {
        Sprintf(txt, "<enter>");
    } else if (c == '\177') {
        Sprintf(txt, "<del>"); /* "<delete>" won't fit */
    } else {
        Strcpy(txt, visctrl((char) c));
    }
    return txt;
}

void
parseautocomplete(autocomplete, condition)
char *autocomplete;
boolean condition;
{
    struct ext_func_tab *efp;
    register char *autoc;

    /* break off first autocomplete from the rest; parse the rest */
    if ((autoc = index(autocomplete, ',')) != 0 ||
        (autoc = index(autocomplete, ':')) != 0) {
        *autoc++ = '\0';
        parseautocomplete(autoc, condition);
    }

    /* strip leading and trailing white space */
    autocomplete = trimspaces(autocomplete);

    if (!*autocomplete) {
        return;
    }

    /* take off negation */
    if (*autocomplete == '!') {
        /* unlike most options, a leading "no" might actually be a part of
         * the extended command.  Thus you have to use ! */
        autocomplete++;
        autocomplete = trimspaces(autocomplete);
        condition = !condition;
    }

    /* find and modify the extended command */
    for (efp = extcmdlist; efp->ef_txt; efp++) {
        if (!strcmp(autocomplete, efp->ef_txt)) {
            if (condition) {
                efp->flags |= AUTOCOMPLETE;
            } else {
                efp->flags &= ~AUTOCOMPLETE;
            }
            return;
        }
    }

    /* not a real extended command */
    raw_printf("Bad autocomplete: invalid extended command '%s'.", autocomplete);
    wait_synch();
}

/* called at startup and after number_pad is twiddled */
void
reset_commands(initial)
boolean initial;
{
    static const char sdir[] = "hykulnjb><",
                      sdir_swap_yz[] = "hzkulnjb><",
                      ndir[] = "47896321><",
                      ndir_phone_layout[] = "41236987><";
    static const int ylist[] = {
        'y', 'Y', C('y'), M('y'), M('Y'), M(C('y'))
    };
    static struct ext_func_tab *back_dir_cmd[8];
    const struct ext_func_tab *cmdtmp;
    boolean flagtemp;
    int c, i, updated = 0;
    static boolean backed_dir_cmd = FALSE;

    if (initial) {
        updated = 1;
        Cmd.num_pad = FALSE;
        Cmd.pcHack_compat = Cmd.phone_layout = Cmd.swap_yz = FALSE;
        for (i = 0; i < SIZE(spkeys_binds); i++) {
            Cmd.spkeys[spkeys_binds[i].nhkf] = spkeys_binds[i].key;
        }
        commands_init();
    } else {

        if (backed_dir_cmd) {
            for (i = 0; i < 8; i++) {
                Cmd.commands[(uchar) Cmd.dirchars[i]] = back_dir_cmd[i];
            }
        }

        /* basic num_pad */
        flagtemp = iflags.num_pad;
        if (flagtemp != Cmd.num_pad) {
            Cmd.num_pad = flagtemp;
            ++updated;
        }
        /* swap_yz mode (only applicable for !num_pad); intended for
           QWERTZ keyboard used in Central Europe, particularly Germany */
        flagtemp = (iflags.num_pad_mode & 1) ? !Cmd.num_pad : FALSE;
        if (flagtemp != Cmd.swap_yz) {
            Cmd.swap_yz = flagtemp;
            ++updated;
            /* Cmd.swap_yz has been toggled;
               perform the swap (or reverse previous one) */
            for (i = 0; i < SIZE(ylist); i++) {
                c = ylist[i] & 0xff;
                cmdtmp = Cmd.commands[c];              /* tmp = [y] */
                Cmd.commands[c] = Cmd.commands[c + 1]; /* [y] = [z] */
                Cmd.commands[c + 1] = cmdtmp;          /* [z] = tmp */
            }
        }
        /* MSDOS compatibility mode (only applicable for num_pad) */
        flagtemp = (iflags.num_pad_mode & 1) ? Cmd.num_pad : FALSE;
        if (flagtemp != Cmd.pcHack_compat) {
            Cmd.pcHack_compat = flagtemp;
            ++updated;
            /* pcHack_compat has been toggled */
            c = M('5') & 0xff;
            cmdtmp = Cmd.commands['5'];
            Cmd.commands['5'] = Cmd.commands[c];
            Cmd.commands[c] = cmdtmp;
            c = M('0') & 0xff;
            Cmd.commands[c] = Cmd.pcHack_compat ? Cmd.commands['I'] : 0;
        }
        /* phone keypad layout (only applicable for num_pad) */
        flagtemp = (iflags.num_pad_mode & 2) ? Cmd.num_pad : FALSE;
        if (flagtemp != Cmd.phone_layout) {
            Cmd.phone_layout = flagtemp;
            ++updated;
            /* phone_layout has been toggled */
            for (i = 0; i < 3; i++) {
                c = '1' + i;             /* 1,2,3 <-> 7,8,9 */
                cmdtmp = Cmd.commands[c];              /* tmp = [1] */
                Cmd.commands[c] = Cmd.commands[c + 6]; /* [1] = [7] */
                Cmd.commands[c + 6] = cmdtmp;          /* [7] = tmp */
                c = (M('1') & 0xff) + i; /* M-1,M-2,M-3 <-> M-7,M-8,M-9 */
                cmdtmp = Cmd.commands[c];              /* tmp = [M-1] */
                Cmd.commands[c] = Cmd.commands[c + 6]; /* [M-1] = [M-7] */
                Cmd.commands[c + 6] = cmdtmp;          /* [M-7] = tmp */
            }
        }
    } /*?initial*/

    if (updated) {
        Cmd.serialno++;
    }
    Cmd.dirchars = !Cmd.num_pad
                       ? (!Cmd.swap_yz      ? sdir : sdir_swap_yz)
                       : (!Cmd.phone_layout ? ndir : ndir_phone_layout);
    Cmd.alphadirchars = !Cmd.num_pad ? Cmd.dirchars : sdir;

    Cmd.move_W = Cmd.dirchars[0];
    Cmd.move_NW = Cmd.dirchars[1];
    Cmd.move_N = Cmd.dirchars[2];
    Cmd.move_NE = Cmd.dirchars[3];
    Cmd.move_E = Cmd.dirchars[4];
    Cmd.move_SE = Cmd.dirchars[5];
    Cmd.move_S = Cmd.dirchars[6];
    Cmd.move_SW = Cmd.dirchars[7];

    if (!initial) {
        for (i = 0; i < 8; i++) {
            back_dir_cmd[i] = (struct ext_func_tab *) Cmd.commands[(uchar) Cmd.dirchars[i]];
            Cmd.commands[(uchar) Cmd.dirchars[i]] = (struct ext_func_tab *) 0;
        }
        backed_dir_cmd = TRUE;
        for (i = 0; i < 8; i++) {
            (void) bind_key(Cmd.dirchars[i], "nothing");
        }
    }
}

/* non-movement commands which accept 'm' prefix to request menu operation */
static boolean
accept_menu_prefix(cmd_func)
int NDECL((*cmd_func));
{
    if (cmd_func == dopickup || cmd_func == dotip ||
        /* eat, #offer, and apply tinning-kit all use floorfood() to pick
           an item on floor or in invent; 'm' skips picking from floor
           (ie, inventory only) rather than request use of menu operation */
         cmd_func == doeat || cmd_func == dosacrifice || cmd_func == doapply ||
        /* 'm' for removing saddle from adjacent monster without checking
           for containers at <u.ux,u.uy> */
         cmd_func == doloot ||
        /* travel: pop up a menu of interesting targets in view */
         cmd_func == dotravel ||
        /* wizard mode ^V and ^T */
         cmd_func == wiz_level_tele || cmd_func == dotelecmd ||
        /* 'm' prefix allowed for some extended commands */
         cmd_func == doextcmd || cmd_func == doextlist) {
        return TRUE;
    }
    return FALSE;
}

char
randomkey()
{
    static unsigned i = 0;
    char c;

    switch (rn2(16)) {
    default:
        c = '\033';
        break;
    case 0:
        c = '\n';
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        c = (char)rn1('~'-' '+1, ' ');
        break;
    case 5:
        c = (char) (rn2(2) ? '\t' : ' ');
        break;
    case 6:
        c = (char) rn1('z'-'a'+1, 'a');
        break;
    case 7:
        c = (char) rn1('Z'-'A'+1, 'A');
        break;
#if 0
    case 8:
        c = extcmdlist[i++ % SIZE(extcmdlist)].key;
        break;
#endif
    case 8:
    case 9:
        c = '#';
        break;
    case 10:
    case 11:
    case 12:
        c = (iflags.num_pad ? ndir[rn2(8)] : sdir[rn2(8)]);
        if (!rn2(7))
            c = !iflags.num_pad ? (!rn2(3) ? C(c) : (c + 'A' - 'a')) : M(c);
        break;
    case 13:
        c = (char) rn1('9'-'0'+1, '0');
        break;
    case 14:
        c = (char) rn2(iflags.wc_eight_bit_input ? 256 : 128);
        break;
    }

    /* increase chances of going down and changing branches */
    boolean stairs_down = ((u.ux == xdnstair && u.uy == ydnstair) ||
                           (u.ux == sstairs.sx && u.uy == sstairs.sy));
    if (stairs_down && rnf(1,5)) {
        c = '>';
    }

    return c;
}

void
random_response(buf, sz)
char *buf;
int sz;
{
    char c;
    int count = 0;

    for (;;) {
        c = randomkey();
        if (c == '\n')
            break;
        if (c == '\033') {
            count = 0;
            break;
        }
        if (count < sz - 1)
            buf[count++] = c;
    }
    buf[count] = '\0';
}

int
rnd_extcmd_idx()
{
#ifdef NEXT_VERSION
    return rn2(extcmdlist_length + 1) - 1;
#else
    return -1;
#endif
}

#define unctrl(c)   ((c) <= C('z') ? (0x60 | (c)) : (c))
#define unmeta(c)   (0x7f & (c))

int
ch2spkeys(char c, int start, int end)
{
    int i;

    for (i = start; i <= end; i++) {
        if (Cmd.spkeys[i] == c) {
            return i;
        }
    }
    return NHKF_ESC;
}

void
rhack(cmd)
register char *cmd;
{
    int spkey = -1;
    boolean do_walk, do_rush, prefix_seen, bad_command,
            firsttime = (cmd == 0);

    iflags.menu_requested = FALSE;
    if (firsttime) {
        flags.nopick = 0;
        cmd = parse();
    }
    if (*cmd == Cmd.spkeys[NHKF_ESC]) {
        flags.move = FALSE;
        return;
    }
#ifdef REDO
    if (*cmd == DOAGAIN && !in_doagain && saveq[0]) {
        in_doagain = TRUE;
        stail = 0;
        rhack((char *)0);   /* read and execute command */
        in_doagain = FALSE;
        return;
    }
    /* Special case of *cmd == ' ' handled better below */
    if(!*cmd || *cmd == (char)0377)
#else
    if(!*cmd || *cmd == (char)0377 || (!iflags.rest_on_space && *cmd == ' '))
#endif
    {
        nhbell();
        flags.move = FALSE;
        return;     /* probably we just had an interrupt */
    }
    /* handle most movement commands */
    do_walk = do_rush = prefix_seen = FALSE;
    flags.travel = iflags.travel1 = 0;
    spkey = ch2spkeys(*cmd, NHKF_RUN, NHKF_CLICKLOOK);

    switch (spkey) {
    case NHKF_RUSH:
        if (movecmd(cmd[1])) {
            flags.run = 2;
            do_rush = TRUE;
        } else {
            prefix_seen = TRUE;
        }
        break;

    case NHKF_RUN2:
        if (!Cmd.num_pad) {
            break;
        }
        /* fall-through */

    case NHKF_RUN:
        if (movecmd(lowc(cmd[1]))) {
            flags.run = 3;
            do_rush = TRUE;
        } else {
            prefix_seen = TRUE;
        }
        break;

    case NHKF_FIGHT2:
        if (!Cmd.num_pad) {
            break;
        }
        /* fall-through */

    /* Effects of movement commands and invisible monsters:
     * m: always move onto space (even if 'I' remembered)
     * F: always attack space (even if 'I' not remembered)
     * normal movement: attack if 'I', move otherwise
     */
    case NHKF_FIGHT:
        if (movecmd(cmd[1])) {
            flags.forcefight = 1;
            do_walk = TRUE;
        } else {
            prefix_seen = TRUE;
        }
        break;

    case NHKF_NOPICKUP:
        if (movecmd(cmd[1]) || u.dz) {
            flags.run = 0;
            flags.nopick = 1;
            if (!u.dz) {
                do_walk = TRUE;
            } else {
                cmd[0] = cmd[1];   /* "m<" or "m>" */
            }
        } else {
            prefix_seen = TRUE;
        }
        break;

    case NHKF_RUN_NOPICKUP:
        if (movecmd(lowc(cmd[1]))) {
            flags.run = 1;
            flags.nopick = 1;
            do_rush = TRUE;
        } else {
            prefix_seen = TRUE;
        }
        break;

    case NHKF_DOINV:
        if (!Cmd.num_pad) {
            break;
        }
        (void)ddoinv();     /* a convenience borrowed from the PC */
        flags.move = FALSE;
        multi = 0;
        return;

    case NHKF_CLICKLOOK:
        if (iflags.clicklook) {
            flags.move = FALSE;
            do_look(2, &clicklook_cc);
        }
        return;

    case NHKF_TRAVEL:
        if (iflags.travelcmd) {
            flags.travel = 1;
            iflags.travel1 = 1;
            flags.run = 8;
            flags.nopick = !iflags.autoexplore;
            do_rush = TRUE;
            break;
        }
        /* fall through */

    default:
        if (movecmd(*cmd)) {
            /* ordinary movement */
            flags.run = 0;  /* only matters here if it was 8 */
            do_walk = TRUE;
        } else if (movecmd(Cmd.num_pad ? unmeta(*cmd) : lowc(*cmd))) {
            flags.run = 1;
            do_rush = TRUE;
        } else if (movecmd(unctrl(*cmd))) {
            flags.run = 3;
            do_rush = TRUE;
        }
        break;
    }

    /* some special prefix handling */
    /* overload 'm' prefix for ',' to mean "request a menu" */
    if (prefix_seen && cmd[0] == Cmd.spkeys[NHKF_REQMENU]) {
        /* (for func_tab cast, see below) */
        const struct ext_func_tab *ft = Cmd.commands[cmd[1] & 0xff];
        int NDECL((*func)) = ft ? ((struct ext_func_tab *) ft)->ef_funct : 0;

        if (func && accept_menu_prefix(func)) {
            iflags.menu_requested = TRUE;
            ++cmd;
        }
    }

    if (do_walk) {
        if (multi) flags.mv = TRUE;
        check_tutorial_command('m');
        domove();
        flags.forcefight = 0;
        return;
    } else if (do_rush) {
        if (firsttime) {
            if (!multi) multi = max(COLNO, ROWNO);
            u.last_str_turn = 0;
        }
        flags.mv = TRUE;
        check_tutorial_command('G');
        domove();
        return;
    } else if (prefix_seen && cmd[1] == Cmd.spkeys[NHKF_ESC]) {
        /* <prefix><escape> */
        /* don't report "unknown command" for change of heart... */
        bad_command = FALSE;
    } else if (*cmd == ' ' && !iflags.rest_on_space) {
        bad_command = TRUE;     /* skip cmdlist[] loop */

        /* handle all other commands */
    } else {
        const struct ext_func_tab *tlist;
        int res, NDECL((*func));

        /* current - use *cmd to directly index cmdlist array */
        if ((tlist = Cmd.commands[*cmd & 0xff]) != 0) {
            check_tutorial_command(*cmd & 0xff);

            if (!wizard && (tlist->flags & WIZMODECMD)) {
                You_cant("do that!");
                res = 0;
            } else if (u.uburied && !(tlist->flags & IFBURIED)) {
                You_cant("do that while you are buried!");
                res = 0;
            } else {
                /* we discard 'const' because some compilers seem to have
                   trouble with the pointer passed to set_occupation() */
                func = ((struct ext_func_tab *) tlist)->ef_funct;
                if (tlist->f_text && !occupation && multi)
                    set_occupation(func, tlist->f_text, multi);
                /* remember pressed character */
                last_cmd_char = *cmd;
                res = (*func)();    /* perform the command */
            }
            if (!res) {
                flags.move = FALSE;
                multi = 0;
            }
            return;
        }
        /* if we reach here, cmd wasn't found in cmdlist[] */
        bad_command = TRUE;
    }

    if (bad_command) {
        char expcmd[10];
        register char *cp = expcmd;

        while (*cmd && (int)(cp - expcmd) < (int)(sizeof expcmd - 3)) {
            if (*cmd >= 040 && *cmd < 0177) {
                *cp++ = *cmd++;
            } else if (*cmd & 0200) {
                *cp++ = 'M';
                *cp++ = '-';
                *cp++ = *cmd++ &= ~0200;
            } else {
                *cp++ = '^';
                *cp++ = *cmd++ ^ 0100;
            }
        }
        *cp = '\0';
        if (!prefix_seen || !iflags.cmdassist ||
            !help_dir(0, spkey, "Invalid direction key!"))
            Norep("Unknown command '%s'.", expcmd);
    }
    /* didn't move */
    flags.move = FALSE;
    multi = 0;
    return;
}

/* convert an x,y pair into a direction code */
int
xytod(x, y)
schar x, y;
{
    register int dd;

    for(dd = 0; dd < 8; dd++)
        if(x == xdir[dd] && y == ydir[dd]) return dd;

    return -1;
}

/* convert a direction code into an x,y pair */
void
dtoxy(cc, dd)
coord *cc;
register int dd;
{
    cc->x = xdir[dd];
    cc->y = ydir[dd];
    return;
}

/* also sets u.dz, but returns false for <> */
int
movecmd(sym)
char sym;
{
    // TODO
    const char *dp;
    const char *sdp;
    if (iflags.num_pad) sdp = ndir; else sdp = sdir; /* DICE workaround */

    u.dz = 0;
    dp = index(sdp, sym);
    if (!dp || !*dp) {
        return 0;
    }
    u.dx = xdir[dp-sdp];
    u.dy = ydir[dp-sdp];
    u.dz = zdir[dp-sdp];
    if (u.dx && u.dy && u.umonnum == PM_GRID_BUG) {
        u.dx = u.dy = 0;
        return 0;
    }
    return !u.dz;
}

/* grid bug handling which used to be in movecmd() */
int
dxdy_moveok()
{
    if (u.dx && u.dy && NODIAG(u.umonnum))
        u.dx = u.dy = 0;
    return u.dx || u.dy;
}

/* decide whether a character (user input keystroke) requests screen repaint */
boolean
redraw_cmd(c)
char c;
{
    return (c == Cmd.spkeys[NHKF_REDRAW] ||
           (Cmd.num_pad && c == Cmd.spkeys[NHKF_REDRAW2]));
}

boolean
prefix_cmd(c)
char c;
{
    return (c == Cmd.spkeys[NHKF_RUSH] ||
            c == Cmd.spkeys[NHKF_RUN] ||
            c == Cmd.spkeys[NHKF_NOPICKUP] ||
            c == Cmd.spkeys[NHKF_RUN_NOPICKUP] ||
            c == Cmd.spkeys[NHKF_FIGHT] ||
            (Cmd.num_pad && (c == Cmd.spkeys[NHKF_RUN2] ||
                             c == Cmd.spkeys[NHKF_FIGHT2])));
}

/*
 * uses getdir() but unlike getdir() it specifically
 * produces coordinates using the direction from getdir()
 * and verifies that those coordinates are ok.
 *
 * If the call to getdir() returns 0, Never_mind is displayed.
 * If the resulting coordinates are not okay, emsg is displayed.
 *
 * Returns non-zero if coordinates in cc are valid.
 */
int
get_adjacent_loc(prompt, emsg, x, y, cc)
const char *prompt, *emsg;
xchar x, y;
coord *cc;
{
    xchar new_x, new_y;
    if (!getdir(prompt)) {
        pline("%s", Never_mind);
        return 0;
    }
    new_x = x + u.dx;
    new_y = y + u.dy;
    if (cc && isok(new_x, new_y)) {
        cc->x = new_x;
        cc->y = new_y;
    } else {
        if (emsg) pline("%s", emsg);
        return 0;
    }
    return 1;
}

int
getdir(s)
const char *s;
{
    char dirsym;
    int is_mov;
    /* saved direction of the previous call of getdir() */
    static char saved_dirsym = '\0';

 retry:
#ifdef REDO
    if(in_doagain || *readchar_queue)
        dirsym = readchar();
    else
#endif
    dirsym = yn_function ((s && *s != '^') ? s : "In what direction?",
                          (char *)0, '\0');
    /* remove the prompt string so caller won't have to */
    clear_nhwindow(WIN_MESSAGE);
    if (redraw_cmd(dirsym)) { /* ^R */
        docrt();              /* redraw */
        goto retry;
    }
#ifdef REDO
    savech(dirsym);
#endif

    if (dirsym == last_cmd_char) {
        /* in here dirsym is not representing a direction
         * but the same sym used before for calling the
         * current cmd */
        movecmd(saved_dirsym);
        dirsym = saved_dirsym;
    } else if (dirsym == Cmd.spkeys[NHKF_GETDIR_SELF] ||
               dirsym == Cmd.spkeys[NHKF_GETDIR_SELF2]) {
        u.dx = u.dy = u.dz = 0;
    } else if (!(is_mov = movecmd(dirsym)) && !u.dz) {
        boolean did_help = FALSE;

        if (!index(quitchars, dirsym)) {
            boolean help_requested = (dirsym == Cmd.spkeys[NHKF_GETDIR_HELP]);
            if (help_requested || iflags.cmdassist) {
                did_help = help_dir((s && *s == '^') ? dirsym : 0,
                                    NHKF_ESC,
                                    help_requested ? (const char *) 0 : "Invalid direction key!");
                if (help_requested) {
                    goto retry;
                }
            }
            if (!did_help) pline("What a strange direction!");
        }
        return 0;
    } else if (is_mov && !dxdy_moveok()) {
        You_cant("orient yourself that direction.");
        return 0;
    }
    saved_dirsym = dirsym;
    if (!u.dz && (Stunned || (Confusion && !rn2(5)))) {
        confdir();
    }
    return 1;
}

static void
show_direction_keys(win, centerchar, nodiag)
winid win; /* should specify a window which is using a fixed-width font... */
char centerchar; /* '.' or '@' or ' ' */
boolean nodiag;
{
    char buf[BUFSZ];

    if (!centerchar) {
        centerchar = ' ';
    }

    if (nodiag) {
        Sprintf(buf, "             %c   ", Cmd.move_N);
        putstr(win, 0, buf);
        putstr(win, 0, "             |   ");
        Sprintf(buf, "          %c- %c -%c", Cmd.move_W, centerchar, Cmd.move_E);
        putstr(win, 0, buf);
        putstr(win, 0, "             |   ");
        Sprintf(buf, "             %c   ", Cmd.move_S);
        putstr(win, 0, buf);
    } else {
        Sprintf(buf, "          %c  %c  %c", Cmd.move_NW, Cmd.move_N, Cmd.move_NE);
        putstr(win, 0, buf);
        putstr(win, 0, "           \\ | / ");
        Sprintf(buf, "          %c- %c -%c", Cmd.move_W, centerchar, Cmd.move_E);
        putstr(win, 0, buf);
        putstr(win, 0, "           / | \\ ");
        Sprintf(buf, "          %c  %c  %c", Cmd.move_SW, Cmd.move_S, Cmd.move_SE);
        putstr(win, 0, buf);
    };
}

/* explain choices if player has asked for getdir() help or has given
   an invalid direction after a prefix key ('F', 'g', 'm', &c), which
   might be bogus but could be up, down, or self when not applicable */
STATIC_OVL boolean
help_dir(sym, spkey, msg)
char sym;
int spkey; /* NHKF_ code for prefix key, if one was used, or for ESC */
const char *msg;
{
    char ctrl;
    winid win;
    static const char wiz_only_list[] = "EFGIOVW";
    char buf[BUFSZ], buf2[BUFSZ], *explain;
    const char *dothat, *how;
    boolean prefixhandling, viawindow;

    /* NHKF_ESC indicates that player asked for help at getdir prompt */
    viawindow = (spkey == NHKF_ESC || iflags.cmdassist);
    prefixhandling = (spkey != NHKF_ESC);
    /*
     * Handling for prefix keys that don't want special directions.
     * Delivered via pline if 'cmdassist' is off, or instead of the
     * general message if it's on.
     */
    dothat = "do that";
    how = " at"; /* for "<action> at yourself"; not used for up/down */
    switch (spkey) {
    case NHKF_NOPICKUP:
        dothat = "move";
        break;
    case NHKF_RUSH:
        dothat = "rush";
        break;
    case NHKF_RUN2:
        if (!Cmd.num_pad) {
            break;
        }
        /* fall-through */
    case NHKF_RUN:
    case NHKF_RUN_NOPICKUP:
        dothat = "run";
        break;
    case NHKF_FIGHT2:
        if (!Cmd.num_pad) {
            break;
        }
        /* fall-through */
    case NHKF_FIGHT:
        dothat = "fight";
        how = ""; /* avoid "fight at yourself" */
        break;
    default:
        prefixhandling = FALSE;
        break;
    }

    buf[0] = '\0';
    if (prefixhandling &&
        /* for movement prefix followed by '.' or (numpad && 's') to mean 'self';
           note: '-' for hands (inventory form of 'self') is not handled here */
        (sym == Cmd.spkeys[NHKF_GETDIR_SELF] ||
         (Cmd.num_pad && sym == Cmd.spkeys[NHKF_GETDIR_SELF2]))) {
        Sprintf(buf, "You can't %s%s yourself.", dothat, how);
    } else if (prefixhandling && (sym == '<' || sym == '>')) {
        /* for movement prefix followed by up or down */
        Sprintf(buf, "You can't %s %s.", dothat,
                /* was "upwards" and "downwards", but they're considered
                   to be variants of canonical "upward" and "downward" */
                (sym == '<') ? "upward" : "downward");
    }
    /* if '!cmdassist', display via pline() and we're done (note: asking
       for help at getdir() prompt forces cmdassist for this operation) */
    if (!viawindow) {
        if (prefixhandling) {
            if (!*buf) {
                Sprintf(buf, "Invalid direction for '%s' prefix.", visctrl(Cmd.spkeys[spkey]));
            }
            pline("%s", buf);
            return TRUE;
        }
        /* when 'cmdassist' is off and caller doesn't insist, do nothing */
        return FALSE;
    }

    win = create_nhwindow(NHW_TEXT);
    if (!win) {
        return FALSE;
    }
    if (*buf) {
        /* show bad-prefix message instead of general invalid-direction one */
        putstr(win, 0, buf);
        putstr(win, 0, "");
    } else if (msg) {
        Sprintf(buf, "cmdassist: %s", msg);
        putstr(win, 0, buf);
        putstr(win, 0, "");
    }
    if (!prefixhandling && (letter(sym) || sym == '[')) {
        /* '[': old 'cmdhelp' showed ESC as ^[ */
        sym = highc(sym); /* @A-Z[ (note: letter() accepts '@') */
        ctrl = (sym - 'A') + 1; /* 0-27 (note: 28-31 aren't applicable) */
        if ((explain = dowhatdoes_core(ctrl, buf2)) &&
             (!index(wiz_only_list, sym) || wizard)) {
            Sprintf(buf, "Are you trying to use ^%c%s?", sym,
                    index(wiz_only_list, sym) ? "" :
                    " as specified in the Guidebook");
            putstr(win, 0, buf);
            putstr(win, 0, "");
            putstr(win, 0, explain);
            putstr(win, 0, "");
            putstr(win, 0,
                  "To use that command, hold down the <Ctrl> key as a shift");
            Sprintf(buf, "and press the <%c> key.", sym);
            putstr(win, 0, buf);
            putstr(win, 0, "");
        }
    }

    Sprintf(buf, "Valid direction keys%s%s%s are:",
            prefixhandling ? " to " : "",
            prefixhandling ? dothat : "",
            NODIAG(u.umonnum) ? " in your current form" : "");
    putstr(win, 0, buf);
    show_direction_keys(win, !prefixhandling ? '.' : ' ', NODIAG(u.umonnum));

    if (!prefixhandling || spkey == NHKF_NOPICKUP) {
        /* NOPICKUP: unlike the other prefix keys, 'm' allows up/down for
           stair traversal; we won't get here when "m<" or "m>" has been
           given but we include up and down for 'm'+invalid_direction;
           self is excluded as a viable direction for every prefix */
        putstr(win, 0, "");
        putstr(win, 0, "          <  up");
        putstr(win, 0, "          >  down");
        if (!prefixhandling) {
            int selfi = Cmd.num_pad ? NHKF_GETDIR_SELF2 : NHKF_GETDIR_SELF;

            Sprintf(buf,   "       %4s  direct at yourself", visctrl(Cmd.spkeys[selfi]));
            putstr(win, 0, buf);
        }
    }

    if (msg) {
        /* non-null msg means that this wasn't an explicit user request */
        putstr(win, 0, "");
        putstr(win, 0, "(Suppress this message with !cmdassist in config file.)");
    }
    display_nhwindow(win, FALSE);
    destroy_nhwindow(win);

    return TRUE;
}

void
confdir()
{
    int x = NODIAG(u.umonnum) ? 2 * rn2(4) : rn2(8);

    u.dx = xdir[x];
    u.dy = ydir[x];
    return;
}

const char *
directionname(dir)
int dir;
{
    static NEARDATA const char *const dirnames[] = {
        "west",      "northwest", "north",     "northeast", "east",
        "southeast", "south",     "southwest", "down",      "up",
    };

    if (dir < 0 || dir >= SIZE(dirnames)) {
        return "invalid";
    }
    return dirnames[dir];
}

int
isok(x, y)
register int x, y;
{
    /* x corresponds to curx, so x==1 is the first column. Ach. %% */
    return x >= 1 && x <= COLNO-1 && y >= 0 && y <= ROWNO-1;
}

/* #herecmdmenu command */
static int
doherecmdmenu()
{
    char ch = here_cmd_menu(TRUE);

    return ch ? 1 : 0;
}

/* #therecmdmenu command, a way to test there_cmd_menu without mouse */
static int
dotherecmdmenu()
{
    char ch;

    if (!getdir((const char *) 0) || !isok(u.ux + u.dx, u.uy + u.dy)) {
        return 0;
    }

    if (u.dx || u.dy) {
        ch = there_cmd_menu(TRUE, u.ux + u.dx, u.uy + u.dy);
    } else {
        ch = here_cmd_menu(TRUE);
    }

    return ch ? 1 : 0;
}

static void
add_herecmd_menuitem(win, func, text)
winid win;
int NDECL((*func));
const char *text;
{
    char ch;
    anything any;

    if ((ch = cmd_from_func(func)) != '\0') {
        any = zeroany;
        any.a_nfunc = func;
        add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, text, MENU_UNSELECTED);
    }
}

static char
there_cmd_menu(doit, x, y)
boolean doit;
int x, y;
{
    winid win;
    char ch;
    char buf[BUFSZ];
    schar typ = levl[x][y].typ;
    int npick, K = 0;
    menu_item *picks = (menu_item *) 0;
    struct trap *ttmp;
    struct monst *mtmp;

    win = create_nhwindow(NHW_MENU);
    start_menu(win);

    if (IS_DOOR(typ)) {
        boolean key_or_pick, card;
        int dm = levl[x][y].doormask;

        if ((dm & (D_CLOSED | D_LOCKED))) {
            add_herecmd_menuitem(win, doopen, "Open the door"), ++K;
            /* unfortunately there's no lknown flag for doors to
               remember the locked/unlocked state */
            key_or_pick = (carrying(SKELETON_KEY) || carrying(LOCK_PICK));
            card = (carrying(CREDIT_CARD) != 0);
            if (key_or_pick || card) {
                Sprintf(buf, "%sunlock the door", key_or_pick ? "lock or " : "");
                add_herecmd_menuitem(win, doapply, upstart(buf)), ++K;
            }
            /* unfortunately there's no tknown flag for doors (or chests)
               to remember whether a trap had been found */
            add_herecmd_menuitem(win, dountrap, "Search the door for a trap"), ++K;
            /* [what about #force?] */
            add_herecmd_menuitem(win, dokick, "Kick the door"), ++K;
        } else if ((dm & D_ISOPEN)) {
            add_herecmd_menuitem(win, doclose, "Close the door"), ++K;
        }
    }

    if ((ttmp = t_at(x, y)) != 0 && ttmp->tseen) {
        add_herecmd_menuitem(win, doidtrap, "Examine trap"), ++K;
        if (ttmp->ttyp != VIBRATING_SQUARE) {
            add_herecmd_menuitem(win, dountrap, "Attempt to disarm trap"), ++K;
        }
    }

    mtmp = m_at(x, y);
    if (mtmp && !canspotmon(mtmp)) {
        mtmp = 0;
    }
    if (mtmp && which_armor(mtmp, W_SADDLE)) {
        char *mnam = x_monnam(mtmp, ARTICLE_THE, (char *) 0, SUPPRESS_SADDLE, FALSE);

        if (!u.usteed) {
            Sprintf(buf, "Ride %s", mnam);
            add_herecmd_menuitem(win, doride, buf), ++K;
        }
        Sprintf(buf, "Remove saddle from %s", mnam);
        add_herecmd_menuitem(win, doloot, buf), ++K;
    }
    if (mtmp && can_saddle(mtmp) && !which_armor(mtmp, W_SADDLE) && carrying(SADDLE)) {
        Sprintf(buf, "Put saddle on %s", mon_nam(mtmp)), ++K;
        add_herecmd_menuitem(win, doapply, buf);
    }
#if 0
    if (mtmp || glyph_is_invisible(glyph_at(x, y))) {
        /* "Attack %s", mtmp ? mon_nam(mtmp) : "unseen creature" */
    } else {
        /* "Move %s", direction */
    }
#endif

    if (K) {
        end_menu(win, "What do you want to do?");
        npick = select_menu(win, PICK_ONE, &picks);
    } else {
        pline("No applicable actions.");
        npick = 0;
    }
    destroy_nhwindow(win);
    ch = '\0';
    if (npick > 0) {
        int NDECL((*func)) = picks->item.a_nfunc;
        free(picks);

        if (doit) {
            int ret = (*func)();

            ch = (char) ret;
        } else {
            ch = cmd_from_func(func);
        }
    }
    return ch;
}

static char
here_cmd_menu(doit)
boolean doit;
{
    winid win;
    char ch;
    char buf[BUFSZ];
    schar typ = levl[u.ux][u.uy].typ;
    int npick;
    menu_item *picks = (menu_item *) 0;

    win = create_nhwindow(NHW_MENU);
    start_menu(win);

    if (IS_FOUNTAIN(typ) || IS_SINK(typ)) {
        Sprintf(buf, "Drink from the %s",
                defsyms[IS_FOUNTAIN(typ) ? S_fountain : S_sink].explanation);
        add_herecmd_menuitem(win, dodrink, buf);
    }
    if (IS_FOUNTAIN(typ)) {
        add_herecmd_menuitem(win, dodip, "Dip something into the fountain");
    }
    if (IS_THRONE(typ)) {
        add_herecmd_menuitem(win, dosit, "Sit on the throne");
    }

    if ((u.ux == xupstair && u.uy == yupstair) ||
        (u.ux == sstairs.sx && u.uy == sstairs.sy && sstairs.up) ||
        (u.ux == xupladder && u.uy == yupladder)) {
        Sprintf(buf, "Go up the %s",
                (u.ux == xupladder && u.uy == yupladder) ? "ladder" : "stairs");
        add_herecmd_menuitem(win, doup, buf);
    }
    if ((u.ux == xdnstair && u.uy == ydnstair) ||
        (u.ux == sstairs.sx && u.uy == sstairs.sy && !sstairs.up) ||
        (u.ux == xdnladder && u.uy == ydnladder)) {
        Sprintf(buf, "Go down the %s",
                (u.ux == xupladder && u.uy == yupladder) ? "ladder" : "stairs");
        add_herecmd_menuitem(win, dodown, buf);
    }
    if (u.usteed) { /* another movement choice */
        Sprintf(buf, "Dismount %s",
                x_monnam(u.usteed, ARTICLE_THE, (char *) 0, SUPPRESS_SADDLE, FALSE));
        add_herecmd_menuitem(win, doride, buf);
    }

#if 0
    if (Upolyd) { /* before objects */
        Sprintf(buf, "Use %s special ability",
                s_suffix(mons[u.umonnum].mname));
        add_herecmd_menuitem(win, domonability, buf);
    }
#endif

    if (OBJ_AT(u.ux, u.uy)) {
        struct obj *otmp = level.objects[u.ux][u.uy];

        Sprintf(buf, "Pick up %s", otmp->nexthere ? "items" : doname(otmp));
        add_herecmd_menuitem(win, dopickup, buf);

        if (Is_container(otmp)) {
            Sprintf(buf, "Loot %s", doname(otmp));
            add_herecmd_menuitem(win, doloot, buf);
        }
        if (otmp->oclass == FOOD_CLASS) {
            Sprintf(buf, "Eat %s", doname(otmp));
            add_herecmd_menuitem(win, doeat, buf);
        }
    }

    if (invent) {
        add_herecmd_menuitem(win, dodrop, "Drop items");
    }

    add_herecmd_menuitem(win, donull, "Rest one turn");
    add_herecmd_menuitem(win, dosearch, "Search around you");
    add_herecmd_menuitem(win, dolook, "Look at what is here");

    end_menu(win, "What do you want to do?");
    npick = select_menu(win, PICK_ONE, &picks);
    destroy_nhwindow(win);
    ch = '\0';
    if (npick > 0) {
        int NDECL((*func)) = picks->item.a_nfunc;
        free(picks);

        if (doit) {
            int ret = (*func)();

            ch = (char) ret;
        } else {
            ch = cmd_from_func(func);
        }
    }
    return ch;
}

static NEARDATA int last_multi;

/*
 * convert a MAP window position into a movecmd
 */
const char *
click_to_cmd(x, y, mod)
int x, y, mod;
{
    int dir;
    static char cmd[4];
    cmd[1]=0;

    if (iflags.clicklook && mod == CLICK_2) {
        clicklook_cc.x = x;
        clicklook_cc.y = y;
        cmd[0] = Cmd.spkeys[NHKF_CLICKLOOK];
        return cmd;
    }

    x -= u.ux;
    y -= u.uy;

    if (iflags.travelcmd) {
        iflags.autoexplore = FALSE;

        if (abs(x) <= 1 && abs(y) <= 1 ) {
            x = sgn(x), y = sgn(y);
        } else {
            u.tx = u.ux+x;
            u.ty = u.uy+y;
            cmd[0] = Cmd.spkeys[NHKF_TRAVEL];
            return cmd;
        }

        if (x == 0 && y == 0) {
            if (iflags.herecmd_menu) {
                cmd[0] = here_cmd_menu(FALSE);
                return cmd;
            }

            /* here */
            if (IS_FOUNTAIN(levl[u.ux][u.uy].typ) || IS_SINK(levl[u.ux][u.uy].typ)) {
                cmd[0] = cmd_from_func(mod == CLICK_1 ? dodrink : dodip);
                return cmd;
            } else if (IS_THRONE(levl[u.ux][u.uy].typ)) {
                cmd[0] = cmd_from_func(dosit);
                return cmd;
            } else if ((u.ux == xupstair && u.uy == yupstair)
                      || (u.ux == sstairs.sx && u.uy == sstairs.sy && sstairs.up)
                      || (u.ux == xupladder && u.uy == yupladder)) {
                cmd[0] = cmd_from_func(doup);
                return cmd;
            } else if ((u.ux == xdnstair && u.uy == ydnstair)
                      || (u.ux == sstairs.sx && u.uy == sstairs.sy && !sstairs.up)
                      || (u.ux == xdnladder && u.uy == ydnladder)) {
                cmd[0] = cmd_from_func(dodown);
                return cmd;
            } else if (OBJ_AT(u.ux, u.uy)) {
                cmd[0] = cmd_from_func(Is_container(level.objects[u.ux][u.uy]) ? doloot : dopickup);
                return cmd;
            } else {
                cmd[0] = cmd_from_func(donull); /* just rest */
                return cmd;
            }
        }

        /* directional commands */

        dir = xytod(x, y);

        if (!m_at(u.ux+x, u.uy+y) && !test_move(u.ux, u.uy, x, y, TEST_MOVE)) {
            cmd[1] = Cmd.dirchars[dir];
            cmd[2] = '\0';
            if (iflags.herecmd_menu) {
                cmd[0] = there_cmd_menu(FALSE, u.ux + x, u.uy + y);
                if (cmd[0] == '\0') {
                    cmd[1] = '\0';
                }
                return cmd;
            }

            if (IS_DOOR(levl[u.ux+x][u.uy+y].typ)) {
                /* slight assistance to the player: choose kick/open for them */
                if (levl[u.ux+x][u.uy+y].doormask & D_LOCKED) {
                    cmd[0] = cmd_from_func(dokick);
                    return cmd;
                }
                if (levl[u.ux+x][u.uy+y].doormask & D_CLOSED) {
                    cmd[0] = cmd_from_func(doopen);
                    return cmd;
                }
            }
            if (levl[u.ux+x][u.uy+y].typ <= SCORR) {
                cmd[0] = cmd_from_func(dosearch);
                cmd[1] = 0;
                return cmd;
            }
        }
    } else {
        /* convert without using floating point, allowing sloppy clicking */
        if(x > 2*abs(y))
            x = 1, y = 0;
        else if(y > 2*abs(x))
            x = 0, y = 1;
        else if(x < -2*abs(y))
            x = -1, y = 0;
        else if(y < -2*abs(x))
            x = 0, y = -1;
        else
            x = sgn(x), y = sgn(y);

        if (x == 0 && y == 0) {
            /* map click on player to "rest" command */
            cmd[0] = cmd_from_func(donull);
            return cmd;
        }

        dir = xytod(x, y);
    }

    /* move, attack, etc. */
    cmd[1] = 0;
    if (mod == CLICK_1) {
        cmd[0] = Cmd.dirchars[dir];
    } else {
        cmd[0] = (Cmd.num_pad ? M(Cmd.dirchars[dir]) :
                                (Cmd.dirchars[dir] - 'a' + 'A')); /* run command */
    }

    return cmd;
}

char
get_count(allowchars, inkey, maxcount, count, historical)
char *allowchars;
char inkey;
long maxcount;
long *count;
boolean historical; /* whether to include in message history: True => yes */
{
    char qbuf[QBUFSZ];
    int key;
    long cnt = 0L;
    boolean backspaced = FALSE;
    /* this should be done in port code so that we have erase_char
       and kill_char available; we can at least fake erase_char */
#define STANDBY_erase_char '\177'

    for (;;) {
        if (inkey) {
            key = inkey;
            inkey = '\0';
        } else
            key = readchar();

        if (digit(key)) {
            cnt = 10 * cnt + (long) (key - '0');
            if (cnt < 0) {
                cnt = 0;
            } else if (maxcount > 0 && cnt > maxcount) {
                cnt = maxcount;
            }
        } else if (cnt && (key == '\b' || key == STANDBY_erase_char)) {
            cnt = cnt / 10;
            backspaced = TRUE;
        } else if (key == Cmd.spkeys[NHKF_ESC]) {
            break;
        } else if (!allowchars || index(allowchars, key)) {
            *count = cnt;
            break;
        }

        if (cnt > 9 || backspaced) {
            clear_nhwindow(WIN_MESSAGE);
            if (backspaced && !cnt) {
                Sprintf(qbuf, "Count: ");
            } else {
                Sprintf(qbuf, "Count: %ld", cnt);
                backspaced = FALSE;
            }
            custompline(SUPPRESS_HISTORY, "%s", qbuf);
            mark_synch();
        }
    }

    if (historical) {
        Sprintf(qbuf, "Count: %ld ", *count);
        (void) key2txt((uchar) key, eos(qbuf));
#if NEXT_VERSION
        putmsghistory(qbuf, FALSE);
#endif
    }

    return key;
}

STATIC_OVL char *
parse()
{
#ifdef LINT /* static char in_line[COLNO]; */
    char in_line[COLNO];
#else
    static char in_line[COLNO];
#endif
    register int foo;
    boolean prezero = FALSE;

    multi = 0;
    flags.move = 1;
    flush_screen(1); /* Flush screen buffer. Put the cursor on the hero. */

#ifdef ALTMETA
    alt_esc = iflags.altmeta; /* readchar() hack */
#endif
    if (!Cmd.num_pad || (foo = readchar()) == Cmd.spkeys[NHKF_COUNT]) {
        long tmpmulti = multi;

        foo = get_count((char *) 0, '\0', LARGEST_INT, &tmpmulti, FALSE);
        last_multi = multi = tmpmulti;
    }
#ifdef ALTMETA
    alt_esc = FALSE; /* readchar() reset */
#endif

#if NEXT_VERSION
    /* if fuzzing, override '!' and ^Z */
    if (iflags.debug_fuzzer &&
        (Cmd.commands[foo & 0x0ff] &&
        (Cmd.commands[foo & 0x0ff]->ef_funct == dosuspend_core ||
         Cmd.commands[foo & 0x0ff]->ef_funct == dosh_core))) {
        foo = Cmd.spkeys[NHKF_ESC];
    }
#endif

    /* esc cancels count (TH) */
    if (foo == Cmd.spkeys[NHKF_ESC]) {
        clear_nhwindow(WIN_MESSAGE);
        multi = last_multi = 0;
# ifdef REDO
    } else if (foo == Cmd.spkeys[NHKF_DOAGAIN] || in_doagain) {
        multi = last_multi;
    } else {
        last_multi = multi;
        savech(0); /* reset input queue */
        savech((char)foo);
# endif
    }

    if (multi) {
        multi--;
        save_cm = in_line;
    } else {
        save_cm = (char *)0;
    }
    /* in 3.4.3 this was in rhack(), where it was too late to handle M-5 */
    if (Cmd.pcHack_compat) {
        /* This handles very old inconsistent DOS/Windows behaviour
           in a different way: earlier, the keyboard handler mapped
           these, which caused counts to be strange when entered
           from the number pad. Now do not map them until here. */
        switch (foo) {
        case '5':
            foo = Cmd.spkeys[NHKF_RUSH];
            break;
        case M('5'):
            foo = Cmd.spkeys[NHKF_RUN];
            break;
        case M('0'):
            foo = Cmd.spkeys[NHKF_DOINV];
            break;
        default:
            break; /* as is */
        }
    }

    in_line[0] = foo;
    in_line[1] = '\0';
    if (prefix_cmd(foo)) {
        foo = readchar();
#ifdef REDO
        savech((char)foo);
#endif
        in_line[1] = foo;
        in_line[2] = 0;
    }
    clear_nhwindow(WIN_MESSAGE);

    iflags.in_parse = FALSE;
    return(in_line);
}

#ifdef UNIX
static
void
end_of_input()
{
#ifdef NOSAVEONHANGUP
#ifdef INSURANCE
    if (flags.ins_chkpt && program_state.something_worth_saving) {
        program_state.preserve_locks = 1; /* keep files for recovery */
    }
#endif
    program_state.something_worth_saving = 0; /* don't save */
#endif

#ifndef SAFERHANGUP
    if (!program_state.done_hup++) {
#endif
        if (program_state.something_worth_saving) {
            (void) dosave0();
        }
#ifndef SAFERHANGUP
    }
#endif
    if (iflags.window_inited) {
        exit_nhwindows((char *) 0);
    }
    clearlocks();
    nh_terminate(EXIT_SUCCESS);

}
#endif /* HANGUPHANDLING */

char
readchar()
{
    register int sym;
    int x = u.ux, y = u.uy, mod = 0;

    if (iflags.debug_fuzzer) {
        return randomkey();
    }

    if ( *readchar_queue )
        sym = *readchar_queue++;
    else
#ifdef REDO
        sym = in_doagain ? pgetchar() : nh_poskey(&x, &y, &mod);
#else
        sym = pgetchar();
#endif

#ifdef NR_OF_EOFS
    if (sym == EOF) {
        register int cnt = NR_OF_EOFS;
        /*
         * Some SYSV systems seem to return EOFs for various reasons
         * (?like when one hits break or for interrupted systemcalls?),
         * and we must see several before we quit.
         */
        do {
            clearerr(stdin); /* omit if clearerr is undefined */
            sym = pgetchar();
        } while (--cnt && sym == EOF);
    }
#endif /* NR_OF_EOFS */

    if (sym == EOF) {
#ifdef HANGUPHANDLING
        hangup(0); /* call end_of_input() or set program_state.done_hup */
#endif
        sym = '\033';
#ifdef ALTMETA
    } else if (sym == '\033' && alt_esc) {
        /* iflags.altmeta: treat two character ``ESC c'' as single `M-c' */
        sym = *readchar_queue ? *readchar_queue++ : pgetchar();
        if (sym == EOF || sym == 0) {
            sym = '\033';
        } else if (sym != '\033') {
            sym |= 0200; /* force 8th bit on */
        }
#endif /*ALTMETA*/
    } else if (sym == 0) {
        /* click event */
        readchar_queue = click_to_cmd(x, y, mod);
        sym = *readchar_queue++;
    }
    return((char) sym);
}

/** Returns the number of known up- or downstairs. */
static int
find_remembered_stairs(boolean upstairs, coord *cc)
{
    xchar x, y;
    int stair, ladder, branch;
    if (upstairs) {
        stair = S_upstair;
        ladder = S_upladder;
        branch = S_upstair;
    } else {
        stair = S_dnstair;
        ladder = S_dnladder;
        branch = S_dnstair;
    }

    /* Prefer already marked travel positions. */
    x = iflags.travelcc.x;
    y = iflags.travelcc.y;
    if (isok(x, y) &&
        (glyph_to_cmap(level.locations[x][y].glyph) == stair ||
         glyph_to_cmap(level.locations[x][y].glyph) == ladder ||
         glyph_to_cmap(level.locations[x][y].glyph) == branch)) {
        cc->x = x;
        cc->y = y;
        return TRUE;
    }

    /* We can't reference the stairs directly because mimics can mimic fake
       ones. */
    int found_stairs = 0;
    for (x = 0; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            if (glyph_to_cmap(level.locations[x][y].glyph) == stair ||
                glyph_to_cmap(level.locations[x][y].glyph) == ladder ||
                glyph_to_cmap(level.locations[x][y].glyph) == branch) {
                if (found_stairs == 0) {
                    cc->x = x;
                    cc->y = y;
                }
                found_stairs++;
            }
        }
    }

    return found_stairs;
}

static int not_interactive=0;
int
do_stair_travel(char up_or_down)
{
    boolean upstairs = (up_or_down == '<');
    coord cc;
    int stairs = 0;
    if ((stairs = find_remembered_stairs(upstairs, &cc)) > 0) {
        iflags.travelcc.x = cc.x;
        iflags.travelcc.y = cc.y;

        /* kludge, the command methods should be properly refactored similar
         * to what nh4 did */
        not_interactive = (stairs == 1);
        dotravel();
        not_interactive = 0;
        return 1;
    }
    return 0;
}

/** '_' command, #travel, via keyboard rather than mouse click */
STATIC_PTR int
dotravel()
{
    /* Keyboard travel command */
    static char cmd[2];
    coord cc;

    /* [FIXME?  Supporting the ability to disable traveling via mouse
       click makes some sense, depending upon overall mouse usage.
       Disabling '_' on a user by user basis makes no sense at all since
       even if it is typed by accident, aborting when picking a target
       destination is trivial.  Travel via mouse predates travel via '_',
       and this use of OPTION=!travel is probably just a mistake....] */
    if (!iflags.travelcmd) return 0;

    cmd[1]=0;
    cc.x = iflags.travelcc.x;
    cc.y = iflags.travelcc.y;
    if (cc.x <= 0 && cc.y <= 0) {
        /* No cached destination, start attempt from current position */
        cc.x = u.ux;
        cc.y = u.uy;
    }

    if (!not_interactive) {
        iflags.getloc_travelmode = TRUE;
        if (iflags.menu_requested) {
            int gf = iflags.getloc_filter;
            iflags.getloc_filter = GFILTER_VIEW;
            if (!getpos_menu(&cc, GLOC_INTERESTING)) {
                iflags.getloc_filter = gf;
                iflags.getloc_travelmode = FALSE;
                return 0;
            }
            iflags.getloc_filter = gf;
        } else {
            pline("Where do you want to travel to?");
            if (getpos(&cc, TRUE, "the desired destination") < 0) {
                /* user pressed ESC */
                iflags.getloc_travelmode = FALSE;
                return 0;
            }
        }
    }
    iflags.getloc_travelmode = FALSE;
    iflags.travelcc.x = u.tx = cc.x;
    iflags.travelcc.y = u.ty = cc.y;
    iflags.autoexplore = FALSE;
    cmd[0] = Cmd.spkeys[NHKF_TRAVEL];
    readchar_queue = cmd;
    return 0;
}

STATIC_PTR int
doautoexplore()
{
    static char cmd[2];
    iflags.autoexplore = TRUE;
    cmd[0] = CMD_TRAVEL;
    readchar_queue = cmd;
    return 0;
}

/** Chooses a suitable monster and fights it. */
STATIC_PTR int
doautofight()
{
    int i, j;
    struct monst *mtmp;
    /* TODO: - selection strategy when facing multiple monsters
     *       - fire ammunition when wielding a launcher
     *       - fire quivered objects when monsters are in range?
     */

    /* check surrounding squares for monster to attack */
    for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
            if (!isok(u.ux+i, u.uy+j)) continue;
            mtmp = m_at(u.ux+i, u.uy+j);
            if (mtmp && canspotmon(mtmp) && !is_safepet(mtmp)) {
                if (attack(mtmp)) return 1;
                break;
            }
        }
    }

    return 0;
}

#ifdef PORT_DEBUG
# ifdef WIN32CON
extern void NDECL(win32con_debug_keystrokes);
extern void NDECL(win32con_handler_info);
# endif

int
wiz_port_debug()
{
    int n, k;
    winid win;
    anything any;
    int item = 'a';
    int num_menu_selections;
    struct menu_selection_struct {
        char *menutext;
        void NDECL((*fn));
    } menu_selections[] = {
#ifdef WIN32CON
        {"test win32 keystrokes", win32con_debug_keystrokes},
        {"show keystroke handler information", win32con_handler_info},
#endif
        {(char *)0, (void NDECL((*))) 0}     /* array terminator */
    };

    num_menu_selections = SIZE(menu_selections) - 1;
    if (num_menu_selections > 0) {
        menu_item *pick_list;
        win = create_nhwindow(NHW_MENU);
        start_menu(win);
        for (k=0; k < num_menu_selections; ++k) {
            any.a_int = k+1;
            add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, item++, 0, ATR_NONE,
                     menu_selections[k].menutext, MENU_UNSELECTED);
        }
        end_menu(win, "Which port debugging feature?");
        n = select_menu(win, PICK_ONE, &pick_list);
        destroy_nhwindow(win);
        if (n > 0) {
            n = pick_list[0].item.a_int - 1;
            free((genericptr_t) pick_list);
            /* execute the function */
            (*menu_selections[n].fn)();
        }
    } else
        pline("No port-specific debug capability defined.");
    return 0;
}
# endif /*PORT_DEBUG*/

/**
 *   Parameter validator for generic yes/no function to prevent
 *   the core from sending too long a prompt string to the
 *   window port causing a buffer overflow there.
 */
char
yn_function(query, resp, def)
const char *query, *resp;
char def;
{
    char res, qbuf[QBUFSZ];

    iflags.last_msg = PLNMSG_UNKNOWN; /* most recent pline is clobbered */

    /* maximum acceptable length is QBUFSZ-1 */
    if (strlen(query) >= QBUFSZ) {
        /* caller shouldn't have passed anything this long */
        paniclog("Query truncated: ", query);
        (void) strncpy(qbuf, query, QBUFSZ-4);
        Strcpy(&qbuf[QBUFSZ-4], "...");
        query = qbuf;
    }
    return (*windowprocs.win_yn_function)(query, resp, def);

}

/**
 * Asks the player a yes/no question if paranoid is true.
 *
 * @return 'y' or 'n'
 */
char
paranoid_yn(query, paranoid)
const char *query;
boolean paranoid;
{
    if (paranoid) {
        char buf[BUFSZ];
        char query_yesno[2*BUFSZ];
        /* put [yes/no] between question and question mark? */
        Sprintf(query_yesno, "%s [yes/no]", query);
        getlin (query_yesno, buf);
        (void) lcase (buf);
        return (!(strcmp (buf, "yes"))) ? 'y' : 'n';
    } else {
        return yn(query);
    }
}

/* ^Z command, #suspend */
static int
dosuspend_core()
{
#if NEXT_VERSION
#ifdef SUSPEND
    /* Does current window system support suspend? */
    if ((*windowprocs.win_can_suspend)()) {
        dosuspend();
    } else
#endif
        Norep(cmdnotavail, "#suspend");
#else
# ifdef SUSPEND
        dosuspend();
# endif
#endif
    return 0;
}

/**
 * Tracks how many times the player "cheated" in Sokoban.
 *
 * Stops tracking after the player found the sokoban prize.
 */
void
sokoban_trickster()
{
#ifdef RECORD_ACHIEVE
    if (In_sokoban(&u.uz)) {
        if (Sokoban && !achieve.finish_sokoban) {
            /* Sokoban unsolved and not yet found the sokoban prize */
            u.uconduct.sokoban += 1;
            /* inform the player on the first transgression */
            if (u.uconduct.sokoban == 1) {
                msg_luck_change(0);
            }
        }
    }
#endif
}

static int
debug_show_colors()
{
    // TODO should be (*windowprocs.tty_debug_show_colors)();
#ifdef TTY_GRAPHICS
    if (!strncmpi(windowprocs.name, "tty", 3)) {
        tty_debug_show_colors();
    }
#endif
#ifdef CURSES_GRAPHICS
    if (!strncmpi(windowprocs.name, "curses", 6)) {
        curses_debug_show_colors();
    }
#endif
    return 0;
}

/*cmd.c*/
