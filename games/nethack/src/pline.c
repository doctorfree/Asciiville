/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#define NEED_VARARGS /* Uses ... */ /* comment line for pre-compiled headers */
#include "hack.h"

static boolean no_repeat = FALSE;

static char *FDECL(You_buf, (int));

#if defined(DUMP_LOG) && defined(DUMPMSGS)
char msgs[DUMPMSGS][BUFSZ];
int msgs_count[DUMPMSGS];
int lastmsg = -1;
#endif

void
msgpline_add(typ, pattern)
int typ;
char *pattern;
{
    struct _plinemsg *tmp = (struct _plinemsg *) alloc(sizeof(struct _plinemsg));
    if (!tmp) return;
    tmp->msgtype = typ;
    tmp->pattern = strdup(pattern);
    tmp->next = pline_msg;
    pline_msg = tmp;
}

void
msgpline_free()
{
    struct _plinemsg *tmp = pline_msg;
    struct _plinemsg *tmp2;
    while (tmp) {
        free(tmp->pattern);
        tmp2 = tmp;
        tmp = tmp->next;
        free(tmp2);
    }
    pline_msg = NULL;
}

int
msgpline_type(msg)
char *msg;
{
    struct _plinemsg *tmp = pline_msg;
    while (tmp) {
        if (pmatch(tmp->pattern, msg)) return tmp->msgtype;
        tmp = tmp->next;
    }
    return MSGTYP_NORMAL;
}

static unsigned pline_flags = 0;

/* keeps windowprocs usage out of pline() */
static void
putmesg(const char *line)
{
    int attr = ATR_NONE;

    if (pline_flags & URGENT_MESSAGE) {
        attr |= ATR_URGENT;
    }
    if (pline_flags & SUPPRESS_HISTORY) {
        attr |= ATR_NOHISTORY;
    }

    putstr(WIN_MESSAGE, attr, line);
}

/*VARARGS1*/
/* Note that these declarations rely on knowledge of the internals
 * of the variable argument handling stuff in "tradstdc.h"
 */

#if defined(USE_STDARG) || defined(USE_VARARGS)
static void FDECL(vpline, (const char *, va_list));

void
pline VA_DECL(const char *, line)
{
    VA_START(line);
    VA_INIT(line, char *);
    vpline(line, VA_ARGS);
    VA_END();
}

char prevmsg[BUFSZ];

# ifdef USE_STDARG
static void
vpline(const char *line, va_list the_args) {
# else
static void
vpline(line, the_args) const char *line; va_list the_args; {
# endif

#else   /* USE_STDARG | USE_VARARG */

#define vpline pline

void
pline VA_DECL(const char *, line)
{
#endif  /* USE_STDARG | USE_VARARG */

    char pbuf[BUFSZ];
    int typ;
/* Do NOT use VA_START and VA_END in here... see above */

    if (!line || !*line) return;
    if (index(line, '%')) {
        Vsprintf(pbuf, line, VA_ARGS);
        line = pbuf;
    }
#if defined(DUMP_LOG) && defined(DUMPMSGS)
    if (DUMPMSGS > 0 && !program_state.gameover) {
        if ((pline_flags & SUPPRESS_HISTORY) == 0) {
            /* count identical messages */
            if (lastmsg >= 0 && !strncmp(msgs[lastmsg], line, BUFSZ)) {
                msgs_count[lastmsg] += 1;
            } else {
                lastmsg = (lastmsg + 1) % DUMPMSGS;
                strncpy(msgs[lastmsg], line, BUFSZ);
                msgs_count[lastmsg] = 1;
            }
        }
    }
#endif
    typ = msgpline_type(line);
    if (!iflags.window_inited) {
        raw_print(line);
        return;
    }
#ifndef MAC
    if (no_repeat && !strcmp(line, toplines))
        return;
#endif /* MAC */
    if (vision_full_recalc) {
        vision_recalc(0);
    }
    if (u.ux) {
        flush_screen(1); /* %% */
    }
    if (typ == MSGTYP_NOSHOW) return;
    if (typ == MSGTYP_NOREP && !strcmp(line, prevmsg)) return;
    putmesg(line);
    strncpy(prevmsg, line, BUFSZ);
    if (typ == MSGTYP_STOP) display_nhwindow(WIN_MESSAGE, TRUE); /* --more-- */
}

/* pline() variant which can override MSGTYPE handling or suppress
   message history (tty interface uses pline() to issue prompts and
   they shouldn't be blockable via MSGTYPE=hide) */
/*VARARGS2*/
void custompline
VA_DECL2(unsigned, pflags, const char *, line)
{
    VA_START(line);
    VA_INIT(line, const char *);
    pline_flags = pflags;
    vpline(line, VA_ARGS);
    pline_flags = 0;
    VA_END();
    return;
}

/*VARARGS1*/
void
Norep VA_DECL(const char *, line)
{
    VA_START(line);
    VA_INIT(line, const char *);
    no_repeat = TRUE;
    vpline(line, VA_ARGS);
    no_repeat = FALSE;
    VA_END();
    return;
}

/* work buffer for You(), &c and verbalize() */
static char *you_buf = 0;
static int you_buf_siz = 0;

static char *
You_buf(siz)
int siz;
{
    if (siz > you_buf_siz) {
        if (you_buf) free((genericptr_t) you_buf);
        you_buf_siz = siz + 10;
        you_buf = (char *) alloc((unsigned) you_buf_siz);
    }
    return you_buf;
}

void
free_youbuf()
{
    if (you_buf) free((genericptr_t) you_buf),  you_buf = (char *)0;
    you_buf_siz = 0;
}

/* `prefix' must be a string literal, not a pointer */
#define YouPrefix(pointer, prefix, text) \
    Strcpy((pointer = You_buf((int)(strlen(text) + sizeof prefix))), prefix)

#define YouMessage(pointer, prefix, text) \
    strcat((YouPrefix(pointer, prefix, text), pointer), text)

/*VARARGS1*/
void
You VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, "You ", line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void
Your VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, "Your ", line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void
You_feel VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, "You feel ", line), VA_ARGS);
    VA_END();
}


/*VARARGS1*/
void
You_cant VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, "You can't ", line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void
pline_The VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, "The ", line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void
There VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, "There ", line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void
You_hear VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    if (Underwater) {
        YouPrefix(tmp, "You barely hear ", line);
    } else if (Unaware) {
        YouPrefix(tmp, "You dream that you hear ", line);
    } else {
        YouPrefix(tmp, "You hear ", line);
    }
    vpline(strcat(tmp, line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void You_see
VA_DECL(const char *, line)
{
    char *tmp;

    VA_START(line);
    VA_INIT(line, const char *);
    if (Unaware) {
        YouPrefix(tmp, "You dream that you see ", line);
    } else if (Blind) { /* caller should have caught this... */
        YouPrefix(tmp, "You sense ", line);
    } else {
        YouPrefix(tmp, "You see ", line);
    }
    vpline(strcat(tmp, line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void
verbalize VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    tmp = You_buf((int)strlen(line) + sizeof "\"\"");
    Strcpy(tmp, "\"");
    Strcat(tmp, line);
    Strcat(tmp, "\"");
    vpline(tmp, VA_ARGS);
    VA_END();
}

/*VARARGS1*/
/* Note that these declarations rely on knowledge of the internals
 * of the variable argument handling stuff in "tradstdc.h"
 */

#if defined(USE_STDARG) || defined(USE_VARARGS)
static void FDECL(vraw_printf, (const char *, va_list));

void
raw_printf VA_DECL(const char *, line)
{
    VA_START(line);
    VA_INIT(line, char *);
    vraw_printf(line, VA_ARGS);
    VA_END();
}

# ifdef USE_STDARG
static void
vraw_printf(const char *line, va_list the_args) {
# else
static void
vraw_printf(line, the_args) const char *line; va_list the_args; {
# endif

#else  /* USE_STDARG | USE_VARARG */

void
raw_printf VA_DECL(const char *, line)
{
#endif
/* Do NOT use VA_START and VA_END in here... see above */

    if(!index(line, '%'))
        raw_print(line);
    else {
        char pbuf[BUFSZ];
        Vsprintf(pbuf, line, VA_ARGS);
        raw_print(pbuf);
    }
}


/*VARARGS1*/
void
impossible VA_DECL(const char *, s)
{
    char pbuf[2*BUFSZ];
    VA_START(s);
    VA_INIT(s, const char *);
    if (program_state.in_impossible) {
        panic("impossible called impossible");
    }

    program_state.in_impossible = 1;
    Vsprintf(pbuf, s, VA_ARGS);
    pbuf[BUFSZ-1] = '\0'; /* sanity */
    paniclog("impossible", pbuf);
    if (iflags.debug_fuzzer) {
        panic("%s", pbuf);
    }
    pline("%s", pbuf);
    /* reuse pbuf[] */
    Strcpy(pbuf, "Program in disorder!");
    if (program_state.something_worth_saving) {
        Strcat(pbuf, "  (Saving and reloading may fix this problem.)");
    }
    pline("%s", pbuf);
    pline("Please report these messages to %s.", DEVTEAM_EMAIL);
    if (sysopt.support) {
        pline("Alternatively, contact local support: %s", sysopt.support);
    }

    program_state.in_impossible = 0;
    VA_END();
}

void
warning VA_DECL(const char *, s)
{
    char pbuf[2*BUFSZ];
    VA_START(s);
    VA_INIT(s, const char *);
    Vsprintf(pbuf, s, VA_ARGS);
    pbuf[BUFSZ-1] = '\0'; /* sanity */
    paniclog("warning", pbuf);
    pline("Warning: %s\n", pbuf);
    if (iflags.debug_fuzzer) {
        panic("Warning: %s", pbuf);
    }
    VA_END();
}

const char *
align_str(alignment)
aligntyp alignment;
{
    switch ((int)alignment) {
    case A_CHAOTIC: return "chaotic";
    case A_NEUTRAL: return "neutral";
    case A_LAWFUL:  return "lawful";
    case A_NONE:    return "unaligned";
    }
    return "unknown";
}

/* stethoscope or probing applied to monster -- one-line feedback */
void
mstatusline(mtmp)
struct monst *mtmp;
{
    aligntyp alignment = mon_aligntyp(mtmp);
    char info[BUFSZ], monnambuf[BUFSZ];

    info[0] = 0;
    if (mtmp->mtame) {    Strcat(info, ", tame");
#ifdef WIZARD
                          if (wizard) {
                              Sprintf(eos(info), " (%d", mtmp->mtame);
                              if (!mtmp->isminion)
                                  Sprintf(eos(info), "; hungry %ld; apport %d",
                                          EDOG(mtmp)->hungrytime, EDOG(mtmp)->apport);
                              Strcat(info, ")");
                          }
#endif
    }
    else if (mtmp->mpeaceful) Strcat(info, ", peaceful");

    if (mtmp->data == &mons[PM_LONG_WORM]) {
        int segndx, nsegs = count_wsegs(mtmp);

        /* the worm code internals don't consider the head of be one of
           the worm's segments, but we count it as such when presenting
           worm feedback to the player */
        if (!nsegs) {
            Strcat(info, ", single segment");
        } else {
            ++nsegs; /* include head in the segment count */
            segndx = wseg_at(mtmp, bhitpos.x, bhitpos.y);
            Sprintf(eos(info), ", %d%s of %d segments", segndx, ordin(segndx), nsegs);
        }
    }
    if (mtmp->cham >= LOW_PM && mtmp->data != &mons[mtmp->cham]) {
        /* don't reveal the innate form (chameleon, vampire, &c),
           just expose the fact that this current form isn't it */
        Strcat(info, ", shapechanger");
    }
    if (mtmp->meating) Strcat(info, ", eating");
    /* a stethoscope exposes mimic before getting here so this
       won't be relevant for it, but wand of probing doesn't */
    if (mtmp->mundetected || mtmp->m_ap_type) {
        mhidden_description(mtmp, TRUE, eos(info));
    }
    if (mtmp->mcan) Strcat(info, ", cancelled");
    if (mtmp->mconf) Strcat(info, ", confused");
    if (mtmp->mblinded || !mtmp->mcansee)
        Strcat(info, ", blind");
    if (mtmp->mstun) Strcat(info, ", stunned");
    if (mtmp->msleeping) Strcat(info, ", asleep");
#if 0   /* unfortunately mfrozen covers temporary sleep and being busy
           (donning armor, for instance) as well as paralysis */
    else if (mtmp->mfrozen) Strcat(info, ", paralyzed");
#else
    else if (mtmp->mfrozen || !mtmp->mcanmove)
        Strcat(info, ", can't move");
#endif
    /* [arbitrary reason why it isn't moving] */
    else if (mtmp->mstrategy & STRAT_WAITMASK)
        Strcat(info, ", meditating");
    else if (mtmp->mflee) Strcat(info, ", scared");
    if (mtmp->mtrapped) Strcat(info, ", trapped");
    if (mtmp->mfeetfrozen) Strcat(info, ", stuck in ice");
    if (mtmp->mspeed) Strcat(info,
                             mtmp->mspeed == MFAST ? ", fast" :
                             mtmp->mspeed == MSLOW ? ", slow" :
                             ", ???? speed");
    if (mtmp->mundetected) Strcat(info, ", concealed");
    if (mtmp->minvis) Strcat(info, ", invisible");
    if (mtmp == u.ustuck) Strcat(info,
                                 (sticks(youmonst.data)) ? ", held by you" :
                                 !u.uswallow ? ", holding you" :
                                 attacktype_fordmg(u.ustuck->data, AT_ENGL, AD_DGST) ? ", digesting you" :
                                 (is_animal(u.ustuck->data) ?  ", swallowing you" :
                                  ", engulfed you"));
#ifdef STEED
    if (mtmp == u.usteed) Strcat(info, ", carrying you");
#endif
#ifdef WIZARD
    if (wizard &&
        mtmp->isshk && ESHK(mtmp)->cheapskate) {
        Strcat(info, ", cheapskate");
    }
#endif

    /* avoid "Status of the invisible newt ..., invisible" */
    /* and unlike a normal mon_nam, use "saddled" even if it has a name */
    Strcpy(monnambuf, x_monnam(mtmp, ARTICLE_THE, (char *)0,
                               (SUPPRESS_IT|SUPPRESS_INVISIBLE), FALSE));

    pline("Status of %s (%s):  Level %d  HP %d(%d)  AC %d%s.",
          monnambuf,
          align_str(alignment),
          mtmp->m_lev,
          mtmp->mhp,
          mtmp->mhpmax,
          find_mac(mtmp),
          info);

    /* Heisenberg's code */
    if (mtmp->data == &mons[PM_QUANTUM_MECHANIC]) {
        if (canspotmon(mtmp))
            pline("Having determined %s's speed, you are unable to know its location.",
                  mon_nam(mtmp));
        (void) rloc(mtmp, FALSE);
    } else if (mtmp->data == &mons[PM_CTHULHU]) {
        pline("There are some things incapable of being understood!");
        make_confused(HConfusion + rnd(20), FALSE);
    }
}

/* stethoscope or probing applied to hero -- one-line feedback */
void
ustatusline()
{
    char info[BUFSZ];

    info[0] = '\0';
    if (Sick) {
        Strcat(info, ", dying from");
        if (u.usick_type & SICK_VOMITABLE)
            Strcat(info, " food poisoning");
        if (u.usick_type & SICK_NONVOMITABLE) {
            if (u.usick_type & SICK_VOMITABLE)
                Strcat(info, " and");
            Strcat(info, " illness");
        }
    }
    if (Stoned) Strcat(info, ", solidifying");
    if (Slimed) Strcat(info, ", becoming slimy");
    if (Strangled) Strcat(info, ", being strangled");
    if (Vomiting) Strcat(info, ", nauseated");       /* !"nauseous" */
    if (Confusion) Strcat(info, ", confused");
    if (Blind) {
        Strcat(info, ", blind");
        if (u.ucreamed) {
            if ((long)u.ucreamed < Blinded || Blindfolded
                || !haseyes(youmonst.data))
                Strcat(info, ", cover");
            Strcat(info, "ed by sticky goop");
        }   /* note: "goop" == "glop"; variation is intentional */
    }
    if (Stunned) Strcat(info, ", stunned");
#ifdef STEED
    if (!u.usteed)
#endif
    if (Wounded_legs) {
        const char *what = body_part(LEG);
        if ((Wounded_legs & BOTH_SIDES) == BOTH_SIDES)
            what = makeplural(what);
        Sprintf(eos(info), ", injured %s", what);
    }
    if (Glib) Sprintf(eos(info), ", slippery %s",
                      makeplural(body_part(HAND)));
    if (u.utrap) Strcat(info, ", trapped");
    if (u.ufeetfrozen) Strcat(info, ", stuck in ice");
    if (Fast) Strcat(info, Very_fast ?
                     ", very fast" : ", fast");
    if (u.uundetected) Strcat(info, ", concealed");
    if (Invis) Strcat(info, ", invisible");
    if (u.ustuck) {
        if (sticks(youmonst.data))
            Strcat(info, ", holding ");
        else
            Strcat(info, ", held by ");
        Strcat(info, mon_nam(u.ustuck));
    }

    pline("Status of %s (%s):  Level %d  HP %d(%d)  AC %d%s.",
          plname,
          piousness(FALSE, align_str(u.ualign.type)),
          Upolyd ? mons[u.umonnum].mlevel : u.ulevel,
          Upolyd ? u.mh : u.uhp,
          Upolyd ? u.mhmax : u.uhpmax,
          u.uac,
          info);
}

void
self_invis_message()
{
    pline("%s %s.",
          Hallucination ? "Far out, man!  You" : "Gee!  All of a sudden, you",
          See_invisible ? "can see right through yourself" :
          "can't see yourself");
}

/*pline.c*/
