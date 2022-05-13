/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This file contains the command routines dowhatis() and dohelp() and */
/* a few other help related facilities */

#include "hack.h"
#include "dlb.h"

STATIC_DCL boolean FDECL(is_swallow_sym, (int));
STATIC_DCL int FDECL(append_str, (char *, const char *));
STATIC_DCL struct permonst * FDECL(lookat, (int, int, char *, char *));
static void look_all(boolean, boolean, boolean);
static void add_obj_info(winid, struct obj *, short);
static void add_mon_info(winid, struct permonst *);
static void FDECL(do_supplemental_info, (char *, struct permonst *, BOOLEAN_P));
STATIC_DCL boolean FDECL(help_menu, (int *));
#ifdef PORT_HELP
extern void NDECL(port_help);
#endif
static boolean lookup_database_entry(dlb *fp, struct obj *obj, const char* dbase_str, const char* inp,
        struct permonst *pm,
        boolean user_typed_name, boolean without_asking, char *supplemental_name);

/* Returns "true" for characters that could represent a monster's stomach. */
STATIC_OVL boolean
is_swallow_sym(c)
int c;
{
    int i;
    for (i = S_sw_tl; i <= S_sw_br; i++)
        if ((int)showsyms[i] == c) return TRUE;
    return FALSE;
}

/*
 * Append new_str to the end of buf if new_str doesn't already exist as
 * a substring of buf.  Return 1 if the string was appended, 0 otherwise.
 * It is expected that buf is of size BUFSZ.
 */
STATIC_OVL int
append_str(buf, new_str)
char *buf;
const char *new_str;
{
    int space_left; /* space remaining in buf */

    if (strstri(buf, new_str)) return 0;

    space_left = BUFSZ - strlen(buf) - 1;
    if (space_left < 1) {
        return 0;
    }
    (void) strncat(buf, " or ", space_left);
    (void) strncat(buf, new_str, space_left - 4);
    return 1;
}

/* shared by monster probing (via query_objlist!) as well as lookat() */
char *
self_lookat(outbuf)
char *outbuf;
{
    char race[QBUFSZ];

    /* include race with role unless polymorphed */
    race[0] = '\0';
    if (!Upolyd) {
        Sprintf(race, "%s ", urace.adj);
    }
    Sprintf(outbuf, "%s%s%s called %s",
            /* being blinded may hide invisibility from self */
            (Invis && (senseself() || !Blind)) ? "invisible " : "", race,
            mons[u.umonnum].mname, plname);
    if (u.usteed) {
        Sprintf(eos(outbuf), ", mounted on %s", y_monnam(u.usteed));
    }
    if (u.uundetected || (Upolyd && U_AP_TYPE)) {
        mhidden_description(&youmonst, FALSE, eos(outbuf));
    }
    return outbuf;
}

/* describe a hidden monster; used for look_at during extended monster
   detection and for probing; also when looking at self */
void
mhidden_description(mon, altmon, outbuf)
struct monst *mon;
boolean altmon; /* for probing: if mimicking a monster, say so */
char *outbuf;
{
    struct obj *otmp;
    boolean fakeobj, isyou = (mon == &youmonst);
    int x = isyou ? u.ux : mon->mx;
    int y = isyou ? u.uy : mon->my;
    int glyph = (level.flags.hero_memory && !isyou) ? levl[x][y].glyph : glyph_at(x, y);

    *outbuf = '\0';
    if (M_AP_TYPE(mon) == M_AP_FURNITURE || M_AP_TYPE(mon) == M_AP_OBJECT) {
        Strcpy(outbuf, ", mimicking ");
        if (M_AP_TYPE(mon) == M_AP_FURNITURE) {
            Strcat(outbuf, an(defsyms[mon->mappearance].explanation));
        } else if (M_AP_TYPE(mon) == M_AP_OBJECT &&
                   /* remembered glyph, not glyph_at() which is 'mon' */
                   glyph_is_object(glyph)) {
 objfrommap:
            otmp = (struct obj *) 0;
            fakeobj = object_from_map(glyph, x, y, &otmp);
            Strcat(outbuf, (otmp && otmp->otyp != STRANGE_OBJECT) ?
                            ansimpleoname(otmp) : an(obj_descr[STRANGE_OBJECT].oc_name));
            if (fakeobj) {
                otmp->where = OBJ_FREE; /* object_from_map set to OBJ_FLOOR */
                dealloc_obj(otmp);
            }
        } else {
            Strcat(outbuf, something);
        }
    } else if (M_AP_TYPE(mon) == M_AP_MONSTER) {
        if (altmon) {
            Sprintf(outbuf, ", masquerading as %s", an(mons[mon->mappearance].mname));
        }
    } else if (isyou ? u.uundetected : mon->mundetected) {
        Strcpy(outbuf, ", hiding");
        if (hides_under(mon->data)) {
            Strcat(outbuf, " under ");
            /* remembered glyph, not glyph_at() which is 'mon' */
            if (glyph_is_object(glyph)) {
                goto objfrommap;
            }
            Strcat(outbuf, something);
        } else if (is_hider(mon->data)) {
            Sprintf(eos(outbuf), " on the %s",
                    (is_flyer(mon->data) ||
                     mon->data->mlet == S_PIERCER) ? "ceiling" : surface(x, y)); /* trapper */
        } else {
            if (mon->data->mlet == S_EEL && is_pool(x, y)) {
                Strcat(outbuf, " in murky water");
            }
        }
    }
}

/* extracted from lookat(); also used by namefloorobj() */
boolean
object_from_map(glyph, x, y, obj_p)
int glyph, x, y;
struct obj **obj_p;
{
    boolean fakeobj = FALSE, mimic_obj = FALSE;
    struct monst *mtmp;
    struct obj *otmp;
    int glyphotyp = glyph_to_obj(glyph);

    *obj_p = (struct obj *) 0;
    /* TODO: check inside containers in case glyph came from detection */
    if ((otmp = sobj_at(glyphotyp, x, y)) == 0) {
        for (otmp = level.buriedobjlist; otmp; otmp = otmp->nobj) {
            if (otmp->ox == x && otmp->oy == y && otmp->otyp == glyphotyp) {
                break;
            }
        }
    }

    /* there might be a mimic here posing as an object */
    mtmp = m_at(x, y);
    if (mtmp && is_obj_mappear(mtmp, (unsigned) glyphotyp)) {
        otmp = 0;
        mimic_obj = TRUE;
    } else {
        mtmp = 0;
    }

    if (!otmp || otmp->otyp != glyphotyp) {
        /* this used to exclude STRANGE_OBJECT; now caller deals with it */
        otmp = mksobj(glyphotyp, FALSE, FALSE);
        if (!otmp) {
            return FALSE;
        }
        fakeobj = TRUE;
        if (otmp->oclass == COIN_CLASS) {
            otmp->quan = 2L; /* to force pluralization */
        } else if (otmp->otyp == SLIME_MOLD) {
            otmp->spe = current_fruit; /* give it a type */
        }
        if (mtmp && has_mcorpsenm(mtmp)) {
            /* mimic as corpse/statue */
            if (otmp->otyp == SLIME_MOLD) {
                /* override current_fruit to avoid
                     look, use 'O' to make new named fruit, look again
                   giving different results when current_fruit changes */
                otmp->spe = MCORPSENM(mtmp);
            } else {
                otmp->corpsenm = MCORPSENM(mtmp);
            }
        } else if (otmp->otyp == CORPSE && glyph_is_body(glyph)) {
            otmp->corpsenm = glyph - GLYPH_BODY_OFF;
        } else if (otmp->otyp == STATUE && glyph_is_statue(glyph)) {
            otmp->corpsenm = glyph - GLYPH_STATUE_OFF;
        }
        if (otmp->otyp == LEASH) {
            otmp->leashmon = 0;
        }
        /* extra fields needed for shop price with doname() formatting */
        otmp->where = OBJ_FLOOR;
        otmp->ox = x, otmp->oy = y;
        otmp->no_charge = (otmp->otyp == STRANGE_OBJECT && costly_spot(x, y));
    }
    /* if located at adjacent spot, mark it as having been seen up close
       (corpse type will be known even if dknown is 0, so we don't need a
       touch check for cockatrice corpse--we're looking without touching) */
    if (otmp && distu(x, y) <= 2 && !Blind && !Hallucination &&
        /* redundant: we only look for an object which matches current
           glyph among floor and buried objects; when !Blind, any buried
           object's glyph will have been replaced by whatever is present
           on the surface as soon as we moved next to its spot */
        (fakeobj || otmp->where == OBJ_FLOOR) && /* not buried */
        /* terrain mode views what's already known, doesn't learn new stuff */
        !iflags.terrainmode) {
        /* so don't set dknown when in terrain mode */
        otmp->dknown = 1; /* if a pile, clearly see the top item only */
    }
    if (fakeobj && mtmp && mimic_obj &&
        (otmp->dknown || (M_AP_FLAG(mtmp) & M_AP_F_DKNOWN))) {
        mtmp->m_ap_type |= M_AP_F_DKNOWN;
        otmp->dknown = 1;
    }
    *obj_p = otmp;
    return fakeobj; /* when True, caller needs to dealloc *obj_p */
}

static void
look_at_object(buf, x, y, glyph)
char *buf; /* output buffer */
int x, y, glyph;
{
    struct obj *otmp = 0;
    boolean fakeobj = object_from_map(glyph, x, y, &otmp);

    if (otmp) {
        Strcpy(buf, (otmp->otyp != STRANGE_OBJECT) ?
                      distant_name(otmp, doname_with_price) : obj_descr[STRANGE_OBJECT].oc_name);
        if (fakeobj) {
            otmp->where = OBJ_FREE; /* object_from_map set it to OBJ_FLOOR */
            dealloc_obj(otmp), otmp = 0;
        }
    } else {
        Strcpy(buf, something); /* sanity precaution */
    }

    if (otmp && otmp->where == OBJ_BURIED) {
        Strcat(buf, " (buried)");
    } else if (levl[x][y].typ == STONE || levl[x][y].typ == SCORR) {
        Strcat(buf, " embedded in stone");
    } else if (IS_WALL(levl[x][y].typ) || levl[x][y].typ == SDOOR) {
        Strcat(buf, " embedded in a wall");
    } else if (closed_door(x, y)) {
        Strcat(buf, " embedded in a door");
    } else if (is_pool(x, y)) {
        Strcat(buf, " in water");
    } else if (is_lava(x, y)) {
        Strcat(buf, " in molten lava"); /* [can this ever happen?] */
    }
    return;
}

static void
look_at_monster(buf, monbuf, mtmp, x, y)
char *buf, *monbuf; /* buf: output, monbuf: optional output */
struct monst *mtmp;
int x, y;
{
    char *name, monnambuf[BUFSZ];
    boolean accurate = !Hallucination;

    name = (mtmp->data == &mons[PM_COYOTE] && accurate) ?
            coyotename(mtmp, monnambuf) : distant_monnam(mtmp, ARTICLE_NONE, monnambuf);

    Sprintf(buf, "%s%s%s",
            (mtmp->mx != x || mtmp->my != y) ?
             ((mtmp->isshk && accurate) ? "tail of " : "tail of a ") : "",
            (mtmp->mtame && accurate) ?
             "tame " : (mtmp->mpeaceful && accurate) ? "peaceful " : "",
            name);

    char *mwounds = mon_wounds(mtmp);
    if (mwounds) {
        Strcat(buf, ", ");
        Strcat(buf, mwounds);
    }

    if (u.ustuck == mtmp) {
        if (u.uswallow || iflags.save_uswallow) {
            /* monster detection */
            Strcat(buf, is_animal(mtmp->data) ? ", swallowing you" : ", engulfing you");
        } else {
            Strcat(buf, (Upolyd && sticks(youmonst.data)) ? ", being held" : ", holding you");
        }
    }
    if (mtmp->mleashed) {
        Strcat(buf, ", leashed to you");
    }

    if (mtmp->mtrapped && cansee(mtmp->mx, mtmp->my)) {
        struct trap *t = t_at(mtmp->mx, mtmp->my);
        int tt = t ? t->ttyp : NO_TRAP;

        /* newsym lets you know of the trap, so mention it here */
        if (tt == BEAR_TRAP || is_pit(tt) || tt == WEB) {
            Sprintf(eos(buf), ", trapped in %s",
                    an(defsyms[trap_to_defsym(tt)].explanation));
            t->tseen = 1;
        }
    }

    /* we know the hero sees a monster at this location, but if it's shown
       due to persistant monster detection he might remember something else */
    if (mtmp->mundetected || M_AP_TYPE(mtmp))
        mhidden_description(mtmp, FALSE, eos(buf));

    if (monbuf) {
        unsigned how_seen = how_mon_is_seen(mtmp);

        monbuf[0] = '\0';
        if (how_seen != 0 && how_seen != MONSEEN_NORMAL) {
            if (how_seen & MONSEEN_NORMAL) {
                Strcat(monbuf, "normal vision");
                how_seen &= ~MONSEEN_NORMAL;
                /* how_seen can't be 0 yet... */
                if (how_seen) {
                    Strcat(monbuf, ", ");
                }
            }
            if (how_seen & MONSEEN_SEEINVIS) {
                Strcat(monbuf, "see invisible");
                how_seen &= ~MONSEEN_SEEINVIS;
                if (how_seen) {
                    Strcat(monbuf, ", ");
                }
            }
            if (how_seen & MONSEEN_INFRAVIS) {
                Strcat(monbuf, "infravision");
                how_seen &= ~MONSEEN_INFRAVIS;
                if (how_seen) {
                    Strcat(monbuf, ", ");
                }
            }
            if (how_seen & MONSEEN_TELEPAT) {
                Strcat(monbuf, "telepathy");
                how_seen &= ~MONSEEN_TELEPAT;
                if (how_seen) {
                    Strcat(monbuf, ", ");
                }
            }
            if (how_seen & MONSEEN_XRAYVIS) {
                /* Eyes of the Overworld */
                Strcat(monbuf, "astral vision");
                how_seen &= ~MONSEEN_XRAYVIS;
                if (how_seen) {
                    Strcat(monbuf, ", ");
                }
            }
            if (how_seen & MONSEEN_DETECT) {
                Strcat(monbuf, "monster detection");
                how_seen &= ~MONSEEN_DETECT;
                if (how_seen) {
                    Strcat(monbuf, ", ");
                }
            }
            if (how_seen & MONSEEN_WARNMON) {
                char wbuf[BUFSZ];
                if (Hallucination) {
                    Strcat(monbuf, "paranoid delusion");
                } else {
                    Sprintf(wbuf, "warned of %s",
                            makeplural(mtmp->data->mname));
                    Strcat(monbuf, wbuf);
                }
                how_seen &= ~MONSEEN_WARNMON;
                if (how_seen) {
                    Strcat(monbuf, ", ");
                }
            }
            /* should have used up all the how_seen bits by now */
            if (how_seen) {
                impossible("lookat: unknown method of seeing monster");
                Sprintf(eos(monbuf), "(%u)", how_seen);
            }
        } /* seen by something other than normal vision */
    } /* monbuf is non-null */
}

/*
 * Return the name of the glyph found at (x,y).
 * If not hallucinating and the glyph is a monster, also monster data.
 */
STATIC_OVL struct permonst *
lookat(x, y, buf, monbuf)
int x, y;
char *buf, *monbuf;
{
    struct monst *mtmp = (struct monst *) 0;
    struct permonst *pm = (struct permonst *) 0;
    int glyph;

    buf[0] = monbuf[0] = '\0';
    glyph = glyph_at(x, y);
    if (u.ux == x && u.uy == y && canspotself() &&
         !(iflags.save_uswallow &&
           glyph == mon_to_glyph(u.ustuck)) &&
         (!iflags.terrainmode || (iflags.terrainmode & TER_MON) != 0)) {
        /* fill in buf[] */
        (void) self_lookat(buf);

        /* file lookup can't distinguish between "gnomish wizard" monster
           and correspondingly named player character, always picking the
           former; force it to find the general "wizard" entry instead */
        if (Role_if(PM_WIZARD) && Race_if(PM_GNOME) && !Upolyd)
            pm = &mons[PM_WIZARD];

        /* When you see yourself normally, no explanation is appended
           (even if you could also see yourself via other means).
           Sensing self while blind or swallowed is treated as if it
           were by normal vision (cf canseeself()). */
        if ((Invisible || u.uundetected) && !Blind && !(u.uswallow || iflags.save_uswallow)) {
            unsigned how = 0;

            if (Infravision) how |= 1;
            if (Unblind_telepat) how |= 2;
            if (Detect_monsters) how |= 4;

            if (how)
                Sprintf(eos(buf), " [seen: %s%s%s%s%s]",
                        (how & 1) ? "infravision" : "",
                        /* add comma if telep and infrav */
                        ((how & 3) > 2) ? ", " : "",
                        (how & 2) ? "telepathy" : "",
                        /* add comma if detect and (infrav or telep or both) */
                        ((how & 7) > 4) ? ", " : "",
                        (how & 4) ? "monster detection" : "");
        }
    } else if (u.uswallow) {
        /* when swallowed, we're only called for spots adjacent to hero,
           and blindness doesn't prevent hero from feeling what holds him */
        Sprintf(buf, "interior of %s", a_monnam(u.ustuck));
        pm = u.ustuck->data;
    } else if (glyph_is_monster(glyph)) {
        bhitpos.x = x;
        bhitpos.y = y;
        if ((mtmp = m_at(x, y)) != 0) {
            look_at_monster(buf, monbuf, mtmp, x, y);
            pm = mtmp->data;
        } else if (Hallucination) {
            /* 'monster' must actually be a statue */
            Strcpy(buf, rndmonnam());
        }
    } else if (glyph_is_object(glyph)) {
        look_at_object(buf, x, y, glyph); /* fill in buf[] */
    } else if (glyph_is_trap(glyph)) {
        int tnum = what_trap(glyph_to_trap(glyph));

        /* Trap detection displays a bear trap at locations having
         * a trapped door or trapped container or both.
         * TODO: we should create actual trap types for doors and
         * chests so that they can have their own glyphs and tiles.
         */
        if (trapped_chest_at(tnum, x, y)) {
            Strcpy(buf, "trapped chest"); /* might actually be a large box */
        } else if (trapped_door_at(tnum, x, y)) {
            Strcpy(buf, "trapped door"); /* not "trap door"... */
        } else {
            Strcpy(buf, defsyms[trap_to_defsym(tnum)].explanation);
        }
    } else if (glyph_is_warning(glyph)) {
        int warnindx = glyph_to_warning(glyph);

        Strcpy(buf, def_warnsyms[warnindx].explanation);
    } else if(!glyph_is_cmap(glyph)) {
        Strcpy(buf, "unexplored area");
    } else switch(glyph_to_cmap(glyph)) {
        case S_altar:
            if(!In_endgame(&u.uz))
                Sprintf(buf, "%s altar",
                        align_str(Amask2align(levl[x][y].altarmask & ~AM_SHRINE)));
            else Sprintf(buf, "aligned altar");
            break;
        case S_ndoor:
            if (is_drawbridge_wall(x, y) >= 0)
                Strcpy(buf, "open drawbridge portcullis");
            else if ((levl[x][y].doormask & ~D_TRAPPED) == D_BROKEN)
                Strcpy(buf, "broken door");
            else
                Strcpy(buf, "doorway");
            break;
        case S_cloud:
            Strcpy(buf, Is_airlevel(&u.uz) ? "cloudy area" : "fog/vapor cloud");
            break;
        case S_icewall:
            Strcat(buf, "ice wall");
            break;
        case S_stone:
            if (!levl[x][y].seenv) {
                Strcpy(buf, "unexplored");
                break;
            } else if (Underwater && !Is_waterlevel(&u.uz)) {
                /* "unknown" == previously mapped but not visible when
                   submerged; better terminology appreciated... */
                Strcpy(buf, (distu(x, y) <= 2) ? "land" : "unknown");
                break;
            } else if (levl[x][y].typ == STONE || levl[x][y].typ == SCORR) {
                Strcpy(buf, "stone");
                break;
            }
            /* fall through */

        default:
            Strcpy(buf, defsyms[glyph_to_cmap(glyph)].explanation);
            break;
        }

    return ((pm && !Hallucination) ? pm : (struct permonst *) 0);
}

extern const int monstr[];

static const char*
attack_type(int aatyp)
{
    switch (aatyp) {
        case AT_ANY:  return "fake"; break;             /* fake attack; dmgtype_fromattack wildcard */
        case AT_NONE: return "passive"; break;          /* passive monster (ex. acid blob) */
        case AT_CLAW: return "claw"; break;             /* claw (punch, hit, etc.) */
        case AT_BITE: return "bite"; break;             /* bite */
        case AT_KICK: return "kick"; break;             /* kick */
        case AT_BUTT: return "butt"; break;             /* head butt (ex. a unicorn) */
        case AT_TUCH: return "touch"; break;            /* touches */
        case AT_STNG: return "sting"; break;            /* sting */
        case AT_HUGS: return "bearhug"; break;          /* crushing bearhug */
        case AT_SPIT: return "spit"; break;             /* spits substance - ranged */
        case AT_ENGL: return "engulf"; break;           /* engulf (swallow or by a cloud) */
        case AT_BREA: return "breath"; break;           /* breath - ranged */
        case AT_EXPL: return "explode"; break;          /* explodes - proximity */
        case AT_BOOM: return "explode on death"; break; /* explodes when killed */
        case AT_GAZE: return "gaze"; break;             /* gaze - ranged */
        case AT_TENT: return "tentacle"; break;         /* tentacles */
        case AT_SCRE: return "scream"; break;           /* scream - sonic attack */
        case AT_WEAP: return "weapon"; break;           /* uses weapon */
        case AT_MAGC: return "spellcast"; break;        /* uses magic spell(s) */
        default: impossible("attack_type: unknown attack type %d", aatyp);
    }
    return "(unknown)";
}

static const char*
damage_type(int adtyp)
{
    switch (adtyp) {
        case AD_ANY:  return "fake"; break;                /* fake damage; attacktype_fordmg wildcard */
        case AD_PHYS: return "physical"; break;            /* ordinary physical */
        case AD_MAGM: return "magic missile"; break;       /* magic missiles */
        case AD_FIRE: return "fire"; break;                /* fire damage */
        case AD_COLD: return "cold"; break;                /* frost damage */
        case AD_SLEE: return "sleep"; break;               /* sleep ray */
        case AD_DISN: return "disintegration"; break;      /* disintegration (death ray) */
        case AD_ELEC: return "shock"; break;               /* shock damage */
        case AD_DRST: return "strength poison"; break;     /* drains str (poison) */
        case AD_LAVA: return "lava"; break;                /* a jet of molten lava */
        case AD_ACID: return "acid"; break;                /* acid damage */
        case AD_SPC1: return NULL; break;                  /* for extension of buzz() */
        case AD_SPC2: return NULL; break;                  /* for extension of buzz() */
        case AD_BLND: return "blind"; break;               /* blinds (yellow light) */
        case AD_STUN: return "stun"; break;                /* stuns */
        case AD_SLOW: return "slow"; break;                /* slows */
        case AD_PLYS: return "paralyze"; break;            /* paralyses */
        case AD_DRLI: return "level drain"; break;         /* drains life levels (Vampire) */
        case AD_DREN: return "energy drain"; break;        /* drains magic energy */
        case AD_LEGS: return "wound leg"; break;           /* damages legs (xan) */
        case AD_STON: return "petrify"; break;             /* petrifies (Medusa, cockatrice) */
        case AD_STCK: return "sticky"; break;              /* sticks to you (mimic) */
        case AD_SGLD: return "steal gold"; break;          /* steals gold (leppie) */
        case AD_SITM: return "steal item"; break;          /* steals item (nymphs) */
        case AD_SEDU: return "charm"; break;               /* seduces & steals multiple items */
        case AD_TLPT: return "teleport"; break;            /* teleports you (Quantum Mech.) */
        case AD_RUST: return "rust"; break;                /* rusts armour (Rust Monster)*/
        case AD_CONF: return "confuse"; break;             /* confuses (Umber Hulk) */
        case AD_DGST: return "digest"; break;              /* digests opponent (trapper, etc.) */
        case AD_HEAL: return "heal"; break;                /* heals opponent's wounds (nurse) */
        case AD_WRAP: return "drown"; break;               /* special "stick" for eels */
        case AD_WERE: return "lycanthropy"; break;         /* confers lycanthropy */
        case AD_DRDX: return "dexterity poison"; break;    /* drains dexterity (quasit) */
        case AD_DRCO: return "constitution poison"; break; /* drains constitution */
        case AD_DRIN: return "eat brains"; break;          /* drains intelligence (mind flayer) */
        case AD_DISE: return "disease"; break;             /* confers diseases */
        case AD_DCAY: return "decay"; break;               /* decays organics (brown Pudding) */
        case AD_SSEX: return "seduce"; break;              /* Succubus seduction (extended) */
        case AD_HALU: return "hallucination"; break;       /* causes hallucination */
        case AD_DETH: return "Death special"; break;       /* for Death only */
        case AD_PEST: return "Pestilence special"; break;  /* for Pestilence only */
        case AD_FAMN: return "Famine special"; break;      /* for Famine only */
        case AD_SLIM: return "slime"; break;               /* turns you into green slime */
        case AD_ENCH: return "disenchant"; break;          /* remove enchantment (disenchanter) */
        case AD_CORR: return "corrode"; break;             /* corrode armor (black pudding) */
        case AD_LUCK: return "steal intrinsic"; break;     /* drain luck (evil eye) */
        case AD_FREZ: return "freeze"; break;              /* freezing attack (blue slime) */
        case AD_HEAD: return "decapitate"; break;          /* decapitate (vorpal jabberwock) */
        case AD_PUNI: return "Punisher speical"; break;    /* punisher spells */
        case AD_LVLT: return "level teleport"; break;      /* level teleport (weeping angel) */
        case AD_BLNK: return "mental invasion"; break;     /* mental invasion (weeping angel) */
        case AD_SPOR: return "generate spore"; break;      /* generate spore */
        case AD_CLRC: return "clerical"; break;            /* random clerical spell */
        case AD_SPEL: return "arcane"; break;              /* random magic spell */
        case AD_RBRE: return "random breath"; break;       /* random breath weapon */
        case AD_SAMU: return "steal Amulet"; break;        /* hits, may steal Amulet (Wizard) */
        case AD_CURS: return "random curse"; break;        /* random curse (ex. gremlin) */
        default: impossible("damage_type: unknown damage type %d", adtyp);
    }
    return "(unknown)";
}

/* Add some information to an encyclopedia window which is printing information
 * about a monster. */
static void
add_mon_info(winid datawin, struct permonst * pm)
{
    char buf[BUFSZ];
    char buf2[BUFSZ];
    int gen = pm->geno;
    int freq = (gen & G_FREQ);
    boolean uniq = !!(gen & G_UNIQ);
    boolean hell = !!(gen & G_HELL);
    boolean nohell = !!(gen & G_NOHELL);
    boolean sheol = !!(gen & G_SHEOL);
    boolean nosheol = !!(gen & G_NOSHEOL);
    boolean identified_monster = is_dragon_identified((pm));

#define ADDRESIST(condition, str)                       \
    if (condition) {                                    \
        if (*buf) {                                     \
            Strcat(buf, ", ");                          \
        }                                               \
        Strcat(buf, str);                               \
    }
#define ADDMR(field, res, str)                          \
    if (field & (res)) {                                \
        if (*buf) {                                     \
            Strcat(buf, ", ");                          \
        }                                               \
        Strcat(buf, str);                               \
    }
#define APPENDC(cond, str)                              \
    if (cond) {                                         \
        if (*buf) {                                     \
            Strcat(buf, ", ");                          \
        }                                               \
        Strcat(buf, str);                               \
    }
#define MONPUTSTR(str) putstr(datawin, ATR_NONE, str)

    /* differentiate the two forms of werecreatures */
    Strcpy(buf2, "");
    if (is_were(pm)) {
        Sprintf(buf2, " (%s form)", pm->mlet == S_HUMAN ? "human" : "animal");
    }

    Snprintf(buf, BUFSZ, "Monster lookup for \"%s\"%s:", pm->mname, buf2);
    putstr(datawin, ATR_BOLD, buf);
    MONPUTSTR("");

    /* Misc */
    Sprintf(buf, "Difficulty %d, speed %d, base level %d, base AC %d, magic saving throw %d, weight %d.",
            monstr[monsndx(pm)], pm->mmove, pm->mlevel, pm->ac, pm->mr, pm->cwt);
    MONPUTSTR(buf);

    /* Generation */
    if (uniq) {
        Strcpy(buf, "Unique.");
    } else if (freq == 0) {
        Strcpy(buf, "Not randomly generated.");
    } else {
        Sprintf(buf, "Normally %s%s, %s.",
                hell ? "only appears in Gehennom" :
                nohell ? "only appears outside Gehennom" :
                sheol ? "only appears in Sheol" :
                nosheol ? "only appears outside Sheol" :
                "appears in any branch",
                (gen & G_SGROUP) ? " in groups" :
                (gen & G_LGROUP) ? " in large groups" : "",
                freq >= 5 ? "very common" :
                freq == 4 ? "common" :
                freq == 3 ? "slightly rare" :
                freq == 2 ? "rare" : "very rare");
        MONPUTSTR(buf);
    }

    /* Resistances */
    buf[0] = '\0';
    ADDRESIST(pm_resistance(pm, MR_FIRE), "fire");
    ADDRESIST(pm_resistance(pm, MR_COLD), "cold");
    ADDRESIST(pm_resistance(pm, MR_SLEEP), "sleep");
    ADDRESIST(pm_resistance(pm, MR_DISINT), "disintegration");
    ADDRESIST(pm_resistance(pm, MR_ELEC), "shock");
    ADDRESIST(pm_resistance(pm, MR_POISON), "poison");
    ADDRESIST(pm_resistance(pm, MR_ACID), "acid");
    ADDRESIST(pm_resistance(pm, MR_STONE), "petrification");
    ADDRESIST(resists_drain(pm), "life-drain");
    /* ADDRESIST(SICK_RES, "sickness"); */
    ADDRESIST(resists_mgc(pm), "magic");
    if (identified_monster) {
        if (*buf) {
            Snprintf(buf2, BUFSZ, "Resists %s.", buf);
            MONPUTSTR(buf2);
        } else {
            MONPUTSTR("Has no resistances.");
        }
    }

    /* Corpse conveyances */
    buf[0] = '\0';
    APPENDC(intrinsic_possible(FIRE_RES, pm), "fire");
    APPENDC(intrinsic_possible(COLD_RES, pm), "cold");
    APPENDC(intrinsic_possible(SHOCK_RES, pm), "shock");
    APPENDC(intrinsic_possible(SLEEP_RES, pm), "sleep");
    APPENDC(intrinsic_possible(POISON_RES, pm), "poison");
    APPENDC(intrinsic_possible(DISINT_RES, pm), "disintegration");
    /* acid and stone resistance aren't currently conveyable */
    if (identified_monster) {
        if (*buf) {
            Strcat(buf, " resistance");
        }
    }
    APPENDC(intrinsic_possible(TELEPORT, pm), "teleportation");
    APPENDC(intrinsic_possible(TELEPORT_CONTROL, pm), "teleport control");
    APPENDC(intrinsic_possible(TELEPAT, pm), "telepathy");
    //APPENDC(intrinsic_possible(INTRINSIC_GAIN_STR, pm), "strength");
    //APPENDC(intrinsic_possible(INTRINSIC_GAIN_EN, pm), "magic energy");
    /* There are a bunch of things that happen in cpostfx (levels for wraiths,
     * stunning for bats...) but only count the ones that actually behave like
     * permanent intrinsic gains.
     * If you find yourself listing multiple things here for the same effect,
     * that may indicate the property should be added to psuedo_intrinsics. */
    APPENDC(pm == &mons[PM_QUANTUM_MECHANIC], "speed or slowness");
    APPENDC(pm == &mons[PM_MIND_FLAYER] || pm == &mons[PM_MASTER_MIND_FLAYER], "intelligence");
    if (identified_monster) {
        if (is_were(pm)) {
            /* Weres need a bit of special handling, since 1) you always get
            * lycanthropy so "may convey" could imply the player might not contract
            * it; 2) the animal forms are flagged as G_NOCORPSE, but still have a
            * meaningless listed corpse nutrition value which shouldn't print. */
            if (pm->mlet == S_HUMAN) {
                Sprintf(buf2, "Provides %d nutrition when eaten.", pm->cnutrit);
                MONPUTSTR(buf2);
            }
            MONPUTSTR("Corpse conveys lycanthropy.");
        } else if (!(gen & G_NOCORPSE)) {
            Sprintf(buf2, "Provides %d nutrition when eaten.", pm->cnutrit);
            MONPUTSTR(buf2);
            if (*buf) {
                Snprintf(buf2, BUFSZ, "Corpse may convey %s.", buf);
                MONPUTSTR(buf2);
            } else {
                MONPUTSTR("Corpse conveys no intrinsics.");
            }
        } else {
            MONPUTSTR("Leaves no corpse.");
        }
    }

    /* Flag descriptions */
    buf[0] = '\0';
    APPENDC(is_male(pm), "male");
    APPENDC(pm->msize == MZ_TINY, "tiny");
    APPENDC(pm->msize == MZ_SMALL, "small");
    APPENDC(pm->msize == MZ_LARGE, "large");
    APPENDC(pm->msize == MZ_HUGE, "huge");
    APPENDC(pm->msize == MZ_GIGANTIC, "gigantic");
    if (!(*buf)) {
        /* for nonstandard sizes */
        APPENDC(verysmall(pm), "small");
        APPENDC(bigmonst(pm), "big");
    }

    /* inherent characteristics: "Monster is X." */
    APPENDC(!(gen & G_GENO), "ungenocideable");
    APPENDC(breathless(pm), "breathless");
    if (!breathless(pm)) {
        APPENDC(amphibious(pm), "amphibious");
    }
    APPENDC(amorphous(pm), "amorphous");
    APPENDC(noncorporeal(pm), "incorporeal");
    if (!noncorporeal(pm)) {
        APPENDC(unsolid(pm), "unsolid");
    }
    APPENDC(acidic(pm), "acidic");
    APPENDC(poisonous(pm), "poisonous");
    APPENDC(regenerates(pm), "regenerating");
    APPENDC(is_reviver(pm), "reviving");
    APPENDC(is_floater(pm), "floating");
    APPENDC(pm_invisible(pm), "invisible");
    APPENDC(is_undead(pm), "undead");
    if (!is_undead(pm)) {
        APPENDC(nonliving(pm), "nonliving");
    }
    if (*buf) {
        if (identified_monster) {
            Snprintf(buf2, BUFSZ, "Is %s.", buf);
            MONPUTSTR(buf2);
        }
        buf[0] = '\0';
    }

    /* inherent abilities: "Monster can X." */
    APPENDC(hides_under(pm), "hide under objects");
    APPENDC(pm->mlet == S_MIMIC, "mimic objects and terrain");
    APPENDC(is_hider(pm) && !(pm->mlet == S_MIMIC), "hide on the ceiling");
    APPENDC(is_swimmer(pm), "swim");
    if (!is_floater(pm)) {
        APPENDC(is_flyer(pm), "fly");
    }
    APPENDC(passes_walls(pm), "phase through walls");
    APPENDC(can_teleport(pm), "teleport");
    APPENDC(is_clinger(pm), "cling to the ceiling");
    APPENDC(needspick(pm), "mine");
    if (!needspick(pm)) {
        APPENDC(tunnels(pm), "dig");
    }
    if (*buf) {
        Snprintf(buf2, BUFSZ, "Can %s.", buf);
        MONPUTSTR(buf2);
        buf[0] = '\0';
    }

    /* Full-line remarks. */
    if (touch_petrifies(pm)) {
        MONPUTSTR("Petrifies by touch.");
    }
    if (perceives(pm)) {
        MONPUTSTR("Can see invisible.");
    }
    if (control_teleport(pm)) {
        MONPUTSTR("Has teleport control.");
    }
    if (your_race(pm)) {
        MONPUTSTR("Is the same race as you.");
    }
    if (!(gen & G_NOCORPSE)) {
        if (vegan(pm)) {
            MONPUTSTR("May be eaten by vegans.");
        } else if (vegetarian(pm)) {
            MONPUTSTR("May be eaten by vegetarians.");
        }
        if (has_blood(pm)) {
            MONPUTSTR("May be drunk by vampiric monsters.");
        }
    }
    snprintf(buf, BUFSZ, "Is %sa valid polymorph form.", polyok(pm) ? "" : "not ");
    MONPUTSTR(buf);

    /* Attacks */
    buf[0] = buf2[0] = '\0';
    int i;
    for (i = 0; i < 6; i++) {
        char dicebuf[20]; /* should be a safe limit */
        struct attack * attk = &(pm->mattk[i]);
        if (attk->damn) {
            Sprintf(dicebuf, "%dd%d", attk->damn, attk->damd);
        } else if (attk->damd) {
            Sprintf(dicebuf, "(level+1)d%d", attk->damd);
        } else {
            if (!attk->aatyp && !attk->adtyp) {
                /* no attack in this slot */
                continue;
            } else {
                /* real attack, but 0d0 damage */
                dicebuf[0] = '\0';
            }
        }

        if (identified_monster || i != 0) {
            Sprintf(buf2, "%s%s%s %s", dicebuf, ((*dicebuf) ? " " : ""),
                    attack_type(attk->aatyp), damage_type(attk->adtyp));
        } else {
            Strcpy(buf2, "unknown");
        }
        APPENDC(TRUE, buf2);
    }
    if (*buf) {
        Snprintf(buf2, BUFSZ, "Attacks: %s", buf);
        MONPUTSTR(buf2);
    } else {
        MONPUTSTR("Has no attacks.");
    }
}

/* Add some information to an encyclopedia window which is printing information
 * about an object. */
static void
add_obj_info(winid datawin, struct obj *obj, short otyp)
{
    struct objclass oc = objects[otyp];
    char olet = oc.oc_class;
    char buf[BUFSZ];
    char buf2[BUFSZ];
    const char* dir = (oc.oc_dir == NODIR ? "Non-directional" :
                      (oc.oc_dir == IMMEDIATE ? "Beam" : "Ray"));

    struct obj dummy = { 0 };
    dummy.otyp = otyp;
    dummy.oclass = oc.oc_class;
    boolean identified = (otyp != STRANGE_OBJECT && oc.oc_name_known);

    if (obj && otyp == STRANGE_OBJECT) {
        oc = objects[obj->otyp];
        olet = oc.oc_class;
    }

#define OBJPUTSTR(str) putstr(datawin, ATR_NONE, str)
#define ADDCLASSPROP(cond, str)            \
    if (cond) {                             \
        if (*buf) { Strcat(buf, ", "); }    \
        Strcat(buf, str);                   \
    }

    if (obj && otyp == STRANGE_OBJECT) {
        Sprintf(buf, "Object lookup for \"%s\":", xname(obj));
    } else {
        Sprintf(buf, "Object lookup for \"%s\":", simple_typename(otyp));
    }
    putstr(datawin, ATR_BOLD, buf);
    OBJPUTSTR("");

    /* Object classes currently with no special messages here: amulets. */
    boolean weptool = (olet == TOOL_CLASS && oc.oc_skill != P_NONE);
    if (olet == WEAPON_CLASS || weptool) {
        const int skill = oc.oc_skill;
        if (skill >= 0) {
            Sprintf(buf, "%s-handed weapon%s using the %s skill.",
                    (oc.oc_bimanual ? "Two" : "Single"),
                    (weptool ? "-tool" : ""),
                    skill_name(skill));
        } else if (skill <= -P_BOW && oc.oc_skill >= -P_CROSSBOW) {
            /* Minor assumption: the skill name will be the same as the launcher
             * itself. Currently this is only bow and crossbow. */
            Sprintf(buf, "Ammunition meant to be fired from a %s.", skill_name(-skill));
        } else {
            Sprintf(buf, "Thrown missile using the %s skill.", skill_name(-skill));
        }
        OBJPUTSTR(buf);

        const char* dmgtyp = "blunt";
        if (oc.oc_dir & PIERCE) {
            dmgtyp = "piercing";
            if (oc.oc_dir & SLASH) {
                dmgtyp = "piercing/slashing";
            }
        } else if (oc.oc_dir & SLASH) {
            dmgtyp = "slashing";
        }
        Sprintf(buf, "Deals %s damage.", dmgtyp);
        OBJPUTSTR(buf);

        struct damage_info_t damage_info = dmgval_info(&dummy);
        Sprintf(buf,
                "Damage: 1d%d%s versus small and 1d%d%s versus large monsters.",
                damage_info.damage_small, damage_info.bonus_small,
                damage_info.damage_large, damage_info.bonus_large);
        OBJPUTSTR(buf);

        if (damage_info.blessed_damage) { OBJPUTSTR(damage_info.blessed_damage); }
        if (damage_info.axe_damage)     { OBJPUTSTR(damage_info.axe_damage); }
        if (damage_info.silver_damage)  { OBJPUTSTR(damage_info.silver_damage); }
        if (damage_info.light_damage)   { OBJPUTSTR(damage_info.light_damage); }

        Sprintf(buf, "Has a %s%d %s to hit.",
                (oc.oc_hitbon >= 0 ? "+" : ""),
                 oc.oc_hitbon,
                (oc.oc_hitbon >= 0 ? "bonus" : "penalty"));
        OBJPUTSTR(buf);
    }

    if (olet == AMULET_CLASS) {
        /* nothing special */
        OBJPUTSTR("Amulet.");
    }

    if (olet == ARMOR_CLASS) {
        /* Indexes here correspond to ARM_SHIELD, etc; not the W_* masks.
         * Expects ARM_SUIT = 0, all the way up to ARM_SHIRT = 6. */
        const char* armorslots[] = {
            "torso", "shield", "helm", "gloves", "boots", "cloak", "shirt"
        };
        Sprintf(buf, "%s, worn in the %s slot.",
                (oc.oc_bulky ? "Bulky armor" : "Armor"),
                armorslots[oc.oc_armcat]);

        OBJPUTSTR(buf);
        if (identified) {
            Sprintf(buf, "Base AC %d, magic cancellation %d.", oc.a_ac, oc.a_can);
        } else {
            Sprintf(buf, "Base AC %d.", oc.a_ac);
        }

        OBJPUTSTR(buf);
        Sprintf(buf, "Takes %d turn%s to put on or remove.", oc.oc_delay, (oc.oc_delay == 1 ? "" : "s"));
    }

    if (olet == FOOD_CLASS) {
        if (otyp == TIN || otyp == CORPSE) {
            if (obj && (obj->otyp == CORPSE)) {
                Sprintf(buf, "Comestible providing %d nutrition at the most.", mons[obj->corpsenm].cnutrit);
                OBJPUTSTR(buf);
            } else {
                OBJPUTSTR("Comestible providing varied nutrition.");
            }
            OBJPUTSTR("Takes various amounts of turns to eat.");
            if (obj) {
                if (vegan(&mons[obj->corpsenm])) {
                    OBJPUTSTR("Is vegan.");
                } else if (vegetarian(&mons[obj->corpsenm])) {
                    OBJPUTSTR("Is vegetarian but not vegan.");
                } else {
                    OBJPUTSTR("Is not vegetarian.");
                }
            } else {
                OBJPUTSTR("May or may not be vegetarian.");
            }
        } else {
            Sprintf(buf, "Comestible providing %d nutrition.", oc.oc_nutrition);
            OBJPUTSTR(buf);
            Sprintf(buf, "Takes %d turn%s to eat.", oc.oc_delay, (oc.oc_delay == 1 ? "" : "s"));
            OBJPUTSTR(buf);
            /* TODO: put special-case VEGGY foods in a list which can be
             * referenced by doeat(), so there's no second source for this. */
            if (oc.oc_material == FLESH && otyp != EGG) {
                OBJPUTSTR("Is not vegetarian.");
            } else {
                /* is either VEGGY food or egg */
                switch (otyp) {
                case PANCAKE:
                case FORTUNE_COOKIE:
                case EGG:
                case CREAM_PIE:
                case CANDY_BAR:
                case LUMP_OF_ROYAL_JELLY:
                    OBJPUTSTR("Is vegetarian but not vegan.");
                    break;
                default:
                    OBJPUTSTR("Is vegan.");
                }
            }
        }
    }

    if (olet == POTION_CLASS) {
        /* nothing special */
        OBJPUTSTR("Potion.");

        if (obj && obj->dknown) {
            const char *potion_desc = OBJ_DESCR(objects[obj->otyp]);
            if (is_colorless_mix_potion(obj)) {
                OBJPUTSTR("Is a colorless potion.");
            } else if (!strcmp(potion_desc, "black")) {
                OBJPUTSTR("Turns light colored potions into their dark colored variants.");
            } else if (!strcmp(potion_desc, "white")) {
                OBJPUTSTR("Turns dark colored potions into their light colored variants.");
            } else if (!is_colorless_mix_potion(obj)) {
                Sprintf(buf, "Is a %s %s colored potion.",
                        is_dark_mix_color(obj) ? "dark" : "light", get_base_mix_color(obj));
                OBJPUTSTR(buf);
            }
        }
    }

    if (olet == SCROLL_CLASS) {
        /* nothing special (ink is covered below) */
        OBJPUTSTR("Scroll.");
    }

    if (olet == SPBOOK_CLASS) {
        if (otyp == SPE_BLANK_PAPER || otyp == SPE_BOOK_OF_THE_DEAD) {
            OBJPUTSTR("Spellbook.");
        } else if (otyp == STRANGE_OBJECT) {
            OBJPUTSTR("Spellbook.");
        } else {
            Sprintf(buf, "Level %d spellbook, in the %s school. %s spell.",
                    oc.oc_level, spelltypemnemonic(oc.oc_skill), dir);
            OBJPUTSTR(buf);
            Sprintf(buf, "Takes %d actions to read.", oc.oc_delay);
            OBJPUTSTR(buf);
        }
    }
    if (olet == WAND_CLASS) {
        if (otyp == STRANGE_OBJECT) {
            Strcpy(buf, "Wand.");
        } else {
            Sprintf(buf, "%s wand.", dir);
        }
        OBJPUTSTR(buf);
    }

    if (olet == RING_CLASS) {
        OBJPUTSTR(identified && oc.oc_charged ? "Chargeable ring." : "Ring.");
        /* see material comment below; only show toughness status if this
         * particular ring is already identified... */
        if (oc.oc_tough && oc.oc_name_known) {
            OBJPUTSTR("Is made of a hard material.");
        }
    }

    if (identified && olet == GEM_CLASS) {
        if (oc.oc_material == MINERAL) {
            OBJPUTSTR("Type of stone.");
        } else if (oc.oc_material == GLASS) {
            OBJPUTSTR("Piece of colored glass.");
        } else {
            OBJPUTSTR("Precious gem.");
        }
        /* can do unconditionally, these aren't randomized */
        if (oc.oc_tough) {
            OBJPUTSTR("Is made of a hard material.");
        }
    }

    if (olet == TOOL_CLASS && !weptool) {
        const char* subclass = "tool";
        switch (otyp) {
        case LARGE_BOX:
        case CHEST:
        case ICE_BOX:
        case SACK:
        case OILSKIN_SACK:
        case BAG_OF_HOLDING:
            subclass = "container";
            break;

        case SKELETON_KEY:
        case LOCK_PICK:
        case CREDIT_CARD:
            subclass = "unlocking tool";
            break;

        case TALLOW_CANDLE:
        case WAX_CANDLE:
        case BRASS_LANTERN:
        case OIL_LAMP:
        case MAGIC_LAMP:
            subclass = "light source";
            break;

        case LAND_MINE:
        case BEARTRAP:
            subclass = "trap which can be set";
            break;

        case TIN_WHISTLE:
        case MAGIC_WHISTLE:
        case BELL:
        case LEATHER_DRUM:
        case DRUM_OF_EARTHQUAKE:
            subclass = "atonal instrument";
            break;

        case BUGLE:
        case MAGIC_FLUTE:
        case WOODEN_FLUTE:
        case TOOLED_HORN:
        case FIRE_HORN:
        case FROST_HORN:
        case WOODEN_HARP:
        case MAGIC_HARP:
            subclass = "tonal instrument";
            break;
        }
        Sprintf(buf, "%s%s.", (identified && oc.oc_charged ? "chargeable " : ""), subclass);
        /* capitalize first letter of buf */
        buf[0] -= ('a' - 'A');
        OBJPUTSTR(buf);
    }

    /* cost, wt should go next */
    buf[0] = '\0';
    int obj_weight = obj ? weight(obj) : oc.oc_weight;
    if (otyp != STRANGE_OBJECT) {
        Sprintf(buf, "Base cost %d, weighs %d aum.", oc.oc_cost, obj_weight);
    } else {
        int i, base_cost = oc.oc_cost;
        for (i = 0; i < NUM_OBJECTS; i++) {
            if (objects[i].oc_class == olet) {
                if (olet == SPBOOK_CLASS && i == SPE_BOOK_OF_THE_DEAD) {
                    continue;
                }
                if (objects[i].oc_cost != base_cost) {
                    base_cost = -1;
                }
            }
        }

        if (base_cost > 0) {
            Sprintf(buf, "Base cost %d, weighs %d aum.", oc.oc_cost, obj_weight);
        } else {
            Sprintf(buf, "Weighs %d aum.", obj_weight);
        }
    }
    OBJPUTSTR(buf);

    if (obj && identified) {
        Sprintf(buf, "Unidentified description is \"%s\".", cxname_unidentified(obj));
        OBJPUTSTR(buf);
    }

    /* Scrolls or spellbooks: ink cost */
    if (otyp != STRANGE_OBJECT) {
        if (olet == SCROLL_CLASS || olet == SPBOOK_CLASS) {
            if (otyp == SCR_BLANK_PAPER || otyp == SPE_BLANK_PAPER) {
                OBJPUTSTR("Can be written on.");
            } else if (otyp != SPE_BOOK_OF_THE_DEAD) {
                Sprintf(buf, "Takes %d to %d ink to write.", ink_cost(&dummy)/2, ink_cost(&dummy)-1);
                OBJPUTSTR(buf);
            }
        }
    }

    /* power conferred */
    extern const struct propname {
        int prop_num;
        const char* prop_name;
    } propertynames[]; /* located in timeout.c */
    if (identified && oc.oc_oprop) {
        int i;
        for (i = 0; propertynames[i].prop_name; ++i) {
            /* hack for alchemy smocks because everything about alchemy smocks
             * is a hack */
            if (propertynames[i].prop_num == ACID_RES && otyp == ALCHEMY_SMOCK) {
                OBJPUTSTR("Confers acid resistance.");
                continue;
            }
            if (oc.oc_oprop == propertynames[i].prop_num) {
                /* proper grammar */
                const char* confers = "Makes you";
                const char* effect = propertynames[i].prop_name;
                switch (propertynames[i].prop_num) {
                    /* special overrides because prop_name is bad */
                    case STRANGLED:
                        effect = "choke";
                        break;
                    case LIFESAVED:
                        effect = "life saving";
                        /* fall through */
                    /* for things that don't work with "Makes you" */
                    case GLIB:
                    case WOUNDED_LEGS:
                    case DETECT_MONSTERS:
                    case SEE_INVIS:
                    case HUNGER:
                    case WARNING:
                    /* don't do special warn_of_mon */
                    case SEARCHING:
                    case INFRAVISION:
                    case AGGRAVATE_MONSTER:
                    case CONFLICT:
                    case JUMPING:
                    case TELEPORT_CONTROL:
                    case SWIMMING:
                    case SLOW_DIGESTION:
                    case HALF_SPDAM:
                    case HALF_PHDAM:
                    case REGENERATION:
                    case ENERGY_REGENERATION:
                    case PROTECTION:
                    case PROT_FROM_SHAPE_CHANGERS:
                    case POLYMORPH_CONTROL:
                    case FREE_ACTION:
                    case FIXED_ABIL:
                        confers = "Confers";
                        break;
                    default:
                        break;
                }
                if (strstri(propertynames[i].prop_name, "resistance")) {
                    confers = "Confers";
                }
                Sprintf(buf, "%s %s.", confers, effect);
                OBJPUTSTR(buf);
            }
        }
    }
    if ((otyp == CHROMATIC_DRAGON_SCALES) || (otyp == CHROMATIC_DRAGON_SCALE_MAIL)) {
        OBJPUTSTR("Confers fire, cold, sleep, disintegration, shock, poison, acid, petrification, "
                  "magic resistance.");
    }

    buf[0] = '\0';
    if (otyp != STRANGE_OBJECT) {
        ADDCLASSPROP(oc.oc_magic, "inherently magical");
        ADDCLASSPROP(oc.oc_nowish, "not wishable");
    }
    if (*buf) {
        Sprintf(buf2, "Is %s.", buf);
        OBJPUTSTR(buf2);
    }

    /* Material.
     * Note that we should not show the material of certain objects if they are
     * subject to description shuffling that includes materials. If the player
     * has already discovered this object, though, then it's fine to show the
     * material.
     * Object classes where this may matter: rings, wands. All randomized tools
     * share materials, and all scrolls and potions are the same material. */
    if (!(olet == RING_CLASS || olet == WAND_CLASS) || oc.oc_name_known) {
        /* char array converting materials to strings; if this is ever needed
        * anywhere else it should be externified. Corresponds exactly to the
        * materials defined in objclass.h.
        * This is very similar to materialnm[], but the slight difference is
        * that this is always the noun form whereas materialnm uses adjective
        * forms; most materials have the same noun and adjective forms but two
        * (wood/wooden, vegetable matter/organic) don't */
        const char* mat_str = materialnm[oc.oc_material];
        /* Two exceptions to materialnm, which uses adjectival forms: most of
         * these work fine as nouns but two don't. */
        if (oc.oc_material == WOOD) {
            mat_str = "wood";
        } else if (oc.oc_material == VEGGY) {
            mat_str = "vegetable matter";
        }

        Sprintf(buf, "Made of %s.", mat_str);
        OBJPUTSTR(buf);
    }

    /* TODO: prevent obj lookup from displaying with monster database entry
     * (e.g. scroll of light gives "light" monster database) */

    /* Full-line remarks */
    if (oc.oc_merge) {
        OBJPUTSTR("Merges with identical items.");
    }
    if (oc.oc_unique) {
        OBJPUTSTR("Unique item.");
    }
}

/*
 * Look in the "data" file for more info.  Called if the user typed in the
 * whole name (user_typed_name == TRUE), or we've found a possible match
 * with a character/glyph and flags.help is TRUE.
 *
 * NOTE: when (user_typed_name == FALSE), inp is considered read-only and
 *   must not be changed directly, e.g. via lcase(). We want to force
 *   lcase() for data.base lookup so that we can have a clean key.
 *   Therefore, we create a copy of inp _just_ for data.base lookup.
 */
void
checkfile(obj, inp, pm, user_typed_name, without_asking, supplemental_name)
struct obj *obj;
char *inp;
struct permonst *pm;
boolean user_typed_name, without_asking;
char *supplemental_name;
{
    dlb *fp;
    char newstr[BUFSZ];
    char *ep, *dbase_str;
    int chk_skip;
    boolean found_in_file = FALSE, skipping_entry = FALSE;
    boolean lookat_mon = (pm != (struct permonst *) 0);

    fp = dlb_fopen_area(NH_DATAAREA, DATAFILE, "r");
    if (!fp) {
        pline("Cannot open 'data' file!");
        return;
    }
    /* If someone passed us garbage, prevent fault. */
    if (!inp || strlen(inp) > (BUFSZ - 1)) {
        impossible("bad do_look buffer passed (%s)!", !inp ? "null" : "too long");
        (void) dlb_fclose(fp);
        return;
    }

    /* To prevent the need for entries in data.base like *ngel to account
     * for Angel and angel, make the lookup string the same for both
     * user_typed_name and picked name.
     */
    if (pm != (struct permonst *) 0 && !user_typed_name)
        dbase_str = strcpy(newstr, pm->mname);
    else dbase_str = strcpy(newstr, inp);
    (void) lcase(dbase_str);

    if (!strncmp(dbase_str, "interior of ", 12)) {
        dbase_str += 12;
    }
    if (!strncmp(dbase_str, "a ", 2)) {
        dbase_str += 2;
    } else if (!strncmp(dbase_str, "an ", 3)) {
        dbase_str += 3;
    } else if (!strncmp(dbase_str, "the ", 4)) {
        dbase_str += 4;
    } else if (!strncmp(dbase_str, "some ", 5)) {
        dbase_str += 5;
    } else if (digit(*dbase_str)) {
        /* remove count prefix ("2 ya") which can come from looking at map */
        while (digit(*dbase_str)) {
            ++dbase_str;
        }
        if (*dbase_str == ' ') {
            ++dbase_str;
        }
    }
    if (!strncmp(dbase_str, "pair of ", 8)) {
        dbase_str += 8;
    }
    if (!strncmp(dbase_str, "tame ", 5)) {
        dbase_str += 5;
    } else if (!strncmp(dbase_str, "peaceful ", 9)) {
        dbase_str += 9;
    }
    if (!strncmp(dbase_str, "invisible ", 10)) {
        dbase_str += 10;
    }
    if (!strncmp(dbase_str, "saddled ", 8)) {
        dbase_str += 8;
    }
    if (!strncmp(dbase_str, "blessed ", 8)) {
        dbase_str += 8;
    } else if (!strncmp(dbase_str, "uncursed ", 9)) {
        dbase_str += 9;
    } else if (!strncmp(dbase_str, "cursed ", 7)) {
        dbase_str += 7;
    }
    if (!strncmp(dbase_str, "empty ", 6)) {
        dbase_str += 6;
    }
    if (!strncmp(dbase_str, "partly used ", 12)) {
        dbase_str += 12;
    } else if (!strncmp(dbase_str, "partly eaten ", 13)) {
        dbase_str += 13;
    }
    if (!strncmp(dbase_str, "statue of ", 10)) {
        dbase_str[6] = '\0';
    } else if (!strncmp(dbase_str, "figurine of ", 12)) {
        dbase_str[8] = '\0';
    }
    /* remove enchantment ("+0 aklys"); [for 3.6.0 and earlier, this wasn't
       needed because looking at items on the map used xname() rather than
       doname() hence known enchantment was implicitly suppressed] */
    if (*dbase_str && index("+-", dbase_str[0]) && digit(dbase_str[1])) {
        ++dbase_str; /* skip sign */
        while (digit(*dbase_str)) {
            ++dbase_str;
        }
        if (*dbase_str == ' ') {
            ++dbase_str;
        }
    }
    /* "towel", "wet towel", and "moist towel" share one data.base entry;
       for "wet towel", we keep prefix so that the prompt will ask about
       "wet towel"; for "moist towel", we also want to ask about "wet towel".
       (note: strncpy() only terminates output string if the specified
       count is bigger than the length of the substring being copied) */
    if (!strncmp(dbase_str, "moist towel", 11)) {
        (void) strncpy(dbase_str += 2, "wet", 3); /* skip "mo" replace "ist" */
    }

    /* Make sure the name is non-empty. */
    if (*dbase_str) {
        if (!lookup_database_entry(fp, obj, dbase_str, inp, pm,
                    user_typed_name, without_asking, supplemental_name)) {
            impossible("'data' file in wrong format or corrupted");
        }
    } else if (obj) {
        if (!lookup_database_entry(fp, obj, dbase_str, inp, pm,
                    user_typed_name, without_asking, supplemental_name)) {
            impossible("'data' file in wrong format or corrupted");
        }
    }

    (void) dlb_fclose(fp);
}

static boolean
lookup_database_entry(
dlb *fp,
struct obj *obj,
const char* dbase_str,
const char *inp,
struct permonst *pm,
boolean user_typed_name,
boolean without_asking,
char *supplemental_name)
{
    char buf[BUFSZ], givenname[BUFSZ];
    char *ep;
    long txt_offset;
    winid datawin = WIN_ERR;

    long pass1offset = -1L;
    int chk_skip, pass = 1;
    boolean yes_to_moreinfo, found_in_file, pass1found_in_file, skipping_entry;
    char *sp, *ap, *alt = 0; /* alternate description */

    /* adjust the input to remove "named " and "called " */
    if ((ep = strstri(dbase_str, " named ")) != 0) {
        alt = ep + 7;
        if ((ap = strstri(dbase_str, " called ")) != 0 && ap < ep)
            ep = ap; /* "named" is alt but truncate at "called" */
    } else if ((ep = strstri(dbase_str, " called ")) != 0) {
        copynchars(givenname, ep + 8, BUFSZ - 1);
        alt = givenname;
        if (supplemental_name && (sp = strstri(inp, " called ")) != 0)
            copynchars(supplemental_name, sp + 8, BUFSZ - 1);
    } else
        ep = strstri(dbase_str, ", ");
    if (ep && ep > dbase_str)
        *ep = '\0';
    /* remove article from 'alt' name ("a pair of lenses named
        The Eyes of the Overworld" simplified above to "lenses named
        The Eyes of the Overworld", now reduced to "The Eyes of the
        Overworld", skip "The" as with base name processing) */
    if (alt && (!strncmpi(alt, "a ", 2)
                || !strncmpi(alt, "an ", 3)
                || !strncmpi(alt, "the ", 4)))
        alt = index(alt, ' ') + 1;
    /* remove charges or "(lit)" or wizmode "(N aum)" */
    if ((ep = strstri(dbase_str, " (")) != 0 && ep > dbase_str)
        *ep = '\0';
    if (alt && (ap = strstri(alt, " (")) != 0 && ap > alt)
        *ap = '\0';

    /*
        * If the object is named, then the name is the alternate description;
        * otherwise, the result of makesingular() applied to the name is.
        * This isn't strictly optimal, but named objects of interest to the
        * user will usually be found under their name, rather than under
        * their object type, so looking for a singular form is pointless.
        */
    if (!alt)
        alt = makesingular(dbase_str);

    pass1found_in_file = FALSE;
    for (pass = !strcmp(alt, dbase_str) ? 0 : 1; pass >= 0; --pass) {
        found_in_file = skipping_entry = FALSE;
        txt_offset = 0L;
        if (dlb_fseek(fp, txt_offset, SEEK_SET) < 0 ) {
            impossible("can't get to start of 'data' file");
            goto checkfile_done;
        }
        /* skip first record; read second */
        if (!dlb_fgets(buf, BUFSZ, fp) || !dlb_fgets(buf, BUFSZ, fp)) {
            impossible("can't read 'data' file");
            goto checkfile_done;
        } else if (sscanf(buf, "%8lx\n", &txt_offset) < 1 || txt_offset == 0L) {
            return FALSE;
        }

        /* look for the appropriate entry */
        while (dlb_fgets(buf, BUFSZ, fp)) {
            if (*buf == '.')
                break; /* we passed last entry without success */

            if (digit(*buf)) {
                /* a number indicates the end of current entry */
                skipping_entry = FALSE;
            } else if (!skipping_entry) {
                if (!(ep = index(buf, '\n'))) {
                    return FALSE;
                }
                (void) strip_newline((ep > buf) ? ep - 1 : ep);
                /* if we match a key that begins with "~", skip
                    this entry */
                chk_skip = (*buf == '~') ? 1 : 0;
                if ((pass == 0 && pmatch(&buf[chk_skip], dbase_str))
                    || (pass == 1 && alt && pmatch(&buf[chk_skip], alt))) {
                    if (chk_skip) {
                        skipping_entry = TRUE;
                        continue;
                    } else {
                        found_in_file = TRUE;
                        if (pass == 1)
                            pass1found_in_file = TRUE;
                        break;
                    }
                }
            }
        }

        /* database entry should exist, now find where it is */
        long entry_offset, fseekoffset;
        int entry_count;
        int i;
        if (found_in_file) {
            /* skip over other possible matches for the info */
            do {
                if (!dlb_fgets(buf, BUFSZ, fp)) {
                    return FALSE;
                }
            } while (!digit(*buf));

            if (sscanf(buf, "%ld,%d\n", &entry_offset, &entry_count) < 2) {
                return FALSE;
            }
            fseekoffset = (long) txt_offset + entry_offset;
            if (pass == 1)
                pass1offset = fseekoffset;
            else if (fseekoffset == pass1offset)
                goto checkfile_done;
        }

        boolean lookat_mon = (pm != (struct permonst *) 0);
        /* monster lookup: try to parse as a monster */
        if (!lookat_mon) {
            pm = (struct permonst *) 0; /* just to be safe */
            int mndx = name_to_mon(dbase_str);
            if (mndx != NON_PM) {
                pm = &mons[mndx];
            }
        }

        /* object lookup: try to parse as an object */
        int otyp = name_to_otyp(dbase_str);

        /* prompt for more info (if using whatis to navigate the map) */
        yes_to_moreinfo = FALSE;
        if (!user_typed_name && !without_asking) {
            const char *entrytext = pass ? alt : dbase_str;
            char question[QBUFSZ];

            Strcpy(question, "More info about \"");
            /* +2 => length of "\"?" */
            copynchars(eos(question), entrytext,
                        (sizeof question - 1 - (strlen(question) + 2)));
            Strcat(question, "\"?");
            if (yn(question) == 'y') {
                yes_to_moreinfo = TRUE;
            }
        }

        /* finally, put the appropriate information into a window */
        if (user_typed_name || without_asking || yes_to_moreinfo) {
            if (!found_in_file && !pm && (obj == NULL && otyp == STRANGE_OBJECT)) {
                if ((user_typed_name && pass == 0 && !pass1found_in_file) || yes_to_moreinfo) {
                    pline("I don't have any information on those things.");
                }
                /* don't print anything otherwise; we don't want it to e.g.
                    * print a database entry and then print the above message. */
            } else {
                boolean do_obj_lookup = FALSE, do_mon_lookup = FALSE;
                if (pm) {
                    do_mon_lookup = TRUE;
                    if (!lookat_mon && otyp != STRANGE_OBJECT) {
                        /* found matches for both and player is NOT looking
                         * at a monster; ask which they want to see */
                        /* TODO: this would ideally be better generalized so
                         * that the caller could communicate that an object
                         * is being looked at, too */
                        pline("That matches both a monster and an object.");
                        if (yn("Show the monster information?") != 'y') {
                            do_obj_lookup = TRUE;
                            do_mon_lookup = FALSE;
                        }
                    }
                } else if (otyp != STRANGE_OBJECT) {
                    do_obj_lookup = TRUE;
                }

                datawin = create_nhwindow(NHW_MENU);

                /* object lookup info */
                do_obj_lookup = TRUE;
                if (do_obj_lookup) {
                    add_obj_info(datawin, obj, otyp);
                    putstr(datawin, 0, "");
                } else if (obj) {
                    add_obj_info(datawin, obj, otyp);
                    putstr(datawin, 0, "");
                } else if (do_mon_lookup) {
                    /* monster lookup info */
                    /* secondary to object lookup because there are some
                     * monsters whose names are substrings of objects, like
                     * "skeleton" and "skeleton key". */
                    add_mon_info(datawin, pm);
                    putstr(datawin, 0, "");
                }

                /* encyclopedia entry */
                if (found_in_file) {
                    if (dlb_fseek(fp, (long) txt_offset + entry_offset, SEEK_SET) < 0) {
                        pline("? Seek error on 'data' file!");
                        return FALSE;
                    }

                    char titlebuf[BUFSZ];
                    Sprintf(titlebuf, "Encyclopedia entry for \"%s\":", dbase_str);
                    putstr(datawin, ATR_BOLD, titlebuf);
                    putstr(datawin, ATR_NONE, "");

                    for (i = 0; i < entry_count; i++) {
                        /* room for 1-tab or 8-space prefix + BUFSZ-1 + \0 */
                        char tabbuf[BUFSZ + 8], *tp;

                        if (!dlb_fgets(tabbuf, BUFSZ, fp)) {
                            return FALSE;
                        }
                        tp = tabbuf;
                        if (!index(tp, '\n')) {
                            return FALSE;
                        }
                        (void) strip_newline(tp);
                        /* text in this file is indented with one tab but
                            someone modifying it might use spaces instead */
                        if (*tp == '\t') {
                            ++tp;
                        } else if (*tp == ' ') {
                            /* remove up to 8 spaces (we expect 8-column
                                tab stops but user might have them set at
                                something else so we don't require it) */
                            do {
                                ++tp;
                            } while (tp < &tabbuf[8] && *tp == ' ');
                        } else if (*tp) { /* empty lines are ok */
                            return FALSE;
                        }
                        /* if a tab after the leading one is found,
                            convert tabs into spaces; the attributions
                            at the end of quotes typically have them */
                        if (index(tp, '\t') != 0) {
                            (void) tabexpand(tp);
                        }
                        putstr(datawin, 0, tp);
                    }
                }
                display_nhwindow(datawin, FALSE);
                destroy_nhwindow(datawin), datawin = WIN_ERR;
            }
        } else if (user_typed_name && pass == 0 && !pass1found_in_file) {
            pline("I don't have any information on those things.");
        }
    }
checkfile_done:
    if (datawin != WIN_ERR) {
        destroy_nhwindow(datawin);
    }
    return TRUE;
}

/* getpos() return values */
#define LOOK_TRADITIONAL 0   /* '.' -- ask about "more info?" */
#define LOOK_QUICK       1   /* ',' -- skip "more info?" */
#define LOOK_ONCE        2   /* ';' -- skip and stop looping */
#define LOOK_VERBOSE     3   /* ':' -- show more info w/o asking */

int
do_screen_description(cc, looked, sym, out_str, firstmatch, for_supplement)
coord cc;
boolean looked;
glyph_t sym;
char *out_str;
const char **firstmatch;
struct permonst **for_supplement;
{
    static const char mon_interior[] = "the interior of a monster";
    static const char unreconnoitered[] = "unreconnoitered";
    static char look_buf[BUFSZ];
    char prefix[BUFSZ];
    int i, alt_i, j, glyph = NO_GLYPH,
        skipped_venom = 0, found = 0; /* count of matching syms found */
    boolean hit_trap, need_to_look = FALSE,
            submerged = (Underwater && !Is_waterlevel(&u.uz));
    const char *x_str;
    nhsym tmpsym;

    if (looked) {
        int oc;
        unsigned os;

        glyph = glyph_at(cc.x, cc.y);
        /* Convert glyph at selected position to a symbol for use below. */
        (void) mapglyph(glyph, &sym, &oc, &os, cc.x, cc.y, 0);

        Sprintf(prefix, "%s        ", encglyph(glyph));
    } else {
        Sprintf(prefix, "%c        ", sym);
    }

    /*
     * Check all the possibilities, saving all explanations in a buffer.
     * When all have been checked then the string is printed.
     */

    /*
     * Handle restricted vision range (limited to adjacent spots when
     * swallowed or underwater) cases first.
     *
     * 3.6.0 listed anywhere on map, other than self, as "interior
     * of a monster" when swallowed, and non-adjacent water or
     * non-water anywhere as "dark part of a room" when underwater.
     * "unreconnoitered" is an attempt to convey "even if you knew
     * what was there earlier, you don't know what is there in the
     * current circumstance".
     *
     * (Note: 'self' will always be visible when swallowed so we don't
     * need special swallow handling for <ux,uy>.
     * Another note: for '#terrain' without monsters, u.uswallow and
     * submerged will always both be False and skip this code.)
     */
    x_str = 0;
    if (!looked) {
        ; /* skip special handling */
    } else if (((u.uswallow || submerged) && distu(cc.x, cc.y) > 2) ||
               /* detection showing some category, so mostly background */
               (((iflags.terrainmode & (TER_DETECT | TER_MAP)) == TER_DETECT) &&
                (glyph == cmap_to_glyph(S_stone)))) {
        x_str = unreconnoitered;
        need_to_look = FALSE;
    } else if (is_swallow_sym(sym)) {
        x_str = mon_interior;
        need_to_look = TRUE; /* for specific monster type */
    }
    if (x_str) {
        /* we know 'found' is zero here, but guard against some other
           special case being inserted ahead of us someday */
        if (!found) {
            Sprintf(out_str, "%s%s", prefix, x_str);
            *firstmatch = x_str;
            found++;
        } else {
            found += append_str(out_str, x_str); /* not 'an(x_str)' */
        }
        /* for is_swallow_sym(), we want to list the current symbol's
           other possibilities (wand for '/', throne for '\\', &c) so
           don't jump to the end for the x_str==mon_interior case */
        if (x_str == unreconnoitered) {
            goto didlook;
        }
    }
 check_monsters:
    /* Check for monsters */
    if (!iflags.terrainmode || (iflags.terrainmode & TER_MON) != 0) {
        for (i = 1; i < MAXMCLASSES; i++) {
            if (sym == (looked ? monsyms[i] : def_monsyms[i]) &&
                 def_monsyms_explain(i) && *def_monsyms_explain(i)) {
                need_to_look = TRUE;
                if (!found) {
                    Sprintf(out_str, "%s%s", prefix, an(def_monsyms_explain(i)));
                    *firstmatch = def_monsyms_explain(i);
                    found++;
                } else {
                    found += append_str(out_str, an(def_monsyms_explain(i)));
                }
            }
        }
        /* handle '@' as a special case if it refers to you and you're
           playing a character which isn't normally displayed by that
           symbol; firstmatch is assumed to already be set for '@' */
        if ((looked ? (sym == monsyms[S_HUMAN] && cc.x == u.ux && cc.y == u.uy) :
                      (sym == def_monsyms[S_HUMAN] && !iflags.showrace)) &&
             !(Race_if(PM_HUMAN) || Race_if(PM_ELF)) && !Upolyd) {
            found += append_str(out_str, "you"); /* tack on "or you" */
        }
    }

    /* Now check for objects */
    if (!iflags.terrainmode || (iflags.terrainmode & TER_OBJ) != 0) {
        for (i = 1; i < MAXOCLASSES; i++) {
            if (sym == (looked ? oc_syms[i] : def_oc_syms[i]) ||
                 (looked && i == ROCK_CLASS && glyph_is_statue(glyph))) {
                need_to_look = TRUE;
                if (looked && i == VENOM_CLASS) {
                    skipped_venom++;
                    continue;
                }
                if (!found) {
                    Sprintf(out_str, "%s%s", prefix, an(def_objsyms_explain(i)));
                    *firstmatch = def_objsyms_explain(i);
                    found++;
                } else {
                    found += append_str(out_str, an(def_objsyms_explain(i)));
                }
            }
        }
    }

    if (sym == DEF_INVISIBLE) {
        extern const char altinvisexplain[]; /* drawing.c */
        /* for active clairvoyance, use alternate "unseen creature" */
        boolean usealt = (EDetect_monsters & I_SPECIAL) != 0L;
        const char *unseen_explain = !usealt ? invisexplain : altinvisexplain;

        if (!found) {
            Sprintf(out_str, "%s%s", prefix, an(unseen_explain));
            *firstmatch = unseen_explain;
            found++;
        } else {
            found += append_str(out_str, an(unseen_explain));
        }
    }

    /* Now check for graphics symbols */
    alt_i = (sym == (looked ? showsyms[0] : defsyms[0].sym)) ? 0 : (2 + 1);
    for (hit_trap = FALSE, i = 0; i < MAXPCHARS; i++) {
        /* when sym is the default background character, we process
           i == 0 three times: unexplored, stone, dark part of a room */
        if (alt_i < 2) {
            x_str = !alt_i++ ? "unexplored" : submerged ? "unknown" : "stone";
            i = 0; /* for second iteration, undo loop increment */
            /* alt_i is now 1 or 2 */
        } else {
            if (alt_i++ == 2) {
                i = 0; /* undo loop increment */
            }
            x_str = defsyms[i].explanation;
            if (submerged && !strcmp(x_str, defsyms[0].explanation)) {
                x_str = "land"; /* replace "dark part of a room" */
            }
            /* alt_i is now 3 or more and no longer of interest */
        }
        if (sym == (looked ? showsyms[i] : defsyms[i].sym) && *x_str) {
            /* avoid "an unexplored", "an stone", "an air", "a water",
               "a floor of a room", "a dark part of a room";
               article==2 => "the", 1 => "an", 0 => (none) */
            int article = strstri(x_str, " of a room") ? 2 :
                              !((alt_i <= 2) ||
                                strcmp(x_str, "air") == 0 ||
                                strcmp(x_str, "land") == 0 ||
                                strcmp(x_str, "water") == 0);

            if (!found) {
                if (is_cmap_trap(i)) {
                    Sprintf(out_str, "%sa trap", prefix);
                    hit_trap = TRUE;
                } else {
                    Sprintf(out_str, "%s%s", prefix,
                            article == 2 ? the(x_str) :
                            article == 1 ? an(x_str) : x_str);
                }
                *firstmatch = x_str;
                found++;
            } else if (!(hit_trap && is_cmap_trap(i)) &&
                       !(found >= 3 && is_cmap_drawbridge(i)) &&
                       /* don't mention vibrating square outside of Gehennom
                          unless this happens to be one (hallucination?) */
                       (i != S_vibrating_square ||
                        Inhell ||
                        (looked && glyph_is_trap(glyph) && glyph_to_trap(glyph) == VIBRATING_SQUARE))) {
                found += append_str(out_str, (article == 2) ? the(x_str) :
                                             (article == 1) ? an(x_str) : x_str);
                if (is_cmap_trap(i)) {
                    hit_trap = TRUE;
                }
            }

            if (i == S_altar || is_cmap_trap(i)) {
                need_to_look = TRUE;
            }
        }
    }

    /* Now check for warning symbols */
    for (i = 1; i < WARNCOUNT; i++) {
        x_str = def_warnsyms[i].explanation;
        if (sym == (looked ? warnsyms[i] : def_warnsyms[i].sym)) {
            if (!found) {
                Sprintf(out_str, "%s%s", prefix, def_warnsyms[i].explanation);
                *firstmatch = def_warnsyms[i].explanation;
                found++;
            } else {
                found += append_str(out_str, def_warnsyms[i].explanation);
            }
            /* Kludge: warning trumps boulders on the display.
               Reveal the boulder too or player can get confused */
            if (looked && sobj_at(BOULDER, cc.x, cc.y)) {
                Strcat(out_str, " co-located with a boulder");
            }
            break; /* out of for loop*/
        }
    }

    /* if we ignored venom and list turned out to be short, put it back */
#if NEXT_VERSION
    if (skipped_venom && found < 2) {
        x_str = def_oc_syms[VENOM_CLASS].explain;
        if (!found) {
            Sprintf(out_str, "%s%s", prefix, an(x_str));
            *firstmatch = x_str;
            found++;
        } else {
            found += append_str(out_str, an(x_str));
        }
    }
#endif

    /* Finally, handle some optional overriding symbols */
    for (j = SYM_OFF_X; j < SYM_MAX; ++j) {
        if (j == (SYM_INVISIBLE + SYM_OFF_X)) {
            continue;       /* already handled above */
        }
        tmpsym = Is_rogue_level(&u.uz) ? ov_rogue_syms[j] : ov_primary_syms[j];
        if (tmpsym && sym == tmpsym) {
            switch (j) {
            case SYM_BOULDER + SYM_OFF_X:
                if (!found) {
                    *firstmatch = "boulder";
                    Sprintf(out_str, "%s%s", prefix, an(*firstmatch));
                    found++;
                } else {
                    found += append_str(out_str, "boulder");
                }
                break;
            case SYM_PET_OVERRIDE + SYM_OFF_X:
                if (looked) {
                    int oc = 0;
                    unsigned os = 0;

                    /* convert to symbol without override in effect */
                    (void) mapglyph(glyph, &sym, &oc, &os, cc.x, cc.y, MG_FLAG_NOOVERRIDE);
                    goto check_monsters;
                }
                break;
#ifdef NEXT_VERSION
            case SYM_HERO_OVERRIDE + SYM_OFF_X:
                sym = showsyms[S_HUMAN + SYM_OFF_M];
                goto check_monsters;
#endif
            }
        }
    }
#if 0
    /* handle optional boulder symbol as a special case */
    if (o_syms[SYM_BOULDER + SYM_OFF_X] && sym == o_syms[SYM_BOULDER + SYM_OFF_X]) {
        if (!found) {
            *firstmatch = "boulder";
            Sprintf(out_str, "%s%s", prefix, an(*firstmatch));
            found++;
        } else {
            found += append_str(out_str, "boulder");
        }
    }
#endif

    /*
     * If we are looking at the screen, follow multiple possibilities or
     * an ambiguous explanation by something more detailed.
     */

    if (found > 4) {
        /* 3.6.3: this used to be "That can be many things" (without prefix)
           which turned it into a sentence that lacked its terminating period;
           we could add one below but reinstating the prefix here is better */
        Sprintf(out_str, "%scan be many things", prefix);
    }

 didlook:
    if (looked) {
        struct permonst *pm = (struct permonst *)0;

        if (found > 1 || need_to_look) {
            char monbuf[BUFSZ];
            char temp_buf[BUFSZ];

            pm = lookat(cc.x, cc.y, look_buf, monbuf);
            if (pm && for_supplement) {
                *for_supplement = pm;
            }
            *firstmatch = look_buf;
            if (*(*firstmatch)) {
                Snprintf(temp_buf, sizeof temp_buf, " (%s)", *firstmatch);
                (void) strncat(out_str, temp_buf, BUFSZ - strlen(out_str) - 1);
                found = 1; /* we have something to look up */
            }
            if (monbuf[0]) {
                Snprintf(temp_buf, sizeof temp_buf, " [seen: %s]", monbuf);
                (void) strncat(out_str, temp_buf, BUFSZ - strlen(out_str) - 1);
            }
        }
    }

    return found;
}

/* also used by getpos hack in do_name.c */
const char what_is_an_unknown_object[] = "an unknown object";

int
do_look(int mode, coord *click_cc)
{
    boolean quick = (mode == 1); /* use cursor; don't search for "more info" */
    boolean clicklook = (mode == 2); /* right mouse-click method */
    char out_str[BUFSZ], look_buf[BUFSZ];
    const char *x_str, *firstmatch = 0;
    struct permonst *pm = 0, *supplemental_pm = 0;
    int i = '\0', ans = 0;
    glyph_t sym;           /* typed symbol or converted glyph */
    int found;             /* count of matching syms found */
    coord cc;              /* screen pos of unknown glyph */
    boolean save_verbose;  /* saved value of flags.verbose */
    boolean from_screen;   /* question from the screen */
    boolean force_defsyms; /* force using glyphs from defsyms[].sym */
    boolean need_to_look;  /* need to get explan. from glyph */
    boolean hit_trap;      /* true if found trap explanation */
    int skipped_venom;     /* non-zero if we ignored "splash of venom" */
    static const char *mon_interior = "the interior of a monster";

    force_defsyms = FALSE;
    if (!clicklook) {
        if (quick) {
            from_screen = TRUE; /* yes, we want to use the cursor */
            i = 'y';
        } else {
            menu_item *pick_list = (menu_item *) 0;
            winid win;
            anything any;

            any = zeroany;
            win = create_nhwindow(NHW_MENU);
            start_menu(win);
            any.a_char = '/';
            /* 'y' and 'n' to keep backwards compatibility with previous
               versions: "Specify unknown object by cursor?" */
            add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, iflags.lootabc ? 0 : any.a_char, 'y', ATR_NONE,
                     "something on the map", MENU_ITEMFLAGS_NONE);
            any.a_char = 'i';
            add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, iflags.lootabc ? 0 : any.a_char, 0, ATR_NONE,
                     "something you're carrying", MENU_ITEMFLAGS_NONE);
            any.a_char = '?';
            add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, iflags.lootabc ? 0 : any.a_char, 'n', ATR_NONE,
                     "something else (by symbol or name)", MENU_ITEMFLAGS_NONE);
            if (!u.uswallow && !Hallucination) {
                any = zeroany;
                add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE, "", MENU_ITEMFLAGS_NONE);
                /* these options work sensibly for the swallowed case,
                   but there's no reason for the player to use them then;
                   objects work fine when hallucinating, but screen
                   symbol/monster class letter doesn't match up with
                   bogus monster type, so suppress when hallucinating */
                any.a_char = 'm';
                add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, iflags.lootabc ? 0 : any.a_char, 0, ATR_NONE,
                         "nearby monsters", MENU_ITEMFLAGS_NONE);
                any.a_char = 'M';
                add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, iflags.lootabc ? 0 : any.a_char, 0, ATR_NONE,
                         "all monsters shown on map", MENU_ITEMFLAGS_NONE);
                any.a_char = 'o';
                add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, iflags.lootabc ? 0 : any.a_char, 0, ATR_NONE,
                         "nearby objects", MENU_ITEMFLAGS_NONE);
                any.a_char = 'O';
                add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, iflags.lootabc ? 0 : any.a_char, 0, ATR_NONE,
                         "all objects shown on map", MENU_ITEMFLAGS_NONE);
                any.a_char = 'd';
                add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, iflags.lootabc ? 0 : any.a_char, 0, ATR_NONE,
                         "nearby dungeon features", MENU_ITEMFLAGS_NONE);
                any.a_char = 'D';
                add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, iflags.lootabc ? 0 : any.a_char, 0, ATR_NONE,
                         "all dungeon features shown on map", MENU_ITEMFLAGS_NONE);
            }
            end_menu(win, "What do you want to look at:");
            if (select_menu(win, PICK_ONE, &pick_list) > 0) {
                i = pick_list->item.a_char;
                free(pick_list);
            }
            destroy_nhwindow(win);
        }

        switch (i) {
        default:
        case 'q':
            return 0;

        case 'y':
        case '/':
            from_screen = TRUE;
            sym = 0;
            cc.x = u.ux;
            cc.y = u.uy;
            break;

        case 'i':
          {
            char invlet;
            struct obj *invobj;

            invlet = display_inventory((const char *) 0, TRUE);
            if (!invlet || invlet == '\033') {
                return 0;
            }
            *out_str = '\0';
            for (invobj = invent; invobj; invobj = invobj->nobj) {
                if (invobj->invlet == invlet) {
                    strcpy(out_str, singular(invobj, xname));
                    break;
                }
            }
            if (*out_str) {
                checkfile(NULL, out_str, pm, TRUE, TRUE, (char *) 0);
            }
            return 0;
          }

        case '?':
            from_screen = FALSE;
            getlin("Specify what? (type the word)", out_str);
            /* keep single space as-is */
            if (strcmp(out_str, " ")) {
                /* remove leading and trailing whitespace and
                   condense consecutive internal whitespace */
                mungspaces(out_str);
            }
            if (out_str[0] == '\0' || out_str[0] == '\033') {
                return 0;
            }

            if (out_str[1]) {
                /* user typed in a complete string */
                checkfile(NULL, out_str, pm, TRUE, TRUE, (char *) 0);
                return 0;
            }
            sym = out_str[0];
            break;

        case 'm':
            look_all(TRUE, TRUE, FALSE); /* list nearby monsters */
            return 0;

        case 'M':
            look_all(FALSE, TRUE, FALSE); /* list all monsters */
            return 0;

        case 'o':
            look_all(TRUE, FALSE, FALSE); /* list nearby objects */
            return 0;

        case 'O':
            look_all(FALSE, FALSE, FALSE); /* list all objects */
            return 0;

        case 'd':
            look_all(TRUE, FALSE, TRUE); /* list nearby dungeon features */
            return 0;

        case 'D':
            look_all(FALSE, FALSE, TRUE); /* list all dungeon features */
            return 0;
        }
    } else { /* clicklook */
        cc.x = click_cc->x;
        cc.y = click_cc->y;
        sym = 0;
        from_screen = FALSE;
    }

    /* Save the verbose flag, we change it later. */
    save_verbose = flags.verbose;
    flags.verbose = flags.verbose && !quick;
    /*
     * The user typed one letter, or we're identifying from the screen.
     */
    do {
        /* Reset some variables. */
        need_to_look = FALSE;
        pm = (struct permonst *)0;
        skipped_venom = 0;
        found = 0;
        out_str[0] = '\0';

        if (from_screen) {
            int glyph; /* glyph at selected position */

            if (flags.verbose)
                pline("Please move the cursor to %s.",
                      what_is_an_unknown_object);
            else
                pline("Pick an object.");

            ans = getpos(&cc, quick, what_is_an_unknown_object);
            if (ans < 0 || cc.x < 0) {
                flags.verbose = save_verbose;
                return 0; /* done */
            }
            flags.verbose = FALSE; /* only print long question once */

            /* Convert the glyph at the selected position to a symbol. */
            glyph = glyph_at(cc.x, cc.y);
            if (glyph_is_cmap(glyph)) {
                if (iflags.UTF8graphics) {
                    /* Temporary workaround as NetHack can't yet
                     * display UTF-8 glyphs on the topline */
                    force_defsyms = TRUE;
                    sym = defsyms[glyph_to_cmap(glyph)].sym;
                } else {
                    sym = showsyms[glyph_to_cmap(glyph)];
                }
            } else if (glyph_is_trap(glyph)) {
                sym = showsyms[trap_to_defsym(glyph_to_trap(glyph))];
            } else if (glyph_is_object(glyph)) {
                sym = oc_syms[(int)objects[glyph_to_obj(glyph)].oc_class];
                if (sym == '`' && iflags.bouldersym && (int)glyph_to_obj(glyph) == BOULDER)
                    sym = iflags.bouldersym;
            } else if (glyph_is_monster(glyph)) {
                /* takes care of pets, detected, ridden, and regular mons */
                sym = monsyms[(int)mons[glyph_to_mon(glyph)].mlet];
            } else if (glyph_is_swallow(glyph)) {
                sym = showsyms[glyph_to_swallow(glyph)+S_sw_tl];
            } else if (glyph_is_invisible(glyph)) {
                sym = DEF_INVISIBLE;
            } else if (glyph_is_warning(glyph)) {
                sym = glyph_to_warning(glyph);
                sym = warnsyms[sym];
            } else {
                impossible("do_look:  bad glyph %d at (%d,%d)",
                           glyph, (int)cc.x, (int)cc.y);
                sym = ' ';
            }
        }

        /*
         * Check all the possibilities, saving all explanations in a buffer.
         * When all have been checked then the string is printed.
         */

        /* Check for monsters */
        for (i = 0; i < MAXMCLASSES; i++) {
            if (sym == (glyph_t)(from_screen ? monsyms[i] : def_monsyms[i]) &&
                monexplain[i]) {
                need_to_look = TRUE;
                if (!found) {
                    Sprintf(out_str, "%c       %s", sym, an(monexplain[i]));
                    firstmatch = monexplain[i];
                    found++;
                } else {
                    found += append_str(out_str, an(monexplain[i]));
                }
            }
        }
        /* handle '@' as a special case if it refers to you and you're
           playing a character which isn't normally displayed by that
           symbol; firstmatch is assumed to already be set for '@' */
        if ((from_screen ?
             (sym == (glyph_t)monsyms[S_HUMAN] && cc.x == u.ux && cc.y == u.uy) :
             (sym == (glyph_t)def_monsyms[S_HUMAN] && !iflags.showrace)) &&
            !(Race_if(PM_HUMAN) || Race_if(PM_ELF)) && !Upolyd)
            found += append_str(out_str, "you"); /* tack on "or you" */

        /*
         * Special case: if identifying from the screen, and we're swallowed,
         * and looking at something other than our own symbol, then just say
         * "the interior of a monster".
         */
        if (u.uswallow && from_screen && is_swallow_sym(sym)) {
            if (!found) {
                Sprintf(out_str, "%c       %s", sym, mon_interior);
                firstmatch = mon_interior;
            } else {
                found += append_str(out_str, mon_interior);
            }
            need_to_look = TRUE;
        }

        /* Now check for objects */
        for (i = 1; i < MAXOCLASSES; i++) {
            if (sym == (glyph_t)(from_screen ? oc_syms[i] : def_oc_syms[i])) {
                need_to_look = TRUE;
                if (from_screen && i == VENOM_CLASS) {
                    skipped_venom++;
                    continue;
                }
                if (!found) {
                    Sprintf(out_str, "%c       %s", sym, an(objexplain[i]));
                    firstmatch = objexplain[i];
                    found++;
                } else {
                    found += append_str(out_str, an(objexplain[i]));
                }
            }
        }

        if (sym == DEF_INVISIBLE) {
            if (!found) {
                Sprintf(out_str, "%c       %s", sym, an(invisexplain));
                firstmatch = invisexplain;
                found++;
            } else {
                found += append_str(out_str, an(invisexplain));
            }
        }

#define is_cmap_trap(i) ((i) >= S_arrow_trap && (i) <= S_polymorph_trap)
#define is_cmap_drawbridge(i) ((i) >= S_vodbridge && (i) <= S_hcdbridge)

        /* Now check for graphics symbols */
        for (hit_trap = FALSE, i = 0; i < MAXPCHARS; i++) {
            x_str = defsyms[i].explanation;
            if (sym == (force_defsyms ? defsyms[i].sym : (from_screen ? showsyms[i] : defsyms[i].sym)) && *x_str) {
                /* avoid "an air", "a water", "a floor of a room", "a dark part of a room" */
                int article = ((i == S_room)||(i == S_darkroom)) ? 2 : /* 2=>"the" */
                              !(strcmp(x_str, "air") == 0 || /* 1=>"an"  */
                                strcmp(x_str, "water") == 0); /* 0=>(none)*/

                if (!found) {
                    if (is_cmap_trap(i)) {
                        Sprintf(out_str, "%c       a trap", sym);
                        hit_trap = TRUE;
                    } else {
                        Sprintf(out_str, "%c       %s", sym,
                                article == 2 ? the(x_str) :
                                article == 1 ? an(x_str) : x_str);
                    }
                    firstmatch = x_str;
                    found++;
                } else if (!u.uswallow && !(hit_trap && is_cmap_trap(i)) &&
                           !(found >= 3 && is_cmap_drawbridge(i))) {
                    found += append_str(out_str,
                                        article == 2 ? the(x_str) :
                                        article == 1 ? an(x_str) : x_str);
                    if (is_cmap_trap(i)) hit_trap = TRUE;
                }

                if (i == S_altar || is_cmap_trap(i))
                    need_to_look = TRUE;
            }
        }

        /* Now check for warning symbols */
        for (i = 1; i < WARNCOUNT; i++) {
            x_str = def_warnsyms[i].explanation;
            if (sym == (from_screen ? warnsyms[i] : def_warnsyms[i].sym)) {
                if (!found) {
                    Sprintf(out_str, "%c       %s",
                            sym, def_warnsyms[i].explanation);
                    firstmatch = def_warnsyms[i].explanation;
                    found++;
                } else {
                    found += append_str(out_str, def_warnsyms[i].explanation);
                }
                /* Kludge: warning trumps boulders on the display.
                   Reveal the boulder too or player can get confused */
                if (from_screen && sobj_at(BOULDER, cc.x, cc.y))
                    Strcat(out_str, " co-located with a boulder");
                break; /* out of for loop*/
            }
        }

        /* if we ignored venom and list turned out to be short, put it back */
        if (skipped_venom && found < 2) {
            x_str = objexplain[VENOM_CLASS];
            if (!found) {
                Sprintf(out_str, "%c       %s", sym, an(x_str));
                firstmatch = x_str;
                found++;
            } else {
                found += append_str(out_str, an(x_str));
            }
        }

        /* handle optional boulder symbol as a special case */
        if (iflags.bouldersym && sym == iflags.bouldersym) {
            if (!found) {
                firstmatch = "boulder";
                Sprintf(out_str, "%c       %s", sym, an(firstmatch));
                found++;
            } else {
                found += append_str(out_str, "boulder");
            }
        }

        /*
         * If we are looking at the screen, follow multiple possibilities or
         * an ambiguous explanation by something more detailed.
         */
        if (from_screen) {
            if (found > 1 || need_to_look) {
                char monbuf[BUFSZ];
                char temp_buf[BUFSZ];

                pm = lookat(cc.x, cc.y, look_buf, monbuf);
                firstmatch = look_buf;
                if (*firstmatch) {
                    Snprintf(temp_buf, sizeof temp_buf, " (%s)", firstmatch);
                    (void)strncat(out_str, temp_buf, BUFSZ-strlen(out_str)-1);
                    found = 1; /* we have something to look up */
                }
                if (monbuf[0]) {
                    Snprintf(temp_buf, sizeof temp_buf, " [seen: %s]", monbuf);
                    (void)strncat(out_str, temp_buf, BUFSZ-strlen(out_str)-1);
                }
            }
        }

        /* Finally, print out our explanation. */
        if (found) {
            pline("%s", out_str);
            /* check the data file for information about this thing */
            if (found == 1 && ans != LOOK_QUICK && ans != LOOK_ONCE &&
                (ans == LOOK_VERBOSE || (flags.help && !quick))) {
                char temp_buf[BUFSZ], supplemental_name[BUFSZ];

                supplemental_name[0] = '\0';
                Strcpy(temp_buf, firstmatch);
                /* don't reveal identity of Riders in the "More info about" prompt */
                if (is_rider(pm)) {
                    pm = NULL;
                }
                checkfile(NULL, temp_buf, pm, FALSE, (boolean) (ans == LOOK_VERBOSE), supplemental_name);
                if (supplemental_pm) {
                    do_supplemental_info(supplemental_name, supplemental_pm, (ans == LOOK_VERBOSE));
                }
            }
        } else {
            pline("I've never heard of such things.");
        }

        if (quick) check_tutorial_farlook(cc.x, cc.y);

    } while (from_screen && !quick && ans != LOOK_ONCE);

    flags.verbose = save_verbose;
    return 0;
}

static void
look_all(boolean nearby,  /* True => within BOLTLIM, False => entire map */
         boolean do_mons, /* True => monsters, False => objects */
         boolean do_dungeon_features)
{
    winid win;
    int x, y, lo_x, lo_y, hi_x, hi_y, glyph, count = 0;
    char lookbuf[BUFSZ], outbuf[BUFSZ];

    win = create_nhwindow(NHW_TEXT);
    lo_y = nearby ? max(u.uy - BOLT_LIM, 0) : 0;
    lo_x = nearby ? max(u.ux - BOLT_LIM, 1) : 1;
    hi_y = nearby ? min(u.uy + BOLT_LIM, ROWNO - 1) : ROWNO - 1;
    hi_x = nearby ? min(u.ux + BOLT_LIM, COLNO - 1) : COLNO - 1;
    for (y = lo_y; y <= hi_y; y++) {
        for (x = lo_x; x <= hi_x; x++) {
            lookbuf[0] = '\0';
            glyph = glyph_at(x, y);
            if (do_mons) {
                if (glyph_is_monster(glyph)) {
                    struct monst *mtmp;

                    bhitpos.x = x; /* [is this actually necessary?] */
                    bhitpos.y = y;
                    if (x == u.ux && y == u.uy && canspotself()) {
                        (void) self_lookat(lookbuf);
                        count++;
                    } else if ((mtmp = m_at(x, y)) != 0) {
                        look_at_monster(lookbuf, (char *) 0, mtmp, x, y);
                        count++;
                    }
                } else if (glyph_is_invisible(glyph)) {
                    /* remembered, unseen, creature */
                    Strcpy(lookbuf, invisexplain);
                    count++;
                } else if (glyph_is_warning(glyph)) {
                    int warnindx = glyph_to_warning(glyph);

                    Strcpy(lookbuf, def_warnsyms[warnindx].explanation);
                    count++;
                }
            } else if (do_dungeon_features) {
                if (glyph_is_cmap(glyph) && (is_cmap_furniture(glyph_to_cmap(glyph)))) {
                    int indx = glyph_to_cmap(glyph);
                    Strcpy(lookbuf, defsyms[indx].explanation);
                    count++;
                }
            } else {
                /* !do_mons */
                if (glyph_is_object(glyph)) {
                    look_at_object(lookbuf, x, y, glyph);
                    count++;
                }
            }
            if (*lookbuf) {
                char coordbuf[20], which[12], cmode;

                cmode = (iflags.getpos_coords != GPCOORDS_NONE) ? iflags.getpos_coords : GPCOORDS_MAP;
                if (count == 1) {
                    Strcpy(which, do_mons ? "monsters" : "objects");
                    if (nearby) {
                        char *loc = (cmode != GPCOORDS_COMPASS) ?  coord_desc(u.ux, u.uy, coordbuf, cmode) :
                                    (!canspotself()) ? "your position" : "you";
                        Sprintf(outbuf, "%s currently shown near %s:", upstart(which), loc);
                    } else {
                        Sprintf(outbuf, "All %s currently shown on the map:", which);
                    }
                    putstr(win, 0, outbuf);
                    /* hack alert! Qt watches a text window for any line
                       with 4 consecutive spaces and renders the window
                       in a fixed-width font it if finds at least one */
                    putstr(win, 0, "    "); /* separator */
                }
                /* prefix: "coords  C  " where 'C' is mon or obj symbol */
                Sprintf(outbuf, (cmode == GPCOORDS_SCREEN) ? "%s  " :
                                (cmode == GPCOORDS_MAP) ? "%8s  " : "%12s  ",
                                coord_desc(x, y, coordbuf, cmode));
#if 0           /* when putmixed */
                (void) mapglyph(glyph, &sym, &oc, &os, cc.x, cc.y, 0);
                Sprintf(eos(outbuf), "%s  ", encglyph(glyph));
#endif
                /* guard against potential overflow */
                lookbuf[sizeof lookbuf - 1 - strlen(outbuf)] = '\0';
                Strcat(outbuf, lookbuf);
                putstr(win, 0, outbuf); /* putmixed(win, 0, outbuf); */
            }
        }
    }
    if (count) {
        display_nhwindow(win, TRUE);
    } else {
        pline("No %s are currently shown %s.", do_mons ? "monsters" : "objects",
              nearby ? "nearby" : "on the map");
    }
    destroy_nhwindow(win);
}

static const char *suptext1[] = {
    "%s is a member of a marauding horde of orcs",
    "rumored to have brutally attacked and plundered",
    "the ordinarily sheltered town that is located ",
    "deep within The Gnomish Mines.",
    "",
    "The members of that vicious horde proudly and ",
    "defiantly acclaim their allegiance to their",
    "leader %s in their names.",
    (char *) 0,
};

static const char *suptext2[] = {
    "\"%s\" is the common dungeon name of",
    "a nefarious orc who is known to acquire property",
    "from thieves and sell it off for profit.",
    "",
    "The perpetrator was last seen hanging around the",
    "stairs leading to the Gnomish Mines.",
    (char *) 0,
};

static void
do_supplemental_info(name, pm, without_asking)
char *name;
struct permonst *pm;
boolean without_asking;
{
    const char **textp;
    winid datawin = WIN_ERR;
    char *entrytext = name, *bp = (char *) 0, *bp2 = (char *) 0;
    char question[QBUFSZ];
    boolean yes_to_moreinfo = FALSE;
    boolean is_marauder = (name && pm && is_orc(pm));

    /*
     * Provide some info on some specific things
     * meant to support in-game mythology, and not
     * available from data.base or other sources.
     */
    if (is_marauder && (strlen(name) < (BUFSZ - 1))) {
        char fullname[BUFSZ];

        bp = strstri(name, " of ");
        bp2 = strstri(name, " the Fence");

        if (bp || bp2) {
            Strcpy(fullname, name);
            if (!without_asking) {
                Strcpy(question, "More info about \"");
                /* +2 => length of "\"?" */
                copynchars(eos(question), entrytext,
                    (int) (sizeof question - 1 - (strlen(question) + 2)));
                Strcat(question, "\"?");
                if (yn(question) == 'y')
                yes_to_moreinfo = TRUE;
            }
            if (yes_to_moreinfo) {
                int i, subs = 0;
                const char *gang = (char *) 0;

                if (bp) {
                    textp = suptext1;
                    gang = bp + 4;
                    *bp = '\0';
                } else {
                    textp = suptext2;
                    gang = "";
                }
                datawin = create_nhwindow(NHW_MENU);
                for (i = 0; textp[i]; i++) {
                    char buf[BUFSZ];
                    const char *txt;

                    if (strstri(textp[i], "%s") != 0) {
                        Sprintf(buf, textp[i], subs++ ? gang : fullname);
                        txt = buf;
                    } else {
                        txt = textp[i];
                    }
                    putstr(datawin, 0, txt);
                }
                display_nhwindow(datawin, FALSE);
                destroy_nhwindow(datawin), datawin = WIN_ERR;
            }
        }
    }
}

/* the '/' command */
int
dowhatis()
{
    return do_look(0, (coord *) 0);
}

/* the ';' command */
int
doquickwhatis()
{
    if(iflags.num_pad)
        check_tutorial_message(QT_T_CURSOR_NUMPAD);
    else
        check_tutorial_message(QT_T_CURSOR_VIKEYS);

    return do_look(1, (coord *) 0);
}

int
doidtrap()
{
    register struct trap *trap;
    int x, y, tt;

    if (!getdir("^")) return 0;
    x = u.ux + u.dx;
    y = u.uy + u.dy;
    for (trap = ftrap; trap; trap = trap->ntrap)
        if (trap->tx == x && trap->ty == y) {
            if (!trap->tseen) break;
            tt = trap->ttyp;
            if (u.dz) {
                if (u.dz < 0 ? (tt == TRAPDOOR || tt == HOLE) :
                    tt == ROCKTRAP) break;
            }
            tt = what_trap(tt);
            pline("That is %s%s%s.",
                  an(defsyms[trap_to_defsym(tt)].explanation),
                  !trap->madeby_u ? "" : (tt == WEB) ? " woven" :
                  /* trap doors & spiked pits can't be made by
                     player, and should be considered at least
                     as much "set" as "dug" anyway */
                  (tt == HOLE || tt == PIT) ? " dug" : " set",
                  !trap->madeby_u ? "" : " by you");
            return 0;
        }
    pline("I can't see a trap there.");
    return 0;
}

char *
dowhatdoes_core(q, cbuf)
char q;
char *cbuf;
{
    dlb *fp;
    char bufr[BUFSZ];
    register char *buf = &bufr[6], *ep, ctrl, meta;

    fp = dlb_fopen_area(NH_CMDHELPAREA, CMDHELPFILE, "r");
    if (!fp) {
        pline("Cannot open data file!");
        return 0;
    }

    ctrl = ((q <= '\033') ? (q - 1 + 'A') : 0);
    meta = ((0x80 & q) ? (0x7f & q) : 0);
    while(dlb_fgets(buf, BUFSZ-6, fp)) {
        if ((ctrl && *buf=='^' && *(buf+1)==ctrl) ||
            (meta && *buf=='M' && *(buf+1)=='-' && *(buf+2)==meta) ||
            *buf==q) {
            ep = index(buf, '\n');
            if(ep) *ep = 0;
            if (ctrl && buf[2] == '\t') {
                buf = bufr + 1;
                (void) strncpy(buf, "^?      ", 8);
                buf[1] = ctrl;
            } else if (meta && buf[3] == '\t') {
                buf = bufr + 2;
                (void) strncpy(buf, "M-?     ", 8);
                buf[2] = meta;
            } else if(buf[1] == '\t') {
                buf = bufr;
                buf[0] = q;
                (void) strncpy(buf+1, "       ", 7);
            }
            (void) dlb_fclose(fp);
            Strcpy(cbuf, buf);
            return cbuf;
        }
    }
    (void) dlb_fclose(fp);
    return (char *)0;
}

int
dowhatdoes()
{
    char bufr[BUFSZ];
    char q, *reslt;

#if defined(UNIX) || defined(VMS)
    introff();
#endif
    q = yn_function("What command?", (char *)0, '\0');
#if defined(UNIX) || defined(VMS)
    intron();
#endif
    reslt = dowhatdoes_core(q, bufr);
    if (reslt)
        pline("%s", reslt);
    else
        pline("I've never heard of such commands.");
    return 0;
}

/* data for help_menu() */
static const char *help_menu_items[] = {
/* 0*/ "Long description of the game and commands.",
/* 1*/ "List of game commands.",
/* 2*/ "Concise history of NetHack.",
/* 3*/ "Info on a character in the game display.",
/* 4*/ "Info on what a given key does.",
/* 5*/ "List of game options.",
/* 6*/ "Longer explanation of game options.",
/* 7*/ "List of extended commands.",
/* 8*/ "The NetHack license.",
/* 9*/ "Redisplay tutorial messages.",
#define TUTHLP_SLOT 9
#ifdef PORT_HELP
    "%s-specific help and commands.",
#define PORT_HELP_ID 100
#define WIZHLP_SLOT 11
#else
#define WIZHLP_SLOT 10
#endif
#ifdef WIZARD
    "List of wizard-mode commands.",
#endif
    "",
    (char *)0
};

STATIC_OVL boolean
help_menu(sel)
int *sel;
{
    winid tmpwin = create_nhwindow(NHW_MENU);
#ifdef PORT_HELP
    char helpbuf[QBUFSZ];
#endif
    int i, n;
    menu_item *selected;
    anything any;

    any.a_void = 0;     /* zero all bits */
    start_menu(tmpwin);
#ifdef WIZARD
    if (!wizard) help_menu_items[WIZHLP_SLOT] = "",
        help_menu_items[WIZHLP_SLOT+1] = (char *)0;
#endif
    for (i = 0; help_menu_items[i]; i++)
#ifdef PORT_HELP
        /* port-specific line has a %s in it for the PORT_ID */
        if (help_menu_items[i][0] == '%') {
            Sprintf(helpbuf, help_menu_items[i], PORT_ID);
            any.a_int = PORT_HELP_ID + 1;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                     helpbuf, MENU_UNSELECTED);
        } else
#endif
    {
        any.a_int = (*help_menu_items[i]) ? i+1 : 0;
        if (flags.tutorial || i != TUTHLP_SLOT)
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0,
                     ATR_NONE, help_menu_items[i], MENU_UNSELECTED);
    }
    end_menu(tmpwin, "Select one item:");
    n = select_menu(tmpwin, PICK_ONE, &selected);
    destroy_nhwindow(tmpwin);
    if (n > 0) {
        *sel = selected[0].item.a_int - 1;
        free((genericptr_t)selected);
        return TRUE;
    }
    return FALSE;
}

int
dohelp()
{
    int sel = 0;

    if (help_menu(&sel)) {
        switch (sel) {
        case  0:  display_file_area(NH_HELP_AREA, HELP, TRUE);  break;
        case  1:  display_file_area(NH_SHELP_AREA, SHELP, TRUE);  break;
        case  2:  (void) dohistory();  break;
        case  3:  (void) dowhatis();  break;
        case  4:  (void) dowhatdoes();  break;
        case  5:  option_help();  break;
        case  6:  display_file_area(NH_OPTIONAREA, OPTIONFILE, TRUE);  break;
        case  7:  (void) doextlist();  break;
        case  8:  display_file_area(NH_LICENSE_AREA, LICENSE, TRUE);  break;
        case  9:  tutorial_redisplay();  break;
#ifdef WIZARD
        /* handle slot 10 or 11 */
        default: display_file_area(NH_DEBUGHELP_AREA, DEBUGHELP, TRUE);  break;
#endif
#ifdef PORT_HELP
        case PORT_HELP_ID:  port_help();  break;
#endif
        }
    }
    return 0;
}

int
dohistory()
{
    display_file_area(NH_HISTORY_AREA, HISTORY, TRUE);
    return 0;
}

/*pager.c*/
