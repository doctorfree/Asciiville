/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

boolean notonhead = FALSE;

static NEARDATA int nothing, unkn;
static NEARDATA const char beverages[] = { POTION_CLASS, 0 };

STATIC_DCL long FDECL(itimeout, (long));
STATIC_DCL long FDECL(itimeout_incr, (long, int));
STATIC_DCL void NDECL(ghost_from_bottle);
STATIC_OVL void NDECL(alchemy_init);
STATIC_DCL boolean FDECL(H2Opotion_dip, (struct obj *, struct obj *, BOOLEAN_P, const char *));
STATIC_DCL short FDECL(mixtype, (struct obj *, struct obj *));

#ifndef TESTING
STATIC_DCL int FDECL(dip, (struct obj *, struct obj *));
#endif

/* force `val' to be within valid range for intrinsic timeout value */
STATIC_OVL long
itimeout(val)
long val;
{
    if (val >= TIMEOUT) val = TIMEOUT;
    else if (val < 1) val = 0;

    return val;
}

/* increment `old' by `incr' and force result to be valid intrinsic timeout */
STATIC_OVL long
itimeout_incr(old, incr)
long old;
int incr;
{
    return itimeout((old & TIMEOUT) + (long)incr);
}

/* set the timeout field of intrinsic `which' */
void
set_itimeout(which, val)
long *which, val;
{
    *which &= ~TIMEOUT;
    *which |= itimeout(val);
}

/* increment the timeout field of intrinsic `which' */
void
incr_itimeout(which, incr)
long *which;
int incr;
{
    set_itimeout(which, itimeout_incr(*which, incr));
}

void
make_confused(xtime, talk)
long xtime;
boolean talk;
{
    long old = HConfusion;

    if (Unaware) {
        talk = FALSE;
    }

    if (!xtime && old) {
        if (talk)
            You_feel("less %s now.",
                     Hallucination ? "trippy" : "confused");
    }
    if ((xtime && !old) || (!xtime && old)) flags.botl = TRUE;

    set_itimeout(&HConfusion, xtime);
}

void
make_stunned(xtime, talk)
long xtime;
boolean talk;
{
    long old = HStun;

    if (Unaware) {
        talk = FALSE;
    }

    if (!xtime && old) {
        if (talk)
            You_feel("%s now.",
                     Hallucination ? "less wobbly" : "a bit steadier");
    }
    if (xtime && !old) {
        if (talk) {
#ifdef STEED
            if (u.usteed)
                You("wobble in the saddle.");
            else
#endif
            You("%s...", stagger(youmonst.data, "stagger"));
        }
    }
    if ((!xtime && old) || (xtime && !old)) flags.botl = TRUE;

    set_itimeout(&HStun, xtime);
}

void
make_sick(xtime, cause, talk, type)
long xtime;
const char *cause;  /* sickness cause */
boolean talk;
int type;
{
    long old = Sick;

    if (xtime > 0L) {
        if (Sick_resistance) return;
        if (!old) {
            /* newly sick */
            You_feel("deathly sick.");
        } else {
            /* already sick */
            if (talk) You_feel("%s worse.",
                               xtime <= Sick/2L ? "much" : "even");
        }
        set_itimeout(&Sick, xtime);
        u.usick_type |= type;
        flags.botl = TRUE;
    } else if (old && (type & u.usick_type)) {
        /* was sick, now not */
        u.usick_type &= ~type;
        if (u.usick_type) { /* only partly cured */
            if (talk) You_feel("somewhat better.");
            set_itimeout(&Sick, Sick * 2); /* approximation */
        } else {
            if (talk) pline("What a relief!");
            Sick = 0L;  /* set_itimeout(&Sick, 0L) */
        }
        flags.botl = TRUE;
    }

    struct kinfo *kptr = find_delayed_killer(SICK);
    if (Sick) {
        exercise(A_CON, FALSE);
        /* setting delayed_killer used to be unconditional, but that's
           not right when make_sick(0) is called to cure food poisoning
           if hero was also fatally ill; this is only approximate */
        if (xtime || !old || !kptr) {
            int kpfx = ((cause && !strcmp(cause, "#wizintrinsic"))
                        ? KILLED_BY : KILLED_BY_AN);

            delayed_killer(SICK, kpfx, cause);
        }
    } else {
        dealloc_killer(kptr);
    }
}

/* start or stop petrification */
void
make_stoned(xtime, msg, killedby, killername)
long xtime;
const char *msg;
int killedby;
const char *killername;
{
    long old = Stoned;

#if 0   /* tell player even if hero is unconscious */
    if (Unaware)
        msg = 0;
#endif
    set_itimeout(&Stoned, xtime);
    if ((xtime != 0L) ^ (old != 0L)) {
        flags.botl = TRUE;
        if (msg)
            pline("%s", msg);
    }
    if (!Stoned) {
        dealloc_killer(find_delayed_killer(STONED));
    } else if (!old) {
        delayed_killer(STONED, killedby, killername);
    }
}

void
make_vomiting(xtime, talk)
long xtime;
boolean talk;
{
    long old = Vomiting;

    if (Unaware) {
        talk = FALSE;
    }

    set_itimeout(&Vomiting, xtime);
    flags.botl = TRUE;
    if (!xtime && old) {
        if (talk) {
            You_feel("much less nauseated now.");
        }
    }
}

void
make_slimed(xtime, msg)
long xtime;
const char *msg;
{
    long old = Slimed;

#if 0   /* tell player even if hero is unconscious */
    if (Unaware)
        msg = 0;
#endif
    set_itimeout(&Slimed, xtime);
    if ((xtime != 0L) ^ (old != 0L)) {
        flags.botl = TRUE;
        if (msg) {
            pline("%s", msg);
        }
    }
    if (!Slimed) {
#if NEXT_VERSION
        dealloc_killer(find_delayed_killer(SLIMED));
#endif
        /* fake appearance is set late in turn-to-slime countdown */
        if (U_AP_TYPE == M_AP_MONSTER && youmonst.mappearance == PM_GREEN_SLIME) {
            youmonst.m_ap_type = M_AP_NOTHING;
            youmonst.mappearance = 0;
        }
    }
}

static const char vismsg[] = "vision seems to %s for a moment but is %s now.";
static const char eyemsg[] = "%s momentarily %s.";

void
make_blinded(xtime, talk)
long xtime;
boolean talk;
{
    long old = Blinded;
    boolean u_could_see, can_see_now;
    int eyecnt;
    char buf[BUFSZ];

    /* we need to probe ahead in case the Eyes of the Overworld
       are or will be overriding blindness */
    u_could_see = !Blind;
    Blinded = xtime ? 1L : 0L;
    can_see_now = !Blind;
    Blinded = old;      /* restore */

    if (u.usleep) talk = FALSE;

    if (can_see_now && !u_could_see) {  /* regaining sight */
        if (talk && !u.incloud) {
            if (Hallucination)
                pline("Far out!  Everything is all cosmic again!");
            else
                You("can see again.");
        }
    } else if (old && !xtime) {
        /* clearing temporary blindness without toggling blindness */
        if (talk && !u.incloud) {
            if (!haseyes(youmonst.data)) {
                strange_feeling((struct obj *)0, (char *)0);
            } else if (Blindfolded) {
                Strcpy(buf, body_part(EYE));
                eyecnt = eyecount(youmonst.data);
                Your(eyemsg, (eyecnt == 1) ? buf : makeplural(buf),
                     (eyecnt == 1) ? "itches" : "itch");
            } else { /* Eyes of the Overworld */
                Your(vismsg, "brighten",
                     Hallucination ? "sadder" : "normal");
            }
        }
    }

    if (u_could_see && !can_see_now) {  /* losing sight */
        if (talk) {
            if (Hallucination)
                pline("Oh, bummer!  Everything is dark!  Help!");
            else
                pline("A cloud of darkness falls upon you.");
        }
        /* Before the hero goes blind, set the ball&chain variables. */
        if (Punished) set_bc(0);
    } else if (!old && xtime) {
        /* setting temporary blindness without toggling blindness */
        if (talk) {
            if (!haseyes(youmonst.data)) {
                strange_feeling((struct obj *)0, (char *)0);
            } else if (Blindfolded) {
                Strcpy(buf, body_part(EYE));
                eyecnt = eyecount(youmonst.data);
                Your(eyemsg, (eyecnt == 1) ? buf : makeplural(buf),
                     (eyecnt == 1) ? "twitches" : "twitch");
            } else { /* Eyes of the Overworld */
                Your(vismsg, "dim",
                     Hallucination ? "happier" : "normal");
            }
        }
    }

    set_itimeout(&Blinded, xtime);

    if (u_could_see ^ can_see_now) {  /* one or the other but not both */
        flags.botl = 1;
        vision_full_recalc = 1; /* blindness just got toggled */
        if (Blind_telepat || Infravision) see_monsters();
    }
}

/* blindness has just started or just ended--caller enforces that;
   called by Blindf_on(), Blindf_off(), and make_blinded() */
void
toggle_blindness()
{
    boolean Stinging = (uwep && (EWarn_of_mon & W_WEP) != 0L);

    /* blindness has just been toggled */
    flags.botl = TRUE; /* status conditions need update */
    vision_full_recalc = 1; /* vision has changed */
    /* this vision recalculation used to be deferred until moveloop(),
       but that made it possible for vision irregularities to occur
       (cited case was force bolt hitting an adjacent potion of blindness
       and then a secret door; hero was blinded by vapors but then got the
       message "a door appears in the wall" because wall spot was IN_SIGHT) */
    vision_recalc(0);
    if (Blind_telepat || Infravision || Stinging) {
        see_monsters(); /* also counts EWarn_of_mon monsters */
    }
    /*
     * Avoid either of the sequences
     * "Sting starts glowing", [become blind], "Sting stops quivering" or
     * "Sting starts quivering", [regain sight], "Sting stops glowing"
     * by giving "Sting is quivering" when becoming blind or
     * "Sting is glowing" when regaining sight so that the eventual
     * "stops" message matches the most recent "Sting is ..." one.
     */
    if (Stinging) {
        Sting_effects(-1);
    }
    /* update dknown flag for inventory picked up while blind */
    if (!Blind) {
        learn_unseen_invent();
    }
}

boolean
make_hallucinated(xtime, talk, mask)
long xtime; /* nonzero if this is an attempt to turn on hallucination */
boolean talk;
long mask;  /* nonzero if resistance status should change by mask */
{
    long old = HHallucination;
    boolean changed = 0;
    const char *message, *verb;

    if (flags.perma_hallu) {
        if (xtime > 0) {
            pline("Oh wow!  You have a yet another vision!");
        }
        return 0;
    }

    message = (!xtime) ? "Everything %s SO boring now." :
              "Oh wow!  Everything %s so cosmic!";
    verb = (!Blind) ? "looks" : "feels";

    if (mask) {
        if (HHallucination) changed = TRUE;

        if (!xtime) EHalluc_resistance |= mask;
        else EHalluc_resistance &= ~mask;
    } else {
        if (!EHalluc_resistance && (!!HHallucination != !!xtime))
            changed = TRUE;
        set_itimeout(&HHallucination, xtime);

        /* clearing temporary hallucination without toggling vision */
        if (!changed && !HHallucination && old && talk) {
            if (!haseyes(youmonst.data)) {
                strange_feeling((struct obj *)0, (char *)0);
            } else if (Blind) {
                char buf[BUFSZ];
                int eyecnt = eyecount(youmonst.data);

                Strcpy(buf, body_part(EYE));
                Your(eyemsg, (eyecnt == 1) ? buf : makeplural(buf),
                     (eyecnt == 1) ? "itches" : "itch");
            } else { /* Grayswandir */
                Your(vismsg, "flatten", "normal");
            }
        }
    }

    if (changed) {
        if (u.uswallow) {
            swallowed(0); /* redraw swallow display */
        } else {
            /* The see_* routines should be called *before* the pline. */
            see_monsters();
            see_objects();
            see_traps();
        }

        /* for perm_inv and anything similar
           (eg. Qt windowport's equipped items display) */
        update_inventory();

        flags.botl = 1;
        if (talk) pline(message, verb);
    }
    return changed;
}

/* set or clear "slippery fingers" */
void
make_glib(xtime)
int xtime;
{
    set_itimeout(&Glib, xtime);
    /* may change "(being worn)" to "(being worn; slippery)" or vice versa */
    if (uarmg) {
        update_inventory();
    }
}

STATIC_OVL void
ghost_from_bottle()
{
    struct monst *mtmp = makemon(&mons[PM_GHOST], u.ux, u.uy, NO_MM_FLAGS);

    if (!mtmp) {
        pline("This bottle turns out to be empty.");
        return;
    }
    if (Blind) {
        pline("As you open the bottle, %s emerges.", something);
        return;
    }
    pline("As you open the bottle, an enormous %s emerges!",
          Hallucination ? rndmonnam() : (const char *)"ghost");
    if(flags.verbose)
        You("are frightened to death, and unable to move.");
    nomul(-3, "being frightened to death");
    nomovemsg = "You regain your composure.";
}

/* "Quaffing is like drinking, except you spill more."  -- Terry Pratchett
 */
int
dodrink()
{
    register struct obj *otmp;
    const char *potion_descr;

    if (Strangled) {
        pline("If you can't breathe air, how can you drink liquid?");
        return 0;
    }
    /* Is there a fountain to drink from here? */
    if (IS_FOUNTAIN(levl[u.ux][u.uy].typ) &&
        /* not as low as floor level but similar restrictions apply */
         can_reach_floor(FALSE)) {
        if (yn("Drink from the fountain?") == 'y') {
            drinkfountain();
            return 1;
        }
    }
#ifdef SINKS
    /* Or a kitchen sink? */
    if (IS_SINK(levl[u.ux][u.uy].typ) &&
        /* not as low as floor level but similar restrictions apply */
         can_reach_floor(FALSE)) {
        if (yn("Drink from the sink?") == 'y') {
            drinksink();
            return 1;
        }
    }
#endif

    /* Or are you surrounded by water? */
    if (Underwater && !u.uswallow) {
        if (yn("Drink the water around you?") == 'y') {
            if (Role_if(PM_ARCHEOLOGIST)) {
                pline("No thank you, fish make love in it!"); /* Indiana Jones 3 */
            } else {
                pline("Do you know what lives in this water!");
            }
            return 1;
        }
    }

    otmp = getobj(beverages, "drink");
    if(!otmp) return(0);

    /* quan > 1 used to be left to useup(), but we need to force
       the current potion to be unworn, and don't want to do
       that for the entire stack when starting with more than 1.
       [Drinking a wielded potion of polymorph can trigger a shape
       change which causes hero's weapon to be dropped.  In 3.4.x,
       that led to an "object lost" panic since subsequent useup()
       was no longer dealing with an inventory item.  Unwearing
       the current potion is intended to keep it in inventory.] */
    if (otmp->quan > 1L) {
        otmp = splitobj(otmp, 1L);
        otmp->owornmask = 0L; /* rest of original stuck unaffected */
    } else if (otmp->owornmask) {
        remove_worn_item(otmp, FALSE);
    }
    otmp->in_use = TRUE; /* you've opened the stopper */

    potion_descr = OBJ_DESCR(objects[otmp->otyp]);
    if (potion_descr) {
        if (!strcmp(potion_descr, "milky") &&
            flags.ghost_count < MAXMONNO &&
            !rn2(POTION_OCCUPANT_CHANCE(flags.ghost_count))) {
            ghost_from_bottle();
            useup(otmp);
            return(1);
        } else if (!strcmp(potion_descr, "smoky") &&
                   flags.djinni_count < MAXMONNO &&
                   !rn2(POTION_OCCUPANT_CHANCE(flags.djinni_count))) {
            djinni_from_bottle(otmp);
            useup(otmp);
            return(1);
        }
    }
    return dopotion(otmp);
}

int
dopotion(otmp)
register struct obj *otmp;
{
    int retval;

    otmp->in_use = TRUE;
    nothing = unkn = 0;
    if((retval = peffects(otmp)) >= 0) return(retval);

    if(nothing) {
        unkn++;
        You("have a %s feeling for a moment, then it passes.",
            Hallucination ? "normal" : "peculiar");
    }
    if(otmp->dknown && !objects[otmp->otyp].oc_name_known) {
        if(!unkn) {
            makeknown(otmp->otyp);
            more_experienced(0, 0, 10);
        } else if(!objects[otmp->otyp].oc_uname)
            docall(otmp);
    }
    useup(otmp);
    return(1);
}

int
peffects(otmp)
register struct obj *otmp;
{
    register int i, ii, lim;

    switch(otmp->otyp) {
    case POT_RESTORE_ABILITY:
    case SPE_RESTORE_ABILITY:
        unkn++;
        if(otmp->cursed) {
            pline("Ulch!  This makes you feel mediocre!");
            break;
        } else {
            /* unlike unicorn horn, overrides Fixed_abil */
            pline("Wow!  This makes you feel %s!",
                  (otmp->blessed) ?
                  (unfixable_trouble_count(FALSE) ? "better" : "great")
                  : "good");
            i = rn2(A_MAX);     /* start at a random point */
            for (ii = 0; ii < A_MAX; ii++) {
                lim = AMAX(i);
                /* this used to adjust 'lim' for A_STR when u.uhs was
                   WEAK or worse, but that's handled via ATEMP(A_STR) now */
                if (ABASE(i) < lim) {
                    ABASE(i) = lim;
                    flags.botl = 1;
                    /* only first found if not blessed */
                    if (!otmp->blessed) break;
                }
                if(++i >= A_MAX) i = 0;
            }
            /* when using the potion (not the spell) also restore lost levels,
               to make the potion more worth keeping around for players with
               the spell or with a unihorn; this is better than full healing
               in that it can restore all of them, not just half, and a
               blessed potion restores them all at once */
            if (otmp->otyp == POT_RESTORE_ABILITY && u.ulevel < u.ulevelmax) {
                do {
                    pluslvl(FALSE);
                } while (u.ulevel < u.ulevelmax && otmp->blessed);
            }
        }
        break;

    case POT_HALLUCINATION:
        if (Hallucination || Halluc_resistance) nothing++;
        (void) make_hallucinated(itimeout_incr(HHallucination,
                                               rn1(200, 600 - 300 * bcsign(otmp))),
                                 TRUE, 0L);
        break;

    case POT_WATER:
        if (!otmp->blessed && !otmp->cursed) {
            pline("This tastes like %s.", hliquid("water"));
            u.uhunger += rnd(10);
            newuhs(FALSE);
            break;
        }
        unkn++;
        if (is_undead(youmonst.data) || is_demon(youmonst.data) ||
            u.ualign.type == A_CHAOTIC) {
            if (otmp->blessed) {
                pline("This burns like %s!", hliquid("acid"));
                exercise(A_CON, FALSE);
                if (u.ulycn >= LOW_PM) {
                    Your("affinity to %s disappears!",
                         makeplural(mons[u.ulycn].mname));
                    if (youmonst.data == &mons[u.ulycn])
                        you_unwere(FALSE);
                    set_ulycn(NON_PM); /* cure lycanthropy */
                }
                losehp(Maybe_Half_Phys(d(2, 6)), "potion of holy water", KILLED_BY_AN);
            } else if (otmp->cursed) {
                You_feel("quite proud of yourself.");
                healup(d(2, 6), 0, 0, 0);
                if (u.ulycn >= LOW_PM && !Upolyd) you_were();
                exercise(A_CON, TRUE);
            }
        } else {
            if (otmp->blessed) {
                You_feel("full of awe.");
                make_sick(0L, (char *) 0, TRUE, SICK_ALL);
                exercise(A_WIS, TRUE);
                exercise(A_CON, TRUE);
                if (u.ulycn >= LOW_PM)
                    you_unwere(TRUE); /* "Purified" */
                /* make_confused(0L,TRUE); */
            } else {
                if (u.ualign.type == A_LAWFUL) {
                    pline("This burns like %s!", hliquid("acid"));
                    losehp(Maybe_Half_Phys(d(2, 6)), "potion of unholy water",
                           KILLED_BY_AN);
                } else
                    You_feel("full of dread.");
                if (u.ulycn >= LOW_PM && !Upolyd) you_were();
                exercise(A_CON, FALSE);
            }
        }
        break;

    case POT_BOOZE:
        unkn++;
        pline("Ooph!  This tastes like %s%s!",
              otmp->odiluted ? "watered down " : "",
              Hallucination ? "dandelion wine" : "liquid fire");
        if (!otmp->blessed)
            make_confused(itimeout_incr(HConfusion, d(3, 8)), FALSE);
        /* the whiskey makes us feel better */
        if (!otmp->odiluted) healup(1, 0, FALSE, FALSE);
        u.uhunger += 10 * (2 + bcsign(otmp));
        newuhs(FALSE);
        exercise(A_WIS, FALSE);
        if (otmp->cursed) {
            You("pass out.");
            multi = -rnd(15);
            nomovemsg = "You awake with a headache.";
        }
        break;

    case POT_ENLIGHTENMENT:
        if (otmp->cursed) {
            unkn++;
            You("have an uneasy feeling...");
            exercise(A_WIS, FALSE);
        } else {
            if (otmp->blessed) {
                (void) adjattrib(A_INT, 1, FALSE);
                (void) adjattrib(A_WIS, 1, FALSE);
            }
            You_feel("self-knowledgeable...");
            display_nhwindow(WIN_MESSAGE, FALSE);
            enlightenment(0, TRUE);
            pline_The("feeling subsides.");
            exercise(A_WIS, TRUE);
        }
        break;
    case SPE_INVISIBILITY:
        /* spell cannot penetrate mummy wrapping */
        if (BInvis && uarmc->otyp == MUMMY_WRAPPING) {
            You_feel("rather itchy under %s.", yname(uarmc));
            break;
        }
        /* fall through */

    case POT_INVISIBILITY:
        if (Invis || Blind || BInvis) {
            nothing++;
        } else {
            self_invis_message();
        }
        if (otmp->blessed) HInvis |= FROMOUTSIDE;
        else incr_itimeout(&HInvis, rn1(15, 31));
        newsym(u.ux, u.uy);  /* update position */
        if (otmp->cursed) {
            pline("For some reason, you feel your presence is known.");
            aggravate();
        }
        break;

    case POT_SEE_INVISIBLE:
    /* tastes like fruit juice in Rogue */
    case POT_FRUIT_JUICE:
    {
        int msg = Invisible && !Blind;

        unkn++;
        if (otmp->cursed)
            pline("Yecch!  This tastes %s.",
                  Hallucination ? "overripe" : "rotten");
        else
            pline(Hallucination ?
                  "This tastes like 10%% real %s%s all-natural beverage." :
                  "This tastes like %s%s.",
                  otmp->odiluted ? "reconstituted " : "",
                  fruitname(TRUE));
        if (otmp->otyp == POT_FRUIT_JUICE) {
            u.uhunger += (otmp->odiluted ? 5 : 10) * (2 + bcsign(otmp));
            newuhs(FALSE);
            break;
        }
        if (!otmp->cursed) {
            /* Tell them they can see again immediately, which
             * will help them identify the potion...
             */
            make_blinded(0L, TRUE);
        }
        if (otmp->blessed)
            HSee_invisible |= FROMOUTSIDE;
        else
            incr_itimeout(&HSee_invisible, rn1(100, 750));
        set_mimic_blocking(); /* do special mimic handling */
        see_monsters();       /* see invisible monsters */
        newsym(u.ux, u.uy);   /* see yourself! */
        if (msg && !Blind) {  /* Blind possible if polymorphed */
            You("can see through yourself, but you are visible!");
            unkn--;
        }
        break;
    }

    case POT_PARALYSIS:
        if (Free_action)
            You("stiffen momentarily.");
        else {
            if (Levitation || Is_airlevel(&u.uz)||Is_waterlevel(&u.uz))
                You("are motionlessly suspended.");
#ifdef STEED
            else if (u.usteed)
                You("are frozen in place!");
#endif
            else
                Your("%s are frozen to the %s!",
                     makeplural(body_part(FOOT)), surface(u.ux, u.uy));
            nomul(-(rn1(10, 25 - 12*bcsign(otmp))), "frozen by a potion");
            nomovemsg = You_can_move_again;
            exercise(A_DEX, FALSE);
        }
        break;

    case POT_SLEEPING:
        if(Sleep_resistance || Free_action)
            You("yawn.");
        else {
            You("suddenly fall asleep!");
            fall_asleep(-rn1(10, 25 - 12*bcsign(otmp)), TRUE);
        }
        break;

    case POT_MONSTER_DETECTION:
    case SPE_DETECT_MONSTERS:
        if (otmp->blessed) {
            int x, y;

            if (Detect_monsters) nothing++;
            unkn++;
            /* after a while, repeated uses become less effective */
            if ((HDetect_monsters & TIMEOUT) >= 300L) {
                i = 1;
            } else {
                i = rn1(40, 21);
            }
            incr_itimeout(&HDetect_monsters, i);
            for (x = 1; x < COLNO; x++) {
                for (y = 0; y < ROWNO; y++) {
                    if (levl[x][y].glyph == GLYPH_INVISIBLE) {
                        unmap_object(x, y);
                        newsym(x, y);
                    }
                    if (MON_AT(x, y)) unkn = 0;
                }
            }
            see_monsters();
            if (unkn) You_feel("lonely.");
            break;
        }
        if (monster_detect(otmp, 0))
            return(1);      /* nothing detected */
        exercise(A_WIS, TRUE);
        break;

    case POT_OBJECT_DETECTION:
    case SPE_DETECT_TREASURE:
        if (object_detect(otmp, 0, FALSE))
            return(1);      /* nothing detected */
        exercise(A_WIS, TRUE);
        break;

    case POT_SICKNESS:
        pline("Yecch!  This stuff tastes like poison.");
        if (otmp->blessed) {
            pline("(But in fact it was mildly stale %s.)",
                  fruitname(TRUE));
            if (!Role_if(PM_HEALER)) {
                /* NB: blessed otmp->fromsink is not possible */
                losehp(1, "mildly contaminated potion", KILLED_BY_AN);
            }
        } else {
            if(Poison_resistance)
                pline(
                    "(But in fact it was biologically contaminated %s.)",
                    fruitname(TRUE));
            if (Role_if(PM_HEALER))
                pline("Fortunately, you have been immunized.");
            else {
                int typ = rn2(A_MAX);

                if (!Fixed_abil) {
                    poisontell(typ);
                    (void) adjattrib(typ,
                                     Poison_resistance ? -1 : -rn1(4, 3),
                                     TRUE);
                }
                if(!Poison_resistance) {
                    if (otmp->fromsink)
                        losehp(rnd(10)+5*!!(otmp->cursed),
                               "contaminated tap water", KILLED_BY);
                    else
                        losehp(rnd(10)+5*!!(otmp->cursed),
                               "contaminated potion", KILLED_BY_AN);
                }
                exercise(A_CON, FALSE);
            }
        }
        if (Hallucination) {
            if(!flags.perma_hallu) {
                You("are shocked back to your senses!");
                (void) make_hallucinated(0L, FALSE, 0L);
            }
            else {
                You("feel less groovy for a harsh second.");
            }
        }
        break;

    case POT_CONFUSION:
        if(!Confusion)
            if (Hallucination) {
                pline("What a trippy feeling!");
                unkn++;
            } else
                pline("Huh, What?  Where am I?");
        else nothing++;
        make_confused(itimeout_incr(HConfusion,
                                    rn1(7, 16 - 8 * bcsign(otmp))),
                      FALSE);
        break;

    case POT_GAIN_ABILITY:
        if (otmp->cursed) {
            pline("Ulch!  That potion tasted foul!");
            unkn++;
        } else if (Fixed_abil) {
            nothing++;
        } else {      /* If blessed, try very hard to find an ability */
                      /* that can be increased; if not, try up to     */
            int itmp; /* 3 times to find one which can be increased.  */
            i = -1;     /* increment to 0 */
            for (ii = (otmp->blessed ? 1000 : A_MAX/2); ii > 0; ii--) {
                i = rn2(A_MAX);
                /* only give "your X is already as high as it can get"
                   message on last attempt */
                itmp = (ii == 1) ? 0 : -1;
                if (adjattrib(i, 1, itmp))
                    break;
            }
        }
        break;

    case POT_SPEED:
        if(Wounded_legs && !otmp->cursed
#ifdef STEED
           && !u.usteed /* heal_legs() would heal steeds legs */
#endif
           ) {
            heal_legs(0);
            unkn++;
            break;
        }
        /* fall through */

    case SPE_HASTE_SELF:
        if(!Very_fast) /* wwf@doe.carleton.ca */
            You("are suddenly moving %sfaster.",
                Fast ? "" : "much ");
        else {
            Your("%s get new energy.",
                 makeplural(body_part(LEG)));
            unkn++;
        }
        exercise(A_DEX, TRUE);
        incr_itimeout(&HFast, rn1(10, 100 + 60 * bcsign(otmp)));
        break;

    case POT_BLINDNESS:
        if(Blind) nothing++;
        make_blinded(itimeout_incr(Blinded,
                                   rn1(200, 250 - 125 * bcsign(otmp))),
                     (boolean) !Blind);
        break;

    case POT_GAIN_LEVEL:
        if (otmp->cursed) {
            unkn++;
            /* they went up a level */
            if ((ledger_no(&u.uz) == 1 && u.uhave.amulet) ||
                Can_rise_up(u.ux, u.uy, &u.uz)) {
                const char *riseup ="rise up, through the %s!";
                if (ledger_no(&u.uz) == 1) {
                    You(riseup, ceiling(u.ux, u.uy));
#ifdef RANDOMIZED_PLANES
                    goto_level(get_first_elemental_plane(), FALSE, FALSE, FALSE);
#else
                    goto_level(&earth_level, FALSE, FALSE, FALSE);
#endif
                } else {
                    register int newlev = depth(&u.uz)-1;
                    d_level newlevel;

                    get_level(&newlevel, newlev);
                    if(on_level(&newlevel, &u.uz)) {
                        pline("It tasted bad.");
                        break;
                    } else You(riseup, ceiling(u.ux, u.uy));
                    goto_level(&newlevel, FALSE, FALSE, FALSE);
                }
            }
            else You("have an uneasy feeling.");
            break;
        }
        pluslvl(FALSE);
        if (otmp->blessed)
            /* blessed potions place you at a random spot in the
             * middle of the new level instead of the low point
             */
            u.uexp = rndexp(TRUE);
        break;

    case POT_HEALING:
        You_feel("%s.", marathon_mode ? "refreshed" : "better");
        healup(d(6 + 2 * bcsign(otmp), 4),
               !otmp->cursed ? 1 : 0, !!otmp->blessed, !otmp->cursed);
        exercise(A_CON, TRUE);
        break;

    case POT_EXTRA_HEALING:
        You_feel("much %s.", marathon_mode ? "more refreshed" : "better");
        healup(d(6 + 2 * bcsign(otmp), 8),
               otmp->blessed ? 5 : !otmp->cursed ? 2 : 0,
               !otmp->cursed, TRUE);
        (void) make_hallucinated(0L, TRUE, 0L);
        exercise(A_CON, TRUE);
        exercise(A_STR, TRUE);
        break;

    case POT_FULL_HEALING:
        You_feel("completely %s.", marathon_mode ? "refreshed" : "healed");
        healup(400, 4+4*bcsign(otmp), !otmp->cursed, TRUE);
        /* Restore one lost level if blessed */
        if (otmp->blessed && u.ulevel < u.ulevelmax) {
            /* when multiple levels have been lost, drinking
               multiple potions will only get half of them back */
            u.ulevelmax -= 1;
            pluslvl(FALSE);
        }
        (void) make_hallucinated(0L, TRUE, 0L);
        exercise(A_STR, TRUE);
        exercise(A_CON, TRUE);
        break;

    case POT_LEVITATION:
    case SPE_LEVITATION:
        /*
         * BLevitation will be set if levitation is blocked due to being
         * inside rock (currently or formerly in phazing xorn form, perhaps)
         * but it doesn't prevent setting or incrementing Levitation timeout
         * (which will take effect after escaping from the rock if it hasn't
         * expired by then).
         */
        if (!Levitation && !BLevitation) {
            /* kludge to ensure proper operation of float_up() */
            set_itimeout(&HLevitation, 1L);
            float_up();
            /* This used to set timeout back to 0, then increment it below
               for blessed and uncursed effects.  But now we leave it so
               that cursed effect yields "you float down" on next turn.
               Blessed and uncursed get one extra turn duration. */
        } else {
            /* already levitating, or can't levitate */
            nothing++;
        }
        if (otmp->cursed) {
            /* 'already levitating' used to block the cursed effect(s)
               aside from ~I_SPECIAL; it was not clear whether that was
               intentional; either way, it no longer does (as of 3.6.1) */
            HLevitation &= ~I_SPECIAL; /* can't descend upon demand */
            if (BLevitation) {
                ; /* rising via levitation is blocked */
            } else if ((u.ux == xupstair && u.uy == yupstair) ||
                       (sstairs.up && u.ux == sstairs.sx && u.uy == sstairs.sy) ||
                       (xupladder && u.ux == xupladder && u.uy == yupladder)) {
                (void) doup();
                /* in case we're already Levitating, which would have
                   resulted in incrementing 'nothing' */
                nothing = 0; /* not nothing after all */
            } else if (has_ceiling(&u.uz)) {
                int dmg = rnd(!uarmh ? 10 : !is_metallic(uarmh) ? 6 : 3);

                You("hit your %s on the %s.", body_part(HEAD), ceiling(u.ux, u.uy));
                losehp(Maybe_Half_Phys(dmg), "colliding with the ceiling", KILLED_BY);
                nothing = 0; /* not nothing after all */
            }
        } else if (otmp->blessed) {
            /* at this point, timeout is already at least 1 */
            incr_itimeout(&HLevitation, rn1(50, 250));
            /* can descend at will (stop levitating via '>') provided timeout
               is the only factor (ie, not also wearing Lev ring or boots) */
            HLevitation |= I_SPECIAL;
        } else {
            /* timeout is already at least 1 */
            incr_itimeout(&HLevitation, rn1(140, 10));
        }
        if (Levitation && IS_SINK(levl[u.ux][u.uy].typ)) {
            spoteffects(FALSE);
        }
        /* levitating blocks flying */
        float_vs_flight();
        break;

    case POT_GAIN_ENERGY: /* M. Stephenson */
    {   register int num;
        if(otmp->cursed)
            You_feel("lackluster.");
        else
            pline("Magical energies course through your body.");
        num = rnd(5) + 5 * otmp->blessed + 1;
        u.uenmax += (otmp->cursed) ? -num : num;
        u.uen += (otmp->cursed) ? -num : num;
        if(u.uenmax <= 0) u.uenmax = 0;
        if(u.uen <= 0) u.uen = 0;
        flags.botl = 1;
        exercise(A_WIS, TRUE);}
        break;

    case POT_OIL: /* P. Winner */
    {
        boolean good_for_you = FALSE;

        if (otmp->lamplit) {
            if (likes_fire(youmonst.data)) {
                pline("Ahh, a refreshing drink.");
                good_for_you = TRUE;
            } else {
                You("burn your %s.", body_part(FACE));
                losehp(d(Fire_resistance ? 1 : 3, 4),
                       "burning potion of oil", KILLED_BY_AN);
            }
        } else if(otmp->cursed)
            pline("This tastes like castor oil.");
        else
            pline("That was smooth!");
        exercise(A_WIS, good_for_you);
    }
    break;

    case POT_ACID:
        if (Acid_resistance)
            /* Not necessarily a creature who _likes_ acid */
            pline("This tastes %s.", Hallucination ? "tangy" : "sour");
        else {
            pline("This burns%s!", otmp->blessed ? " a little" :
                  otmp->cursed ? " a lot" : " like acid");
            int dmg = d(otmp->cursed ? 2 : 1, otmp->blessed ? 4 : 8);
            losehp(Maybe_Half_Phys(dmg), "potion of acid", KILLED_BY_AN);
            exercise(A_CON, FALSE);
        }
        if (Stoned) fix_petrification();
        unkn++; /* holy/unholy water can burn like acid too */
        break;
    case POT_POLYMORPH:
        You_feel("a little %s.", Hallucination ? "normal" : "strange");
        if (!Unchanging) polyself(FALSE);
        break;

    case POT_BLOOD:
    case POT_VAMPIRE_BLOOD:
        unkn++;
        u.uconduct.unvegan++;
        if (maybe_polyd(is_vampire(youmonst.data), Race_if(PM_VAMPIRE))) {
            violated_vegetarian();
            if (otmp->cursed)
                pline("Yecch!  This %s.", Hallucination ?
                      "liquid could do with a good stir" : "blood has congealed");
            else pline(Hallucination ?
                       "The %s liquid stirs memories of home." :
                       "The %s blood tastes delicious.",
                       otmp->odiluted ? "watery" : "thick");
            if (!otmp->cursed)
                lesshungry((otmp->odiluted ? 1 : 2) *
                           (otmp->otyp == POT_VAMPIRE_BLOOD ? 400 :
                            otmp->blessed ? 15 : 10));
            if (otmp->otyp == POT_VAMPIRE_BLOOD && otmp->blessed) {
                int num = newhp();
                if (Upolyd) {
                    u.mhmax += num;
                    u.mh += num;
                } else {
                    u.uhpmax += num;
                    u.uhp += num;
                }
            }
        } else if (otmp->otyp == POT_VAMPIRE_BLOOD) {
            /* [CWC] fix conducts for potions of (vampire) blood -
               doesn't use violated_vegetarian() to prevent
               duplicated "you feel guilty" messages */
            u.uconduct.unvegetarian++;
            if (u.ualign.type == A_LAWFUL || Role_if(PM_MONK)) {
                You_feel("%sguilty about drinking such a vile liquid.",
                         Role_if(PM_MONK) ? "especially " : "");
                u.ugangr++;
                adjalign(-15);
            } else if (u.ualign.type == A_NEUTRAL)
                adjalign(-3);
            exercise(A_CON, FALSE);
            if (Race_if(PM_VAMPIRE)) {
                if (!Unchanging) rehumanize();
                break;
            } else if (!Unchanging) {
                int successful_polymorph = FALSE;
                if (otmp->blessed)
                    successful_polymorph = polymon(PM_VAMPIRE_LORD);
                else if (otmp->cursed)
                    successful_polymorph = polymon(PM_VAMPIRE_BAT);
                else
                    successful_polymorph = polymon(PM_VAMPIRE);
                if (successful_polymorph)
                    u.mtimedone = 0; /* "Permament" change */
            }
        } else {
            violated_vegetarian();
            pline("Ugh.  That was vile.");
            make_vomiting(Vomiting+d(10, 8), TRUE);
        }
        break;

    default:
        impossible("What a funny potion! (%u)", otmp->otyp);
        return(0);
    }
    return(-1);
}

void
healup(nhp, nxtra, curesick, cureblind)
int nhp, nxtra;
register boolean curesick, cureblind;
{
    if (nhp && !marathon_mode) {
        if (Upolyd) {
            u.mh += nhp;
            if (u.mh > u.mhmax) u.mh = (u.mhmax += nxtra);
        } else {
            u.uhp += nhp;
            if(u.uhp > u.uhpmax) u.uhp = (u.uhpmax += nxtra);
        }
    }
    check_uhpmax();

    if (cureblind) {
        /* 3.6.1: it's debatible whether healing magic should clean off
           mundane 'dirt', but if it doesn't, blindness isn't cured */
        u.ucreamed = 0;
        make_blinded(0L, TRUE);
    }
    if (curesick) {
        make_vomiting(0L, TRUE);
        make_sick(0L, (char *) 0, TRUE, SICK_ALL);
    }
    flags.botl = 1;
}

void
strange_feeling(obj, txt)
register struct obj *obj;
register const char *txt;
{
    if (flags.beginner || !txt)
        You("have a %s feeling for a moment, then it passes.",
            Hallucination ? "normal" : "strange");
    else
        pline("%s", txt);

    if(!obj)    /* e.g., crystal ball finds no traps */
        return;

    if(obj->dknown && !objects[obj->otyp].oc_name_known &&
       !objects[obj->otyp].oc_uname)
        docall(obj);
    useup(obj);
}

const char *bottlenames[] = {
    "bottle", "phial", "flagon", "carafe", "flask", "jar", "vial"
};


const char *
bottlename()
{
    return bottlenames[rn2(SIZE(bottlenames))];
}

/* handle item dipped into water potion or steed saddle splashed by same */
STATIC_OVL boolean
H2Opotion_dip(potion, targobj, useeit, objphrase)
struct obj *potion, *targobj;
boolean useeit;
const char *objphrase; /* "Your widget glows" or "Steed's saddle glows" */
{
    void FDECL((*func), (OBJ_P)) = 0;
    const char *glowcolor = 0;
#define COST_alter (-2)
#define COST_none (-1)
    int costchange = COST_none;
    boolean altfmt = FALSE, res = FALSE;

    if (!potion || potion->otyp != POT_WATER) {
        return FALSE;
    }

    if (potion->blessed) {
        if (targobj->cursed) {
            func = uncurse;
            glowcolor = NH_AMBER;
            costchange = COST_UNCURS;
        } else if (!targobj->blessed) {
            func = bless;
            glowcolor = NH_LIGHT_BLUE;
            costchange = COST_alter;
            altfmt = TRUE; /* "with a <color> aura" */
        }
    } else if (potion->cursed) {
        if (targobj->blessed) {
            func = unbless;
            glowcolor = "brown";
            costchange = COST_UNBLSS;
        } else if (!targobj->cursed) {
            func = curse;
            glowcolor = NH_BLACK;
            costchange = COST_alter;
            altfmt = TRUE;
        }
    } else {
        /* dipping into uncursed water; carried() check skips steed saddle */
        if (carried(targobj)) {
            if (water_damage(targobj, 0, TRUE) != ER_NOTHING) {
                res = TRUE;
            }
        }
    }
    if (func) {
        /* give feedback before altering the target object;
           this used to set obj->bknown even when not seeing
           the effect; now hero has to see the glow, and bknown
           is cleared instead of set if perception is distorted */
        if (useeit) {
            glowcolor = hcolor(glowcolor);
            if (altfmt) {
                pline("%s with %s aura.", objphrase, an(glowcolor));
            } else {
                pline("%s %s.", objphrase, glowcolor);
            }
            iflags.last_msg = PLNMSG_OBJ_GLOWS;
            targobj->bknown = !Hallucination;
        } else {
            /* didn't see what happened:  forget the BUC state if that was
               known unless the bless/curse state of the water is known;
               without this, hero would know the new state even without
               seeing the glow; priest[ess] will immediately relearn it */
            if (!potion->bknown || !potion->dknown) {
                targobj->bknown = 0;
            }
            /* [should the bknown+dknown exception require that water
               be discovered or at least named?] */
        }
        /* potions of water are the only shop goods whose price depends
           on their curse/bless state */
        if (targobj->unpaid && targobj->otyp == POT_WATER) {
            if (costchange == COST_alter) {
                /* added blessing or cursing; update shop
                   bill to reflect item's new higher price */
                alter_cost(targobj, 0L);
            } else if (costchange != COST_none) {
                /* removed blessing or cursing; you
                   degraded it, now you'll have to buy it... */
                costly_alteration(targobj, costchange);
            }
        }
        /* finally, change curse/bless state */
        (*func)(targobj);
        res = TRUE;
    }
    return res;
}

/* potion obj hits monster mon, which might be youmonst; obj always used up */
void
potionhit(mon, obj, how)
struct monst *mon;
struct obj *obj;
int how;
{
    const char *botlnam = bottlename();
    boolean isyou = (mon == &youmonst);
    int distance;
    int tx, ty;
    struct obj *saddle = (struct obj *) 0;
    boolean hit_saddle = FALSE;
    boolean your_fault = (how <= POTHIT_HERO_THROW);
#ifdef WEBB_DISINT
    boolean disint = (touch_disintegrates(mon->data) &&
                      !oresist_disintegration(obj) &&
                      !mon->mcan &&
                      mon->mhp>6);
#endif

    if (isyou) {
        tx = u.ux, ty = u.uy;
        distance = 0;
        pline_The("%s crashes on your %s and breaks into shards.",
                  botlnam, body_part(HEAD));
        if (!heaven_or_hell_mode) {
            const char *message = (how == POTHIT_OTHER_THROW) ? "propelled potion" /* scatter */
                                                              : "thrown potion";
            losehp(Maybe_Half_Phys(rnd(2)), message, KILLED_BY_AN);
        } else {
            You_feel("as if something protected you.");
        }
    } else {
        tx = mon->mx, ty = mon->my;
        /* sometimes it hits the saddle */
        if (((mon->misc_worn_check & W_SADDLE) &&
             (saddle = which_armor(mon, W_SADDLE))) &&
            (!rn2(10) ||
             (obj->otyp == POT_WATER &&
              ((rnl(10) > 7 && obj->cursed) ||
               (rnl(10) < 4 && obj->blessed) || !rn2(3))))) {
            hit_saddle = TRUE;
        }
        distance = distu(mon->mx, mon->my);
#ifdef WEBB_DISINT
        if (!cansee(mon->mx, mon->my)) pline(disint ? "Vip!" : "Crash!");
#else
        if (!cansee(mon->mx, mon->my)) pline("Crash!");
#endif
        else {
            char *mnam = mon_nam(mon);
            char buf[BUFSZ];

            if (hit_saddle && saddle) {
                Sprintf(buf, "%s saddle",
                        s_suffix(x_monnam(mon, ARTICLE_THE, (char *) 0,
                                          (SUPPRESS_IT | SUPPRESS_SADDLE), FALSE)));
            } else if (has_head(mon->data)) {
                Sprintf(buf, "%s %s", s_suffix(mnam), (notonhead ? "body" : "head"));
            } else {
                Strcpy(buf, mnam);
            }
#ifdef WEBB_DISINT
            pline_The("%s crashes on %s and %s.",
                      botlnam, buf, disint ? "disintegrates" : "breaks into shards");
#else
            pline_The("%s crashes on %s breaks into shards.",
                      botlnam, buf);
#endif
        }
        if (rn2(5) && mon->mhp > 1 && !hit_saddle) {
            mon->mhp--;
        }
    }

    /* oil doesn't instantly evaporate; Neither does a saddle hit */
    if (obj->otyp != POT_OIL && !hit_saddle && cansee(tx, ty) && !disint) {
        pline("%s.", Tobjnam(obj, "evaporate"));
    }

    if (isyou) {
        switch (obj->otyp) {
        case POT_OIL:
            if (obj->lamplit)
                explode_oil(obj, u.ux, u.uy);
            break;
        case POT_POLYMORPH:
            You_feel("a little %s.", Hallucination ? "normal" : "strange");
            if (!Unchanging && !Antimagic) polyself(FALSE);
            break;
        case POT_ACID:
            if (!Acid_resistance) {
                pline("This burns%s!", obj->blessed ? " a little" :
                      obj->cursed ? " a lot" : "");
                int dmg = d(obj->cursed ? 2 : 1, obj->blessed ? 4 : 8);
                losehp(Maybe_Half_Phys(dmg), "potion of acid", KILLED_BY_AN);
            }
            break;
        }
    } else if (hit_saddle && saddle) {
        char *mnam, buf[BUFSZ], saddle_glows[BUFSZ];
        boolean affected = FALSE;
        boolean useeit = !Blind && canseemon(mon) && cansee(tx, ty);

        mnam = x_monnam(mon, ARTICLE_THE, (char *) 0, (SUPPRESS_IT | SUPPRESS_SADDLE), FALSE);
        Sprintf(buf, "%s", upstart(s_suffix(mnam)));

        switch (obj->otyp) {
        case POT_WATER:
            Snprintf(saddle_glows, sizeof(saddle_glows), "%s %s", buf, aobjnam(saddle, "glow"));
            affected = H2Opotion_dip(obj, saddle, useeit, saddle_glows);
            break;
        case POT_POLYMORPH:
            /* Do we allow the saddle to polymorph? */
            break;
        }
        if (useeit && !affected) {
            pline("%s %s wet.", buf, aobjnam(saddle, "get"));
        }
    } else {
        boolean angermon = your_fault;
        boolean cureblind = FALSE;

#ifdef WEBB_DISINT
        if (!disint)
#endif
        {
            switch (obj->otyp) {
            case POT_FULL_HEALING:
                cureblind = TRUE;
                /* fall through */
            case POT_EXTRA_HEALING:
                if (!obj->cursed) {
                    cureblind = TRUE;
                }
                /* fall through */
            case POT_HEALING:
                if (obj->blessed) {
                    cureblind = TRUE;
                }
                if (mon->data == &mons[PM_PESTILENCE]) {
                    goto do_illness;
                }
                /* fall through */
            case POT_RESTORE_ABILITY:
            case POT_GAIN_ABILITY:
do_healing:
                angermon = FALSE;
                if(mon->mhp < mon->mhpmax) {
                    mon->mhp = mon->mhpmax;
                    if (canseemon(mon))
                        pline("%s looks sound and hale again.", Monnam(mon));
                }
                if (cureblind) {
                    mcureblindness(mon, canseemon(mon));
                }
                break;
            case POT_SICKNESS:
                if (mon->data == &mons[PM_PESTILENCE]) goto do_healing;
                if (dmgtype(mon->data, AD_DISE) ||
                    dmgtype(mon->data, AD_PEST) || /* won't happen, see prior goto */
                    /* most common case */
                    resists_poison(mon)) {
                    if (canseemon(mon))
                        pline("%s looks unharmed.", Monnam(mon));
                    break;
                }
do_illness:
                if((mon->mhpmax > 3) && !resist(mon, POTION_CLASS, 0, NOTELL))
                    mon->mhpmax /= 2;
                if((mon->mhp > 2) && !resist(mon, POTION_CLASS, 0, NOTELL))
                    mon->mhp /= 2;
                if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
                if (canseemon(mon))
                    pline("%s looks rather ill.", Monnam(mon));
                break;
            case POT_CONFUSION:
            case POT_BOOZE:
                if(!resist(mon, POTION_CLASS, 0, NOTELL)) mon->mconf = TRUE;
                break;
            case POT_INVISIBILITY: {
                boolean sawit = canspotmon(mon);
                angermon = FALSE;
                mon_set_minvis(mon);
                if (sawit && !canspotmon(mon) && cansee(mon->mx, mon->my)) {
                    map_invisible(mon->mx, mon->my);
                }
                break;
            }
            case POT_SLEEPING:
                /* wakeup() doesn't rouse victims of temporary sleep */
                if (sleep_monst(mon, rnd(12), POTION_CLASS)) {
                    pline("%s falls asleep.", Monnam(mon));
                    slept_monst(mon);
                }
                break;
            case POT_PARALYSIS:
                if (mon->mcanmove) {
                    /* really should be rnd(5) for consistency with players
                     * breathing potions, but...
                     */
                    paralyze_monst(mon, rnd(25));
                }
                break;
            case POT_SPEED:
                angermon = FALSE;
                mon_adjust_speed(mon, 1, obj);
                break;
            case POT_BLINDNESS:
                if (haseyes(mon->data)) {
                    register int btmp = 64 + rn2(32) +
                                        rn2(32) * !resist(mon, POTION_CLASS, 0, NOTELL);
                    btmp += mon->mblinded;
                    mon->mblinded = min(btmp, 127);
                    mon->mcansee = 0;
                }
                break;
            case POT_WATER:
                if (is_undead(mon->data) ||
                     is_demon(mon->data) ||
                     is_were(mon->data) ||
                     is_vampshifter(mon)) {
                    if (obj->blessed) {
                        pline("%s %s in pain!", Monnam(mon),
                              is_silent(mon->data) ? "writhes" : "shrieks");
                        if (!is_silent(mon->data)) {
                            wake_nearto(tx, ty, mon->data->mlevel * 10);
                        }
                        mon->mhp -= d(2, 6);
                        /* should only be by you */
                        if (DEADMONSTER(mon)) {
                            killed(mon);
                        } else if (is_were(mon->data) && !is_human(mon->data)) {
                            new_were(mon); /* revert to human */
                        }
                    } else if (obj->cursed) {
                        angermon = FALSE;
                        if (canseemon(mon))
                            pline("%s looks healthier.", Monnam(mon));
                        mon->mhp += d(2, 6);
                        if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
                        if (is_were(mon->data) && is_human(mon->data) &&
                            !Protection_from_shape_changers)
                            new_were(mon); /* transform into beast */
                    }
                } else if(mon->data == &mons[PM_GREMLIN]) {
                    angermon = FALSE;
                    (void)split_mon(mon, (struct monst *)0);
                } else if(mon->data == &mons[PM_IRON_GOLEM]) {
                    if (canseemon(mon))
                        pline("%s rusts.", Monnam(mon));
                    mon->mhp -= d(1, 6);
                    /* should only be by you */
                    if (DEADMONSTER(mon)) {
                        killed(mon);
                    }
                }
                break;
            case POT_OIL:
                if (obj->lamplit) {
                    explode_oil(obj, tx, ty);
                }
                break;
            case POT_ACID:
                if (!resists_acid(mon) && !resist(mon, POTION_CLASS, 0, NOTELL)) {
                    pline("%s %s in pain!", Monnam(mon),
                          is_silent(mon->data) ? "writhes" : "shrieks");
                    if (!is_silent(mon->data)) {
                        wake_nearto(tx, ty, mon->data->mlevel * 10);
                    }
                    mon->mhp -= d(obj->cursed ? 2 : 1, obj->blessed ? 4 : 8);
                    if (DEADMONSTER(mon)) {
                        if (your_fault)
                            killed(mon);
                        else
                            monkilled(mon, "", AD_ACID);
                    }
                }
                break;
            case POT_POLYMORPH:
                (void) bhitm(mon, obj);
                break;
/*
    case POT_GAIN_LEVEL:
    case POT_LEVITATION:
    case POT_FRUIT_JUICE:
    case POT_MONSTER_DETECTION:
    case POT_OBJECT_DETECTION:
        break;
 */
            }
        }

        /* target might have been killed */
        if (!DEADMONSTER(mon)) {
            if (angermon)
                wakeup(mon, TRUE);
            else
                mon->msleeping = 0;
        }
    }
#ifdef WEBB_DISINT
    if (!disint)
#endif
    {

        /* Note: potionbreathe() does its own docall() */
        if ((distance == 0 || ((distance < 3) && rn2(5))) &&
            (!breathless(youmonst.data) || haseyes(youmonst.data))) {
            potionbreathe(obj);
        } else if (obj->dknown && !objects[obj->otyp].oc_name_known &&
                 !objects[obj->otyp].oc_uname && cansee(tx, ty)) {
            docall(obj);
        }
    }
    if (*u.ushops && obj->unpaid) {
        struct monst *shkp = shop_keeper(*in_rooms(u.ux, u.uy, SHOPBASE));

        /* neither of the first two cases should be able to happen;
        only the hero should ever have an unpaid item, and only
        when inside a tended shop */
        if (!shkp) {
            /* if shkp was killed, unpaid ought to cleared already */
            obj->unpaid = 0;
        } else if (flags.mon_moving) {
            /* obj thrown by monster */
            subfrombill(obj, shkp);
        } else {
            (void)stolen_value(obj, u.ux, u.uy,
                               (boolean)shkp->mpeaceful, FALSE);
            subfrombill(obj, shkp);
        }
    }
    obfree(obj, (struct obj *)0);
}

/* vapors are inhaled or get in your eyes */
void
potionbreathe(obj)
register struct obj *obj;
{
    register int i, ii, kn = 0;
    boolean cureblind = FALSE;

    /* potion of unholy water might be wielded; prevent
       you_were() -> drop_weapon() from dropping it so that it
       remains in inventory where our caller expects it to be */
    obj->in_use = 1;

    /* wearing a wet towel protects both eyes and breathing, even when
       the breath effect might be beneficial; we still pass down to the
       naming opportunity in case potion was thrown at hero by a monster */
    switch (Half_gas_damage ? TOWEL : obj->otyp) {
    case TOWEL:
        pline("Some vapor passes harmlessly around you.");
        break;
    case POT_RESTORE_ABILITY:
    case POT_GAIN_ABILITY:
        if (obj->cursed) {
            if (!breathless(youmonst.data))
                pline("Ulch!  That potion smells terrible!");
            else if (haseyes(youmonst.data)) {
                int numeyes = eyecount(youmonst.data);
                Your("%s sting%s!",
                     (numeyes == 1) ? body_part(EYE) : makeplural(body_part(EYE)),
                     (numeyes == 1) ? "s" : "");
            }
            break;
        } else {
            i = rn2(A_MAX);     /* start at a random point */
            for (ii = 0; ii < A_MAX; ii++) {
                if(ABASE(i) < AMAX(i)) {
                    ABASE(i)++;
                    flags.botl = 1;
                }
                if(++i >= A_MAX) i = 0;
            }
        }
        break;

    case POT_FULL_HEALING:
        if (Upolyd && u.mh < u.mhmax) u.mh++, flags.botl = 1;
        if (u.uhp < u.uhpmax) u.uhp++, flags.botl = 1;
        /* fall through */

    case POT_EXTRA_HEALING:
        if (Upolyd && u.mh < u.mhmax) u.mh++, flags.botl = 1;
        if (u.uhp < u.uhpmax) u.uhp++, flags.botl = 1;
        /* fall through */

    case POT_HEALING:
        if (Upolyd && u.mh < u.mhmax) u.mh++, flags.botl = 1;
        if (u.uhp < u.uhpmax) u.uhp++, flags.botl = 1;
        exercise(A_CON, TRUE);
        break;

    case POT_SICKNESS:
        kn++;
        if (!Role_if(PM_HEALER)) {
            if (Upolyd) {
                if (u.mh <= 5) u.mh = 1; else u.mh -= 5;
            } else {
                if (u.uhp <= 5) u.uhp = 1; else u.uhp -= 5;
            }
            flags.botl = 1;
            exercise(A_CON, FALSE);
        }
        break;

    case POT_HALLUCINATION:
        kn++;
        You("have a momentary vision.");
        break;

    case POT_CONFUSION:
    case POT_BOOZE:
        if(!Confusion)
            You_feel("somewhat dizzy.");
        make_confused(itimeout_incr(HConfusion, rnd(5)), FALSE);
        break;

    case POT_INVISIBILITY:
        if (!Blind && !Invis) {
            kn++;
            pline("For an instant you %s!",
                  See_invisible ? "could see right through yourself"
                  : "couldn't see yourself");
        }
        break;

    case POT_PARALYSIS:
        kn++;
        if (!Free_action) {
            pline("%s seems to be holding you.", Something);
            nomul(-rnd(5), "frozen by a potion");
            nomovemsg = You_can_move_again;
            exercise(A_DEX, FALSE);
        } else You("stiffen momentarily.");
        break;

    case POT_SLEEPING:
        kn++;
        if (!Free_action && !Sleep_resistance) {
            You_feel("rather tired.");
            nomul(-rnd(5), "sleeping off a magical draught");
            nomovemsg = You_can_move_again;
            exercise(A_DEX, FALSE);
        } else You("yawn.");
        break;

    case POT_SPEED:
        if (!Fast) {
            kn++;
            Your("knees seem more flexible now.");
        }
        incr_itimeout(&HFast, rnd(5));
        exercise(A_DEX, TRUE);
        break;

    case POT_BLINDNESS:
        if (!Blind && !Unaware) {
            kn++;
            pline("It suddenly gets dark.");
        }
        make_blinded(itimeout_incr(Blinded, rnd(5)), FALSE);
        if (!Blind && !u.usleep) Your("%s", vision_clears);
        break;

    case POT_WATER:
        if (u.umonnum == PM_GREMLIN) {
            (void)split_mon(&youmonst, (struct monst *)0);
        } else if (u.ulycn >= LOW_PM) {
            /* vapor from [un]holy water will trigger
               transformation but won't cure lycanthropy */
            if (obj->blessed && youmonst.data == &mons[u.ulycn])
                you_unwere(FALSE);
            else if (obj->cursed && !Upolyd)
                you_were();
        }
        break;

    case POT_ACID:
    case POT_POLYMORPH:
        exercise(A_CON, FALSE);
        break;

    case POT_BLOOD:
    case POT_VAMPIRE_BLOOD:
        if (maybe_polyd(is_vampire(youmonst.data), Race_if(PM_VAMPIRE))) {
            exercise(A_WIS, FALSE);
            You_feel("a %ssense of loss.",
                     obj->otyp == POT_VAMPIRE_BLOOD ? "terrible " : "");
        } else
            exercise(A_CON, FALSE);
        break;
/*
    case POT_GAIN_LEVEL:
    case POT_LEVITATION:
    case POT_FRUIT_JUICE:
    case POT_MONSTER_DETECTION:
    case POT_OBJECT_DETECTION:
    case POT_OIL:
        break;
 */
    }
    /* note: no obfree() */
    if (obj->dknown) {
        if (kn)
            makeknown(obj->otyp);
        else if (!objects[obj->otyp].oc_name_known &&
                 !objects[obj->otyp].oc_uname)
            docall(obj);
    }
}

/* new alchemy scheme based on color mixing
 * YANI by Graham Cox <aca00gac@shef.ac.uk>
 * Implemented by Nephi Allred <zindorsky@hotmail.com> on 15 Apr 2003
 *
 *  Alchemical tables are based on 4 bits describing dark/light level, yellow, blue and red
 *
 *  DYBR
 *  0000    white
 *  0001    pink
 *  0010    sky-blue
 *  0011    puce
 *  0100    yellow
 *  0101    orange
 *  0110    emerald
 *  0111    ochre
 *  1000    black
 *  1001    ruby
 *  1010    indigo
 *  1011    magenta
 *  1100    golden
 *  1101    amber
 *  1110    dark green
 *  1111    brown
 */

/* Assumes gain ability is first potion and vampire blood is last */
char alchemy_table1[POT_VAMPIRE_BLOOD - POT_GAIN_ABILITY + 1];
short alchemy_table2[17];

#define ALCHEMY_WHITE 0
#define ALCHEMY_BLACK 8
#define ALCHEMY_GRAY (alchemy_table2[16])
#define IS_PRIMARY_COLOR(x)     (((x)&7)==1 || ((x)&7)==2 || ((x)&7)==4)
#define IS_SECONDARY_COLOR(x)   (((x)&7)==3 || ((x)&7)==5 || ((x)&7)==6)
#define IS_LIGHT_COLOR(x)       (((x)&8)==0)
#define IS_DARK_COLOR(x)        ((x)&8)

/** Does a one-time set up of alchemical tables. */
STATIC_OVL void
alchemy_init()
{
    static boolean init = FALSE;

    if (!init) {
        short i;
        const char* potion_desc;

        for (i = POT_GAIN_ABILITY; i <= POT_VAMPIRE_BLOOD; i++) {
            potion_desc = OBJ_DESCR(objects[i]);
            if (0==strcmp(potion_desc, "white")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=0;
                alchemy_table2[0]=i;
            } else if (0==strcmp(potion_desc, "pink")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=1;
                alchemy_table2[1]=i;
            } else if (0==strcmp(potion_desc, "sky blue")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=2;
                alchemy_table2[2]=i;
            } else if (0==strcmp(potion_desc, "puce")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=3;
                alchemy_table2[3]=i;
            } else if (0==strcmp(potion_desc, "yellow")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=4;
                alchemy_table2[4]=i;
            } else if (0==strcmp(potion_desc, "orange")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=5;
                alchemy_table2[5]=i;
            } else if (0==strcmp(potion_desc, "emerald")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=6;
                alchemy_table2[6]=i;
            } else if (0==strcmp(potion_desc, "ochre")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=7;
                alchemy_table2[7]=i;
            } else if (0==strcmp(potion_desc, "black")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=8;
                alchemy_table2[8]=i;
            } else if (0==strcmp(potion_desc, "ruby")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=9;
                alchemy_table2[9]=i;
            } else if (0==strcmp(potion_desc, "indigo")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=10;
                alchemy_table2[10]=i;
            } else if (0==strcmp(potion_desc, "magenta")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=11;
                alchemy_table2[11]=i;
            } else if (0==strcmp(potion_desc, "golden")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=12;
                alchemy_table2[12]=i;
            } else if (0==strcmp(potion_desc, "amber")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=13;
                alchemy_table2[13]=i;
            } else if (0==strcmp(potion_desc, "dark green")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=14;
                alchemy_table2[14]=i;
            } else if (0==strcmp(potion_desc, "brown")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=15;
                alchemy_table2[15]=i;
            } else if (0==strcmp(potion_desc, "silver")) {
                alchemy_table1[i-POT_GAIN_ABILITY]=-1;
                alchemy_table2[16]=i;
            } else {
                alchemy_table1[i-POT_GAIN_ABILITY]=-1;
            }
        }
        init = TRUE;
    }
}

/** Returns true if the potion is a dark colored potion. */
boolean
is_dark_mix_color(struct obj *potion)
{
    alchemy_init();
    int alchemy_index = alchemy_table1[potion->otyp - POT_GAIN_ABILITY];
    if (alchemy_index >= 0 && IS_DARK_COLOR(alchemy_index)) {
        return TRUE;
    }

    return FALSE;
}

/** Returns true if the potion is a colorless potion with regards to color alchemy. */
boolean
is_colorless_mix_potion(struct obj *potion)
{
    if (potion->otyp >= POT_WATER) {
        return TRUE;
    }
    alchemy_init();
    if (alchemy_table1[potion->otyp - POT_GAIN_ABILITY] < 0) {
        return TRUE;
    }

    return FALSE;
}

/** Returns true if the potion is a colorless potion with regards to color alchemy. */
const char*
get_base_mix_color(struct obj *potion)
{
    const char *desc = OBJ_DESCR(objects[potion->otyp]);

    if (!strcmp(desc, "pink") || !strcmp(desc, "ruby")) {
        return "red";
    } else if (!strcmp(desc, "sky blue") || !strcmp(desc, "indigo")) {
        return "blue";
    } else if (!strcmp(desc, "yellow") || !strcmp(desc, "golden")) {
        return "yellow";
    } else if (!strcmp(desc, "orange") || !strcmp(desc, "amber")) {
        return "orange";
    } else if (!strcmp(desc, "emerald") || !strcmp(desc, "dark green")) {
        return "green";
    } else if (!strcmp(desc, "puce") || !strcmp(desc, "magenta")) {
        return "purple";
    } else if (!strcmp(desc, "ochre") || !strcmp(desc, "brown")) {
        return "brown";
    }

    return "colorless";
}

/** Returns the potion type when object o1 is dipped into object o2 */
STATIC_OVL short
mixtype(o1, o2)
register struct obj *o1, *o2;
{
    /* cut down on the number of cases below */
    if (o1->oclass == POTION_CLASS &&
        (o2->otyp == POT_FRUIT_JUICE)) {
        struct obj *swp;

        swp = o1; o1 = o2; o2 = swp;
    }
    switch (o1->otyp) {
    case POT_FRUIT_JUICE:
        switch (o2->otyp) {
        case POT_BLOOD:
            return POT_BLOOD;
        case POT_VAMPIRE_BLOOD:
            return POT_VAMPIRE_BLOOD;
        }
        break;
    }

    if (o1->oclass == POTION_CLASS) {
        int i1, i2, result;

        alchemy_init();
        i1 = alchemy_table1[o1->otyp-POT_GAIN_ABILITY];
        i2 = alchemy_table1[o2->otyp-POT_GAIN_ABILITY];

        /* check that both potions are of mixable types */
        if (i1<0 || i2<0)
            return 0;

        /* swap for simplified checks */
        if (i2==ALCHEMY_WHITE || (i2==ALCHEMY_BLACK && i1!=ALCHEMY_WHITE)) {
            result = i1;
            i1 = i2;
            i2 = result;
        }

        if (i1==ALCHEMY_WHITE && i2==ALCHEMY_BLACK) {
            if (OBJ_NAME(objects[ALCHEMY_GRAY]) == 0) {
                /* silver potions don't exist in this game */
                return 0;
            } else {
                return ALCHEMY_GRAY;
            }
        } else if ( (IS_PRIMARY_COLOR(i1) && IS_PRIMARY_COLOR(i2))
                    || (IS_SECONDARY_COLOR(i1) && IS_SECONDARY_COLOR(i2)) ) {
            /* bitwise OR simulates pigment addition */
            result = i1 | i2;
            /* adjust light/dark level if necessary */
            if ((i1^i2)&8) {
                if (o1->odiluted==o2->odiluted) {
                    /* same dilution level, randomly toggle */
                    result ^= (rn2(2)<<3);
                } else {
                    /* use dark/light level of undiluted potion */
                    result ^= (o1->odiluted ? i1 : i2)&8;
                }
            }
        } else if ((i1==ALCHEMY_WHITE && IS_DARK_COLOR(i2)) ||
                   (i1==ALCHEMY_BLACK && IS_LIGHT_COLOR(i2))) {
            /* toggle light/dark bit */
            result = i2 ^ 8;
        } else {
            return 0;
        }
        if (OBJ_NAME(objects[alchemy_table2[result]]) == 0) {
            /* mixed potion doesn't exist in this game */
            return 0;
        } else {
            return alchemy_table2[result];
        }
    } else {
        switch (o1->otyp) {
        case UNICORN_HORN:
            switch (o2->otyp) {
            case POT_SICKNESS:
                return POT_FRUIT_JUICE;
            case POT_HALLUCINATION:
            case POT_BLINDNESS:
            case POT_CONFUSION:
            case POT_BLOOD:
            case POT_VAMPIRE_BLOOD:
                return POT_WATER;
            }
            break;
        case AMETHYST:          /* "a-methyst" == "not intoxicated" */
            if (o2->otyp == POT_BOOZE)
                return POT_FRUIT_JUICE;
            break;
        }
    }

    return 0;
}


boolean
get_wet(obj)
register struct obj *obj;
/* returns TRUE if something happened (potion should be used up) */
{
    char Your_buf[BUFSZ];

    if (snuff_lit(obj)) return(TRUE);

    if (obj->greased) {
        grease_protect(obj, (char *)0, &youmonst);
        return(FALSE);
    }
    (void) Shk_Your(Your_buf, obj);
    /* (Rusting shop goods ought to be charged for.) */
    switch (obj->oclass) {
    case POTION_CLASS:
        if (obj->otyp == POT_WATER) return FALSE;
        /* KMH -- Water into acid causes an explosion */
        if (obj->otyp == POT_ACID) {
            pline("It boils vigorously!");
            You("are caught in the explosion!");
            losehp(Acid_resistance ? rnd(5) : rnd(10),
                   "elementary chemistry", KILLED_BY);
            makeknown(obj->otyp);
            update_inventory();
            return (TRUE);
        }
        pline("%s %s%s.", Your_buf, aobjnam(obj, "dilute"),
              obj->odiluted ? " further" : "");
        if(obj->unpaid && costly_spot(u.ux, u.uy)) {
            You("dilute it, you pay for it.");
            bill_dummy_object(obj);
        }
        if (obj->odiluted) {
            obj->odiluted = 0;
#ifdef UNIXPC
            obj->blessed = FALSE;
            obj->cursed = FALSE;
#else
            obj->blessed = obj->cursed = FALSE;
#endif
            obj->otyp = POT_WATER;
        } else obj->odiluted++;
        update_inventory();
        return TRUE;
    case SCROLL_CLASS:
        if (obj->otyp != SCR_BLANK_PAPER && obj->otyp != SCR_FLOOD
#ifdef MAIL
            && obj->otyp != SCR_MAIL
#endif
            ) {
            if (!Blind) {
                boolean oq1 = obj->quan == 1L;
                pline_The("scroll%s %s.",
                          oq1 ? "" : "s", otense(obj, "fade"));
            }
            if(obj->unpaid && costly_spot(u.ux, u.uy)) {
                You("erase it, you pay for it.");
                bill_dummy_object(obj);
            }
            obj->otyp = SCR_BLANK_PAPER;
            obj->spe = 0;
            update_inventory();
            return TRUE;
        } else break;
    case SPBOOK_CLASS:
        if (obj->otyp != SPE_BLANK_PAPER) {

            if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
                pline("%s suddenly heats up; steam rises and it remains dry.",
                      The(xname(obj)));
            } else {
                if (!Blind) {
                    boolean oq1 = obj->quan == 1L;
                    pline_The("spellbook%s %s.",
                              oq1 ? "" : "s", otense(obj, "fade"));
                }
                if(obj->unpaid && costly_spot(u.ux, u.uy)) {
                    You("erase it, you pay for it.");
                    bill_dummy_object(obj);
                }
                obj->otyp = SPE_BLANK_PAPER;
                update_inventory();
            }
            return TRUE;
        }
        break;
    case WEAPON_CLASS:
    /* Just "fall through" to generic rustprone check for now. */
    /* fall through */
    default:
        if (!obj->oerodeproof && is_rustprone(obj) &&
            (obj->oeroded < MAX_ERODE) && !rn2(2)) {
            pline("%s %s some%s.",
                  Your_buf, aobjnam(obj, "rust"),
                  obj->oeroded ? " more" : "what");
            obj->oeroded++;
            update_inventory();
            return TRUE;
        } else break;
    }
    pline("%s %s wet.", Your_buf, aobjnam(obj, "get"));
    return FALSE;
}

/** User command for dipping objects. */
int
dodip()
{
    static const char Dip_[] = "Dip ";
    register struct obj *potion, *obj;
    struct obj *singlepotion;
    uchar here;
    char allowall[2];
    short mixture;
    char qbuf[QBUFSZ], obuf[QBUFSZ];
    const char *shortestname; /* last resort obj name for prompt */

    allowall[0] = ALL_CLASSES; allowall[1] = '\0';
    if (!(obj = getobj(allowall, "dip"))) {
        return(0);
    }
    if (inaccessible_equipment(obj, "dip", FALSE)) {
        return 0;
    }

    shortestname = (is_plural(obj) || pair_of(obj)) ? "them" : "it";

    /*
     * Bypass safe_qbuf() since it doesn't handle varying suffix without
     * an awful lot of support work.  Format the object once, even though
     * the fountain and pool prompts offer a lot more room for it.
     * 3.6.0 used thesimpleoname() unconditionally, which posed no risk
     * of buffer overflow but drew bug reports because it omits user-
     * supplied type name.
     * getobj: "What do you want to dip <the object> into? [xyz or ?*] "
     */
    Strcpy(obuf, short_oname(obj, doname, thesimpleoname,
                             /* 128 - (24 + 54 + 1) leaves 49 for <object> */
                             QBUFSZ - sizeof "What do you want to dip \
 into? [abdeghjkmnpqstvwyzBCEFHIKLNOQRTUWXZ#-# or ?*] "));

    here = levl[u.ux][u.uy].typ;

    /* Is there a fountain to dip into here? */
    if (IS_FOUNTAIN(here)) {
        Sprintf(qbuf, "%s%s into the fountain?", Dip_,
                flags.verbose ? obuf : shortestname);
        /* "Dip <the object> into the fountain?" */
        if (yn(qbuf) == 'y') {
            dipfountain(obj);
            return(1);
        }
    } else if (is_pool(u.ux, u.uy) || is_swamp(u.ux, u.uy)) {
        const char *pooltype = waterbody_name(u.ux, u.uy);

        Sprintf(qbuf, "%s%s into the %s?", Dip_,
                flags.verbose ? obuf : shortestname, pooltype);
        /* "Dip <the object> into the {pool, moat, &c}?" */
        if (yn(qbuf) == 'y') {
            if (Levitation) {
                floating_above(pooltype);
#ifdef STEED
            } else if (u.usteed && !is_swimmer(u.usteed->data) &&
                       P_SKILL(P_RIDING) < P_BASIC) {
                rider_cant_reach(); /* not skilled enough to reach */
#endif
            } else {
                (void) get_wet(obj);
                if (obj->otyp == POT_ACID) useup(obj);
#if 0
                if (obj->otyp == POT_ACID) {
                    obj->in_use = 1;
                }
                if (water_damage(obj, 0, TRUE) != ER_DESTROYED && obj->in_use) {
                    useup(obj);
                }
#endif
            }
            return 1;
        }
    }

    /* "What do you want to dip <the object> into? [xyz or ?*] " */
    Sprintf(qbuf, "dip %s into", flags.verbose ? obuf : shortestname);
    potion = getobj(beverages, qbuf);
    if (!potion) {
        return(0);
    }
    if (potion == obj && potion->quan == 1L) {
        pline("That is a potion bottle, not a Klein bottle!");
        return 0;
    }

    return dip(potion, obj);
}

/** Dip an arbitrary object into a potion. */
int
dip(potion, obj)
struct obj *potion, *obj;
{
    struct obj *singlepotion;
    const char *tmp;
    short mixture;
    char Your_buf[BUFSZ];

    potion->in_use = TRUE;      /* assume it will be used up */
    if (potion->otyp == POT_WATER) {
        boolean useeit = !Blind;
        if (useeit) (void) Shk_Your(Your_buf, obj);
        if (potion->blessed) {
            if (obj->cursed) {
                if (useeit)
                    pline("%s %s %s.",
                          Your_buf,
                          aobjnam(obj, "softly glow"),
                          hcolor(NH_AMBER));
                uncurse(obj);
                obj->bknown=1;

                goto poof;
            } else if(!obj->blessed) {
                if (useeit) {
                    tmp = hcolor(NH_LIGHT_BLUE);
                    pline("%s %s with a%s %s aura.",
                          Your_buf,
                          aobjnam(obj, "softly glow"),
                          index(vowels, *tmp) ? "n" : "", tmp);
                }
                bless(obj);
                obj->bknown=1;
                goto poof;
            }
        } else if (potion->cursed) {
            if (obj->blessed) {
                if (useeit)
                    pline("%s %s %s.",
                          Your_buf,
                          aobjnam(obj, "glow"),
                          hcolor((const char *)"brown"));
                unbless(obj);
                obj->bknown=1;
                goto poof;
            } else if(!obj->cursed) {
                if (useeit) {
                    tmp = hcolor(NH_BLACK);
                    pline("%s %s with a%s %s aura.",
                          Your_buf,
                          aobjnam(obj, "glow"),
                          index(vowels, *tmp) ? "n" : "", tmp);
                }
                curse(obj);
                obj->bknown=1;
                goto poof;
            }
        } else
        if (get_wet(obj))
            goto poof;
    } else if (obj->otyp == POT_POLYMORPH ||
               potion->otyp == POT_POLYMORPH) {
        /* some objects can't be polymorphed */
        if (obj->otyp == potion->otyp ||    /* both POT_POLY */
            obj->otyp == WAN_POLYMORPH ||
            obj->otyp == SPE_POLYMORPH ||
            obj->otyp == AMULET_OF_UNCHANGING ||
            obj == uball || obj == uskin ||
            obj_resists(obj->otyp == POT_POLYMORPH ?
                        potion : obj, 5, 95)) {
            pline("%s", nothing_happens);
        } else {
            boolean was_wep = FALSE, was_swapwep = FALSE, was_quiver = FALSE;
            short save_otyp = obj->otyp;
            /* KMH, conduct */
            u.uconduct.polypiles++;

            if (obj == uwep) was_wep = TRUE;
            else if (obj == uswapwep) was_swapwep = TRUE;
            else if (obj == uquiver) was_quiver = TRUE;

            obj = poly_obj(obj, STRANGE_OBJECT);

            if (was_wep) setuwep(obj);
            else if (was_swapwep) setuswapwep(obj);
            else if (was_quiver) setuqwep(obj);

            if (obj->otyp != save_otyp) {
                makeknown(POT_POLYMORPH);
                useup(potion);
                prinv((char *)0, obj, 0L);
                return 1;
            } else {
                pline("Nothing seems to happen.");
                goto poof;
            }
        }
        potion->in_use = FALSE; /* didn't go poof */
        return(1);
    } else if(obj->oclass == POTION_CLASS && obj->otyp != potion->otyp) {
        /* Mixing potions is dangerous... */
        /* Give a clue to what's going on ... */
        if(potion->dknown && obj->dknown) {
            You("mix the %s potion with the %s one ...",
                OBJ_DESCR(objects[potion->otyp]),
                OBJ_DESCR(objects[obj->otyp]));
        } else
            pline_The("potions mix...");
        /* KMH, balance patch -- acid is particularly unstable */
        if (obj->cursed || obj->otyp == POT_ACID ||
            potion->otyp == POT_ACID ||
            !rn2(10)) {
            pline("BOOM!  They explode!");
            exercise(A_STR, FALSE);
            if (!breathless(youmonst.data) || haseyes(youmonst.data))
                potionbreathe(obj);
            useup(obj);
            useup(potion);
            losehp(Acid_resistance ? rnd(5) : rnd(10),
                   "alchemic blast", KILLED_BY_AN);
            return(1);
        }

        obj->blessed = obj->cursed = obj->bknown = 0;
        if (Blind || Hallucination) obj->dknown = 0;

        if ((mixture = mixtype(obj, potion)) != 0) {
            obj->otyp = mixture;
        } else {
            switch (obj->odiluted ? 1 : rnd(8)) {
            case 1:
                obj->otyp = POT_WATER;
                break;
            case 2:
            case 3:
                obj->otyp = POT_SICKNESS;
                break;
            case 4:
            {
                struct obj *otmp;
                otmp = mkobj(POTION_CLASS, FALSE);
                obj->otyp = otmp->otyp;
                obfree(otmp, (struct obj *)0);
            }
            break;
            default:
                if (!Blind)
                    pline_The("mixture glows brightly and evaporates.");
                useup(obj);
                useup(potion);
                return(1);
            }
        }

        obj->odiluted = (obj->otyp != POT_WATER);

        if (OBJ_NAME(objects[obj->otyp]) == 0) {
            panic("dipping created an inexistant potion (%d)", obj->otyp);
        }

        if (obj->otyp == POT_WATER && !Hallucination) {
            pline_The("mixture bubbles%s.",
                      Blind ? "" : ", then clears");
        } else if (!Blind) {
            pline_The("mixture looks %s.",
                      hcolor(OBJ_DESCR(objects[obj->otyp])));
        }

        useup(potion);
        return(1);
    }

#ifdef INVISIBLE_OBJECTS
    if (potion->otyp == POT_INVISIBILITY && !obj->oinvis) {
        obj->oinvis = TRUE;
        if (!Blind) {
            if (!See_invisible) pline("Where did %s go?",
                                      the(xname(obj)));
            else You("notice a little haziness around %s.",
                     the(xname(obj)));
        }
        goto poof;
    } else if (potion->otyp == POT_SEE_INVISIBLE && obj->oinvis) {
        obj->oinvis = FALSE;
        if (!Blind) {
            if (!See_invisible) pline("So that's where %s went!",
                                      the(xname(obj)));
            else pline_The("haziness around %s disappears.",
                           the(xname(obj)));
        }
        goto poof;
    }
#endif
    if (potion->otyp == POT_ACID && obj->otyp == CORPSE &&
        obj->corpsenm == PM_LICHEN && !Blind) {
        pline("%s %s %s around the edges.", The(cxname(obj)),
              otense(obj, "turn"),
              potion->odiluted ? hcolor(NH_ORANGE) : hcolor(NH_RED));
        makeknown(POT_ACID);
        potion->in_use = FALSE; /* didn't go poof */
        return(1);
    }

    if(is_poisonable(obj)) {
        if(potion->otyp == POT_SICKNESS && !obj->opoisoned) {
            char buf[BUFSZ];
            if (potion->quan > 1L)
                Sprintf(buf, "One of %s", the(xname(potion)));
            else
                Strcpy(buf, The(xname(potion)));
            pline("%s forms a coating on %s.",
                  buf, the(xname(obj)));
            obj->opoisoned = TRUE;
            goto poof;
        } else if(obj->opoisoned &&
                  (potion->otyp == POT_HEALING ||
                   potion->otyp == POT_EXTRA_HEALING ||
                   potion->otyp == POT_FULL_HEALING)) {
            pline("A coating wears off %s.", the(xname(obj)));
            obj->opoisoned = 0;
            goto poof;
        }
    }

    if (potion->otyp == POT_OIL) {
        boolean wisx = FALSE;
        if (potion->lamplit) {  /* burning */
            int omat = objects[obj->otyp].oc_material;
            /* the code here should be merged with fire_damage */
            if (catch_lit(obj)) {
                /* catch_lit does all the work if true */
            } else if (obj->oerodeproof || obj_resists(obj, 5, 95) ||
                       !is_flammable(obj) || obj->oclass == FOOD_CLASS) {
                pline("%s %s to burn for a moment.",
                      Yname2(obj), otense(obj, "seem"));
            } else {
                if ((omat == PLASTIC || omat == PAPER) && !obj->oartifact)
                    obj->oeroded = MAX_ERODE;
                pline_The("burning oil %s %s.",
                          obj->oeroded == MAX_ERODE ? "destroys" : "damages",
                          yname(obj));
                if (obj->oeroded == MAX_ERODE) {
                    setnotworn(obj);
                    obj_extract_self(obj);
                    obfree(obj, (struct obj *)0);
                    obj = (struct obj *) 0;
                } else {
                    /* we know it's carried */
                    if (obj->unpaid) {
                        /* create a dummy duplicate to put on bill */
                        verbalize("You burnt it, you bought it!");
                        bill_dummy_object(obj);
                    }
                    obj->oeroded++;
                }
            }
        } else if (potion->cursed) {
            pline_The("potion spills and covers your %s with oil.",
                      makeplural(body_part(FINGER)));
            incr_itimeout(&Glib, d(2, 10));
        } else if (obj->oclass != WEAPON_CLASS && !is_weptool(obj)) {
            /* the following cases apply only to weapons */
            goto more_dips;
            /* Oil removes rust and corrosion, but doesn't unburn.
             * Arrows, etc are classed as metallic due to arrowhead
             * material, but dipping in oil shouldn't repair them.
             */
        } else if ((!is_rustprone(obj) && !is_corrodeable(obj)) ||
                   is_ammo(obj) || (!obj->oeroded && !obj->oeroded2)) {
            /* uses up potion, doesn't set obj->greased */
            pline("%s %s with an oily sheen.",
                  Yname2(obj), otense(obj, "gleam"));
        } else {
            pline("%s %s less %s.",
                  Yname2(obj), otense(obj, "are"),
                  (obj->oeroded && obj->oeroded2) ? "corroded and rusty" :
                  obj->oeroded ? "rusty" : "corroded");
            if (obj->oeroded > 0) obj->oeroded--;
            if (obj->oeroded2 > 0) obj->oeroded2--;
            wisx = TRUE;
        }
        exercise(A_WIS, wisx);
        makeknown(potion->otyp);
        useup(potion);
        return 1;
    }
more_dips:

    /* Allow filling of MAGIC_LAMPs to prevent identification by player */
    if ((obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP) &&
        (potion->otyp == POT_OIL)) {
        /* Turn off engine before fueling, turn off fuel too :-)  */
        if (obj->lamplit || potion->lamplit) {
            useup(potion);
            explode(u.ux, u.uy, 11, d(6, 6), 0, EXPL_FIERY);
            exercise(A_WIS, FALSE);
            return 1;
        }
        /* Adding oil to an empty magic lamp renders it into an oil lamp */
        if ((obj->otyp == MAGIC_LAMP) && obj->spe == 0) {
            obj->otyp = OIL_LAMP;
            obj->age = 0;
        }
        if (obj->age > MAX_LAMP_FUEL*2/3) {
            pline("%s %s full.", Yname2(obj), otense(obj, "are"));
            potion->in_use = FALSE; /* didn't go poof */
        } else {
            You("fill %s with oil.", yname(obj));
            check_unpaid(potion); /* Yendorian Fuel Tax */
            /* burns more efficiently in a lamp than in a bottle;
               diluted potion provides less benefit but we don't attempt
               to track that the lamp now also has some non-oil in it */
            obj->age += (!potion->odiluted ? 4L : 3L) * potion->age / 2L;
            if (obj->age > MAX_LAMP_FUEL) {
                obj->age = MAX_LAMP_FUEL;
            }
            useup(potion);
            exercise(A_WIS, TRUE);
        }
        makeknown(POT_OIL);
        obj->spe = 1;
        update_inventory();
        return 1;
    }

    potion->in_use = FALSE;     /* didn't go poof */
    if ((obj->otyp == UNICORN_HORN || obj->otyp == AMETHYST) &&
        (mixture = mixtype(obj, potion)) != 0) {
        char oldbuf[BUFSZ], newbuf[BUFSZ];
        short old_otyp = potion->otyp;
        boolean old_dknown = FALSE;
        boolean more_than_one = potion->quan > 1;

        oldbuf[0] = '\0';
        if (potion->dknown) {
            old_dknown = TRUE;
            Sprintf(oldbuf, "%s ",
                    hcolor(OBJ_DESCR(objects[potion->otyp])));
        }
        /* with multiple merged potions, split off one and
           just clear it */
        if (potion->quan > 1L) {
            singlepotion = splitobj(potion, 1L);
        } else singlepotion = potion;

        costly_alteration(singlepotion, COST_NUTRLZ);
        singlepotion->otyp = mixture;
        singlepotion->blessed = 0;
        if (mixture == POT_WATER)
            singlepotion->cursed = singlepotion->odiluted = 0;
        else
            singlepotion->cursed = obj->cursed; /* odiluted left as-is */
        singlepotion->bknown = FALSE;
        if (Blind) {
            singlepotion->dknown = FALSE;
        } else {
            singlepotion->dknown = !Hallucination;
            *newbuf = '\0';
            if (mixture == POT_WATER && singlepotion->dknown) {
                Sprintf(newbuf, "clears");
            } else if (!Blind) {
                Sprintf(newbuf, "turns %s",
                        hcolor(OBJ_DESCR(objects[mixture])));
            }
            if (*newbuf) {
                pline_The("%spotion%s %s.", oldbuf,
                        more_than_one ? " that you dipped into" : "",
                        newbuf);
            } else {
                pline("Something happens.");
            }
            if (!objects[old_otyp].oc_uname &&
                !objects[old_otyp].oc_name_known && old_dknown) {
                struct obj fakeobj;

                fakeobj = zeroobj;
                fakeobj.dknown = 1;
                fakeobj.otyp = old_otyp;
                fakeobj.oclass = POTION_CLASS;
                docall(&fakeobj);
            }
        }
        obj_extract_self(singlepotion);
        singlepotion = hold_another_object(singlepotion,
                                           "You juggle and drop %s!",
                                           doname(singlepotion), (const char *)0);
        nhUse(singlepotion);
        update_inventory();
        return(1);
    }

    pline("Interesting...");
    return(1);

 poof:
    if (potion->dknown &&
         !objects[potion->otyp].oc_name_known &&
         !objects[potion->otyp].oc_uname) {
        docall(potion);
    }
    useup(potion);
    return 1;
}

/* *monp grants a wish and then leaves the game */
void
mongrantswish(monp)
struct monst **monp;
{
    struct monst *mon = *monp;
    int mx = mon->mx, my = mon->my, glyph = glyph_at(mx, my);

    /* remove the monster first in case wish proves to be fatal
       (blasted by artifact), to keep it out of resulting bones file */
    mongone(mon);
    *monp = 0; /* inform caller that monster is gone */
    /* hide that removal from player--map is visible during wish prompt */
    tmp_at(DISP_ALWAYS, glyph);
    tmp_at(mx, my);
    /* grant the wish */
    makewish(FALSE);
    /* clean up */
    tmp_at(DISP_END, 0);
}


void
djinni_from_bottle(obj)
register struct obj *obj;
{
    struct monst *mtmp;
    int chance;

    if(!(mtmp = makemon(&mons[PM_DJINNI], u.ux, u.uy, NO_MM_FLAGS))) {
        pline("It turns out to be empty.");
        return;
    }

    if (!Blind) {
        pline("In a cloud of smoke, %s emerges!", a_monnam(mtmp));
        pline("%s speaks.", Monnam(mtmp));
    } else {
        You("smell acrid fumes.");
        pline("%s speaks.", Something);
    }

    chance = rn2(5);
    if (obj->blessed) chance = (chance == 4) ? rnd(4) : 0;
    else if (obj->cursed) chance = (chance == 0) ? rn2(4) : 4;
    /* 0,1,2,3,4:  b=80%,5,5,5,5; nc=20%,20,20,20,20; c=5%,5,5,5,80 */

    switch (chance) {
    case 0: verbalize("I am in your debt.  I will grant one wish!");
        makewish(FALSE);
        mongone(mtmp);
        break;
    case 1: verbalize("Thank you for freeing me!");
        (void) tamedog(mtmp, (struct obj *)0);
        break;
    case 2: verbalize("You freed me!");
        mtmp->mpeaceful = TRUE;
        set_malign(mtmp);
        break;
    case 3: verbalize("It is about time!");
        pline("%s vanishes.", Monnam(mtmp));
        mongone(mtmp);
        break;
    default: verbalize("You disturbed me, fool!");
        break;
    }
}

/* clone a gremlin or mold (2nd arg non-null implies heat as the trigger);
   hit points are cut in half (odd HP stays with original) */
struct monst *
split_mon(mon, mtmp)
struct monst *mon,  /* monster being split */
             *mtmp; /* optional attacker whose heat triggered it */
{
    struct monst *mtmp2;
    char reason[BUFSZ];

    reason[0] = '\0';
    if (mtmp) Sprintf(reason, " from %s heat",
                      (mtmp == &youmonst) ? (const char *)"your" :
                      (const char *)s_suffix(mon_nam(mtmp)));

    if (mon == &youmonst) {
        mtmp2 = cloneu();
        if (mtmp2) {
            mtmp2->mhpmax = u.mhmax / 2;
            u.mhmax -= mtmp2->mhpmax;
            flags.botl = 1;
            You("multiply%s!", reason);
        }
    } else {
        mtmp2 = clone_mon(mon, 0, 0);
        if (mtmp2) {
            mtmp2->mhpmax = mon->mhpmax / 2;
            mon->mhpmax -= mtmp2->mhpmax;
            if (canspotmon(mon))
                pline("%s multiplies%s!", Monnam(mon), reason);
        }
    }
    return mtmp2;
}

/*potion.c*/
