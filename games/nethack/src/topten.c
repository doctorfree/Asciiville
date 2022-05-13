/*  SCCS Id: @(#)topten.c   3.4 2000/01/21  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "dlb.h"
#ifdef SHORT_FILENAMES
#include "patchlev.h"
#else
#include "patchlevel.h"
#endif

#ifdef VMS
/* We don't want to rewrite the whole file, because that entails     */
/* creating a new version which requires that the old one be deletable. */
# define UPDATE_RECORD_IN_PLACE
#endif

/*
 * Updating in place can leave junk at the end of the file in some
 * circumstances (if it shrinks and the O.S. doesn't have a straightforward
 * way to truncate it).  The trailing junk is harmless and the code
 * which reads the scores will ignore it.
 */
#ifdef UPDATE_RECORD_IN_PLACE
static long final_fpos;
#endif

#define done_stopprint program_state.stopprint

#define newttentry() (struct toptenentry *) alloc(sizeof(struct toptenentry))
#define dealloc_ttentry(ttent) free((genericptr_t) (ttent))
#define NAMSZ   10
#define DTHSZ   100
#define ROLESZ   3
#define PERSMAX  3      /* entries per name/uid per char. allowed */
#define POINTSMIN   1   /* must be > 0 */
#define ENTRYMAX    100 /* must be >= 10 */

#ifndef AUTOCONF
#if !defined(MICRO) && !defined(MAC) && !defined(WIN32)
#define PERS_IS_UID     /* delete for PERSMAX per name; now per uid */
#endif
#endif /* AUTOCONF_H */
struct toptenentry {
    struct toptenentry *tt_next;
#ifdef UPDATE_RECORD_IN_PLACE
    long fpos;
#endif
    long points;
    int deathdnum, deathlev;
    int maxlvl, hp, maxhp, deaths;
    int ver_major, ver_minor, patchlevel;
    long deathdate, birthdate;
    int uid;
    char plrole[ROLESZ+1];
    char plrace[ROLESZ+1];
    char plgend[ROLESZ+1];
    char plalign[ROLESZ+1];
    char name[NAMSZ+1];
    char death[DTHSZ+1];
} *tt_head;

STATIC_DCL void FDECL(topten_print, (const char *));
STATIC_DCL void FDECL(topten_print_bold, (const char *));
STATIC_DCL xchar FDECL(observable_depth, (d_level *));
STATIC_DCL void NDECL(outheader);
STATIC_DCL void FDECL(outentry, (int, struct toptenentry *, BOOLEAN_P));
STATIC_DCL void FDECL(readentry, (FILE *, struct toptenentry *));
STATIC_DCL void FDECL(writeentry, (FILE *, struct toptenentry *));
#ifdef XLOGFILE
STATIC_DCL void FDECL(munge_xlstring, (char *dest, char *src, int n));
STATIC_DCL void FDECL(write_xlentry, (FILE *, struct toptenentry *));
static void FDECL(add_achieveX, (char *, const char *, BOOLEAN_P));
static char *NDECL(encode_extended_achievements);
static char *NDECL(encode_extended_conducts);
#endif
STATIC_DCL void FDECL(free_ttlist, (struct toptenentry *));
STATIC_DCL int FDECL(classmon, (char *, BOOLEAN_P));
STATIC_DCL int FDECL(score_wanted,
                     (BOOLEAN_P, int, struct toptenentry *, int, const char **, int));
#ifdef RECORD_ACHIEVE
STATIC_DCL long FDECL(encodeachieve, (void));
#endif
STATIC_DCL long FDECL(encode_xlogflags, (void));
#ifdef NO_SCAN_BRACK
STATIC_DCL void FDECL(nsb_mung_line, (char*));
STATIC_DCL void FDECL(nsb_unmung_line, (char*));
#endif
static char* FDECL(killed_uniques, (void));

/* must fit with end.c; used in rip.c */
NEARDATA const char * const killed_by_prefix[] = {
    "killed by ", "choked on ", "poisoned by ", "died of ", "drowned in ",
    "burned by ", "dissolved in ", "crushed to death by ", "petrified by ",
    "turned to slime by ", "killed by ",
#ifdef WEBB_DISINT
    "disintegrated by ",
#endif
    "turned to slime by ", "killed by ",
#ifdef ASTRAL_ESCAPE
    "",
#endif
    "", "", "", ""
};

static winid toptenwin = WIN_ERR;

STATIC_OVL void
topten_print(x)
const char *x;
{
    if (toptenwin == WIN_ERR)
        raw_print(x);
    else
        putstr(toptenwin, ATR_NONE, x);
}

STATIC_OVL void
topten_print_bold(x)
const char *x;
{
    if (toptenwin == WIN_ERR)
        raw_print_bold(x);
    else
        putstr(toptenwin, ATR_BOLD, x);
}

STATIC_OVL xchar
observable_depth(lev)
d_level *lev;
{
#ifdef RANDOMIZED_PLANES    /* if we ever randomize the order of the elemental planes, we
                               must use a constant external representation in the record file */
    if (In_endgame(lev)) {
        if (Is_astralevel(lev)) return -5;
        else if (Is_waterlevel(lev)) return -4;
        else if (Is_firelevel(lev)) return -3;
        else if (Is_airlevel(lev)) return -2;
        else if (Is_earthlevel(lev)) return -1;
        else return 0;              /* ? */
    } else
#endif
    return depth(lev);
}

STATIC_OVL void
readentry(rfile, tt)
FILE *rfile;
struct toptenentry *tt;
{
#ifdef NO_SCAN_BRACK /* Version_ Pts DgnLevs_ Hp___ Died__Born id */
    static const char fmt[] = GAME_SHORT_NAME " %d %d %d %ld %d %d %d %d %d %d %ld %ld %d%*c";
    static const char fmt32[] = "%c%c %s %s%*c";
    static const char fmt33[] = "%s %s %s %s %s %s%*c";
#else
    static const char fmt[] = GAME_SHORT_NAME "-%d.%d.%d %ld %d %d %d %d %d %d %ld %ld %d ";
    static const char fmt32[] = "%c%c %[^,],%[^\n]%*c";
    static const char fmt33[] = "%s %s %s %s %[^,],%[^\n]%*c";
#endif

#ifdef UPDATE_RECORD_IN_PLACE
    /* note: fscanf() below must read the record's terminating newline */
    final_fpos = tt->fpos = ftell(rfile);
#endif
#define TTFIELDS 13
    if(fscanf(rfile, fmt,
              &tt->ver_major, &tt->ver_minor, &tt->patchlevel,
              &tt->points, &tt->deathdnum, &tt->deathlev,
              &tt->maxlvl, &tt->hp, &tt->maxhp, &tt->deaths,
              &tt->deathdate, &tt->birthdate,
              &tt->uid) != TTFIELDS)
#undef TTFIELDS
        tt->points = 0;
    else {
        /* Check for backwards compatibility */
        if (tt->ver_major < 3 ||
            (tt->ver_major == 3 && tt->ver_minor < 3)) {
            int i;

            if (fscanf(rfile, fmt32,
                       tt->plrole, tt->plgend,
                       tt->name, tt->death) != 4)
                tt->points = 0;
            tt->plrole[1] = '\0';
            if ((i = str2role(tt->plrole)) >= 0)
                Strcpy(tt->plrole, roles[i].filecode);
            Strcpy(tt->plrace, "?");
            Strcpy(tt->plgend, (tt->plgend[0] == 'M') ? "Mal" : "Fem");
            Strcpy(tt->plalign, "?");
        } else if (fscanf(rfile, fmt33,
                          tt->plrole, tt->plrace, tt->plgend,
                          tt->plalign, tt->name, tt->death) != 6)
            tt->points = 0;
#ifdef NO_SCAN_BRACK
        if(tt->points > 0) {
            nsb_unmung_line(tt->name);
            nsb_unmung_line(tt->death);
        }
#endif
    }

    /* check old score entries for Y2K problem and fix whenever found */
    if (tt->points > 0) {
        if (tt->birthdate < 19000000L) tt->birthdate += 19000000L;
        if (tt->deathdate < 19000000L) tt->deathdate += 19000000L;
    }
}

STATIC_OVL void
writeentry(rfile, tt)
FILE *rfile;
struct toptenentry *tt;
{
#ifdef NO_SCAN_BRACK
    nsb_mung_line(tt->name);
    nsb_mung_line(tt->death);
    /* Version_ Pts DgnLevs_ Hp___ Died__Born id */
    (void) fprintf(rfile, "%s %d %d %d %ld %d %d %d %d %d %d %ld %ld %d ",
#else
    (void) fprintf(rfile, "%s-%d.%d.%d %ld %d %d %d %d %d %d %ld %ld %d ",
#endif
                   GAME_SHORT_NAME,
                   tt->ver_major, tt->ver_minor, tt->patchlevel,
                   tt->points, tt->deathdnum, tt->deathlev,
                   tt->maxlvl, tt->hp, tt->maxhp, tt->deaths,
                   tt->deathdate, tt->birthdate, tt->uid);
    if (tt->ver_major < 3 ||
        (tt->ver_major == 3 && tt->ver_minor < 3))
#ifdef NO_SCAN_BRACK
        (void) fprintf(rfile, "%c%c %s %s\n",
#else
        (void) fprintf(rfile, "%c%c %s,%s\n",
#endif
                       tt->plrole[0], tt->plgend[0],
                       onlyspace(tt->name) ? "_" : tt->name, tt->death);
    else
#ifdef NO_SCAN_BRACK
        (void) fprintf(rfile, "%s %s %s %s %s %s\n",
#else
        (void) fprintf(rfile, "%s %s %s %s %s,%s\n",
#endif
                       tt->plrole, tt->plrace, tt->plgend, tt->plalign,
                       onlyspace(tt->name) ? "_" : tt->name, tt->death);

#ifdef NO_SCAN_BRACK
    nsb_unmung_line(tt->name);
    nsb_unmung_line(tt->death);
#endif
}

#ifdef XLOGFILE
#define SEP "\t"
#define SEPC '\t'

/* copy a maximum of n-1 characters from src to dest, changing ':' and '\n'
 * to '_'; always null-terminate. */
STATIC_OVL void
munge_xlstring(dest, src, n)
char *dest;
char *src;
int n;
{
    int i;

    for(i = 0; i < (n - 1) && src[i] != '\0'; i++) {
        if(src[i] == SEPC || src[i] == '\n')
            dest[i] = '_';
        else
            dest[i] = src[i];
    }

    dest[i] = '\0';

    return;
}

STATIC_OVL unsigned long
encode_uevent()
{
    unsigned long c = 0UL;

    /* game plot events */
    if (u.uevent.minor_oracle ||
        u.uevent.major_oracle) c |= 0x00001UL;      /* any Oracle consultation */
    if (u.uevent.qcalled) c |= 0x00002UL;           /* reached quest portal level */
    if (quest_status.got_quest ||
        quest_status.got_thanks) c |= 0x00004UL;    /* was accepted for quest */
    if (u.uevent.qcompleted) c |= 0x00008UL;        /* showed quest arti to leader */
    if (u.uevent.uopened_dbridge) c |= 0x00010UL;   /* opened/destroyed Castle drawbridge */
    if (u.uevent.gehennom_entered) c |= 0x00020UL;  /* entered Gehennom the front way */
    if (u.uevent.udemigod) c |= 0x00040UL;          /* provoked Rodney's wrath */
    if (u.uevent.invoked) c |= 0x00080UL;           /* did the invocation */
    if (u.uevent.ascended) c |= 0x00100UL;          /* someone needs to use this variable */

    /* notable other events */
#ifdef ELBERETH
    if (u.uevent.uhand_of_elbereth) c |= 0x00200UL; /* was crowned */
#endif

    /* boss kills */
    if (quest_status.killed_nemesis) c |= 0x00400UL; /* defeated quest nemesis */
    if (mvitals[PM_CROESUS].died) c |= 0x00800UL;   /* defeated Croesus */
    if (mvitals[PM_MEDUSA].died) c |= 0x01000UL;    /* defeated Medusa */
    if (mvitals[PM_VLAD_THE_IMPALER].
        died) c |= 0x02000UL;                       /* defeated Vlad */
    if (mvitals[PM_WIZARD_OF_YENDOR].
        died) c |= 0x04000UL;                       /* defeated Rodney */
    if (mvitals[PM_HIGH_PRIEST].died) c |= 0x08000UL; /* defeated a high priest */
    if (mvitals[PM_ONE_EYED_SAM].
        died) c |= 0x10000UL;                       /* defeated One-eyed Sam */
    if (mvitals[PM_CTHULHU].died) c |= 0x20000UL;   /* defeated Cthulhu */

    return c;
}

STATIC_OVL unsigned long
encode_carried()
{
    unsigned long c = 0UL;

    /* this encodes important items potentially owned by the player at the
       time of death */
    if (u.uhave.amulet) c |= 0x0001UL; /* real Amulet of Yendor */
    if (u.uhave.bell) c |= 0x0002UL;   /* Bell of Opening */
    if (u.uhave.book) c |= 0x0004UL;   /* Book of the Dead */
    if (u.uhave.menorah) c |= 0x0008UL; /* Candelabrum of Invocation */
    if (u.uhave.questart) c |= 0x0010UL; /* own quest artifact */

    return c;
}

STATIC_OVL void
write_xlentry(rfile, tt)
FILE *rfile;
struct toptenentry *tt;
{
    s_level *lev = Is_special(&u.uz);
    char buf[DTHSZ+1];

    /* Log all of the data found in the regular logfile */
    (void)fprintf(rfile,
                  "version=%s-%d.%d.%d"
                  SEP "points=%ld"
                  SEP "deathdnum=%d"
                  SEP "deathdname=%s"
                  SEP "deathlev=%d"
                  SEP "maxlvl=%d"
                  SEP "dlev_name=%s"
                  SEP "hp=%d"
                  SEP "maxhp=%d"
                  SEP "deaths=%d"
                  SEP "deathdate=%ld"
                  SEP "birthdate=%ld"
                  SEP "uid=%d",
                  GAME_SHORT_NAME,
                  tt->ver_major, tt->ver_minor, tt->patchlevel,
                  tt->points, tt->deathdnum, dungeons[tt->deathdnum].dname, tt->deathlev,
                  tt->maxlvl,
                  lev ? lev->proto : "", /* proto level name if special level */
                  tt->hp, tt->maxhp, tt->deaths,
                  tt->deathdate, tt->birthdate, tt->uid);

    (void)fprintf(rfile,
                  SEP "role=%s"
                  SEP "race=%s"
                  SEP "gender=%s"
                  SEP "align=%s",
                  tt->plrole, tt->plrace, tt->plgend, tt->plalign);

    munge_xlstring(buf, plname, DTHSZ + 1);
    (void)fprintf(rfile, SEP "name=%s", buf);

    munge_xlstring(buf, tt->death, DTHSZ + 1);
    (void)fprintf(rfile, SEP "death=%s", buf);

    (void)fprintf(rfile, SEP "flags=0x%lx", encode_xlogflags());

#ifdef RECORD_CONDUCT
    (void)fprintf(rfile, SEP "conduct=0x%lx", encodeconduct());
#endif

#ifdef RECORD_TURNS
    (void)fprintf(rfile, SEP "turns=%ld", moves);
#endif

#ifdef RECORD_ACHIEVE
    (void)fprintf(rfile, SEP "achieve=0x%lx", encodeachieve());
#endif
    (void)fprintf(rfile, SEP "killed_uniques=%s", killed_uniques());
    (void)fprintf(rfile, SEP "killed_nazgul=%d", mvitals[PM_NAZGUL].died);
    (void)fprintf(rfile, SEP "killed_erinyes=%d", mvitals[PM_ERINYS].died);
    (void)fprintf(rfile, SEP "killed_weeping_archangels=%d", mvitals[PM_WEEPING_ARCHANGEL].died);

    (void)fprintf(rfile, SEP "event=%ld", encode_uevent());
    (void)fprintf(rfile, SEP "carried=%ld", encode_carried());

    (void)fprintf(rfile, SEP "achieveX=%s", encode_extended_achievements());
    (void)fprintf(rfile, SEP "conductX=%s", encode_extended_conducts());
#ifdef RECORD_REALTIME
    (void)fprintf(rfile, SEP "realtime=%ld", (long)urealtime.realtime);
#endif

#ifdef RECORD_START_END_TIME
    (void)fprintf(rfile, SEP "starttime=%ld", (long)u.ubirthday);
    (void)fprintf(rfile, SEP "endtime=%ld", (long)u.udeathday);
#endif

#ifdef RECORD_GENDER0
    (void)fprintf(rfile, SEP "gender0=%s", genders[flags.initgend].filecode);
#endif

#ifdef RECORD_ALIGN0
    (void)fprintf(rfile, SEP "align0=%s",
                  aligns[1 - u.ualignbase[A_ORIGINAL]].filecode);
#endif
    (void)fprintf(rfile, SEP "elbereths=%ld", u.uconduct.elbereths);

    (void)fprintf(rfile, SEP "xplevel=%d", u.ulevel); /* XP level */
    (void)fprintf(rfile, SEP "exp=%ld", (long)u.uexp); /* Experience points */

    (void)fprintf(rfile, SEP "mode=%s", (flags.debug ? "debug" :
                                         flags.explore ? "explore" :
                                         hell_and_hell_mode ? "hah" :
                                         heaven_or_hell_mode ? "hoh" :
                                         marathon_mode ? "marathon" :
                                         "normal"));
    (void)fprintf(rfile, SEP "gold=%ld", money_cnt(invent) + hidden_gold());
    (void)fprintf(rfile, SEP "wish_cnt=%ld", u.uconduct.wishes);
    (void)fprintf(rfile, SEP "magic_wish_cnt=%ld", u.uconduct.wishmagic);
    (void)fprintf(rfile, SEP "arti_wish_cnt=%ld", u.uconduct.wisharti);
    (void)fprintf(rfile, SEP "bones=%ld", u.uconduct.bones);
    (void)fprintf(rfile, SEP "rngseed=u%s", encode_base32(level_info[0].seed));
    (void)fprintf(rfile, SEP "user_seed=%u", is_game_pre_seeded ? 1 : 0);

    (void)fprintf(rfile, "\n");

}

/* add the achievement or conduct comma-separated to string */
static void
add_achieveX(buf, achievement, condition)
char *buf;
const char *achievement;
boolean condition;
{
    if (condition) {
        if (buf[0] != '\0') {
            Strcat(buf, ",");
        }
        Strcat(buf, achievement);
    }
}

static char *
encode_extended_achievements()
{
    static char buf[30*40];

    buf[0] = '\0';
    /* the original 12 xlogfile achievements */
    add_achieveX(buf, "ascended", u.uevent.ascended);
    add_achieveX(buf, "entered_astral_plane", Is_astralevel(&u.uz));
    add_achieveX(buf, "entered_elemental_planes", In_endgame(&u.uz));
    add_achieveX(buf, "obtained_the_amulet_of_yendor", achieve.get_amulet);
    add_achieveX(buf, "performed_the_invocation_ritual", achieve.perform_invocation);
    add_achieveX(buf, "obtained_the_book_of_the_dead", achieve.get_book);
    add_achieveX(buf, "obtained_the_bell_of_opening", achieve.get_bell);
    add_achieveX(buf, "obtained_the_candelabrum_of_invocation", achieve.get_candelabrum);
    add_achieveX(buf, "entered_gehennom", achieve.enter_gehennom);
    add_achieveX(buf, "defeated_medusa", achieve.killed_medusa);
    add_achieveX(buf, "obtained_the_luckstone_from_the_mines", achieve.get_luckstone);
    add_achieveX(buf, "obtained_the_sokoban_prize", achieve.finish_sokoban);

    /* oracle */
    add_achieveX(buf, "consulted_the_oracle", (u.uevent.minor_oracle || u.uevent.major_oracle));
    add_achieveX(buf, "got_minor_oracle_consultation", u.uevent.minor_oracle);
    add_achieveX(buf, "got_major_oracle_consultation", u.uevent.major_oracle);

    /* quest */
    add_achieveX(buf, "entered_quest_portal_level", u.uevent.qcalled);
    add_achieveX(buf, "accepted_for_quest", quest_status.got_quest);
    add_achieveX(buf, "defeated_quest_nemesis", quest_status.killed_nemesis);
    add_achieveX(buf, "quest_completed", u.uevent.qcompleted);

    /* other notable achievements */
    add_achieveX(buf, "entered_gehennom_the_front_way", u.uevent.gehennom_entered);
    add_achieveX(buf, "opened_castle_drawbridge", u.uevent.uopened_dbridge);
    add_achieveX(buf, "got_crowned", u.uevent.uhand_of_elbereth);

    add_achieveX(buf, "entered_the_gnomish_mines", u.uevent.entered_the_gnomish_mines);
    add_achieveX(buf, "entered_mine_town", u.uevent.entered_mine_town);
    add_achieveX(buf, "entered_sokoban", u.uevent.entered_sokoban);
    add_achieveX(buf, "entered_bigroom", u.uevent.entered_the_bigroom);
    add_achieveX(buf, "entered_the_town", u.uevent.entered_the_town);
    add_achieveX(buf, "entered_the_blackmarket", u.uevent.entered_the_blackmarket);
    add_achieveX(buf, "entered_moria", u.uevent.entered_moria);
    add_achieveX(buf, "entered_the_dragon_caves", u.uevent.entered_the_dragon_caves);
    add_achieveX(buf, "entered_sheol", u.uevent.entered_sheol);
    add_achieveX(buf, "entered_vlads_tower", u.uevent.entered_vlads_tower);
    add_achieveX(buf, "entered_fort_ludios", u.uevent.entered_fort_ludios);

    add_achieveX(buf, "entered_a_shop", u.uevent.entered_a_shop);
    add_achieveX(buf, "entered_a_temple", u.uevent.entered_a_temple);

    return buf;
}

static char *
encode_extended_conducts()
{
    static char buf[BUFSZ];

    buf[0] = '\0';
    add_achieveX(buf, "foodless",     !u.uconduct.food);
    add_achieveX(buf, "vegan",        !u.uconduct.unvegan);
    add_achieveX(buf, "vegetarian",   !u.uconduct.unvegetarian);
    add_achieveX(buf, "atheist",      !u.uconduct.gnostic);
    add_achieveX(buf, "weaponless",   !u.uconduct.weaphit);
    add_achieveX(buf, "pacifist",     !u.uconduct.killer);
    add_achieveX(buf, "illiterate",   !u.uconduct.literate);
    add_achieveX(buf, "polyless",     !u.uconduct.polypiles);
    add_achieveX(buf, "polyselfless", !u.uconduct.polyselfs);
    add_achieveX(buf, "wishless",     !u.uconduct.wishes);
    add_achieveX(buf, "artiwishless", !u.uconduct.wisharti);
    add_achieveX(buf, "genocideless", !num_genocides());
    add_achieveX(buf, "sokoban",      !u.uconduct.sokoban);
    add_achieveX(buf, "blindfolded",   u.roleplay.blindfolded);
    add_achieveX(buf, "nudist",        u.roleplay.nudist);
    add_achieveX(buf, "bonesless",    !flags.bones);
    add_achieveX(buf, "elberethless",  !u.uconduct.elbereths);
    add_achieveX(buf, "deactivated_elbereth", flags.elberethignore);

    return buf;
}

#undef SEP
#undef SEPC
#endif /* XLOGFILE */

STATIC_OVL void
free_ttlist(tt)
struct toptenentry *tt;
{
    struct toptenentry *ttnext;

    while (tt->points > 0) {
        ttnext = tt->tt_next;
        dealloc_ttentry(tt);
        tt = ttnext;
    }
    dealloc_ttentry(tt);
}

void
topten(how)
int how;
{
    int uid = getuid();
    int rank, rank0 = -1, rank1 = 0;
    int occ_cnt = PERSMAX;
    register struct toptenentry *t0, *tprev;
    struct toptenentry *t1;
    FILE *rfile;
    register int flg = 0;
    boolean t0_used;
#ifdef LOGFILE
    FILE *lfile;
#endif /* LOGFILE */
#ifdef XLOGFILE
    FILE *xlfile;
#endif /* XLOGFILE */

/* Under DICE 3.0, this crashes the system consistently, apparently due to
 * corruption of *rfile somewhere.  Until I figure this out, just cut out
 * topten support entirely - at least then the game exits cleanly.  --AC
 */
#ifdef _DCC
    return;
#endif

/* If we are in the midst of a panic, cut out topten entirely.
 * topten uses alloc() several times, which will lead to
 * problems if the panic was the result of an alloc() failure.
 */
    if (program_state.panicking)
        return;

    if (flags.toptenwin) {
        toptenwin = create_nhwindow(NHW_TEXT);
    }

#if defined(UNIX) || defined(VMS) || defined(__EMX__)
#define HUP if (!program_state.done_hup)
#else
#define HUP
#endif

#ifdef TOS
    restore_colors();   /* make sure the screen is black on white */
#endif
    /* create a new 'topten' entry */
    t0_used = FALSE;
    t0 = newttentry();
    /* deepest_lev_reached() is in terms of depth(), and reporting the
     * deepest level reached in the dungeon death occurred in doesn't
     * seem right, so we have to report the death level in depth() terms
     * as well (which also seems reasonable since that's all the player
     * sees on the screen anyway)
     */
    t0->ver_major = VERSION_MAJOR;
    t0->ver_minor = VERSION_MINOR;
    t0->patchlevel = PATCHLEVEL;
    t0->points = u.urscore;
    t0->deathdnum = u.uz.dnum;
    t0->deathlev = observable_depth(&u.uz);
    t0->maxlvl = deepest_lev_reached(TRUE);
    t0->hp = u.uhp;
    t0->maxhp = u.uhpmax;
    t0->deaths = u.umortality;
    t0->uid = uid;
    (void) strncpy(t0->plrole, urole.filecode, ROLESZ);
    t0->plrole[ROLESZ] = '\0';
    (void) strncpy(t0->plrace, urace.filecode, ROLESZ);
    t0->plrace[ROLESZ] = '\0';
    (void) strncpy(t0->plgend, genders[flags.female].filecode, ROLESZ);
    t0->plgend[ROLESZ] = '\0';
    (void) strncpy(t0->plalign, aligns[1-u.ualign.type].filecode, ROLESZ);
    t0->plalign[ROLESZ] = '\0';
    (void) strncpy(t0->name, plname, NAMSZ);
    t0->name[NAMSZ] = '\0';
    t0->death[0] = '\0';
    switch (killer.format) {
    case KILLED_BY_AN:
        Strcat(t0->death, killed_by_prefix[how]);
        (void) strncat(t0->death, an(killer.name),
                       DTHSZ-strlen(t0->death));
        break;
    case KILLED_BY:
        Strcat(t0->death, killed_by_prefix[how]);
        (void) strncat(t0->death, killer.name,
                       DTHSZ-strlen(t0->death));
        break;
    case NO_KILLER_PREFIX:
        (void) strncat(t0->death, killer.name, DTHSZ);
        break;
    default: warning("bad killer format?");
    }
    t0->birthdate = yyyymmdd(u.ubirthday);
    t0->deathdate = yyyymmdd(u.udeathday);

    t0->tt_next = 0;
#ifdef UPDATE_RECORD_IN_PLACE
    t0->fpos = -1L;
#endif

#ifdef LOGFILE      /* used for debugging (who dies of what, where) */
#ifdef FILE_AREAS
    if (lock_file_area(LOGAREA, LOGFILE, 10)) {
#else
    if (lock_file(LOGFILE, SCOREPREFIX, 10)) {
#endif
        if(!(lfile = fopen_datafile_area(LOGAREA, LOGFILE, "a", SCOREPREFIX))) {
            HUP raw_print("Cannot open log file!");
        } else {
            writeentry(lfile, t0);
            (void) fclose(lfile);
        }
        unlock_file_area(LOGAREA, LOGFILE);
    }
#endif /* LOGFILE */

#ifdef XLOGFILE
#ifdef FILE_AREAS
    if (lock_file_area(LOGAREA, XLOGFILE, 10)) {
#else
    if (lock_file(XLOGFILE, SCOREPREFIX, 10)) {
#endif
        if(!(xlfile = fopen_datafile_area(LOGAREA, XLOGFILE, "a", SCOREPREFIX))) {
            HUP raw_print("Cannot open extended log file!");
        } else {
            write_xlentry(xlfile, t0);
            (void) fclose(xlfile);
        }
        unlock_file_area(LOGAREA, XLOGFILE);
    }
#endif /* XLOGFILE */

    if (wizard || discover) {
        if (how != PANICKED) HUP {
                char pbuf[BUFSZ];
                topten_print("");
                Sprintf(pbuf,
                        "Since you were in %s mode, the score list will not be checked.",
                        wizard ? "wizard" : "discover");
                topten_print(pbuf);
#ifdef DUMP_LOG
                if (dump_fn[0]) {
                    dump("", pbuf);
                    dump("", "");
                }
#endif
            }
            goto showwin;
    }

#ifdef FILE_AREAS
    if (!lock_file_area(NH_RECORD_AREA, RECORD, 60))
#else
    if (!lock_file(RECORD, SCOREPREFIX, 60))
#endif
        goto destroywin;

#ifdef UPDATE_RECORD_IN_PLACE
    rfile = fopen_datafile_area(NH_RECORD_AREA, RECORD, "r+", SCOREPREFIX);
#else
    rfile = fopen_datafile_area(NH_RECORD_AREA, RECORD, "r", SCOREPREFIX);
#endif

    if (!rfile) {
        HUP raw_print("Cannot open record file!");
        unlock_file_area(NH_RECORD_AREA, RECORD);
        goto destroywin;
    }

    HUP topten_print("");
#ifdef DUMP_LOG
    dump("", "");
#endif

    /* assure minimum number of points */
    if(t0->points < POINTSMIN) t0->points = 0;

    t1 = tt_head = newttentry();
    tprev = 0;
    /* rank0: -1 undefined, 0 not_on_list, n n_th on list */
    for(rank = 1; ; ) {
        readentry(rfile, t1);
        if (t1->points < POINTSMIN) t1->points = 0;
        if(rank0 < 0 && t1->points < t0->points) {
            rank0 = rank++;
            if(tprev == 0)
                tt_head = t0;
            else
                tprev->tt_next = t0;
            t0->tt_next = t1;
#ifdef UPDATE_RECORD_IN_PLACE
            t0->fpos = t1->fpos; /* insert here */
#endif
            t0_used = TRUE;
            occ_cnt--;
            flg++;  /* ask for a rewrite */
        } else tprev = t1;

        if(t1->points == 0) break;
        if(
#ifdef PERS_IS_UID
            t1->uid == t0->uid &&
#else
            strncmp(t1->name, t0->name, NAMSZ) == 0 &&
#endif
            !strncmp(t1->plrole, t0->plrole, ROLESZ) &&
            --occ_cnt <= 0) {
            if(rank0 < 0) {
                rank0 = 0;
                rank1 = rank;
                HUP {
                    char pbuf[BUFSZ];
                    Sprintf(pbuf,
                            "You didn't beat your previous score of %ld points.",
                            t1->points);
                    topten_print(pbuf);
                    topten_print("");
#ifdef DUMP_LOG
                    dump("", pbuf);
                    dump("", "");
#endif
                }
            }
            if(occ_cnt < 0) {
                flg++;
                continue;
            }
        }
        if(rank <= ENTRYMAX) {
            t1->tt_next = newttentry();
            t1 = t1->tt_next;
            rank++;
        }
        if(rank > ENTRYMAX) {
            t1->points = 0;
            break;
        }
    }
    if(flg) {   /* rewrite record file */
#ifdef UPDATE_RECORD_IN_PLACE
        (void) fseek(rfile, (t0->fpos >= 0 ?
                             t0->fpos : final_fpos), SEEK_SET);
#else
        (void) fclose(rfile);
        if(!(rfile = fopen_datafile_area(NH_RECORD_AREA, RECORD, "w", SCOREPREFIX))) {
            HUP raw_print("Cannot write record file");
            unlock_file_area(NH_RECORD_AREA, RECORD);
            free_ttlist(tt_head);
            goto destroywin;
        }
#endif  /* UPDATE_RECORD_IN_PLACE */
        if(rank0 > 0) {
            if(rank0 <= 10) {
                if(!done_stopprint)
                    topten_print("You made the top ten list!");
#ifdef DUMP_LOG
                dump("", "You made the top ten list!");
#endif
            } else {
                char pbuf[BUFSZ];
                Sprintf(pbuf,
                        "You reached the %d%s place on the top %d list.",
                        rank0, ordin(rank0), ENTRYMAX);
                if(!done_stopprint) topten_print(pbuf);
#ifdef DUMP_LOG
                dump("", pbuf);
#endif
            }
            if(!done_stopprint) topten_print("");
#ifdef DUMP_LOG
            dump("", "");
#endif
        }
    }
#ifdef DUMP_LOG
    dump_html("<pre>", "");
#endif
    if(rank0 == 0) rank0 = rank1;
    if(rank0 <= 0) rank0 = rank;
    if(!done_stopprint) outheader();
    t1 = tt_head;
    for(rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
        if(flg
#ifdef UPDATE_RECORD_IN_PLACE
           && rank >= rank0
#endif
           ) writeentry(rfile, t1);
        /* if (done_stopprint) continue; */
        if (rank > flags.end_top &&
            (rank < rank0 - flags.end_around ||
             rank > rank0 + flags.end_around) &&
            (!flags.end_own ||
#ifdef PERS_IS_UID
             t1->uid != t0->uid
#else
             strncmp(t1->name, t0->name, NAMSZ)
#endif
            )) continue;
        if (rank == rank0 - flags.end_around &&
            rank0 > flags.end_top + flags.end_around + 1 &&
            !flags.end_own) {
            if(!done_stopprint) topten_print("");
#ifdef DUMP_LOG
            dump("", "");
#endif
        }
        if(rank != rank0)
            outentry(rank, t1, FALSE);
        else if(!rank1)
            outentry(rank, t1, TRUE);
        else {
            outentry(rank, t1, TRUE);
            outentry(0, t0, TRUE);
        }
    }
    if(rank0 >= rank) if(!done_stopprint)
            outentry(0, t0, TRUE);
#ifdef DUMP_LOG
    dump_html("</pre>", "");
#endif
#ifdef UPDATE_RECORD_IN_PLACE
    if (flg) {
# ifdef TRUNCATE_FILE
        /* if a reasonable way to truncate a file exists, use it */
        truncate_file(rfile);
# else
        /* use sentinel record rather than relying on truncation */
        t1->points = 0L;    /* terminates file when read back in */
        t1->ver_major = t1->ver_minor = t1->patchlevel = 0;
        t1->uid = t1->deathdnum = t1->deathlev = 0;
        t1->maxlvl = t1->hp = t1->maxhp = t1->deaths = 0;
        t1->plrole[0] = t1->plrace[0] = t1->plgend[0] = t1->plalign[0] = '-';
        t1->plrole[1] = t1->plrace[1] = t1->plgend[1] = t1->plalign[1] = 0;
        t1->birthdate = t1->deathdate = yyyymmdd((time_t)0L);
        Strcpy(t1->name, "@");
        Strcpy(t1->death, "<eod>\n");
        writeentry(rfile, t1);
        (void) fflush(rfile);
# endif /* TRUNCATE_FILE */
    }
#endif  /* UPDATE_RECORD_IN_PLACE */
    (void) fclose(rfile);
    unlock_file_area(NH_RECORD_AREA, RECORD);
    free_ttlist(tt_head);

showwin:
    if (flags.toptenwin && !done_stopprint) display_nhwindow(toptenwin, 1);
destroywin:
    if (!t0_used) dealloc_ttentry(t0);
    if (flags.toptenwin) {
        destroy_nhwindow(toptenwin);
        toptenwin=WIN_ERR;
    }
}

STATIC_OVL void
outheader()
{
    char linebuf[BUFSZ];
    register char *bp;

    Strcpy(linebuf, " No  Points     Name");
    bp = eos(linebuf);
    while(bp < linebuf + COLNO - 9) *bp++ = ' ';
    Strcpy(bp, "Hp [max]");
    if(!done_stopprint) topten_print(linebuf);
#ifdef DUMP_LOG
    dump("", linebuf);
#endif
}

/* so>0: standout line; so=0: ordinary line */
STATIC_OVL void
outentry(rank, t1, so)
struct toptenentry *t1;
int rank;
boolean so;
{
    boolean second_line = TRUE;
    char linebuf[BUFSZ];
    char *bp, hpbuf[24], linebuf3[BUFSZ];
    int hppos, lngr;
    boolean nospacefound = FALSE;


    linebuf[0] = '\0';
    if (rank) Sprintf(eos(linebuf), "%3d", rank);
    else Strcat(linebuf, "   ");

    Sprintf(eos(linebuf), " %10ld  %.10s", t1->points, t1->name);
    Sprintf(eos(linebuf), "-%s", t1->plrole);
    if (t1->plrace[0] != '?')
        Sprintf(eos(linebuf), "-%s", t1->plrace);
    /* Printing of gender and alignment is intentional.  It has been
     * part of the NetHack Geek Code, and illustrates a proper way to
     * specify a character from the command line.
     */
    Sprintf(eos(linebuf), "-%s", t1->plgend);
    if (t1->plalign[0] != '?')
        Sprintf(eos(linebuf), "-%s ", t1->plalign);
    else
        Strcat(linebuf, " ");
#ifdef ASTRAL_ESCAPE
    if (!strncmp("defied", t1->death, 6)) {
        Sprintf(eos(linebuf), "defied the Gods and escaped the dungeon %s",
                !strncmp(" (", t1->death + 7, 2) ? t1->death + 7 + 2 : "");
        second_line = FALSE;
    } else if (!strncmp("escaped", t1->death, 7)) {
#endif
        Sprintf(eos(linebuf), "escaped the dungeon %s[max level %d]",
                !strncmp(" (", t1->death + 7, 2) ? t1->death + 7 + 2 : "",
                t1->maxlvl);
        /* fixup for closing paren in "escaped... with...Amulet)[max..." */
        if ((bp = index(linebuf, ')')) != 0)
            *bp = (t1->deathdnum == astral_level.dnum) ? '\0' : ' ';
        second_line = FALSE;
    } else if (!strncmp("ascended ", t1->death, 9)) {
        Strcat(linebuf, "the ");
        Strcat(linebuf, t1->death + 9);
        Sprintf(eos(linebuf), " ascended to demigod%s-hood",
                (t1->plgend[0] == 'F') ? "dess" : "");
        second_line = FALSE;
    } else if (!strncmp("ascended", t1->death, 8)) {
        Sprintf(eos(linebuf), "ascended to demigod%s-hood",
                (t1->plgend[0] == 'F') ? "dess" : "");
        second_line = FALSE;
    } else {
        if (!strncmp(t1->death, "quit", 4)) {
            Strcat(linebuf, "quit");
            second_line = FALSE;
        } else if (!strncmp(t1->death, "died of st", 10)) {
            Strcat(linebuf, "starved to death");
            second_line = FALSE;
        } else if (!strncmp(t1->death, "choked", 6)) {
            Sprintf(eos(linebuf), "choked on h%s food",
                    (t1->plgend[0] == 'F') ? "er" : "is");
        } else if (!strncmp(t1->death, "poisoned", 8)) {
            Strcat(linebuf, "was poisoned");
        } else if (!strncmp(t1->death, "crushed", 7)) {
            Strcat(linebuf, "was crushed to death");
        } else if (!strncmp(t1->death, "petrified by ", 13)) {
            Strcat(linebuf, "turned to stone");
        } else Strcat(linebuf, "died");

        if (t1->deathdnum == astral_level.dnum) {
            const char *arg, *fmt = " on the Plane of %s";

            switch (t1->deathlev) {
            case -5:
                fmt = " on the %s Plane";
                arg = "Astral"; break;
            case -4:
                arg = "Water";  break;
            case -3:
                arg = "Fire";   break;
            case -2:
                arg = "Air";    break;
            case -1:
                arg = "Earth";  break;
            default:
                arg = "Void";   break;
            }
            Sprintf(eos(linebuf), fmt, arg);
        } else {
            Sprintf(eos(linebuf), " in %s", dungeons[t1->deathdnum].dname);
            if (t1->deathdnum != knox_level.dnum)
                Sprintf(eos(linebuf), " on level %d", t1->deathlev);
            if (t1->deathlev != t1->maxlvl)
                Sprintf(eos(linebuf), " [max %d]", t1->maxlvl);
        }

        /* kludge for "quit while already on Charon's boat" */
        /* and "quit after breaking pacifism conduct"   */
        if (!strncmp(t1->death, "quit ", 5))
            Strcat(linebuf, t1->death + 4);
    }
    Strcat(linebuf, ".");

    /* Quit, starved, ascended, and escaped contain no second line */
    if (second_line)
        Sprintf(eos(linebuf), "  %c%s.", highc(*(t1->death)), t1->death+1);

    lngr = (int)strlen(linebuf);
    if (t1->hp <= 0) hpbuf[0] = '-', hpbuf[1] = '\0';
    else Sprintf(hpbuf, "%d", t1->hp);
    /* beginning of hp column after padding (not actually padded yet) */
    hppos = COLNO - (sizeof("  Hp [max]")-1); /* sizeof(str) includes \0 */
    while (lngr >= hppos) {
        for(bp = eos(linebuf);
            !(*bp == ' ' && (bp-linebuf < hppos));
            bp--)
            ;
        /* check for un-wrappable word */
        if (linebuf+15 == bp) {
            bp += hppos-15;
            nospacefound = TRUE;
        }
        /* special case: if about to wrap in the middle of maximum
           dungeon depth reached, wrap in front of it instead */
        if (bp > linebuf + 5 && !strncmp(bp - 5, " [max", 5)) bp -= 5;
        if (nospacefound) {
            /* word wrap in the middle of a long word */
            Strcpy(linebuf3, bp);
            nospacefound = FALSE;
        } else {
            /* word wrap on space */
            Strcpy(linebuf3, bp+1);
        }
        *bp = 0;
        if (so) {
            while (bp < linebuf + (COLNO-1)) *bp++ = ' ';
            *bp = 0;
            if(!done_stopprint) topten_print_bold(linebuf);
#ifdef DUMP_LOG
            dump("*", linebuf[0]==' ' ? linebuf+1 : linebuf);
#endif
        } else {
            if(!done_stopprint) topten_print(linebuf);
#ifdef DUMP_LOG
            dump(" ", linebuf[0]==' ' ? linebuf+1 : linebuf);
#endif
        }
        Sprintf(linebuf, "%15s %s", "", linebuf3);
        lngr = strlen(linebuf);
    }
    /* beginning of hp column not including padding */
    hppos = COLNO - 7 - (int)strlen(hpbuf);
    bp = eos(linebuf);

    if (bp <= linebuf + hppos) {
        /* pad any necessary blanks to the hit point entry */
        while (bp < linebuf + hppos) *bp++ = ' ';
        Strcpy(bp, hpbuf);
        Sprintf(eos(bp), " %s[%d]",
                (t1->maxhp < 10) ? "  " : (t1->maxhp < 100) ? " " : "",
                t1->maxhp);
    }

    if (so) {
        bp = eos(linebuf);
        if (so >= COLNO) so = COLNO-1;
        while (bp < linebuf + so) *bp++ = ' ';
        *bp = 0;
        if(!done_stopprint) topten_print_bold(linebuf);
    } else
    if(!done_stopprint) topten_print(linebuf);
#ifdef DUMP_LOG
    dump(" ", linebuf[0]==' ' ? linebuf+1 : linebuf);
#endif
}

STATIC_OVL int
score_wanted(current_ver, rank, t1, playerct, players, uid)
boolean current_ver;
int rank;
struct toptenentry *t1;
int playerct;
const char **players;
int uid;
{
    int i;

    if (current_ver && (t1->ver_major != VERSION_MAJOR ||
                        t1->ver_minor != VERSION_MINOR ||
                        t1->patchlevel != PATCHLEVEL))
        return 0;

#ifdef PERS_IS_UID
    if (!playerct && t1->uid == uid)
        return 1;
#else
    nhUse(uid);
#endif

    for (i = 0; i < playerct; i++) {
        if (players[i][0] == '-' && index("pr", players[i][1]) &&
            players[i][2] == 0 && i + 1 < playerct) {
            char *arg = (char *)players[i + 1];
            if ((players[i][1] == 'p' &&
                 str2role(arg) == str2role(t1->plrole)) ||
                (players[i][1] == 'r' &&
                 str2race(arg) == str2race(t1->plrace)))
                return 1;
            i++;
        } else if (strcmp(players[i], "all") == 0 ||
                   strncmp(t1->name, players[i], NAMSZ) == 0 ||
                   (players[i][0] == '-' &&
                    players[i][1] == t1->plrole[0] &&
                    players[i][2] == 0) ||
                   (digit(players[i][0]) && rank <= atoi(players[i])))
            return 1;
    }
    return 0;
}

long
encode_xlogflags(void)
{
    long e = 0L;

    if (wizard) e |= 0x001L;              /* wizard mode */
    if (discover) e |= 0x002L;            /* explore mode */
    if (killer_flags & 0x1) e |= 0x004L;  /* died, (with the Amulet) */
    if (killer_flags & 0x2) e |= 0x008L;  /* died, (in celestial disgrace) */
    if (killer_flags & 0x4) e |= 0x010L;  /* died, (with a fake Amulet) */

    return e;
}

#ifdef RECORD_CONDUCT
long
encodeconduct(void)
{
    long e = 0L;

    if(!u.uconduct.food) e |= 0x001L;
    if(!u.uconduct.unvegan) e |= 0x002L;
    if(!u.uconduct.unvegetarian) e |= 0x004L;
    if(!u.uconduct.gnostic) e |= 0x008L;
    if(!u.uconduct.weaphit) e |= 0x010L;
    if(!u.uconduct.killer) e |= 0x020L;
    if(!u.uconduct.literate) e |= 0x040L;
    if(!u.uconduct.polypiles) e |= 0x080L;
    if(!u.uconduct.polyselfs) e |= 0x100L;
    if(!u.uconduct.wishes) e |= 0x200L;
    if(!u.uconduct.wisharti) e |= 0x400L;
    if(!num_genocides()) e |= 0x800L;

    return e;
}
#endif

#ifdef RECORD_ACHIEVE
long
encodeachieve(void)
{
    /* Achievement bitfield:
     * bit  meaning
     *  0   obtained the Bell of Opening
     *  1   entered gehennom (by any means)
     *  2   obtained the Candelabrum of Invocation
     *  3   obtained the Book of the Dead
     *  4   performed the invocation ritual
     *  5   obtained the amulet
     *  6   entered elemental planes
     *  7   entered astral plane
     *  8   ascended (not "escaped in celestial disgrace!")
     *  9   obtained the luckstone from the Mines
     *  10  obtained the sokoban prize
     *  11  killed medusa
     */

    long r;

    r = 0;

    if(achieve.get_bell) r |= 1L << 0;
    if(achieve.enter_gehennom) r |= 1L << 1;
    if(achieve.get_candelabrum) r |= 1L << 2;
    if(achieve.get_book) r |= 1L << 3;
    if(achieve.perform_invocation) r |= 1L << 4;
    if(achieve.get_amulet) r |= 1L << 5;
    if(In_endgame(&u.uz)) r |= 1L << 6;
    if(Is_astralevel(&u.uz)) r |= 1L << 7;
    if(achieve.ascended) r |= 1L << 8;
    if(achieve.get_luckstone) r |= 1L << 9;
    if(achieve.finish_sokoban) r |= 1L << 10;
    if(achieve.killed_medusa) r |= 1L << 11;

    return r;
}
#endif

static char _killed_uniques[640]; /* 640 characters ought to be enough for anybody */
static char*
killed_uniques(void)
{
    _killed_uniques[0] = '\0';

    int i;
    for (i = LOW_PM; i < NUMMONS; i++) {
        if ((mons[i].geno & G_UNIQ) && mvitals[i].died) {
            if (i == PM_LONG_WORM_TAIL) continue;
            if (i == PM_HIGH_PRIEST) continue;
            Sprintf(eos(_killed_uniques), "%s,", mons[i].mname);
        }
    }

    int len;
    if ((len=strlen(_killed_uniques))) { _killed_uniques[len-1] = '\0'; }

    return _killed_uniques;
}

/*
 * print selected parts of score list.
 * argc >= 2, with argv[0] untrustworthy (directory names, et al.),
 * and argv[1] starting with "-s".
 */
void
prscore(argc, argv)
int argc;
char **argv;
{
    const char **players;
    int playerct, rank;
    boolean current_ver = TRUE, init_done = FALSE;
    register struct toptenentry *t1;
    FILE *rfile;
    boolean match_found = FALSE;
    register int i;
    char pbuf[BUFSZ];
    int uid = -1;
#ifndef PERS_IS_UID
    const char *player0;
#endif

    if (argc < 2 || strncmp(argv[1], "-s", 2)) {
        raw_printf("prscore: bad arguments (%d)", argc);
        return;
    }

    rfile = fopen_datafile_area(NH_RECORD_AREA, RECORD, "r", SCOREPREFIX);
    if (!rfile) {
        raw_print("Cannot open record file!");
        return;
    }

#ifdef  AMIGA
    {
        extern winid amii_rawprwin;
        init_nhwindows(&argc, argv);
        amii_rawprwin = create_nhwindow(NHW_TEXT);
    }
#endif

    /* If the score list isn't after a game, we never went through
     * initialization. */
    if (wiz1_level.dlevel == 0) {
        dlb_init();
        init_dungeons();
        init_done = TRUE;
    }

    if (!argv[1][2]) {   /* plain "-s" */
        argc--;
        argv++;
    } else argv[1] += 2;

    if (argc > 1 && !strcmp(argv[1], "-v")) {
        current_ver = FALSE;
        argc--;
        argv++;
    }

    if (argc <= 1) {
#ifdef PERS_IS_UID
        uid = getuid();
        playerct = 0;
        players = (const char **)0;
#else
        player0 = plname;
        if (!*player0)
# ifdef AMIGA
            player0 = "all";    /* single user system */
# else
            player0 = "hackplayer";
# endif
        playerct = 1;
        players = &player0;
#endif
    } else {
        playerct = --argc;
        players = (const char **)++argv;
    }
    raw_print("");

    t1 = tt_head = newttentry();
    for (rank = 1; ; rank++) {
        readentry(rfile, t1);
        if (t1->points == 0) break;
        if (!match_found &&
            score_wanted(current_ver, rank, t1, playerct, players, uid))
            match_found = TRUE;
        t1->tt_next = newttentry();
        t1 = t1->tt_next;
    }

    (void) fclose(rfile);
    if (init_done) {
        free_dungeons();
        dlb_cleanup();
    }

    if (match_found) {
        outheader();
        t1 = tt_head;
        for (rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
            if (score_wanted(current_ver, rank, t1, playerct, players, uid))
                (void) outentry(rank, t1, 0);
        }
    } else {
        Sprintf(pbuf, "Cannot find any %sentries for ",
                current_ver ? "current " : "");
        if (playerct < 1) Strcat(pbuf, "you.");
        else {
            if (playerct > 1) Strcat(pbuf, "any of ");
            for (i = 0; i < playerct; i++) {
                /* stop printing players if there are too many to fit */
                if (strlen(pbuf) + strlen(players[i]) + 2 >= BUFSZ) {
                    if (strlen(pbuf) < BUFSZ-4) Strcat(pbuf, "...");
                    else Strcpy(pbuf+strlen(pbuf)-4, "...");
                    break;
                }
                Strcat(pbuf, players[i]);
                if (i < playerct-1) {
                    if (players[i][0] == '-' &&
                        index("pr", players[i][1]) && players[i][2] == 0)
                        Strcat(pbuf, " ");
                    else Strcat(pbuf, ":");
                }
            }
        }
        raw_print(pbuf);
        raw_printf("Usage: %s -s [-v] <playertypes> [maxrank] [playernames]",

                   hname);
        raw_printf("Player types are: [-p role] [-r race]");
    }
    free_ttlist(tt_head);
#ifdef  AMIGA
    {
        extern winid amii_rawprwin;
        display_nhwindow(amii_rawprwin, 1);
        destroy_nhwindow(amii_rawprwin);
        amii_rawprwin = WIN_ERR;
    }
#endif
}

STATIC_OVL int
classmon(plch, fem)
char *plch;
boolean fem;
{
    int i;

    /* Look for this role in the role table */
    for (i = 0; roles[i].name.m; i++)
        if (!strncmp(plch, roles[i].filecode, ROLESZ)) {
            if (fem && roles[i].femalenum != NON_PM)
                return roles[i].femalenum;
            else if (roles[i].malenum != NON_PM)
                return roles[i].malenum;
            else
                return PM_HUMAN;
        }
    /* this might be from a 3.2.x score for former Elf class */
    if (!strcmp(plch, "E")) return PM_RANGER;

    warning("What weird role is this? (%s)", plch);
    return (PM_HUMAN_MUMMY);
}

/*
 * Get a random player name and class from the high score list,
 * and attach them to an object (for statues or morgue corpses).
 */
struct obj *
tt_oname(otmp)
struct obj *otmp;
{
    int rank;
    register int i;
    register struct toptenentry *tt;
    FILE *rfile;
    struct toptenentry tt_buf;

    if (!otmp) return((struct obj *) 0);

    rfile = fopen_datafile_area(NH_RECORD_AREA, RECORD, "r", SCOREPREFIX);
    if (!rfile) {
        impossible("Cannot open record file!");
        return (struct obj *)0;
    }

    tt = &tt_buf;
    rank = rnd(10);
pickentry:
    for(i = rank; i; i--) {
        readentry(rfile, tt);
        if(tt->points == 0) break;
    }

    if(tt->points == 0) {
        if(rank > 1) {
            rank = 1;
            rewind(rfile);
            goto pickentry;
        }
        otmp = (struct obj *) 0;
    } else {
        /* reset timer in case corpse started out as lizard or troll */
        if (otmp->otyp == CORPSE) obj_stop_timers(otmp);
        otmp->corpsenm = classmon(tt->plrole, (tt->plgend[0] == 'F'));
        otmp->owt = weight(otmp);
        otmp = oname(otmp, tt->name);
        if (otmp->otyp == CORPSE) start_corpse_timeout(otmp);
    }

    (void) fclose(rfile);
    return otmp;
}

#ifdef NO_SCAN_BRACK
/* Lattice scanf isn't up to reading the scorefile.  What */
/* follows deals with that; I admit it's ugly. (KL) */
/* Now generally available (KL) */
STATIC_OVL void
nsb_mung_line(p)
char *p;
{
    while ((p = index(p, ' ')) != 0) *p = '|';
}

STATIC_OVL void
nsb_unmung_line(p)
char *p;
{
    while ((p = index(p, '|')) != 0) *p = ' ';
}
#endif /* NO_SCAN_BRACK */

/*topten.c*/
