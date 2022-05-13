/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
/* #define DEBUG */ /* uncomment to enable new eat code debugging */

#ifdef DEBUG
# ifdef WIZARD
#define debugpline  if (wizard) pline
# else
#define debugpline  pline
# endif
#endif

STATIC_PTR int NDECL(eatmdone);
STATIC_PTR int NDECL(eatfood);
STATIC_PTR int NDECL(opentin);
STATIC_PTR int NDECL(unfaint);

STATIC_DCL const char *FDECL(food_xname, (struct obj *, BOOLEAN_P));
STATIC_DCL void FDECL(choke, (struct obj *));
STATIC_DCL void NDECL(recalc_wt);
STATIC_DCL struct obj *FDECL(touchfood, (struct obj *));
STATIC_DCL void NDECL(do_reset_eat);
STATIC_DCL void FDECL(done_eating, (BOOLEAN_P));
STATIC_DCL void FDECL(cprefx, (int));
STATIC_DCL void FDECL(givit, (int, struct permonst *));
STATIC_DCL void FDECL(cpostfx, (int));
static void FDECL(consume_tin, (const char *));
STATIC_DCL void FDECL(start_tin, (struct obj *));
STATIC_DCL int FDECL(eatcorpse, (struct obj *));
STATIC_DCL void FDECL(fprefx, (struct obj *));
STATIC_DCL void FDECL(accessory_has_effect, (struct obj *));
STATIC_DCL void FDECL(fpostfx, (struct obj *));
STATIC_DCL int NDECL(bite);
STATIC_DCL int FDECL(edibility_prompts, (struct obj *));
STATIC_DCL int FDECL(rottenfood, (struct obj *));
STATIC_DCL void NDECL(eatspecial);
STATIC_DCL void FDECL(eataccessory, (struct obj *));
STATIC_DCL const char *FDECL(foodword, (struct obj *));
STATIC_DCL boolean FDECL(maybe_cannibal, (int, BOOLEAN_P));
STATIC_DCL int FDECL(tin_variety, (struct obj *, BOOLEAN_P));

char msgbuf[BUFSZ];

/* also used to see if you're allowed to eat cats and dogs */
#define CANNIBAL_ALLOWED() (Role_if(PM_CAVEMAN) || Race_if(PM_ORC) || Race_if(PM_VAMPIRE))

/* monster types that cause hero to be turned into stone if eaten */
#define flesh_petrifies(pm) (touch_petrifies(pm) || (pm) == &mons[PM_MEDUSA])

/* Rider corpses are treated as non-rotting so that attempting to eat one
   will be sure to reach the stage of eating where that meal is fatal */
#define nonrotting_corpse(mnum) \
    ((mnum) == PM_LIZARD || (mnum) == PM_LICHEN || is_rider(&mons[mnum]))

/* non-rotting non-corpses; unlike lizard corpses, these items will behave
   as if rotten if they are cursed (fortune cookies handled elsewhere) */
#define nonrotting_food(otyp) \
    ((otyp) == LEMBAS_WAFER || (otyp) == CRAM_RATION)

STATIC_OVL NEARDATA const char comestibles[] = { FOOD_CLASS, 0 };

#ifdef ASTRAL_ESCAPE
STATIC_OVL NEARDATA const char sacrifice_types[] = { AMULET_CLASS, FOOD_CLASS, 0 };
#endif

/* Gold must come first for getobj(). */
STATIC_OVL NEARDATA const char allobj[] = {
    COIN_CLASS, WEAPON_CLASS, ARMOR_CLASS, POTION_CLASS, SCROLL_CLASS,
    WAND_CLASS, RING_CLASS, AMULET_CLASS, FOOD_CLASS, TOOL_CLASS,
    GEM_CLASS, ROCK_CLASS, BALL_CLASS, CHAIN_CLASS, SPBOOK_CLASS, 0
};

STATIC_OVL boolean force_save_hs = FALSE;

const char *hu_stat[] = {
    "Satiated",
    "        ",
    "Hungry  ",
    "Weak    ",
    "Fainting",
    "Fainted ",
    "Starved "
};

/*
 * Decide whether a particular object can be eaten by the possibly
 * polymorphed character.  Not used for monster checks.
 */
boolean
is_edible(obj)
register struct obj *obj;
{
    /* protect invocation tools but not Rider corpses (handled elsewhere)*/
    /* if (obj->oclass != FOOD_CLASS && obj_resists(obj, 0, 0)) */
    if (objects[obj->otyp].oc_unique)
        return FALSE;
    /* above also prevents the Amulet from being eaten, so we must never
       allow fake amulets to be eaten either [which is already the case] */

    if (metallivorous(youmonst.data) && is_metallic(obj) &&
        (youmonst.data != &mons[PM_RUST_MONSTER] || is_rustprone(obj)))
        return TRUE;

    /* Ghouls only eat non-veggy corpses or eggs (see dogfood()) */
    if (u.umonnum == PM_GHOUL) {
        return ((obj->otyp == CORPSE && !vegan(&mons[obj->corpsenm])) ||
                (obj->otyp == EGG));
    }

    if (u.umonnum == PM_GELATINOUS_CUBE && is_organic(obj) &&
        /* [g.cubes can eat containers and retain all contents
            as engulfed items, but poly'd player can't do that] */
        !Has_contents(obj))
        return TRUE;

    /* Vampires drink the blood of meaty corpses */
    /* [ALI] (fully) drained food is not presented as an option,
     * but partly eaten food is (even though you can't drain it).
     */
    if (is_vampiric(youmonst.data))
        return (boolean)(obj->otyp == CORPSE &&
                         has_blood(&mons[obj->corpsenm]) && (!obj->odrained ||
                                                             obj->oeaten > drainlevel(obj)));

    /* return((boolean)(!!index(comestibles, obj->oclass))); */
    return (boolean)(obj->oclass == FOOD_CLASS);
}

/* used for hero init, life saving (if choking), and prayer results of fix
   starving, fix weak from hunger, or golden glow boon (if u.uhunger < 900) */
void
init_uhunger()
{
    flags.botl = (u.uhs != NOT_HUNGRY || ATEMP(A_STR) < 0);
    u.uhunger = 900;
    u.uhs = NOT_HUNGRY;
    if (ATEMP(A_STR) < 0) {
        ATEMP(A_STR) = 0;
        (void) encumber_msg();
    }
}

/* tin types [SPINACH_TIN = -1, overrides corpsenm, nut==600] */
static const struct {
    const char *txt;                      /* description */
    int nut;                              /* nutrition */
} tintxts[] = {
    { "deep fried",               60 },
    { "pickled",                  40 },
    { "soup made from",           20 },
    { "pureed",                  500 },
    {"rotten",                   -50 },
    { "homemade",                 50 },
    { "stir fried",               80 },
    { "candied",                 100 },
    { "boiled",                   50 },
    { "dried",                    55 },
#define SZECHUAN_TIN 10
    {"szechuan",                  70 },
#define FRENCH_FRIED_TIN 11
    {"french fried",              55 },
    {"sauteed",                   95 },
    {"broiled",                   80 },
    {"smoked",                    50 },
#define DELICIOUS_SOUP_TIN 15
    {"delicious soup made from", 200 },
    {"", 0}
};
#define TTSZ    SIZE(tintxts)

static NEARDATA struct {
    struct  obj *tin;
    unsigned o_id; /* o_id of tin in save file */
    int usedtime, reqtime;
} tin;

static NEARDATA struct {
    struct  obj *piece; /* the thing being eaten, or last thing that
                         * was partially eaten, unless that thing was
                         * a tin, which uses the tin structure above,
                         * in which case this should be 0 */
    unsigned o_id;      /* o_id of food object in save file */
    /* doeat() initializes these when piece is valid */
    int usedtime,   /* turns spent eating */
        reqtime;    /* turns required to eat */
    int nmod;       /* coded nutrition per turn */
    Bitfield(canchoke, 1);   /* was satiated at beginning */

    /* start_eating() initializes these */
    Bitfield(fullwarn, 1);   /* have warned about being full */
    Bitfield(eating, 1); /* victual currently being eaten */
    Bitfield(doreset, 1);    /* stop eating at end of turn */
} victual;

static char *eatmbuf = 0;   /* set by cpostfx() */

/* called after mimicing is over */
STATIC_PTR
int
eatmdone()      /* called after mimicing is over */
{
    /* release `eatmbuf' */
    if (eatmbuf) {
        if (nomovemsg == eatmbuf) nomovemsg = 0;
        free((genericptr_t)eatmbuf),  eatmbuf = 0;
    }
    /* update display */
    if (U_AP_TYPE) {
        youmonst.m_ap_type = M_AP_NOTHING;
        newsym(u.ux, u.uy);
    }
    return 0;
}

/* called when hallucination is toggled */
void
eatmupdate()
{
    const char *altmsg = 0;
    int altapp = 0; /* lint suppression */

    if (!eatmbuf || nomovemsg != eatmbuf) {
        return;
    }

    if (is_obj_mappear(&youmonst,ORANGE) && !Hallucination) {
        /* revert from hallucinatory to "normal" mimicking */
        altmsg = "You now prefer mimicking yourself.";
        altapp = GOLD_PIECE;
    } else if (is_obj_mappear(&youmonst,GOLD_PIECE) && Hallucination) {
        /* won't happen; anything which might make immobilized
           hero begin hallucinating (black light attack, theft
           of Grayswandir) will terminate the mimicry first */
        altmsg = "Your rind escaped intact.";
        altapp = ORANGE;
    }

    if (altmsg) {
        /* replace end-of-mimicking message */
        if (strlen(altmsg) > strlen(eatmbuf)) {
            free(eatmbuf);
            eatmbuf = (char *) alloc(strlen(altmsg) + 1);
        }
        nomovemsg = strcpy(eatmbuf, altmsg);
        /* update current image */
        youmonst.mappearance = altapp;
        newsym(u.ux, u.uy);
    }
}

/* ``[the(] singular(food, xname) [)]'' with awareness of unique monsters */
STATIC_OVL const char *
food_xname(food, the_pfx)
struct obj *food;
boolean the_pfx;
{
    const char *result;

    if (food->otyp == CORPSE) {
        result = corpse_xname(food, (const char *) 0, CXN_SINGULAR | (the_pfx ? CXN_PFX_THE : 0));
        /* not strictly needed since pname values are capitalized
           and the() is a no-op for them */
        if (type_is_pname(&mons[food->corpsenm])) {
            the_pfx = FALSE;
        }
    } else {
        /* the ordinary case */
        result = singular(food, xname);
    }
    if (the_pfx) {
        result = the(result);
    }
    return result;
}

/* Created by GAN 01/28/87
 * Amended by AKP 09/22/87: if not hard, don't choke, just vomit.
 * Amended by 3.  06/12/89: if not hard, sometimes choke anyway, to keep risk.
 *                11/10/89: if hard, rarely vomit anyway, for slim chance.
 *
 * To a full belly all food is bad. (It.)
 */
STATIC_OVL void
choke(food)
struct obj *food;
{
    /* only happens if you were satiated */
    if (u.uhs != SATIATED) {
        if (!food || food->otyp != AMULET_OF_STRANGULATION)
            return;
    } else if (Role_if(PM_KNIGHT) && u.ualign.type == A_LAWFUL) {
        adjalign(-1); /* gluttony is unchivalrous */
        You_feel("like a glutton!");
    }

    exercise(A_CON, FALSE);

    if (Breathless || (!Strangled && !rn2(20))) {
        /* choking by eating AoS doesn't involve stuffing yourself */
        if (food && food->otyp == AMULET_OF_STRANGULATION) {
            You("choke, but recover your composure.");
            return;
        }
        You("stuff yourself and then vomit voluminously.");
        morehungry(1000);   /* you just got *very* sick! */
        nomovemsg = 0;
        vomit();
    } else {
        killer.format = KILLED_BY_AN;
        /*
         * Note all "killer"s below read "Choked on %s" on the
         * high score list & tombstone.  So plan accordingly.
         */
        if (food) {
            You("choke over your %s.", foodword(food));
            if (food->oclass == COIN_CLASS) {
                Strcpy(killer.name, "very rich meal");
            } else {
                killer.format = KILLED_BY;
                Strcpy(killer.name, killer_xname(food));
            }
        } else {
            You("choke over it.");
            Strcpy(killer.name, "quick snack");
        }
        You("die...");
        done(CHOKING);
    }
}

/* modify object wt. depending on time spent consuming it */
STATIC_OVL void
recalc_wt()
{
    struct obj *piece = victual.piece;
    if (!piece) {
        impossible("recalc_wt without piece");
        return;
    }
#ifdef DEBUG
    debugpline("Old weight = %d", piece->owt);
    debugpline("Used time = %d, Req'd time = %d",
               victual.usedtime, victual.reqtime);
#endif
    piece->owt = weight(piece);
#ifdef DEBUG
    debugpline("New weight = %d", piece->owt);
#endif
}

/* called when eating interrupted by an event */
void
reset_eat()
{
    /* we only set a flag here - the actual reset process is done after
     * the round is spent eating.
     */
    if (victual.eating && !victual.doreset) {
#ifdef DEBUG
        debugpline("reset_eat...");
#endif
        victual.doreset = TRUE;
    }
    return;
}

/* base nutrition of a food-class object */
static unsigned
obj_nutrition(otmp)
struct obj *otmp;
{
    unsigned nut;
    if (otmp->otyp == CORPSE) {
        nut = mons[otmp->corpsenm].cnutrit;
    } else {
        nut = objects[otmp->otyp].oc_nutrition;
    }

    if (otmp->otyp == LEMBAS_WAFER) {
        if (maybe_polyd(is_elf(youmonst.data), Race_if(PM_ELF))) {
            nut += nut / 4; /* 800 -> 1000 */
        } else if (maybe_polyd(is_orc(youmonst.data), Race_if(PM_ORC))) {
            nut -= nut / 4; /* 800 -> 600 */
        }
        /* prevent polymorph making a partly eaten wafer
           become more nutritious than an untouched one */
        if (otmp->oeaten >= nut) {
            otmp->oeaten = (otmp->oeaten < objects[LEMBAS_WAFER].oc_nutrition) ? (nut - 1) : nut;
        }
    } else if (otmp->otyp == CRAM_RATION) {
        if (maybe_polyd(is_dwarf(youmonst.data), Race_if(PM_DWARF))) {
            nut += nut / 6; /* 600 -> 700 */
        }
    }
    return nut;
}

STATIC_OVL struct obj *
touchfood(otmp)
struct obj *otmp;
{
    if (otmp->quan > 1L) {
        if(!carried(otmp))
            (void) splitobj(otmp, otmp->quan - 1L);
        else
            otmp = splitobj(otmp, 1L);
#ifdef DEBUG
        debugpline("split object,");
#endif
    }
    if (!otmp->oeaten) {
        costly_alteration(otmp, COST_BITE);
        otmp->oeaten = obj_nutrition(otmp);
    }

    if (carried(otmp)) {
        freeinv(otmp);
        if (inv_cnt(FALSE) >= 52) {
            sellobj_state(SELL_DONTSELL);
            dropy(otmp);
            sellobj_state(SELL_NORMAL);
        } else {
            otmp->nomerge = 1; /* used to prevent merge */
            otmp = addinv(otmp);
            otmp->nomerge = 0;
        }
    }
    return(otmp);
}

/* When food decays, in the middle of your meal, we don't want to dereference
 * any dangling pointers, so set it to null (which should still trigger
 * do_reset_eat() at the beginning of eatfood()) and check for null pointers
 * in do_reset_eat().
 */
void
food_disappears(obj)
struct obj *obj;
{
    if (obj == victual.piece) {
        victual.piece = (struct obj *) 0;
        victual.o_id = 0;
    }
    if (obj->timed) {
        obj_stop_timers(obj);
    }
}

/* renaming an object usually results in it having a different address;
   so the sequence start eating/opening, get interrupted, name the food,
   resume eating/opening would restart from scratch */
void
food_substitution(old_obj, new_obj)
struct obj *old_obj, *new_obj;
{
    if (old_obj == victual.piece) {
        victual.piece = new_obj;
        victual.o_id = new_obj->o_id;
    }
    if (old_obj == tin.tin) {
        tin.tin = new_obj;
        tin.o_id = new_obj->o_id;
    }
}

STATIC_OVL void
do_reset_eat()
{
#ifdef DEBUG
    debugpline("do_reset_eat...");
#endif
    if (victual.piece) {
        victual.o_id = 0;
        victual.piece = touchfood(victual.piece);
        if (victual.piece) {
            victual.o_id = victual.piece->o_id;
        }
        recalc_wt();
    }
    victual.fullwarn = victual.eating = victual.doreset = FALSE;
    /* Do not set canchoke to FALSE; if we continue eating the same object
     * we need to know if canchoke was set when they started eating it the
     * previous time.  And if we don't continue eating the same object
     * canchoke always gets recalculated anyway.
     */
    stop_occupation();
    newuhs(FALSE);
}

/* called each move during eating process */
static int
eatfood()
{
    if (!victual.piece ||
         (!carried(victual.piece) && !obj_here(victual.piece, u.ux, u.uy))) {
        /* maybe it was stolen? */
        do_reset_eat();
        return(0);
    }
    if (is_vampiric(youmonst.data) != victual.piece->odrained) {
        /* Polymorphed while eating/draining */
        do_reset_eat();
        return(0);
    }
    if(!victual.eating) return(0);

    if (++victual.usedtime <= victual.reqtime) {
        if(bite()) return(0);
        return(1);  /* still busy */
    } else {    /* done */
        int crumbs = victual.piece->oeaten;     /* The last crumbs */
        if (victual.piece->odrained) crumbs -= drainlevel(victual.piece);
        if (crumbs > 0) {
            lesshungry(crumbs);
            victual.piece->oeaten -= crumbs;
        }
        done_eating(TRUE);
        return(0);
    }
}

STATIC_OVL void
done_eating(message)
boolean message;
{
    victual.piece->in_use = TRUE;

    occupation = 0; /* do this early, so newuhs() knows we're done */
    newuhs(FALSE);
    if (nomovemsg) {
        if (message) pline("%s", nomovemsg);
        nomovemsg = 0;
    } else if (message)
        You("finish %s %s.", victual.piece->odrained ? "draining" :
            "eating", food_xname(victual.piece, TRUE));

    if (victual.piece->otyp == CORPSE) {
        if (!victual.piece->odrained || (Race_if(PM_VAMPIRE) && !rn2(5)))
            cpostfx(victual.piece->corpsenm);
    } else
        fpostfx(victual.piece);

    if (victual.piece->odrained) {
        victual.piece->in_use = FALSE;
    } else if (carried(victual.piece)) {
        useup(victual.piece);
    } else {
        useupf(victual.piece, 1L);
    }
    victual.piece = (struct obj *) 0;
    victual.o_id = 0;
    victual.fullwarn = victual.eating = victual.doreset = FALSE;
}

/* eating a corpse or egg of one's own species is usually naughty */
STATIC_OVL boolean
maybe_cannibal(pm, allowmsg)
int pm;
boolean allowmsg;
{
    static NEARDATA long ate_brains = 0L;
    struct permonst *fptr = &mons[pm]; /* food type */

    /* when poly'd into a mind flayer, multiple tentacle hits in one
       turn cause multiple digestion checks to occur; avoid giving
       multiple luck penalties for the same attack */
    if (moves == ate_brains) {
        return FALSE;
    }
    ate_brains = moves; /* ate_anything, not just brains... */

    if (!CANNIBAL_ALLOWED() &&
        /* non-cannibalistic heroes shouldn't eat own species ever
           and also shouldn't eat current species when polymorphed
           (even if having the form of something which doesn't care
           about cannibalism--hero's innate traits aren't altered) */
         (your_race(fptr) ||
          (Upolyd && same_race(youmonst.data, fptr)) ||
          (u.ulycn >= LOW_PM && were_beastie(pm) == u.ulycn))) {
        if (allowmsg) {
            if (Upolyd && your_race(fptr)) {
                You("have a bad feeling deep inside.");
            }
            if (Hallucination) {
                You("feel unaccountably peckish.");      /* Fallen London */
            } else {
                You("cannibal!  You will regret this!");
            }
        }
        HAggravate_monster |= FROMOUTSIDE;
        change_luck(-rn1(4, 2)); /* -5..-2 */
        return TRUE;
    }
    return FALSE;
}

STATIC_OVL void
cprefx(pm)
register int pm;
{
    (void) maybe_cannibal(pm, TRUE);
    if (flesh_petrifies(&mons[pm])) {
        if (!Stone_resistance &&
            !(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))) {
            Sprintf(killer.name, "tasting %s meat", mons[pm].mname);
            killer.format = KILLED_BY;
            You("turn to stone.");
            done(STONING);
            if (victual.piece)
                victual.eating = FALSE;
            return; /* lifesaved */
        }
    }

    switch(pm) {
    case PM_LITTLE_DOG:
    case PM_DOG:
    case PM_LARGE_DOG:
    case PM_KITTEN:
    case PM_HOUSECAT:
    case PM_LARGE_CAT:
        /* cannibals are allowed to eat domestic animals without penalty */
        if (!CANNIBAL_ALLOWED()) {
            You_feel("that eating the %s was a bad idea.", mons[pm].mname);
            HAggravate_monster |= FROMOUTSIDE;
        }
        break;
    case PM_LIZARD:
        if (Stoned) fix_petrification();
        break;
    case PM_DISENCHANTER:
        /* hallucination decreases chance of removed intrinsic */
        if ((!flags.perma_hallu && Hallucination) ? rn2(5) : rn2(2)) attrcurse();
        if ((!flags.perma_hallu && Hallucination)) {
            (void) make_hallucinated(0L, FALSE, 0L);
            pline("The world seems less enchanting.");
        } if (!!flags.perma_hallu) {
            pline("The world briefly seems less enchanting.");
        }
        break;
    case PM_DEATH:
    case PM_PESTILENCE:
    case PM_FAMINE:
    { char buf[BUFSZ];
      pline("Eating that is instantly fatal.");
      Sprintf(killer.name, "unwisely ate the brain of %s", mons[pm].mname);
      killer.format = NO_KILLER_PREFIX;
      done(DIED);
      /* life-saving needed to reach here */
      exercise(A_WIS, FALSE);
      /* It so happens that since we know these monsters */
      /* cannot appear in tins, victual.piece will always */
      /* be what we want, which is not generally true. */
      if (revive_corpse(victual.piece)) {
          victual.piece = (struct obj *)0;
          victual.o_id = 0;
      }
      return;}
    case PM_GREEN_SLIME:
        if (!Slimed && !Unchanging && !slimeproof(youmonst.data)) {
            You("don't feel very well.");
            make_slimed(10L, (char *) 0);
            delayed_killer(SLIMED, KILLED_BY_AN, "");
            flags.botl = 1;
        }
    /* Fall through */
    default:
        if (acidic(&mons[pm]) && Stoned)
            fix_petrification();
        break;
    }
}

/**
 * Called when a vampire bites a monster.
 * Returns TRUE if hero died and was lifesaved.
 */

boolean
bite_monster(mon)
struct monst *mon;
{
    switch(monsndx(mon->data)) {
    case PM_LIZARD:
        if (Stoned) fix_petrification();
        break;
    case PM_DEATH:
    case PM_PESTILENCE:
    case PM_FAMINE:
        pline("Unfortunately, eating any of it is fatal.");
        done_in_by(mon);
        return TRUE;        /* lifesaved */

    case PM_GREEN_SLIME:
        if (!Unchanging && youmonst.data != &mons[PM_FIRE_VORTEX] &&
            youmonst.data != &mons[PM_FIRE_ELEMENTAL] &&
            youmonst.data != &mons[PM_GREEN_SLIME]) {
            You("don't feel very well.");
            make_slimed(10L, (char *) 0);
            delayed_killer(SLIMED, KILLED_BY_AN, "");
            flags.botl = 1;
        }
    /* Fall through */
    default:
        if (acidic(mon->data) && Stoned)
            fix_petrification();
        break;
    }
    return FALSE;
}

void
fix_petrification()
{
    char buf[BUFSZ];

    if (Hallucination) {
        Sprintf(buf, "What a pity--you just ruined a future piece of %sart!",
                ACURR(A_CHA) > 15 ? "fine " : "");
    } else {
        Strcpy(buf, "You feel limber!");
    }
    make_stoned(0L, buf, 0, (char *) 0);
}

/*
 * If you add an intrinsic that can be gotten by eating a monster, add it
 * to intrinsic_possible() and givit().  (It must already be in prop.h to
 * be an intrinsic property.)
 * It would be very easy to make the intrinsics not try to give you one
 * that you already had by checking to see if you have it in
 * intrinsic_possible() instead of givit(), but we're not that nice.
 */

/* intrinsic_possible() returns TRUE iff a monster can give an intrinsic. */
int
intrinsic_possible(type, ptr)
int type;
register struct permonst *ptr;
{
    switch (type) {
    case FIRE_RES:
#ifdef DEBUG
        if (ptr->mconveys & MR_FIRE) {
            debugpline("can get fire resistance");
            return(TRUE);
        } else return(FALSE);
#else
        return(ptr->mconveys & MR_FIRE);
#endif
    case SLEEP_RES:
#ifdef DEBUG
        if (ptr->mconveys & MR_SLEEP) {
            debugpline("can get sleep resistance");
            return(TRUE);
        } else return(FALSE);
#else
        return(ptr->mconveys & MR_SLEEP);
#endif
    case COLD_RES:
#ifdef DEBUG
        if (ptr->mconveys & MR_COLD) {
            debugpline("can get cold resistance");
            return(TRUE);
        } else return(FALSE);
#else
        return(ptr->mconveys & MR_COLD);
#endif
    case DISINT_RES:
#ifdef DEBUG
        if (ptr->mconveys & MR_DISINT) {
            debugpline("can get disintegration resistance");
            return(TRUE);
        } else return(FALSE);
#else
        return(ptr->mconveys & MR_DISINT);
#endif
    case SHOCK_RES:     /* shock (electricity) resistance */
#ifdef DEBUG
        if (ptr->mconveys & MR_ELEC) {
            debugpline("can get shock resistance");
            return(TRUE);
        } else return(FALSE);
#else
        return(ptr->mconveys & MR_ELEC);
#endif
    case POISON_RES:
#ifdef DEBUG
        if (ptr->mconveys & MR_POISON) {
            debugpline("can get poison resistance");
            return(TRUE);
        } else return(FALSE);
#else
        return(ptr->mconveys & MR_POISON);
#endif
    case TELEPORT:
#ifdef DEBUG
        if (can_teleport(ptr)) {
            debugpline("can get teleport");
            return(TRUE);
        } else return(FALSE);
#else
        return(can_teleport(ptr));
#endif
    case TELEPORT_CONTROL:
#ifdef DEBUG
        if (control_teleport(ptr)) {
            debugpline("can get teleport control");
            return(TRUE);
        } else return(FALSE);
#else
        return(control_teleport(ptr));
#endif
    case TELEPAT:
#ifdef DEBUG
        if (telepathic(ptr)) {
            debugpline("can get telepathy");
            return(TRUE);
        } else return(FALSE);
#else
        return(telepathic(ptr));
#endif
    default:
        return(FALSE);
    }
    /*NOTREACHED*/
}

/* givit() tries to give you an intrinsic based on the monster's level
 * and what type of intrinsic it is trying to give you.
 */
STATIC_OVL void
givit(type, ptr)
int type;
register struct permonst *ptr;
{
    register int chance;

#ifdef DEBUG
    debugpline("Attempting to give intrinsic %d", type);
#endif
    /* some intrinsics are easier to get than others */
    switch (type) {
    case POISON_RES:
        if ((ptr == &mons[PM_KILLER_BEE] ||
             ptr == &mons[PM_SCORPION]) && !rn2(4))
            chance = 1;
        else
            chance = 15;
        break;
    case TELEPORT:
        chance = 10;
        break;
    case TELEPORT_CONTROL:
        chance = 12;
        break;
    case TELEPAT:
        chance = 1;
        break;
    default:
        chance = 15;
        break;
    }

    if (ptr->mlevel <= rn2(chance))
        return; /* failed die roll */

    switch (type) {
    case FIRE_RES:
#ifdef DEBUG
        debugpline("Trying to give fire resistance");
#endif
        if(!(HFire_resistance & FROMOUTSIDE)) {
            You(Hallucination ? "be chillin'." :
                "feel a momentary chill.");
            HFire_resistance |= FROMOUTSIDE;
        }
        break;
    case SLEEP_RES:
#ifdef DEBUG
        debugpline("Trying to give sleep resistance");
#endif
        if(!(HSleep_resistance & FROMOUTSIDE)) {
            You_feel("wide awake.");
            HSleep_resistance |= FROMOUTSIDE;
        }
        break;
    case COLD_RES:
#ifdef DEBUG
        debugpline("Trying to give cold resistance");
#endif
        if(!(HCold_resistance & FROMOUTSIDE)) {
            You_feel("full of hot air.");
            HCold_resistance |= FROMOUTSIDE;
        }
        break;
    case DISINT_RES:
#ifdef DEBUG
        debugpline("Trying to give disintegration resistance");
#endif
        if(!(HDisint_resistance & FROMOUTSIDE)) {
            You_feel(Hallucination ?
                     "totally together, man." :
                     "very firm.");
            HDisint_resistance |= FROMOUTSIDE;
        }
        break;
    case SHOCK_RES:     /* shock (electricity) resistance */
#ifdef DEBUG
        debugpline("Trying to give shock resistance");
#endif
        if(!(HShock_resistance & FROMOUTSIDE)) {
            if (Hallucination)
                You_feel("grounded in reality.");
            else
                Your("health currently feels amplified!");
            HShock_resistance |= FROMOUTSIDE;
        }
        break;
    case POISON_RES:
#ifdef DEBUG
        debugpline("Trying to give poison resistance");
#endif
        if(!(HPoison_resistance & FROMOUTSIDE)) {
            You_feel(Poison_resistance ?
                     "especially healthy." : "healthy.");
            HPoison_resistance |= FROMOUTSIDE;
        }
        break;
    case TELEPORT:
#ifdef DEBUG
        debugpline("Trying to give teleport");
#endif
        if(!(HTeleportation & FROMOUTSIDE)) {
            You_feel(Hallucination ? "diffuse." :
                     "very jumpy.");
            HTeleportation |= FROMOUTSIDE;
        }
        break;
    case TELEPORT_CONTROL:
#ifdef DEBUG
        debugpline("Trying to give teleport control");
#endif
        if(!(HTeleport_control & FROMOUTSIDE)) {
            You_feel(Hallucination ?
                     "centered in your personal space." :
                     "in control of yourself.");
            HTeleport_control |= FROMOUTSIDE;
        }
        break;
    case TELEPAT:
#ifdef DEBUG
        debugpline("Trying to give telepathy");
#endif
        if(!(HTelepat & FROMOUTSIDE)) {
            if (!BTelepat) {
                You_feel(Hallucination ?
                         "in touch with the cosmos." :
                         "a strange mental acuity.");
            } else {
                You_feel(Hallucination ?
                         "a strange mental acuity." :
                         "a dull aching inside your mind.");
            }

            HTelepat |= FROMOUTSIDE;
            /* If blind, make sure monsters show up. */
            if (Blind) see_monsters();
        }
        break;
    default:
#ifdef DEBUG
        debugpline("Tried to give an impossible intrinsic");
#endif
        break;
    }
}

/* called after completely consuming a corpse */
STATIC_OVL void
cpostfx(pm)
int pm;
{
    int tmp = 0;
    int catch_lycanthropy = NON_PM;
    boolean check_intrinsics = FALSE;

    /* in case `afternmv' didn't get called for previously mimicking
       gold, clean up now to avoid `eatmbuf' memory leak */
    if (eatmbuf) (void)eatmdone();

    switch(pm) {
    case PM_NEWT:
        /* MRKR: "eye of newt" may give small magical energy boost */
        if (rn2(3) || 3 * u.uen <= 2 * u.uenmax) {
            int old_uen = u.uen;

            u.uen += rnd(3);
            if (u.uen > u.uenmax) {
                if (!rn2(3)) u.uenmax++;
                u.uen = u.uenmax;
            }
            if (old_uen != u.uen) {
                You_feel("a mild buzz.");
                flags.botl = 1;
            }
        }
        break;

    case PM_WRAITH:
        pluslvl(FALSE);
        break;

    case PM_HUMAN_WERERAT:
        catch_lycanthropy = PM_WERERAT;
        break;

    case PM_HUMAN_WEREJACKAL:
        catch_lycanthropy = PM_WEREJACKAL;
        break;

    case PM_HUMAN_WEREWOLF:
        catch_lycanthropy = PM_WEREWOLF;
        break;

    case PM_NURSE:
        if (Upolyd) u.mh = u.mhmax;
        else u.uhp = u.uhpmax;
        make_blinded(0L, !u.ucreamed);
        flags.botl = 1;
        check_intrinsics = TRUE; /* might also convey poison resistance */
        break;

    case PM_STALKER:
        if (!Invis) {
            set_itimeout(&HInvis, (long)rn1(100, 50));
            if (!Blind && !BInvis) self_invis_message();
        } else {
            if (!(HInvis & INTRINSIC)) You_feel("hidden!");
            HInvis |= FROMOUTSIDE;
            HSee_invisible |= FROMOUTSIDE;
        }
        newsym(u.ux, u.uy);
        /* fall through */

    case PM_YELLOW_LIGHT:
    case PM_GIANT_BAT:
        make_stunned(HStun + 30, FALSE);
        /* fall through */

    case PM_BAT:
        make_stunned(HStun + 30, FALSE);
        break;

    case PM_GIANT_MIMIC:
        tmp += 10;
        /* fall through */

    case PM_LARGE_MIMIC:
        tmp += 20;
        /* fall through */

    case PM_SMALL_MIMIC:
        tmp += 20;
        if (youmonst.data->mlet != S_MIMIC && !Unchanging) {
            char buf[BUFSZ];

            u.uconduct.polyselfs++; /* you're changing form */
            You_cant("resist the temptation to mimic %s.",
                     Hallucination ? "an orange" : "a pile of gold");
#ifdef STEED
            /* A pile of gold can't ride. */
            if (u.usteed) dismount_steed(DISMOUNT_FELL);
#endif
            nomul(-tmp, "pretending to be a pile of gold");
            Sprintf(buf, Hallucination ?
                    "You suddenly dread being peeled and mimic %s again!" :
                    "You now prefer mimicking %s again.",
                    an(Upolyd ? youmonst.data->mname : urace.noun));
            eatmbuf = dupstr(buf);
            nomovemsg = eatmbuf;
            afternmv = eatmdone;
            /* ??? what if this was set before? */
            youmonst.m_ap_type = M_AP_OBJECT;
            youmonst.mappearance = Hallucination ? ORANGE : GOLD_PIECE;
            newsym(u.ux, u.uy);
            curs_on_u();
            /* make gold symbol show up now */
            display_nhwindow(WIN_MAP, TRUE);
        }
        break;

    case PM_QUANTUM_MECHANIC:
        Your("velocity suddenly seems very uncertain!");
        if (HFast & INTRINSIC) {
            HFast &= ~INTRINSIC;
            You("seem slower.");
        } else {
            HFast |= FROMOUTSIDE;
            You("seem faster.");
        }
        break;

    case PM_LIZARD:
        if ((HStun & TIMEOUT) > 2) {
            make_stunned(2L, FALSE);
        }
        if ((HConfusion & TIMEOUT) > 2) {
            make_confused(2L, FALSE);
        }
        break;

    case PM_CHAMELEON:
    case PM_DOPPELGANGER:
    case PM_SANDESTIN: /* moot--they don't leave corpses */
        if (Unchanging) {
            You_feel("momentarily different."); /* same as poly trap */
        } else {
            You_feel("a change coming over you.");
            polyself(0);
        }
        break;

    case PM_EVIL_EYE:
        if (victual.piece->blessed) {
            You_feel("more fortunate.");
            change_luck(rnd(3));
        } else if (victual.piece->cursed) {
            You_feel("less fortunate.");
            change_luck(-rnd(3));
        } else {
            You_feel("your fortunes in flux.");
            change_luck(2-rn2(4));
        }
        break;

    case PM_MIND_FLAYER:
    case PM_MASTER_MIND_FLAYER:
        if (ABASE(A_INT) < ATTRMAX(A_INT)) {
            if (!rn2(2)) {
                pline("Yum!  That was real brain food!");
                (void) adjattrib(A_INT, 1, FALSE);
                break; /* don't give them telepathy, too */
            }
        }
        else {
            pline("For some reason, that tasted bland.");
        }
    /* fall through */

    default:
        check_intrinsics = TRUE;
        break;
    }

    /* possibly convey an intrinsic */
    if (check_intrinsics) {
        struct permonst *ptr = &mons[pm];
        boolean conveys_STR = is_giant(ptr);
        int i, count;

        if (dmgtype(ptr, AD_STUN) || dmgtype(ptr, AD_HALU) ||
            pm == PM_VIOLET_FUNGUS) {
            pline("Oh wow!  Great stuff!");
            (void) make_hallucinated((HHallucination & TIMEOUT) + 200L, FALSE, 0L);
        }

        /* Check the monster for all of the intrinsics.  If this
         * monster can give more than one, pick one to try to give
         * from among all it can give.
         *
         * Strength from giants is now treated like an intrinsic
         * rather than being given unconditionally.
         *
         * If a monster can give 4 intrinsics then you have
         * a 1/1 * 1/2 * 2/3 * 3/4 = 1/4 chance of getting the first,
         * a 1/2 * 2/3 * 3/4 = 1/4 chance of getting the second,
         * a 1/3 * 3/4 = 1/4 chance of getting the third,
         * and a 1/4 chance of getting the fourth.
         *
         * And now a proof by induction:
         * it works for 1 intrinsic (1 in 1 of getting it)
         * for 2 you have a 1 in 2 chance of getting the second,
         *  otherwise you keep the first
         * for 3 you have a 1 in 3 chance of getting the third,
         *  otherwise you keep the first or the second
         * for n+1 you have a 1 in n+1 chance of getting the (n+1)st,
         *  otherwise you keep the previous one.
         * Elliott Kleinrock, October 5, 1990
         */

        count = 0; /* number of possible intrinsics */
        tmp = 0;   /* which one we will try to give */
        if (conveys_STR) {
            count = 1;
            tmp = -1; /* use -1 as fake prop index for STR */
            debug_pline("\"Intrinsic\" strength, %d", tmp);
        }
        for (i = 1; i <= LAST_PROP; i++) {
            if (intrinsic_possible(i, ptr)) {
                count++;
                /* a 1 in count chance of replacing the old
                 * one with this one, and a count-1 in count
                 * chance of keeping the old one.  (note
                 * that 1 in 1 and 0 in 1 are what we want
                 * for the first one
                 */
                if (!rn2(count)) {
#ifdef DEBUG
                    debugpline("Intrinsic %d replacing %d",
                               i, tmp);
#endif
                    tmp = i;
                }
            }
        }
        /* if strength is the only candidate, give it 50% chance */
        if (conveys_STR && count == 1 && !rn2(2)) {
            tmp = 0;
        }
        /* if something was chosen, give it now (givit() might fail) */
        if (tmp == -1) {
            gainstr((struct obj *) 0, 0);
        } else if (tmp > 0) {
            givit(tmp, ptr);
        }
    }

    if (catch_lycanthropy >= LOW_PM) {
        set_ulycn(catch_lycanthropy);
        retouch_equipment(2);
    }

    return;
}

/**
 * @deprecated violated(CONDUCT_VEGETARIAN) replaces violated_vegetarian() */
void
violated_vegetarian()
{
    violated(CONDUCT_VEGETARIAN);
}

/* common code to check and possibly charge for 1 context.tin.tin,
 * will split() context.tin.tin if necessary */
static struct obj *
costly_tin(alter_type)
int alter_type; /* COST_xxx */
{
    if (carried(tin.tin) ? tin.tin->unpaid :
        (costly_spot(tin.tin->ox, tin.tin->oy) && !tin.tin->no_charge)) {
        if (tin.tin->quan > 1L) {
            tin.tin = splitobj(tin.tin, 1L);
        }
        costly_alteration(tin.tin, alter_type);
    }
    return tin.tin;
}

int
tin_variety_txt(s, tinvariety)
char *s;
int *tinvariety;
{
    int k, l;

    if (s && tinvariety) {
        *tinvariety = -1;
        for (k = 0; k < TTSZ - 1; ++k) {
            l = (int) strlen(tintxts[k].txt);
            if (!strncmpi(s, tintxts[k].txt, l) && ((int) strlen(s) > l) && s[l] == ' ') {
                *tinvariety = k;
                return (l + 1);
            }
        }
    }
    return 0;
}

/*
 * This assumes that buf already contains the word "tin",
 * as is the case with caller xname().
 */
void
tin_details(obj, mnum, buf)
struct obj *obj;
int mnum;
char *buf;
{
    char buf2[BUFSZ];
    int r = tin_variety(obj, TRUE);

    if (obj && buf) {
        if (r == SPINACH_TIN) {
            Strcat(buf, " of spinach");
        } else if (mnum == NON_PM) {
            Strcpy(buf, "empty tin");
        } else {
            if ((obj->cknown || iflags.override_ID) && obj->spe < 0) {
                if (r == ROTTEN_TIN || r == HOMEMADE_TIN) {
                    /* put these before the word tin */
                    Sprintf(buf2, "%s %s of ", tintxts[r].txt, buf);
                    Strcpy(buf, buf2);
                } else {
                    Sprintf(eos(buf), " of %s ", tintxts[r].txt);
                }
            } else {
                Strcpy(eos(buf), " of ");
            }
            if (vegetarian(&mons[mnum])) {
                Sprintf(eos(buf), "%s", mons[mnum].mname);
            } else {
                Sprintf(eos(buf), "%s meat", mons[mnum].mname);
            }
        }
    }
}

void
set_tin_variety(obj, forcetype)
struct obj *obj;
int forcetype;
{
    register int r;

    if (forcetype == SPINACH_TIN
        || (forcetype == HEALTHY_TIN
            && (obj->corpsenm == NON_PM /* empty or already spinach */
                || !vegetarian(&mons[obj->corpsenm])))) { /* replace meat */
        obj->corpsenm = NON_PM; /* not based on any monster */
        obj->spe = 1;           /* spinach */
        return;
    } else if (forcetype == HEALTHY_TIN) {
        r = tin_variety(obj, FALSE);
        if (r < 0 || r >= TTSZ) {
            r = ROTTEN_TIN; /* shouldn't happen */
        }
        while ((r == ROTTEN_TIN && !obj->cursed)) {
            r = rn2(TTSZ - 1);
        }
    } else if (forcetype >= 0 && forcetype < TTSZ - 1) {
        r = forcetype;
    } else {               /* RANDOM_TIN */
        r = rn2(TTSZ - 1); /* take your pick */
        if (r == ROTTEN_TIN && nonrotting_corpse(obj->corpsenm)) {
            r = HOMEMADE_TIN; /* lizards don't rot */
        }
    }
    obj->spe = -(r + 1); /* offset by 1 to allow index 0 */
}

STATIC_OVL int
tin_variety(obj, disp)
struct obj *obj;
boolean disp; /* we're just displaying so leave things alone */
{
    register int r;

    if (obj->spe == 1) {
        r = SPINACH_TIN;
    } else if (obj->cursed) {
        r = ROTTEN_TIN; /* always rotten if cursed */
    } else if (obj->spe < 0) {
        r = -(obj->spe);
        --r; /* get rid of the offset */
    } else {
        r = rn2(TTSZ-2);
    }

    if (!disp && r == HOMEMADE_TIN && !obj->blessed && !rn2(7)) {
        r = ROTTEN_TIN; /* some homemade tins go bad */
    }

    if (obj->corpsenm == PM_GIANT_TURTLE && !obj->cursed) {
        /* Giant turtles are always endangeredelicious! */
        r = DELICIOUS_SOUP_TIN;
    }

    if (r == ROTTEN_TIN && nonrotting_corpse(obj->corpsenm)) {
        r = HOMEMADE_TIN; /* lizards don't rot */
    }

    return r;
}

/* called during each move whilst opening a tin */
static int
opentin()
{
    register int r;
    const char *what;
    int which;

    /* perhaps it was stolen (although that should cause interruption) */
    if (!carried(tin.tin) &&
        (!obj_here(tin.tin, u.ux, u.uy) || !can_reach_floor(TRUE))) {
        return(0); /* %% probably we should use tinoid */
    }
    if (tin.usedtime++ >= 50) {
        You("give up your attempt to open the tin.");
        return(0);
    }
    if (tin.usedtime < tin.reqtime) {
        return 1; /* still busy */
    }

    consume_tin("You succeed in opening the tin.");
    return 0;
}

static void
consume_tin(mesg)
const char *mesg;
{
    const char *what;
    int mnum, which, r;

    r = tin_variety(tin.tin, FALSE);
    if (tin.tin->otrapped || (tin.tin->cursed && r != HOMEMADE_TIN && !rn2(8))) {
        b_trapped("tin", 0);
        tin.tin = costly_tin(COST_DSTROY);
        goto use_up_tin;
    }

    pline("%s", mesg); /* "You succeed in opening the tin." */

    if (r != SPINACH_TIN) {
        mnum = tin.tin->corpsenm;
        if (mnum == NON_PM) {
            pline("It turns out to be empty.");
            tin.tin->dknown = tin.tin->known = TRUE;
            tin.tin = costly_tin(COST_OPEN);
            goto use_up_tin;
        }

        which = 0;  /* 0=>plural, 1=>as-is, 2=>"the" prefix */
        if ((mnum == PM_COCKATRICE || mnum == PM_CHICKATRICE) && (Stone_resistance || Hallucination)) {
            what = "chicken";
            which = 1; /* suppress pluralization */
        } else if (Hallucination) {
            what = rndmonnam();
        } else {
            what = mons[mnum].mname;
            if (the_unique_pm(&mons[mnum])) {
                which = 2;
            } else if (type_is_pname(&mons[mnum])) {
                which = 1;
            }
        }
        if (which == 0) {
            what = makeplural(what);
        } else if (which == 2) {
            what = the(what);
        }
        pline("It smells like %s.", what);
        if (yn("Eat it?") == 'n') {
            if (!Hallucination) tin.tin->dknown = tin.tin->known = TRUE;
            if (flags.verbose) You("discard the open tin.");
            tin.tin = costly_tin(COST_OPEN);
            goto use_up_tin;
        }

        /* in case stop_occupation() was called on previous meal */
        victual.piece = (struct obj *)0;
        victual.o_id = 0;
        victual.fullwarn = victual.eating = victual.doreset = FALSE;

        You("consume %s %s.", tintxts[r].txt, mons[mnum].mname);

        /* SZECHUAN_TIN might have free fortune cookie */
        if (r == SZECHUAN_TIN && rn2(2)) {
            struct obj* cookie = mksobj(FORTUNE_COOKIE, FALSE, FALSE);
            cookie->blessed = tin.tin->blessed;
            cookie->cursed = tin.tin->cursed;
#ifdef INVISIBLE_OBJECTS
            cookie->oinvis = tin.tin->oinvis;
#endif
            There("is a free fortune cookie inside!");
            (void)hold_another_object(cookie,
                                      "It falls to the floor.", 0, 0);
        }

        /* KMH, conduct */
        /* Eating a tin of monstermeat breaks food-conducts here */
        if (!vegetarian(&mons[tin.tin->corpsenm]))
            violated(CONDUCT_VEGETARIAN);
        else if (!vegan(&mons[tin.tin->corpsenm]))
            violated(CONDUCT_VEGAN);
        else
            violated(CONDUCT_FOODLESS);

        tin.tin->dknown = tin.tin->known = TRUE;
        cprefx(mnum);
        cpostfx(mnum);

        /* charge for one at pre-eating cost */
        tin.tin = costly_tin(COST_OPEN);

        /* check for vomiting added by GAN 01/16/87 */
        if(tintxts[r].nut < 0) make_vomiting((long)rn1(15, 10), FALSE);
        else lesshungry(tintxts[r].nut);

        if (r == 0 || r == FRENCH_FRIED_TIN) {
            /* Assume !Glib, because you can't open tins when Glib. */
            make_glib(rn1(11, 5)); /* 5..15 */
            pline("Eating deep fried food made your %s very slippery.",
                  fingers_or_gloves(TRUE));
        }
    } else {
        if (tin.tin->cursed) {
            pline("It contains some decaying%s%s substance.",
                  Blind ? "" : " ", Blind ? "" : hcolor(NH_GREEN));
        } else {
            pline("It contains spinach.");
            tin.tin->dknown = tin.tin->known = 1;
        }

        if (yn("Eat it?") == 'n') {
            if (flags.verbose)
                You("discard the open tin.");
            tin.tin = costly_tin(COST_OPEN);
            goto use_up_tin;
        }

        /*
         * Same order as with non-spinach above:
         * conduct update, side-effects, shop handling, and nutrition.
         */
        /* Eating a tin of spinach breaks foodless-conduct here */
        violated(CONDUCT_FOODLESS);
        if (!tin.tin->cursed)
            pline("This makes you feel like %s!",
                  /* "Swee'pea" is a character from the Popeye cartoons */
                  Hallucination ? "Swee'pea" :
                  /* "feel like Popeye" unless sustain ability suppresses
                     any attribute change; this slightly oversimplifies
                     things:  we want "Popeye" if no strength increase
                     occurs due to already being at maximum, but we won't
                     get it if at-maximum and fixed-abil both apply */
                  !Fixed_abil ? "Popeye" :
                  /* no gain, feel like another character from Popeye */
                  (flags.female ? "Olive Oyl" : "Bluto"));
        gainstr(tin.tin, 0);

        tin.tin = costly_tin(COST_OPEN);
        lesshungry(tin.tin->blessed ? 600 :              /* blessed */
                   !tin.tin->cursed ? (400 + rnd(200)) : /* uncursed */
                   (200 + rnd(400)));                    /* cursed */
    }
use_up_tin:
    if (carried(tin.tin)) useup(tin.tin);
    else useupf(tin.tin, 1L);
    tin.tin = (struct obj *) 0;
    tin.o_id = 0;
}

/* called when starting to open a tin */
STATIC_OVL void
start_tin(otmp)
register struct obj *otmp;
{
    const char *mesg = 0;
    register int tmp;

    if (metallivorous(youmonst.data)) {
        mesg = "You bite right into the metal tin...";
        tmp = 1;
    } else if (cant_wield(youmonst.data)) {
        You("cannot handle the tin properly to open it.");
        return;
    } else if (otmp->blessed) {
        mesg = "The tin opens like magic!";
        tmp = 0;
    } else if (uwep) {
        switch (uwep->otyp) {
        case TIN_OPENER:
            mesg = "You easily open the tin."; /* iff tmp==0 */
            tmp = rn2(uwep->cursed ? 3 : !uwep->blessed ? 2 : 1);
            break;
        case DAGGER:
        case SILVER_DAGGER:
        case ELVEN_DAGGER:
        case ORCISH_DAGGER:
        case ATHAME:
        case KNIFE:
        case STILETTO:
        case CRYSKNIFE:
            tmp = 3;
            break;
        case PICK_AXE:
        case AXE:
            tmp = 6;
            break;
        default:
            goto no_opener;
        }
        pline("Using %s you try to open the tin.", yobjnam(uwep, NULL));
    } else {
no_opener:
        pline("It is not so easy to open this tin.");
        if (Glib) {
            pline_The("tin slips from your %s.", fingers_or_gloves(FALSE));
            if (otmp->quan > 1L) {
                otmp = splitobj(otmp, 1L);
            }
            if (carried(otmp)) dropx(otmp);
            else stackobj(otmp);
            return;
        }
        tmp = rn1(1 + 500/((int)(ACURR(A_DEX) + ACURRSTR)), 10);
    }
    tin.tin = otmp;
    tin.o_id = otmp->o_id;
    if (!tmp) {
        consume_tin(mesg); /* begin immediately */
    } else {
        tin.reqtime = tmp;
        tin.usedtime = 0;
        set_occupation(opentin, "opening the tin", 0);
    }
    return;
}

/* called when waking up after fainting */
int
Hear_again()
{
    flags.soundok = 1;
    flags.botl = TRUE;
    return 0;
}

/* called on the "first bite" of rotten food */
STATIC_OVL int
rottenfood(obj)
struct obj *obj;
{
    pline("Blecch!  Rotten %s!",
          maybe_polyd(is_vampire(youmonst.data),
                      Race_if(PM_VAMPIRE)) ? "blood" : foodword(obj));
    if (!rn2(4)) {
        if (Hallucination) You_feel("rather trippy.");
        else You_feel("rather %s.", body_part(LIGHT_HEADED));
        make_confused(HConfusion + d(2, 4), FALSE);
    } else if (!rn2(4) && !Blind) {
        pline("Everything suddenly goes dark.");
        /* hero is not Blind, but Blinded timer might be nonzero if
           blindness is being overridden by the Eyes of the Overworld */
        make_blinded((long)d(2, 10), FALSE);
        if (!Blind) Your("%s", vision_clears);
    } else if (!rn2(3)) {
        const char *what, *where;
        int duration = rnd(10);

        if (!Blind)
            what = "goes",  where = "dark";
        else if (Levitation || Is_airlevel(&u.uz) ||
                 Is_waterlevel(&u.uz))
            what = "you lose control of",  where = "yourself";
        else
            what = "you slap against the", where =
#ifdef STEED
                (u.usteed) ? "saddle" :
#endif
                surface(u.ux, u.uy);
        pline_The("world spins and %s %s.", what, where);
        flags.soundok = 0;
        nomul(-duration, "unconscious from rotten food");
        nomovemsg = "You are conscious again.";
        afternmv = Hear_again;
        return(1);
    }
    return(0);
}

/* called when a corpse is selected as food */
STATIC_OVL int
eatcorpse(otmp)
struct obj *otmp;
{
    int tp = 0, mnum = otmp->corpsenm;
    long rotted = 0L;
    boolean uniq = !!(mons[mnum].geno & G_UNIQ);
    int retcode = 0;
    boolean stoneable = (flesh_petrifies(&mons[mnum]) && !Stone_resistance &&
                         !poly_when_stoned(youmonst.data));
    boolean slimeable = (mnum == PM_GREEN_SLIME && !Slimed && !Unchanging &&
                         !slimeproof(youmonst.data));

    /* KMH, conduct */
    /* eating a corpse breaks food-conducts here */
    if (!vegetarian(&mons[mnum])) violated(CONDUCT_VEGETARIAN);
    else if (!vegan(&mons[mnum])) violated(CONDUCT_VEGAN);
    else violated(CONDUCT_FOODLESS);

    if (!nonrotting_corpse(mnum)) {
        long age = peek_at_iced_corpse_age(otmp);

        rotted = (monstermoves - age)/(10L + rn2(20));
        if (otmp->cursed) rotted += 2L;
        else if (otmp->blessed) rotted -= 2L;
    }

    /* Vampires only drink the blood of very young, meaty corpses
     * is_edible only allows meaty corpses here
     * Blood is assumed to be 1/5 of the nutrition
     * Thus happens before the conduct checks intentionally - should it be after?
     * Blood is assumed to be meat and flesh.
     */
    if (is_vampiric(youmonst.data)) {
        /* oeaten is set up by touchfood */
        if (otmp->odrained ? otmp->oeaten <= drainlevel(otmp) :
            otmp->oeaten < mons[otmp->corpsenm].cnutrit) {
            pline("There is no blood left in this corpse!");
            return 3;
        } else if (rotted <= 0 &&
                   (peek_at_iced_corpse_age(otmp) + 5) >= monstermoves) {
            char *buf = corpse_xname(otmp, NULL, CXN_PFX_THE);
            pline("You drain the blood from %s.", buf);
            otmp->odrained = 1;
        } else {
            pline("The blood in this corpse has coagulated!");
            return 3;
        }
    }
    else
        otmp->odrained = 0;

    if (mnum != PM_ACID_BLOB && !stoneable && !slimeable && rotted > 5L) {
        boolean cannibal = maybe_cannibal(mnum, FALSE);

        pline("Ulch!  That %s was tainted%s!",
              mons[mnum].mlet == S_FUNGUS ? "fungoid vegetation" :
              !vegetarian(&mons[mnum]) ? "meat" : "protoplasm",
              cannibal ? ", you cannibal" : "");
        if (Sick_resistance) {
            pline("It doesn't seem at all sickening, though...");
        } else {
            long sick_time;

            sick_time = (long) rn1(10, 10);
            /* make sure new ill doesn't result in improvement */
            if (Sick && (sick_time > Sick))
                sick_time = (Sick > 1L) ? Sick - 1L : 1L;
            make_sick(sick_time, corpse_xname(otmp, "rotted", CXN_NORMAL),
                      TRUE, SICK_VOMITABLE);

            pline("(It must have died too long ago to be safe to eat.)");
        }
        if (carried(otmp)) useup(otmp);
        else useupf(otmp, 1L);
        return(2);
    } else if (acidic(&mons[mnum]) && !Acid_resistance) {
        tp++;
        You("have a very bad case of stomach acid."); /* not body_part() */
        losehp(rnd(15), "acidic corpse", KILLED_BY_AN);
    } else if (poisonous(&mons[mnum]) && rn2(5)) {
        tp++;
        pline("Ecch - that must have been poisonous!");
        if (!Poison_resistance) {
            losestr(rnd(4));
            losehp(rnd(15), "poisonous corpse", KILLED_BY_AN);
        } else You("seem unaffected by the poison.");
        /* now any corpse left too long will make you mildly ill */
    } else if ((rotted > 5L || (rotted > 3L && rn2(5))) && !Sick_resistance) {
        tp++;
        You_feel("%ssick.", (Sick) ? "very " : "");
        losehp(rnd(8), "cadaver", KILLED_BY_AN);
    }

    /* delay is weight dependent */
    victual.reqtime = 3 + (mons[mnum].cwt >> 6);
    if (otmp->odrained) victual.reqtime = rounddiv(victual.reqtime, 5);

    if (!tp && !nonrotting_corpse(mnum) && (otmp->orotten || !rn2(7))) {
        if (rottenfood(otmp)) {
            otmp->orotten = TRUE;
            (void)touchfood(otmp);
            retcode = 1;
        }

        if (!mons[otmp->corpsenm].cnutrit) {
            /* no nutrition: rots away, no message if you passed out */
            if (!retcode) pline_The("corpse rots away completely.");
            if (carried(otmp)) useup(otmp);
            else useupf(otmp, 1L);
            retcode = 2;
        }

        if (!retcode) consume_oeaten(otmp, 2);  /* oeaten >>= 2 */
        if (retcode<2 && otmp->odrained && otmp->oeaten < drainlevel(otmp))
            otmp->oeaten = drainlevel(otmp);
    } else if ((mnum == PM_COCKATRICE || mnum == PM_CHICKATRICE) && (Stone_resistance || Hallucination)) {
        pline("This tastes just like chicken!");
    } else if (mnum == PM_FLOATING_EYE && u.umonnum == PM_RAVEN) {
        You("peck the eyeball with delight.");
    } else if (!is_vampiric(youmonst.data)) {
        /* special messages for certain corpses */
        if (is_rat(&mons[mnum]) && is_dwarf(youmonst.data)) {
            pline("This %s is delicious!", food_xname(otmp, FALSE));
        } else if (is_longworm(&mons[mnum])) {
            pline("This %s is spicy!", food_xname(otmp, FALSE));
        } else {
            pline("%s%s %s!",
                  !uniq ? "This " : !type_is_pname(&mons[mnum]) ? "The " : "",
                  food_xname(otmp, FALSE),
                  (vegan(&mons[mnum]) ?
                   (!carnivorous(youmonst.data) && herbivorous(youmonst.data)) :
                   (carnivorous(youmonst.data) && !herbivorous(youmonst.data)))
                  ? "is delicious" : "tastes terrible");
        }
    }

    /* Eating slimy or oily corpses makes your fingers slippery.
       Note: Snakes are not slimy. */
    if ((amorphous(&mons[mnum]) || slithy(&mons[mnum]) ||
         mons[mnum].mlet == S_BLOB) &&
        mons[mnum].mlet != S_SNAKE && mons[mnum].mlet != S_NAGA &&
        mons[mnum].mlet != S_MIMIC && rnf(1, 5) &&
        !is_vampiric(youmonst.data)) {
        pline("Eating this %s corpse makes your %s %s slippery.",
              amorphous(&mons[mnum]) ? "glibbery" : "slimy",
              makeplural(body_part(FINGER)),
              Glib ? "even more" : "very");
        incr_itimeout(&Glib, rnd(15));
    }

    return(retcode);
}

/* called as you start to eat */
static void
start_eating(otmp, already_partly_eaten)
struct obj *otmp;
boolean already_partly_eaten;
{
#ifdef DEBUG
    debugpline("start_eating: %lx (victual = %lx)", otmp, victual.piece);
    debugpline("reqtime = %d", victual.reqtime);
    debugpline("(original reqtime = %d)", objects[otmp->otyp].oc_delay);
    debugpline("nmod = %d", victual.nmod);
    debugpline("oeaten = %d", otmp->oeaten);
#endif
    victual.fullwarn = victual.doreset = FALSE;
    victual.eating = TRUE;

    if (otmp->otyp == CORPSE) {
        cprefx(victual.piece->corpsenm);
        if (!victual.piece || !victual.eating) {
            /* rider revived, or died and lifesaved */
            return;
        }
    }

    const char *old_nomovemsg = nomovemsg;
    if (bite()) {
        /* survived choking, finish off food that's nearly done;
           need this to handle cockatrice eggs, fortune cookies, etc */
        if (++victual.usedtime >= victual.reqtime) {
            /* don't want done_eating() to issue nomovemsg if it
               is due to vomit() called by bite() */
            const char *save_nomovemsg = nomovemsg;
            if (!old_nomovemsg) {
                nomovemsg = 0;
            }
            done_eating(FALSE);
            if (!old_nomovemsg) {
                nomovemsg = save_nomovemsg;
            }
        }
        return;
    }

    if (++victual.usedtime >= victual.reqtime) {
        /* print "finish eating" message if they just resumed -dlc */
        done_eating((victual.reqtime > 1 || already_partly_eaten) ? TRUE : FALSE);
        return;
    }

    Sprintf(msgbuf, "%s %s", otmp->odrained ? "draining" : "eating",
            food_xname(otmp, TRUE));
    set_occupation(eatfood, msgbuf, 0);
}


/*
 * Called on "first bite" of (non-corpse) food, after touchfood() has
 * marked it 'partly eaten'.  Used for non-rotten non-tin non-corpse food.
 * Messages should use present tense since multi-turn food won't be
 * finishing at the time they're issued.
 */
STATIC_OVL void
fprefx(otmp)
struct obj *otmp;
{
    switch (otmp->otyp) {
    case FOOD_RATION:
        /* nutrition == 800 */
        if (u.uhunger <= 200) {
            /* 200+800 remains below 1000+1, the satiation threshold */
            pline(Hallucination ? "Oh wow, like, superior, man!" :
                                  "That food really hit the spot!");
        } else if (u.uhunger <= 700) {
            /* 700-1+800 remains below 1500, the choking threshold which
            triggers "you're having a hard time getting it down" feedback */
            pline("That satiated your %s!", body_part(STOMACH));
        }
        /* [satiation message may be inaccurate if eating gets interrupted] */
        break;

    case TRIPE_RATION:
        if (carnivorous(youmonst.data) && !humanoid(youmonst.data))
            pline("That tripe ration was surprisingly good!");
        else if (maybe_polyd(is_orc(youmonst.data), Race_if(PM_ORC)))
            pline(Hallucination ? "Tastes great!  Less filling!" :
                  "Mmm, tripe... not bad!");
        else {
            pline("Yak - dog food!");
#ifdef CONVICT
            if (Role_if(PM_CONVICT))
                pline("At least it's not prison food.");
#endif /* CONVICT */
            more_experienced(1, 1, 0);
            newexplevel();
            /* not cannibalism, but we use similar criteria
               for deciding whether to be sickened by this meal */
            if (rn2(2) && !CANNIBAL_ALLOWED())
#ifdef CONVICT
                if (!Role_if(PM_CONVICT))
#endif /* CONVICT */
                make_vomiting((long)rn1(victual.reqtime, 14), FALSE);
        }
        break;

    case LEMBAS_WAFER:
        if (maybe_polyd(is_orc(youmonst.data), Race_if(PM_ORC))) {
            pline("%s", "!#?&* elf kibble!");
            break;
        } else if (maybe_polyd(is_elf(youmonst.data), Race_if(PM_ELF))) {
            pline("A little goes a long way.");
            break;
        }
        goto give_feedback;

    case MEATBALL:
    case MEAT_STICK:
    case HUGE_CHUNK_OF_MEAT:
    case MEAT_RING:
        goto give_feedback;
    /* break; */
    case CLOVE_OF_GARLIC:
        if (is_undead(youmonst.data)) {
            make_vomiting((long)rn1(victual.reqtime, 5), FALSE);
            break;
        }
        /* fall through */

    default:
        if (otmp->otyp==SLIME_MOLD && !otmp->cursed
            && otmp->spe == current_fruit)
            pline("My, that was a %s %s!",
                  Hallucination ? "primo" : "yummy",
                  singular(otmp, xname));
        else
#ifdef UNIX
        if (otmp->otyp == APPLE || otmp->otyp == PEAR) {
            if (!Hallucination) pline("Core dumped.");
            else {
/* This is based on an old Usenet joke, a fake a.out manual page */
                int x = rnd(100);
                if (x <= 75)
                    pline("Segmentation fault -- core dumped.");
                else if (x <= 99)
                    pline("Bus error -- core dumped.");
                else pline("Yo' mama -- core dumped.");
            }
        } else
#endif
#ifdef MAC  /* KMH -- Why should Unix have all the fun? */
        if (otmp->otyp == APPLE) {
            pline("Delicious!  Must be a Macintosh!");
        } else
#endif
        if (otmp->otyp == EGG && stale_egg(otmp)) {
            pline("Ugh.  Rotten egg."); /* perhaps others like it */
#ifdef CONVICT
            if (Role_if(PM_CONVICT) && (rn2(8) > u.ulevel)) {
                You_feel("a slight stomach ache."); /* prisoners are used to bad food */
            } else
#endif /* CONVICT */
            /* increasing existing nausea means that it will take longer
               before eventual vomit, but also means that constitution
               will be abused more times before illness completes */
            make_vomiting(Vomiting+d(10, 4), TRUE);
        } else
give_feedback:
            pline("This %s is %s", singular(otmp, xname),
                  otmp->cursed ? (Hallucination ? "grody!" : "terrible!") :
                  (otmp->otyp == CRAM_RATION
                   || otmp->otyp == K_RATION
                   || otmp->otyp == C_RATION)
                  ? "bland." :
                  Hallucination ? "gnarly!" : "delicious!");
        break;
    }
}

/* increment a combat intrinsic with limits on its growth */
static int
bounded_increase(old, inc, typ)
int old, inc, typ;
{
    int absold, absinc, sgnold, sgninc;

    /* don't include any amount coming from worn rings (caller handles
       'protection' differently) */
    if (uright && uright->otyp == typ && typ != RIN_PROTECTION) {
        old -= uright->spe;
    }
    if (uleft && uleft->otyp == typ && typ != RIN_PROTECTION) {
        old -= uleft->spe;
    }
    absold = abs(old), absinc = abs(inc);
    sgnold = sgn(old), sgninc = sgn(inc);

    if (absinc == 0 || sgnold != sgninc || absold + absinc < 10) {
        ; /* use inc as-is */
    } else if (absold + absinc < 20) {
        absinc = rnd(absinc); /* 1..n */
        if (absold + absinc < 10) {
            absinc = 10 - absold;
        }
        inc = sgninc * absinc;
    } else if (absold + absinc < 40) {
        absinc = rn2(absinc) ? 1 : 0;
        if (absold + absinc < 20) {
            absinc = rnd(20 - absold);
        }
        inc = sgninc * absinc;
    } else {
        inc = 0; /* no further increase allowed via this method */
    }
    /* put amount from worn rings back */
    if (uright && uright->otyp == typ && typ != RIN_PROTECTION) {
        old += uright->spe;
    }
    if (uleft && uleft->otyp == typ && typ != RIN_PROTECTION) {
        old += uleft->spe;
    }
    return old + inc;
}

STATIC_OVL void
accessory_has_effect(otmp)
struct obj *otmp;
{
    pline("Magic spreads through your body as you digest the %s.",
          otmp->oclass == RING_CLASS ? "ring" : "amulet");
}

STATIC_OVL void
eataccessory(otmp)
struct obj *otmp;
{
    int typ = otmp->otyp;
    long oldprop;

    /* Note: rings are not so common that this is unbalancing. */
    /* (How often do you even _find_ 3 rings of polymorph in a game?) */
    oldprop = u.uprops[objects[typ].oc_oprop].intrinsic;
    if (otmp == uleft || otmp == uright) {
        Ring_gone(otmp);
        if (u.uhp <= 0) return; /* died from sink fall */
    }
    otmp->known = otmp->dknown = 1; /* by taste */
    if (!rn2(otmp->oclass == RING_CLASS ? 3 : 5)) {
        switch (otmp->otyp) {
        default:
            if (!objects[typ].oc_oprop) break; /* should never happen */

            if (!(u.uprops[objects[typ].oc_oprop].intrinsic & FROMOUTSIDE))
                accessory_has_effect(otmp);

            u.uprops[objects[typ].oc_oprop].intrinsic |= FROMOUTSIDE;

            switch (typ) {
            case RIN_SEE_INVISIBLE:
                set_mimic_blocking();
                see_monsters();
                if (Invis && !oldprop && !ESee_invisible &&
                    !perceives(youmonst.data) && !Blind) {
                    newsym(u.ux, u.uy);
                    pline("Suddenly you can see yourself.");
                    makeknown(typ);
                }
                break;
            case RIN_INVISIBILITY:
                if (!oldprop && !EInvis && !BInvis &&
                    !See_invisible && !Blind) {
                    newsym(u.ux, u.uy);
                    Your("body takes on a %s transparency...",
                         Hallucination ? "normal" : "strange");
                    makeknown(typ);
                }
                break;
            case RIN_PROTECTION_FROM_SHAPE_CHAN:
                rescham();
                break;
            case RIN_LEVITATION:
                /* undo the `.intrinsic |= FROMOUTSIDE' done above */
                u.uprops[LEVITATION].intrinsic = oldprop;
                if (!Levitation) {
                    float_up();
                    incr_itimeout(&HLevitation, d(10, 20));
                    makeknown(typ);
                }
                break;
            }
            break;
        case RIN_ADORNMENT:
            accessory_has_effect(otmp);
            if (adjattrib(A_CHA, otmp->spe, -1))
                makeknown(typ);
            break;
        case RIN_GAIN_STRENGTH:
            accessory_has_effect(otmp);
            if (adjattrib(A_STR, otmp->spe, -1))
                makeknown(typ);
            break;
        case RIN_GAIN_CONSTITUTION:
            accessory_has_effect(otmp);
            if (adjattrib(A_CON, otmp->spe, -1))
                makeknown(typ);
            break;
        case RIN_GAIN_INTELLIGENCE:
            accessory_has_effect(otmp);
            if (adjattrib(A_INT, otmp->spe, -1))
                makeknown(typ);
            break;
        case RIN_GAIN_WISDOM:
            accessory_has_effect(otmp);
            if (adjattrib(A_WIS, otmp->spe, -1))
                makeknown(typ);
            break;
        case RIN_GAIN_DEXTERITY:
            accessory_has_effect(otmp);
            if (adjattrib(A_DEX, otmp->spe, -1))
                makeknown(typ);
            break;
        case RIN_INCREASE_ACCURACY:
            accessory_has_effect(otmp);
            u.uhitinc += otmp->spe;
            break;
        case RIN_INCREASE_DAMAGE:
            accessory_has_effect(otmp);
            u.udaminc += otmp->spe;
            break;
        case RIN_PROTECTION:
            accessory_has_effect(otmp);
            HProtection |= FROMOUTSIDE;
            u.ublessed = bounded_increase(u.ublessed, otmp->spe, RIN_PROTECTION);
            flags.botl = 1;
            break;
        case RIN_FREE_ACTION:
            /* Give sleep resistance instead */
            if (!(HSleep_resistance & FROMOUTSIDE))
                accessory_has_effect(otmp);
            if (!Sleep_resistance)
                You_feel("wide awake.");
            HSleep_resistance |= FROMOUTSIDE;
            break;
        case AMULET_OF_CHANGE:
            accessory_has_effect(otmp);
            makeknown(typ);
            change_sex();
            You("are suddenly very %s!",
                flags.female ? "feminine" : "masculine");
            flags.botl = 1;
            break;
        case AMULET_OF_UNCHANGING:
            /* un-change: it's a pun */
            if (!Unchanging && Upolyd) {
                accessory_has_effect(otmp);
                makeknown(typ);
                rehumanize();
            }
            break;
        case AMULET_OF_STRANGULATION: /* bad idea! */
            /* no message--this gives no permanent effect */
            choke(otmp);
            break;
        case AMULET_OF_RESTFUL_SLEEP: { /* another bad idea! */
            long newnap = (long) rnd(100), oldnap = (HSleeping & TIMEOUT);

            if (!(HSleeping & FROMOUTSIDE)) {
                accessory_has_effect(otmp);
            }
            HSleeping |= FROMOUTSIDE;
            /* might also be wearing one; use shorter of two timeouts */
            if (newnap < oldnap || oldnap == 0L) {
                HSleeping = (HSleeping & ~TIMEOUT) | newnap;
            }
            break;
        }

        case RIN_SUSTAIN_ABILITY:
        case AMULET_OF_FLYING:
        case AMULET_OF_LIFE_SAVING:
        case AMULET_OF_REFLECTION: /* nice try */
            /* can't eat Amulet of Yendor or fakes,
             * and no oc_prop even if you could -3.
             */
            break;
        }
    }
}
/* called after eating non-food */
STATIC_OVL void
eatspecial()
{
    struct obj *otmp = victual.piece;

    /* lesshungry wants an occupation to handle choke messages correctly */
    set_occupation(eatfood, "eating non-food", 0);
    lesshungry(victual.nmod);
    occupation = 0;
    victual.piece = (struct obj *)0;
    victual.o_id = 0;
    victual.eating = 0;
    if (otmp->oclass == COIN_CLASS) {
        if (carried(otmp))
            useupall(otmp);
        else
            useupf(otmp, otmp->quan);
        vault_gd_watching(GD_EATGOLD);
        return;
    }
    if (objects[otmp->otyp].oc_material == PAPER) {
#ifdef MAIL
        if (otmp->otyp == SCR_MAIL) {
            /* no nutrition */
            pline("This junk mail is less than satisfying.");
        }
#endif
        if (otmp->otyp == SCR_SCARE_MONSTER) {
            /* to eat scroll, hero is currently polymorphed into a monster */
            pline("Yuck%c", otmp->blessed ? '!' : '.');
        } else if (otmp->oclass == SCROLL_CLASS &&
                /* check description after checking for specific scrolls */
                !strcmpi(OBJ_DESCR(objects[otmp->otyp]), "YUM YUM")) {
            pline("Yum%c", otmp->blessed ? '!' : '.');
        } else {
            pline("Needs salt...");
        }
    }
    if (otmp->oclass == POTION_CLASS) {
        otmp->quan++; /* dopotion() does a useup() */
        (void)dopotion(otmp);
    }
    if (otmp->oclass == RING_CLASS || otmp->oclass == AMULET_CLASS)
        eataccessory(otmp);
    else if (otmp->otyp == LEASH && otmp->leashmon)
        o_unleash(otmp);

    /* KMH -- idea by "Tommy the Terrorist" */
    if ((otmp->otyp == TRIDENT) && !otmp->cursed)
    {
        /* sugarless chewing gum which used to be heavily advertised on TV */
        pline(Hallucination ? "Four out of five dentists agree." :
              "That was pure chewing satisfaction!");
        exercise(A_WIS, TRUE);
    }
    if ((otmp->otyp == FLINT) && !otmp->cursed)
    {
        /* chewable vitamin for kids based on "The Flintstones" TV cartoon */
        pline("Yabba-dabba delicious!");
        exercise(A_CON, TRUE);
    }

    if (otmp == uwep && otmp->quan == 1L) uwepgone();
    if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
    if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();

    if (otmp == uball) unpunish();
    if (otmp == uchain) unpunish(); /* but no useup() */
    else if (carried(otmp)) useup(otmp);
    else useupf(otmp, 1L);
}

/* NOTE: the order of these words exactly corresponds to the
   order of oc_material values #define'd in objclass.h. */
static const char *foodwords[] = {
    "meal", "liquid", "wax", "food", "meat",
    "paper", "cloth", "leather", "wood", "bone", "scale",
    "metal", "metal", "metal", "silver", "gold", "platinum", "mithril",
    "plastic", "glass", "rich food", "stone"
};

STATIC_OVL const char *
foodword(otmp)
register struct obj *otmp;
{
    if (otmp->oclass == FOOD_CLASS) return "food";
    if (otmp->oclass == GEM_CLASS &&
        objects[otmp->otyp].oc_material == GLASS &&
        otmp->dknown)
        makeknown(otmp->otyp);
    return foodwords[objects[otmp->otyp].oc_material];
}

/* called after consuming (non-corpse) food */
STATIC_OVL void
fpostfx(otmp)
struct obj *otmp;
{
    switch (otmp->otyp) {
    case SPRIG_OF_WOLFSBANE:
        if (u.ulycn >= LOW_PM || is_were(youmonst.data))
            you_unwere(TRUE);
        break;

    case CARROT:
        if (!u.uswallow || !attacktype_fordmg(u.ustuck->data, AT_ENGL, AD_BLND)) {
            make_blinded((long)u.ucreamed, TRUE);
        }
        break;

    case FORTUNE_COOKIE:
        if (u.roleplay.illiterate) {
            pline("This cookie has a scrap of paper inside.");
            pline("What a pity that you cannot read!");
        } else {
            outrumor(bcsign(otmp), BY_COOKIE);
            if (!Blind) violated(CONDUCT_ILLITERACY);
        }
        break;
    case LUMP_OF_ROYAL_JELLY:
        /* This stuff seems to be VERY healthy! */
        gainstr(otmp, 1);
        if (Upolyd) {
            u.mh += otmp->cursed ? -rnd(20) : rnd(20);
            if (u.mh > u.mhmax) {
                if (!rn2(17)) u.mhmax++;
                u.mh = u.mhmax;
            } else if (u.mh <= 0) {
                rehumanize();
            }
        } else {
            u.uhp += otmp->cursed ? -rnd(20) : rnd(20);
            if (u.uhp > u.uhpmax) {
                if(!rn2(17)) u.uhpmax++;
                u.uhp = u.uhpmax;
            } else if (u.uhp <= 0) {
                killer.format = KILLED_BY_AN;
                Strcpy(killer.name, "rotten lump of royal jelly");
                done(POISONING);
            }
        }
        if (!otmp->cursed) {
            heal_legs(0);
        }
        break;

    case EGG:
        if (flesh_petrifies(&mons[otmp->corpsenm])) {
            if (!Stone_resistance && !(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))) {
                if (!Stoned) {
                    Sprintf(killer.name, "%s egg", mons[otmp->corpsenm].mname);
                    make_stoned(5L, (char *) 0, KILLED_BY_AN, killer.name);
                }
            }
        }
        break;

    case EUCALYPTUS_LEAF:
        if (Sick && !otmp->cursed)
            make_sick(0L, (char *)0, TRUE, SICK_ALL);
        if (Vomiting && !otmp->cursed)
            make_vomiting(0L, TRUE);
        break;

    case APPLE:
        if (otmp->cursed && !Sleep_resistance) {
            /* Snow White; 'poisoned' applies to [a subset of] weapons,
               not food, so we substitute cursed; fortunately our hero
               won't have to wait for a prince to be rescued/revived */
            if (Race_if(PM_DWARF) && Hallucination) {
                verbalize("Heigh-ho, ho-hum, I think I'll skip work today.");
            } else if (Deaf || !flags.acoustics) {
                You("fall asleep.");
            } else {
                You_hear("sinister laughter as you fall asleep...");
            }
            fall_asleep(-rn1(11, 20), TRUE);
        }
        break;
    }
}

/*
 * return 0 if the food was not dangerous.
 * return 1 if the food was dangerous and you chose to stop.
 * return 2 if the food was dangerous and you chose to eat it anyway.
 */
STATIC_OVL int
edibility_prompts(otmp)
struct obj *otmp;
{
    /* blessed food detection granted you a one-use
       ability to detect food that is unfit for consumption
       or dangerous and avoid it. */

    char buf[BUFSZ], foodsmell[BUFSZ],
         it_or_they[QBUFSZ], eat_it_anyway[QBUFSZ];
    boolean cadaver = (otmp->otyp == CORPSE),
            stoneorslime = FALSE;
    int material = objects[otmp->otyp].oc_material,
        mnum = otmp->corpsenm;
    long rotted = 0L;

    Strcpy(foodsmell, Tobjnam(otmp, "smell"));
    Strcpy(it_or_they, (otmp->quan == 1L) ? "it" : "they");
    Sprintf(eat_it_anyway, "Eat %s anyway?",
            (otmp->quan == 1L) ? "it" : "one");

    if (cadaver || otmp->otyp == EGG || otmp->otyp == TIN) {
        /* These checks must match those in eatcorpse() */
        stoneorslime = (flesh_petrifies(&mons[mnum]) &&
                        !Stone_resistance &&
                        !poly_when_stoned(youmonst.data));

        if (mnum == PM_GREEN_SLIME) {
            stoneorslime = (!Unchanging && !slimeproof(youmonst.data));
        }

        if (cadaver && !nonrotting_corpse(mnum)) {
            long age = peek_at_iced_corpse_age(otmp);

            /* worst case rather than random
               in this calculation to force prompt */
            rotted = (monstermoves - age)/(10L + 0 /* was rn2(20) */);
            if (otmp->cursed) rotted += 2L;
            else if (otmp->blessed) rotted -= 2L;
        }
    }

    /*
     * These problems with food should be checked in
     * order from most detrimental to least detrimental.
     */
    if (cadaver && mnum != PM_ACID_BLOB && rotted > 5L && !Sick_resistance) {
        /* Tainted meat */
        Sprintf(buf, "%s like %s could be tainted! %s",
                foodsmell, it_or_they, eat_it_anyway);
        if (yn_function(buf, ynchars, 'n')=='n') return 1;
        else return 2;
    }
    if (stoneorslime) {
        Sprintf(buf, "%s like %s could be something very dangerous! %s",
                foodsmell, it_or_they, eat_it_anyway);
        if (yn_function(buf, ynchars, 'n')=='n') return 1;
        else return 2;
    }
    if (otmp->orotten || (cadaver && rotted > 3L)) {
        /* Rotten */
        Sprintf(buf, "%s like %s could be rotten! %s",
                foodsmell, it_or_they, eat_it_anyway);
        if (yn_function(buf, ynchars, 'n')=='n') return 1;
        else return 2;
    }
    if (cadaver && poisonous(&mons[mnum]) && !Poison_resistance) {
        /* poisonous */
        Sprintf(buf, "%s like %s might be poisonous! %s",
                foodsmell, it_or_they, eat_it_anyway);
        if (yn_function(buf, ynchars, 'n')=='n') return 1;
        else return 2;
    }
    if (otmp->otyp == APPLE && otmp->cursed && !Sleep_resistance) {
        /* causes sleep, for long enough to be dangerous */
        Sprintf(buf, "%s like %s might have been poisoned.  %s", foodsmell,
                it_or_they, eat_it_anyway);
        return (yn_function(buf, ynchars, 'n') == 'n') ? 1 : 2;
    }
    if (cadaver && !vegetarian(&mons[mnum]) &&
        u.roleplay.vegetarian) {
        Sprintf(buf, "%s unhealthy. %s",
                foodsmell, eat_it_anyway);
        if (yn_function(buf, ynchars, 'n')=='n') return 1;
        else return 2;
    }
    if (cadaver && acidic(&mons[mnum]) && !Acid_resistance) {
        Sprintf(buf, "%s rather acidic. %s",
                foodsmell, eat_it_anyway);
        if (yn_function(buf, ynchars, 'n')=='n') return 1;
        else return 2;
    }
    if (Upolyd && u.umonnum == PM_RUST_MONSTER &&
        is_metallic(otmp) && otmp->oerodeproof) {
        Sprintf(buf, "%s disgusting to you right now. %s",
                foodsmell, eat_it_anyway);
        if (yn_function(buf, ynchars, 'n')=='n') return 1;
        else return 2;
    }

    /*
     * Breaks conduct, but otherwise safe.
     */
    if (u.roleplay.vegan &&
        ((material == LEATHER || material == BONE ||
          material == DRAGON_HIDE || material == WAX) ||
         (cadaver && !vegan(&mons[mnum])))) {
        Sprintf(buf, "%s foul and unfamiliar to you. %s",
                foodsmell, eat_it_anyway);
        if (yn_function(buf, ynchars, 'n')=='n') return 1;
        else return 2;
    }
    if (u.roleplay.vegetarian &&
        ((material == LEATHER || material == BONE ||
          material == DRAGON_HIDE) ||
         (cadaver && !vegetarian(&mons[mnum])))) {
        Sprintf(buf, "%s unfamiliar to you. %s",
                foodsmell, eat_it_anyway);
        if (yn_function(buf, ynchars, 'n')=='n') return 1;
        else return 2;
    }

    if (cadaver && mnum != PM_ACID_BLOB && rotted > 5L && Sick_resistance) {
        /* Tainted meat with Sick_resistance */
        Sprintf(buf, "%s like %s could be tainted! %s",
                foodsmell, it_or_they, eat_it_anyway);
        if (yn_function(buf, ynchars, 'n')=='n') return 1;
        else return 2;
    }
    return 0;
}

/* 'e' command */
int
doeat()
{
    struct obj *otmp;
    int basenutrit; /* nutrition of full item */
    int nutrit;     /* nutrition available */
    boolean already_partly_eaten = FALSE;
    boolean dont_start = FALSE;
    boolean nodelicious = FALSE;

    if (Strangled) {
        pline("If you can't breathe air, how can you consume solids?");
        return 0;
    }
    if (!(otmp = floorfood("eat", 0))) return 0;
    if (check_capacity((char *)0)) return 0;

    if (u.uedibility) {
        int res = edibility_prompts(otmp);
        if (res) {
            Your("%s stops tingling and your sense of smell returns to normal.",
                 body_part(NOSE));
            u.uedibility = 0;
            if (res == 1) return 0;
        }
    }

    /* We have to make non-foods take 1 move to eat, unless we want to
     * do ridiculous amounts of coding to deal with partly eaten plate
     * mails, players who polymorph back to human in the middle of their
     * metallic meal, etc....
     */
    if (!is_edible(otmp)) {
        You("cannot eat that!");
        return 0;
    } else if ((otmp->owornmask & (W_ARMOR|W_TOOL|W_AMUL
#ifdef STEED
                                   |W_SADDLE
#endif
                                   )) != 0) {
        /* let them eat rings */
        You_cant("eat %s you're wearing.", something);
        return 0;
    } else if (!(carried(otmp) ? retouch_object(&otmp, FALSE) : touch_artifact(otmp, &youmonst))) {
        return 1; /* got blasted so use a turn */
    }
    if (is_metallic(otmp) &&
        u.umonnum == PM_RUST_MONSTER && otmp->oerodeproof) {
        otmp->rknown = TRUE;
        if (otmp->quan > 1L) {
            if(!carried(otmp))
                (void) splitobj(otmp, otmp->quan - 1L);
            else
                otmp = splitobj(otmp, 1L);
        }
        pline("Ulch!  That %s was rustproofed!", xname(otmp));
        /* The regurgitated object's rustproofing is gone now */
        otmp->oerodeproof = 0;
        make_stunned((HStun & TIMEOUT) + (long) rn2(10), TRUE);
        /*
         * We don't expect rust monsters to be wielding welded weapons
         * or wearing cursed rings which were rustproofed, but guard
         * against the possibility just in case.
         */
        if (welded(otmp) || (otmp->cursed && (otmp->owornmask & W_RING))) {
            set_bknown(otmp, 1); /* for ring; welded() does this for weapon */
            You("spit out %s.", the(xname(otmp)));
        } else {
            You("spit %s out onto the %s.", the(xname(otmp)),
                surface(u.ux, u.uy));
            if (carried(otmp)) {
                /* no need to check for leash in use; it's not metallic */
                if (otmp->owornmask) {
                    remove_worn_item(otmp, FALSE);
                }
                freeinv(otmp);
                dropy(otmp);
            }
            stackobj(otmp);
        }
        return 1;
    }
    /* KMH -- Slow digestion is... indigestible */
    if (otmp->otyp == RIN_SLOW_DIGESTION) {
        pline("This ring is indigestible!");
        (void) rottenfood(otmp);
        if (otmp->dknown && !objects[otmp->otyp].oc_name_known
            && !objects[otmp->otyp].oc_uname)
            docall(otmp);
        return (1);
    }
    if (otmp->oclass != FOOD_CLASS) {
        int material;

        victual.reqtime = 1;
        victual.piece = otmp;
        victual.o_id = otmp->o_id;
        /* Don't split it, we don't need to if it's 1 move */
        victual.usedtime = 0;
        victual.canchoke = (u.uhs == SATIATED);
        /* Note: gold weighs 1 pt. for each 1000 pieces (see */
        /* pickup.c) so gold and non-gold is consistent. */
        if (otmp->oclass == COIN_CLASS)
            basenutrit = ((otmp->quan > 200000L) ? 2000
                          : (int)(otmp->quan/100L));
        else if(otmp->oclass == BALL_CLASS || otmp->oclass == CHAIN_CLASS)
            basenutrit = weight(otmp);
        /* oc_nutrition is usually weight anyway */
        else basenutrit = objects[otmp->otyp].oc_nutrition;
#ifdef MAIL
        if (otmp->otyp == SCR_MAIL) {
            basenutrit = 0;
            nodelicious = TRUE;
        }
#endif
        victual.nmod = basenutrit;
        victual.eating = TRUE; /* needed for lesshungry() */

        /* Eating non-food-objects breaks food-conducts here */
        material = objects[otmp->otyp].oc_material;
        if (material == LEATHER ||
            material == BONE || material == DRAGON_HIDE) {
            violated(CONDUCT_VEGETARIAN);
        } else if (material == WAX)
            violated(CONDUCT_VEGAN);
        else
            violated(CONDUCT_FOODLESS);

        if (otmp->cursed) {
            (void) rottenfood(otmp);
            nodelicious = TRUE;
        } else if (objects[otmp->otyp].oc_material == PAPER) {
            nodelicious = TRUE;
        }

        if (otmp->oclass == WEAPON_CLASS && otmp->opoisoned) {
            pline("Ecch - that must have been poisonous!");
            if (!Poison_resistance) {
                losestr(rnd(4));
                losehp(rnd(15), xname(otmp), KILLED_BY_AN);
            } else
                You("seem unaffected by the poison.");
        } else if (!nodelicious) {
            pline("%s%s is delicious!",
                  (obj_is_pname(otmp) && otmp->oartifact < ART_HEART_OF_AHRIMAN) ? "" : "This ",
                  (otmp->oclass == COIN_CLASS) ? foodword(otmp) : singular(otmp, xname));
        }

        eatspecial();
        return 1;
    }

    /* [ALI] Hero polymorphed in the meantime.
     */
    if (otmp == victual.piece &&
        is_vampiric(youmonst.data) != otmp->odrained)
        victual.piece = (struct obj *)0;    /* Can't resume */

    /* [ALI] Blood can coagulate during the interruption
     *       but not during the draining process.
     */
    if(otmp == victual.piece && otmp->odrained &&
       (peek_at_iced_corpse_age(otmp) + victual.usedtime + 5) < monstermoves)
        victual.piece = (struct obj *)0;    /* Can't resume */

    if (otmp == victual.piece) {
        /* If they weren't able to choke, they don't suddenly become able to
         * choke just because they were interrupted.  On the other hand, if
         * they were able to choke before, if they lost food it's possible
         * they shouldn't be able to choke now.
         */
        if (u.uhs != SATIATED) victual.canchoke = FALSE;
        victual.o_id = 0;
        victual.piece = touchfood(otmp);
        if (victual.piece) {
            victual.o_id = victual.piece->o_id;
        }
        You("resume your meal.");
        start_eating(victual.piece, FALSE);
        return(1);
    }

    /* nothing in progress - so try to find something. */
    /* tins are a special case */
    /* tins must also check conduct separately in case they're discarded */
    if (otmp->otyp == TIN) {
        start_tin(otmp);
        return(1);
    }

    already_partly_eaten = otmp->oeaten ? TRUE : FALSE;
    victual.o_id = 0;
    victual.piece = otmp = touchfood(otmp);
    if (victual.piece) {
        victual.o_id = victual.piece->o_id;
    }
    victual.usedtime = 0;

    /* Now we need to calculate delay and nutritional info.
     * The base nutrition calculated here and in eatcorpse() accounts
     * for normal vs. rotten food.  The reqtime and nutrit values are
     * then adjusted in accordance with the amount of food left.
     */
    if (otmp->otyp == CORPSE) {
        int tmp = eatcorpse(otmp);

        if (tmp == 3) {
            /* inedible */
            victual.piece = (struct obj *)0;
            /*
             * The combination of odrained == TRUE and oeaten == cnutrit
             * represents the case of starting to drain a corpse but not
             * getting any further (eg., loosing consciousness due to
             * rotten food). We must preserve this case to avoid corpses
             * changing appearance after a failed attempt to eat.
             */
            if (!otmp->odrained &&
                otmp->oeaten == mons[otmp->corpsenm].cnutrit)
                otmp->oeaten = 0;
            /* ALI, conduct: didn't eat it after all */
            u.uconduct.food--;
            return 0;
        } else if (tmp == 2) {
            /* used up */
            victual.piece = (struct obj *)0;
            victual.o_id = 0;
            return(1);
        } else if (tmp)
            dont_start = TRUE;
        /* if not used up, eatcorpse sets up reqtime and may modify
         * oeaten */
    } else {
        /* No checks for WAX, LEATHER, BONE, DRAGON_HIDE.  These are
         * all handled in the != FOOD_CLASS case, above */
        switch (objects[otmp->otyp].oc_material) {
        case FLESH:
            if (otmp->otyp == EGG)
                violated(CONDUCT_VEGAN);
            else
                violated(CONDUCT_VEGETARIAN);
            break;

        default:
            if (otmp->otyp == PANCAKE ||
                otmp->otyp == FORTUNE_COOKIE || /* eggs */
                otmp->otyp == CREAM_PIE ||
                otmp->otyp == CANDY_BAR || /* milk */
                otmp->otyp == LUMP_OF_ROYAL_JELLY)
                violated(CONDUCT_VEGAN);
            else
                violated(CONDUCT_FOODLESS);
            break;
        }

        victual.reqtime = objects[otmp->otyp].oc_delay;
        if (otmp->otyp != FORTUNE_COOKIE &&
            (otmp->cursed ||
             (((monstermoves - otmp->age) > (otmp->blessed ? 50 : 30)) &&
              (otmp->orotten || !rn2(7))))) {

            if (rottenfood(otmp)) {
                otmp->orotten = TRUE;
                dont_start = TRUE;
            }
            if (otmp->oeaten < 2) {
                victual.piece = (struct obj *)0;
                if (carried(otmp)) useup(otmp);
                else useupf(otmp, 1L);
                return 1;
            } else {
                consume_oeaten(otmp, 1); /* oeaten >>= 1 */
            }
        } else if (!already_partly_eaten) {
            fprefx(otmp);
        } else {
            You("%s %s.",
                (victual.reqtime == 1) ? "eat" : "begin eating",
                doname(otmp));
        }
    }

    /* re-calc the nutrition */
    basenutrit = (int) obj_nutrition(otmp);
    nutrit = otmp->oeaten;
    if (otmp->otyp == CORPSE && otmp->odrained) {
        nutrit -= drainlevel(otmp);
        basenutrit -= drainlevel(otmp);
    }

#ifdef DEBUG
    debugpline("before rounddiv: victual.reqtime == %d", victual.reqtime);
    debugpline("oeaten == %d, basenutrit == %d", otmp->oeaten, basenutrit);
    debugpline("nutrit == %d, cnutrit == %d", nutrit, otmp->otyp == CORPSE ?
               mons[otmp->corpsenm].cnutrit : objects[otmp->otyp].oc_nutrition);
#endif
    victual.reqtime = (basenutrit == 0 ? 0 :
                       rounddiv(victual.reqtime * (long)nutrit, basenutrit));
#ifdef DEBUG
    debugpline("after rounddiv: victual.reqtime == %d", victual.reqtime);
#endif
    /* calculate the modulo value (nutrit. units per round eating)
     * note: this isn't exact - you actually lose a little nutrition
     *   due to this method.
     * TODO: add in a "remainder" value to be given at the end of the
     *   meal.
     */
    if (victual.reqtime == 0 || nutrit == 0)
        /* possible if most has been eaten before */
        victual.nmod = 0;
    else if (nutrit >= victual.reqtime)
        victual.nmod = -(nutrit / victual.reqtime);
    else
        victual.nmod = victual.reqtime % nutrit;
    victual.canchoke = (u.uhs == SATIATED);

    if (!dont_start) {
        start_eating(otmp, already_partly_eaten);
    }
    return(1);
}

int
use_tin_opener(obj)
struct obj *obj;
{
    struct obj *otmp;
    int res = 0;

    if (!carrying(TIN)) {
        You("have no tin to open.");
        return 0;
    }

    if (obj != uwep) {
        if (obj->cursed && obj->bknown) {
            char qbuf[QBUFSZ];

            if (ynq(safe_qbuf(qbuf, "Really wield ", "?", obj, doname, thesimpleoname, "that")) != 'y') {
                return 0;
            }
        }
        if (!wield_tool(obj, "use")) {
            return 0;
        }
        res = 1;
    }

    otmp = getobj(comestibles, "open");
    if (!otmp) {
        return res;
    }

    start_tin(otmp);
    return 1;
}

/* Take a single bite from a piece of food, checking for choking and
 * modifying usedtime.  Returns 1 if they choked and survived, 0 otherwise.
 */
STATIC_OVL int
bite()
{
    if (victual.canchoke && u.uhunger >= 2000) {
        choke(victual.piece);
        return 1;
    }
    if (victual.doreset) {
        do_reset_eat();
        return 0;
    }
    force_save_hs = TRUE;
    if (victual.nmod < 0) {
        lesshungry(-victual.nmod);
        consume_oeaten(victual.piece, victual.nmod); /* -= -nmod */
    } else if(victual.nmod > 0 && (victual.usedtime % victual.nmod)) {
        lesshungry(1);
        consume_oeaten(victual.piece, -1); /* -= 1 */
    }
    force_save_hs = FALSE;
    recalc_wt();
    return 0;
}

/* as time goes by - called by moveloop(every move) & domove(melee attack) */
void
gethungry()
{
    if (u.uinvulnerable) return;    /* you don't feel hungrier */

    /* being polymorphed into a creature which doesn't eat prevents
       this first uhunger decrement, but to stay in such form the hero
       will need to wear an Amulet of Unchanging so still burn a small
       amount of nutrition in the 'moves % 20' ring/amulet check below */
    if ((!u.usleep || !rn2(10)) /* slow metabolic rate while asleep */
        && (carnivorous(youmonst.data) ||
            herbivorous(youmonst.data) ||
            metallivorous(youmonst.data) ||
            is_vampire(youmonst.data))
#ifdef CONVICT
        /* Convicts can last twice as long at hungry and below */
        && (!Role_if(PM_CONVICT) || (moves % 2) || (u.uhs < HUNGRY))
#endif /* CONVICT */
        && !Slow_digestion)
        u.uhunger--;        /* ordinary food consumption */

    if (moves % 2) {    /* odd turns */
        /* Regeneration uses up food when injured, unless due to an artifact */
        if (HRegeneration || (((ERegeneration & (~W_ART)) &&
                               (ERegeneration != W_WEP || !uwep->oartifact)) &&
                              (uhp() < uhpmax())))
            u.uhunger--;
        if (near_capacity() > SLT_ENCUMBER) u.uhunger--;
    } else {        /* even turns */
        if (Hunger) u.uhunger--;
        /* Conflict uses up food too */
        if (HConflict || (EConflict & (~W_ARTI))) u.uhunger--;
        /* +0 charged rings don't do anything, so don't affect hunger */
        /* Slow digestion still uses ring hunger */
        switch ((int)(moves % 20)) {    /* note: use even cases only */
        case  4: if (uleft &&
                     (uleft->spe || !objects[uleft->otyp].oc_charged))
                u.uhunger--;
            break;
        case  8: if (uamul) u.uhunger--;
            break;
        case 12: if (uright &&
                     (uright->spe || !objects[uright->otyp].oc_charged))
                u.uhunger--;
            break;
        case 16: if (u.uhave.amulet) u.uhunger--;
            break;
        default: break;
        }
    }
    newuhs(TRUE);
}

/* called after vomiting and after performing feats of magic */
void
morehungry(num)
int num;
{
    u.uhunger -= num;
    newuhs(TRUE);
}

/* called after eating (and after drinking fruit juice) */
void
lesshungry(num)
int num;
{
    /* See comments in newuhs() for discussion on force_save_hs */
    boolean iseating = (occupation == eatfood) || force_save_hs;
#ifdef DEBUG
    debugpline("lesshungry(%d)", num);
#endif
    u.uhunger += num;
    if (u.uhunger >= 2000) {
        if (!iseating || victual.canchoke) {
            if (iseating) {
                choke(victual.piece);
                reset_eat();
            } else
                choke(occupation == opentin ? tin.tin : (struct obj *)0);
            /* no reset_eat() */
        }
    } else {
        /* Have lesshungry() report when you're nearly full so all eating
         * warns when you're about to choke.
         */
        if (u.uhunger >= 1500) {
            if (!victual.eating || (victual.eating && !victual.fullwarn)) {
                pline("You're having a hard time getting all of it down.");
                nomovemsg = "You're finally finished.";
                if (!victual.eating)
                    multi = -2;
                else {
                    victual.fullwarn = TRUE;
                    if (victual.canchoke && victual.reqtime > 1) {
                        /* a one-gulp food will not survive a stop */
                        if (yn_function("Stop eating?", ynchars, 'y')=='y') {
                            reset_eat();
                            nomovemsg = (char *)0;
                        }
                    }
                }
            }
        }
    }
    newuhs(FALSE);
}

STATIC_PTR
int
unfaint()
{
    (void) Hear_again();
    if(u.uhs > FAINTING)
        u.uhs = FAINTING;
    stop_occupation();
    flags.botl = 1;
    if (iflags.debug_fuzzer) {
        /* keep the fuzzer from fainting repeatedly */
        u.uhs = SATIATED;
        u.uhunger = 2000;
    }
    return 0;
}

boolean
is_fainted()
{
    return((boolean)(u.uhs == FAINTED));
}

/* call when a faint must be prematurely terminated */
void
reset_faint()
{
    if (is_fainted()) {
        nomul(0, 0);
        if (afternmv == unfaint) {
            unmul("You revive.");
        }
    }
}

/* compute and comment on your (new?) hunger status */
void
newuhs(incr)
boolean incr;
{
    unsigned newhs;
    static unsigned save_hs;
    static boolean saved_hs = FALSE;
    int h = u.uhunger;

    newhs = (h > 1000) ? SATIATED :
            (h > 150) ? NOT_HUNGRY :
            (h > 50) ? HUNGRY :
            (h > 0) ? WEAK : FAINTING;

    /* While you're eating, you may pass from WEAK to HUNGRY to NOT_HUNGRY.
     * This should not produce the message "you only feel hungry now";
     * that message should only appear if HUNGRY is an endpoint.  Therefore
     * we check to see if we're in the middle of eating.  If so, we save
     * the first hunger status, and at the end of eating we decide what
     * message to print based on the _entire_ meal, not on each little bit.
     */
    /* It is normally possible to check if you are in the middle of a meal
     * by checking occupation == eatfood, but there is one special case:
     * start_eating() can call bite() for your first bite before it
     * sets the occupation.
     * Anyone who wants to get that case to work _without_ an ugly static
     * force_save_hs variable, feel free.
     */
    /* Note: If you become a certain hunger status in the middle of the
     * meal, and still have that same status at the end of the meal,
     * this will incorrectly print the associated message at the end of
     * the meal instead of the middle.  Such a case is currently
     * impossible, but could become possible if a message for SATIATED
     * were added or if HUNGRY and WEAK were separated by a big enough
     * gap to fit two bites.
     */
    if (occupation == eatfood || force_save_hs) {
        if (!saved_hs) {
            save_hs = u.uhs;
            saved_hs = TRUE;
        }
        u.uhs = newhs;
        return;
    } else {
        if (saved_hs) {
            u.uhs = save_hs;
            saved_hs = FALSE;
        }
    }

    if (newhs == FAINTING) {
        /* u.uhunger is likely to be negative at this point */
        int uhunger_div_by_10 = sgn(u.uhunger) * ((abs(u.uhunger) + 5) / 10);

        if (is_fainted()) {
            newhs = FAINTED;
        }
        if (u.uhs <= WEAK || rn2(20-uhunger_div_by_10) >= 19) {
            if (!is_fainted() && multi >= 0 /* %% */) {
                int duration = 10-uhunger_div_by_10;

                /* stop what you're doing, then faint */
                stop_occupation();
                You("faint from lack of food.");
                flags.soundok = 0;
                flags.botl = TRUE;
                nomul(-duration, "fainted from lack of food");
                nomovemsg = "You regain consciousness.";
                afternmv = unfaint;
                newhs = FAINTED;
            }
        /* this used to be -(200 + 20 * Con) but that was when being asleep
           suppressed per-turn uhunger decrement but being fainted didn't;
           now uhunger becomes more negative at a slower rate */
        } else if (u.uhunger < -(100 + 10 * ACURR(A_CON))) {
            u.uhs = STARVED;
            flags.botl = 1;
            bot();
            You("die from starvation.");
            killer.format = KILLED_BY;
            Strcpy(killer.name, "starvation");
            done(STARVING);
            /* if we return, we lifesaved, and that calls newuhs */
            return;
        }
    }

    if (newhs != u.uhs) {
        if (newhs >= WEAK && u.uhs < WEAK) {
            /* this used to be losestr(1) which had the potential to
               be fatal (still handled below) by reducing HP if it
               tried to take base strength below minimum of 3 */
            ATEMP(A_STR) = -1; /* temporary loss overrides Fixed_abil */
            /* defer context.botl status update until after hunger message */
        } else if (newhs < WEAK && u.uhs >= WEAK) {
            /* this used to be losestr(-1) which could be abused by
               becoming weak while wearing ring of sustain ability,
               removing ring, eating to 'restore' strength which boosted
               strength by a point each time the cycle was performed;
               substituting "while polymorphed" for sustain ability and
               "rehumanize" for ring removal might have done that too */
            ATEMP(A_STR) = 0; /* repair of loss also overrides Fixed_abil */
            /* defer context.botl status update until after hunger message */
        }

        switch (newhs) {
        case HUNGRY:
            if (Hallucination) {
                You((!incr) ?
                    "now have a lesser case of the munchies." :
                    "are getting the munchies.");
            } else
                You((!incr) ? "only feel hungry now." :
                    (u.uhunger < 145) ? "feel hungry." :
                    "are beginning to feel hungry.");
            if (incr && occupation &&
                (occupation != eatfood && occupation != opentin))
                stop_occupation();
            break;

        case WEAK:
            if (Hallucination)
                pline((!incr) ?
                      "You still have the munchies." :
                      "The munchies are interfering with your motor capabilities.");
            else if (incr &&
                     (Role_if(PM_WIZARD) || Race_if(PM_ELF) ||
                      Role_if(PM_VALKYRIE)))
                pline("%s needs food, badly!",
                      (Role_if(PM_WIZARD) || Role_if(PM_VALKYRIE)) ?
                      urole.name.m : "Elf");
            else
                You((!incr) ? "feel weak now." :
                    (u.uhunger < 45) ? "feel weak." :
                    "are beginning to feel weak.");
            if (incr && occupation &&
                (occupation != eatfood && occupation != opentin))
                stop_occupation();
            break;
        }
        u.uhs = newhs;
        flags.botl = 1;
        bot();
        nomul(0, 0); /* stop running or travelling */
        if ((Upolyd ? u.mh : u.uhp) < 1) {
            You("die from hunger and exhaustion.");
            killer.format = KILLED_BY;
            Strcpy(killer.name, "exhaustion");
            done(STARVING);
            return;
        }
    }
}

/* Returns an object representing food.  Object may be either on floor or
 * in inventory.
 */
struct obj *
floorfood(verb, corpsecheck) /* get food from floor or pack */
const char *verb;
int corpsecheck; /* 0, no check, 1, corpses, 2, tinnable corpses */
{
    register struct obj *otmp;
    char qbuf[QBUFSZ];
    char c;
    boolean feeding = (!strcmp(verb, "eat"));           // corpsecheck == 0
    boolean sacrificing = (!strcmp(verb, "sacrifice")); // corpsecheck == 1

    /* if we can't touch floor objects then use invent food only */
    if (!can_reach_floor(TRUE) ||
        iflags.menu_requested || /* command was preceded by 'm' prefix */
#ifdef STEED
        (feeding && u.usteed) || /* can't eat off floor while riding */
#endif
        ((is_pool(u.ux, u.uy) || is_lava(u.ux, u.uy)) &&
         (Wwalking || is_clinger(youmonst.data) ||
          (Flying && !Breathless))))
        goto skipfloor;

    if (feeding && metallivorous(youmonst.data)) {
        struct obj *gold;
        struct trap *ttmp = t_at(u.ux, u.uy);

        if (ttmp && ttmp->tseen && ttmp->ttyp == BEAR_TRAP) {
            boolean u_in_beartrap = (u.utrap && u.utraptype == TT_BEARTRAP);

            /* If not already stuck in the trap, perhaps there should
               be a chance to becoming trapped?  Probably not, because
               then the trap would just get eaten on the _next_ turn... */
            Sprintf(qbuf, "There is a bear trap here (%s); eat it?",
                    u_in_beartrap ?
                    "holding you" : "armed");
            if ((c = yn_function(qbuf, ynqchars, 'n')) == 'y') {
                deltrap(ttmp);
                if (u_in_beartrap) {
                    reset_utrap(TRUE);
                }
                return mksobj(BEARTRAP, TRUE, FALSE);
            } else if (c == 'q') {
                return (struct obj *)0;
            }
        }

        if (youmonst.data != &mons[PM_RUST_MONSTER] &&
            (gold = g_at(u.ux, u.uy)) != 0) {
            if (gold->quan == 1L)
                Sprintf(qbuf, "There is 1 gold piece here; eat it?");
            else
                Sprintf(qbuf, "There are %ld gold pieces here; eat them?",
                        gold->quan);
            if ((c = yn_function(qbuf, ynqchars, 'n')) == 'y') {
                return gold;
            } else if (c == 'q') {
                return (struct obj *)0;
            }
        }
    }
#ifdef ASTRAL_ESCAPE
    if (sacrificing) {
        for (otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere) {
            if(otmp->otyp == AMULET_OF_YENDOR || otmp->otyp == FAKE_AMULET_OF_YENDOR) {
                Sprintf(qbuf, "There %s %s here; %s %s?",
                        otense(otmp, "are"),
                        doname(otmp), verb,
                        "it");
                if((c = yn_function(qbuf, ynqchars, 'n')) == 'y')
                    return(otmp);
                else if(c == 'q')
                    return((struct obj *) 0);
            }
        }
    }
#endif

    /* Is there some food (probably a heavy corpse) here on the ground? */
    for (otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere) {
        if(corpsecheck ?
           (otmp->otyp==CORPSE && (corpsecheck == 1 || tinnable(otmp))) :
           feeding ? (otmp->oclass != COIN_CLASS && is_edible(otmp)) :
           otmp->oclass==FOOD_CLASS) {

            /* if blind and without gloves, attempting to eat (or tin or
               offer) a cockatrice corpse is fatal before asking whether
               or not to use it; otherwise, 'm<dir>' followed by 'e' could
               be used to locate cockatrice corpses without touching them */
            if (otmp->otyp == CORPSE && will_feel_cockatrice(otmp, FALSE)) {
                feel_cockatrice(otmp, FALSE);
                /* if life-saved (or poly'd into stone golem), terminate
                   attempt to eat off floor */
                return (struct obj *) 0;
            }

            Sprintf(qbuf, "There %s %s here; %s %s?",
                    otense(otmp, "are"),
                    doname(otmp), verb,
                    (otmp->quan == 1L) ? "it" : "one");
            if((c = yn_function(qbuf, ynqchars, 'n')) == 'y')
                return(otmp);
            else if(c == 'q')
                return((struct obj *) 0);
        }
    }

skipfloor:
    /* We cannot use ALL_CLASSES since that causes getobj() to skip its
     * "ugly checks" and we need to check for inedible items.
     */
#ifdef ASTRAL_ESCAPE
    otmp = getobj(sacrificing ? (const char *)sacrifice_types :
                  feeding ? (const char *)allobj :
                  (const char *)comestibles, verb);
#else
    otmp = getobj(feeding ? (const char *)allobj :
                  (const char *)comestibles, verb);
#endif
    if (corpsecheck && otmp)
        /* Kludge to allow Amulet of Yendor to be sacrificed on non-High altars */
        if (otmp->otyp != AMULET_OF_YENDOR && otmp->otyp != FAKE_AMULET_OF_YENDOR)
            if (otmp->otyp != CORPSE || (corpsecheck == 2 && !tinnable(otmp))) {
                You_cant("%s that!", verb);
                return (struct obj *)0;
            }
    return otmp;
}

/* Side effects of vomiting */
/* added nomul (MRS) - it makes sense, you're too busy being sick! */
void
vomit()     /* A good idea from David Neves */
{
    if (cantvomit(youmonst.data)) {
        /* doesn't cure food poisoning; message assumes that we aren't
           dealing with some esoteric body_part() */
        Your("jaw gapes convulsively.");
    } else {
        if (Sick && (u.usick_type & SICK_VOMITABLE) != 0) {
            make_sick(0L, (char *) 0, TRUE, SICK_VOMITABLE);
        }
        /* if not enough in stomach to actually vomit then dry heave;
           vomiting_dialog() gives a vomit message when its countdown
           reaches 0, but only if u.uhs < FAINTING (and !cantvomit()) */
        if (u.uhs >= FAINTING) {
            Your("%s heaves convulsively!", body_part(STOMACH));
        }
    }

    /* nomul()/You_can_move_again used to be unconditional, which was
       viable while eating but not for Vomiting countdown where hero might
       be immobilized for some other reason at the time vomit() is called */
    if (multi >= -2) {
        nomul(-2, "vomiting");
        multi_reason = "vomiting";
        nomovemsg = You_can_move_again;
    }
}

int
eaten_stat(base, obj)
register int base;
register struct obj *obj;
{
    long uneaten_amt, full_amount;

    /* get full_amount first; obj_nutrition() might modify obj->oeaten */
    full_amount = (long) obj_nutrition(obj);
    uneaten_amt = (long) obj->oeaten;
    if (uneaten_amt > full_amount) {
        warning(
            "partly eaten food (%ld) more nutritious than untouched food (%ld)",
            uneaten_amt, full_amount);
        uneaten_amt = full_amount;
    }

    base = (int)(full_amount ? (long)base * uneaten_amt / full_amount : 0L);
    return (base < 1) ? 1 : base;
}

/* reduce obj's oeaten field, making sure it never hits or passes 0 */
void
consume_oeaten(obj, amt)
struct obj *obj;
int amt;
{
    /*
     * This is a hack to try to squelch several long standing mystery
     * food bugs.  A better solution would be to rewrite the entire
     * victual handling mechanism from scratch using a less complex
     * model.  Alternatively, this routine could call done_eating()
     * or food_disappears() but its callers would need revisions to
     * cope with victual.piece unexpectedly going away.
     *
     * Multi-turn eating operates by setting the food's oeaten field
     * to its full nutritional value and then running a counter which
     * independently keeps track of whether there is any food left.
     * The oeaten field can reach exactly zero on the last turn, and
     * the object isn't removed from inventory until the next turn
     * when the "you finish eating" message gets delivered, so the
     * food would be restored to the status of untouched during that
     * interval.  This resulted in unexpected encumbrance messages
     * at the end of a meal (if near enough to a threshold) and would
     * yield full food if there was an interruption on the critical
     * turn.  Also, there have been reports over the years of food
     * becoming massively heavy or producing unlimited satiation;
     * this would occur if reducing oeaten via subtraction attempted
     * to drop it below 0 since its unsigned type would produce a
     * huge positive value instead.  So far, no one has figured out
     * _why_ that inappropriate subtraction might sometimes happen.
     */

    if (amt > 0) {
        /* bit shift to divide the remaining amount of food */
        obj->oeaten >>= amt;
    } else {
        /* simple decrement; value is negative so we actually add it */
        if ((int) obj->oeaten > -amt)
            obj->oeaten += amt;
        else
            obj->oeaten = 0;
    }

    if (obj->oeaten == 0) {
        if (obj == victual.piece) /* always true unless wishing... */
            victual.reqtime = victual.usedtime; /* no bites left */
        obj->oeaten = 1; /* smallest possible positive value */
    }
}

/* called when eatfood occupation has been interrupted,
   or in the case of theft, is about to be interrupted */
boolean
maybe_finished_meal(stopping)
boolean stopping;
{
    /* in case consume_oeaten() has decided that the food is all gone */
    if (occupation == eatfood && victual.usedtime >= victual.reqtime) {
        if (stopping) occupation = 0;   /* for do_reset_eat */
        (void) eatfood(); /* calls done_eating() to use up victual.piece */
        return TRUE;
    }
    return FALSE;
}

/* Tin of <something> to the rescue?  Decide whether current occupation
   is an attempt to eat a tin of something capable of saving hero's life.
   We don't care about consumption of non-tinned food here because special
   effects there take place on first bite rather than at end of occupation.
   [Popeye the Sailor gets out of trouble by eating tins of spinach. :-] */
boolean
Popeye(threat)
int threat;
{
    struct obj *otin;
    int mndx;

    if (occupation != opentin) {
        return FALSE;
    }
    otin = tin.tin;
    /* make sure hero still has access to tin */
    if (!carried(otin) &&
            (!obj_here(otin, u.ux, u.uy) || !can_reach_floor(TRUE))) {
        return FALSE;
    }
    /* unknown tin is assumed to be helpful */
    if (!otin->known) {
        return TRUE;
    }
    /* known tin is helpful if it will stop life-threatening problem */
    mndx = otin->corpsenm;
    switch (threat) {
    /* note: not used; hunger code bypasses stop_occupation() when eating */
    case HUNGER:
        return (boolean) (mndx != NON_PM || otin->spe == 1);
    /* flesh from lizards and acidic critters stops petrification */
    case STONED:
        return (boolean) (mndx >= LOW_PM && (mndx == PM_LIZARD || acidic(&mons[mndx])));
    /* no tins can cure these (yet?) */
    case SLIMED:
    case SICK:
    case VOMITING:
        break;
    default:
        break;
    }
    return FALSE;
}

/*eat.c*/
