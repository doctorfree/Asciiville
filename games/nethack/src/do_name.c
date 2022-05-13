/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
STATIC_DCL char *NDECL(nextmbuf);
static void FDECL(getpos_help, (BOOLEAN_P, const char *));
static void FDECL(gather_locs, (coord **, int *, int));
static void auto_describe(int, int);
static void call_object(int, char *);
static void call_input(int, char *);

extern const char what_is_an_unknown_object[];      /* from pager.c */

#define NUMMBUF 5

/* manage a pool of BUFSZ buffers, so callers don't have to */
STATIC_OVL char *
nextmbuf()
{
    static char NEARDATA bufs[NUMMBUF][BUFSZ];
    static int bufidx = 0;

    bufidx = (bufidx + 1) % NUMMBUF;
    return bufs[bufidx];
}

/* function for getpos() to highlight desired map locations.
 * parameter value 0 = initialize, 1 = highlight, 2 = done
 */
static void FDECL((*getpos_hilitefunc), (int)) = (void FDECL((*), (int))) 0;
static boolean FDECL((*getpos_getvalid), (int, int)) = (boolean FDECL((*), (int, int))) 0;

void
getpos_sethilite(gp_hilitef, gp_getvalidf)
void FDECL((*gp_hilitef), (int));
boolean FDECL((*gp_getvalidf), (int, int));
{
    getpos_hilitefunc = gp_hilitef;
    getpos_getvalid = gp_getvalidf;
}

static const char *const gloc_descr[NUM_GLOCS][4] = {
    { "any monsters", "monster", "next/previous monster", "monsters" },
    { "any items", "item", "next/previous object", "objects" },
    { "any dungeon features", "dungeon feature", "next/previous dungeon feature", "dungeon features" },
    { "any doors", "door", "next/previous door or doorway", "doors or doorways" },
    { "any unexplored areas", "unexplored area", "unexplored location", "unexplored locations" },
    { "anything interesting", "interesting thing", "anything interesting", "anything interesting" },
    { "any valid locations", "valid location", "valid location", "valid locations" }
};

static const char *const gloc_filtertxt[NUM_GFILTER] = {
    "",
    " in view",
    " in this area"
};

void
getpos_help_keyxhelp(tmpwin, k1, k2, gloc)
winid tmpwin;
const char *k1;
const char *k2;
int gloc;
{
    char sbuf[BUFSZ];

    Sprintf(sbuf, "Use '%s'/'%s' to %s%s%s.",
            k1, k2,
            iflags.getloc_usemenu ? "get a menu of " : "move the cursor to ",
            gloc_descr[gloc][2 + iflags.getloc_usemenu],
            gloc_filtertxt[iflags.getloc_filter]);
    putstr(tmpwin, 0, sbuf);
}

/* the response for '?' help request in getpos() */
static void
getpos_help(force, goal)
boolean force;
const char *goal;
{
    static const char *const fastmovemode[2] = {
        "8 units at a time", "skipping same glyphs",
    };
    char sbuf[BUFSZ*2];
    boolean doing_what_is;
    winid tmpwin = create_nhwindow(NHW_MENU);

    Sprintf(sbuf,
            "Use '%c', '%c', '%c', '%c' to move the cursor to %s.", /* hjkl */
            Cmd.move_W, Cmd.move_S, Cmd.move_N, Cmd.move_E, goal);
    putstr(tmpwin, 0, sbuf);
    Sprintf(sbuf,
            "Use 'H', 'J', 'K', 'L' to fast-move the cursor, %s.",
            fastmovemode[iflags.getloc_moveskip]);
    putstr(tmpwin, 0, sbuf);
    putstr(tmpwin, 0, "Or enter a background symbol (ex. '<').");
    Sprintf(sbuf, "Use '%s' to move the cursor on yourself.",
           visctrl(Cmd.spkeys[NHKF_GETPOS_SELF]));
    putstr(tmpwin, 0, sbuf);
    if (!iflags.terrainmode || (iflags.terrainmode & TER_MON) != 0) {
        getpos_help_keyxhelp(tmpwin,
                             visctrl(Cmd.spkeys[NHKF_GETPOS_MON_NEXT]),
                             visctrl(Cmd.spkeys[NHKF_GETPOS_MON_PREV]),
                             GLOC_MONS);
    }
    if (!iflags.terrainmode || (iflags.terrainmode & TER_OBJ) != 0) {
        getpos_help_keyxhelp(tmpwin,
                             visctrl(Cmd.spkeys[NHKF_GETPOS_OBJ_NEXT]),
                             visctrl(Cmd.spkeys[NHKF_GETPOS_OBJ_PREV]),
                             GLOC_OBJS);
    }
    if (!iflags.terrainmode || (iflags.terrainmode & TER_MAP) != 0) {
        /* these are primarily useful when choosing a travel
           destination for the '_' command */
        getpos_help_keyxhelp(tmpwin,
                             visctrl(Cmd.spkeys[NHKF_GETPOS_DUNGEON_FEATURE_NEXT]),
                             visctrl(Cmd.spkeys[NHKF_GETPOS_DUNGEON_FEATURE_PREV]),
                             GLOC_DUNGEON_FEATURE);
        getpos_help_keyxhelp(tmpwin,
                             visctrl(Cmd.spkeys[NHKF_GETPOS_DOOR_NEXT]),
                             visctrl(Cmd.spkeys[NHKF_GETPOS_DOOR_PREV]),
                             GLOC_DOOR);
        getpos_help_keyxhelp(tmpwin,
                             visctrl(Cmd.spkeys[NHKF_GETPOS_UNEX_NEXT]),
                             visctrl(Cmd.spkeys[NHKF_GETPOS_UNEX_PREV]),
                             GLOC_EXPLORE);
        getpos_help_keyxhelp(tmpwin,
                             visctrl(Cmd.spkeys[NHKF_GETPOS_INTERESTING_NEXT]),
                             visctrl(Cmd.spkeys[NHKF_GETPOS_INTERESTING_PREV]),
                             GLOC_INTERESTING);
    }
    Sprintf(sbuf, "Use '%s' to change fast-move mode to %s.",
            visctrl(Cmd.spkeys[NHKF_GETPOS_MOVESKIP]),
            fastmovemode[!iflags.getloc_moveskip]);
    putstr(tmpwin, 0, sbuf);
    if (!iflags.terrainmode || (iflags.terrainmode & TER_DETECT) == 0) {
        Sprintf(sbuf, "Use '%s' to toggle menu listing for possible targets.",
                visctrl(Cmd.spkeys[NHKF_GETPOS_MENU]));
        putstr(tmpwin, 0, sbuf);
        Sprintf(sbuf,
                "Use '%s' to change the mode of limiting possible targets.",
                visctrl(Cmd.spkeys[NHKF_GETPOS_LIMITVIEW]));
        putstr(tmpwin, 0, sbuf);
    }
    if (!iflags.terrainmode) {
        char kbuf[BUFSZ];

        if (getpos_getvalid) {
            Sprintf(sbuf, "Use '%s' or '%s' to move to valid locations.",
                    visctrl(Cmd.spkeys[NHKF_GETPOS_VALID_NEXT]),
                    visctrl(Cmd.spkeys[NHKF_GETPOS_VALID_PREV]));
            putstr(tmpwin, 0, sbuf);
        }
        if (getpos_hilitefunc) {
            Sprintf(sbuf, "Use '%s' to display valid locations.",
                    visctrl(Cmd.spkeys[NHKF_GETPOS_SHOWVALID]));
            putstr(tmpwin, 0, sbuf);
        }
        Sprintf(sbuf, "Use '%s' to toggle automatic description.",
                visctrl(Cmd.spkeys[NHKF_GETPOS_AUTODESC]));
        putstr(tmpwin, 0, sbuf);
        if (iflags.cmdassist) { /* assisting the '/' command, I suppose... */
            Sprintf(sbuf,
                    (iflags.getpos_coords == GPCOORDS_NONE)
         ? "(Set 'whatis_coord' option to include coordinates with '%s' text.)"
         : "(Reset 'whatis_coord' option to omit coordinates from '%s' text.)",
                    visctrl(Cmd.spkeys[NHKF_GETPOS_AUTODESC]));
        }
        /* disgusting hack; the alternate selection characters work for any
           getpos call, but only matter for dowhatis (and doquickwhatis) */
        doing_what_is = (goal == what_is_an_unknown_object);
        if (doing_what_is) {
            Sprintf(kbuf, "'%s' or '%s' or '%s' or '%s'",
                    visctrl(Cmd.spkeys[NHKF_GETPOS_PICK]),
                    visctrl(Cmd.spkeys[NHKF_GETPOS_PICK_Q]),
                    visctrl(Cmd.spkeys[NHKF_GETPOS_PICK_O]),
                    visctrl(Cmd.spkeys[NHKF_GETPOS_PICK_V]));
        } else {
            Sprintf(kbuf, "'%s'", visctrl(Cmd.spkeys[NHKF_GETPOS_PICK]));
        }
        Sprintf(sbuf, "Type a %s when you are at the right place.", kbuf);
        putstr(tmpwin, 0, sbuf);
        if (doing_what_is) {
            Sprintf(sbuf, "  '%s' describe current spot, show 'more info', move to another spot.",
                    visctrl(Cmd.spkeys[NHKF_GETPOS_PICK_V]));
            putstr(tmpwin, 0, sbuf);
            Sprintf(sbuf,
                    "  '%s' describe current spot,%s move to another spot;",
                    visctrl(Cmd.spkeys[NHKF_GETPOS_PICK]),
                    flags.help ? " prompt if 'more info'," : "");
            putstr(tmpwin, 0, sbuf);
            Sprintf(sbuf,
                    "  '%s' describe current spot, move to another spot;",
                    visctrl(Cmd.spkeys[NHKF_GETPOS_PICK_Q]));
            putstr(tmpwin, 0, sbuf);
            Sprintf(sbuf,
                    "  '%s' describe current spot, stop looking at things;",
                    visctrl(Cmd.spkeys[NHKF_GETPOS_PICK_O]));
            putstr(tmpwin, 0, sbuf);
        }
    }

    if (!force)
        putstr(tmpwin, 0, "Type Space or Escape when you're done.");
    putstr(tmpwin, 0, "");
    display_nhwindow(tmpwin, TRUE);
    destroy_nhwindow(tmpwin);
}

struct _getpos_monarr {
    coord pos;
    long du;
};
static int getpos_monarr_len = 0;
static int getpos_monarr_idx = 0;
static struct _getpos_monarr *getpos_monarr_pos = NULL;

void
getpos_freemons()
{
    if (getpos_monarr_pos) free(getpos_monarr_pos);
    getpos_monarr_pos = NULL;
    getpos_monarr_len = 0;
}

static int
getpos_monarr_cmp(a, b)
const void *a;
const void *b;
{
    const struct _getpos_monarr *m1 = (const struct _getpos_monarr *)a;
    const struct _getpos_monarr *m2 = (const struct _getpos_monarr *)b;
    return (m1->du - m2->du);
}

void
getpos_initmons()
{
    struct monst *mtmp = fmon;
    if (getpos_monarr_pos) getpos_freemons();
    while (mtmp) {
        if (!DEADMONSTER(mtmp) && canspotmon(mtmp)) getpos_monarr_len++;
        mtmp = mtmp->nmon;
    }
    if (getpos_monarr_len) {
        int idx = 0;
        getpos_monarr_pos = (struct _getpos_monarr *)malloc(sizeof(struct _getpos_monarr) * getpos_monarr_len);
        mtmp = fmon;
        while (mtmp) {
            if (!DEADMONSTER(mtmp) && canspotmon(mtmp)) {
                getpos_monarr_pos[idx].pos.x = mtmp->mx;
                getpos_monarr_pos[idx].pos.y = mtmp->my;
                getpos_monarr_pos[idx].du = distu(mtmp->mx, mtmp->my);
                idx++;
            }
            mtmp = mtmp->nmon;
        }
        qsort(getpos_monarr_pos, getpos_monarr_len, sizeof(struct _getpos_monarr), getpos_monarr_cmp);
    }
}

struct monst *
getpos_nextmon()
{
    if (!getpos_monarr_pos) {
        getpos_initmons();
        if (getpos_monarr_len < 1) return NULL;
        getpos_monarr_idx = -1;
    }
    if (getpos_monarr_idx >= -1 && getpos_monarr_idx < getpos_monarr_len) {
        struct monst *mon;
        getpos_monarr_idx = (getpos_monarr_idx + 1) % getpos_monarr_len;
        mon = m_at(getpos_monarr_pos[getpos_monarr_idx].pos.x,
                   getpos_monarr_pos[getpos_monarr_idx].pos.y);
        return mon;
    }
    return NULL;
}

struct monst *
getpos_prevmon()
{
    if (!getpos_monarr_pos) {
        getpos_initmons();
        if (getpos_monarr_len < 1) return NULL;
        getpos_monarr_idx = getpos_monarr_len;
    }
    if (getpos_monarr_idx >= 0 && getpos_monarr_idx <= getpos_monarr_len) {
        struct monst *mon;
        getpos_monarr_idx = (getpos_monarr_idx - 1);
        if (getpos_monarr_idx < 0) getpos_monarr_idx = getpos_monarr_len - 1;
        mon = m_at(getpos_monarr_pos[getpos_monarr_idx].pos.x,
                   getpos_monarr_pos[getpos_monarr_idx].pos.y);
        return mon;
    }
    return NULL;
}

static int
cmp_coord_distu(a, b)
const void *a;
const void *b;
{
    const coord *c1 = a;
    const coord *c2 = b;
    int dx, dy, dist_1, dist_2;

    dx = u.ux - c1->x;
    dy = u.uy - c1->y;
    dist_1 = max(abs(dx), abs(dy));
    dx = u.ux - c2->x;
    dy = u.uy - c2->y;
    dist_2 = max(abs(dx), abs(dy));

    if (dist_1 == dist_2) {
        return (c1->y != c2->y) ? (c1->y - c2->y) : (c1->x - c2->x);
    }

    return dist_1 - dist_2;
}

#define IS_UNEXPLORED_LOC(x,y) \
    (isok((x), (y))                                 && \
     glyph_is_cmap(levl[(x)][(y)].glyph)            && \
     glyph_to_cmap(levl[(x)][(y)].glyph) == S_stone && \
     !levl[(x)][(y)].seenv)

static struct opvar *gloc_filter_map = (struct opvar *) 0;

#define GLOC_SAME_AREA(x,y)                          \
    (isok((x), (y)) &&                               \
     (selection_getpoint((x),(y), gloc_filter_map)))

static int gloc_filter_floodfill_match_glyph;

int
gloc_filter_classify_glyph(glyph)
int glyph;
{
    int c;

    if (!glyph_is_cmap(glyph)) {
        return 0;
    }

    c = glyph_to_cmap(glyph);

    if (is_cmap_room(c) || is_cmap_furniture(c)) {
        return 1;
    } else if (is_cmap_wall(c) || c == S_tree || c == S_deadtree) {
        return 2;
    } else if (is_cmap_corr(c)) {
        return 3;
    } else if (is_cmap_water(c)) {
        return 4;
    } else if (is_cmap_lava(c)) {
        return 5;
    }
    return 0;
}

static int
gloc_filter_floodfill_matcharea(x, y)
int x, y;
{
    int glyph = back_to_glyph(x, y);

    if (!levl[x][y].seenv) {
        return FALSE;
    }

    if (glyph == gloc_filter_floodfill_match_glyph) {
        return TRUE;
    }

    if (gloc_filter_classify_glyph(glyph) == gloc_filter_classify_glyph(gloc_filter_floodfill_match_glyph)) {
        return TRUE;
    }

    return FALSE;
}

void
gloc_filter_floodfill(x, y)
int x, y;
{
    gloc_filter_floodfill_match_glyph = back_to_glyph(x, y);

    set_selection_floodfillchk(gloc_filter_floodfill_matcharea);
    selection_floodfill(gloc_filter_map, x, y, FALSE);
}

void
gloc_filter_init()
{
    if (iflags.getloc_filter == GFILTER_AREA) {
        if (!gloc_filter_map) {
            gloc_filter_map = selection_opvar((char *) 0);
        }
        /* special case: if we're in a doorway, try to figure out which
           direction we're moving, and use that side of the doorway */
        if (IS_DOOR(levl[u.ux][u.uy].typ)) {
            if (u.dx || u.dy) {
                if (isok(u.ux + u.dx, u.uy + u.dy)) {
                    gloc_filter_floodfill(u.ux + u.dx, u.uy + u.dy);
                }
            } else {
                /* TODO: maybe add both sides of the doorway? */
            }
        } else {
            gloc_filter_floodfill(u.ux, u.uy);
        }
    }
}

void
gloc_filter_done()
{
    if (gloc_filter_map) {
        opvar_free_x(gloc_filter_map);
        gloc_filter_map = (struct opvar *) 0;
    }
}

static boolean
gather_locs_interesting(x, y, gloc)
int x, y, gloc;
{
    /* TODO: if glyph is a pile glyph, convert to ordinary one
     *       in order to keep tail/boulder/rock check simple.
     */
    int glyph = glyph_at(x, y);

    if (iflags.getloc_filter == GFILTER_VIEW && !cansee(x, y)) {
        return FALSE;
    }
    if (iflags.getloc_filter == GFILTER_AREA &&
         !GLOC_SAME_AREA(x, y) &&
         !GLOC_SAME_AREA(x - 1, y) &&
         !GLOC_SAME_AREA(x, y - 1) &&
         !GLOC_SAME_AREA(x + 1, y) &&
         !GLOC_SAME_AREA(x, y + 1)) {
        return FALSE;
    }

    switch (gloc) {
    default:
    case GLOC_MONS:
        /* unlike '/M', this skips monsters revealed by
           warning glyphs and remembered unseen ones */
        return (glyph_is_monster(glyph) && glyph != monnum_to_glyph(PM_LONG_WORM_TAIL));

    case GLOC_OBJS:
        return (glyph_is_object(glyph) &&
                glyph != objnum_to_glyph(BOULDER) &&
                glyph != objnum_to_glyph(ROCK));

    case GLOC_DUNGEON_FEATURE:
        return (glyph_is_cmap(glyph) &&
                (is_cmap_furniture(glyph_to_cmap(glyph))));

    case GLOC_DOOR:
        return (glyph_is_cmap(glyph) &&
                (is_cmap_door(glyph_to_cmap(glyph)) ||
                 is_cmap_drawbridge(glyph_to_cmap(glyph)) ||
                 glyph_to_cmap(glyph) == S_ndoor));

    case GLOC_EXPLORE:
        return (glyph_is_cmap(glyph) &&
                (is_cmap_door(glyph_to_cmap(glyph)) ||
                 is_cmap_drawbridge(glyph_to_cmap(glyph)) ||
                 glyph_to_cmap(glyph) == S_ndoor ||
                 glyph_to_cmap(glyph) == S_room ||
                 glyph_to_cmap(glyph) == S_darkroom ||
                 glyph_to_cmap(glyph) == S_corr ||
                 glyph_to_cmap(glyph) == S_litcorr) &&
                (IS_UNEXPLORED_LOC(x + 1, y) ||
                 IS_UNEXPLORED_LOC(x - 1, y) ||
                 IS_UNEXPLORED_LOC(x, y + 1) ||
                 IS_UNEXPLORED_LOC(x, y - 1)));

    case GLOC_VALID:
        if (getpos_getvalid) {
            return (*getpos_getvalid)(x,y);
        }
        /* fall through */

    case GLOC_INTERESTING:
        return !gather_locs_interesting(x,y, GLOC_DOOR) &&
               !(glyph_is_cmap(glyph) &&
                 (is_cmap_wall(glyph_to_cmap(glyph)) ||
                  glyph_to_cmap(glyph) == S_tree     ||
                  glyph_to_cmap(glyph) == S_deadtree ||
                  glyph_to_cmap(glyph) == S_bars     ||
                  glyph_to_cmap(glyph) == S_ice      ||
                  glyph_to_cmap(glyph) == S_air      ||
                  glyph_to_cmap(glyph) == S_cloud    ||
                  glyph_to_cmap(glyph) == S_lava     ||
                  glyph_to_cmap(glyph) == S_water    ||
                  glyph_to_cmap(glyph) == S_pool     ||
                  glyph_to_cmap(glyph) == S_ndoor    ||
                  glyph_to_cmap(glyph) == S_room     ||
                  glyph_to_cmap(glyph) == S_darkroom ||
                  glyph_to_cmap(glyph) == S_corr     ||
                  glyph_to_cmap(glyph) == S_litcorr));
    }
    /*NOTREACHED*/
    return FALSE;
}

/* gather locations for monsters or objects shown on the map */
static void
gather_locs(arr_p, cnt_p, gloc)
coord **arr_p;
int *cnt_p;
int gloc;
{
    int x, y, pass, idx;

    /*
     * We always include the hero's location even if there is no monster
     * (invisible hero without see invisible) or object (usual case)
     * displayed there.  That way, the count will always be at least 1,
     * and player has a visual indicator (cursor returns to hero's spot)
     * highlighting when successive 'm's or 'o's have cycled all the way
     * through all monsters or objects.
     *
     * Hero's spot will always sort to array[0] because it will always
     * be the shortest distance (namely, 0 units) away from <u.ux,u.uy>.
     */

    gloc_filter_init();

    *cnt_p = idx = 0;
    for (pass = 0; pass < 2; pass++) {
        for (x = 1; x < COLNO; x++) {
            for (y = 0; y < ROWNO; y++) {
                if ((x == u.ux && y == u.uy) || gather_locs_interesting(x, y, gloc)) {
                    if (!pass) {
                        ++*cnt_p;
                    } else {
                        (*arr_p)[idx].x = x;
                        (*arr_p)[idx].y = y;
                        ++idx;
                    }
                }
            }
        }

        if (!pass) {
            /* end of first pass */
            *arr_p = (coord *) alloc(*cnt_p * sizeof (coord));
        } else {
            /* end of second pass */
            qsort(*arr_p, *cnt_p, sizeof (coord), cmp_coord_distu);
        }
    } /* pass */

    gloc_filter_done();
}

char *
dxdy_to_dist_descr(dx, dy, fulldir)
int dx, dy;
boolean fulldir;
{
    static char buf[30];
    int dst;

    if (!dx && !dy) {
        Sprintf(buf, "here");
    } else if ((dst = xytod(dx, dy)) != -1) {
        /* explicit direction; 'one step' is implicit */
        Sprintf(buf, "%s", directionname(dst));
    } else {
        static const char *dirnames[4][2] = {
            { "n", "north" },
            { "s", "south" },
            { "w", "west" },
            { "e", "east" } };
        buf[0] = '\0';
        /* 9999: protect buf[] against overflow caused by invalid values */
        if (dy) {
            if (abs(dy) > 9999) {
                dy = sgn(dy) * 9999;
            }
            Sprintf(eos(buf), "%d%s%s", abs(dy), dirnames[(dy > 0)][fulldir], dx ? "," : "");
        }
        if (dx) {
            if (abs(dx) > 9999) {
                dx = sgn(dx) * 9999;
            }
            Sprintf(eos(buf), "%d%s", abs(dx), dirnames[2 + (dx > 0)][fulldir]);
        }
    }
    return buf;
}

/* coordinate formatting for 'whatis_coord' option */
char *
coord_desc(x, y, outbuf, cmode)
int x, y;
char *outbuf, cmode;
{
    static char screen_fmt[16]; /* [12] suffices: "[%02d,%02d]" */
    int dx, dy;

    outbuf[0] = '\0';
    switch (cmode) {
    default:
        break;
    case GPCOORDS_COMFULL:
    case GPCOORDS_COMPASS:
        /* "east", "3s", "2n,4w" */
        dx = x - u.ux;
        dy = y - u.uy;
        Sprintf(outbuf, "(%s)", dxdy_to_dist_descr(dx, dy, cmode == GPCOORDS_COMFULL));
        break;
    case GPCOORDS_MAP: /* x,y */
        /* upper left corner of map is <1,0>;
           with default COLNO,ROWNO lower right corner is <79,20> */
        Sprintf(outbuf, "<%d,%d>", x, y);
        break;
    case GPCOORDS_SCREEN: /* y+2,x */
        /* for normal map sizes, force a fixed-width formatting so that
           /m, /M, /o, and /O output lines up cleanly; map sizes bigger
           than Nx999 or 999xM will still work, but not line up like normal
           when displayed in a column setting */
        if (!*screen_fmt) {
            Sprintf(screen_fmt, "[%%%sd,%%%sd]",
                    (ROWNO-1 + 2 < 100) ? "02" :  "03",
                    (COLNO-1 < 100) ? "02" : "03");
        }
        /* map line 0 is screen row 2;
           map column 0 isn't used, map column 1 is screen column 1 */
        Sprintf(outbuf, screen_fmt, y + 2, x);
        break;
    }
    return outbuf;
}

static void
auto_describe(int cx, int cy)
{
    coord cc;
    int sym = 0;
    char tmpbuf[BUFSZ];
    const char *firstmatch = "unknown";

    cc.x = cx;
    cc.y = cy;
    if (do_screen_description(cc, TRUE, sym, tmpbuf, &firstmatch, (struct permonst **) 0)) {
        (void) coord_desc(cx, cy, tmpbuf, iflags.getpos_coords);
        boolean illegal = (iflags.autodescribe && getpos_getvalid && !(*getpos_getvalid)(cx, cy));
        boolean no_travel_path = iflags.getloc_travelmode && !is_valid_travelpt(cx, cy);
        custompline((SUPPRESS_HISTORY | OVERRIDE_MSGTYPE),
                    "%s%s%s%s%s", firstmatch, *tmpbuf ? " " : "", tmpbuf,
                    illegal ? " (illegal)" : "",
                    no_travel_path ? " (no travel path)" : "");
        curs(WIN_MAP, cx, cy);
        flush_screen(0);
    }
}

boolean
getpos_menu(ccp, gloc)
coord *ccp;
int gloc;
{
    coord *garr = DUMMY;
    int gcount = 0;
    winid tmpwin;
    anything any;
    int i, pick_cnt;
    menu_item *picks = (menu_item *) 0;
    char tmpbuf[BUFSZ];

    gather_locs(&garr, &gcount, gloc);

    /* gcount always includes the hero */
    if (gcount < 2) {
        free(garr);
        You("cannot %s %s.",
            iflags.getloc_filter == GFILTER_VIEW ? "see" : "detect",
            gloc_descr[gloc][0]);
        return FALSE;
    }

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);
    any = zeroany;

    /* gather_locs returns array[0] == you. skip it. */
    for (i = 1; i < gcount; i++) {
        char fullbuf[BUFSZ];
        coord tmpcc;
        const char *firstmatch = "unknown";
        int sym = 0;

        any.a_int = i + 1;
        tmpcc.x = garr[i].x;
        tmpcc.y = garr[i].y;
        if (do_screen_description(tmpcc, TRUE, sym, tmpbuf, &firstmatch, (struct permonst **)0)) {
            (void) coord_desc(garr[i].x, garr[i].y, tmpbuf, iflags.getpos_coords);
            Snprintf(fullbuf, sizeof fullbuf, "%s%s%s", firstmatch, (*tmpbuf ? " " : ""), tmpbuf);
            int glyph = (tmpcc.x + tmpcc.y * COLNO) * -1;
            add_menu(tmpwin, glyph, MENU_DEFCNT, &any, 0, 0, ATR_NONE, fullbuf, MENU_UNSELECTED);
        }
    }

    Sprintf(tmpbuf, "Pick %s%s%s",
            an(gloc_descr[gloc][1]),
            gloc_filtertxt[iflags.getloc_filter],
            iflags.getloc_travelmode ? " for travel destination" : "");
    end_menu(tmpwin, tmpbuf);
    pick_cnt = select_menu(tmpwin, PICK_ONE, &picks);
    destroy_nhwindow(tmpwin);
    if (pick_cnt > 0) {
        ccp->x = garr[picks->item.a_int - 1].x;
        ccp->y = garr[picks->item.a_int - 1].y;
        free(picks);
    }
    free(garr);
    return (pick_cnt > 0);
}

int
getpos(coord *ccp, boolean force, const char *goal)
{
    const char *cp;
    static struct {
        int nhkf, ret;
    } const pick_chars_def[] = {
        { NHKF_GETPOS_PICK, LOOK_TRADITIONAL },
        { NHKF_GETPOS_PICK_Q, LOOK_QUICK },
        { NHKF_GETPOS_PICK_O, LOOK_ONCE },
        { NHKF_GETPOS_PICK_V, LOOK_VERBOSE }
    };
    static const int mMoOdDxX_def[] = {
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
        NHKF_GETPOS_VALID_PREV
    };
    char pick_chars[6];
    char mMoOdDxX[NUM_GLOCS*2 + 1];
    int result = 0;
    int cx, cy, i, c;
    int sidx, tx, ty;
    boolean msg_given = TRUE; /* clear message window by default */
    boolean show_goal_msg = FALSE;
    boolean hilite_state = FALSE;
    coord *garr[NUM_GLOCS] = DUMMY;
    int gcount[NUM_GLOCS] = DUMMY;
    int gidx[NUM_GLOCS] = DUMMY;

    for (i = 0; i < SIZE(pick_chars_def); i++) {
        pick_chars[i] = Cmd.spkeys[pick_chars_def[i].nhkf];
    }
    pick_chars[SIZE(pick_chars_def)] = '\0';

    for (i = 0; i < SIZE(mMoOdDxX_def); i++) {
        mMoOdDxX[i] = Cmd.spkeys[mMoOdDxX_def[i]];
    }
    mMoOdDxX[SIZE(mMoOdDxX_def)] = '\0';

    if (!goal) {
        goal = "desired location";
    }
    if (flags.verbose) {
        pline("(For instructions type a '%s')", visctrl(Cmd.spkeys[NHKF_GETPOS_HELP]));
        msg_given = TRUE;
    }
    cx = ccp->x;
    cy = ccp->y;
#ifdef CLIPPING
    cliparound(cx, cy);
#endif
    curs(WIN_MAP, cx, cy);
    flush_screen(0);
#ifdef MAC
    lock_mouse_cursor(TRUE);
#endif
    for (;;) {
        if (show_goal_msg) {
            pline("Move cursor to %s:", goal);
            curs(WIN_MAP, cx, cy);
            flush_screen(0);
            show_goal_msg = FALSE;
        } else if (iflags.autodescribe && !msg_given && !hilite_state) {
            auto_describe(cx, cy);
        }

        c = nh_poskey(&tx, &ty, &sidx);

        if (hilite_state) {
            (*getpos_hilitefunc)(2);
            hilite_state = FALSE;
            curs(WIN_MAP, cx, cy);
            flush_screen(0);
        }

        if (iflags.autodescribe) {
            msg_given = FALSE;
        }

        if (c == Cmd.spkeys[NHKF_ESC]) {
            cx = cy = -10;
            msg_given = TRUE; /* force clear */
            result = -1;
            break;
        }
        if (c == 0) {
            if (!isok(tx, ty)) {
                continue;
            }
            /* a mouse click event, just assign and return */
            cx = tx;
            cy = ty;
            break;
        }
        if ((cp = index(pick_chars, c)) != 0) {
            /* '.' => 0, ',' => 1, ';' => 2, ':' => 3 */
            result = pick_chars_def[(int) (cp - pick_chars)].ret;
            break;
        }
        for (i = 0; i < 8; i++) {
            int dx, dy;

            if (Cmd.dirchars[i] == c) {
                /* a normal movement letter or digit */
                dx = xdir[i];
                dy = ydir[i];
            } else if (Cmd.alphadirchars[i] == lowc((char) c) ||
                       (Cmd.num_pad && Cmd.dirchars[i] == (c & 0177))) {
                /* a shifted movement letter or Meta-digit */
                if (iflags.getloc_moveskip) {
                    /* skip same glyphs */
                    int glyph = glyph_at(cx, cy);

                    dx = xdir[i];
                    dy = ydir[i];
                    while (isok(cx + dx, cy + dy) &&
                           glyph == glyph_at(cx + dx, cy + dy) &&
                           isok(cx + dx + xdir[i], cy + dy + ydir[i]) &&
                           glyph == glyph_at(cx + dx + xdir[i], cy + dy + ydir[i])) {
                        dx += xdir[i];
                        dy += ydir[i];
                    }
                } else {
                    dx = 8 * xdir[i];
                    dy = 8 * ydir[i];
                }
            } else {
                continue;
            }

            /* truncate at map edge; diagonal moves complicate this... */
            if (cx + dx < 1) {
                dy -= sgn(dy) * (1 - (cx + dx));
                dx = 1 - cx; /* so that (cx+dx == 1) */
            } else if (cx + dx > COLNO - 1) {
                dy += sgn(dy) * ((COLNO - 1) - (cx + dx));
                dx = (COLNO - 1) - cx;
            }
            if (cy + dy < 0) {
                dx -= sgn(dx) * (0 - (cy + dy));
                dy = 0 - cy; /* so that (cy+dy == 0) */
            } else if (cy + dy > ROWNO - 1) {
                dx += sgn(dx) * ((ROWNO - 1) - (cy + dy));
                dy = (ROWNO - 1) - cy;
            }
            cx += dx;
            cy += dy;
            goto nxtc;
        }

        if (c == Cmd.spkeys[NHKF_GETPOS_HELP] || redraw_cmd(c)) {
            if (c == Cmd.spkeys[NHKF_GETPOS_HELP]) {
                getpos_help(force, goal);
            } else {
                /* ^R */
                docrt(); /* redraw */
            }
            /* update message window to reflect that we're still targetting */
            show_goal_msg = TRUE;
            msg_given = TRUE;
        } else if (c == Cmd.spkeys[NHKF_GETPOS_SHOWVALID] && getpos_hilitefunc) {
            if (!hilite_state) {
                (*getpos_hilitefunc)(0);
                (*getpos_hilitefunc)(1);
                hilite_state = TRUE;
            }
            goto nxtc;
        } else if (c == Cmd.spkeys[NHKF_GETPOS_AUTODESC]) {
            iflags.autodescribe = !iflags.autodescribe;
            pline("Automatic description %sis %s.",
                  flags.verbose ? "of features under cursor " : "",
                  iflags.autodescribe ? "on" : "off");
            if (!iflags.autodescribe) {
                show_goal_msg = TRUE;
            }
            msg_given = TRUE;
            goto nxtc;
        } else if (c == Cmd.spkeys[NHKF_GETPOS_LIMITVIEW]) {
            static const char *const view_filters[NUM_GFILTER] = {
                "Not limiting targets",
                "Limiting targets to those in sight",
                "Limiting targets to those in same area"
            };

            iflags.getloc_filter = (iflags.getloc_filter + 1) % NUM_GFILTER;
            for (i = 0; i < NUM_GLOCS; i++) {
                if (garr[i]) {
                    free(garr[i]);
                    garr[i] = NULL;
                }
                gidx[i] = gcount[i] = 0;
            }
            pline("%s.", view_filters[iflags.getloc_filter]);
            msg_given = TRUE;
            goto nxtc;
        } else if (c == Cmd.spkeys[NHKF_GETPOS_MENU]) {
            iflags.getloc_usemenu = !iflags.getloc_usemenu;
            pline("%s a menu to show possible targets.",
                  iflags.getloc_usemenu ? "Using" : "Not using");
            msg_given = TRUE;
            goto nxtc;
        } else if (c == Cmd.spkeys[NHKF_GETPOS_SELF]) {
            /* reset 'm&M', 'o&O', &c; otherwise, there's no way for player
               to achieve that except by manually cycling through all spots */
            for (i = 0; i < NUM_GLOCS; i++) {
                gidx[i] = 0;
            }
            cx = u.ux;
            cy = u.uy;
            goto nxtc;
        } else if (c == Cmd.spkeys[NHKF_GETPOS_MOVESKIP]) {
            iflags.getloc_moveskip = !iflags.getloc_moveskip;
            pline("%skipping over similar terrain when fastmoving the cursor.",
                  iflags.getloc_moveskip ? "S" : "Not s");
            msg_given = TRUE;
            goto nxtc;
        } else if ((cp = index(mMoOdDxX, c)) != 0) { /* 'm|M', 'o|O', &c */
            /* nearest or farthest monster or object or door or unexplored */
            int gtmp = (int) (cp - mMoOdDxX); /* 0..7 */
            int gloc = gtmp >> 1;             /* 0..3 */

            if (iflags.getloc_usemenu) {
                coord tmpcrd;

                if (getpos_menu(&tmpcrd, gloc)) {
                    cx = tmpcrd.x;
                    cy = tmpcrd.y;
                    goto nxtc;
                } else {
                    result = -1;
                    break;
                }
            }

            if (!garr[gloc]) {
                gather_locs(&garr[gloc], &gcount[gloc], gloc);
                gidx[gloc] = 0; /* garr[][0] is hero's spot */
            }
            if (!(gtmp & 1)) {
                /* c=='m' || c=='o' || c=='d' || c=='x') */
                gidx[gloc] = (gidx[gloc] + 1) % gcount[gloc];
            } else {
                /* c=='M' || c=='O' || c=='D' || c=='X') */
                if (--gidx[gloc] < 0) {
                    gidx[gloc] = gcount[gloc] - 1;
                }
            }
            cx = garr[gloc][gidx[gloc]].x;
            cy = garr[gloc][gidx[gloc]].y;
            goto nxtc;
        } else {
            if (!index(quitchars, c)) {
                char matching[MAXPCHARS];
                int pass, lo_x, lo_y, hi_x, hi_y, k = 0;

                (void) memset(matching, 0, sizeof matching);
                /* [0] left as 0 */
                for (sidx = 1; sidx < MAXPCHARS; sidx++) {
                    if (is_cmap_door(sidx) ||
                         is_cmap_wall(sidx) ||
                         is_cmap_corr(sidx) ||
                         is_cmap_room(sidx)) {
                        continue;
                    }
                    if (c == defsyms[sidx].sym || c == (int) showsyms[sidx]) {
                        matching[sidx] = (char) ++k;
                    }
                }
                if (k) {
                    for (pass = 0; pass <= 1; pass++) {
                        /* pass 0: just past current pos to lower right;
                           pass 1: upper left corner to current pos */
                        lo_y = (pass == 0) ? cy : 0;
                        hi_y = (pass == 0) ? ROWNO - 1 : cy;
                        for (ty = lo_y; ty <= hi_y; ty++) {
                            lo_x = (pass == 0 && ty == lo_y) ? cx + 1 : 1;
                            hi_x = (pass == 1 && ty == hi_y) ? cx : COLNO - 1;
                            for (tx = lo_x; tx <= hi_x; tx++) {
                                /* first, look at what is currently visible
                                   (might be monster) */
                                k = glyph_at(tx, ty);
                                if (glyph_is_cmap(k) && matching[glyph_to_cmap(k)]) {
                                    goto foundc;
                                }
                                /* next, try glyph that's remembered here
                                   (might be trap or object) */
                                if (level.flags.hero_memory &&
                                    /* !terrainmode: don't move to remembered
                                       trap or object if not currently shown */
                                     !iflags.terrainmode) {
                                    k = levl[tx][ty].glyph;
                                    if (glyph_is_cmap(k) && matching[glyph_to_cmap(k)]) {
                                        goto foundc;
                                    }
                                }
                                /* last, try actual terrain here (shouldn't
                                   we be using g.lastseentyp[][] instead?) */
                                if (levl[tx][ty].seenv) {
                                    k = back_to_glyph(tx, ty);
                                    if (glyph_is_cmap(k) && matching[glyph_to_cmap(k)]) {
                                        goto foundc;
                                    }
                                }
                                continue;
                            foundc:
                                cx = tx, cy = ty;
                                if (msg_given) {
                                    clear_nhwindow(WIN_MESSAGE);
                                    msg_given = FALSE;
                                }
                                goto nxtc;
                            } /* column */
                        }     /* row */
                    }         /* pass */
                    pline("Can't find dungeon feature '%c'.", c);
                    msg_given = TRUE;
                    goto nxtc;
                } else {
                    char note[QBUFSZ];

                    if (!force) {
                        Strcpy(note, "aborted");
                    } else {
                        /* hjkl */
                        Sprintf(note, "use '%c', '%c', '%c', '%c' or '%s'",
                                Cmd.move_W, Cmd.move_S, Cmd.move_N, Cmd.move_E,
                                visctrl(Cmd.spkeys[NHKF_GETPOS_PICK]));
                    }
                    pline("Unknown direction: '%s' (%s).", visctrl((char) c), note);
                    msg_given = TRUE;
                } /* k => matching */
            }     /* !quitchars */
            if (force) {
                goto nxtc;
            }
            pline("Done.");
            msg_given = FALSE; /* suppress clear */
            cx = -1;
            cy = 0;
            result = 0; /* not -1 */
            break;
        }
    nxtc:
#ifdef CLIPPING
        cliparound(cx, cy);
#endif
        curs(WIN_MAP, cx, cy);
        flush_screen(0);
    }
#ifdef MAC
    lock_mouse_cursor(FALSE);
#endif
    if (msg_given) {
        clear_nhwindow(WIN_MESSAGE);
    }
    ccp->x = cx;
    ccp->y = cy;
    for (i = 0; i < NUM_GLOCS; i++) {
        if (garr[i]) {
            free(garr[i]);
        }
    }
    getpos_hilitefunc = (void (*)(int)) 0;
    getpos_getvalid = (boolean (*)(int, int)) 0;

    return result;
}

/* allocate space for a monster's name; removes old name if there is one */
void
new_mname(mon, lth)
struct monst *mon;
int lth; /* desired length (caller handles adding 1 for terminator) */
{
    if (lth) {
        /* allocate mextra if necessary; otherwise get rid of old name */
        if (!mon->mextra) {
            mon->mextra = newmextra();
        } else {
            free_mname(mon); /* already has mextra, might also have name */
        }
        MNAME(mon) = (char *) alloc((unsigned) lth);
    } else {
        /* zero length: the new name is empty; get rid of the old name */
        if (has_mname(mon))
            free_mname(mon);
    }
}

/* release a monster's name; retains mextra even if all fields are now null */
void
free_mname(mon)
struct monst *mon;
{
    if (has_mname(mon)) {
        free((genericptr_t) MNAME(mon));
        MNAME(mon) = (char *) 0;
    }
}

/* allocate space for an object's name; removes old name if there is one */
void
new_oname(obj, lth)
struct obj *obj;
int lth; /* desired length (caller handles adding 1 for terminator) */
{
    if (lth) {
        /* allocate oextra if necessary; otherwise get rid of old name */
        if (!obj->oextra)
            obj->oextra = newoextra();
        else
            free_oname(obj); /* already has oextra, might also have name */
        ONAME(obj) = (char *) alloc((unsigned) lth);
    } else {
        /* zero length: the new name is empty; get rid of the old name */
        if (has_oname(obj))
            free_oname(obj);
    }
}

/* release an object's name; retains oextra even if all fields are now null */
void
free_oname(obj)
struct obj *obj;
{
    if (has_oname(obj)) {
        free((genericptr_t) ONAME(obj));
        ONAME(obj) = (char *) 0;
    }
}

/*  safe_oname() always returns a valid pointer to
 *  a string, either the pointer to an object's name
 *  if it has one, or a pointer to an empty string
 *  if it doesn't.
 */
const char *
safe_oname(obj)
struct obj *obj;
{
    if (has_oname(obj)) {
        return ONAME(obj);
    }
    return "";
}

/* historical note: this returns a monster pointer because it used to
   allocate a new bigger block of memory to hold the monster and its name */
struct monst *
christen_monst(mtmp, name)
struct monst *mtmp;
const char *name;
{
    int lth;
    char buf[PL_PSIZ];

    /* dogname & catname are PL_PSIZ arrays; object names have same limit */
    lth = (name && *name) ? ((int) strlen(name) + 1) : 0;
    if(lth > PL_PSIZ) {
        lth = PL_PSIZ;
        name = strncpy(buf, name, PL_PSIZ - 1);
        buf[PL_PSIZ - 1] = '\0';
    }
    new_mname(mtmp, lth); /* removes old name if one is present */
    if (lth) {
        Strcpy(MNAME(mtmp), name);
    }
    return mtmp;
}

int
do_mname()
{
    char buf[BUFSZ];
    coord cc;
    register int cx, cy;
    register struct monst *mtmp;
    char qbuf[BUFSZ*2];

    if (Hallucination) {
        You("would never recognize it anyway.");
        return 0;
    }
    cc.x = u.ux;
    cc.y = u.uy;
    if (getpos(&cc, FALSE, "the monster you want to name") < 0 ||
        (cx = cc.x) < 0)
        return 0;
    cy = cc.y;

    if (cx == u.ux && cy == u.uy) {
#ifdef STEED
        if (u.usteed && canspotmon(u.usteed))
            mtmp = u.usteed;
        else {
#endif
        pline("This %s creature is called %s and cannot be renamed.",
              beautiful(),
              plname);
        return(0);
#ifdef STEED
    }
#endif
    } else
        mtmp = m_at(cx, cy);

    if (!mtmp || (!sensemon(mtmp) &&
                  (!(cansee(cx, cy) || see_with_infrared(mtmp)) || mtmp->mundetected
                   || mtmp->m_ap_type == M_AP_FURNITURE
                   || mtmp->m_ap_type == M_AP_OBJECT
                   || (mtmp->minvis && !See_invisible)))) {
        pline("I see no monster there.");
        return(0);
    }
    /* special case similar to the one in lookat() */
    (void) distant_monnam(mtmp, ARTICLE_THE, buf);
    Sprintf(qbuf, "What do you want to call %s?", buf);
    getlin(qbuf, buf);
    if(!*buf || *buf == '\033') return(0);
    /* strip leading and trailing spaces; unnames monster if all spaces */
    (void)mungspaces(buf);

    if (mtmp->data->geno & G_UNIQ)
        pline("%s doesn't like being called names!", Monnam(mtmp));
    else
        (void) christen_monst(mtmp, buf);
    return(0);
}

STATIC_VAR int via_naming = 0;

/*
 * This routine changes the address of obj. Be careful not to call it
 * when there might be pointers around in unknown places. For now: only
 * when obj is in the inventory.
 */
void
do_oname(obj)
register struct obj *obj;
{
    char *bufp, buf[BUFSZ], bufcpy[BUFSZ], qbuf[QBUFSZ];
    const char *aname;
    short objtyp;

#ifdef SPE_NOVEL
    /* Do this now because there's no point in even asking for a name */
    if (obj->otyp == SPE_NOVEL) {
        pline("%s already has a published name.", Ysimple_name2(obj));
        return;
    }
#endif

    Sprintf(qbuf, "What do you want to name %s ",
            is_plural(obj) ? "these" : "this");
    (void) safe_qbuf(qbuf, qbuf, "?", obj, xname, simpleonames, "item");
    buf[0] = '\0';
#ifdef EDIT_GETLIN
    /* if there's an existing name, make it be the default answer */
    if (has_oname(obj)) {
        Strcpy(buf, ONAME(obj));
    }
#endif
    getlin(qbuf, buf);
    if (!*buf || *buf == '\033') {
        return;
    }

    /* strip leading and trailing spaces; unnames item if all spaces */
    (void)mungspaces(buf);

    /*
     * We don't violate illiteracy conduct here, although it is
     * arguable that we should for anything other than "X".  Doing so
     * would make attaching player's notes to hero's inventory have an
     * in-game effect, which may or may not be the correct thing to do.
     *
     * We do violate illiteracy in oname() if player creates Sting or
     * Orcrist, clearly being literate (no pun intended...).
     */

    /* relax restrictions over proper capitalization for artifacts */
    if ((aname = artifact_name(buf, &objtyp)) != 0 && objtyp == obj->otyp)
        Strcpy(buf, aname);

    if (obj->oartifact) {
        pline_The("artifact seems to resist the attempt.");
        return;
    } else if (restrict_name(obj, buf, FALSE) || exist_artifact(obj->otyp, buf)) {
        int n = rn2((int)strlen(buf));
        register char c1, c2;

        c1 = lowc(buf[n]);
        do c2 = 'a' + rn2('z'-'a'); while (c1 == c2);
        buf[n] = (buf[n] == c1) ? c2 : highc(c2);  /* keep same case */
        pline("While engraving your %s slips.", body_part(HAND));
        display_nhwindow(WIN_MESSAGE, FALSE);
        You("engrave: \"%s\".", buf);
    }
    ++via_naming; /* This ought to be an argument rather than a static... */
    obj = oname(obj, buf);
    --via_naming; /* ...but oname() is used in a lot of places, so defer. */
}

struct obj *
oname(obj, name)
struct obj *obj;
const char *name;
{
    int lth;
    char buf[PL_PSIZ];

    lth = *name ? (int)(strlen(name) + 1) : 0;
    if (lth > PL_PSIZ) {
        lth = PL_PSIZ;
        name = strncpy(buf, name, PL_PSIZ - 1);
        buf[PL_PSIZ - 1] = '\0';
    }
    /* If named artifact exists in the game, do not create another.
     * Also trying to create an artifact shouldn't de-artifact
     * it (e.g. Excalibur from prayer). In this case the object
     * will retain its current name. */
    if (obj->oartifact || (lth && exist_artifact(obj->otyp, name))) {
        return obj;
    }

    new_oname(obj, lth); /* removes old name if one is present */
    if (lth) {
        Strcpy(ONAME(obj), name);
    }
    if (lth) {
        artifact_exists(obj, name, TRUE);
    }
    if (obj->oartifact) {
        /* can't dual-wield with artifact as secondary weapon */
        if (obj == uswapwep) {
            untwoweapon();
        }
        /* activate warning if you've just named your weapon "Sting" */
        if (obj == uwep) {
            set_artifact_intrinsic(obj, TRUE, W_WEP);
        }
        /* if obj is owned by a shop, increase your bill */
        if (obj->unpaid) {
            alter_cost(obj, 0L);
        }
        if (via_naming) {
            /* violate illiteracy conduct since successfully wrote arti-name */
            if (successful_cdt(CONDUCT_ILLITERACY)) {
                livelog_printf(LL_CONDUCT | LL_ARTIFACT, "became literate by naming %s",
                               bare_artifactname(obj));
            } else {
                livelog_printf(LL_ARTIFACT, "chose %s to be named \"%s\"",
                               ansimpleoname(obj), bare_artifactname(obj));
            }
            violated(CONDUCT_ILLITERACY);
        }
    }
    if (carried(obj)) {
        update_inventory();
    }
    return obj;
}

static NEARDATA const char callable[] = {
    SCROLL_CLASS, POTION_CLASS, WAND_CLASS,  RING_CLASS, AMULET_CLASS,
    GEM_CLASS,    SPBOOK_CLASS, ARMOR_CLASS, TOOL_CLASS, 0
};

boolean
objtyp_is_callable(i)
int i;
{
    return (objects[i].oc_uname ||
            (OBJ_DESCR(objects[i]) && index(callable, objects[i].oc_class)));
}

int
ddocall()
{
    register struct obj *obj;
#ifdef REDO
    char ch;
#endif
    char allowall[2];

    switch(
#ifdef REDO
        ch =
#endif
        ynq("Name an individual object?")) {
    case 'q':
        break;
    case 'y':
#ifdef REDO
        savech(ch);
#endif
        allowall[0] = ALL_CLASSES; allowall[1] = '\0';
        obj = getobj(allowall, "name");
        if(obj) do_oname(obj);
        break;
    default:
#ifdef REDO
        savech(ch);
#endif
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
    }
    return 0;
}

/* for use by safe_qbuf() */
STATIC_PTR char *
docall_xname(obj)
struct obj *obj;
{
    struct obj otemp;

    otemp = *obj;
    otemp.oextra = (struct oextra *) 0;
    otemp.quan = 1L;
    /* in case water is already known, convert "[un]holy water" to "water" */
    otemp.blessed = otemp.cursed = 0;
    /* remove attributes that are doname() caliber but get formatted
       by xname(); most of these fixups aren't really needed because the
       relevant type of object isn't callable so won't reach this far */
    if (otemp.oclass == WEAPON_CLASS)
        otemp.opoisoned = 0; /* not poisoned */
    else if (otemp.oclass == POTION_CLASS)
        otemp.odiluted = 0; /* not diluted */
    else if (otemp.otyp == TOWEL || otemp.otyp == STATUE)
        otemp.spe = 0; /* not wet or historic */
    else if (otemp.otyp == TIN)
        otemp.known = 0; /* suppress tin type (homemade, &c) and mon type */
    else if (otemp.otyp == FIGURINE)
        otemp.corpsenm = NON_PM; /* suppress mon type */
    else if (otemp.otyp == HEAVY_IRON_BALL)
        otemp.owt = objects[HEAVY_IRON_BALL].oc_weight; /* not "very heavy" */
    else if (otemp.oclass == FOOD_CLASS && otemp.globby)
        otemp.owt = 120; /* 6*20, neither a small glob nor a large one */

    return an(xname(&otemp));
}

void
docall(obj)
struct obj *obj;
{
    char qbuf[QBUFSZ];

    if (!obj->dknown) {
        /* probably blind */
        return;
    }
    flush_screen(1); /* buffered updates might matter to player's response */
    check_tutorial_message(QT_T_CALLITEM);

    if (obj->oclass == POTION_CLASS && obj->fromsink) {
        /* kludge, meaning it's sink water */
        Sprintf(qbuf, "Call a stream of %s fluid:",
                OBJ_DESCR(objects[obj->otyp]));
    } else {
        (void) safe_qbuf(qbuf, "Call ", ":", obj, docall_xname, simpleonames, "thing");
    }
    call_input(obj->otyp, qbuf);
}

void
docall_input(int obj_otyp)
{
    char qbuf[QBUFSZ];
    struct obj otemp = {0};

    otemp.otyp = obj_otyp;
    otemp.oclass = objects[obj_otyp].oc_class;
    otemp.quan = 1L;
    Sprintf(qbuf, "Call %s:", an(xname(&otemp)));
    call_input(obj_otyp, qbuf);
}

/* Using input from player to name an object type. */
static void
call_input(int obj_otyp, char *prompt)
{
    char buf[BUFSZ];

    getlin(prompt, buf);

    if(!*buf || *buf == '\033') {
        flags.last_broken_otyp = obj_otyp;
        return;
    } else {
        flags.last_broken_otyp = STRANGE_OBJECT;
    }

    call_object(obj_otyp, buf);
}

static void
call_object(int obj_otyp, char *buf)
{
    register char **str1;
    /* clear old name */
    str1 = &(objects[obj_otyp].oc_uname);
    if(*str1) free((genericptr_t)*str1);

    /* strip leading and trailing spaces; uncalls item if all spaces */
    (void)mungspaces(buf);
    if (!*buf) {
        if (*str1) {    /* had name, so possibly remove from disco[] */
            /* strip name first, for the update_inventory() call
               from undiscover_object() */
            *str1 = (char *)0;
            undiscover_object(obj_otyp);
        }
    } else {
        *str1 = strcpy((char *) alloc((unsigned)strlen(buf)+1), buf);
        discover_object(obj_otyp, FALSE, TRUE); /* possibly add to disco[] */
    }
}

void
namefloorobj()
{
    coord cc;
    int glyph;
    char buf[BUFSZ];
    struct obj *obj = 0;
    boolean fakeobj = FALSE, use_plural;

    cc.x = u.ux, cc.y = u.uy;
    /* "dot for under/over you" only makes sense when the cursor hasn't
       been moved off the hero's '@' yet, but there's no way to adjust
       the help text once getpos() has started */
    Sprintf(buf, "object on map (or '.' for one %s you)",
            (u.uundetected && hides_under(youmonst.data)) ? "over" : "under");
    if (getpos(&cc, FALSE, buf) < 0 || cc.x <= 0) {
        return;
    }
    if (cc.x == u.ux && cc.y == u.uy) {
        obj = vobj_at(u.ux, u.uy);
    } else {
        glyph = glyph_at(cc.x, cc.y);
        if (glyph_is_object(glyph)) {
            fakeobj = object_from_map(glyph, cc.x, cc.y, &obj);
        }
        /* else 'obj' stays null */
    }
    if (!obj) {
        /* "under you" is safe here since there's no object to hide under */
        pline("There doesn't seem to be any object %s.",
              (cc.x == u.ux && cc.y == u.uy) ? "under you" : "there");
        return;
    }
    /* note well: 'obj' might be an instance of STRANGE_OBJECT if target
       is a mimic; passing that to xname (directly or via simpleonames)
       would yield "glorkum" so we need to handle it explicitly; it will
       always fail the Hallucination test and pass the !callable test,
       resulting in the "can't be assigned a type name" message */
    Strcpy(buf, (obj->otyp != STRANGE_OBJECT) ? simpleonames(obj) : obj_descr[STRANGE_OBJECT].oc_name);
    use_plural = (obj->quan > 1L);
    if (Hallucination) {
        const char *unames[6];
        char tmpbuf[BUFSZ];

        /* straight role name */
        unames[0] = ((Upolyd ? u.mfemale : flags.female) && urole.name.f) ? urole.name.f : urole.name.m;
        /* random rank title for hero's role

           note: the 30 is hardcoded in xlev_to_rank, so should be
           hardcoded here too */
        unames[1] = rank_of(rn2(30) + 1, Role_switch, flags.female);
        /* random fake monster */
        unames[2] = rndmonnam();
        /* increased chance for fake monster */
        unames[3] = unames[2];
        /* traditional */
        unames[4] = roguename();
        /* silly */
        unames[5] = "Wibbly Wobbly";
        pline("%s %s to call you \"%s.\"",
              The(buf), use_plural ? "decide" : "decides",
              unames[rn2(SIZE(unames))]);
    } else if (!objtyp_is_callable(obj->otyp)) {
        pline("%s %s can't be assigned a type name.", use_plural ? "Those" : "That", buf);
    } else if (!obj->dknown) {
        You("don't know %s %s well enough to name %s.",
            use_plural ? "those" : "that", buf, use_plural ? "them" : "it");
    } else {
        docall(obj);
    }
    if (fakeobj) {
        obj->where = OBJ_FREE; /* object_from_map() sets it to OBJ_FLOOR */
        dealloc_obj(obj);
    }
}

static const char * const ghostnames[] = {
    /* these names should have length < PL_NSIZ */
    /* Capitalize the names for aesthetics -dgk */
    "Adri", "Andries", "Andreas", "Bert", "David", "Dirk", "Emile",
    "Frans", "Fred", "Greg", "Hether", "Jay", "John", "Jon", "Karnov",
    "Kay", "Kenny", "Kevin", "Maud", "Michiel", "Mike", "Peter", "Pipes", "Robert",
    "Ron", "Tom", "Wilmar", "Nick Danger", "Phoenix", "Jiro", "Mizue",
    "Stephan", "Lance Braccus", "Shadowhawk"
};

/* ghost names formerly set by x_monnam(), now by makemon() instead */
const char *
rndghostname()
{
    return rn2(7) ? ghostnames[rn2(SIZE(ghostnames))] : (const char *)plname;
}

/* Monster naming functions:
 * x_monnam is the generic monster-naming function.
 *        seen        unseen       detected       named
 * mon_nam: the newt    it  the invisible orc   Fido
 * noit_mon_nam:the newt (as if detected) the invisible orc Fido
 * l_monnam:    newt        it  invisible orc       dog called fido
 * Monnam:  The newt    It  The invisible orc   Fido
 * noit_Monnam: The newt (as if detected) The invisible orc Fido
 * Adjmonnam:   The poor newt   It  The poor invisible orc  The poor Fido
 * Amonnam: A newt      It  An invisible orc    Fido
 * a_monnam:    a newt      it  an invisible orc    Fido
 * m_monnam:    newt        xan orc         Fido
 * y_monnam:    your newt     your xan  your invisible orc  Fido
 */

/* Bug: if the monster is a priest or shopkeeper, not every one of these
 * options works, since those are special cases.
 */
char *
x_monnam(mtmp, article, adjective, suppress, called)
register struct monst *mtmp;
int article;
/* ARTICLE_NONE, ARTICLE_THE, ARTICLE_A: obvious
 * ARTICLE_YOUR: "your" on pets, "the" on everything else
 *
 * If the monster would be referred to as "it" or if the monster has a name
 * _and_ there is no adjective, "invisible", "saddled", etc., override this
 * and always use no article.
 */
const char *adjective;
int suppress;
/* SUPPRESS_IT, SUPPRESS_INVISIBLE, SUPPRESS_HALLUCINATION, SUPPRESS_SADDLE.
 * EXACT_NAME: combination of all the above
 * SUPPRESS_NAME: omit monster's assigned name (unless uniq w/ pname).
 */
boolean called;
{
    char *buf = nextmbuf();
    struct permonst *mdat = mtmp->data;
    const char *pm_name = mdat->mname;
    boolean do_hallu, do_invis, do_it, do_saddle, do_name;
    boolean name_at_start, has_adjectives;
    char *bp;

    if (program_state.gameover)
        suppress |= SUPPRESS_HALLUCINATION;
    if (article == ARTICLE_YOUR && !mtmp->mtame)
        article = ARTICLE_THE;

    do_hallu = Hallucination && !(suppress & SUPPRESS_HALLUCINATION);
    do_invis = mtmp->minvis && !(suppress & SUPPRESS_INVISIBLE);
    do_it = !canspotmon(mtmp) &&
            article != ARTICLE_YOUR &&
            !program_state.gameover &&
            mtmp != u.usteed &&
            !(u.uswallow && mtmp == u.ustuck) &&
            !(suppress & SUPPRESS_IT);
    do_saddle = !(suppress & SUPPRESS_SADDLE);
    do_name = !(suppress & SUPPRESS_NAME) || type_is_pname(mdat);

    buf[0] = '\0';

    /* unseen monsters, etc.  Use "it" */
    if (do_it) {
        Strcpy(buf, "it");
        return buf;
    }

    /* priests and minions: don't even use this function */
    if (mtmp->ispriest || mtmp->isminion) {
        char priestnambuf[BUFSZ];
        char *name;
        long save_prop = EHalluc_resistance;
        unsigned save_invis = mtmp->minvis;

        /* when true name is wanted, explicitly block Hallucination */
        if (!do_hallu) {
            EHalluc_resistance = 1L;
        }
        if (!do_invis) {
            mtmp->minvis = 0;
        }
        name = priestname(mtmp, priestnambuf);
        EHalluc_resistance = save_prop;
        mtmp->minvis = save_invis;
        if (article == ARTICLE_NONE && !strncmp(name, "the ", 4))
            name += 4;
        return strcpy(buf, name);
    }
    /* an "aligned priest" not flagged as a priest or minion should be
       "priest" or "priestess" (normally handled by priestname()) */
    if (mdat == &mons[PM_ALIGNED_PRIEST]) {
        pm_name = mtmp->female ? "priestess" : "priest";
    } else if (mdat == &mons[PM_HIGH_PRIEST] && mtmp->female) {
        pm_name = "high priestess";
    }

    /* Shopkeepers: use shopkeeper name.  For normal shopkeepers, just
     * "Asidonhopo"; for unusual ones, "Asidonhopo the invisible
     * shopkeeper" or "Asidonhopo the blue dragon".  If hallucinating,
     * none of this applies.
     */
    if (mtmp->isshk && !do_hallu
#ifdef BLACKMARKET
        && mtmp->data != &mons[PM_ONE_EYED_SAM]
#endif /* BLACKMARKET */
        ) {
        if (adjective && article == ARTICLE_THE) {
            /* pathological case: "the angry Asidonhopo the blue dragon"
               sounds silly */
            Strcpy(buf, "the ");
            Strcat(strcat(buf, adjective), " ");
            Strcat(buf, shkname(mtmp));
            return buf;
        }
        Strcat(buf, shkname(mtmp));
        if (mdat == &mons[PM_SHOPKEEPER] && !do_invis)
            return buf;
        Strcat(buf, " the ");
        if (do_invis)
            Strcat(buf, "invisible ");
        Strcat(buf, pm_name);
        return buf;
    }

    /* Put the adjectives in the buffer */
    if (adjective)
        Strcat(strcat(buf, adjective), " ");
    if (do_invis)
        Strcat(buf, "invisible ");
    if (do_saddle && (mtmp->misc_worn_check & W_SADDLE) &&
        !Blind && !Hallucination)
        Strcat(buf, "saddled ");
    has_adjectives = (buf[0] != '\0');

    /* Put the actual monster name or type into the buffer now.
       Remember whether the buffer starts with a personal name. */
    if (do_hallu) {
        Strcat(buf, rndmonnam());
        name_at_start = FALSE;
    } else if (do_name && has_mname(mtmp)) {
        char *name = MNAME(mtmp);

        if (mdat == &mons[PM_GHOST]) {
            Sprintf(eos(buf), "%s ghost", s_suffix(name));
            name_at_start = TRUE;
        } else if (called) {
            Sprintf(eos(buf), "%s called %s", pm_name, name);
            name_at_start = (boolean)type_is_pname(mdat);
        } else if (is_mplayer(mdat) && (bp = strstri(name, " the ")) != 0) {
            /* <name> the <adjective> <invisible> <saddled> <rank> */
            char pbuf[BUFSZ];

            Strcpy(pbuf, name);
            pbuf[bp - name + 5] = '\0'; /* adjectives right after " the " */
            if (has_adjectives)
                Strcat(pbuf, buf);
            Strcat(pbuf, bp + 5); /* append the rest of the name */
            Strcpy(buf, pbuf);
            article = ARTICLE_NONE;
            name_at_start = TRUE;
        } else {
            Strcat(buf, name);
            name_at_start = TRUE;
        }
    } else if (is_mplayer(mdat) && !In_endgame(&u.uz)) {
        char pbuf[BUFSZ];
        Strcpy(pbuf, rank_of((int)mtmp->m_lev,
                             monsndx(mdat),
                             (boolean)mtmp->female));
        Strcat(buf, lcase(pbuf));
        name_at_start = FALSE;
    } else if (is_rider(mtmp->data) &&
               (distu(mtmp->mx, mtmp->my) > 2) &&
               !(canseemon(mtmp)) &&
               /* for livelog reporting */
               !(suppress & SUPPRESS_IT)) {
        /* prevent the three horsemen to be identified from afar */
        Strcat(buf, "Rider");
        name_at_start = FALSE;
    } else {
        Strcat(buf, pm_name);
        name_at_start = (boolean)type_is_pname(mdat);
    }

    if (name_at_start && (article == ARTICLE_YOUR || !has_adjectives)) {
        if (mdat == &mons[PM_WIZARD_OF_YENDOR])
            article = ARTICLE_THE;
        else
            article = ARTICLE_NONE;
    } else if ((mdat->geno & G_UNIQ) && article == ARTICLE_A) {
        article = ARTICLE_THE;
    }

    {
        char buf2[BUFSZ];

        switch(article) {
        case ARTICLE_YOUR:
            Strcpy(buf2, "your ");
            Strcat(buf2, buf);
            Strcpy(buf, buf2);
            return buf;
        case ARTICLE_THE:
            Strcpy(buf2, "the ");
            Strcat(buf2, buf);
            Strcpy(buf, buf2);
            return buf;
        case ARTICLE_A:
            return(an(buf));
        case ARTICLE_NONE:
        default:
            return buf;
        }
    }
}

char *
l_monnam(mtmp)
struct monst *mtmp;
{
    return x_monnam(mtmp, ARTICLE_NONE, (char *) 0,
                    (has_mname(mtmp)) ? SUPPRESS_SADDLE : 0, TRUE);
}

char *
mon_nam(mtmp)
struct monst *mtmp;
{
    return x_monnam(mtmp, ARTICLE_THE, (char *) 0,
                    (has_mname(mtmp)) ? SUPPRESS_SADDLE : 0, FALSE);
}

/* print the name as if mon_nam() was called, but assume that the player
 * can always see the monster--used for probing and for monsters aggravating
 * the player with a cursed potion of invisibility
 */
char *
noit_mon_nam(mtmp)
struct monst *mtmp;
{
    return x_monnam(mtmp, ARTICLE_THE, (char *) 0,
                    (has_mname(mtmp)) ? (SUPPRESS_SADDLE | SUPPRESS_IT)
                                      : SUPPRESS_IT,
                    FALSE);
}

char *
Monnam(mtmp)
struct monst *mtmp;
{
    register char *bp = mon_nam(mtmp);

    *bp = highc(*bp);
    return(bp);
}

char *
noit_Monnam(mtmp)
struct monst *mtmp;
{
    register char *bp = noit_mon_nam(mtmp);

    *bp = highc(*bp);
    return(bp);
}

/* return "a dog" rather than "Fido", honoring hallucination and visibility */
char *
noname_monnam(mtmp, article)
struct monst *mtmp;
int article;
{
    return x_monnam(mtmp, article, (char *) 0, SUPPRESS_NAME, FALSE);
}

/* monster's own name -- overrides hallucination and [in]visibility
   so shouldn't be used in ordinary messages (mainly for disclosure) */
char *
m_monnam(mtmp)
struct monst *mtmp;
{
    return x_monnam(mtmp, ARTICLE_NONE, (char *)0, EXACT_NAME, FALSE);
}

/* pet name: "your little dog" */
char *
y_monnam(mtmp)
struct monst *mtmp;
{
    int prefix, suppression_flag;

    prefix = mtmp->mtame ? ARTICLE_YOUR : ARTICLE_THE;
    suppression_flag = (has_mname(mtmp) ||
                        /* "saddled" is redundant when mounted */
                        mtmp == u.usteed) ? SUPPRESS_SADDLE : 0;

    return x_monnam(mtmp, prefix, (char *)0, suppression_flag, FALSE);
}

char *
Adjmonnam(mtmp, adj)
struct monst *mtmp;
const char *adj;
{
    char *bp = x_monnam(mtmp, ARTICLE_THE, adj,
                        has_mname(mtmp) ? SUPPRESS_SADDLE : 0, FALSE);

    *bp = highc(*bp);
    return(bp);
}

char *
a_monnam(mtmp)
register struct monst *mtmp;
{
    return x_monnam(mtmp, ARTICLE_A, (char *)0,
                    has_mname(mtmp) ? SUPPRESS_SADDLE : 0, FALSE);
}

char *
Amonnam(mtmp)
struct monst *mtmp;
{
    char *bp = a_monnam(mtmp);

    *bp = highc(*bp);
    return(bp);
}

/* used for monster ID by the '/', ';', and 'C' commands to block remote
   identification of the endgame altars via their attending priests */
char *
distant_monnam(mon, article, outbuf)
struct monst *mon;
int article;    /* only ARTICLE_NONE and ARTICLE_THE are handled here */
char *outbuf;
{
    /* high priest(ess)'s identity is concealed on the Astral Plane,
       unless you're adjacent (overridden for hallucination which does
       its own obfuscation) */
    if (mon->data == &mons[PM_HIGH_PRIEST] && !Hallucination &&
        Is_astralevel(&u.uz) && distu(mon->mx, mon->my) > 2) {
        Strcpy(outbuf, article == ARTICLE_THE ? "the " : "");
        Strcat(outbuf, mon->female ? "high priestess" : "high priest");
    } else {
        Strcpy(outbuf, x_monnam(mon, article, (char *)0, 0, TRUE));
    }
    return outbuf;
}

/* returns mon_nam(mon) relative to other_mon; normal name unless they're
   the same, in which case the reference is to {him|her|it} self */
char *
mon_nam_too(mon, other_mon)
struct monst *mon, *other_mon;
{
    char *outbuf;

    if (mon != other_mon) {
        outbuf = mon_nam(mon);
    } else {
        outbuf = nextmbuf();
        switch (pronoun_gender(mon, FALSE)) {
        case 0:
            Strcpy(outbuf, "himself");
            break;
        case 1:
            Strcpy(outbuf, "herself");
            break;
        default:
            Strcpy(outbuf, "itself");
            break;
        }
    }
    return outbuf;
}

/* for debugging messages, where data might be suspect and we aren't
   taking what the hero does or doesn't know into consideration */
char *
minimal_monnam(mon, ckloc)
struct monst *mon;
boolean ckloc;
{
    struct permonst *ptr;
    char *outbuf = nextmbuf();

    if (!mon) {
        Strcpy(outbuf, "[Null monster]");
    } else if ((ptr = mon->data) == 0) {
        Strcpy(outbuf, "[Null mon->data]");
#if NEXT_VERSION
    } else if (ptr < &mons[0]) {
        Sprintf(outbuf, "[Invalid mon->data %s < %s]",
                fmt_ptr((genericptr_t) mon->data),
                fmt_ptr((genericptr_t) &mons[0]));
    } else if (ptr >= &mons[NUMMONS]) {
        Sprintf(outbuf, "[Invalid mon->data %s >= %s]",
                fmt_ptr((genericptr_t) mon->data),
                fmt_ptr((genericptr_t) &mons[NUMMONS]));
#endif
    } else if (ckloc && ptr == &mons[PM_LONG_WORM]
               && level.monsters[mon->mx][mon->my] != mon) {
        Sprintf(outbuf, "%s <%d,%d>",
                mons[PM_LONG_WORM_TAIL].mname, mon->mx, mon->my);
    } else {
        Sprintf(outbuf, "%s%s <%d,%d>",
                mon->mtame ? "tame " : mon->mpeaceful ? "peaceful " : "",
                mon->data->mname, mon->mx, mon->my);
        if (mon->cham != NON_PM)
            Sprintf(eos(outbuf), "{%s}", mons[mon->cham].mname);
    }
    return outbuf;
}

static const char * const bogusmons[] = {
    "jumbo shrimp", "giant pigmy", "gnu", "killer penguin",
    "giant cockroach", "giant slug", "maggot", "pterodactyl",
    "tyrannosaurus rex", "basilisk", "beholder", "nightmare",
    "efreeti", "marid", "rot grub", "bookworm", "master lichen",
    "shadow", "hologram", "jester", "attorney", "sleazoid",
    "killer tomato", "amazon", "robot", "battlemech",
    "rhinovirus", "harpy", "lion-dog", "rat-ant", "Y2K bug",
    /* misc. */
    "grue", "Christmas-tree monster", "luck sucker", "paskald",
    "brogmoid", "dornbeast",        /* Quendor (Zork, &c.) */
    "Ancient Multi-Hued Dragon", "Evil Iggy",
    /* Moria */
    "rattlesnake", "ice monster", "phantom",
    "quagga", "aquator", "griffin",
    "emu", "kestrel", "xeroc", "venus flytrap",
    /* Rogue V5 http://rogue.rogueforge.net/vade-mecum/ */
    "creeping coins",           /* Wizardry */
    "hydra", "siren",           /* Greek legend */
    "killer bunny",             /* Monty Python */
    "rodent of unusual size",       /* The Princess Bride */
    "Smokey the bear",          /* "Only you can prevent forest fires!" */
    "Luggage",              /* Discworld */
    "Ent",                  /* Lord of the Rings */
    "tangle tree", "nickelpede", "wiggle",  /* Xanth */
    "white rabbit", "snark",        /* Lewis Carroll */
    "pushmi-pullyu",            /* Dr. Dolittle */
    "smurf",                /* The Smurfs */
    "tribble", "Klingon", "Borg",       /* Star Trek */
    "Ewok",                 /* Star Wars */
    "Totoro",               /* Tonari no Totoro */
    "ohmu",                 /* Nausicaa */
    "youma",                /* Sailor Moon */
    "nyaasu",               /* Pokemon (Meowth) */
    "Godzilla", "King Kong",        /* monster movies */
    "earthquake beast",         /* old L of SH */
    "Invid",                /* Robotech */
    "Terminator",               /* The Terminator */
    "boomer",               /* Bubblegum Crisis */
    "Dalek",                /* Dr. Who ("Exterminate!") */
    "microscopic space fleet", "Ravenous Bugblatter Beast of Traal",
    /* HGttG */
    "teenage mutant ninja turtle",      /* TMNT */
    "samurai rabbit",           /* Usagi Yojimbo */
    "aardvark",             /* Cerebus */
    "Audrey II",                /* Little Shop of Horrors */
    "witch doctor", "one-eyed one-horned flying purple people eater",
    /* 50's rock 'n' roll */
    "Barney the dinosaur",          /* saccharine kiddy TV */
    "Morgoth",              /* Angband */
    "Vorlon",               /* Babylon 5 */
    "questing beast",           /* King Arthur */
    "Predator",             /* Movie */
    "mother-in-law",            /* common pest */
    /* from NAO */
    "one-winged dewinged stab-bat",     /* KoL */
    "praying mantis",
    "arch-pedant",
    "beluga whale",
    "bluebird of happiness",
    "bouncing eye", "floating nose", "wandering eye",
    "buffer overflow", "dangling pointer", "walking disk drive", "floating point",
    "cacodemon", "scrag",
    "cardboard golem", "duct tape golem",
    "chess pawn",
    "chicken",
    "chocolate pudding",
    "coelacanth",
    "corpulent porpoise",
    "Crow T. Robot",
    "diagonally moving grid bug",
    "dropbear",
    "Dudley",
    "El Pollo Diablo",
    "evil overlord",
    "existential angst",
    "figment of your imagination", "flash of insight",
    "flying pig",
    "gazebo",
    "gonzo journalist",
    "gray goo", "magnetic monopole",
    "heisenbug",
    "lag monster",
    "loan shark",
    "Lord British",
    "newsgroup troll",
    "ninja pirate zombie robot",
    "octarine dragon",
    "particle man",
    "possessed waffle iron",
    "poultrygeist",
    "raging nerd",
    "roomba",
    "sea cucumber",
    "spelling bee",
    "Strong Bad",
    "stuffed raccoon puppet",
    "tapeworm",
    "liger",
    "velociraptor",
    "vermicious knid",
    "viking",
    "voluptuous ampersand",
    "wee green blobbie",
    "wereplatypus",
    "zergling",
    "hag of bolding",
    "grind bug",
    "enderman",
    "wight supremacist",
    "Magical Trevor",
    "first category perpetual motion device",

    /* soundex and typos of monsters */
    "gloating eye",
    "flush golem",
    "martyr orc",
    "mortar orc",
    "acute blob",
    "aria elemental",
    "aliasing priest",
    "aligned parasite",
    "aligned parquet",
    "aligned proctor",
    "baby balky dragon",
    "baby blues dragon",
    "baby caricature",
    "baby crochet",
    "baby grainy dragon",
    "baby bong worm",
    "baby long word",
    "baby parable worm",
    "barfed devil",
    "beer wight",
    "boor wight",
    "brawny mold",
    "rave spider",
    "clue golem",
    "bust vortex",
    "errata elemental",
    "elastic eel",
    "electrocardiogram eel",
    "fir elemental",
    "tire elemental",
    "flamingo sphere",
    "fallacy golem",
    "frizzed centaur",
    "forest centerfold",
    "fierceness sphere",
    "frosted giant",
    "geriatric snake",
    "gnat ant",
    "giant bath",
    "grant beetle",
    "giant mango",
    "glossy golem",
    "gnome laureate",
    "gnome dummy",
    "gooier ooze",
    "green slide",
    "guardian nacho",
    "hell hound pun",
    "high purist",
    "hairnet devil",
    "ice trowel",
    "feather golem",
    "lounge worm",
    "mountain lymph",
    "pager golem",
    "pie fiend",
    "prophylactic worm",
    "sock mole",
    "rogue piercer",
    "seesawing sphere",
    "simile mimic",
    "moldier ant",
    "stain vortex",
    "scone giant",
    "umbrella hulk",
    "vampire mace",
    "verbal jabberwock",
    "water lemon",
    "winged grizzly",
    "yellow wight",

    /* from NetHack */
    "apostrophe golem", "Bob the angry flower",
    "bonsai-kitten", "Boxxy", "lonelygirl15",
    "tie-thulu", "Domo-kun", "nyan cat",
    "Zalgo", "common comma",
    "looooooooooooong cat",         /* internet memes */
    "bohrbug", "mandelbug", "schroedinbug", /* bugs */
    "Gerbenok",             /* Monty Python killer rabbit */
    "doenertier",               /* Erkan & Stefan */
    "Invisible Pink Unicorn",
    "Flying Spaghetti Monster",     /* deities */
    "Bluebear", "Professor Abdullah Nightingale",
    "Qwerty Uiop", "troglotroll",       /* Zamonien */
    "wolpertinger", "elwedritsche", "skvader",
    "Nessie", "tatzelwurm", "dahu",     /* european cryptids */
    "three-headed monkey",          /* Monkey Island */
    "little green man",         /* modern folklore */
    "weighted Companion Cube",  "defective turret", /* Portal */
    "/b/tard",              /* /b/ */
    "manbearpig",               /* South Park */
    "ceiling cat", "basement cat",
    "monorail cat",             /* the Internet is made for cat pix */
    "stoned golem", "wut golem", "nice golem",
    "flash golem",  "trash golem",          /* silly golems */
    "tridude",              /* POWDER */
    "orcus cosmicus",           /* Radomir Dopieralski */
    "yeek", "quylthulg",
    "Greater Hell Beast",           /* Angband */
    "Vendor of Yizard",         /* Souljazz */
    "Sigmund", "lernaean hydra", "Ijyb",
    "Gloorx Vloq", "Blork the orc",     /* Dungeon Crawl Stone Soup */
    "unicorn pegasus kitten",       /* Wil Wheaton, John Scalzi */
    "dwerga nethackus", "dwerga castrum",   /* Ask ASCII Ponies */
    "Irrenhaus the Third",          /* http://www.youtube.com/user/Irrenhaus3 */
    "semipotent demidog", "shale imp",  /* Homestuck */
    "mercury imp", "Betty Crocker",
    "Spades Slick",
    "patriarchy", "bourgeiose",     /* talking points */
    "mainstream media",
    "Demonhead Mobster Kingpin",        /* Problem Sleuth */
    "courtesan angel", "fractal bee",
    "Weasel King",
    "GLaDOS", "dangerous mute lunatic", /* Portal */
    "skag", "thresher", "LoaderBot", "varkid",  /* Borderlands */
    "Yarakeen", "Veil Warden",                       /*  War of Omens  */
    "Trogdor the Burninator",                        /* Homestar Runner */
    "Guy made of Bees", "Toot Oriole",               /* Kingdom of Loathing */
    "Magnus Burnsides the Fighter", "Taako the Wizard", "Merle Highchurch the Cleric", /* The Adventure Zone */
    "rubber duck", /* rubber ducks are a programmer's best friend */

    /* bogus NetHack monsters */
    "weeping angle",
    "gelatinous sphere", "gelatinous pyramid",
    "gelatinous Klein bottle", "gelatinous Mandelbrot set",
    "robot unicorn",

    /* Welcome to Night Vale*/
    "John Peters, you know, the farmer",

    /* Fallen London and Sunless Sea */
    "Lorn Fluke",
    "sorrow spider", "spider council",
    "chair of the department of antiquarian esquivalience",
    "the Starveling Cat",
    "the Tree of Ages",

    /* from Slashem Extended */
    "amphitheatre", /* evil patch idea by yasdorian */
    "banana peel golem", /* evil patch idea from DCSS */
    "dissolved undead potato",
    "dragonbreath nymph"
    "fart elemental", /* evil patch idea from DCSS */
    "floating nose", /* evil patch idea by K2 */
    "goldfish", /* evil patch idea from DCSS */
    "kamikaze tribble", /* evil patch idea by jonadab */
    "speedotrice", /* evil patch idea by jonadab */
    "techno ant",
    "drum stint reluctance",
    "tackle deice",
};


/* Return a random monster name, for hallucination.
 * KNOWN BUG: May be a proper name (Godzilla, Barney), may not
 * (the Terminator, a Dalek).  There's no elegant way to deal
 * with this without radically modifying the calling functions.
 */
const char *
rndmonnam()
{
    int name;

    do {
        name = rn1(SPECIAL_PM + SIZE(bogusmons) - LOW_PM, LOW_PM);
    } while (name < SPECIAL_PM &&
             (type_is_pname(&mons[name]) || (mons[name].geno & G_NOGEN)));

    if (name >= SPECIAL_PM) return bogusmons[name - SPECIAL_PM];
    return mons[name].mname;
}

/* check bogusmon prefix to decide whether it's a personal name */
boolean
bogon_is_pname(code)
char code;
{
    if (!code)
        return FALSE;
    return index("-+=", code) ? TRUE : FALSE;
}

#ifdef REINCARNATION
/* Name of a Rogue player */
const char *
roguename()
{
    char *i, *opts;

    if ((opts = nh_getenv("ROGUEOPTS")) != 0) {
        for (i = opts; *i; i++)
            if (!strncmp("name=", i, 5)) {
                char *j;
                if ((j = index(i+5, ',')) != 0)
                    *j = (char)0;
                return i+5;
            }
    }
    return rn2(3) ? (rn2(2) ? "Michael Toy" : "Kenneth Arnold")
           : "Glenn Wichman";
}
#endif /* REINCARNATION */

static NEARDATA const char * const hcolors[] = {
    "ultraviolet", "infrared", "bluish-orange",
    "reddish-green", "dark white", "light black", "sky blue-pink",
    "salty", "sweet", "sour", "bitter", "umami",
    "striped", "spiral", "swirly", "plaid", "checkered", "argyle",
    "paisley", "blotchy", "guernsey-spotted", "polka-dotted",
    "square", "round", "triangular",
    "cabernet", "sangria", "fuchsia", "wisteria",
    "lemon-lime", "strawberry-banana", "peppermint",
    "romantic", "incandescent",
    "octarine", /* Discworld: the Colour of Magic */
};

const char *
hcolor(colorpref)
const char *colorpref;
{
    return (Hallucination || !colorpref) ?
           hcolors[rn2(SIZE(hcolors))] : colorpref;
}

/* return a random real color unless hallucinating */
const char *
rndcolor()
{
    int k = rn2(CLR_MAX);

    return Hallucination ? hcolor((char *)0) : (k == NO_COLOR) ?
           "colorless" : c_obj_colors[k];
}

static NEARDATA const char *const hliquids[] = {
    "yoghurt", "oobleck", "clotted blood", "diluted water", "purified water",
    "instant coffee", "tea", "herbal infusion", "liquid rainbow",
    "creamy foam", "mulled wine", "bouillon", "nectar", "grog", "flubber",
    "ketchup", "slow light", "oil", "vinaigrette", "liquid crystal", "honey",
    "caramel sauce", "ink", "aqueous humour", "milk substitute",
    "fruit juice", "glowing lava", "gastric acid", "mineral water",
    "cough syrup", "quicksilver", "sweet vitriol", "grey goo", "pink slime",
};

const char *
hliquid(liquidpref)
const char *liquidpref;
{
    return (Hallucination || !liquidpref) ? hliquids[rn2(SIZE(hliquids))]
                                          : liquidpref;
}

/* Aliases for road-runner nemesis
 */
static const char * const coynames[] = {
    "Carnivorous Vulgaris", "Road-Runnerus Digestus",
    "Eatibus Anythingus", "Famishus-Famishus",
    "Eatibus Almost Anythingus", "Eatius Birdius",
    "Famishius Fantasticus", "Eternalii Famishiis",
    "Famishus Vulgarus", "Famishius Vulgaris Ingeniusi",
    "Eatius-Slobbius", "Hardheadipus Oedipus",
    "Carnivorous Slobbius", "Hard-Headipus Ravenus",
    "Evereadii Eatibus", "Apetitius Giganticus",
    "Hungrii Flea-Bagius", "Overconfidentii Vulgaris",
    "Caninus Nervous Rex", "Grotesques Appetitus",
    "Nemesis Ridiculii", "Canis latrans"
};

char *
coyotename(mtmp, buf)
struct monst *mtmp;
char *buf;
{
    if (mtmp && buf) {
        Sprintf(buf, "%s - %s",
                x_monnam(mtmp, ARTICLE_NONE, (char *)0, 0, TRUE),
                mtmp->mcan ? coynames[SIZE(coynames)-1]
                           : coynames[mtmp->m_id % (SIZE(coynames)-1)]);
    }
    return buf;
}

char *
rndorcname(s)
char *s;
{
    static const char *v[] = { "a", "ai", "og", "u" };
    static const char *snd[] = { "gor", "gris", "un", "bane", "ruk",
                                 "oth","ul", "z", "thos","akh","hai" };
    int i, iend = rn1(2, 3), vstart = rn2(2);

    if (s) {
        *s = '\0';
        for (i = 0; i < iend; ++i) {
            vstart = 1 - vstart;                /* 0 -> 1, 1 -> 0 */
            Sprintf(eos(s), "%s%s", (i > 0 && !rn2(30)) ? "-" : "",
                    vstart ? v[rn2(SIZE(v))] : snd[rn2(SIZE(snd))]);
        }
    }
    return s;
}

struct monst *
christen_orc(mtmp, gang, other)
struct monst *mtmp;
const char *gang, *other;
{
    int sz = 0;
    char buf[BUFSZ], buf2[BUFSZ], *orcname;

    orcname = rndorcname(buf2);
    sz = (int) strlen(orcname);
    if (gang) {
        sz += (int) (strlen(gang) + sizeof " of " - sizeof "");
    } else if (other) {
        sz += (int) strlen(other);
    }

    if (sz < BUFSZ) {
        char gbuf[BUFSZ];
        boolean nameit = FALSE;

        if (gang && orcname) {
            Sprintf(buf, "%s of %s", upstart(orcname),
                    upstart(strcpy(gbuf, gang)));
            nameit = TRUE;
        } else if (other && orcname) {
            Sprintf(buf, "%s%s", upstart(orcname), other);
            nameit = TRUE;
        }
        if (nameit) {
            mtmp = christen_monst(mtmp, buf);
        }
    }
    return mtmp;
}

/* make sure "The Colour of Magic" remains the first entry in here */
static const char *const sir_Terry_novels[] = {
    "The Colour of Magic", "The Light Fantastic", "Equal Rites", "Mort",
    "Sourcery", "Wyrd Sisters", "Pyramids", "Guards! Guards!", "Eric",
    "Moving Pictures", "Reaper Man", "Witches Abroad", "Small Gods",
    "Lords and Ladies", "Men at Arms", "Soul Music", "Interesting Times",
    "Maskerade", "Feet of Clay", "Hogfather", "Jingo", "The Last Continent",
    "Carpe Jugulum", "The Fifth Elephant", "The Truth", "Thief of Time",
    "The Last Hero", "The Amazing Maurice and His Educated Rodents",
    "Night Watch", "The Wee Free Men", "Monstrous Regiment",
    "A Hat Full of Sky", "Going Postal", "Thud!", "Wintersmith",
    "Making Money", "Unseen Academicals", "I Shall Wear Midnight", "Snuff",
    "Raising Steam", "The Shepherd's Crown"
};

const char *
noveltitle(novidx)
int *novidx;
{
    int j, k = SIZE(sir_Terry_novels);

    j = rn2(k);
    if (novidx) {
        if (*novidx == -1)
            *novidx = j;
        else if (*novidx >= 0 && *novidx < k)
            j = *novidx;
    }
    return sir_Terry_novels[j];
}

const char *
lookup_novel(lookname, idx)
const char *lookname;
int *idx;
{
    int k;

    /* Take American or U.K. spelling of this one */
    if (!strcmpi(The(lookname), "The Color of Magic"))
        lookname = sir_Terry_novels[0];

    for (k = 0; k < SIZE(sir_Terry_novels); ++k) {
        if (!strcmpi(lookname, sir_Terry_novels[k])
            || !strcmpi(The(lookname), sir_Terry_novels[k])) {
            if (idx)
                *idx = k;
            return sir_Terry_novels[k];
        }
    }
    /* name not found; if novelidx is already set, override the name */
    if (idx && *idx >= 0 && *idx < SIZE(sir_Terry_novels))
        return sir_Terry_novels[*idx];

    return (const char *) 0;
}

/* Return a string describing how wounded a monster is.
 * This is in do_name.c only because it gives us access to nextmbuf().
 */
char *
mon_wounds(mon)
struct monst *mon;
{
    char *outbuf;
    const struct permonst *mdat = mon->data;
    const char *adverb, *adjective;

    /* Cases where you can't see wounds. */
    if (!Role_if(PM_HEALER) ||
        !canseemon(mon) ||
        (u.uswallow && mon == u.ustuck && Blind)) {
        return NULL;
    }
    if (mon->mhp == mon->mhpmax || DEADMONSTER(mon)) {
        return NULL;
    }

    adjective = is_fleshy(mdat) ? "wounded" : "damaged";

    if (Hallucination) {
        const char *wound_adverbs[] = {
            "mildly", "mostly", "somewhat", "lightly", "massively", "extremely",
            "flagrantly", "flamboyantly", "supremely", "excessively", "truly",
            "terribly", "incredibly", "unbelievably", "obscenely", "insanely",
            "amazingly", "absolutely"
        };
        const char *wound_adjectives[] = {
            "bloodied", "bruised", "crushed", "marred", "blemished",
            "dinged up", "totaled", "wrecked", "beaten up", "defaced",
            "pulverized", "battered", "tarnished", "ruined", "mangled"
        };
        adverb = wound_adverbs[rn2(SIZE(wound_adverbs))];
        adjective = wound_adjectives[rn2(SIZE(wound_adjectives))];
    } else if (mon->mhp * 10 <= mon->mhpmax) {    /* <= 10%   */
        adverb = "almost";
        adjective = nonliving(mdat) ? "destroyed" : "dead";
    } else if (mon->mhp * 6 <= mon->mhpmax) {     /* <= 16.6% */
        adverb = "critically";
    } else if (mon->mhp * 4 <= mon->mhpmax) {     /* <= 25%   */
        adverb = "badly";
    } else if (mon->mhp * 3 <= mon->mhpmax) {     /* <= 33.3% */
        adverb = "seriously";
    } else if (mon->mhp * 4 <= mon->mhpmax * 3) { /* <= 75%   */
        adverb = "moderately";
    } else {                                      /*  < 100%  */
        adverb = "slightly";
    }

    outbuf = nextmbuf();
    Sprintf(outbuf, "%s %s", adverb, adjective);
    return outbuf;
}

/* Print a message indicating that mon is wounded.
 * It will only print something if the monster's wound adjective changes (e.g.
 * if it goes from slightly wounded to moderately wounded).
 * pre_wound_hp is its HP amount before being damaged in the caller of this
 * function; the caller must save its initial HP to pass through to this.
 */
void
print_mon_wounded(mon, pre_wound_hp)
struct monst *mon;
int pre_wound_hp;
{
    int mcurrhp;
    char *m_old_wounds, *m_curr_wounds;

    if (!flags.wounds) {
        return;
    }

    if (!mon) {
        impossible("print_mon_wounded: null mon!");
        return;
    }
    if (!Role_if(PM_HEALER)) {
        return;  /* optimization; redundant with healer check in mon_wounds */
    }
    if (mon->mhp == pre_wound_hp && !Hallucination) {
        /* impossible for adjective to change */
        return;
    }

    mcurrhp = mon->mhp;
    mon->mhp = pre_wound_hp;
    m_old_wounds = mon_wounds(mon);
    mon->mhp = mcurrhp;

    m_curr_wounds = mon_wounds(mon);

    if (m_curr_wounds != NULL &&
        (Hallucination ||
         m_old_wounds == NULL ||
         strcmp(m_old_wounds, m_curr_wounds))) {
        pline("%s is %s.", Monnam(mon), m_curr_wounds);
    }
}

/*do_name.c*/
