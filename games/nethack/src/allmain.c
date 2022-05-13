/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* various code that was replicated in *main.c */

#include "hack.h"

#include <limits.h>

#ifndef NO_SIGNAL
#include <signal.h>
#endif

void
early_init()
{
#if 0
    decl_globals_init();
    objects_globals_init();
    monst_globals_init();
#endif
    sys_early_init();
}

#ifdef POSITIONBAR
STATIC_DCL void NDECL(do_positionbar);
#endif
STATIC_DCL void FDECL(interrupt_multi, (const char *, int, int));

static int prev_hp_notify;
enum monster_generation monclock;

char *
hpnotify_format_str(char *str)
{
    static char buf[128];
    char *f, *p, *end;
    int ispercent = 0;

    buf[0] = '\0';

    if (!str) return NULL;

    f = str;
    p = buf;
    end = buf + sizeof(buf) - 10;

    while (*f) {
        if (ispercent) {
            switch (*f) {
            case 'a':
                snprintf (p, end + 1 - p, "%ld", (long)abs(uhp()-prev_hp_notify));
                while (*p != '\0')
                    p++;
                break;
            case 'c':
                snprintf (p, end + 1 - p, "%c", (prev_hp_notify > uhp() ? '-' : '+'));
                p++;
                break;
            case 'm':
                snprintf (p, end + 1 - p, "%ld", (long)uhpmax());
                while (*p != '\0')
                    p++;
                break;
            case 'H':
                if (uhp() == uhpmax()) {
                    snprintf (p, end + 1 - p, "%s", "max");
                } else {
                    snprintf (p, end + 1 - p, "%ld", (long)uhp());
                }
                while (*p != '\0')
                    p++;
                break;
            case 'h':
                snprintf (p, end + 1 - p, "%ld", (long)uhp());
                while (*p != '\0')
                    p++;
                break;
            default:
                *p = *f;
                if (p < end)
                    p++;
            }
            ispercent = 0;
        } else {
            if (*f == '%')
                ispercent = 1;
            else {
                *p = *f;
                if (p < end)
                    p++;
            }
        }
        f++;
    }
    *p = '\0';

    return buf;
}

/* Elven players cannot regenerate if in direct contact with cold
 * iron, and vampiric players cannot if in direct contact with silver.
 * Make an exception for your quest artifact, though -- currently,
 * the only worn iron quest artifacts are weapons, a helm, and an
 * amulet, and the only worn silver quest artifact is a shield.
 * It is also not possible to hold artifacts as secondary weapons.
 */
boolean
can_regenerate()
{
    if (marathon_mode) {
        return 0;
    }
    if (is_elf(youmonst.data)) {
        if (uwep && is_iron(uwep) &&
            !is_quest_artifact(uwep) && !uarmg) return 0;
#ifdef TOURIST
        if (uarm && is_iron(uarm) && !uarmu) return 0;
        if (uarmu && is_iron(uarmu)) return 0;
        if (uarmc && is_iron(uarmc) && !uarmu && !uarm) return 0;
#else
        if (uarm && is_iron(uarm)) return 0;
        if (uarmc && is_iron(uarmc) && !uarm) return 0;
#endif
        if (uarmh && is_iron(uarmh) &&
            !is_quest_artifact(uarmh)) return 0;
        if (uarms && is_iron(uarms) && !uarmg) return 0;
        if (uarmg && is_iron(uarmg)) return 0;
        if (uarmf && is_iron(uarmf)) return 0;
        if (uleft && is_iron(uleft)) return 0;
        if (uright && is_iron(uright)) return 0;
#ifdef TOURIST
        if (uamul && is_iron(uamul) &&
            !is_quest_artifact(uamul) && !uarmu && !uarm) return 0;
#else
        if (uamul && is_iron(uamul) &&
            !is_quest_artifact(uamul) && !uarm) return 0;
#endif
        if (ublindf && is_iron(ublindf)) return 0;
        if (uchain && is_iron(uchain)) return 0;
        if (uswapwep && is_iron(uswapwep) && u.twoweap) return 0;
    } else if (is_vampiric(youmonst.data)) {
        if (uwep && is_silver(uwep) && !uarmg) return 0;
        if (uarms && is_silver(uarms) &&
            !is_quest_artifact(uarms) && !uarmg) return 0;
        if (uleft && is_silver(uleft)) return 0;
        if (uright && is_silver(uright)) return 0;
    }
    return 1;
}

void
moveloop(resuming)
boolean resuming;
{
#if defined(MICRO) || defined(WIN32)
    char ch;
    int abort_lev;
#endif
    int moveamt = 0, wtcap = 0, change = 0;
    boolean didmove = FALSE, monscanmove = FALSE;
    /* don't make it obvious when monsters will start speeding up */
    int timeout_start = rnd(10000)+25000;
    int past_clock;
    /* for keeping track of Elbereth and correctstatus line display */
#ifdef ELBERETH
    int was_on_elbereth = 0;
    int is_on_elbereth = 0;
#endif
    boolean can_regen = can_regenerate();

    flags.moonphase = phase_of_the_moon();
    if(flags.moonphase == FULL_MOON) {
        You("are lucky!  Full moon tonight.");
        change_luck(1);
    } else if(flags.moonphase == NEW_MOON) {
        pline("Be careful!  New moon tonight.");
    }
    flags.friday13 = friday_13th();
    if (flags.friday13) {
        pline("Watch out!  Bad things can happen on Friday the 13th.");
        change_luck(-1);
    }

    if (!resuming) {
        /* new game */
        flags.rndencode = rnd(9000);
        set_wear(); /* for side-effects of starting gear */
        (void) pickup(1);      /* autopickup at initial location */
    }

    initrack();


    /* Note:  these initializers don't do anything except guarantee that
       we're linked properly.
     */
    decl_init();
    monst_init();
    monstr_init();  /* monster strengths */
    objects_init();
    dragons_init();
    doredraw(); /* partial workaround to http://sourceforge.net/apps/trac/nethack/ticket/2 */
    shop_selection_init();

    (void) encumber_msg(); /* in case they auto-picked up something */
    if (defer_see_monsters) {
        defer_see_monsters = FALSE;
        see_monsters();
    }

    u.uz0.dlevel = u.uz.dlevel;
    youmonst.movement = NORMAL_SPEED;   /* give the hero some movement points */
    prev_hp_notify = uhp();
#ifdef WHEREIS_FILE
    touch_whereis();
#endif

    for(;;) {
        get_nh_event();
#ifdef POSITIONBAR
        do_positionbar();
#endif

        didmove = flags.move;
        if(didmove) {
            /* actual time passed */
            youmonst.movement -= NORMAL_SPEED;

            do { /* hero can't move this turn loop */
                wtcap = encumber_msg();

                flags.mon_moving = TRUE;
                do {
                    monscanmove = movemon();
                    if (youmonst.movement > NORMAL_SPEED)
                        break;  /* it's now your turn */
                } while (monscanmove);
                flags.mon_moving = FALSE;

                /* heaven or hell mode: player always has 1 maxhp */
                check_uhpmax();

                if (!monscanmove && youmonst.movement < NORMAL_SPEED) {
                    /* both you and the monsters are out of steam this round */
                    /* set up for a new turn */
                    struct monst *mtmp;
                    mcalcdistress();    /* adjust monsters' trap, blind, etc */

                    /* reallocate movement rations to monsters */
                    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
                        mtmp->movement += mcalcmove(mtmp);

                    /* Vanilla generates a critter every 70-ish turns.
                     * The rate accelerates to every 50 or so below the Castle,
                     * and 'round every 25 turns once you've done the Invocation.
                     *
                     * We will push it even further.  Monsters post-Invocation
                     * will almost always appear on the stairs (if present), and
                     * much more frequently; this, along with the extra intervene()
                     * calls, should certainly make it seem like you're wading back
                     * through the teeming hordes.
                     *
                     * Aside from that, a more general clock should be put on things;
                     * after about 30,000 turns, the frequency rate of appearance
                     * and (TODO) difficulty of monsters generated will slowly increase until
                     * it reaches the point it will be at as if you were post-Invocation.
                     *
                     * The rate increases linearly with turns.  The rule of thumb is that
                     * at turn x the rate is approximately (x / 30,000) times the normal
                     * rate.  Maximal rate is 7x the normal rate.
                     */
                    monclock = MIN_MONGEN_RATE;
                    if (u.uevent.udemigod) {
                        monclock = MAX_MONGEN_RATE;
                    } else {
                        past_clock = moves - timeout_start;
                        if (past_clock > 0) {
                            monclock = MIN_MONGEN_RATE * 30000 / (past_clock + 30000);
                        }
                        if (monclock > MIN_MONGEN_RATE/2 && depth(&u.uz) > depth(&stronghold_level)) {
                            monclock = MIN_MONGEN_RATE/2;
                        }
                    }
                    /* make sure we don't fall off the edges */
                    if (monclock < MAX_MONGEN_RATE) { monclock = MAX_MONGEN_RATE; }
                    if (monclock > MIN_MONGEN_RATE) { monclock = MIN_MONGEN_RATE; }

                    /* TODO: adj difficulty in makemon */
                    if (!rn2(monclock)) {
                        if (u.uevent.udemigod && xupstair && rn2(10)) {
                            (void) makemon((struct permonst *)0, xupstair, yupstair, MM_ADJACENTOK);
                        } else if (u.uevent.udemigod && sstairs.sx && rn2(10)) {
                            (void) makemon((struct permonst *)0, sstairs.sx, sstairs.sy, MM_ADJACENTOK);
                        } else {
                            (void) makemon((struct permonst *)0, 0, 0, NO_MM_FLAGS);
                        }
                    }

                    /* calculate how much time passed. */
#ifdef STEED
                    if (u.usteed && u.umoved) {
                        /* your speed doesn't augment steed's speed */
                        moveamt = mcalcmove(u.usteed);
                    } else
#endif
                    {
                        moveamt = youmonst.data->mmove;

                        if (Very_fast) {    /* speed boots or potion */
                            /* average movement is 1.67 times normal */
                            moveamt += NORMAL_SPEED / 2;
                            if (rn2(3) == 0) moveamt += NORMAL_SPEED / 2;
                        } else if (Fast) {
                            /* average movement is 1.33 times normal */
                            if (rn2(3) != 0) moveamt += NORMAL_SPEED / 2;
                        }
                    }

                    switch (wtcap) {
                    case UNENCUMBERED: break;
                    case SLT_ENCUMBER: moveamt -= (moveamt / 4); break;
                    case MOD_ENCUMBER: moveamt -= (moveamt / 2); break;
                    case HVY_ENCUMBER: moveamt -= ((moveamt * 3) / 4); break;
                    case EXT_ENCUMBER: moveamt -= ((moveamt * 7) / 8); break;
                    default: break;
                    }

                    youmonst.movement += moveamt;
                    if (youmonst.movement < 0) youmonst.movement = 0;
                    settrack();

                    monstermoves++;
                    moves++;

                    /********************************/
                    /* once-per-turn things go here */
                    /********************************/

                    if (flags.bypasses) clear_bypasses();
                    if(Glib) glibr();
                    nh_timeout();
                    run_regions();
#ifdef DUNGEON_GROWTH
                    dgn_growths(TRUE, TRUE);
#endif
                    if (u.ublesscnt) u.ublesscnt--;
                    if(flags.time && !flags.run)
                        flags.botl = 1;

                    /* One possible result of prayer is healing.  Whether or
                     * not you get healed depends on your current hit points.
                     * If you are allowed to regenerate during the prayer, the
                     * end-of-prayer calculation messes up on this.
                     * Another possible result is rehumanization, which requires
                     * that encumbrance and movement rate be recalculated.
                     */
                    if (u.uinvulnerable) {
                        /* for the moment at least, you're in tiptop shape */
                        wtcap = UNENCUMBERED;
                    } else if (Upolyd && youmonst.data->mlet == S_EEL && !is_pool(u.ux, u.uy) && !Is_waterlevel(&u.uz)) {
                        if (u.mh > 1) {
                            u.mh--;
                            flags.botl = 1;
                        } else if (u.mh < 1)
                            rehumanize();
                    } else if (Upolyd && u.mh < u.mhmax) {
                        if (u.mh < 1)
                            rehumanize();
                        else if (can_regenerate() && (Regeneration ||
                                                      (wtcap < MOD_ENCUMBER && !(moves%20)))) {
                            flags.botl = 1;
                            u.mh++;
                            interrupt_multi("Hit points", u.mh, u.mhmax);
                        }
                    } else if (u.uhp < u.uhpmax && can_regenerate() &&
                               (wtcap < MOD_ENCUMBER || !u.umoved || Regeneration)) {
                        if (u.ulevel > 9 && !(moves % 3)) {
                            if (!marathon_mode) {
                                int heal, Con = (int) ACURR(A_CON);

                                if (Con <= 12) {
                                    heal = 1;
                                } else {
                                    heal = rnd(Con);
                                    if (heal > u.ulevel-9) heal = u.ulevel-9;
                                }
                                flags.botl = 1;
                                u.uhp += heal;
                                if(u.uhp > u.uhpmax)
                                    u.uhp = u.uhpmax;
                                interrupt_multi("Hit points", u.uhp, u.uhpmax);
                            }
                        } else if (Regeneration ||
                                   (u.ulevel <= 9 &&
                                    !(moves % ((MAXULEV+12) / (u.ulevel+2) + 1)))) {
                            if (!marathon_mode) {
                                flags.botl = 1;
                                u.uhp++;
                                interrupt_multi("Hit points", u.uhp, u.uhpmax);
                            }
                        }
                    }

                    /* moving around while encumbered is hard work */
                    if (wtcap > MOD_ENCUMBER && u.umoved) {
                        if(!(wtcap < EXT_ENCUMBER ? moves%30 : moves%10)) {
                            if (Upolyd && u.mh > 1) {
                                u.mh--;
                            } else if (!Upolyd && u.uhp > 1) {
                                u.uhp--;
                            } else {
                                You("pass out from exertion!");
                                exercise(A_CON, FALSE);
                                fall_asleep(-10, FALSE);
                            }
                        }
                    }

                    if ((u.uen < u.uenmax) &&
                        ((wtcap < MOD_ENCUMBER &&
                          (!(moves%((MAXULEV + 8 - u.ulevel) *
                                    (Role_if(PM_WIZARD) ? 3 : 4) / 6))))
                         || Energy_regeneration)) {
                        u.uen += rn1((int)(ACURR(A_WIS) + ACURR(A_INT)) / 15 + 1, 1);
                        if (u.uen > u.uenmax) u.uen = u.uenmax;
                        flags.botl = 1;
                        interrupt_multi("Magic energy", u.uen, u.uenmax);
                    }

                    if(!u.uinvulnerable) {
                        if(Teleportation && !rn2(85)) {
                            xchar old_ux = u.ux, old_uy = u.uy;
                            tele();
                            if (u.ux != old_ux || u.uy != old_uy) {
                                if (!next_to_u()) {
                                    check_leash(old_ux, old_uy);
                                }
#ifdef REDO
                                /* clear doagain keystrokes */
                                pushch(0);
                                savech(0);
#endif
                            }
                        }
                        /* delayed change may not be valid anymore */
                        if ((change == 1 && !Polymorph) ||
                            (change == 2 && u.ulycn == NON_PM))
                            change = 0;
                        if(Polymorph && !rn2(100))
                            change = 1;
                        else if (u.ulycn >= LOW_PM && !Upolyd &&
                                 !rn2(80 - (20 * night())))
                            change = 2;
                        if (change && !Unchanging) {
                            if (multi >= 0) {
                                if (occupation)
                                    stop_occupation();
                                else
                                    nomul(0, 0);
                                if (change == 1) polyself(FALSE);
                                else you_were();
                                change = 0;
                            }
                        }
                    }

                    if(Searching && multi >= 0) (void) dosearch0(1);
                    dosounds();
                    do_storms();
                    gethungry();
                    age_spells();
                    exerchk();
                    invault();
                    if (u.uhave.amulet) amulet();
                    if (!rn2(40+(int)(ACURR(A_DEX)*3)))
                        u_wipe_engr(rnd(3));
                    if (u.uevent.udemigod && !u.uinvulnerable) {
                        if (u.udg_cnt) u.udg_cnt--;
                        if (!u.udg_cnt) {
                            intervene();
                            u.udg_cnt = rn1(200, (42+Luck));
                        }
                    }
                    restore_attrib();
                    /* underwater and waterlevel vision are done here */
                    if (Is_waterlevel(&u.uz))
                        movebubbles();
                    else if (Underwater)
                        under_water(0);
                    /* vision while buried done here */
                    else if (u.uburied) under_ground(0);

                    /* when immobile, count is in turns */
                    if(multi < 0) {
                        if (++multi == 0) { /* finished yet? */
                            unmul((char *)0);
                            /* if unmul caused a level change, take it now */
                            if (u.utotype) deferred_goto();
                        }
                    }
                }
            } while (youmonst.movement<NORMAL_SPEED); /* hero can't move loop */

            /******************************************/
            /* once-per-hero-took-time things go here */
            /******************************************/

            if (u.utrap && u.utraptype == TT_LAVA) {
                if (!is_lava(u.ux, u.uy))
                    u.utrap = 0;
                else if (!u.uinvulnerable) {
                    u.utrap -= 1<<8;
                    if (u.utrap < 1<<8) {
                        killer.format = KILLED_BY;
                        Strcpy(killer.name, "molten lava");
                        You("sink below the surface and die.");
                        done(DISSOLVED);
                    } else if (!u.umoved) {
                        Norep("You sink deeper into the lava.");
                        u.utrap += rnd(4);
                    }
                }
            }

        } /* actual time passed */

        /****************************************/
        /* once-per-player-input things go here */
        /****************************************/
        game_loop_counter++;

        find_ac();
        if(!flags.mv || Blind) {
            /* redo monsters if hallu or wearing a helm of telepathy */
            if (Hallucination) {    /* update screen randomly */
                see_monsters();
                see_objects();
                see_traps();
                if (u.uswallow) swallowed(0);
            } else if (Unblind_telepat) {
                see_monsters();
            } else if (Warning || Warn_of_mon)
                see_monsters();

            if (vision_full_recalc) vision_recalc(0);   /* vision! */
        }

#ifdef ELBERETH
        /* check changes of Elbereth at current player location */
        is_on_elbereth = sengr_at("Elbereth", u.ux, u.uy);
        if (was_on_elbereth != is_on_elbereth) {
            was_on_elbereth = is_on_elbereth;
            flags.botlx = 1;
        }
#endif

#ifdef REALTIME_ON_BOTL
        if (iflags.showrealtime) {
            time_t currenttime = get_realtime();
            if (currenttime >= 3600 &&
                 (iflags.realtime_format != REALTIME_FORMAT_SECONDS)) {
                if (currenttime / 60 != urealtime.last_displayed_time / 60) {
                    flags.botl = 1;
                    urealtime.last_displayed_time = currenttime;
                }
            } else {
                if (currenttime != urealtime.last_displayed_time) {
                    flags.botl = 1;
                    urealtime.last_displayed_time = currenttime;
                }
            }
        }
#endif

        if(flags.botl || flags.botlx) bot();

        if (iflags.hp_notify && (prev_hp_notify != uhp())) {
            pline("%s", hpnotify_format_str(iflags.hp_notify_fmt ? iflags.hp_notify_fmt : "[HP%c%a=%h]"));
            prev_hp_notify = uhp();
        }

        if (can_regen != can_regenerate()) {
            if (!Hallucination) {
                You_feel("%s.", (can_regen) ? "itchy" : "relief");
            } else {
                You_feel("%s.", (can_regen) ? (is_elf(youmonst.data) ? "magnetic" :
                                               "tarnished") : "like you are no longer failing Organic Chemistry");
            }
            can_regen = can_regenerate();
        }

        flags.move = 1;

        if(multi >= 0 && occupation) {
#if defined(MICRO) || defined(WIN32)
            abort_lev = 0;
            if (kbhit()) {
                if ((ch = Getchar()) == ABORT)
                    abort_lev++;
# ifdef REDO
                else
                    pushch(ch);
# endif /* REDO */
            }
            if (!abort_lev && (*occupation)() == 0)
#else
            if ((*occupation)() == 0)
#endif
                occupation = 0;
            if(
#if defined(MICRO) || defined(WIN32)
                abort_lev ||
#endif
                monster_nearby()) {
                stop_occupation();
                reset_eat();
            }
#if defined(MICRO) || defined(WIN32)
            if (!(++occtime % 7))
                display_nhwindow(WIN_MAP, FALSE);
#endif
            continue;
        }

        if ((u.uhave.amulet || Clairvoyant) &&
            !In_endgame(&u.uz) && !BClairvoyant &&
            !(moves % 15) && !rn2(2))
            do_vicinity_map();

#ifdef WIZARD
        if (iflags.sanity_check || iflags.debug_fuzzer)
            sanity_check();
#endif

#ifdef CLIPPING
        /* just before rhack */
        cliparound(u.ux, u.uy);
#endif

        u.umoved = FALSE;

        if (multi > 0) {
            lookaround();
            if (!multi) {
                /* lookaround may clear multi */
                flags.move = 0;
                if (flags.time) flags.botl = 1;
                continue;
            }
            if (flags.mv) {
                if(multi < COLNO && !--multi)
                    flags.travel = iflags.travel1 = flags.mv = flags.run = 0;
                domove();
            } else {
                --multi;
                rhack(save_cm);
            }
        } else if (multi == 0) {
#ifdef MAIL
            ckmailstatus();
            maybe_hint();
#endif
            maybe_tutorial();
            rhack((char *)0);
        }
        if (u.utotype)      /* change dungeon level */
            deferred_goto();    /* after rhack() */
        /* !flags.move here: multiple movement command stopped */
        else if (flags.time && (!flags.move || !flags.mv))
            flags.botl = 1;

        if (vision_full_recalc) vision_recalc(0);   /* vision! */
        /* when running in non-tport mode, this gets done through domove() */
        if ((!flags.run || iflags.runmode == RUN_TPORT) &&
            (multi && (!flags.travel ? !(multi % 7) : !(moves % 7L)))) {
            if (flags.time && flags.run) flags.botl = 1;
            display_nhwindow(WIN_MAP, FALSE);
        }
    }
}

void
stop_occupation()
{
    if(occupation) {
        if (!maybe_finished_meal(TRUE))
            You("stop %s.", occtxt);
        occupation = 0;
        flags.botl = 1; /* in case u.uhs changed */
/* fainting stops your occupation, there's no reason to sync.
        sync_hunger();
 */
#ifdef REDO
        nomul(0, 0);
        pushch(0);
#endif
    }
}

void
display_gamewindows()
{
    curses_stupid_hack = 0;
    WIN_MESSAGE = create_nhwindow(NHW_MESSAGE);
    WIN_STATUS = create_nhwindow(NHW_STATUS);
    WIN_MAP = create_nhwindow(NHW_MAP);
    WIN_INVEN = create_nhwindow(NHW_MENU);

#ifdef MAC
    /*
     * This _is_ the right place for this - maybe we will
     * have to split display_gamewindows into create_gamewindows
     * and show_gamewindows to get rid of this ifdef...
     */
    if ( !strcmp ( windowprocs.name, "mac" ) ) {
        SanePositions ( );
    }
#endif

    /*
     * The mac port is not DEPENDENT on the order of these
     * displays, but it looks a lot better this way...
     */
    display_nhwindow(WIN_STATUS, FALSE);
    display_nhwindow(WIN_MESSAGE, FALSE);
    clear_glyph_buffer();
    display_nhwindow(WIN_MAP, FALSE);
}

static
void
init_level_seeds()
{
    int i;
    if (is_game_pre_seeded) {
        set_random_state(level_info[0].seed + sysopt.serverseed);
    } else {
        set_random_state(level_info[0].seed);
    }
    for (i=1; i<MAXLINFO; i++) {
        level_info[i].seed = rn2(INT_MAX);
    }
}


void
newgame()
{
    int i;

#ifdef MFLOPPY
    gameDiskPrompt();
#endif

    flags.ident = 1;

    for (i = 0; i < NUMMONS; i++)
        mvitals[i].mvflags = mons[i].geno & G_NOCORPSE;

    init_level_seeds();

    init_objects();     /* must be before u_init() */

    flags.pantheon = -1;    /* role_init() will reset this */
    role_init();        /* must be before init_dungeons(), u_init(),
                         * and init_artifacts() */

    init_dungeons();    /* must be before u_init() to avoid rndmonst()
                         * creating odd monsters for any tins and eggs
                         * in hero's initial inventory */
    init_artifacts();   /* before u_init() in case $WIZKIT specifies
                         * any artifacts */
    u_init();

#ifndef NO_SIGNAL
    (void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
#ifdef NEWS
    if(iflags.news) display_file_area(NEWS_AREA, NEWS, FALSE);
#endif
    load_qtlist();  /* load up the quest text info */
/*  quest_init();*/ /* Now part of role_init() */

    mklev();
    u_on_upstairs();
    vision_reset();     /* set up internals for level (after mklev) */
    check_special_room(FALSE);

    flags.botlx = 1;

    /* Move the monster from under you or else
     * makedog() will fail when it calls makemon().
     *          - ucsfcgl!kneller
     */
    if(MON_AT(u.ux, u.uy)) mnexto(m_at(u.ux, u.uy));
    (void) makedog();
    docrt();
#ifdef CONVICT
    if (Role_if(PM_CONVICT)) {
        setworn(mkobj(CHAIN_CLASS, TRUE), W_CHAIN);
        setworn(mkobj(BALL_CLASS, TRUE), W_BALL);
        uball->spe = 1;
        placebc();
        newsym(u.ux, u.uy);
    }
#endif /* CONVICT */

    if (flags.legacy) {
        flush_screen(1);
#ifdef CONVICT
        if (Role_if(PM_CONVICT)) {
            com_pager(199);
        } else {
            com_pager(1);
        }
#else
        com_pager(1);
#endif /* CONVICT */
    }

#ifdef INSURANCE
    save_currentstate();
#endif
    program_state.something_worth_saving++; /* useful data now exists */

#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)

    /* Start the timer here */
    urealtime.realtime = (time_t)0L;
    urealtime.start_timing = current_epoch();
#endif /* RECORD_REALTIME || REALTIME_ON_BOTL */

    /* Success! */
    welcome(TRUE);
    return;
}

/* show "welcome [back] to nethack" message at program startup */
void
welcome(new_game)
boolean new_game;   /* false => restoring an old game */
{
    char buf[BUFSZ];
    boolean currentgend = Upolyd ? u.mfemale : flags.female;
    const char *role_name;
    char *annotation;

    /*
     * The "welcome back" message always describes your innate form
     * even when polymorphed or wearing a helm of opposite alignment.
     * Alignment is shown unconditionally for new games; for restores
     * it's only shown if it has changed from its original value.
     * Sex is shown for new games except when it is redundant; for
     * restores it's only shown if different from its original value.
     */
    *buf = '\0';
    if (new_game || u.ualignbase[A_ORIGINAL] != u.ualignbase[A_CURRENT])
        Sprintf(eos(buf), " %s", align_str(u.ualignbase[A_ORIGINAL]));
    if (!urole.name.f &&
        (new_game ? (urole.allow & ROLE_GENDMASK) == (ROLE_MALE|ROLE_FEMALE) :
         currentgend != flags.initgend))
        Sprintf(eos(buf), " %s", genders[currentgend].adj);

    role_name = (currentgend && urole.name.f) ? urole.name.f : urole.name.m;
    pline(new_game ? "%s %s, welcome to NetHack!  You are a%s %s %s."
          : "%s %s, the%s %s %s, welcome back to NetHack!",
          Hello((struct monst *) 0), plname, buf, urace.adj, role_name);
#ifdef LIVELOGFILE
    /* Start live reporting */
    livelog_start();
    livelog_game_started(new_game ? "started" : "resumed",
                         buf, urace.adj, role_name);
#endif
    /* show level annotation when restoring the level */
    if (!new_game && iflags.show_annotation &&
        (annotation = get_annotation(&u.uz))) {
        You("annotated this level: %s", annotation);
    }
}

#ifdef POSITIONBAR
STATIC_DCL void
do_positionbar()
{
    static char pbar[COLNO];
    char *p;

    p = pbar;
    /* up stairway */
    if (upstair.sx &&
        (glyph_to_cmap(level.locations[upstair.sx][upstair.sy].glyph) ==
         S_upstair ||
         glyph_to_cmap(level.locations[upstair.sx][upstair.sy].glyph) ==
         S_upladder)) {
        *p++ = '<';
        *p++ = upstair.sx;
    }
    if (sstairs.sx &&
        (glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
         S_upstair ||
         glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
         S_upladder)) {
        *p++ = '<';
        *p++ = sstairs.sx;
    }

    /* down stairway */
    if (dnstair.sx &&
        (glyph_to_cmap(level.locations[dnstair.sx][dnstair.sy].glyph) ==
         S_dnstair ||
         glyph_to_cmap(level.locations[dnstair.sx][dnstair.sy].glyph) ==
         S_dnladder)) {
        *p++ = '>';
        *p++ = dnstair.sx;
    }
    if (sstairs.sx &&
        (glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
         S_dnstair ||
         glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
         S_dnladder)) {
        *p++ = '>';
        *p++ = sstairs.sx;
    }

    /* hero location */
    if (u.ux) {
        *p++ = '@';
        *p++ = u.ux;
    }
    /* fence post */
    *p = 0;

    update_positionbar(pbar);
}
#endif

#if defined(REALTIME_ON_BOTL) || defined (RECORD_REALTIME)
time_t
get_realtime(void)
{
    /* Get current time */
    time_t current_time = current_epoch();

    /* Since the timer isn't set until the game starts, this prevents us
     * from displaying nonsense on the bottom line before it does. */
    if (urealtime.start_timing == 0) {
        current_time = urealtime.realtime;
    } else {
        current_time -= urealtime.start_timing;
        current_time += urealtime.realtime;
    }

    return current_time;
}
#endif /* REALTIME_ON_BOTL || RECORD_REALTIME */

/** Interrupt a multiturn action if current_points is equal to max_points. */
STATIC_DCL
void
interrupt_multi(points, current_points, max_points)
const char *points;
int current_points;
int max_points;
{
    if (multi > 0 &&
        current_points == max_points) {
        nomul(0, 0);
        if (flags.verbose) pline("%s restored.", points);
    }
}

/*allmain.c*/
