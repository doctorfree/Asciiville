/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "sp_lev.h"
#include "lev.h"    /* save & restore info */

/* from sp_lev.c, for fixup_special() */
extern lev_region *lregions;
extern int num_lregions;
/* for preserving the insect legs when wallifying baalz level */
static lev_region bughack = { { COLNO, ROWNO, 0, 0 }, { COLNO, ROWNO, 0, 0 }, 0, 0, 0, 0, { 0 } };
extern char SpLev_Map[COLNO][ROWNO];

static int FDECL(iswall, (int, int));
static int FDECL(iswall_or_stone, (int, int));
STATIC_DCL boolean FDECL(is_solid, (int, int));
STATIC_DCL int FDECL(extend_spine, (int [3][3], int, int, int));
STATIC_DCL boolean FDECL(okay, (int, int, int));
STATIC_DCL void FDECL(maze0xy, (coord *));
STATIC_DCL boolean FDECL(put_lregion_here, (XCHAR_P, XCHAR_P, XCHAR_P,
                                            XCHAR_P, XCHAR_P, XCHAR_P, XCHAR_P, BOOLEAN_P, d_level *, XCHAR_P));
STATIC_DCL void FDECL(move, (int *, int *, int));
STATIC_DCL void NDECL(setup_waterlevel);
STATIC_DCL void NDECL(unsetup_waterlevel);

static void FDECL(check_ransacked, (char *));
static void FDECL(migr_booty_item, (int, const char *));
static void FDECL(migrate_orc, (struct monst *, unsigned long));
static void NDECL(stolen_booty);

/* adjust a coordinate one step in the specified direction */
#define mz_move(X, Y, dir) \
    do {                                                         \
        switch (dir) {                                           \
        case 0:  --(Y);  break;                                  \
        case 1:  (X)++;  break;                                  \
        case 2:  (Y)++;  break;                                  \
        case 3:  --(X);  break;                                  \
        default: panic("mz_move: bad direction %d", dir);        \
        }                                                        \
    } while (0)

static int
iswall(x, y)
int x, y;
{
    int type;

    if (!isok(x, y)) {
        return 0;
    }
    type = levl[x][y].typ;
    return (IS_WALL(type) || IS_DOOR(type) ||
            type == SDOOR || type == IRONBARS);
}

static int
iswall_or_stone(x, y)
int x, y;
{
    /* out of bounds = stone */
    if (!isok(x, y)) {
        return 1;
    }

    return (levl[x][y].typ == STONE || iswall(x, y));
}

/* return TRUE if out of bounds, wall or rock */
STATIC_OVL boolean
is_solid(x, y)
int x, y;
{
    return (!isok(x, y) || IS_STWALL(levl[x][y].typ));
}

/*
 * Return 1 (not TRUE - we're doing bit vectors here) if we want to extend
 * a wall spine in the (dx,dy) direction.  Return 0 otherwise.
 *
 * To extend a wall spine in that direction, first there must be a wall there.
 * Then, extend a spine unless the current position is surrounded by walls
 * in the direction given by (dx,dy).  E.g. if 'x' is our location, 'W'
 * a wall, '.' a room, 'a' anything (we don't care), and our direction is
 * (0,1) - South or down - then:
 *
 *      a a a
 *      W x W       This would not extend a spine from x down
 *      W W W       (a corridor of walls is formed).
 *
 *      a a a
 *      W x W       This would extend a spine from x down.
 *      . W W
 */
STATIC_OVL int
extend_spine(locale, wall_there, dx, dy)
int locale[3][3];
int wall_there, dx, dy;
{
    int spine, nx, ny;

    nx = 1 + dx;
    ny = 1 + dy;

    if (wall_there) {   /* wall in that direction */
        if (dx) {
            if (locale[ 1][0] && locale[ 1][2] && /* EW are wall/stone */
                locale[nx][0] && locale[nx][2]) { /* diag are wall/stone */
                spine = 0;
            } else {
                spine = 1;
            }
        } else { /* dy */
            if (locale[0][ 1] && locale[2][ 1] && /* NS are wall/stone */
                locale[0][ny] && locale[2][ny]) { /* diag are wall/stone */
                spine = 0;
            } else {
                spine = 1;
            }
        }
    } else {
        spine = 0;
    }

    return spine;
}


/*
 * Wall cleanup.  This function has two purposes: (1) remove walls that
 * are totally surrounded by stone - they are redundant.  (2) correct
 * the types so that they extend and connect to each other.
 */
void
wallification(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
    uchar type;
    int x, y;
    struct rm *lev;

    /* sanity check on incoming variables */
    if (x1<0 || x2>=COLNO || x1>x2 || y1<0 || y2>=ROWNO || y1>y2)
        panic("wallification: bad bounds (%d,%d) to (%d,%d)", x1, y1, x2, y2);

    /* change walls surrounded by rock to rock. */
    for (x = x1; x <= x2; x++) {
        for (y = y1; y <= y2; y++) {
            lev = &levl[x][y];
            type = lev->typ;
            if (IS_WALL(type) && type != DBWALL) {
                if (is_solid(x-1, y-1) &&
                    is_solid(x-1, y  ) &&
                    is_solid(x-1, y+1) &&
                    is_solid(x,  y-1) &&
                    is_solid(x,  y+1) &&
                    is_solid(x+1, y-1) &&
                    is_solid(x+1, y  ) &&
                    is_solid(x+1, y+1))
                    lev->typ = STONE;
            }
        }
    }

    wall_extends(x1, y1, x2, y2);
}

void
wall_extends(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
    uchar type;
    int x, y;
    struct rm *lev;
    int bits;
    int locale[3][3];   /* rock or wall status surrounding positions */

    /*
     * Value 0 represents a free-standing wall.  It could be anything,
     * so even though this table says VWALL, we actually leave whatever
     * typ was there alone.
     */
    static xchar spine_array[16] = {
        VWALL,  HWALL,      HWALL,      HWALL,
        VWALL,  TRCORNER,   TLCORNER,   TDWALL,
        VWALL,  BRCORNER,   BLCORNER,   TUWALL,
        VWALL,  TLWALL,     TRWALL,     CROSSWALL
    };

    /* sanity check on incoming variables */
    if (x1<0 || x2>=COLNO || x1>x2 || y1<0 || y2>=ROWNO || y1>y2)
        panic("wall_extends: bad bounds (%d,%d) to (%d,%d)", x1, y1, x2, y2);

    /* set the correct wall type. */
    for (x = x1; x <= x2; x++) {
        for (y = y1; y <= y2; y++) {
            lev = &levl[x][y];
            type = lev->typ;
            if ( !(IS_WALL(type) && type != DBWALL)) continue;

            /* set the locations TRUE if rock or wall or out of bounds */
            locale[0][0] = iswall_or_stone(x-1, y-1);
            locale[1][0] = iswall_or_stone(  x, y-1);
            locale[2][0] = iswall_or_stone(x+1, y-1);

            locale[0][1] = iswall_or_stone(x-1,  y);
            locale[2][1] = iswall_or_stone(x+1,  y);

            locale[0][2] = iswall_or_stone(x-1, y+1);
            locale[1][2] = iswall_or_stone(  x, y+1);
            locale[2][2] = iswall_or_stone(x+1, y+1);

            /* determine if wall should extend to each direction NSEW */
            bits =   (extend_spine(locale, iswall(x, y-1),  0, -1) << 3)
                   | (extend_spine(locale, iswall(x, y+1),  0,  1) << 2)
                   | (extend_spine(locale, iswall(x+1, y),  1,  0) << 1)
                   |  extend_spine(locale, iswall(x-1, y), -1,  0);

            /* don't change typ if wall is free-standing */
            if (bits) lev->typ = spine_array[bits];
        }
    }
}

STATIC_OVL boolean
okay(x, y, dir)
int x, y;
int dir;
{
    move(&x, &y, dir);
    move(&x, &y, dir);
    if (x<3 || y<3 || x>x_maze_max || y>y_maze_max || levl[x][y].typ != STONE) {
        return(FALSE);
    }
    return(TRUE);
}

/* find random starting point for maze generation */
STATIC_OVL void
maze0xy(cc)
coord   *cc;
{
    cc->x = 3 + 2*rn2((x_maze_max>>1) - 1);
    cc->y = 3 + 2*rn2((y_maze_max>>1) - 1);
    return;
}

boolean
bad_location(x, y, lx, ly, hx, hy, lax)
xchar x, y;
xchar lx, ly, hx, hy;
xchar lax;
{
    return((boolean)(t_at(x, y) || invocation_pos(x, y) ||
                     within_bounded_area(x, y, lx, ly, hx, hy) ||
                     (lax < 2 && IS_FURNITURE(levl[x][y].typ)) ||
                     !((levl[x][y].typ == CORR && level.flags.is_maze_lev) ||
                       levl[x][y].typ == ROOM || levl[x][y].typ == AIR ||
                       (lax && (levl[x][y].typ == ICE || levl[x][y].typ == CLOUD)) ||
                       ((lax > 1) && (ACCESSIBLE(levl[x][y].typ) &&
                                      levl[x][y].typ != STAIRS &&
                                      levl[x][y].typ != LADDER)) ||
                       ((lax > 2) && (levl[x][y].typ == POOL || levl[x][y].typ == MOAT ||
                                      levl[x][y].typ == WATER || levl[x][y].typ == LAVAPOOL ||
                                      levl[x][y].typ == BOG))
                       )));
}

/** Pick a location in area (lx, ly, hx, hy) but not in (nlx, nly, nhx, nhy)
 * and place something (based on rtype) in that region.
 *
 * Returns TRUE if it could place the location. */
int
place_lregion(lx, ly, hx, hy, nlx, nly, nhx, nhy, rtype, lev)
xchar lx, ly, hx, hy;
xchar nlx, nly, nhx, nhy;
xchar rtype;
d_level *lev;
{
    int trycnt;
    boolean oneshot;
    xchar x, y;
    int lax = 0;

    if (!lx) { /* default to whole level */
        /*
         * if there are rooms and this a branch, let place_branch choose
         * the branch location (to avoid putting branches in corridors).
         */
        if (rtype == LR_BRANCH && nroom) {
            place_branch(Is_branchlev(&u.uz), 0, 0);
            return TRUE;
        }

        lx = 0; /* column 0 is not used */
        hx = COLNO - 1;
        ly = 0; /* 3.6.0 and earlier erroneously had 1 here */
        hy = ROWNO - 1;
    }

    do {
        /* first a probabilistic approach */
        oneshot = (lx == hx && ly == hy);
        for (trycnt = 0; trycnt < 200; trycnt++) {
            x = rn1((hx - lx) + 1, lx);
            y = rn1((hy - ly) + 1, ly);
            if (put_lregion_here(x, y, nlx, nly, nhx, nhy, rtype, oneshot, lev, lax))
                return TRUE;
        }

        /* then a deterministic one */
        oneshot = TRUE;
        for (x = lx; x <= hx; x++)
            for (y = ly; y <= hy; y++)
                if (put_lregion_here(x, y, nlx, nly, nhx, nhy, rtype, oneshot, lev, lax))
                    return TRUE;
    } while (lax++ < 4);

    return FALSE;
}

STATIC_OVL boolean
put_lregion_here(x, y, nlx, nly, nhx, nhy, rtype, oneshot, lev, lax)
xchar x, y;
xchar nlx, nly, nhx, nhy;
xchar rtype;
boolean oneshot;
d_level *lev;
xchar lax;
{
    struct monst *mtmp;

    if (bad_location(x, y, nlx, nly, nhx, nhy, lax)) {
        if (!oneshot) {
            return FALSE;   /* caller should try again */
        } else {
            /* Must make do with the only location possible;
               avoid failure due to a misplaced trap.
               It might still fail if there's a dungeon feature here. */
            struct trap *t = t_at(x, y);

            if (t && t->ttyp != MAGIC_PORTAL && t->ttyp != VIBRATING_SQUARE) {
                deltrap(t);
            }
            if (bad_location(x, y, nlx, nly, nhx, nhy, lax))
                return FALSE;
        }
    }

    switch (rtype) {
    case LR_TELE:
    case LR_UPTELE:
    case LR_DOWNTELE:
        /* "something" means the player in this case */
        if ((mtmp = m_at(x, y))) {
            /* move the monster if no choice, or just try again */
            if (oneshot) {
                if (!rloc(mtmp, TRUE)) {
                    m_into_limbo(mtmp);
                }
            } else {
                return FALSE;
            }
        }
        u_on_newpos(x, y);
        break;
    case LR_PORTAL:
        mkportal(x, y, lev->dnum, lev->dlevel);
        break;
    case LR_DOWNSTAIR:
    case LR_UPSTAIR:
        mkstairs(x, y, (char)rtype, (struct mkroom *)0);
        break;
    case LR_BRANCH:
        place_branch(Is_branchlev(&u.uz), x, y);
        break;
    }
    return(TRUE);
}

static boolean was_waterlevel; /* ugh... this shouldn't be needed */

/* fix up Baalzebub's lair, which depicts a level-sized beetle;
   its legs are walls within solid rock--regular wallification
   classifies them as superfluous and gets rid of them */
static void
baalz_fixup()
{
    struct monst *mtmp;
    int x, y, lastx, lasty;

    /*
     * baalz level's nondiggable region surrounds the "insect" and rooms.
     * The outermost perimeter of that region is subject to wall cleanup
     * (hence 'x + 1' and 'y + 1' for starting don't-clean column and row,
     * 'lastx - 1' and 'lasty - 1' for ending don't-clean column and row)
     * and the interior is protected against that (in wall_cleanup()).
     *
     * Assumes level.flags.corrmaze is TRUE, otherwise the bug legs will
     * have already been "cleaned" away by general wallification.
     */

    /* find low and high x for to-be-wallified portion of level */
    y = ROWNO / 2;
    for (lastx = x = 0; x < COLNO; ++x) {
        if ((levl[x][y].wall_info & W_NONDIGGABLE) != 0) {
            if (!lastx) {
                bughack.inarea.x1 = x + 1;
            }
            lastx = x;
        }
    }
    bughack.inarea.x2 = ((lastx > bughack.inarea.x1) ? lastx : x) - 1;
    /* find low and high y for to-be-wallified portion of level */
    x = bughack.inarea.x1;
    for (lasty = y = 0; y < ROWNO; ++y) {
        if ((levl[x][y].wall_info & W_NONDIGGABLE) != 0) {
            if (!lasty) {
                bughack.inarea.y1 = y + 1;
            }
            lasty = y;
        }
    }
    bughack.inarea.y2 = ((lasty > bughack.inarea.y1) ? lasty : y) - 1;
    /* two pools mark where special post-wallify fix-ups are needed */
    for (x = bughack.inarea.x1; x <= bughack.inarea.x2; ++x) {
        for (y = bughack.inarea.y1; y <= bughack.inarea.y2; ++y) {
            if (levl[x][y].typ == POOL) {
                levl[x][y].typ = HWALL;
                if (bughack.delarea.x1 == COLNO) {
                    bughack.delarea.x1 = x, bughack.delarea.y1 = y;
                } else {
                    bughack.delarea.x2 = x, bughack.delarea.y2 = y;
                }
            } else if (levl[x][y].typ == IRONBARS) {
                /* novelty effect; allowing digging in front of 'eyes' */
                levl[x - 1][y].wall_info &= ~W_NONDIGGABLE;
                if (isok(x - 2, y)) {
                    levl[x - 2][y].wall_info &= ~W_NONDIGGABLE;
                }
            }
        }
    }

    wallification(max(bughack.inarea.x1 - 2, 1),
                  max(bughack.inarea.y1 - 2, 0),
                  min(bughack.inarea.x2 + 2, COLNO - 1),
                  min(bughack.inarea.y2 + 2, ROWNO - 1));

    /* bughack hack for rear-most legs on baalz level; first joint on
       both top and bottom gets a bogus extra connection to room area,
       producing unwanted rectangles; change back to separated legs */
    x = bughack.delarea.x1, y = bughack.delarea.y1;
    if (isok(x, y) &&
         levl[x][y].typ == TLWALL &&
         isok(x, y + 1) &&
         levl[x][y + 1].typ == TUWALL) {
        levl[x][y].typ = BRCORNER;
        levl[x][y + 1].typ = HWALL;
        if ((mtmp = m_at(x, y)) != 0) {
            /* something at temporary pool... */
            (void) rloc(mtmp, FALSE);
        }
    }
    x = bughack.delarea.x2, y = bughack.delarea.y2;
    if (isok(x, y) &&
         levl[x][y].typ == TLWALL &&
         isok(x, y - 1) &&
         levl[x][y - 1].typ == TDWALL) {
        levl[x][y].typ = TRCORNER;
        levl[x][y - 1].typ = HWALL;
        if ((mtmp = m_at(x, y)) != 0) {
            /* something at temporary pool... */
            (void) rloc(mtmp, FALSE);
        }
    }

    /* reset bughack region; set low end to <COLNO,ROWNO> so that
       within_bounded_region() in fix_wall_spines() will fail
       most quickly--on its first test--when loading other levels */
    bughack.inarea.x1 = bughack.delarea.x1 = COLNO;
    bughack.inarea.y1 = bughack.delarea.y1 = ROWNO;
    bughack.inarea.x2 = bughack.delarea.x2 = 0;
    bughack.inarea.y2 = bughack.delarea.y2 = 0;
}

/* this is special stuff that the level compiler cannot (yet) handle */
void
fixup_special()
{
    lev_region *r = lregions;
    struct d_level lev;
    int x, y;
    struct mkroom *croom;
    boolean added_branch = FALSE;

    if (was_waterlevel) {
        was_waterlevel = FALSE;
        u.uinwater = 0;
        unsetup_waterlevel();
    } else if (Is_waterlevel(&u.uz)) {
        was_waterlevel = TRUE;
        /* water level is an odd beast - it has to be set up
           before calling place_lregions etc. */
        setup_waterlevel();
    }
    for (x = 0; x < num_lregions; x++, r++) {
        switch(r->rtype) {
        case LR_BRANCH:
            added_branch = TRUE;
            goto place_it;

        case LR_PORTAL:
            if (*r->rname.str >= '0' && *r->rname.str <= '9') {
                /* "chutes and ladders" */
                lev = u.uz;
                lev.dlevel = atoi(r->rname.str);
            } else {
#ifdef RANDOMIZED_PLANES
                s_level *sp;
                if (strcmp("random_plane", r->rname.str)==0) {
                    sp = get_next_elemental_plane(&u.uz);
                } else {
                    sp = find_level(r->rname.str);
                }
#else
                s_level *sp = find_level(r->rname.str);
#endif
                if (sp) {
                    lev = sp->dlevel;
                } else {
                    warning("Couldn't find_level \"%s\".", r->rname.str);
                    break;
                }
            }
            /* fall through */

        case LR_UPSTAIR:
        case LR_DOWNSTAIR:
place_it:
            if (!place_lregion(r->inarea.x1, r->inarea.y1,
                               r->inarea.x2, r->inarea.y2,
                               r->delarea.x1, r->delarea.y1,
                               r->delarea.x2, r->delarea.y2,
                               r->rtype, &lev)) {
                /* Couldn't place it in inarea, try again with
                 * the whole level. */
                if (!place_lregion(0, 0, 0, 0,
                                   r->delarea.x1, r->delarea.y1,
                                   r->delarea.x2, r->delarea.y2,
                                   r->rtype, &lev)) {
                    impossible("Couldn't place lregion type %d!", r->rtype);
                }
            }
            break;

        case LR_TELE:
        case LR_UPTELE:
        case LR_DOWNTELE:
            /* save the region outlines for goto_level() */
            if (r->rtype == LR_TELE || r->rtype == LR_UPTELE) {
                updest.lx = r->inarea.x1;
                updest.ly = r->inarea.y1;
                updest.hx = r->inarea.x2;
                updest.hy = r->inarea.y2;
                updest.nlx = r->delarea.x1;
                updest.nly = r->delarea.y1;
                updest.nhx = r->delarea.x2;
                updest.nhy = r->delarea.y2;
            }
            if (r->rtype == LR_TELE || r->rtype == LR_DOWNTELE) {
                dndest.lx = r->inarea.x1;
                dndest.ly = r->inarea.y1;
                dndest.hx = r->inarea.x2;
                dndest.hy = r->inarea.y2;
                dndest.nlx = r->delarea.x1;
                dndest.nly = r->delarea.y1;
                dndest.nhx = r->delarea.x2;
                dndest.nhy = r->delarea.y2;
            }
            /* place_lregion gets called from goto_level() */
            break;
        }

        if (r->rname.str) free((genericptr_t) r->rname.str),  r->rname.str = 0;
    }

    /* place dungeon branch if not placed above */
    if (!added_branch && Is_branchlev(&u.uz)) {
        place_lregion(0, 0, 0, 0, 0, 0, 0, 0, LR_BRANCH, (d_level *)0);
    }

    /* KMH -- Sokoban levels */
    if (In_sokoban(&u.uz)) {
        /* randomize Sokoban prize */
        if (dunlev(&u.uz)==1) {
            int price=0;
            register struct engr *ep;
            for (ep = head_engr; ep; ep = ep->nxt_engr) {
                /* Sokoban top levels have no random, burned engravings */
                if (ep && ep->engr_txt[0] && ep->engr_type == BURN &&
                    (!strcmp(ep->engr_txt, "Elbereth"))) {
                    int price_obj = STRANGE_OBJECT;
                    struct obj *otmp;
                    switch (price) {
                    case 0: price_obj = BAG_OF_HOLDING; break;
                    case 1: price_obj = rn2(2) ? CLOAK_OF_MAGIC_RESISTANCE : CLOAK_OF_DISPLACEMENT; break;
                    case 2: price_obj = !rn2(3) ? AMULET_OF_LIFE_SAVING : rn2(2) ? AMULET_OF_FLYING : AMULET_OF_REFLECTION; break;
                    }
                    otmp = mksobj_at(price_obj, ep->engr_x, ep->engr_y, TRUE, FALSE);
                    otmp->sokoprize = TRUE;
                    /* sokoban prizes are never cursed or
                     * have negative enchantment */
                    otmp->cursed = FALSE;
                    if (otmp->spe < 0) { otmp->spe = 0; }

                    price++;
#ifdef RECORD_ACHIEVE
                    if (otmp) otmp->record_achieve_special = 1;
#endif
                }
            }
        }
    }

    /* Still need to add some stuff to level file */
    if (Is_medusa_level(&u.uz)) {
        struct obj *otmp;
        int tryct;

        croom = &rooms[0]; /* only one room on the medusa level */
        for (tryct = rnd(4); tryct; tryct--) {
            x = somex(croom);
            y = somey(croom);
            if (goodpos(x, y, (struct monst *)0, 0)) {
                otmp = mk_tt_object(STATUE, x, y);
                while (otmp && (poly_when_stoned(&mons[otmp->corpsenm]) ||
                                pm_resistance(&mons[otmp->corpsenm], MR_STONE))) {
                    /* set_corpsenm() handles weight too */
                    set_corpsenm(otmp, rndmonnum());
                }
            }
        }

        if (rn2(2))
            otmp = mk_tt_object(STATUE, somex(croom), somey(croom));
        else /* Medusa statues don't contain books */
            otmp = mkcorpstat(STATUE, (struct monst *)0, (struct permonst *)0,
                              somex(croom), somey(croom), CORPSTAT_NONE);
        if (otmp) {
            while (pm_resistance(&mons[otmp->corpsenm], MR_STONE)
                   || poly_when_stoned(&mons[otmp->corpsenm])) {
                /* set_corpsenm() handles weight too */
                set_corpsenm(otmp, rndmonnum());
            }
        }

        /* Put a Fort Ludios vault on Medusa if it hasn't been generated before. */
        branch *br = dungeon_branch("Fort Ludios");
        d_level *not_existing = get_floating_branch(&knox_level, br);
        if (not_existing) {
            int x, y;
            /* Search for a suitable location for the portal.
             * Ignore the arrival and possible Medusa home. */
            for (x = 10; x < COLNO-10; x++) {
                for (y = 0; y < ROWNO; y++) {
                    if ((isok(x+1, y+1) && levl[x+1][y+1].typ == ROOM) &&
                        (isok(x+1, y+2) && levl[x+1][y+2].typ == ROOM) &&
                        (isok(x+2, y+1) && levl[x+2][y+1].typ == ROOM) &&
                        (isok(x+2, y+2) && levl[x+2][y+2].typ == ROOM)) {

                        /* count the room and water tiles */
                        int i, j;
                        int cnt_room = 0, cnt_water = 0;
                        for (i = x; i-x < 4; i++) {
                            for (j = y; j-y < 4; j++) {
                                if (isok(i, j)) {
                                    if (levl[i][j].typ == ROOM) {
                                        cnt_room++;
                                    }
                                    if (levl[i][j].typ == MOAT) {
                                        cnt_water++;
                                    }
                                }
                            }
                        }
                        /* only put the vault into a specific location */
                        if (cnt_room == 5 && cnt_water == 11) {
                            mk_knox_vault(x+1, y+1, 0, 0);

                            /* This vault is not very well hidden,
                             * add additional security for the gold. */
                            struct obj *gold = sobj_at(GOLD_PIECE, x+1, y+1);
                            obj_extract_self(gold);
                            struct obj *iron_safe = mksobj(IRON_SAFE, TRUE, FALSE);
                            add_to_container(iron_safe, gold);
                            place_object(iron_safe, x+1, y+1);


                            /* break out of loop */
                            x = COLNO;
                            y = ROWNO;
                        }
                    }
                }
            }
        }
    } else if (Is_wiz1_level(&u.uz)) {
        croom = search_special(MORGUE);

        create_secret_door(croom, W_EAST|W_WEST);
    } else if (Is_knox(&u.uz)) {
        /* using an unfilled morgue for rm id */
        croom = search_special(MORGUE);
        /* avoid inappropriate morgue-related messages */
        level.flags.graveyard = level.flags.has_morgue = 0;
        croom->rtype = OROOM; /* perhaps it should be set to VAULT? */
        /* stock the main vault */
        for (x = croom->lx; x <= croom->hx; x++) {
            for (y = croom->ly; y <= croom->hy; y++) {
                if (!is_solid(x, y)) {
                    (void) mkgold((long) rn1(300, 600), x, y);
                    if (!rn2(3) && !is_pool(x, y))
                        (void)maketrap(x, y, rn2(3) ? LANDMINE : SPIKED_PIT);
                }
            }
        }
    } else if (Is_sanctum(&u.uz)) {
        croom = search_special(TEMPLE);

        create_secret_door(croom, W_ANY);
    } else if (on_level(&u.uz, &orcus_level)) {
        struct monst *mtmp, *mtmp2;

        /* it's a ghost town, get rid of shopkeepers */
        for (mtmp = fmon; mtmp; mtmp = mtmp2) {
            mtmp2 = mtmp->nmon;
            if(mtmp->isshk) mongone(mtmp);
        }
    } else if (u.uz.dnum == mines_dnum && ransacked) {
       stolen_booty();
#ifdef ADVENT_CALENDAR
    } else if (Is_advent_calendar(&u.uz)) {
        fill_advent_calendar(TRUE);
#endif
    }

    if (lregions)
        free((genericptr_t) lregions),  lregions = 0;
    num_lregions = 0;
}

#ifdef ADVENT_CALENDAR
void
fill_advent_calendar(init)
boolean init;
{
    int door_nr=1;
    char buf[BUFSZ];
    int x, y;
    int in_x, in_y, out_x, out_y;

    for(x = 1; x < COLNO; x++) {
        for(y = 1; y < ROWNO; y++) {
            if (door_nr < 25 && isok(x, y) && IS_DOOR(levl[x][y].typ)) {
                if (y < 10) {
                    out_x = x; out_y = y+1; in_x = x; in_y = y-1;
                } else {
                    out_x = x; out_y = y-1; in_x = x; in_y = y+1;
                }
                if (init) {
                    sprintf(buf, "%d", door_nr);
                    /* place number in front of the door */
                    make_engr_at(out_x, out_y, buf, 0L, MARK);
                    if (door_nr == 24) {
                        int object = CANDY_BAR;
                        /* Christmas present! */
                        switch(rn2(15)) {
                        case  0: object = BAG_OF_HOLDING; break;
                        case  1: object = OILSKIN_SACK; break;
                        case  2: object = FIRE_HORN; break;
                        case  3: object = FROST_HORN; break;
                        case  4: object = MAGIC_FLUTE; break;
                        case  5: object = MAGIC_HARP; break;
                        case  6: object = DRUM_OF_EARTHQUAKE; break;
                        case  7: object = MAGIC_WHISTLE; break;
                        case  8: object = MAGIC_LAMP; break;
                        case  9: object = UNICORN_HORN; break;
                        case 10: object = BAG_OF_TRICKS; break;
                        case 11: object = EXPENSIVE_CAMERA; break;
                        case 12: object = HORN_OF_PLENTY; break;
                        case 13: object = STETHOSCOPE; break;
                        case 14: object = TINNING_KIT; break;
                        }
                        mksobj_at(object, in_x, in_y, TRUE, TRUE);
                    } else if (rn2(4)) {
                        mksobj_at((rn2(4)) ? CANDY_BAR : FORTUNE_COOKIE, in_x, in_y, FALSE, FALSE);
                    } else {
                        mkobj_at((rn2(4)) ? RING_CLASS : TOOL_CLASS, in_x, in_y, FALSE);
                    }
                }
                if (levl[x][y].doormask & D_LOCKED && getmonth()==12) {
                    if (getmday() == 24 && door_nr == 24) {
                        You_hear("a little bell ringing!");
                        levl[x][y].doormask = D_CLOSED;
                    } else if (getmday() == door_nr) {
                        You_hear("a door unlocking!");
                        levl[x][y].doormask = D_CLOSED;
                    }
                }
                door_nr++;
            }
        }
    }
}
#endif

static void
check_ransacked(s)
char *s;
{
    /* this kludge only works as long as orctown is minetn-1 */
    ransacked = (u.uz.dnum == mines_dnum && !strcmp(s, "minetn-1"));
}

#define ORC_LEADER 1
static const char *orcfruit[] = { "paddle cactus", "dwarven root" };

static void
migrate_orc(mtmp, mflags)
struct monst *mtmp;
unsigned long mflags;
{
    int nlev, max_depth, cur_depth;
    d_level dest;

    cur_depth = (int) depth(&u.uz);
    max_depth = dunlevs_in_dungeon(&u.uz) + (dungeons[u.uz.dnum].depth_start - 1);
    if (mflags == ORC_LEADER) {
        /* Note that the orc leader will take possession of any
         * remaining stuff not already delivered to other
         * orcs between here and the bottom of the mines.
         */
        nlev = max_depth;
        /* once in a blue moon, he won't be at the very bottom */
        if (!rn2(40)) {
            nlev--;
        }
        mtmp->mspare1 |= MIGR_LEFTOVERS;
    } else {
        nlev = rn2((max_depth - cur_depth) + 1) + cur_depth;
        if (nlev == cur_depth) {
            nlev++;
        }
        if (nlev > max_depth) {
            nlev = max_depth;
        }
        mtmp->mspare1 = (mtmp->mspare1 & ~MIGR_LEFTOVERS);
    }
    get_level(&dest, nlev);
    migrate_to_level(mtmp, ledger_no(&dest), MIGR_RANDOM, (coord *) 0);
}

void
shiny_orc_stuff(mtmp)
struct monst *mtmp;
{
    int gemprob, goldprob, otyp;
    struct obj *otmp;
    boolean is_captain = (mtmp->data == &mons[PM_ORC_CAPTAIN]);

    /* probabilities */
    goldprob = is_captain ? 600 : 300;
    gemprob = goldprob / 4;
    if (rn2(1000) < goldprob) {
        if ((otmp = mksobj(GOLD_PIECE, TRUE, FALSE)) != 0) {
            otmp->quan = 1L + rnd(goldprob);
            otmp->owt = weight(otmp);
            add_to_minv(mtmp, otmp);
        }
    }
    if (rn2(1000) < gemprob) {
        if ((otmp = mkobj(GEM_CLASS, FALSE)) != 0) {
            if (otmp->otyp == ROCK) {
                dealloc_obj(otmp);
            } else {
                add_to_minv(mtmp, otmp);
            }
        }
    }
    if (is_captain || !rn2(8)) {
        otyp = shiny_obj(RING_CLASS);
        if (otyp != STRANGE_OBJECT && (otmp = mksobj(otyp, TRUE, FALSE))) {
            add_to_minv(mtmp, otmp);
        }
    }
}
static void
migr_booty_item(otyp, gang)
int otyp;
const char *gang;
{
    struct obj *otmp;

    otmp = mksobj_migr_to_species(otyp, (unsigned long) M2_ORC, TRUE, FALSE);
    if (otmp && gang) {
        new_oname(otmp, strlen(gang) + 1); /* removes old name if present */
        Strcpy(ONAME(otmp), gang);
        if (otyp >= TRIPE_RATION && otyp <= TIN) {
            if (otyp == SLIME_MOLD) {
                otmp->spe = fruitadd((char *) orcfruit[rn2(SIZE(orcfruit))]);
            }
            otmp->quan += (long) rn2(3);
            otmp->owt = weight(otmp);
        }
    }
}

static void
stolen_booty()
{
    char *gang, gang_name[BUFSZ];
    struct monst *mtmp;
    int cnt, i, otyp;

    /*
     * --------------------------------------------------------
     * Mythos:
     *
     *      A tragic accident has occurred in Frontier Town...
     *      It has been overrun by orcs.
     *
     *      The booty that the orcs took from the town is now
     *      in the possession of the orcs that did this and
     *      have long since fled the level.
     * --------------------------------------------------------
     */

    gang = rndorcname(gang_name);
    /* create the stuff that the gang took */
    cnt = rnd(4);
    for (i = 0; i < cnt; ++i) {
        migr_booty_item(rn2(4) ? TALLOW_CANDLE : WAX_CANDLE, gang);
    }
    cnt = rnd(3);
    for (i = 0; i < cnt; ++i) {
        migr_booty_item(SKELETON_KEY, gang);
    }
    otyp = rn2((GAUNTLETS_OF_DEXTERITY - LEATHER_GLOVES) + 1) + LEATHER_GLOVES;
    migr_booty_item(otyp, gang);
    cnt = rnd(10);
    for (i = 0; i < cnt; ++i) {
        /* Food items - but no lembas! (or some other weird things) */
        otyp = rn2((TIN - TRIPE_RATION) + 1) + TRIPE_RATION;
        if (otyp != LEMBAS_WAFER &&
             otyp != MEAT_STICK &&
             otyp != MEATBALL &&
             otyp != MEAT_STICK &&
             otyp != MEAT_RING &&
             otyp != HUGE_CHUNK_OF_MEAT &&
             otyp != CORPSE) {
            migr_booty_item(otyp, gang);
        }
    }
    migr_booty_item(rn2(2) ? LONG_SWORD : SILVER_SABER, gang);
    /* create the leader of the orc gang */
    mtmp = makemon(&mons[PM_ORC_CAPTAIN], 0, 0, MM_NONAME);
    if (mtmp) {
        mtmp = christen_monst(mtmp, upstart(gang));
        mtmp->mpeaceful = 0;
        shiny_orc_stuff(mtmp);
        migrate_orc(mtmp, ORC_LEADER);
    }
    /* Make most of the orcs on the level be part of the invading gang */
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp)) {
            continue;
        }

        if (is_orc(mtmp->data) && !has_mname(mtmp) && rn2(10)) {
            /*
             * We'll consider the orc captain from the level
             * .des file to be the captain of a rival orc horde
             * who is there to see what has transpired, and to
             * contemplate future action.
             *
             * Don't christen the orc captain as a subordinate
             * member of the main orc horde.
             */
            if (mtmp->data != &mons[PM_ORC_CAPTAIN]) {
                mtmp = christen_orc(mtmp, upstart(gang), "");
            }
        }
    }
    /* Lastly, ensure there's several more orcs from the gang along the way.
     * The mechanics are such that they aren't actually identified as
     * members of the invading gang until they get their spoils assigned
     * to the inventory; handled during that assignment.
     */
    cnt = rn2(10) + 5;
    for (i = 0; i < cnt; ++i) {
        int mtyp;

        mtyp = rn2((PM_ORC_SHAMAN - PM_ORC) + 1) + PM_ORC;
        mtmp = makemon(&mons[mtyp], 0, 0, MM_NONAME);
        if (mtmp) {
            shiny_orc_stuff(mtmp);
            migrate_orc(mtmp, 0UL);
        }
    }
    ransacked = 0;
}

#undef ORC_LEADER

boolean
maze_inbounds(x, y)
int x, y;
{
    return (x >= 2 && y >= 2 &&
            x < x_maze_max && y < y_maze_max &&
            isok(x, y));
}

void
maze_remove_deadends(typ)
xchar typ;
{
    char dirok[4];
    int x, y, dir, idx, idx2, dx, dy, dx2, dy2;

    dirok[0] = 0; /* lint suppression */
    for (x = 2; x < x_maze_max; x++) {
        for (y = 2; y < y_maze_max; y++) {
            if (ACCESSIBLE(levl[x][y].typ) && (x % 2) && (y % 2)) {
                idx = idx2 = 0;
                for (dir = 0; dir < 4; dir++) {
                    /* note: mz_move() is a macro which modifies
                       one of its first two parameters */
                    dx = dx2 = x;
                    dy = dy2 = y;
                    mz_move(dx, dy, dir);
                    if (!maze_inbounds(dx, dy)) {
                        idx2++;
                        continue;
                    }
                    mz_move(dx2, dy2, dir);
                    mz_move(dx2, dy2, dir);
                    if (!maze_inbounds(dx2, dy2)) {
                        idx2++;
                        continue;
                    }
                    if (!ACCESSIBLE(levl[dx][dy].typ) && ACCESSIBLE(levl[dx2][dy2].typ)) {
                        dirok[idx++] = dir;
                        idx2++;
                    }
                }
                if (idx2 >= 3 && idx > 0) {
                    dx = x;
                    dy = y;
                    dir = dirok[rn2(idx)];
                    mz_move(dx, dy, dir);
                    levl[dx][dy].typ = typ;
                }
            }
        }
    }
}

/* Create a maze with specified corridor width and wall thickness
 * TODO: rewrite walkfrom so it works on temp space, not levl
 */
void
create_maze(corrwid, wallthick)
int corrwid;
int wallthick;
{
    int x,y;
    coord mm;
    int tmp_xmax = x_maze_max;
    int tmp_ymax = y_maze_max;
    int rdx = 0;
    int rdy = 0;
    int scale;

    if (wallthick < 1) {
        wallthick = 1;
    } else if (wallthick > 5) {
        wallthick = 5;
    }

    if (corrwid < 1) {
        corrwid = 1;
    } else if (corrwid > 5) {
        corrwid = 5;
    }

    scale = corrwid + wallthick;
    rdx = (x_maze_max / scale);
    rdy = (y_maze_max / scale);

    if (level.flags.corrmaze) {
        for (x = 2; x < (rdx * 2); x++) {
            for (y = 2; y < (rdy * 2); y++) {
                levl[x][y].typ = STONE;
            }
        }
    } else {
        for (x = 2; x <= (rdx * 2); x++) {
            for (y = 2; y <= (rdy * 2); y++) {
                levl[x][y].typ = ((x % 2) && (y % 2)) ? STONE : HWALL;
            }
        }
    }

    /* set upper bounds for maze0xy and walkfrom */
    x_maze_max = (rdx * 2);
    y_maze_max = (rdy * 2);

    /* create maze */
    maze0xy(&mm);
    walkfrom((int) mm.x, (int) mm.y, 0);

    if (!rn2(5)) {
        maze_remove_deadends((level.flags.corrmaze) ? CORR : ROOM);
    }

    /* restore bounds */
    x_maze_max = tmp_xmax;
    y_maze_max = tmp_ymax;

    /* scale maze up if needed */
    if (scale > 2) {
        char tmpmap[COLNO][ROWNO];
        int rx = 1, ry = 1;

        /* back up the existing smaller maze */
        for (x = 1; x < x_maze_max; x++) {
            for (y = 1; y < y_maze_max; y++) {
                tmpmap[x][y] = levl[x][y].typ;
            }
        }

        /* do the scaling */
        rx = x = 2;
        while (rx < x_maze_max) {
            int mx = (x % 2) ? corrwid
                             : ((x == 2 || x == (rdx * 2)) ? 1
                                                           : wallthick);
            ry = y = 2;
            while (ry < y_maze_max) {
                int dx = 0, dy = 0;
                int my = (y % 2) ? corrwid
                                 : ((y == 2 || y == (rdy * 2)) ? 1
                                                               : wallthick);
                for (dx = 0; dx < mx; dx++) {
                    for (dy = 0; dy < my; dy++) {
                        if (rx+dx >= x_maze_max || ry+dy >= y_maze_max) {
                            break;
                        }
                        levl[rx + dx][ry + dy].typ = tmpmap[x][y];
                    }
                }
                ry += my;
                y++;
            }
            rx += mx;
            x++;
        }
    }
}

void
makemaz(s)
const char *s;
{
    int x, y;
    char protofile[20];
    s_level *sp = Is_special(&u.uz);
    coord mm;

    if (*s) {
        if (sp && sp->rndlevs) {
            Sprintf(protofile, "%s-%d", s, rnd((int) sp->rndlevs));
        } else {
            Strcpy(protofile, s);
        }
    } else if (*(dungeons[u.uz.dnum].proto)) {
        if (dunlevs_in_dungeon(&u.uz) > 1) {
            if (sp && sp->rndlevs) {
                Sprintf(protofile, "%s%d-%d", dungeons[u.uz.dnum].proto,
                        dunlev(&u.uz),
                        rnd((int) sp->rndlevs));
            } else {
                Sprintf(protofile, "%s%d", dungeons[u.uz.dnum].proto,
                         dunlev(&u.uz));
            }
        } else if(sp && sp->rndlevs) {
            Sprintf(protofile, "%s-%d", dungeons[u.uz.dnum].proto,
                    rnd((int) sp->rndlevs));
        } else {
            Strcpy(protofile, dungeons[u.uz.dnum].proto);
        }
    } else {
        Strcpy(protofile, "");
    }

#ifdef WIZARD
    /* SPLEVTYPE format is "level-choice,level-choice"... */
    if (wizard && *protofile && sp && sp->rndlevs) {
        char *ep = getenv("SPLEVTYPE"); /* not nh_getenv */
        if (ep) {
            /* rindex always succeeds due to code in prior block */
            int len = (rindex(protofile, '-') - protofile) + 1;

            while (ep && *ep) {
                if (!strncmp(ep, protofile, len)) {
                    int pick = atoi(ep + len);
                    /* use choice only if valid */
                    if (pick > 0 && pick <= (int) sp->rndlevs)
                        Sprintf(protofile + len, "%d", pick);
                    break;
                } else {
                    ep = index(ep, ',');
                    if (ep) ++ep;
                }
            }
        }
    }
#endif

    if (*protofile) {
        check_ransacked(protofile);
        Strcat(protofile, LEV_EXT);
        in_mk_rndvault = FALSE;
        if (load_special(protofile)) {
            /* some levels can end up with monsters
               on dead mon list, including light source monsters */
            dmonsfree();
            return; /* no mazification right now */
        }
        impossible("Couldn't load \"%s\" - making a maze.", protofile);
    }

    level.flags.is_maze_lev = TRUE;

#ifndef WALLIFIED_MAZE
    for(x = 2; x < x_maze_max; x++)
        for(y = 2; y < y_maze_max; y++)
            levl[x][y].typ = STONE;
#else
    for(x = 2; x <= x_maze_max; x++)
        for(y = 2; y <= y_maze_max; y++)
            levl[x][y].typ = ((x % 2) && (y % 2)) ? STONE : HWALL;
#endif

    maze0xy(&mm);
    walkfrom((int) mm.x, (int) mm.y, 0);
    /* put a boulder at the maze center */
    (void) mksobj_at(BOULDER, (int) mm.x, (int) mm.y, TRUE, FALSE);

#ifdef WALLIFIED_MAZE
    wallification(2, 2, x_maze_max, y_maze_max);
#endif
    mazexy(&mm);
    mkstairs(mm.x, mm.y, 1, (struct mkroom *)0);        /* up */
    if (!Invocation_lev(&u.uz)) {
        mazexy(&mm);
        mkstairs(mm.x, mm.y, 0, (struct mkroom *)0);    /* down */
    } else {    /* choose "vibrating square" location */
#define x_maze_min 2
#define y_maze_min 2
        /*
         * Pick a position where the stairs down to Moloch's Sanctum
         * level will ultimately be created.  At that time, an area
         * will be altered:  walls removed, moat and traps generated,
         * boulders destroyed.  The position picked here must ensure
         * that that invocation area won't extend off the map.
         *
         * We actually allow up to 2 squares around the usual edge of
         * the area to get truncated; see mkinvokearea(mklev.c).
         */
#define INVPOS_X_MARGIN (6 - 2)
#define INVPOS_Y_MARGIN (5 - 2)
#define INVPOS_DISTANCE 11
        int x_range = x_maze_max - x_maze_min - 2*INVPOS_X_MARGIN - 1,
            y_range = y_maze_max - y_maze_min - 2*INVPOS_Y_MARGIN - 1;

#ifdef DEBUG
        if (x_range <= INVPOS_X_MARGIN || y_range <= INVPOS_Y_MARGIN ||
            (x_range * y_range) <= (INVPOS_DISTANCE * INVPOS_DISTANCE))
            panic("inv_pos: maze is too small! (%d x %d)",
                  x_maze_max, y_maze_max);
#endif
        inv_pos.x = inv_pos.y = 0; /*{occupied() => invocation_pos()}*/
        do {
            x = rn1(x_range, x_maze_min + INVPOS_X_MARGIN + 1);
            y = rn1(y_range, y_maze_min + INVPOS_Y_MARGIN + 1);
            /* we don't want it to be too near the stairs, nor
               to be on a spot that's already in use (wall|trap) */
        } while (x == xupstair || y == yupstair ||  /*(direct line)*/
                 abs(x - xupstair) == abs(y - yupstair) ||
                 distmin(x, y, xupstair, yupstair) <= INVPOS_DISTANCE ||
                 !SPACE_POS(levl[x][y].typ) || occupied(x, y));
        inv_pos.x = x;
        inv_pos.y = y;

        /* "'X' never, ever marks the spot" */
        if (Role_if(PM_ARCHEOLOGIST))
            make_engr_at(x, y, "X", 0L, DUST);
#undef INVPOS_X_MARGIN
#undef INVPOS_Y_MARGIN
#undef INVPOS_DISTANCE
#undef x_maze_min
#undef y_maze_min
    }

    /* place branch stair or portal */
    place_branch(Is_branchlev(&u.uz), 0, 0);

    for(x = rn1(8, 11); x; x--) {
        mazexy(&mm);
        (void) mkobj_at(rn2(2) ? GEM_CLASS : 0, mm.x, mm.y, TRUE);
    }
    for(x = rn1(10, 2); x; x--) {
        mazexy(&mm);
        (void) mksobj_at(BOULDER, mm.x, mm.y, TRUE, FALSE);
    }
    for (x = rn2(3); x; x--) {
        mazexy(&mm);
        (void) makemon(&mons[PM_MINOTAUR], mm.x, mm.y, NO_MM_FLAGS);
    }
    for(x = rn1(5, 7); x; x--) {
        mazexy(&mm);
        (void) makemon((struct permonst *) 0, mm.x, mm.y, NO_MM_FLAGS);
    }
    for(x = rn1(6, 7); x; x--) {
        mazexy(&mm);
        (void) mkgold(0L, mm.x, mm.y);
    }
    for(x = rn1(6, 7); x; x--)
        mktrap(0, 1, (struct mkroom *) 0, (coord*) 0);
}

#ifdef MICRO
/* Make the mazewalk iterative by faking a stack.  This is needed to
 * ensure the mazewalk is successful in the limited stack space of
 * the program.  This iterative version uses the minimum amount of stack
 * that is totally safe.
 */
void
walkfrom(x, y, typ)
int x, y;
schar typ;
{
#define CELLS (ROWNO * COLNO) / 4       /* a maze cell is 4 squares */
    char mazex[CELLS + 1], mazey[CELLS + 1];    /* char's are OK */
    int q, a, dir, pos;
    int dirs[4];

    if (!typ) {
        if (level.flags.corrmaze) {
            typ = CORR;
        } else {
            typ = ROOM;
        }
    }

    pos = 1;
    mazex[pos] = (char) x;
    mazey[pos] = (char) y;
    while (pos) {
        x = (int) mazex[pos];
        y = (int) mazey[pos];
        if(!IS_DOOR(levl[x][y].typ)) {
            /* might still be on edge of MAP, so don't overwrite */
            levl[x][y].typ = typ;
            levl[x][y].flags = 0;
            SpLev_Map[x][y] = 1;
        }
        q = 0;
        for (a = 0; a < 4; a++)
            if(okay(x, y, a)) dirs[q++]= a;
        if (!q)
            pos--;
        else {
            dir = dirs[rn2(q)];
            move(&x, &y, dir);
            levl[x][y].typ = typ;
            SpLev_Map[x][y] = 1;
            move(&x, &y, dir);
            SpLev_Map[x][y] = 1;
            pos++;
            if (pos > CELLS)
                panic("Overflow in walkfrom");
            mazex[pos] = (char) x;
            mazey[pos] = (char) y;
        }
    }
}
#else

void
walkfrom(x, y, typ)
int x, y;
schar typ;
{
    int q, a, dir;
    int dirs[4];

    if (!typ) {
        if (level.flags.corrmaze) {
            typ = CORR;
        } else {
            typ = ROOM;
        }
    }

    if (!IS_DOOR(levl[x][y].typ)) {
        /* might still be on edge of MAP, so don't overwrite */
        levl[x][y].typ = typ;
        levl[x][y].flags = 0;
        SpLev_Map[x][y] = 1;
    }

    while (1) {
        q = 0;
        for(a = 0; a < 4; a++)
            if(okay(x, y, a)) dirs[q++]= a;
        if(!q) return;
        dir = dirs[rn2(q)];
        move(&x, &y, dir);
        levl[x][y].typ = typ;
        SpLev_Map[x][y] = 1;
        move(&x, &y, dir);
        SpLev_Map[x][y] = 1;
        walkfrom(x, y, typ);
    }
}
#endif /* MICRO */

STATIC_OVL void
move(x, y, dir)
register int *x, *y;
register int dir;
{
    switch(dir) {
    case 0: --(*y); break;
    case 1: (*x)++; break;
    case 2: (*y)++; break;
    case 3: --(*x); break;
    default: panic("move: bad direction");
    }
}

/* find random point in generated corridors,
   so we don't create items in moats, bunkers, or walls */
void
mazexy(cc)
coord *cc;
{
    int cpt=0;

    do {
        cc->x = 3 + 2*rn2((x_maze_max>>1) - 1);
        cc->y = 3 + 2*rn2((y_maze_max>>1) - 1);
        cpt++;
    } while (cpt < 100 &&
             levl[cc->x][cc->y].typ != (level.flags.corrmaze ? CORR : ROOM));

    if (cpt >= 100) {
        int x, y;
        /* last try */
        for (x = 0; x < (x_maze_max>>1) - 1; x++)
            for (y = 0; y < (y_maze_max>>1) - 1; y++) {
                cc->x = 3 + 2 * x;
                cc->y = 3 + 2 * y;
                if (levl[cc->x][cc->y].typ == (level.flags.corrmaze ? CORR : ROOM)) {
                    return;
                }
            }
        panic("mazexy: can't find a place!");
    }
    return;
}

void
get_level_extends(int *left, int *top, int *right, int *bottom)
{
    int x, y;
    unsigned typ;
    struct rm *lev;
    boolean found, nonwall;
    int xmin, xmax, ymin, ymax;

    found = nonwall = FALSE;
    for (xmin = 0; !found && xmin <= COLNO; xmin++) {
        lev = &levl[xmin][0];
        for (y = 0; y <= ROWNO-1; y++, lev++) {
            typ = lev->typ;
            if (typ != STONE) {
                found = TRUE;
                if (!IS_WALL(typ)) {
                    nonwall = TRUE;
                }
            }
        }
    }
    xmin -= (nonwall || !level.flags.is_maze_lev) ? 2 : 1;
    if (xmin < 0) {
        xmin = 0;
    }

    found = nonwall = FALSE;
    for (xmax = COLNO - 1; !found && xmax >= 0; xmax--) {
        lev = &levl[xmax][0];
        for (y = 0; y <= ROWNO - 1; y++, lev++) {
            typ = lev->typ;
            if (typ != STONE) {
                found = TRUE;
                if (!IS_WALL(typ)) {
                    nonwall = TRUE;
                }
            }
        }
    }
    xmax += (nonwall || !level.flags.is_maze_lev) ? 2 : 1;
    if (xmax >= COLNO) {
        xmax = COLNO - 1;
    }

    found = nonwall = FALSE;
    for (ymin = 0; !found && ymin <= ROWNO; ymin++) {
        lev = &levl[xmin][ymin];
        for (x = xmin; x <= xmax; x++, lev += ROWNO) {
            typ = lev->typ;
            if (typ != STONE) {
                found = TRUE;
                if (!IS_WALL(typ)) {
                    nonwall = TRUE;
                }
            }
        }
    }
    ymin -= (nonwall || !level.flags.is_maze_lev) ? 2 : 1;

    found = nonwall = FALSE;
    for (ymax = ROWNO - 1; !found && ymax >= 0; ymax--) {
        lev = &levl[xmin][ymax];
        for (x = xmin; x <= xmax; x++, lev += ROWNO) {
            typ = lev->typ;
            if (typ != STONE) {
                found = TRUE;
                if (!IS_WALL(typ)) {
                    nonwall = TRUE;
                }
            }
        }
    }
    ymax += (nonwall || !level.flags.is_maze_lev) ? 2 : 1;

    *left = xmin;
    *right = xmax;
    *top = ymin;
    *bottom = ymax;
}

/* put a non-diggable boundary around the initial portion of a level map.
 * assumes that no level will initially put things beyond the isok() range.
 *
 * we can't bound unconditionally on the last line with something in it,
 * because that something might be a niche which was already reachable,
 * so the boundary would be breached
 *
 * we can't bound unconditionally on one beyond the last line, because
 * that provides a window of abuse for WALLIFIED_MAZE special levels
 */
void
bound_digging()
{
    int x, y;
    int xmin, xmax, ymin, ymax;

    if (Is_earthlevel(&u.uz)) {
        return; /* everything diggable here */
    }

    get_level_extends(&xmin, &ymin, &xmax, &ymax);

    for (x = 0; x < COLNO; x++) {
        for (y = 0; y < ROWNO; y++) {
            if (y <= ymin || y >= ymax || x <= xmin || x >= xmax) {
                levl[x][y].wall_info |= W_NONDIGGABLE;
            }
        }
    }
}

void
mkportal(x, y, todnum, todlevel)
xchar x, y, todnum, todlevel;
{
    /* a portal "trap" must be matched by a */
    /* portal in the destination dungeon/dlevel */
    struct trap *ttmp = maketrap(x, y, MAGIC_PORTAL);

    if (!ttmp) {
        impossible("portal on top of portal??");
        return;
    }
#ifdef DEBUG
    pline("mkportal: at (%d,%d), to %s, level %d",
          x, y, dungeons[todnum].dname, todlevel);
#endif
    ttmp->dst.dnum = todnum;
    ttmp->dst.dlevel = todlevel;
    return;
}

void
fumaroles()
{
    xchar n;
    boolean snd = FALSE, loud = FALSE;

    for (n = rn2(3) + 2; n; n--) {
        xchar x = rn1(COLNO - 4, 3);
        xchar y = rn1(ROWNO - 4, 3);

        if (levl[x][y].typ == LAVAPOOL) {
            NhRegion *r = create_gas_cloud(x, y, 4+rn2(5), rn1(10, 5), rn1(3,4));

            clear_heros_fault(r);
            snd = TRUE;
            if (distu(x, y) < 15) {
                loud = TRUE;
            }
        }
    }
    if (snd && !Deaf) {
        Norep("You hear a %swhoosh!", loud ? "loud " : "");  /* Deaf-aware */
    }
}

/*
 * Special waterlevel stuff in endgame (TH).
 *
 * Some of these functions would probably logically belong to some
 * other source files, but they are all so nicely encapsulated here.
 */

/* to ease the work of debuggers at this stage */
#define register

#define CONS_OBJ   0
#define CONS_MON   1
#define CONS_HERO  2
#define CONS_TRAP  3

static struct bubble *bbubbles, *ebubbles;

static struct trap *wportal;
static int xmin, ymin, xmax, ymax;  /* level boundaries */
/* bubble movement boundaries */
#define bxmin (xmin + 1)
#define bymin (ymin + 1)
#define bxmax (xmax - 1)
#define bymax (ymax - 1)

STATIC_DCL void NDECL(set_wportal);
STATIC_DCL void FDECL(mk_bubble, (int, int, int));
STATIC_DCL void FDECL(mv_bubble, (struct bubble *, int, int, BOOLEAN_P));

void
movebubbles()
{
    static boolean up = FALSE;
    struct bubble *b;
    int x, y, i, j;
    struct trap *btrap;
    static const struct rm water_pos =
    { cmap_to_glyph(S_water), WATER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    /* set up the portal the first time bubbles are moved */
    if (!wportal) set_wportal();

    vision_recalc(2);
    /* keep attached ball&chain separate from bubble objects */
    if (Punished) unplacebc();

    /*
     * Pick up everything inside of a bubble then fill all bubble
     * locations.
     */

    for (b = up ? bbubbles : ebubbles; b; b = up ? b->next : b->prev) {
        if (b->cons) panic("movebubbles: cons != null");
        for (i = 0, x = b->x; i < (int) b->bm[0]; i++, x++)
            for (j = 0, y = b->y; j < (int) b->bm[1]; j++, y++)
                if (b->bm[j + 2] & (1 << i)) {
                    if (!isok(x, y)) {
                        impossible("movebubbles: bad pos (%d,%d)", x, y);
                        continue;
                    }

                    /* pick up objects, monsters, hero, and traps */
                    if (OBJ_AT(x, y)) {
                        struct obj *olist = (struct obj *) 0, *otmp;
                        struct container *cons = (struct container *)
                                                 alloc(sizeof(struct container));

                        while ((otmp = level.objects[x][y]) != 0) {
                            remove_object(otmp);
                            otmp->ox = otmp->oy = 0;
                            otmp->nexthere = olist;
                            olist = otmp;
                        }

                        cons->x = x;
                        cons->y = y;
                        cons->what = CONS_OBJ;
                        cons->list = (genericptr_t) olist;
                        cons->next = b->cons;
                        b->cons = cons;
                    }
                    if (MON_AT(x, y)) {
                        struct monst *mon = m_at(x, y);
                        struct container *cons = (struct container *)
                                                 alloc(sizeof(struct container));

                        cons->x = x;
                        cons->y = y;
                        cons->what = CONS_MON;
                        cons->list = (genericptr_t) mon;

                        cons->next = b->cons;
                        b->cons = cons;

                        if(mon->wormno)
                            remove_worm(mon);
                        else
                            remove_monster(x, y);

                        newsym(x, y); /* clean up old position */
                        mon->mx = mon->my = 0;
                    }
                    if (!u.uswallow && x == u.ux && y == u.uy) {
                        struct container *cons = (struct container *)
                                                 alloc(sizeof(struct container));

                        cons->x = x;
                        cons->y = y;
                        cons->what = CONS_HERO;
                        cons->list = (genericptr_t) 0;

                        cons->next = b->cons;
                        b->cons = cons;
                    }
                    if ((btrap = t_at(x, y)) != 0) {
                        struct container *cons = (struct container *)
                                                 alloc(sizeof(struct container));

                        cons->x = x;
                        cons->y = y;
                        cons->what = CONS_TRAP;
                        cons->list = (genericptr_t) btrap;

                        cons->next = b->cons;
                        b->cons = cons;
                    }

                    levl[x][y] = water_pos;
                    block_point(x, y);
                }
    }

    /*
     * Every second time traverse down.  This is because otherwise
     * all the junk that changes owners when bubbles overlap
     * would eventually end up in the last bubble in the chain.
     */

    up = !up;
    for (b = up ? bbubbles : ebubbles; b; b = up ? b->next : b->prev) {
        int rx = rn2(3), ry = rn2(3);

        mv_bubble(b, b->dx + 1 - (!b->dx ? rx : (rx ? 1 : 0)),
                  b->dy + 1 - (!b->dy ? ry : (ry ? 1 : 0)),
                  FALSE);
    }

    /* put attached ball&chain back */
    if (Punished) placebc();
    vision_full_recalc = 1;
}

/* when moving in water, possibly (1 in 3) alter the intended destination */
void
water_friction()
{
    int x, y, dx, dy;
    boolean eff = FALSE;

    if (Swimming && rn2(4))
        return;     /* natural swimmers have advantage */

    if (u.dx && !rn2(!u.dy ? 3 : 6)) {  /* 1/3 chance or half that */
        /* cancel delta x and choose an arbitrary delta y value */
        x = u.ux;
        do {
            dy = rn2(3) - 1;        /* -1, 0, 1 */
            y = u.uy + dy;
        } while (dy && (!isok(x, y) || !is_pool(x, y)));
        u.dx = 0;
        u.dy = dy;
        eff = TRUE;
    } else if (u.dy && !rn2(!u.dx ? 3 : 5)) {   /* 1/3 or 1/5*(5/6) */
        /* cancel delta y and choose an arbitrary delta x value */
        y = u.uy;
        do {
            dx = rn2(3) - 1;        /* -1 .. 1 */
            x = u.ux + dx;
        } while (dx && (!isok(x, y) || !is_pool(x, y)));
        u.dy = 0;
        u.dx = dx;
        eff = TRUE;
    }
    if (eff) pline("Water turbulence affects your movements.");
}

void
save_waterlevel(fd, mode)
int fd, mode;
{
    struct bubble *b;

    if (!Is_waterlevel(&u.uz)) return;

    if (perform_bwrite(mode)) {
        int n = 0;
        for (b = bbubbles; b; b = b->next) ++n;
        bwrite(fd, &n, sizeof n);
        bwrite(fd, &xmin, sizeof xmin);
        bwrite(fd, &ymin, sizeof ymin);
        bwrite(fd, &xmax, sizeof xmax);
        bwrite(fd, &ymax, sizeof ymax);
        for (b = bbubbles; b; b = b->next)
            bwrite(fd, b, sizeof *b);
    }
    if (release_data(mode))
        unsetup_waterlevel();
}

void
restore_waterlevel(fd)
int fd;
{
    struct bubble *b = (struct bubble *)0, *btmp;
    int i, n;

    if (!Is_waterlevel(&u.uz)) return;

    set_wportal();
    mread(fd, &n, sizeof n);
    mread(fd, &xmin, sizeof xmin);
    mread(fd, &ymin, sizeof ymin);
    mread(fd, &xmax, sizeof xmax);
    mread(fd, &ymax, sizeof ymax);
    for (i = 0; i < n; i++) {
        btmp = b;
        b = (struct bubble *) alloc(sizeof *b);
        mread(fd, b, sizeof *b);
        if (bbubbles) {
            btmp->next = b;
            b->prev = btmp;
        } else {
            bbubbles = b;
            b->prev = (struct bubble *)0;
        }
        mv_bubble(b, 0, 0, TRUE);
    }
    ebubbles = b;
    b->next = (struct bubble *)0;
    was_waterlevel = TRUE;
}

const char *
waterbody_name(x, y)
xchar x, y;
{
    struct rm *lev;
    schar ltyp;

    if (!isok(x, y))
        return "drink";     /* should never happen */

    lev = &levl[x][y];
    ltyp = lev->typ;
    if (ltyp == DRAWBRIDGE_UP) {
        ltyp = db_under_typ(lev);
    }

    if (is_lava(x, y)) {
        return hliquid("lava");
    } else if (ltyp == ICE) {
        return "ice";
    } else if (ltyp == POOL) {
        return "pool of water";
    } else if (ltyp == BOG) {
        return "swamp";
    } else if (ltyp == WATER || Is_waterlevel(&u.uz)) {
        ; /* fall through to default return value */
    } else if (Is_juiblex_level(&u.uz)) {
        return "swamp";
    } else if (ltyp == MOAT && !Is_medusa_level(&u.uz)) {
        return "moat";
    }

    return hliquid("water");
}

STATIC_OVL void
set_wportal()
{
    /* there better be only one magic portal on water level... */
    for (wportal = ftrap; wportal; wportal = wportal->ntrap)
        if (wportal->ttyp == MAGIC_PORTAL) return;
    impossible("set_wportal(): no portal!");
}

STATIC_OVL void
setup_waterlevel()
{
    int x, y;
    int xskip, yskip;
    int water_glyph = cmap_to_glyph(S_water);

    /* ouch, hardcoded... (file scope statics and used in bxmin,bymax,&c) */
    xmin = 3;
    ymin = 1;
    /* use separate statements so that compiler won't complain about min()
       comparing two constants; the alternative is to do this in the
       preprocessor: #if (20 > ROWNO-1) ymax=ROWNO-1 #else ymax=20 #endif */
    xmax = 78;
    xmax = min(xmax, (COLNO - 1) - 1);
    ymax = 20;
    ymax = min(ymax, (ROWNO - 1));

    /* set hero's memory to water */

    for (x = xmin; x <= xmax; x++)
        for (y = ymin; y <= ymax; y++)
            levl[x][y].glyph = water_glyph;

    /* make bubbles */

    xskip = 10 + rn2(10);
    yskip = 4 + rn2(4);
    for (x = bxmin; x <= bxmax; x += xskip)
        for (y = bymin; y <= bymax; y += yskip)
            mk_bubble(x, y, rn2(7));
}

STATIC_OVL void
unsetup_waterlevel()
{
    struct bubble *b, *bb;

    /* free bubbles */
    for (b = bbubbles; b; b = bb) {
        bb = b->next;
        free((genericptr_t)b);
    }
    bbubbles = ebubbles = (struct bubble *)0;
}

STATIC_OVL void
mk_bubble(x, y, n)
int x, y, n;
{
    /*
     * These bit masks make visually pleasing bubbles on a normal aspect
     * 25x80 terminal, which naturally results in them being mathematically
     * anything but symmetric.  For this reason they cannot be computed
     * in situ, either.  The first two elements tell the dimensions of
     * the bubble's bounding box.
     */
    static const uchar
        bm2[] = {2, 1, 0x3},
        bm3[] = {3, 2, 0x7, 0x7},
        bm4[] = {4, 3, 0x6, 0xf, 0x6},
        bm5[] = {5, 3, 0xe, 0x1f, 0xe},
        bm6[] = {6, 4, 0x1e, 0x3f, 0x3f, 0x1e},
        bm7[] = {7, 4, 0x3e, 0x7f, 0x7f, 0x3e},
        bm8[] = {8, 4, 0x7e, 0xff, 0xff, 0x7e},
        *const bmask[] = {bm2, bm3, bm4, bm5, bm6, bm7, bm8};

    struct bubble *b;

    if (x >= bxmax || y >= bymax) return;
    if (n >= SIZE(bmask)) {
        impossible("n too large (mk_bubble)");
        n = SIZE(bmask) - 1;
    }
    if (bmask[n][1] > MAX_BMASK) {
        panic("bmask size is larger than MAX_BMASK");
    }
    b = (struct bubble *)alloc(sizeof(struct bubble));
    if ((x + (int) bmask[n][0] - 1) > bxmax) x = bxmax - bmask[n][0] + 1;
    if ((y + (int) bmask[n][1] - 1) > bymax) y = bymax - bmask[n][1] + 1;
    b->x = x;
    b->y = y;
    b->dx = 1 - rn2(3);
    b->dy = 1 - rn2(3);
    /* y dimension is the length of bitmap data - see bmask above */
    (void)memcpy(b->bm, bmask[n], (bmask[n][1]+2)*sizeof(b->bm[0]));
    b->cons = 0;
    if (!bbubbles) bbubbles = b;
    if (ebubbles) {
        ebubbles->next = b;
        b->prev = ebubbles;
    }
    else
        b->prev = (struct bubble *)0;
    b->next =  (struct bubble *)0;
    ebubbles = b;
    mv_bubble(b, 0, 0, TRUE);
}

/*
 * The player, the portal and all other objects and monsters
 * float along with their associated bubbles.  Bubbles may overlap
 * freely, and the contents may get associated with other bubbles in
 * the process.  Bubbles are "sticky", meaning that if the player is
 * in the immediate neighborhood of one, he/she may get sucked inside.
 * This property also makes leaving a bubble slightly difficult.
 */
STATIC_OVL void
mv_bubble(b, dx, dy, ini)
struct bubble *b;
int dx, dy;
boolean ini;
{
    int x, y, i, j, colli = 0;
    struct container *cons, *ctemp;

    /* move bubble */
    if (dx < -1 || dx > 1 || dy < -1 || dy > 1) {
        /* pline("mv_bubble: dx = %d, dy = %d", dx, dy); */
        dx = sgn(dx);
        dy = sgn(dy);
    }

    /*
     * collision with level borders?
     *  1 = horizontal border, 2 = vertical, 3 = corner
     */
    if (b->x <= bxmin) colli |= 2;
    if (b->y <= bymin) colli |= 1;
    if ((int) (b->x + b->bm[0] - 1) >= bxmax) colli |= 2;
    if ((int) (b->y + b->bm[1] - 1) >= bymax) colli |= 1;

    if (b->x < bxmin) {
        pline("bubble xmin: x = %d, xmin = %d", b->x, bxmin);
        b->x = bxmin;
    }
    if (b->y < bymin) {
        pline("bubble ymin: y = %d, ymin = %d", b->y, bymin);
        b->y = bymin;
    }
    if ((int) (b->x + b->bm[0] - 1) > bxmax) {
        pline("bubble xmax: x = %d, xmax = %d",
              b->x + b->bm[0] - 1, bxmax);
        b->x = bxmax - b->bm[0] + 1;
    }
    if ((int) (b->y + b->bm[1] - 1) > bymax) {
        pline("bubble ymax: y = %d, ymax = %d",
              b->y + b->bm[1] - 1, bymax);
        b->y = bymax - b->bm[1] + 1;
    }

    /* bounce if we're trying to move off the border */
    if (b->x == bxmin && dx < 0) dx = -dx;
    if (b->x + b->bm[0] - 1 == bxmax && dx > 0) dx = -dx;
    if (b->y == bymin && dy < 0) dy = -dy;
    if (b->y + b->bm[1] - 1 == bymax && dy > 0) dy = -dy;

    b->x += dx;
    b->y += dy;

    /* draw the bubbles */
    for (i = 0, x = b->x; i < (int) b->bm[0]; i++, x++)
        for (j = 0, y = b->y; j < (int) b->bm[1]; j++, y++)
            if (b->bm[j + 2] & (1 << i)) {
                levl[x][y].typ = AIR;
                levl[x][y].lit = 1;
                unblock_point(x, y);
            }

    /* replace contents of bubble */
    for (cons = b->cons; cons; cons = ctemp) {
        ctemp = cons->next;
        cons->x += dx;
        cons->y += dy;

        switch(cons->what) {
        case CONS_OBJ: {
            struct obj *olist, *otmp;

            for (olist=(struct obj *)cons->list; olist; olist=otmp) {
                otmp = olist->nexthere;
                place_object(olist, cons->x, cons->y);
                stackobj(olist);
            }
            break;
        }

        case CONS_MON: {
            struct monst *mon = (struct monst *) cons->list;
            /* mnearto() might fail. We can jump right to elemental_clog
                from here rather than deal_with_overcrowding() */
            if (!mnearto(mon, cons->x, cons->y, TRUE)) {
                elemental_clog(mon);
            }
            break;
        }

        case CONS_HERO: {
            int ux0 = u.ux, uy0 = u.uy;

            u_on_newpos(cons->x, cons->y);
            newsym(ux0, uy0); /* clean up old position */

            struct monst *mtmp = m_at(cons->x, cons->y);
            if (mtmp) {
                mnexto(mtmp);
            }
            break;
        }

        case CONS_TRAP: {
            struct trap *btrap = (struct trap *) cons->list;
            btrap->tx = cons->x;
            btrap->ty = cons->y;
            break;
        }

        default:
            impossible("mv_bubble: unknown bubble contents");
            break;
        }
        free((genericptr_t)cons);
    }
    b->cons = 0;

    /* boing? */

    switch (colli) {
    case 1: b->dy = -b->dy; break;
    case 3: b->dy = -b->dy;     /* fall through */
    case 2: b->dx = -b->dx; break;
    default:
        /* sometimes alter direction for fun anyway
           (higher probability for stationary bubbles) */
        if (!ini && ((b->dx || b->dy) ? !rn2(20) : !rn2(5))) {
            b->dx = 1 - rn2(3);
            b->dy = 1 - rn2(3);
        }
    }
}

/*mkmaze.c*/
