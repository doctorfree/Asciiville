/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* Contains code for 'd', 'D' (drop), '>', '<' (up, down) */

#include "hack.h"
#include "lev.h"

#include <limits.h>

#ifdef SINKS
static void NDECL(polymorph_sink);
static boolean NDECL(teleport_sink);
STATIC_DCL void FDECL(dosinkring, (struct obj *));
#endif /* SINKS */

STATIC_PTR int FDECL(drop, (struct obj *));
STATIC_PTR int NDECL(wipeoff);

STATIC_DCL int FDECL(menu_drop, (int));
STATIC_DCL int NDECL(currentlevel_rewrite);
STATIC_DCL void NDECL(final_level);
/* static boolean FDECL(badspot, (XCHAR_P,XCHAR_P)); */
STATIC_DCL boolean NDECL(unique_item_check);
STATIC_DCL void NDECL(levelport_monsters);

static NEARDATA const char drop_types[] =
{ ALLOW_COUNT, COIN_CLASS, ALL_CLASSES, 0 };

/* 'd' command: drop one inventory item */
int
dodrop()
{
    int result, i = (invent) ? 0 : (SIZE(drop_types) - 1);

    if (*u.ushops) sellobj_state(SELL_DELIBERATE);
    result = drop(getobj(&drop_types[i], "drop"));
    if (*u.ushops) sellobj_state(SELL_NORMAL);
    if (result) {
        reset_occupations();
    }

    return result;
}

/* Called when a boulder is dropped, thrown, or pushed.  If it ends up
 * in a pool, it either fills the pool up or sinks away.  In either case,
 * it's gone for good...  If the destination is not a pool, returns FALSE.
 */
boolean
boulder_hits_pool(otmp, rx, ry, pushing)
struct obj *otmp;
register int rx, ry;
boolean pushing;
{
    if (!otmp || otmp->otyp != BOULDER)
        warning("Not a boulder?");
    else if (!Is_waterlevel(&u.uz) &&
             (is_pool(rx, ry) || is_lava(rx, ry) || is_swamp(rx, ry))) {
        boolean lava = is_lava(rx, ry), fills_up;
        boolean swamp = is_swamp(rx, ry);
        const char *what = waterbody_name(rx, ry);
        schar ltyp = levl[rx][ry].typ;
        int chance = rn2(10);       /* water: 90%; lava: 10% */
        fills_up = swamp ? 1 : lava ? chance == 0 : chance != 0;

        if (fills_up) {
            struct trap *ttmp = t_at(rx, ry);

            if (ltyp == DRAWBRIDGE_UP) {
                levl[rx][ry].drawbridgemask &= ~DB_UNDER; /* clear lava */
                levl[rx][ry].drawbridgemask |= DB_FLOOR;
            } else {
                levl[rx][ry].typ = ROOM;
                levl[rx][ry].flags = 0;
            }

            if (ttmp) (void) delfloortrap(ttmp);
            bury_objs(rx, ry);

            newsym(rx, ry);
            if (pushing) {
                char whobuf[BUFSZ];

                Strcpy(whobuf, "you");
                if (u.usteed) {
                    Strcpy(whobuf, y_monnam(u.usteed));
                }
                pline("%s %s %s into the %s.", upstart(whobuf),
                      vtense(whobuf, "push"), the(xname(otmp)), what);
                if (flags.verbose && !Blind) {
                    pline("Now you can cross it!");
                }
                /* no splashing in this case */
            }
        }
        if (!fills_up || !pushing) {    /* splashing occurs */
            if (!u.uinwater) {
                if (pushing ? !Blind : cansee(rx, ry)) {
                    There("is a large splash as %s %s the %s.",
                          the(xname(otmp)), fills_up ? "fills" : "falls into",
                          what);
                } else if (!Deaf) {
                    You_hear("a%s splash.", lava ? " sizzling" : "");
                }
                wake_nearto(rx, ry, 40);
            }

            if (fills_up && u.uinwater && distu(rx, ry) == 0) {
                u.uinwater = 0;
                docrt();
                vision_full_recalc = 1;
                You("find yourself on dry land again!");
            } else if (lava && distu(rx, ry) <= 2) {
                You("are hit by molten %s%c", hliquid("lava"),
                    Fire_resistance ? '.' : '!');
                burn_away_slime();
                int dmg = d((Fire_resistance ? 1 : 3), 6);
                losehp(Maybe_Half_Phys(dmg), /* lava damage */
                       "molten lava", KILLED_BY);
            } else if (!fills_up && flags.verbose &&
                       (pushing ? !Blind : cansee(rx, ry)))
                pline("It sinks without a trace!");
        }

        /* boulder is now gone */
        if (pushing) delobj(otmp);
        else obfree(otmp, (struct obj *)0);
        return TRUE;
    }
    return FALSE;
}

/* Used for objects which sometimes do special things when dropped; must be
 * called with the object not in any chain.  Returns TRUE if the object goes
 * away.
 */
boolean
flooreffects(obj, x, y, verb)
struct obj *obj;
int x, y;
const char *verb;
{
    struct trap *t;
    struct monst *mtmp;
    boolean tseen;
    int ttyp = NO_TRAP;

    if (obj->where != OBJ_FREE)
        panic("flooreffects: obj not free (%d,%d,%d)", obj->where, obj->otyp, obj->invlet);

    /* make sure things like water_damage() have no pointers to follow */
    obj->nobj = obj->nexthere = (struct obj *)0;

    if (obj->otyp == BOULDER && boulder_hits_pool(obj, x, y, FALSE))
        return TRUE;
    else if (obj->otyp == BOULDER && (t = t_at(x, y)) != 0 &&
             (t->ttyp==PIT || t->ttyp==SPIKED_PIT
              || t->ttyp==TRAPDOOR || t->ttyp==HOLE)) {
        ttyp = t->ttyp;
        tseen = t->tseen ? TRUE : FALSE;
        if (((mtmp = m_at(x, y)) && mtmp->mtrapped) ||
            (u.utrap && u.ux == x && u.uy == y)) {
            if (*verb && (cansee(x, y) || distu(x, y) == 0)) {
                pline("%s boulder %s into the pit%s.",
                      Blind ? "A" : "The",
                      vtense((const char *) 0, verb),
                      mtmp ? "" : " with you");
            }
            if (mtmp) {
                if (!passes_walls(mtmp->data) &&
                     !throws_rocks(mtmp->data)) {
                    /* dieroll was rnd(20); 1: maximum chance to hit
                       since trapped target is a sitting duck */
                    int damage, dieroll = 1;

                    /* As of NH 3.6.2: this was calling hmon() unconditionally
                       so always credited/blamed the hero but the boulder
                       might have been thrown by a giant or launched by
                       a rolling boulder trap triggered by a monster or
                       dropped by a scroll of earth read by a monster */
                    if (flags.mon_moving) {
                        /* normally we'd use ohitmon() but it can call
                           drop_throw() which calls flooreffects() */
                        damage = dmgval(obj, mtmp);
                        mtmp->mhp -= damage;
                        if (DEADMONSTER(mtmp)) {
                            if (canspotmon(mtmp)) {
                                pline("%s is %s!", Monnam(mtmp),
                                      (nonliving(mtmp->data) || is_vampshifter(mtmp)) ? "destroyed" : "killed");
                            }
                            mondied(mtmp);
                        }
                    } else {
                        (void) hmon(mtmp, obj, HMON_THROWN, dieroll);
                    }
                    if (!DEADMONSTER(mtmp) && !is_whirly(mtmp->data)) {
                        return FALSE; /* still alive */
                    }
                }
                mtmp->mtrapped = 0;
            } else {
                if (!Passes_walls && !throws_rocks(youmonst.data)) {
                    losehp(Maybe_Half_Phys(rnd(15)), "squished under a boulder",
                           NO_KILLER_PREFIX);
                    return FALSE; /* player remains trapped */
                } else {
                    reset_utrap(TRUE);
                }
            }
        }
        if (*verb) {
            if (Blind && (x == u.ux) && (y == u.uy)) {
                You_hear("a CRASH! beneath you.");
            } else if (!Blind && cansee(x, y)) {
                pline_The("boulder %s%s.",
                          (ttyp == TRAPDOOR && !tseen) ? "triggers and " : "",
                          (ttyp == TRAPDOOR) ? "plugs a trap door" :
                          (ttyp == HOLE) ? "plugs a hole" :
                          "fills a pit");
            } else {
                You_hear("a boulder %s.", verb);
            }
        }
        /*
         * Note:  trap might have gone away via ((hmon -> killed -> xkilled)
         *  || mondied) -> mondead -> m_detach -> fill_pit.
         */
        if ((t = t_at(x, y)) != 0) {
            deltrap(t);
        }
        useupf(obj, 1L);
        bury_objs(x, y);
        newsym(x, y);
        return TRUE;
    } else if (obj->otyp==AMULET_OF_YENDOR &&
               (obj->cursed ? rnf(1, 2) :
                obj->blessed ? rnf(1, 16) : rnf(1, 4))) {
        /* prevent recursive call of teleportation through flooreffects */
        if (!obj->orecursive &&
            distu(x, y) < 9) {
            if (Blind) {
                You_hear("%s!",
                         (Hallucination) ? "nothing special happening" :
                         "something teleporting");
            } else if (cansee(x, y)) {
                pline("Right after touching the %s the %s teleports away!",
                      surface(x, y),
                      (Hallucination) ? "teddy bear" : "amulet");
            }
            obj->orecursive = TRUE;
            rloco(obj);
            obj->orecursive = FALSE;
            return TRUE;
        } else {
            return FALSE;
        }
    } else if (is_lava(x, y)) {
        return lava_damage(obj, x, y);
    } else if (is_pool(x, y) || is_swamp(x, y)) {
        /* Reasonably bulky objects (arbitrary) splash when dropped.
         * If you're floating above the water even small things make noise.
         * Stuff dropped near fountains always misses */
        if ((Blind || (Levitation || Flying || Wwalking)) &&
             !Deaf && ((x == u.ux) && (y == u.uy))) {
            if (!Underwater) {
                if (weight(obj) > 9) {
                    pline("Splash!");
                } else if (Levitation || Flying) {
                    pline("Plop!");
                }
            }
            map_background(x, y, 0);
            newsym(x, y);
        }
        return water_damage(obj, NULL, FALSE) == ER_DESTROYED;
    } else if (u.ux == x && u.uy == y && (t = t_at(x, y)) != 0 &&
               (uteetering_at_seen_pit(t) || uescaped_shaft(t))) {
        /* you escaped a pit and are standing on the precipice */
        if (Blind && !Deaf) {
            You_hear("%s tumble downwards.", the(xname(obj)));
        } else {
            pline("%s %s into %s %s.",
                  The(xname(obj)), otense(obj, "tumble"),
                  the_your[t->madeby_u],
                  is_pit(t->ttyp) ? "pit" : "hole");
        }
    }
    return FALSE;
}

/** obj is an object dropped on an altar */
void
doaltarobj(obj)
register struct obj *obj;
{
    if (Blind)
        return;

    if (obj->oclass != COIN_CLASS) {
        /* KMH, conduct */
        if (!u.uconduct.gnostic) {
            livelog_printf(LL_CONDUCT, "eschewed atheism, by dropping %s on an altar", doname(obj));
        }
        u.uconduct.gnostic++;
    } else {
        /* coins don't have bless/curse status */
        obj->blessed = obj->cursed = 0;
    }

    if (obj->blessed || obj->cursed) {
        There("is %s flash as %s %s the altar.",
              an(hcolor(obj->blessed ? NH_AMBER : NH_BLACK)),
              doname(obj), otense(obj, "hit"));
        if (!Hallucination) {
            obj->bknown = 1; /* ok to bypass set_bknown() */
        }
    } else {
        pline("%s %s on the altar.", Doname2(obj),
              otense(obj, "land"));
        if (obj->oclass != COIN_CLASS) {
            obj->bknown = 1; /* ok to bypass set_bknown() */
        }
    }

    /* Also BUC one level deep inside containers */
    if (Has_contents(obj) && !obj->olocked) {
        int blessed = 0;
        int cursed = 0;
        struct obj * otmp;

        obj->cknown = 1;

        for (otmp = obj->cobj; otmp; otmp = otmp->nobj) {
            if (otmp->blessed) {
                blessed++;
            }
            if (otmp->cursed) {
                cursed++;
            }
            if (!Hallucination) {
                otmp->bknown = 1;
            }
        }
        /* even when hallucinating, if you get no flashes at all, you know
         * everything's uncursed, so save the player the trouble of manually
         * naming them all */
        if (Hallucination && blessed + cursed == 0) {
            for (otmp = obj->cobj; otmp; otmp = otmp->nobj) {
                otmp->bknown = 1;
            }
        }

        if (blessed + cursed > 0) {
            const char* color;
            if (Hallucination && blessed + cursed > 1) {
                color = "pretty multichromatic";
            } else if (Hallucination) {
                color = hcolor(NULL);
            } else if (blessed == 0) {
                color = hcolor(NH_BLACK);
            } else if (cursed == 0) {
                color = hcolor(NH_AMBER);
            } else {
                color = "colored";
            }

            pline("Looking inside %s, you see %s flash%s.",
                  the(xname(obj)),
                  (blessed + cursed == 1 ? an(color) : color),
                  (blessed + cursed == 1 ? "" : "es"));
        }
    }
}

#ifdef SINKS
/** Transforms the sink at the player's position into
 * a fountain, throne, altar or grave. */
static void
polymorph_sink()
{
    if (levl[u.ux][u.uy].typ != SINK) { return; }

    boolean sinklooted = levl[u.ux][u.uy].looted != 0;
    level.flags.nsinks--;
    levl[u.ux][u.uy].doormask = 0; /* levl[][].flags */
    switch(rn2(4)) {
    case 0:
        levl[u.ux][u.uy].typ = FOUNTAIN;
        levl[u.ux][u.uy].blessedftn = 0;
        if (sinklooted) {
            SET_FOUNTAIN_LOOTED(u.ux, u.uy);
        }
        level.flags.nfountains++;
        break;
    case 1:
        levl[u.ux][u.uy].typ = THRONE;
        if (sinklooted) {
            levl[u.ux][u.uy].looted = T_LOOTED;
        }
        break;
    case 2:
        levl[u.ux][u.uy].typ = ALTAR;
        /* This used to pass 'rn2(A_LAWFUL + 2) - 1' to
           Align2amask() but that evaluates its argument more than once */
        int algn = rn2(3) - 1; /* -1 (A_Cha) or 0 (A_Neu) or +1 (A_Law) */
        levl[u.ux][u.uy].altarmask = ((Inhell && rn2(3)) ? AM_NONE : Align2amask(algn));
        break;
    case 3:
        levl[u.ux][u.uy].typ = ROOM;
        make_grave(u.ux, u.uy, (char *) 0);
        break;
    }
    /* give message even if blind; we know we're not levitating,
       so can feel the outcome even if we can't directly see it */
    pline_The("sink transforms into %s!",
            (levl[u.ux][u.uy].typ == THRONE) ?
            "a throne" : an(surface(u.ux, u.uy)));
    newsym(u.ux, u.uy);
}

/** Teleports the sink at the player's position.
 * @return TRUE if sink teleported */
static boolean
teleport_sink()
{
    int cx, cy;
    int cnt = 0;
    struct trap *trp;
    struct engr *eng;

    do {
        cx = rnd(COLNO-1);
        cy = rn2(ROWNO);
        trp = t_at(cx, cy);
        eng = engr_at(cx, cy);
    } while (((levl[cx][cy].typ != ROOM) || trp || eng || cansee(cx, cy)) && cnt++ < 200);

    if (levl[cx][cy].typ == ROOM && !trp && !eng) {
        /* create sink at new position */
        levl[cx][cy].typ = SINK;
        levl[cx][cy].looted = levl[u.ux][u.uy].looted;
        newsym(cx, cy);
        /* remove old sink */
        levl[u.ux][u.uy].typ = ROOM;
        levl[u.ux][u.uy].looted = 0;
        newsym(u.ux, u.uy);
        return TRUE;
    }
    return FALSE;
}

STATIC_OVL
void
dosinkring(obj)  /* obj is a ring being dropped over a kitchen sink */
register struct obj *obj;
{
    struct obj *otmp, *otmp2;
    boolean ideed = TRUE;

    You("drop %s down the drain.", doname(obj));
    obj->in_use = TRUE; /* block free identification via interrupt */
    switch(obj->otyp) { /* effects that can be noticed without eyes */
    case RIN_SEARCHING:
        You("thought %s got lost in the sink, but there it is!", yname(obj));
        goto giveback;
    case RIN_SLOW_DIGESTION:
        pline_The("ring is regurgitated!");
giveback:
        obj->in_use = FALSE;
        dropx(obj);
        makeknown(obj->otyp);
        return;
    case RIN_LEVITATION:
        pline_The("sink quivers upward for a moment.");
        break;
    case RIN_POISON_RESISTANCE:
        You("smell rotten %s.", makeplural(fruitname(FALSE)));
        break;
    case RIN_AGGRAVATE_MONSTER:
        pline("Several %s buzz angrily around the sink.",
              Hallucination ? makeplural(rndmonnam()) : "flies");
        break;
    case RIN_SHOCK_RESISTANCE:
        pline("Static electricity surrounds the sink.");
        break;
    case RIN_CONFLICT:
        You_hear("loud noises coming from the drain.");
        break;
    case RIN_SUSTAIN_ABILITY:       /* KMH */
        pline_The("%s flow seems fixed.", hliquid("water"));
        break;
    case RIN_GAIN_STRENGTH:
        pline_The("%s flow seems %ser now.", hliquid("water"),
                  (obj->spe<0) ? "weak" : "strong");
        break;
    case RIN_GAIN_CONSTITUTION:
        pline_The("%s flow seems %ser now.", hliquid("water"),
                  (obj->spe<0) ? "less" : "great");
        break;
    case RIN_GAIN_INTELLIGENCE:
        pline("The %s seems %ser now.", hliquid("water"),
              (obj->spe<0) ? "dimm" : "bright");
        break;
    case RIN_GAIN_WISDOM:
        pline("The %s flow seems %ser now.", hliquid("water"),
              (obj->spe<0) ? "dull" : "quick");
        break;
    case RIN_GAIN_DEXTERITY:
        pline("The %s flow seems %ser now.", hliquid("water"),
              (obj->spe<0) ? "slow" : "fast");
        break;
    case RIN_INCREASE_ACCURACY:     /* KMH */
        pline_The("%s flow %s the drain.", hliquid("water"),
                  (obj->spe<0) ? "misses" : "hits");
        break;
    case RIN_INCREASE_DAMAGE:
        pline_The("water's force seems %ser now.",
                  (obj->spe<0) ? "small" : "great");
        break;
    case RIN_HUNGER:
        ideed = FALSE;
        for(otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp2) {
            otmp2 = otmp->nexthere;
            if (otmp != uball && otmp != uchain &&
                !obj_resists(otmp, 1, 99)) {
                if (!Blind) {
                    pline("Suddenly, %s %s from the sink!",
                          doname(otmp), otense(otmp, "vanish"));
                    ideed = TRUE;
                }
                delobj(otmp);
            }
        }
        break;
    case MEAT_RING:
        /* Not the same as aggravate monster; besides, it's obvious. */
        pline("Several %s buzz around the sink.",
              Hallucination ? makeplural(rndmonnam()) : "flies");
        break;

    default:
        ideed = FALSE;
        break;
    }
    if(!Blind && !ideed && obj->otyp != RIN_HUNGER) {
        ideed = TRUE;
        switch(obj->otyp) {     /* effects that need eyes */
        case RIN_ADORNMENT:
            pline_The("faucets flash brightly for a moment.");
            break;
        case RIN_REGENERATION:
            pline_The("sink looks as good as new.");
            break;
        case RIN_INVISIBILITY:
            pline_The("%s flow momentarily vanishes.", hliquid("water"));
            break;
        case RIN_FREE_ACTION:
            You_see("the ring slide right down the drain!");
            break;
        case RIN_SEE_INVISIBLE:
            You_see("some %s in the sink.",
                Hallucination ? "oxygen molecules" : "air");
            break;
        case RIN_STEALTH:
            pline_The("sink seems to blend into the floor for a moment.");
            break;
        case RIN_FIRE_RESISTANCE:
            pline_The("hot %s faucet flashes brightly for a moment.", hliquid("water"));
            break;
        case RIN_COLD_RESISTANCE:
            pline_The("cold %s faucet flashes brightly for a moment.", hliquid("water"));
            break;
        case RIN_PROTECTION_FROM_SHAPE_CHAN:
            pline_The("sink looks nothing like a fountain.");
            break;
        case RIN_PROTECTION:
            pline_The("sink glows %s for a moment.",
                      hcolor((obj->spe<0) ? NH_BLACK : NH_SILVER));
            break;
        case RIN_WARNING:
            pline_The("sink glows %s for a moment.", hcolor(NH_RED));
            break;
        case RIN_TELEPORTATION:
            if (teleport_sink())
                pline_The("sink vanishes.");
            else
                pline_The("sink momentarily vanishes.");
            break;
        case RIN_TELEPORT_CONTROL:
            pline_The("sink looks like it is being beamed aboard somewhere.");
            break;
        case RIN_POLYMORPH:
            polymorph_sink();
            break;
        case RIN_POLYMORPH_CONTROL:
            pline_The("sink transforms into another sink!");
            levl[u.ux][u.uy].looted = 0;
            break;
        }
    }
    if(ideed)
        makeknown(obj->otyp);
    else
        You_hear("the ring bouncing down the drainpipe.");
    if (!rn2(20)) {
        pline_The("sink backs up, leaving %s.", doname(obj));
        obj->in_use = FALSE;
        dropx(obj);
    } else
        useup(obj);
}
#endif

/* some common tests when trying to drop or throw items */
boolean
canletgo(obj, word)
struct obj *obj;
const char *word;
{
    if (obj->owornmask & (W_ARMOR | W_ACCESSORY)) {
        if (*word)
            Norep("You cannot %s %s you are wearing.", word,
                  something);
        return(FALSE);
    }
    if (obj->otyp == LOADSTONE && obj->cursed) {
        /* getobj() kludge sets corpsenm to user's specified count
           when refusing to split a stack of cursed loadstones */
        if (*word) {
            /* getobj() ignores a count for throwing since that is
               implicitly forced to be 1; replicate its kludge... */
            if (!strcmp(word, "throw") && obj->quan > 1L)
                obj->corpsenm = 1;
            pline("For some reason, you cannot %s%s the stone%s!",
                  word, obj->corpsenm ? " any of" : "",
                  plur(obj->quan));
        }
        obj->corpsenm = 0;  /* reset */
        set_bknown(obj, 1);
        return(FALSE);
    }
    if (obj->otyp == LEASH && obj->leashmon != 0) {
        if (*word)
            pline_The("leash is tied around your %s.",
                      body_part(HAND));
        return(FALSE);
    }
#ifdef STEED
    if (obj->owornmask & W_SADDLE) {
        if (*word)
            You("cannot %s %s you are sitting on.", word,
                something);
        return (FALSE);
    }
#endif
    return(TRUE);
}

STATIC_PTR
int
drop(obj)
register struct obj *obj;
{
    if(!obj) return(0);
    if(!canletgo(obj, "drop"))
        return(0);
    if(obj == uwep) {
        if(welded(uwep)) {
            weldmsg(obj);
            return(0);
        }
        setuwep((struct obj *)0);
    }
    if(obj == uquiver) {
        setuqwep((struct obj *)0);
    }
    if (obj == uswapwep) {
        setuswapwep((struct obj *)0);
    }

    obj->was_dropped = TRUE;

    if (u.uswallow) {
        /* barrier between you and the floor */
        if (flags.verbose) {
            char *onam_p, monbuf[BUFSZ];

            /* doname can call s_suffix, reusing its buffer */
            Strcpy(monbuf, s_suffix(mon_nam(u.ustuck)));
            onam_p = is_unpaid(obj) ? yobjnam(obj, (char *) 0) : doname(obj);
            You("drop %s into %s %s.", onam_p, monbuf,
                mbodypart(u.ustuck, STOMACH));
        }
    } else {
#ifdef SINKS
        if((obj->oclass == RING_CLASS || obj->otyp == MEAT_RING) &&
           IS_SINK(levl[u.ux][u.uy].typ)) {
            dosinkring(obj);
            return(1);
        }
#endif
        if (!can_reach_floor(TRUE)) {
            if(flags.verbose) You("drop %s.", doname(obj));
            /* Ensure update when we drop gold objects */
            if (obj->oclass == COIN_CLASS) flags.botl = 1;
            freeinv(obj);
            hitfloor(obj, TRUE);
            return(1);
        }
        if (!IS_ALTAR(levl[u.ux][u.uy].typ) && flags.verbose)
            You("drop %s.", doname(obj));
    }
    dropx(obj);
    return(1);
}

/* Called in several places - may produce output */
/* eg ship_object() and dropy() -> sellobj() both produce output */
void
dropx(obj)
register struct obj *obj;
{
    /* Tipped objects aren't considered carried, even if
     * their container is, so don't freeinv() it. */
    if (carried(obj)) {
        /* Ensure update when we drop gold objects */
        if (obj->oclass == COIN_CLASS) flags.botl = 1;
        freeinv(obj);
    }
    if (!u.uswallow) {
        if (ship_object(obj, u.ux, u.uy, FALSE)) return;
        if (IS_ALTAR(levl[u.ux][u.uy].typ))
            doaltarobj(obj); /* set bknown */
    }
    dropy(obj);
}

/* dropy - put dropped object at destination; called from lots of places */
void
dropy(obj)
struct obj *obj;
{
    dropz(obj, FALSE);
}

/* dropz - really put dropped object at its destination... */
void
dropz(obj, with_impact)
struct obj *obj;
boolean with_impact;
{
    if (obj == uwep) setuwep((struct obj *)0);
    if (obj == uquiver) setuqwep((struct obj *)0);
    if (obj == uswapwep) setuswapwep((struct obj *)0);

    if (!u.uswallow && flooreffects(obj, u.ux, u.uy, "drop")) return;
    /* uswallow check done by GAN 01/29/87 */
    if(u.uswallow) {
        boolean could_petrify = FALSE;
        boolean could_poly = FALSE;
        boolean could_slime = FALSE;
        boolean could_grow = FALSE;
        boolean could_heal = FALSE;

        if (obj != uball) {     /* mon doesn't pick up ball */
            if (obj->otyp == CORPSE) {
                could_petrify = touch_petrifies(&mons[obj->corpsenm]);
                could_poly = polyfodder(obj);
                could_slime = (obj->corpsenm == PM_GREEN_SLIME);
                could_grow = (obj->corpsenm == PM_WRAITH);
                could_heal = (obj->corpsenm == PM_NURSE);
            }
            if (is_unpaid(obj)) {
                (void) stolen_value(obj, u.ux, u.uy, TRUE, FALSE);
            }
            (void) mpickobj(u.ustuck, obj);
            if (is_animal(u.ustuck->data)) {
                if (could_poly || could_slime) {
                    (void) newcham(u.ustuck,
                                   could_poly ? (struct permonst *)0 :
                                   &mons[PM_GREEN_SLIME],
                                   FALSE, could_slime);
                    delobj(obj); /* corpse is digested */
                } else if (could_petrify) {
                    minstapetrify(u.ustuck, TRUE);
                    /* Don't leave a cockatrice corpse in a statue */
                    if (!u.uswallow) delobj(obj);
                } else if (could_grow) {
                    (void) grow_up(u.ustuck, (struct monst *)0);
                    delobj(obj); /* corpse is digested */
                } else if (could_heal) {
                    u.ustuck->mhp = u.ustuck->mhpmax;
                    delobj(obj); /* corpse is digested */
                }
            }
        }
    } else {
        place_object(obj, u.ux, u.uy);
        if (with_impact) {
            container_impact_dmg(obj, u.ux, u.uy);
        }
        if (obj == uball)
            drop_ball(u.ux, u.uy);
        else
            sellobj(obj, u.ux, u.uy);
        stackobj(obj);
        if(Blind && Levitation)
            map_object(obj, 0);
        newsym(u.ux, u.uy);  /* remap location under self */
    }
}

/* things that must change when not held; recurse into containers.
   Called for both player and monsters */
void
obj_no_longer_held(obj)
struct obj *obj;
{
    if (!obj) {
        return;
    } else if (Has_contents(obj)) {
        struct obj *contents;
        for(contents=obj->cobj; contents; contents=contents->nobj)
            obj_no_longer_held(contents);
    }
    switch(obj->otyp) {
    case CRYSKNIFE:
        /* KMH -- Fixed crysknives have only 10% chance of reverting */
        /* only changes when not held by player or monster */
        if (!obj->oerodeproof || !rn2(10)) {
            if (!flags.mon_moving && !program_state.gameover) {
                costly_alteration(obj, COST_DEGRD);
            }
            obj->otyp = WORM_TOOTH;
            obj->oerodeproof = 0;
        }
        break;
    }
}

/* 'D' command: drop several things */
int
doddrop()
{
    int result = 0;

    if (!invent) {
        You("have nothing to drop.");
        return 0;
    }
    add_valid_menu_class(0); /* clear any classes already there */
    if (*u.ushops) sellobj_state(SELL_DELIBERATE);
    if (flags.menu_style != MENU_TRADITIONAL ||
        (result = ggetobj("drop", drop, 0, FALSE, (unsigned *)0)) < -1)
        result = menu_drop(result);
    if (*u.ushops) sellobj_state(SELL_NORMAL);
    if (result) {
        reset_occupations();
    }

    return result;
}

/* Drop things from the hero's inventory, using a menu. */
STATIC_OVL int
menu_drop(retry)
int retry;
{
    int n, i, n_dropped = 0;
    long cnt;
    struct obj *otmp, *otmp2;
    menu_item *pick_list;
    boolean all_categories = TRUE;
    boolean drop_everything = FALSE;

    if (retry) {
        all_categories = (retry == -2);
    } else if (flags.menu_style == MENU_FULL) {
        all_categories = FALSE;
        n = query_category("Drop what type of items?",
                           invent,
                           UNIDENTIFIED_TYPES | UNPAID_TYPES | ALL_TYPES | CHOOSE_ALL |
                           RECENTLY_PICKED_UP |
                           BUC_BLESSED | BUC_CURSED | BUC_UNCURSED | BUC_UNKNOWN,
                           &pick_list, PICK_ANY);
        if (!n) goto drop_done;
        for (i = 0; i < n; i++) {
            if (pick_list[i].item.a_int == ALL_TYPES_SELECTED)
                all_categories = TRUE;
            else if (pick_list[i].item.a_int == 'A')
                drop_everything = TRUE;
            else
                add_valid_menu_class(pick_list[i].item.a_int);
        }
        free((genericptr_t) pick_list);
    } else if (flags.menu_style == MENU_COMBINATION) {
        unsigned ggoresults = 0;
        all_categories = FALSE;
        /* Gather valid classes via traditional NetHack method */
        i = ggetobj("drop", drop, 0, TRUE, &ggoresults);
        if (i == -2) all_categories = TRUE;
        if (ggoresults & ALL_FINISHED) {
            n_dropped = i;
            goto drop_done;
        }
    }

    if (drop_everything) {
        /*
         * Dropping a burning potion of oil while levitating can cause
         * an explosion which might destroy some of hero's inventory,
         * so the old code
         *      for (otmp = invent; otmp; otmp = otmp2) {
         *          otmp2 = otmp->nobj;
         *          n_dropped += drop(otmp);
         *      }
         * was unreliable and could lead to an "object lost" panic.
         *
         * Use the bypass bit to mark items already processed (hence
         * not droppable) and rescan inventory until no unbypassed
         * items remain.
         */
        bypass_objlist(invent, FALSE); /* clear bypass bit for invent */
        while ((otmp = nxt_unbypassed_obj(invent)) != 0) {
            n_dropped += drop(otmp);
        }
        /* we might not have dropped everything (worn armor, welded weapon,
           cursed loadstones), so reset any remaining inventory to normal */
        bypass_objlist(invent, FALSE);
    } else {
        /* should coordinate with perm invent, maybe not show worn items */
        n = query_objlist("What would you like to drop?", invent,
                          USE_INVLET|INVORDER_SORT, &pick_list,
                          PICK_ANY, all_categories ? allow_all : allow_category);
        if (n > 0) {
            /*
             * picklist[] contains a set of pointers into inventory, but
             * as soon as something gets dropped, they might become stale
             * (see the drop_everything code above for an explanation).
             * Just checking to see whether one is still in the invent
             * chain is not sufficient validation since destroyed items
             * will be freed and items we've split here might have already
             * reused that memory and put the same pointer value back into
             * invent.  Ditto for using invlet to validate.  So we start
             * by setting bypass on all of invent, then check each pointer
             * to verify that it is in invent and has that bit set.
             */
            bypass_objlist(invent, TRUE);
            for (i = 0; i < n; i++) {
                otmp = pick_list[i].item.a_obj;
                for (otmp2 = invent; otmp2; otmp2 = otmp2->nobj) {
                    if (otmp2 == otmp) {
                        break;
                    }
                }
                if (!otmp2 || !otmp2->bypass) {
                    continue;
                }
                /* found next selected invent item */
                cnt = pick_list[i].count;
                if (cnt < otmp->quan) {
                    if (welded(otmp)) {
                        ; /* don't split */
                    } else if (otmp->otyp == LOADSTONE && otmp->cursed) {
                        /* same kludge as getobj(), for canletgo()'s use */
                        otmp->corpsenm = (int) cnt; /* don't split */
                    } else {
                        otmp = splitobj(otmp, cnt);
                    }
                }
                n_dropped += drop(otmp);
            }
            bypass_objlist(invent, FALSE); /* reset invent to normal */
            free((genericptr_t) pick_list);
        }
    }

drop_done:
    return n_dropped;
}

/* on a ladder, used in goto_level */
static NEARDATA boolean at_ladder = FALSE;

/** the '>' command */
int
dodown()
{
    struct trap *trap = 0;
    boolean stairs_down = ((u.ux == xdnstair && u.uy == ydnstair) ||
                           (u.ux == sstairs.sx && u.uy == sstairs.sy && !sstairs.up)),
            ladder_down = (u.ux == xdnladder && u.uy == ydnladder);

    if (u_rooted()) {
        return 1;
    }

#ifdef STEED
    if (stucksteed(TRUE)) {
        return 0;
    }
#endif
    /* Levitation might be blocked, but player can still use '>' to
       turn off controlled levitation */
    if (HLevitation || ELevitation) {
        if ((HLevitation & I_SPECIAL) || (ELevitation & W_ARTI)) {
            /* end controlled levitation */
            if (ELevitation & W_ARTI) {
                struct obj *obj;

                for(obj = invent; obj; obj = obj->nobj) {
                    if (obj->oartifact &&
                        artifact_has_invprop(obj, LEVITATION)) {
                        if (obj->age < monstermoves)
                            obj->age = monstermoves + rnz(100);
                        else
                            obj->age += rnz(100);
                    }
                }
            }
            if (float_down(I_SPECIAL|TIMEOUT, W_ARTI)) {
                return (1); /* came down, so moved */
            } else if (!HLevitation && !ELevitation) {
                Your("latent levitation ceases.");
                return 1; /* did something, effectively moved */
            }
        }
        if (BLevitation) {
            ; /* weren't actually floating after all */
        } else if (Blind) {
            /* Avoid alerting player to an unknown stair or ladder.
             * Changes the message for a covered, known staircase
             * too; staircase knowledge is not stored anywhere.
             */
            if (stairs_down) {
                stairs_down = (glyph_to_cmap(levl[u.ux][u.uy].glyph) == S_dnstair);
            } else if (ladder_down) {
                ladder_down = (glyph_to_cmap(levl[u.ux][u.uy].glyph) == S_dnladder);
            }
        }
        if (Is_airlevel(&u.uz)) {
            You("are floating in the %s.", surface(u.ux, u.uy));
        } else if (Is_waterlevel(&u.uz)) {
            You("are floating in %s.",
                is_pool(u.ux, u.uy) ? "the water" : "a bubble of air");
        } else {
            floating_above(stairs_down ? "stairs" : ladder_down ?
                           "ladder" : surface(u.ux, u.uy));
        }
        return (0);   /* didn't move */
    }

    if (Upolyd && ceiling_hider(&mons[u.umonnum]) && u.uundetected) {
        u.uundetected = 0;
        if (Flying) {
            /* lurker above */
            You("fly out of hiding.");
        } else { /* piercer */
            You("drop to the %s.", surface(u.ux, u.uy));
            if (is_pool_or_lava(u.ux, u.uy)) {
                pooleffects(FALSE);
            } else {
                (void) pickup(1);
                if ((trap = t_at(u.ux, u.uy)) != 0) {
                    dotrap(trap, TOOKPLUNGE);
                }
            }
        }
        return 1; /* came out of hiding; might need '>' again to go down */
    }

    if (!stairs_down && !ladder_down) {
        trap = t_at(u.ux, u.uy);
        boolean can_fall_thru_trap = trap && (trap->ttyp == TRAPDOOR || trap->ttyp == HOLE);
        if (trap && (uteetering_at_seen_pit(trap) || uescaped_shaft(trap))) {
            dotrap(trap, TOOKPLUNGE);
            return 1;
        } else if (!trap || !is_hole(trap->ttyp) || !Can_fall_thru(&u.uz) || !trap->tseen) {
            if (flags.autodig && !flags.nopick &&
                uwep && is_pick(uwep)) {
                return use_pick_axe2(uwep);
            } else if (do_stair_travel('>')) {
                return 0;
            } else {
                You_cant("go down here.");
                return(0);
            }
        }
    }
    if (u.ustuck) {
        You("are %s, and cannot go down.",
            !u.uswallow ? "being held" : is_animal(u.ustuck->data) ?
            "swallowed" : "engulfed");
        return(1);
    }
    if (on_level(&valley_level, &u.uz) && stairs_down && !u.uevent.gehennom_entered) {
        You("are standing at the gate to Gehennom.");
        pline("Unspeakable cruelty and harm lurk down there.");
        if (yn("Are you sure you want to enter?") != 'y')
            return(0);
        else pline("So be it.");
        u.uevent.gehennom_entered = 1;  /* don't ask again */
    }

    if (!next_to_u()) {
        You("are held back by your pet!");
        return(0);
    }

    if (trap) {
        if (trap->ttyp == PIT || trap->ttyp == SPIKED_PIT) {
            if (u.utrap && (u.utraptype == TT_PIT)) {
                if (flags.autodig && !flags.nopick &&
                    uwep && is_pick(uwep)) {
                    return use_pick_axe2(uwep);
                } else {
                    You("are already in the pit."); /* YAFM needed */
                }
            } else {
                u.utrap = 1;
                u.utraptype = TT_PIT;
                You("%s down into the pit.", locomotion(youmonst.data, "go"));
            }
            return(0);
        } else {
            You("%s %s.", locomotion(youmonst.data, "jump"),
                trap->ttyp == HOLE ? "down the hole" : "through the trap door");
        }
    }

    if (trap && Is_stronghold(&u.uz)) {
        goto_hell(FALSE, TRUE);
    } else {
        at_ladder = (boolean) (levl[u.ux][u.uy].typ == LADDER);
        next_level(!trap);
        at_ladder = FALSE;
    }
    return(1);
}

/** the '<' command */
int
doup()
{
    if (u_rooted()) {
        return 1;
    }

    /* "up" to get out of a pit... */
    if (u.utrap && u.utraptype == TT_PIT) {
        climb_pit();
        return 1;
    }

    if( (u.ux != xupstair || u.uy != yupstair)
        && (!xupladder || u.ux != xupladder || u.uy != yupladder)
        && (!sstairs.sx || u.ux != sstairs.sx || u.uy != sstairs.sy
            || !sstairs.up)
        ) {
        if (do_stair_travel('<')) {
            return 0;
        } else {
            You_cant("go up here.");
            return(0);
        }
    }
#ifdef STEED
    if (stucksteed(TRUE)) {
        return 0;
    }
#endif
    if(u.ustuck) {
        You("are %s, and cannot go up.",
            !u.uswallow ? "being held" : is_animal(u.ustuck->data) ?
            "swallowed" : "engulfed");
        return(1);
    }
    if(near_capacity() > SLT_ENCUMBER) {
        /* No levitation check; inv_weight() already allows for it */
        Your("load is too heavy to climb the %s.",
             levl[u.ux][u.uy].typ == STAIRS ? "stairs" : "ladder");
        return(1);
    }
    if(ledger_no(&u.uz) == 1) {
        if (iflags.debug_fuzzer) {
            return 0;
        }
        if (yn("Beware, there will be no return! Still climb?") != 'y')
            return(0);
    }
    if(!next_to_u()) {
        You("are held back by your pet!");
        return(0);
    }
    at_ladder = (boolean) (levl[u.ux][u.uy].typ == LADDER);
    prev_level(TRUE);
    at_ladder = FALSE;
    return(1);
}

d_level save_dlevel = {0, 0};

/* check that we can write out the current level */
STATIC_OVL int
currentlevel_rewrite()
{
    register int fd;
    char whynot[BUFSZ];

    /* since level change might be a bit slow, flush any buffered screen
     *  output (like "you fall through a trap door") */
    mark_synch();

    fd = create_levelfile(ledger_no(&u.uz), whynot);
    if (fd < 0) {
        /*
         * This is not quite impossible: e.g., we may have
         * exceeded our quota. If that is the case then we
         * cannot leave this level, and cannot save either.
         * Another possibility is that the directory was not
         * writable.
         */
        pline("%s", whynot);
        return -1;
    }

#ifdef MFLOPPY
    if (!savelev(fd, ledger_no(&u.uz), COUNT_SAVE)) {
        (void) nhclose(fd);
        delete_levelfile(ledger_no(&u.uz));
        pline("NetHack is out of disk space for making levels!");
        You("can save, quit, or continue playing.");
        return -1;
    }
#endif
    return fd;
}

#ifdef INSURANCE
void
save_currentstate()
{
    int fd;

    if (flags.ins_chkpt) {
        /* write out just-attained level, with pets and everything */
        fd = currentlevel_rewrite();
        if(fd < 0) return;
        bufon(fd);
        savelev(fd, ledger_no(&u.uz), WRITE_SAVE);
        bclose(fd);
    }

    /* write out non-level state */
    savestateinlock();
}
#endif

/*
   static boolean
   badspot(x, y)
   register xchar x, y;
   {
    return((levl[x][y].typ != ROOM && levl[x][y].typ != AIR &&
             levl[x][y].typ != CORR) || MON_AT(x, y));
   }
 */

/* when arriving on a level, if hero and a monster are trying to share same
   spot, move one; extracted from goto_level(); also used by wiz_makemap() */
void
u_collide_m(mtmp)
struct monst *mtmp;
{
    coord cc;

    if (!mtmp || mtmp == u.usteed || mtmp != m_at(u.ux, u.uy)) {
        impossible("level arrival collision: %s?",
                   !mtmp ? "no monster" :
                   (mtmp == u.usteed) ? "steed is on map" : "monster not co-located");
        return;
    }

    /* There's a monster at your target destination; it might be one
       which accompanied you--see mon_arrive(dogmove.c)--or perhaps
       it was already here.  Randomly move you to an adjacent spot
       or else the monster to any nearby location.  Prior to 3.3.0
       the latter was done unconditionally. */
    if (!rn2(2) && enexto(&cc, u.ux, u.uy, youmonst.data) && distu(cc.x, cc.y) <= 2) {
        u_on_newpos(cc.x, cc.y); /*[maybe give message here?]*/
    } else {
        mnexto(mtmp);
    }

    if ((mtmp = m_at(u.ux, u.uy)) != 0) {
        /* there was an unconditional impossible("mnexto failed")
           here, but it's not impossible and we're prepared to cope
           with the situation, so only say something when debugging */
        if (wizard) {
            pline("(monster in hero's way)");
        }
        if (!rloc(mtmp, TRUE) || (mtmp = m_at(u.ux, u.uy)) != 0) {
            /* no room to move it; send it away, to return later */
            m_into_limbo(mtmp);
        }
    }
}

#ifdef BLACKMARKET
d_level new_dlevel = {0, 0};
#endif

void
goto_level(newlevel, at_stairs, falling, portal)
d_level *newlevel;
boolean at_stairs, falling, portal;
{
    int fd, l_idx;
    xchar new_ledger;
    boolean cant_go_back,
            up = (depth(newlevel) < depth(&u.uz)),
            newdungeon = (u.uz.dnum != newlevel->dnum),
            was_in_W_tower = In_W_tower(u.ux, u.uy, &u.uz),
            familiar = FALSE;
    boolean new = FALSE;    /* made a new level? */
    boolean persistent_level = TRUE;
    struct monst *mtmp;
    char whynot[BUFSZ];
    char *annotation;

    if (dunlev(newlevel) > dunlevs_in_dungeon(newlevel))
        newlevel->dlevel = dunlevs_in_dungeon(newlevel);
    if (newdungeon && In_endgame(newlevel)) { /* 1st Endgame Level !!! */
        if (u.uhave.amulet) {
#ifdef RANDOMIZED_PLANES
            pline("Well done, mortal!");
            pline("But now thou must face the final Test...");
            pline("Prove thyself worthy or perish!");
            assign_level(newlevel, get_first_elemental_plane());
#else
            assign_level(newlevel, &earth_level);
#endif
        }
        else return;
    }
    new_ledger = ledger_no(newlevel);
    if (new_ledger <= 0)
        done(ESCAPED);  /* in fact < 0 is impossible */

#ifdef BLACKMARKET
    assign_level(&new_dlevel, newlevel);
#endif

    /* Prevent the player from going past the first quest level unless
     * (s)he has been given the go-ahead by the leader.
     */
    if (on_level(&u.uz, &qstart_level) && !newdungeon && !ok_to_quest()) {
        pline("A mysterious force prevents you from descending.");
        return;
    }

    if (on_level(newlevel, &u.uz)) return; /* this can happen */

    /* tethered movement makes level change while trapped feasible */
    if (u.utrap && u.utraptype == TT_BURIEDBALL) {
        buried_ball_to_punishment(); /* (before we save/leave old level) */
    }

    fd = currentlevel_rewrite();
    if (fd < 0) return;

    if (falling) /* assuming this is only trap door or hole */
        impact_drop((struct obj *)0, u.ux, u.uy, newlevel->dlevel);

    check_special_room(TRUE); /* probably was a trap door */
    if (Punished) unplacebc();
    reset_trapset(); /* even if to-be-armed trap obj is accompanying hero */
    fill_pit(u.ux, u.uy);
    u.ustuck = 0; /* idem */
    u.uinwater = 0;
    u.uundetected = 0;/* not hidden, even if means are available */
    keepdogs(FALSE);
    if (u.uswallow) {
        /* idem */
        u.uswldtim = u.uswallow = 0;
    }
    recalc_mapseen(); /* recalculate map overview before we leave the level */
    /*
     *  We no longer see anything on the level.  Make sure that this
     *  follows u.uswallow set to null since uswallow overrides all
     *  normal vision.
     */
    vision_recalc(2);

    /*
     *  If this level is supposed to be non-persistent
     *  and there are no unique items present, then set
     *  persistent_level to false and levelport all monsters.
     */
    if (Is_moria_level(&u.uz) && !unique_item_check()) {
        persistent_level = FALSE;
        levelport_monsters();
    }

    /*
     * Save the level we're leaving.  If we're entering the endgame,
     * we can get rid of all existing levels because they cannot be
     * reached any more.  We still need to use savelev()'s cleanup
     * for the level being left, to recover dynamic memory in use and
     * to avoid dangling timers and light sources.
     */
    cant_go_back = (newdungeon && In_endgame(newlevel));
    if (!cant_go_back) {
        update_mlstmv();    /* current monsters are becoming inactive */
        bufon(fd);      /* use buffered output */
    }
    savelev(fd, ledger_no(&u.uz),
            cant_go_back ? FREE_SAVE : (WRITE_SAVE | FREE_SAVE));
    bclose(fd);
    if (cant_go_back) {
        /* discard unreachable levels; keep #0 */
        for (l_idx = maxledgerno(); l_idx > 0; --l_idx)
            delete_levelfile(l_idx);
    }

    /* check if the level should remain persistent */
    if (!persistent_level) {
        delete_levelfile(ledger_no(&u.uz));
        level_info[ledger_no(&u.uz)].flags = 0;
        level_info[ledger_no(&u.uz)].seed = rn2(INT_MAX);
    }

#ifdef REINCARNATION
    if (Is_rogue_level(newlevel) || Is_rogue_level(&u.uz))
        assign_rogue_graphics(Is_rogue_level(newlevel));
#endif
    if (Is_moria_level(newlevel) || Is_moria_level(&u.uz)) {
        assign_moria_graphics(Is_moria_level(newlevel));
    }
#ifdef USE_TILES
    substitute_tiles(newlevel);
#endif
    /* record this level transition as a potential seen branch unless using
     * some non-standard means of transportation (level teleport).
     */
    if ((at_stairs || falling || portal) && (u.uz.dnum != newlevel->dnum))
        recbranch_mapseen(&u.uz, newlevel);
    assign_level(&u.uz0, &u.uz);
    assign_level(&u.uz, newlevel);
    assign_level(&u.utolev, newlevel);
    u.utotype = 0;
    if (!builds_up(&u.uz)) {
        /* usual case */
        if (dunlev(&u.uz) > dunlev_reached(&u.uz)) {
            dunlev_reached(&u.uz) = dunlev(&u.uz);
        }
    } else {
        if (dunlev_reached(&u.uz) == 0 || dunlev(&u.uz) < dunlev_reached(&u.uz)) {
            dunlev_reached(&u.uz) = dunlev(&u.uz);
        }
    }
    reset_rndmonst(NON_PM); /* u.uz change affects monster generation */

    /* set default level change destination areas */
    /* the special level code may override these */
    (void) memset((genericptr_t) &updest, 0, sizeof updest);
    (void) memset((genericptr_t) &dndest, 0, sizeof dndest);

    if (!(level_info[new_ledger].flags & LFILE_EXISTS)) {
        /* entering this level for first time; make it now */
        if (level_info[new_ledger].flags & (FORGOTTEN|VISITED)) {
            impossible("goto_level: returning to discarded level?");
            level_info[new_ledger].flags &= ~(FORGOTTEN|VISITED);
        }
        mklev();
        new = TRUE; /* made the level */
        livelog_printf(LL_DEBUG, "entered new level %d, %s.", dunlev(&u.uz), dungeons[u.uz.dnum].dname);
    } else {
        /* returning to previously visited level; reload it */
        fd = open_levelfile(new_ledger, whynot);
        if (fd < 0) {
            pline("%s", whynot);
            pline("Probably someone removed it.");
            Strcpy(killer.name, whynot);
            done(TRICKED);
            /* we'll reach here if running in wizard mode */
            error("Cannot continue this game.");
        }
        minit();    /* ZEROCOMP */
        getlev(fd, hackpid, new_ledger, FALSE);
        (void) close(fd);
    }
    u.uinwater = 0;
    /* do this prior to level-change pline messages */
    vision_reset();     /* clear old level's line-of-sight */
    vision_full_recalc = 0; /* don't let that reenable vision yet */
    flush_screen(-1);   /* ensure all map flushes are postponed */

    if (portal && !In_endgame(&u.uz)) {
        /* find the portal on the new level */
        register struct trap *ttrap;

        for (ttrap = ftrap; ttrap; ttrap = ttrap->ntrap)
            /* find the portal with the right destination level */
            if (ttrap->ttyp == MAGIC_PORTAL &&
                u.uz0.dnum == ttrap->dst.dnum &&
                u.uz0.dlevel == ttrap->dst.dlevel) break;

        if (!ttrap) panic("goto_level: no corresponding portal!");
        seetrap(ttrap);
        u_on_newpos(ttrap->tx, ttrap->ty);
    } else if (at_stairs && !In_endgame(&u.uz)) {
        if (up) {
            if (at_ladder) {
                u_on_newpos(xdnladder, ydnladder);
            } else {
                if (newdungeon) {
                    if (Is_stronghold(&u.uz)) {
                        register xchar x, y;
                        int trycnt = 0;

                        do {
                            do {
                                x = rn1((updest.hx - updest.lx)+1, updest.lx);
                                y = rn1((updest.hy - updest.ly)+1, updest.ly);
                            } while ((x < updest.nlx ||
                                      x > updest.nhx) &&
                                     (y < updest.nly ||
                                      y > updest.nhy));
                        } while ((occupied(x, y) ||
                                  IS_STWALL(levl[x][y].typ)) && (trycnt++ < 1000));
                        if (trycnt >= 1000) warning("castle: placement failed to find good position"); /* TODO: change impossible() to warning() */
                        u_on_newpos(x, y);
                    } else u_on_sstairs(1);
                } else u_on_dnstairs();
            }
            /* Remove bug which crashes with levitation/punishment  KAA */
            if (Punished && !Levitation) {
                pline("With great effort you climb the %s.",
                      at_ladder ? "ladder" : "stairs");
            } else if (at_ladder)
                You("climb up the ladder.");
        } else {    /* down */
            if (at_ladder) {
                u_on_newpos(xupladder, yupladder);
            } else {
                if (newdungeon) {
                    u_on_sstairs(1);
                } else {
                    u_on_upstairs();
                }
            }
            if (u.dz && Flying)
                You("fly down along the %s.",
                    at_ladder ? "ladder" : "stairs");
            else if (u.dz &&
#ifdef CONVICT
                     (near_capacity() > UNENCUMBERED || (Punished &&
                                                         ((uwep != uball) || ((P_SKILL(P_FLAIL) < P_BASIC))
                                                          || !Role_if(PM_CONVICT)))
                      || Fumbling)) {
#else
                     (near_capacity() > UNENCUMBERED || Punished || Fumbling)) {
#endif /* CONVICT */
                You("fall down the %s.", at_ladder ? "ladder" : "stairs");
                if (Punished) {
                    drag_down();
                    ballrelease(FALSE);
                }
#ifdef STEED
                /* falling off steed has its own losehp() call */
                if (u.usteed)
                    dismount_steed(DISMOUNT_FELL);
                else
#endif
                    losehp(Maybe_Half_Phys(rnd(3)),
                           at_ladder ? "falling off a ladder" : "falling downstairs",
                           KILLED_BY);
                selftouch("Falling, you");
            } else if (u.dz && at_ladder)
                You("climb down the ladder.");
        }
    } else {    /* trap door or level_tele or In_endgame */
        if (was_in_W_tower && On_W_tower_level(&u.uz))
            /* Stay inside the Wizard's tower when feasible.    */
            /* Note: up vs down doesn't really matter in this case. */
            place_lregion(dndest.nlx, dndest.nly,
                          dndest.nhx, dndest.nhy,
                          0, 0, 0, 0, LR_DOWNTELE, (d_level *) 0);
        else if (up)
            place_lregion(updest.lx, updest.ly,
                          updest.hx, updest.hy,
                          updest.nlx, updest.nly,
                          updest.nhx, updest.nhy,
                          LR_UPTELE, (d_level *) 0);
        else
            place_lregion(dndest.lx, dndest.ly,
                          dndest.hx, dndest.hy,
                          dndest.nlx, dndest.nly,
                          dndest.nhx, dndest.nhy,
                          LR_DOWNTELE, (d_level *) 0);
        if (falling) {
            if (Punished) ballfall();
            if (!Flying) selftouch("Falling, you");
        }
    }

    if (Punished) placebc();
    obj_delivery(FALSE);
    losedogs();
    kill_genocided_monsters();  /* for those wiped out while in limbo */
    /*
     * Expire all timers that have gone off while away.  Must be
     * after migrating monsters and objects are delivered
     * (losedogs and obj_delivery).
     */
    run_timers();

    initrack();

#ifdef RECORD_ACHIEVE
#ifdef LIVELOGFILE
    livelog_achieve_update();
#endif
#endif

    if ((mtmp = m_at(u.ux, u.uy)) != 0
#ifdef STEED
        && mtmp != u.usteed
#endif
        ) {
        /* There's a monster at your target destination; it might be one
           which accompanied you--see mon_arrive(dogmove.c)--or perhaps
           it was already here.  Randomly move you to an adjacent spot
           or else the monster to any nearby location.  Prior to 3.3.0
           the latter was done unconditionally. */
        coord cc;

        if (!rn2(2) &&
            enexto(&cc, u.ux, u.uy, youmonst.data) &&
            distu(cc.x, cc.y) <= 2)
            u_on_newpos(cc.x, cc.y); /*[maybe give message here?]*/
        else
            mnexto(mtmp);

        if ((mtmp = m_at(u.ux, u.uy)) != 0) {
            /* there was an unconditional impossible("mnexto failed (do.c)")
               here, but it's not impossible and we're prepared to cope
               with the situation, so only say something when debugging */
            if (wizard)
                pline("(monster in hero's way)");
            if (!rloc(mtmp, TRUE))
                /* no room to move it; send it away, to return later */
                migrate_to_level(mtmp, ledger_no(&u.uz), MIGR_RANDOM,
                                 (coord *) 0);
        }

    }

    /* initial movement of bubbles just before vision_recalc */
    if (Is_waterlevel(&u.uz))
        movebubbles();

    if (level_info[new_ledger].flags & FORGOTTEN) {
        forget_map(ALL_MAP);    /* forget the map */
        forget_traps();     /* forget all traps too */
        familiar = TRUE;
        level_info[new_ledger].flags &= ~FORGOTTEN;
    }

    /* Reset the screen. */
    vision_reset();     /* reset the blockages */
    docrt();        /* does a full vision recalc */
    flush_screen(-1);

    /*
     *  Move all plines beyond the screen reset.
     */

    /* special levels can have a custom arrival message */
    deliver_splev_message();

    /* give room entrance message, if any */
    check_special_room(FALSE);

    /* deliver objects traveling with player */
    obj_delivery(TRUE);

    /* Check whether we just entered Gehennom. */
    if (!In_hell(&u.uz0) && Inhell) {
        if (Is_valley(&u.uz)) {
            You("arrive at the Valley of the Dead...");
            pline_The("odor of burnt flesh and decay pervades the air.");
#ifdef MICRO
            display_nhwindow(WIN_MESSAGE, FALSE);
#endif
            You_hear("groans and moans everywhere.");
        } else if (!In_sheol(&u.uz))
            pline("It is hot here.  You smell smoke...");
        else
            pline("It is freezing here.  You feel cold wind...");

#ifdef RECORD_ACHIEVE
        achieve.enter_gehennom = 1;
#ifdef LIVELOGFILE
        livelog_achieve_update();
#endif
#endif
    }
    else if (In_hell(&u.uz0) &&
             !In_sheol(&u.uz0) &&
             In_sheol(&u.uz)) {
        pline("It is freezing here.  You feel cold wind...");
    }
    else if (In_sheol(&u.uz0) &&
             In_hell(&u.uz) &&
             !In_sheol(&u.uz)) {
        pline("It is hot here.  You smell smoke...");
    }

    if (familiar) {
        static const char * const fam_msgs[4] = {
            "You have a sense of deja vu.",
            "You feel like you've been here before.",
            "This place %s familiar...",
            0 /* no message */
        };
        static const char * const halu_fam_msgs[4] = {
            "Whoa!  Everything %s different.",
            "You are surrounded by twisty little passages, all alike.",
            "Gee, this %s like uncle Conan's place...",
            0 /* no message */
        };
        const char *mesg;
        char buf[BUFSZ];
        int which = rn2(4);

        if (Hallucination)
            mesg = halu_fam_msgs[which];
        else
            mesg = fam_msgs[which];
        if (mesg && index(mesg, '%')) {
            Sprintf(buf, mesg, !Blind ? "looks" : "seems");
            mesg = buf;
        }
        if (mesg) pline("%s", mesg);
    }

#ifdef REINCARNATION
    if (new && Is_rogue_level(&u.uz))
        You("enter what seems to be an older, more primitive world.");
#endif
    if (new && Hallucination &&
        Role_if(PM_ARCHEOLOGIST) &&
        Is_juiblex_level(&u.uz))
        pline("Ahh, Venice.");

    /* Final confrontation */
    if (In_endgame(&u.uz) && newdungeon && u.uhave.amulet)
        resurrect();
    if (newdungeon && In_V_tower(&u.uz) && In_hell(&u.uz0))
        pline_The("heat and smoke are gone.");

    /* the message from your quest leader */
    if (!In_quest(&u.uz0) && at_dgn_entrance("The Quest") &&
        !(u.uevent.qexpelled || u.uevent.qcompleted || quest_status.leader_is_dead)) {
        if (u.uprops[TELEPAT].blocked) {
            if (uarmh)
                You("sense something being blocked by %s.", yname(uarmh));
        } else if (u.uevent.qcalled) {
            com_pager(Role_if(PM_ROGUE) ? 4 : 3);
        } else {
            com_pager(2);
            u.uevent.qcalled = TRUE;
        }
    }

#ifdef ADVENT_CALENDAR
    if ((getmonth()==12) && (getmday() < 25)) {
        if (mk_advcal_portal())
            You("smell chocolate!");
    }
#endif

    /* once Croesus is dead, his alarm doesn't work any more */
    if (Is_knox(&u.uz) && (new || !mvitals[PM_CROESUS].died)) {
        You("penetrated a high security area!");
        pline("An alarm sounds!");
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (!DEADMONSTER(mtmp) && mtmp->msleeping) {
                mtmp->msleeping = 0;
            }
        }
    }

    if (newdungeon) {
        if (In_mines(&u.uz)) {
            record_uevent_achievement("entered the Gnomish Mines", entered_the_gnomish_mines);
        } else if (In_sokoban(&u.uz)) {
            record_uevent_achievement("entered Sokoban", entered_sokoban);
        } else if (Is_blackmarket(&u.uz)) {
            record_uevent_achievement("entered the Blackmarket", entered_the_blackmarket);
        } else if (In_dragon_caves(&u.uz)) {
            record_uevent_achievement("entered the Dragon Caves", entered_the_dragon_caves);
        } else if (In_moria(&u.uz)) {
            record_uevent_achievement("entered Moria", entered_moria);
        } else if (In_sheol(&u.uz)) {
            record_uevent_achievement("entered Sheol", entered_sheol);
        } else if (In_V_tower(&u.uz)) {
            record_uevent_achievement("entered Vlad's tower", entered_vlads_tower);
        } else if (Is_knox(&u.uz)) {
            record_uevent_achievement("entered Fort Ludios", entered_fort_ludios);
        }
    }

    if (new) {
        if (Is_bigroom(&u.uz)) {
            record_uevent_achievement("entered the Big Room", entered_the_bigroom);
        } else if (Is_town_level(&u.uz)) {
            record_uevent_achievement("entered the Town", entered_the_town);
        }
    }

    if (on_level(&u.uz, &astral_level))
        final_level();
    else
        onquest();
    assign_level(&u.uz0, &u.uz); /* reset u.uz0 */

    /* show level annotation when entering the level */
    if (iflags.show_annotation &&
        (annotation = get_annotation(&u.uz))) {
        You("annotated this level: %s", annotation);
    }

#ifdef INSURANCE
    save_currentstate();
#endif

    /* assume this will always return TRUE when changing level */
    (void) in_out_region(u.ux, u.uy);
    (void) pickup(1);
#ifdef WHEREIS_FILE
    touch_whereis();
#endif
}

STATIC_OVL void
final_level()
{
    struct monst *mtmp;
    struct obj *otmp;
    coord mm;
    int i;

    /* reset monster hostility relative to player */
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
        if (!DEADMONSTER(mtmp)) reset_hostility(mtmp);

    /* create some player-monsters */
    create_mplayers(rn1(4, 3), TRUE);

    /* create a guardian angel next to player, if worthy */
    if (Conflict) {
        pline(
            "A voice booms: \"Thy desire for conflict shall be fulfilled!\"");
        for (i = rnd(4); i > 0; --i) {
            mm.x = u.ux;
            mm.y = u.uy;
            if (enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL]))
                (void) mk_roamer(&mons[PM_ANGEL], u.ualign.type,
                                 mm.x, mm.y, FALSE);
        }

    } else if (u.ualign.record > 8) {   /* fervent */
        pline("A voice whispers: \"Thou hast been worthy of me!\"");
        mm.x = u.ux;
        mm.y = u.uy;
        if (enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL])) {
            if ((mtmp = mk_roamer(&mons[PM_ANGEL], u.ualign.type,
                                  mm.x, mm.y, TRUE)) != 0) {
                if (!Blind)
                    pline("An angel appears near you.");
                else
                    You_feel("the presence of a friendly angel near you.");
                /* guardian angel -- the one case mtame doesn't
                 * imply an edog structure, so we don't want to
                 * call tamedog().
                 */
                mtmp->mtame = 10;
                /* make him strong enough vs. endgame foes */
                mtmp->m_lev = rn1(8, 15);
                mtmp->mhp = mtmp->mhpmax =
                    d((int)mtmp->m_lev, 10) + 30 + rnd(30);
                if ((otmp = select_hwep(mtmp)) == 0) {
                    otmp = mksobj(SILVER_SABER, FALSE, FALSE);
                    if (mpickobj(mtmp, otmp))
                        panic("merged weapon?");
                }
                bless(otmp);
                if (otmp->spe < 4) otmp->spe += rnd(4);
                if ((otmp = which_armor(mtmp, W_ARMS)) == 0 ||
                    otmp->otyp != SHIELD_OF_REFLECTION) {
                    (void) mongets(mtmp, AMULET_OF_REFLECTION);
                    m_dowear(mtmp, TRUE);
                }
            }
        }
    }
}

static char *dfr_pre_msg = 0,   /* pline() before level change */
            *dfr_post_msg = 0; /* pline() after level change */

/* change levels at the end of this turn, after monsters finish moving */
void
schedule_goto(tolev, at_stairs, falling, portal_flag, pre_msg, post_msg)
d_level *tolev;
boolean at_stairs, falling;
int portal_flag;
const char *pre_msg, *post_msg;
{
    int typmask = 0100;     /* non-zero triggers `deferred_goto' */

    /* destination flags (`goto_level' args) */
    if (at_stairs) typmask |= 1;
    if (falling) typmask |= 2;
    if (portal_flag) typmask |= 4;
    if (portal_flag < 0) typmask |= 0200;   /* flag for portal removal */
    u.utotype = typmask;
    /* destination level */
    assign_level(&u.utolev, tolev);

    if (pre_msg) {
        dfr_pre_msg = dupstr(pre_msg);
    }
    if (post_msg) {
        dfr_post_msg = dupstr(post_msg);
    }
}

/* handle something like portal ejection */
void
deferred_goto()
{
    if (!on_level(&u.uz, &u.utolev)) {
        d_level dest;
        int typmask = u.utotype; /* save it; goto_level zeroes u.utotype */

        assign_level(&dest, &u.utolev);
        if (dfr_pre_msg) pline("%s", dfr_pre_msg);
        goto_level(&dest, !!(typmask&1), !!(typmask&2), !!(typmask&4));
        if (typmask & 0200) {   /* remove portal */
            struct trap *t = t_at(u.ux, u.uy);

            if (t) {
                deltrap(t);
                newsym(u.ux, u.uy);
            }
        }
        if (dfr_post_msg) pline("%s", dfr_post_msg);
    }
    u.utotype = 0; /* our caller keys off of this */
    if (dfr_pre_msg)
        free((genericptr_t)dfr_pre_msg),  dfr_pre_msg = 0;
    if (dfr_post_msg)
        free((genericptr_t)dfr_post_msg),  dfr_post_msg = 0;
}

/*
 * Tries to create a monster for the corpse.
 *
 * Returns TRUE if the corpse is gone afterwards.
 */
boolean
revive_corpse(corpse)
struct obj *corpse;
{
    struct monst *mtmp = 0, *mcarry;
    boolean is_uwep, chewed;
    xchar where;
    char cname[BUFSZ];
    struct obj *container = (struct obj *)0;
    int container_where = 0;
    boolean is_zomb = (mons[corpse->corpsenm].mlet == S_ZOMBIE);

    where = corpse->where;
    is_uwep = corpse == uwep;
    chewed = (corpse->oeaten != 0);
    Strcpy(cname, corpse_xname(corpse,
                               chewed ? "bite-covered" : (const char *) 0,
                               CXN_SINGULAR));
    mcarry = (where == OBJ_MINVENT) ? corpse->ocarry : 0;

    if (where == OBJ_CONTAINED) {
        struct monst *mtmp2 = (struct monst *)0;
        container = corpse->ocontainer;
        mtmp2 = get_container_location(container, &container_where, (int *)0);
        /* container_where is the outermost container's location even if nested */
        if (container_where == OBJ_MINVENT && mtmp2) mcarry = mtmp2;
    }

    if (corpse->odrained &&
        !is_rider(&mons[corpse->corpsenm]) &&
        corpse->corpsenm != PM_WIZARD_OF_YENDOR &&
        (corpse->oeaten > drainlevel(corpse)) == !!rn2(3)) {
        /* 2/3 chance of failing to revive a drained corpse (1/3 for partially drained) */
        boolean showmsg = cansee(corpse->ox, corpse->oy) &&
                          (where == OBJ_INVENT || where == OBJ_FLOOR);
        if (rn2(2)) {
            remove_corpse(corpse);
            if (showmsg) {
                pline("%s drained %s stirs briefly, then evaporates into dust.",
                      where == OBJ_INVENT ? "Your" : "The", cname);
            }
            return TRUE;
        } else if (showmsg) {
            pline("%s drained %s stirs with life, but doesn't have enough blood to survive.",
                  where == OBJ_INVENT ? "Your" : "The", cname);
            return FALSE;
        }
    }

    mtmp = revive(corpse, FALSE); /* corpse is gone if successful */

    if (mtmp) {
        switch (where) {
        case OBJ_INVENT:
            if (is_uwep)
                pline_The("%s writhes out of your grasp!", cname);
            else
                You_feel("squirming in your backpack!");
            break;

        case OBJ_FLOOR:
            if (cansee(mtmp->mx, mtmp->my))
                pline("%s rises from the dead!", chewed ?
                      Adjmonnam(mtmp, "bite-covered") : Monnam(mtmp));
            break;

        case OBJ_MINVENT:       /* probably a nymph's */
            if (cansee(mtmp->mx, mtmp->my)) {
                if (canseemon(mcarry))
                    pline("Startled, %s drops %s as it revives!",
                          mon_nam(mcarry), an(cname));
                else
                    pline("%s suddenly appears!", chewed ?
                          Adjmonnam(mtmp, "bite-covered") : Monnam(mtmp));
            }
            break;
        case OBJ_CONTAINED:
            if (container_where == OBJ_MINVENT && cansee(mtmp->mx, mtmp->my) &&
                mcarry && canseemon(mcarry) && container) {
                char sackname[BUFSZ];
                Sprintf(sackname, "%s %s", s_suffix(mon_nam(mcarry)),
                        xname(container));
                pline("%s writhes out of %s!", Amonnam(mtmp), sackname);
            } else if (container_where == OBJ_INVENT && container) {
                char sackname[BUFSZ];
                Strcpy(sackname, an(xname(container)));
                pline("%s %s out of %s in your pack!",
                      Blind ? Something : Amonnam(mtmp),
                      locomotion(mtmp->data, "writhes"),
                      sackname);
            } else if (container_where == OBJ_FLOOR && container &&
                       cansee(mtmp->mx, mtmp->my)) {
                char sackname[BUFSZ];
                Strcpy(sackname, an(xname(container)));
                pline("%s escapes from %s!", Amonnam(mtmp), sackname);
            }
            break;

        case OBJ_BURIED:
            if (is_zomb) {
                maketrap(mtmp->mx, mtmp->my, PIT);
                if (cansee(mtmp->mx, mtmp->my)) {
                    struct trap *ttmp;

                    ttmp = t_at(mtmp->mx, mtmp->my);
                    ttmp->tseen = TRUE;
                    pline("%s claws itself out of the ground!", Amonnam(mtmp));
                    newsym(mtmp->mx, mtmp->my);
                } else if (distu(mtmp->mx, mtmp->my) < 5*5) {
                    You_hear("scratching noises.");
                }
                break;
            }
            /* fall through */

        default:
            /* we should be able to handle the other cases... */
            impossible("revive_corpse: lost corpse @ %d", where);
            break;
        }
        return TRUE;
    }
    return FALSE;
}

/* Revive the corpse via a timeout. */
/*ARGSUSED*/
void
revive_mon(arg, timeout)
anything *arg;
long timeout UNUSED;
{
    struct obj *body = arg->a_obj;
    struct permonst *mptr = &mons[body->corpsenm];

    /* if we succeed, the corpse is gone, otherwise, rot it away */
    if (!revive_corpse(body)) {
        long when;
        int action;

        if (is_rider(mptr) && rn2(99)) {
            /* Rider usually tries again */
            action = REVIVE_MON;
            for (when = 3L; when < 67L; when++) {
                if (!rn2(3)) {
                    break;
                }
            }
        } else { /* rot this corpse away */
            if (!obj_has_timer(body, ROT_CORPSE)) {
                You_feel("%sless hassled.", is_rider(mptr) ? "much " : "");
            }
            action = ROT_CORPSE;
            when = 250L - (moves - body->age);
            if (when < 1L) {
                when = 1L;
            }
        }
        if (!obj_has_timer(body, action)) {
            (void) start_timer(when, TIMER_OBJECT, action, arg);
        }
    }
}

/** Timeout callback. Revive the corpse as a zombie. */
void
zombify_mon(anything *arg, long timeout)
{
    struct obj *body = arg->a_obj;
    int zmon = zombie_form(&mons[body->corpsenm]);

    if (zmon != NON_PM && !(mvitals[zmon].mvflags & G_GENOD)) {
        if (has_omid(body)) {
            free_omid(body);
        }
        if (has_omonst(body)) {
            free_omonst(body);
        }

        set_corpsenm(body, zmon);
        revive_mon(arg, timeout);
    }
}

int
donull()
{
    return(1);  /* Do nothing, but let other things happen */
}

STATIC_PTR int
wipeoff()
{
    if(u.ucreamed < 4) u.ucreamed = 0;
    else u.ucreamed -= 4;
    if (Blinded < 4) Blinded = 0;
    else Blinded -= 4;
    if (!Blinded) {
        pline("You've got the glop off.");
        u.ucreamed = 0;
        if (!gulp_blnd_check()) {
            Blinded = 1;
            make_blinded(0L, TRUE);
        }
        return(0);
    } else if (!u.ucreamed) {
        Your("%s feels clean now.", body_part(FACE));
        return(0);
    }
    return(1);      /* still busy */
}

int
dowipe()
{
    if(u.ucreamed)  {
        static NEARDATA char buf[39];

        Sprintf(buf, "wiping off your %s", body_part(FACE));
        set_occupation(wipeoff, buf, 0);
        /* Not totally correct; what if they change back after now
         * but before they're finished wiping?
         */
        return(1);
    }
    Your("%s is already clean.", body_part(FACE));
    return(1);
}

void
set_wounded_legs(side, timex)
register long side;
register int timex;
{
    /* KMH -- STEED
     * If you are riding, your steed gets the wounded legs instead.
     * You still call this function, but don't lose hp.
     * Caller is also responsible for adjusting messages.
     */

    if(!Wounded_legs) {
        ATEMP(A_DEX)--;
        flags.botl = 1;
    }

    if(!Wounded_legs || (HWounded_legs & TIMEOUT))
        HWounded_legs = timex;
    EWounded_legs = side;
    (void)encumber_msg();
}

void
heal_legs(how)
int how; /* 0: ordinary, 1: dismounting steed, 2: limbs turn to stone */
{
    if (Wounded_legs) {
        if (ATEMP(A_DEX) < 0) {
            ATEMP(A_DEX)++;
            flags.botl = 1;
        }

        /* when mounted, wounded legs applies to the steed;
           during petrification countdown, "your limbs turn to stone"
           before the final stages and that calls us (how==2) to cure
           wounded legs, but we want to suppress the feel better message */
#ifdef STEED
        if (!u.usteed && how != 2)
#endif
        {
            /* KMH, intrinsics patch */
            if((EWounded_legs & BOTH_SIDES) == BOTH_SIDES) {
                Your("%s feel somewhat better.",
                     makeplural(body_part(LEG)));
            } else {
                Your("%s feels somewhat better.",
                     body_part(LEG));
            }
        }
        HWounded_legs = EWounded_legs = 0;

        /* Wounded_legs reduces carrying capacity, so we want
           an encumbrance check when they're healed.  However,
           while dismounting, first steed's legs get healed,
           then hero is dropped to floor and a new encumbrance
           check is made [in dismount_steed()].  So don't give
           encumbrance feedback during the dismount stage
           because it could seem to be shown out of order and
           it might be immediately contradicted [able to carry
           more when steed becomes healthy, then possible floor
           feedback, then able to carry less when back on foot]. */
        if (how == 0) {
            (void) encumber_msg();
        }
    }
}

/* return true if any unique item is on the floor or in monsters' possession */
boolean
unique_item_check()
{
    register struct obj *obj;
    register struct monst *mtmp;

    for (obj = fobj; obj; obj = obj->nobj) {
        if (is_unique(obj)) return TRUE;
    }
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        for (obj = mtmp->minvent; obj; obj = obj->nobj) {
            if (is_unique(obj)) return TRUE;
        }
    }

    return FALSE;
}

/* prevent monsters from "poofing" -- disappearing due to non-persistent levels */
void
levelport_monsters()
{
    register struct monst *mtmp, *mtmp2;
    int nlev;

    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;
        d_level flev;
        nlev = random_teleport_level();
        get_level(&flev, nlev);
        migrate_to_level(mtmp, ledger_no(&flev), MIGR_RANDOM, (coord *)0);
    }
}

/*do.c*/
