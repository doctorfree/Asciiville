/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#ifdef REINCARNATION
#include <ctype.h>
#endif

/* this assumes that a human quest leader or nemesis is an archetype
   of the corresponding role; that isn't so for some roles (tourist
   for instance) but is for the priests and monks we use it for... */
#define quest_mon_represents_role(mptr, role_pm) \
    (mptr->mlet == S_HUMAN && Role_if(role_pm) && \
     (mptr->msound == MS_LEADER || mptr->msound == MS_NEMESIS))

STATIC_DCL boolean FDECL(uncommon, (int));
STATIC_DCL int FDECL(align_shift, (struct permonst *));
STATIC_DCL boolean FDECL(mk_gen_ok, (int, int, int));
STATIC_DCL boolean FDECL(wrong_elem_type, (struct permonst *));
static void FDECL(m_initgrp, (struct monst *, int, int, int, int));
STATIC_DCL void FDECL(m_initthrow, (struct monst *, int, int));
STATIC_DCL void FDECL(m_initweap, (struct monst *));
STATIC_DCL void FDECL(m_inityour, (struct monst *, struct obj *));
STATIC_DCL void FDECL(m_initinv, (struct monst *));

extern const int monstr[];

#define m_initsgrp(mtmp, x, y, mmf) m_initgrp(mtmp, x, y, 3, mmf)
#define m_initlgrp(mtmp, x, y, mmf) m_initgrp(mtmp, x, y, 10, mmf)
#define toostrong(monindx, lev) (monstr[monindx] > lev)
#define tooweak(monindx, lev)   (monstr[monindx] < lev)

boolean
is_home_elemental(ptr)
register struct permonst *ptr;
{
    if (ptr->mlet == S_ELEMENTAL)
        switch (monsndx(ptr)) {
        case PM_AIR_ELEMENTAL: return Is_airlevel(&u.uz);
        case PM_FIRE_ELEMENTAL: return Is_firelevel(&u.uz);
        case PM_EARTH_ELEMENTAL: return Is_earthlevel(&u.uz);
        case PM_WATER_ELEMENTAL: return Is_waterlevel(&u.uz);
        }
    return FALSE;
}

/*
 * Return true if the given monster cannot exist on this elemental level.
 */
STATIC_OVL boolean
wrong_elem_type(ptr)
struct permonst *ptr;
{
    if (ptr->mlet == S_ELEMENTAL) {
        return((boolean)(!is_home_elemental(ptr)));
    } else if (Is_earthlevel(&u.uz)) {
        /* no restrictions? */
    } else if (Is_waterlevel(&u.uz)) {
        /* just monsters that can swim */
        if (!is_swimmer(ptr)) {
            return TRUE;
        }
    } else if (Is_firelevel(&u.uz)) {
        if (!pm_resistance(ptr, MR_FIRE)) {
            return TRUE;
        }
    } else if (Is_airlevel(&u.uz)) {
        if(!(is_flyer(ptr) && ptr->mlet != S_TRAPPER) && !is_floater(ptr)
           && !amorphous(ptr) && !noncorporeal(ptr) && !is_whirly(ptr))
            return TRUE;
    }
    return FALSE;
}
/* make a group just like mtmp */
STATIC_OVL void
m_initgrp(mtmp, x, y, n, mmflags)
struct monst *mtmp;
int x, y, n, mmflags;
{
    coord mm;
    register int cnt = rnd(n);
    struct monst *mon;
#if defined(__GNUC__) && (defined(HPUX) || defined(DGUX))
    /* There is an unresolved problem with several people finding that
     * the game hangs eating CPU; if interrupted and restored, the level
     * will be filled with monsters.  Of those reports giving system type,
     * there were two DG/UX and two HP-UX, all using gcc as the compiler.
     * hcroft@hpopb1.cern.ch, using gcc 2.6.3 on HP-UX, says that the
     * problem went away for him and another reporter-to-newsgroup
     * after adding this debugging code.  This has almost got to be a
     * compiler bug, but until somebody tracks it down and gets it fixed,
     * might as well go with the "but it went away when I tried to find
     * it" code.
     */
    int cnttmp, cntdiv;

    cnttmp = cnt;
# ifdef DEBUG
    pline("init group call x=%d,y=%d,n=%d,cnt=%d.", x, y, n, cnt);
# endif
    cntdiv = ((u.ulevel < 3) ? 4 : (u.ulevel < 5) ? 2 : 1);
#endif
    /* Tuning: cut down on swarming at low character levels [mrs] */
    cnt /= (u.ulevel < 3) ? 4 : (u.ulevel < 5) ? 2 : 1;
#if defined(__GNUC__) && (defined(HPUX) || defined(DGUX))
    if (cnt != (cnttmp/cntdiv)) {
        pline("cnt=%d using %d, cnttmp=%d, cntdiv=%d", cnt,
              (u.ulevel < 3) ? 4 : (u.ulevel < 5) ? 2 : 1,
              cnttmp, cntdiv);
    }
#endif
    if(!cnt) cnt++;
#if defined(__GNUC__) && (defined(HPUX) || defined(DGUX))
    if (cnt < 0) cnt = 1;
    if (cnt > 10) cnt = 10;
#endif

    mm.x = x;
    mm.y = y;
    while (cnt--) {
        if (peace_minded(mtmp->data)) continue;
        /* Don't create groups of peaceful monsters since they'll get
         * in our way.  If the monster has a percentage chance so some
         * are peaceful and some are not, the result will just be a
         * smaller group.
         */
        if (enexto(&mm, mm.x, mm.y, mtmp->data)) {
            mon = makemon(mtmp->data, mm.x, mm.y, (mmflags | MM_NOGRP));
            if (mon) {
                mon->mpeaceful = FALSE;
                mon->mavenge = 0;
                set_malign(mon);
            }
            /* Undo the second peace_minded() check in makemon(); if the
             * monster turned out to be peaceful the first time we
             * didn't create it at all; we don't want a second check.
             */
        }
    }
}

STATIC_OVL
void
m_initthrow(mtmp, otyp, oquan)
struct monst *mtmp;
int otyp, oquan;
{
    register struct obj *otmp;

    otmp = mksobj(otyp, TRUE, FALSE);
    otmp->quan = (long) rn1(oquan, 3);
    otmp->owt = weight(otmp);
    if (otyp == ORCISH_ARROW) otmp->opoisoned = TRUE;
    (void) mpickobj(mtmp, otmp);
}

STATIC_OVL void
m_inityour(mtmp, obj)
struct monst *mtmp;
struct obj *obj;
{
    register struct obj *otmp;

    otmp = mksobj(obj->otyp, FALSE, FALSE);
    if (obj->blessed) bless(obj);
    else if (obj->cursed) curse(obj);
    otmp->quan = obj->quan;
    otmp->owt = weight(obj);
    otmp->opoisoned = obj->opoisoned;
    otmp->oerodeproof = TRUE;
    /* otmp->spe = obj->spe; */
    (void) mpickobj(mtmp, otmp);
}

STATIC_OVL void
m_initweap(mtmp)
register struct monst *mtmp;
{
    register struct permonst *ptr = mtmp->data;
    register int mm = monsndx(ptr);
    struct obj *otmp;
    int bias, w1, w2;

#ifdef REINCARNATION
    if (Is_rogue_level(&u.uz)) return;
#endif
    /*
     *  first a few special cases:
     *      giants get a boulder to throw sometimes.
     *      ettins get clubs
     *      kobolds get darts to throw
     *      centaurs get some sort of bow & arrows or bolts
     *      soldiers get all sorts of things.
     *      kops get clubs & cream pies.
     */
    switch (ptr->mlet) {
        case S_GIANT:
            if (rn2(2)) (void)mongets(mtmp, (mm != PM_ETTIN) ?
                    BOULDER : CLUB);
            break;

        case S_HUMAN:
            if (mm == PM_EXECUTIONER) {
                otmp = mksobj(BATTLE_AXE, FALSE, FALSE);
                otmp = oname(otmp, artiname(ART_CLEAVER));
                bless(otmp);
                otmp->oerodeproof = TRUE;
                otmp->spe = rn2(6);
                (void) mpickobj(mtmp, otmp);

                (void) mongets(mtmp, CLOAK_OF_MAGIC_RESISTANCE);
            }
            else if(is_mercenary(ptr)) {
                int w1 = 0, w2 = 0;
                switch (mm) {
                    case PM_WATCHMAN:
                    case PM_SOLDIER:
                        if (!rn2(3)) {
                            w1 = rn1(BEC_DE_CORBIN - PARTISAN + 1, PARTISAN);
                            w2 = rn2(2) ? DAGGER : KNIFE;
                        } else w1 = rn2(2) ? SPEAR : SHORT_SWORD;
                        break;
                    case PM_SERGEANT:
                        w1 = rn2(2) ? FLAIL : MACE;
                        break;
                    case PM_LIEUTENANT:
                        w1 = rn2(2) ? BROADSWORD : LONG_SWORD;
                        break;
#ifdef CONVICT
                    case PM_PRISON_GUARD:
#endif /* CONVICT */
                    case PM_CAPTAIN:
                    case PM_WATCH_CAPTAIN:
                        w1 = rn2(2) ? LONG_SWORD : SILVER_SABER;
                        break;
                    default:
                        if (!rn2(4)) w1 = DAGGER;
                        if (!rn2(7)) w2 = SPEAR;
                        break;
                }
                if (w1) (void)mongets(mtmp, w1);
                if (!w2 && w1 != DAGGER && !rn2(4)) w2 = KNIFE;
                if (w2) (void)mongets(mtmp, w2);
        } else if (is_elf(ptr)) {
            if (rn2(2))
                (void) mongets(mtmp,
                        rn2(2) ? ELVEN_MITHRIL_COAT : ELVEN_CLOAK);
            if (rn2(2)) (void)mongets(mtmp, ELVEN_LEATHER_HELM);
            else if (!rn2(4)) (void)mongets(mtmp, ELVEN_BOOTS);
            if (rn2(2)) (void)mongets(mtmp, ELVEN_DAGGER);
            switch (rn2(3)) {
                case 0:
                    if (!rn2(4)) (void)mongets(mtmp, ELVEN_SHIELD);
                    if (rn2(3)) (void)mongets(mtmp, ELVEN_SHORT_SWORD);
                    (void)mongets(mtmp, ELVEN_BOW);
                    m_initthrow(mtmp, ELVEN_ARROW, 12);
                    break;
                case 1:
                    (void)mongets(mtmp, ELVEN_BROADSWORD);
                    if (rn2(2)) (void)mongets(mtmp, ELVEN_SHIELD);
                    break;
                case 2:
                    if (rn2(2)) {
                        (void)mongets(mtmp, ELVEN_SPEAR);
                        (void)mongets(mtmp, ELVEN_SHIELD);
                    }
                    break;
            }
            if (mm == PM_ELVENKING) {
                if (rn2(3) || (in_mklev && Is_earthlevel(&u.uz)))
                    (void)mongets(mtmp, PICK_AXE);
                if (!rn2(50)) (void)mongets(mtmp, CRYSTAL_BALL);
            }
        } else if (ptr->msound == MS_PRIEST ||
                    quest_mon_represents_role(ptr, PM_PRIEST)) {
                otmp = mksobj(MACE, FALSE, FALSE);
                if(otmp) {
                    otmp->spe = rnd(3);
                    if(!rn2(2)) curse(otmp);
                    (void) mpickobj(mtmp, otmp);
                }
        } else if (mm == PM_NINJA) { /* extra quest villains */
            (void) mongets(mtmp, rn2(4) ? SHURIKEN : DART);
            (void) mongets(mtmp, rn2(4) ? SHORT_SWORD : AXE);
        } else if (ptr->msound == MS_GUARDIAN) {
            /* quest "guardians" */
            switch (mm) {
            case PM_STUDENT:
            case PM_ATTENDANT:
            case PM_ABBOT:
            case PM_ACOLYTE:
            case PM_GUIDE:
            case PM_APPRENTICE:
                if (rn2(2)) {
                    (void) mongets(mtmp, rn2(3) ? DAGGER : KNIFE);
                }
                if (rn2(5)) {
                    (void) mongets(mtmp, rn2(3) ? LEATHER_JACKET : LEATHER_CLOAK);
                }
                if (rn2(3)) {
                    (void) mongets(mtmp, rn2(3) ? LOW_BOOTS : HIGH_BOOTS);
                }
                if (rn2(3)) {
                    (void) mongets(mtmp, POT_HEALING);
                }
                break;
            case PM_CHIEFTAIN:
            case PM_PAGE:
            case PM_ROSHI:
            case PM_WARRIOR:
                (void) mongets(mtmp, rn2(3) ? LONG_SWORD : SHORT_SWORD);
                (void) mongets(mtmp, rn2(3) ? CHAIN_MAIL : LEATHER_ARMOR);
                if (rn2(2)) {
                    (void) mongets(mtmp, rn2(2) ? LOW_BOOTS : HIGH_BOOTS);
                }
                if (!rn2(3)) {
                    (void) mongets(mtmp, LEATHER_CLOAK);
                }
                if (!rn2(3)) {
                    (void) mongets(mtmp, BOW);
                    m_initthrow(mtmp, ARROW, 12);
                }
                break;
            case PM_HUNTER:
                (void) mongets(mtmp, rn2(3) ? SHORT_SWORD : DAGGER);
                if (rn2(2)) {
                    (void) mongets(mtmp, rn2(2) ? LEATHER_JACKET : LEATHER_ARMOR);
                }
                (void) mongets(mtmp, BOW);
                m_initthrow(mtmp, ARROW, 12);
                break;
            case PM_THUG:
                (void) mongets(mtmp, CLUB);
                (void) mongets(mtmp, rn2(3) ? DAGGER : KNIFE);
                if (rn2(2)) {
                    (void) mongets(mtmp, LEATHER_GLOVES);
                }
                (void) mongets(mtmp, rn2(2) ? LEATHER_JACKET : LEATHER_ARMOR);
                break;
            case PM_NEANDERTHAL:
                (void) mongets(mtmp, CLUB);
                (void) mongets(mtmp, LEATHER_ARMOR);
                break;
            }
#ifdef CONVICT
        } else if (mm == PM_MINER) {
            (void)mongets(mtmp, PICK_AXE);
            otmp = mksobj(BRASS_LANTERN, TRUE, FALSE);
            (void) mpickobj(mtmp, otmp);
            begin_burn(otmp, FALSE);
#endif /* CONVICT */
        }
        break;

        case S_ANGEL:
            if (mm == PM_ALEAX) {
                /* mimic your weapon */
                if (uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
                    m_inityour(mtmp, uwep);
                else (void) mongets(mtmp, LONG_SWORD);
                if (uswapwep && (uswapwep->oclass == WEAPON_CLASS ||
                            is_weptool(uswapwep)))
                    m_inityour(mtmp, uswapwep);
                if (uquiver && (is_ammo(uquiver) || is_missile(uquiver)))
                    m_inityour(mtmp, uquiver);

                /* mimic your armor */
                if (uarm) m_inityour(mtmp, uarm);
                if (uarmc) m_inityour(mtmp, uarmc);
                if (uarmh) m_inityour(mtmp, uarmh);
                if (uarms) m_inityour(mtmp, uarms);
                if (uarmg) m_inityour(mtmp, uarmg);
                if (uarmf) m_inityour(mtmp, uarmf);
#ifdef TOURIST
                if (uarmu) m_inityour(mtmp, uarmu);
#endif
                /* same chance for an amulet of reflection as other
                   angelic beings have for a shield of reflection */
                if (!rn2(4)) (void)mongets(mtmp, AMULET_OF_REFLECTION);
            } else {
                int spe2;

                /* create minion stuff; can't use mongets */
                otmp = mksobj(LONG_SWORD, FALSE, FALSE);

                /* maybe make it special */
                if (!rn2(20) || is_lord(ptr))
                    otmp = oname(otmp, artiname(
                                rn2(2) ? ART_DEMONBANE : ART_SUNSWORD));
                bless(otmp);
                otmp->oerodeproof = TRUE;
                otmp->spe = max(otmp->spe, rn2(4));
                (void) mpickobj(mtmp, otmp);

                otmp = mksobj(!rn2(4) || is_lord(ptr) ?
                        SHIELD_OF_REFLECTION : LARGE_SHIELD,
                        FALSE, FALSE);
                uncurse(otmp);
                otmp->oerodeproof = TRUE;
                otmp->spe = 0;
                (void) mpickobj(mtmp, otmp);
            }
            break;

        case S_HUMANOID:
            if (mm == PM_HOBBIT) {
                switch (rn2(3)) {
                    case 0:
                        (void)mongets(mtmp, DAGGER);
                        break;
                    case 1:
                        (void)mongets(mtmp, ELVEN_DAGGER);
                        break;
                    case 2:
                        (void)mongets(mtmp, SLING);
                        break;
                }
                if (!rn2(10)) (void)mongets(mtmp, ELVEN_MITHRIL_COAT);
                if (!rn2(10)) (void)mongets(mtmp, DWARVISH_CLOAK);
            } else if (is_dwarf(ptr)) {
                if (rn2(7)) (void)mongets(mtmp, DWARVISH_CLOAK);
                if (rn2(7)) (void)mongets(mtmp, IRON_SHOES);
                if (!rn2(4)) {
                    (void)mongets(mtmp, DWARVISH_SHORT_SWORD);
                    /* note: you can't use a mattock with a shield */
                    if (rn2(2)) (void)mongets(mtmp, DWARVISH_MATTOCK);
                    else {
                        (void)mongets(mtmp, rn2(2) ? AXE : DWARVISH_SPEAR);
                        (void)mongets(mtmp, DWARVISH_ROUNDSHIELD);
                    }
                    (void)mongets(mtmp, DWARVISH_IRON_HELM);
                    if (!rn2(3))
                        (void)mongets(mtmp, DWARVISH_MITHRIL_COAT);
                } else {
                    (void)mongets(mtmp, !rn2(3) ? PICK_AXE : DAGGER);
                }
                if (!In_mines(&u.uz) && rn2(7)) {
                    /* outside the mines, dwarves sometimes have booze */
                    mongets(mtmp, POT_BOOZE);
                }
            }
            break;
# ifdef KOPS
        case S_KOP:     /* create Keystone Kops with cream pies to
                         * throw. As suggested by KAA.     [MRS]
                         */
            if (!rn2(4)) m_initthrow(mtmp, CREAM_PIE, 2);
            if (!rn2(3)) (void)mongets(mtmp, (rn2(2)) ? CLUB : RUBBER_HOSE);
            break;
# endif

        case S_ORC:
            if(rn2(2)) (void)mongets(mtmp, ORCISH_HELM);
            switch (mm != PM_ORC_CAPTAIN ? mm :
                    rn2(2) ? PM_MORDOR_ORC : PM_URUK_HAI) {
                case PM_MORDOR_ORC:
                    if(!rn2(3)) (void)mongets(mtmp, SCIMITAR);
                    if(!rn2(3)) (void)mongets(mtmp, ORCISH_SHIELD);
                    if(!rn2(3)) (void)mongets(mtmp, KNIFE);
                    if(!rn2(3)) (void)mongets(mtmp, ORCISH_CHAIN_MAIL);
                    break;
                case PM_URUK_HAI:
                    if(!rn2(3)) (void)mongets(mtmp, ORCISH_CLOAK);
                    if(!rn2(3)) (void)mongets(mtmp, ORCISH_SHORT_SWORD);
                    if(!rn2(3)) (void)mongets(mtmp, IRON_SHOES);
                    if(!rn2(3)) {
                        (void)mongets(mtmp, ORCISH_BOW);
                        m_initthrow(mtmp, ORCISH_ARROW, 12);
                    }
                    if(!rn2(3)) (void)mongets(mtmp, URUK_HAI_SHIELD);
                    break;
                default:
                    if (mm != PM_ORC_SHAMAN && rn2(2))
                        (void)mongets(mtmp, (mm == PM_GOBLIN || rn2(2) == 0)
                                ? ORCISH_DAGGER : SCIMITAR);
            }
            break;

        case S_OGRE:
            if (!rn2(mm == PM_OGRE_KING ? 3 : mm == PM_OGRE_LORD ? 6 : 12))
                (void) mongets(mtmp, BATTLE_AXE);
            else
                (void) mongets(mtmp, CLUB);
            break;

        case S_TROLL:
            if (!rn2(2)) switch (rn2(4)) {
                case 0: (void)mongets(mtmp, RANSEUR); break;
                case 1: (void)mongets(mtmp, PARTISAN); break;
                case 2: (void)mongets(mtmp, GLAIVE); break;
                case 3: (void)mongets(mtmp, SPETUM); break;
            }
            break;

        case S_KOBOLD:
            if (!rn2(4)) m_initthrow(mtmp, DART, 12);
            break;

        case S_CENTAUR:
            if (rn2(2)) {
                if(ptr == &mons[PM_FOREST_CENTAUR]) {
                    (void)mongets(mtmp, BOW);
                    m_initthrow(mtmp, ARROW, 12);
                } else {
                    (void)mongets(mtmp, CROSSBOW);
                    m_initthrow(mtmp, CROSSBOW_BOLT, 12);
                }
            }
            break;

        case S_WRAITH:
            (void)mongets(mtmp, KNIFE);
            (void)mongets(mtmp, LONG_SWORD);
            break;

        case S_ZOMBIE:
            if (!rn2(4)) (void)mongets(mtmp, LEATHER_ARMOR);
            if (!rn2(4))
                (void)mongets(mtmp, (rn2(3) ? KNIFE : SHORT_SWORD));
            break;

        case S_LIZARD:
            if (mm == PM_SALAMANDER)
                (void)mongets(mtmp, (rn2(7) ? SPEAR : rn2(3) ?
                            TRIDENT : STILETTO));
            break;

        case S_DEMON:
            switch (mm) {
                case PM_BALROG:
                    (void)mongets(mtmp, BULLWHIP);
                    (void)mongets(mtmp, BROADSWORD);
                    break;
                case PM_HORNED_DEVIL:
                    (void)mongets(mtmp, rn2(4) ? TRIDENT : BULLWHIP);
                    break;
                case PM_DISPATER:
                    (void)mongets(mtmp, WAN_STRIKING);
                    break;
                case PM_YEENOGHU:
                    (void)mongets(mtmp, FLAIL);
                    break;
            }
            /* prevent djinnis and mail daemons from leaving objects when
             * they vanish
             */
            if (!is_demon(ptr)) break;
            /* fall thru */
            /*
             *  Now the general case, Some chance of getting some type
             *  of weapon for "normal" monsters.  Certain special types
             *  of monsters will get a bonus chance or different selections.
             */
        default:
            {
                /*
                 * Now the general case, some chance of getting some type
                 * of weapon for "normal" monsters.  Certain special types
                 * of monsters will get a bonus chance or different selections.
                 */
                int bias;

                bias = is_lord(ptr) + is_prince(ptr) * 2 + extra_nasty(ptr);
                switch(rnd(14 - (2 * bias))) {
                    case 1:
                        if(strongmonst(ptr)) (void) mongets(mtmp, BATTLE_AXE);
                        else m_initthrow(mtmp, DART, 12);
                        break;
                    case 2:
                        if(strongmonst(ptr))
                            (void) mongets(mtmp, TWO_HANDED_SWORD);
                        else {
                            (void) mongets(mtmp, CROSSBOW);
                            m_initthrow(mtmp, CROSSBOW_BOLT, 12);
                        }
                        break;
                    case 3:
                        (void) mongets(mtmp, BOW);
                        m_initthrow(mtmp, ARROW, 12);
                        break;
                    case 4:
                        if(strongmonst(ptr)) (void) mongets(mtmp, LONG_SWORD);
                        else m_initthrow(mtmp, DAGGER, 3);
                        break;
                    case 5:
                        if(strongmonst(ptr))
                            (void) mongets(mtmp, LUCERN_HAMMER);
                        else (void) mongets(mtmp, AKLYS);
                        break;
                    default:
                        break;
                }
            }
            break;
    }
    if ((int) mtmp->m_lev > rn2(75))
        (void) mongets(mtmp, rnd_offensive_item(mtmp));
}

/*
 *   Makes up money for monster's inventory.
 */
void
mkmonmoney(mtmp, amount)
struct monst *mtmp;
long amount;
{
    struct obj *gold = mksobj(GOLD_PIECE, FALSE, FALSE);

    gold->quan = amount;
    add_to_minv(mtmp, gold);
}

STATIC_OVL void
m_initinv(mtmp)
register struct monst   *mtmp;
{
    register int cnt;
    register struct obj *otmp;
    register struct permonst *ptr = mtmp->data;
    int i;
#ifdef REINCARNATION
    if (Is_rogue_level(&u.uz)) return;
#endif
    /*
     *  Soldiers get armour & rations - armour approximates their ac.
     *  Nymphs may get mirror or potion of object detection.
     */
    switch(ptr->mlet) {

        case S_HUMAN:
            if(is_mercenary(ptr)) {
                register int mac;

                switch(monsndx(ptr)) {
                    case PM_GUARD: mac = -1; break;
#ifdef CONVICT
                    case PM_PRISON_GUARD: mac = -2; break;
#endif /* CONVICT */
                    case PM_SOLDIER: mac = 3; break;
                    case PM_SERGEANT: mac = 0; break;
                    case PM_LIEUTENANT: mac = -2; break;
                    case PM_CAPTAIN: mac = -3; break;
                    case PM_WATCHMAN: mac = 3; break;
                    case PM_WATCH_CAPTAIN: mac = -2; break;
                    default: warning("odd mercenary %d?", monsndx(ptr));
                             mac = 0;
                             break;
                }

                if (mac < -1 && rn2(5))
                    mac += 7 + mongets(mtmp, (rn2(5)) ?
                            PLATE_MAIL : CRYSTAL_PLATE_MAIL);
                else if (mac < 3 && rn2(5))
                    mac += 6 + mongets(mtmp, (rn2(3)) ?
                            SPLINT_MAIL : BANDED_MAIL);
                else if (rn2(5))
                    mac += 3 + mongets(mtmp, (rn2(3)) ?
                            RING_MAIL : STUDDED_LEATHER_ARMOR);
                else
                    mac += 2 + mongets(mtmp, LEATHER_ARMOR);

                if (mac < 10 && rn2(3))
                    mac += 1 + mongets(mtmp, HELMET);
                else if (mac < 10 && rn2(2))
                    mac += 1 + mongets(mtmp, DENTED_POT);
                if (mac < 10 && rn2(3))
                    mac += 1 + mongets(mtmp, SMALL_SHIELD);
                else if (mac < 10 && rn2(2))
                    mac += 2 + mongets(mtmp, LARGE_SHIELD);
                if (mac < 10 && rn2(3))
                    mac += 1 + mongets(mtmp, LOW_BOOTS);
                else if (mac < 10 && rn2(2))
                    mac += 2 + mongets(mtmp, HIGH_BOOTS);
                if (mac < 10 && rn2(3))
                    mac += 1 + mongets(mtmp, LEATHER_GLOVES);
                else if (mac < 10 && rn2(2))
                    mac += 1 + mongets(mtmp, LEATHER_CLOAK);

                if(ptr != &mons[PM_GUARD] &&
#ifdef CONVICT
                        ptr != &mons[PM_PRISON_GUARD] &&
#endif /* CONVICT */
                        ptr != &mons[PM_WATCHMAN] &&
                        ptr != &mons[PM_WATCH_CAPTAIN]) {
                    if (!rn2(3)) (void) mongets(mtmp, K_RATION);
                    if (!rn2(2)) (void) mongets(mtmp, C_RATION);
                    if (ptr != &mons[PM_SOLDIER] && !rn2(3))
                        (void) mongets(mtmp, BUGLE);
                } else
                    if (ptr == &mons[PM_WATCHMAN] && rn2(3))
                        (void) mongets(mtmp, TIN_WHISTLE);
            } else if (ptr == &mons[PM_SHOPKEEPER]) {
                (void) mongets(mtmp, SKELETON_KEY);
                switch (rn2(4)) {
                    /* MAJOR fall through ... */
                    case 0: (void) mongets(mtmp, WAN_MAGIC_MISSILE);
                            /* fall through */
                    case 1: (void) mongets(mtmp, POT_EXTRA_HEALING);
                            /* fall through */
                    case 2: (void) mongets(mtmp, POT_HEALING);
                            /* fall through */
                    case 3: (void) mongets(mtmp, WAN_STRIKING);
                }
            } else if (ptr == &mons[PM_ONE_EYED_SAM]) {
                otmp = mksobj(TWO_HANDED_SWORD, FALSE, FALSE);
                otmp = oname(otmp, artiname(ART_THIEFBANE));
                mpickobj(mtmp, otmp);
                if (otmp->spe < 5) otmp->spe += rnd(5);
                otmp = mksobj(SHIELD_OF_REFLECTION, FALSE, FALSE);
                mpickobj(mtmp, otmp);
                if (otmp->spe < 5) otmp->spe += rnd(5);
                otmp = mksobj(GRAY_DRAGON_SCALE_MAIL, FALSE, FALSE);
                mpickobj(mtmp, otmp);
                if (otmp->spe < 5) otmp->spe += rnd(5);
                otmp = mksobj(SPEED_BOOTS, FALSE, FALSE);
                mpickobj(mtmp, otmp);
                if (otmp->spe < 5) otmp->spe += rnd(5);
                otmp = mksobj(AMULET_OF_LIFE_SAVING, FALSE, FALSE);
                mpickobj(mtmp, otmp);
                (void) mongets(mtmp, SKELETON_KEY);
            } else if (ptr->msound == MS_PRIEST ||
                    quest_mon_represents_role(ptr, PM_PRIEST)) {
                (void) mongets(mtmp, rn2(7) ? ROBE :
                        rn2(3) ? CLOAK_OF_PROTECTION :
                        CLOAK_OF_MAGIC_RESISTANCE);
                (void) mongets(mtmp, SMALL_SHIELD);
                mkmonmoney(mtmp, (long)rn1(10, 20));
            } else if (quest_mon_represents_role(ptr, PM_MONK)) {
                (void) mongets(mtmp, rn2(11) ? ROBE :
                        CLOAK_OF_MAGIC_RESISTANCE);
            }
            break;

        case S_NYMPH:
            if(!rn2(2)) (void) mongets(mtmp, MIRROR);
            if(!rn2(2)) (void) mongets(mtmp, POT_OBJECT_DETECTION);
            break;

        case S_GIANT:
            if (ptr == &mons[PM_MINOTAUR]) {
                if (!rn2(3) || (in_mklev && Is_earthlevel(&u.uz)))
                    (void) mongets(mtmp, WAN_DIGGING);
            } else if (is_giant(ptr)) {
                for (cnt = rn2((int)(mtmp->m_lev / 2)); cnt; cnt--) {
                    otmp = mksobj(rnd_class(DILITHIUM_CRYSTAL, LUCKSTONE-1),
                            FALSE, FALSE);
                    otmp->quan = (long) rn1(2, 3);
                    otmp->owt = weight(otmp);
                    (void) mpickobj(mtmp, otmp);
                }
            }
            break;

        case S_WRAITH:
            if (ptr == &mons[PM_NAZGUL]) {
                otmp = mksobj(RIN_INVISIBILITY, FALSE, FALSE);
                curse(otmp);
                (void) mpickobj(mtmp, otmp);
            }
            break;

        case S_LICH:
            if (ptr == &mons[PM_MASTER_LICH] && !rn2(13))
                (void)mongets(mtmp, (rn2(7) ? ATHAME : WAN_NOTHING));
            else if (ptr == &mons[PM_ARCH_LICH] && !rn2(3)) {
                otmp = mksobj(rn2(3) ? ATHAME : QUARTERSTAFF,
                        TRUE, rn2(13) ? FALSE : TRUE);
                if (otmp->spe < 2) otmp->spe = rnd(3);
                if (!rn2(4)) otmp->oerodeproof = 1;
                (void) mpickobj(mtmp, otmp);
            }
            break;

        case S_MUMMY:
            if (rn2(7)) (void)mongets(mtmp, MUMMY_WRAPPING);
            break;

        case S_QUANTMECH:
            if (!rn2(20)) {
                struct obj *catcorpse;

                otmp = mksobj(LARGE_BOX, FALSE, FALSE);
                /* we used to just set the flag, which resulted in weight()
                treating the box as being heavier by the weight of a cat;
                now we include a cat corpse that won't rot; when opening or
                disclosing the box's contents, the corpse might be revived,
                otherwise it's given a rot timer; weight is now ordinary */
                if ((catcorpse = mksobj(CORPSE, TRUE, FALSE)) != 0) {
                    otmp->spe = 1; /* flag for special SchroedingersBox */
                    set_corpsenm(catcorpse, PM_HOUSECAT);
                    (void) stop_timer(ROT_CORPSE, obj_to_any(catcorpse));
                    add_to_container(otmp, catcorpse);
                    otmp->owt = weight(otmp);
            }
            (void) mpickobj(mtmp, otmp);
            }
            break;

        case S_LEPRECHAUN:
            mkmonmoney(mtmp, (long) d(level_difficulty(), 30));
            break;

        case S_VAMPIRE:
            /* some vampires get an opera cloak */
            for (i = CLOAK_OF_PROTECTION; i < NUM_OBJECTS; i++) {
                const char *zn;
                if ((zn = OBJ_DESCR(objects[i])) && !strcmpi(zn, "opera cloak")) {
                    if (!OBJ_NAME(objects[i])) i = STRANGE_OBJECT;
                    break;
                }
            }
            if (i != NUM_OBJECTS && rnf(1, 8)) {
                mongets(mtmp, i);
            }

            if (rn2(2)) {
                if ((int) mtmp->m_lev > rn2(30))
                    (void)mongets(mtmp, POT_VAMPIRE_BLOOD);
                else
                    (void)mongets(mtmp, POT_BLOOD);
            }
            break;

        case S_DEMON:
            /* moved here from m_initweap() because these don't
               have AT_WEAP so m_initweap() is not called for them */
            if (ptr == &mons[PM_ICE_DEVIL] && !rn2(4)) {
                (void)mongets(mtmp, SPEAR);
            }
            break;

        case S_HUMANOID:
            /* [DS] Cthulhu isn't fully integrated yet, and he won't be
             *      until Moloch's Sanctum is rearranged */
            if (ptr == &mons[PM_CTHULHU]) {
                (void)mongets(mtmp, AMULET_OF_YENDOR);
                (void)mongets(mtmp, POT_FULL_HEALING);
            }
            break;

        case S_GNOME:
            /* In AceHack, these have a chance of generating with
               candles, especially on dark Mines levels. */
            if (In_mines(&u.uz) && rnf(1, 5)) {
                /* Pick a random square. If it's a floor square and unlit,
                   generate a lit candle. If it isn't a floor square,
                   pick once more. */
                int x, y;
                x = rnd(COLNO-1); y = rn2(ROWNO);
                if (isok(x, y) && levl[x][y].typ != ROOM) {
                    x = rnd(COLNO-1); y = rn2(ROWNO);
                }
                if (isok(x, y) && levl[x][y].typ == ROOM &&
                        !levl[x][y].lit) {
                    otmp = mksobj(rn2(4) ? TALLOW_CANDLE : WAX_CANDLE,
                            TRUE, FALSE);
                    otmp->quan = 1;
                    /* We need to add the object to the monster inventory
                       before we light it. */
                    if (!mpickobj(mtmp, otmp)) {
                        /* mpickobj returns FALSE if the object is still
                           addressable, i.e. not merged with another object */
                        begin_burn(otmp, FALSE);
                    }
                } else if (!rn2(5)) {
                    /* Add a candle to inventory anyway, but don't light it. */
                    otmp = mksobj(rn2(4) ? TALLOW_CANDLE : WAX_CANDLE,
                            TRUE, FALSE);
                    otmp->quan = 1;
                    (void) mpickobj(mtmp, otmp);
                }
            } else if (!rn2(10)) {
                /* A small chance gnomes away from home have one too */
                otmp = mksobj(rn2(4) ? TALLOW_CANDLE : WAX_CANDLE,
                        TRUE, FALSE);
                otmp->quan = 1;
                (void) mpickobj(mtmp, otmp);
            }
            break;

        case S_ANGEL:
            if (is_weeping(ptr))
                if(!rn2(3)) (void) mongets(mtmp, POT_BLINDNESS);
            if (ptr == &mons[PM_WEEPING_ARCHANGEL])
                if(!rn2(3)) {
                    (void) mongets(mtmp, WAN_LIGHTNING);
                    (void) mongets(mtmp, AMULET_OF_REFLECTION);
                }
            break;
        default:
            break;
    }

    /* ordinary soldiers rarely have access to magic (or gold :-) */
    if (ptr == &mons[PM_SOLDIER] && rn2(13)) return;

    if ((int) mtmp->m_lev > rn2(50))
        (void) mongets(mtmp, rnd_defensive_item(mtmp));
    if ((int) mtmp->m_lev > rn2(100))
        (void) mongets(mtmp, rnd_misc_item(mtmp));
    if (likes_gold(ptr) && !findgold(mtmp->minvent) && !rn2(5))
        mkmonmoney(mtmp, (long) d(level_difficulty(), mtmp->minvent ? 5 : 10));
}

/* Note: for long worms, always call cutworm (cutworm calls clone_mon) */
struct monst *
clone_mon(mon, x, y)
struct monst *mon;
xchar x, y; /* clone's preferred location or 0 (near mon) */
{
    coord mm;
    struct monst *m2;

    /* may be too weak or have been extinguished for population control */
    if (mon->mhp <= 1 || (mvitals[monsndx(mon->data)].mvflags & G_EXTINCT))
        return (struct monst *) 0;

    if (x == 0) {
        mm.x = mon->mx;
        mm.y = mon->my;
    } else {
        mm.x = x;
        mm.y = y;
    }
    if (!isok(mm.x, mm.y)) { /* paranoia */
        impossible("clone_mon trying to create a monster at <%d,%d>?",
                   mm.x, mm.y);
        return (struct monst *) 0;
    }
    if (MON_AT(mm.x, mm.y)) { /* (always True for the x==0 case) */
        if (!enexto(&mm, mm.x, mm.y, mon->data) || MON_AT(mm.x, mm.y))
            return (struct monst *) 0;
    }

    m2 = newmonst();
    *m2 = *mon; /* copy condition of old monster */
    m2->mextra = (struct mextra *) 0;
    m2->nmon = fmon;
    fmon = m2;
    m2->m_id = flags.ident++;
    if (!m2->m_id) {
        m2->m_id = flags.ident++; /* ident overflowed */
    }
    m2->mx = mm.x;
    m2->my = mm.y;

    m2->mundetected = 0;
    m2->mtrapped = 0;
    m2->mcloned = 1;
    m2->minvent = (struct obj *) 0; /* objects don't clone */
    m2->mleashed = 0;
    /* Max HP the same, but current HP halved for both.  The caller
     * might want to override this by halving the max HP also.
     * When current HP is odd, the original keeps the extra point.
     * We know original has more than 1 HP, so both end up with at least 1.
     */
    m2->mhpmax = mon->mhpmax;
    m2->mhp = mon->mhp / 2;
    mon->mhp -= m2->mhp;

    /* clone doesn't have mextra so mustn't retain special monster flags */
    m2->isshk = 0;
    m2->isgd = 0;
    m2->ispriest = 0;
    /* ms->isminion handled below */

    /* clone shouldn't be reluctant to move on spots 'parent' just moved on */
    (void) memset((genericptr_t) m2->mtrack, 0, sizeof m2->mtrack);

    place_monster(m2, m2->mx, m2->my);
    if (emits_light(m2->data))
        new_light_source(m2->mx, m2->my, emits_light(m2->data), LS_MONSTER,
                         monst_to_any(m2));
    /* if 'parent' is named, give the clone the same name */
    if (has_mname(mon)) {
        m2 = christen_monst(m2, MNAME(mon));
    } else if (mon->isshk) {
        m2 = christen_monst(m2, shkname(mon));
    }

    /* not all clones caused by player are tame or peaceful */
    if (!flags.mon_moving && mon->mpeaceful) {
        if (mon->mtame)
            m2->mtame = rn2(max(2 + u.uluck, 2)) ? mon->mtame : 0;
        else if (mon->mpeaceful)
            m2->mpeaceful = rn2(max(2 + u.uluck, 2)) ? 1 : 0;
    }
    /* if guardian angel could be cloned (maybe after polymorph?),
       m2 could be both isminion and mtame; isminion takes precedence */
    if (m2->isminion) {
        int atyp;

        newemin(m2);
        *EMIN(m2) = *EMIN(mon);
        /* renegade when same alignment as hero but not peaceful or
           when peaceful while being different alignment from hero */
        atyp = EMIN(m2)->min_align;
        EMIN(m2)->renegade = (atyp != u.ualign.type) ^ !m2->mpeaceful;
    } else if (m2->mtame) {
        /* Because m2 is a copy of mon it is tame but not init'ed.
           However, tamedog() will not re-tame a tame dog, so m2
           must be made non-tame to get initialized properly. */
        m2->mtame = 0;
        if (tamedog(m2, (struct obj *) 0))
            *EDOG(m2) = *EDOG(mon);
        /* [TODO? some (most? all?) edog fields probably should be
           reinitialized rather that retain the 'parent's values] */
    }
    set_malign(m2);
    newsym(m2->mx, m2->my); /* display the new monster */

    return m2;
}

/*
 * Propagate a species
 *
 * Once a certain number of monsters are created, don't create any more
 * at random (i.e. make them extinct).  The previous (3.2) behavior was
 * to do this when a certain number had _died_, which didn't make
 * much sense.
 *
 * Returns FALSE propagation unsuccessful
 *         TRUE  propagation successful
 */
boolean
propagate(mndx, tally, ghostly)
int mndx;
boolean tally;
boolean ghostly;
{
    boolean result;
    uchar lim = mbirth_limit(mndx);
    boolean gone = (mvitals[mndx].mvflags & G_GONE); /* genocided or extinct */

    result = (((int) mvitals[mndx].born < lim) && !gone) ? TRUE : FALSE;

    /* if it's unique, don't ever make it again */
    if (mons[mndx].geno & G_UNIQ) mvitals[mndx].mvflags |= G_EXTINCT;

    if (mvitals[mndx].born < 255 && tally && (!ghostly || (ghostly && result)))
        mvitals[mndx].born++;
    if ((int) mvitals[mndx].born >= lim && !(mons[mndx].geno & G_NOGEN) &&
            !(mvitals[mndx].mvflags & G_EXTINCT)) {
#if defined(DEBUG) && defined(WIZARD)
        if (wizard) pline("Automatically extinguished %s.",
                makeplural(mons[mndx].mname));
#endif
        mvitals[mndx].mvflags |= G_EXTINCT;
        reset_rndmonst(mndx);
    }
    return result;
}

/* amount of HP to lose from level drain (or gain from Stormbringer) */
int
monhp_per_lvl(mon)
struct monst *mon;
{
    struct permonst *ptr = mon->data;
    int hp = rnd(8); /* default is d8 */

    /* like newmonhp, but home elementals are ignored, riders use normal d8 */
    if (is_golem(ptr)) {
        /* draining usually won't be applicable for these critters */
        hp = golemhp(monsndx(ptr)) / (int) ptr->mlevel;
    } else if (ptr->mlevel > 49) {
        /* arbitrary; such monsters won't be involved in draining anyway */
        hp = 4 + rnd(4); /* 5..8 */
    } else if (ptr->mlet == S_DRAGON && monsndx(ptr) >= PM_GRAY_DRAGON) {
        /* adult dragons; newmonhp() uses In_endgame(&u.uz) ? 8 : 4 + rnd(4)
         */
        hp = 4 + rn2(5); /* 4..8 */
    } else if (!mon->m_lev) {
        /* level 0 monsters use 1d4 instead of Nd8 */
        hp = rnd(4);
    }
    return hp;
}

static boolean
makemon_rnd_goodpos(mon, gpflags, cc)
struct monst *mon;
unsigned gpflags;
coord *cc;
{
    int tryct = 0;
    int nx,ny;
    boolean good;

    do {
        nx = rn1(COLNO - 3, 2);
        ny = rn2(ROWNO);
        good = (!in_mklev && cansee(nx,ny)) ? FALSE
                                            : goodpos(nx, ny, mon, gpflags);
    } while ((++tryct < 50) && !good);

    if (!good) {
        /* else go through all map positions, twice, first round
           ignoring positions in sight, and pick first good one.
           skip first round if we're in special level loader or blind */
        int xofs = nx;
        int yofs = ny;
        int dx,dy;
        int bl = (in_mklev || Blind) ? 1 : 0;

        for ( ; bl < 2; bl++) {
            for (dx = 0; dx < COLNO; dx++) {
                for (dy = 0; dy < ROWNO; dy++) {
                    nx = ((dx + xofs) % (COLNO - 1)) + 1;
                    ny = ((dy + yofs) % (ROWNO - 1)) + 1;
                    if (bl == 0 && cansee(nx,ny)) {
                        continue;
                    }
                    if (goodpos(nx, ny, mon, gpflags)) {
                        goto gotgood;
                    }
                }
            }
            if (bl == 0 && (!mon || mon->data->mmove)) {
                /* all map positions are visible (or not good),
                   try to pick something logical */
                if (dnstair.sx && !rn2(2)) {
                    nx = dnstair.sx;
                    ny = dnstair.sy;
                } else if (upstair.sx && !rn2(2)) {
                    nx = upstair.sx;
                    ny = upstair.sy;
                } else if (dnladder.sx && !rn2(2)) {
                    nx = dnladder.sx;
                    ny = dnladder.sy;
                } else if (upladder.sx && !rn2(2)) {
                    nx = upladder.sx;
                    ny = upladder.sy;
                }
                if (goodpos(nx, ny, mon, gpflags)) {
                    goto gotgood;
                }
            }
        }
    } else {
 gotgood:
        cc->x = nx;
        cc->y = ny;
        return TRUE;
    }
    return FALSE;
}

static
struct monst *
_makemon(ptr, x, y, mmflags)
register struct permonst *ptr;
register int x, y;
register int mmflags;
{
    register struct monst *mtmp;
    int mndx, mcham, ct, mitem, xlth, mhitdie;
    boolean anymon = (!ptr);
    boolean byyou = (x == u.ux && y == u.uy);
    boolean allow_minvent = ((mmflags & NO_MINVENT) == 0);
    boolean countbirth = ((mmflags & MM_NOCOUNTBIRTH) == 0);
    unsigned gpflags = (mmflags & MM_IGNOREWATER) ? MM_IGNOREWATER : 0;

    struct monst fakemon = zeromonst;
    coord cc = { .x = 0, .y = 0 };

    /* if caller wants random location, do it here */
    if (x == 0 && y == 0) {
        fakemon.data = ptr; /* set up for goodpos */
        if (!makemon_rnd_goodpos(ptr ? &fakemon : (struct monst *) 0, gpflags, &cc)) {
            return NULL;
        }
        x = cc.x;
        y = cc.y;
    } else if (byyou && !in_mklev) {
        if (!enexto_core(&cc, u.ux, u.uy, ptr, gpflags)) {
            return NULL;
        }
        x = cc.x;
        y = cc.y;
    }

    /* sanity check */
    if (!isok(x, y)) {
        impossible("makemon trying to create a monster at <%d,%d>?", x, y);
        return (struct monst *) 0;
    }

    /* Does monster already exist at the position? */
    if (MON_AT(x, y)) {
        if (!(mmflags & MM_ADJACENTOK) || !enexto_core(&cc, x, y, ptr, gpflags)) {
            return NULL;
        }
        x = cc.x;
        y = cc.y;
    }

    if (ptr) {
        mndx = monsndx(ptr);
        /* if you are to make a specific monster and it has
           already been genocided, return */
        if (mvitals[mndx].mvflags & G_GENOD) return((struct monst *) 0);
#if defined(WIZARD) && defined(DEBUG)
        if (wizard && (mvitals[mndx].mvflags & G_EXTINCT))
            pline("Explicitly creating extinct monster %s.",
                    mons[mndx].mname);
#endif
    } else {
        /* make a random (common) monster that can survive here.
         * (the special levels ask for random monsters at specific
         * positions, causing mass drowning on the medusa level,
         * for instance.)
         */
        int tryct = 0; /* maybe there are no good choices */

        do {
            if (!(ptr = rndmonst())) {
#ifdef DEBUG
                pline("Warning: no monster.");
#endif
                return((struct monst *) 0); /* no more monsters! */
            }
            fakemon.data = ptr; /* set up for goodpos */
        } while(!goodpos(x, y, &fakemon, gpflags) && tryct++ < 50);
        mndx = monsndx(ptr);
    }
    (void) propagate(mndx, countbirth, FALSE);
    mtmp = newmonst();
    *mtmp = zeromonst; /* clear all entries in structure */

    if (mmflags & MM_EGD)
        newegd(mtmp);
    if (mmflags & MM_EPRI)
        newepri(mtmp);
    if (mmflags & MM_ESHK)
        neweshk(mtmp);
    if (mmflags & MM_EMIN)
        newemin(mtmp);
    if (mmflags & MM_EDOG)
        newedog(mtmp);
    if (mmflags & MM_ASLEEP)
        mtmp->msleeping = 1;

    mtmp->nmon = fmon;
    fmon = mtmp;
    mtmp->m_id = flags.ident++;
    if (!mtmp->m_id) {
        mtmp->m_id = flags.ident++; /* ident overflowed */
    }
    set_mon_data(mtmp, ptr);
    if (ptr->msound == MS_LEADER && quest_info(MS_LEADER) == mndx) {
        quest_status.leader_m_id = mtmp->m_id;
    }
    mtmp->mnum = mndx;

    /* set up level and hit points */
    newmonhp(mtmp, mndx);

    if (is_female(ptr)) {
        mtmp->female = TRUE;
    } else if (is_male(ptr)) {
        mtmp->female = FALSE;
    /* leader and nemesis gender is usually hardcoded in mons[],
       but for ones which can be random, it has already been chosen
       (in role_init(), for possible use by the quest pager code) */
    } else if (ptr->msound == MS_LEADER && quest_info(MS_LEADER) == mndx) {
        mtmp->female = quest_status.ldrgend;
    } else if (ptr->msound == MS_NEMESIS && quest_info(MS_NEMESIS) == mndx) {
        mtmp->female = quest_status.nemgend;
    } else {
        mtmp->female = rn2(2); /* ignored for neuters */
    }

    if (In_sokoban(&u.uz) && !mindless(ptr)) /* know about traps here */
        mtmp->mtrapseen = (1L << (PIT - 1)) | (1L << (HOLE - 1));
    /* quest leader and nemesis both know about all trap types */
    if (ptr->msound == MS_LEADER || ptr->msound == MS_NEMESIS) {
        mtmp->mtrapseen = ~0;
    }

    place_monster(mtmp, x, y);
    mtmp->mcansee = mtmp->mcanmove = TRUE;
    mtmp->mpeaceful = (mmflags & MM_ANGRY) ? FALSE : peace_minded(ptr);

    switch (ptr->mlet) {
        case S_MIMIC:
            set_mimic_sym(mtmp);
            break;

        case S_SPIDER:
        case S_SNAKE:
            if(in_mklev)
                if(x && y)
                    (void) mkobj_at(0, x, y, TRUE);
            (void) hideunder(mtmp);
            break;

        case S_LIGHT:
        case S_ELEMENTAL:
            if (mndx == PM_STALKER || mndx == PM_BLACK_LIGHT) {
                mtmp->perminvis = TRUE;
                mtmp->minvis = TRUE;
            }
            break;

        case S_EEL:
            (void) hideunder(mtmp);
            break;

        case S_LEPRECHAUN:
            mtmp->msleeping = 1;
            break;

        case S_JABBERWOCK:
        case S_NYMPH:
            if (rn2(5) && !u.uhave.amulet) mtmp->msleeping = 1;
            break;

        case S_ORC:
            if (Race_if(PM_ELF)) mtmp->mpeaceful = FALSE;
            break;

        case S_UNICORN:
            if (is_unicorn(ptr) &&
                    sgn(u.ualign.type) == sgn(ptr->maligntyp))
                mtmp->mpeaceful = TRUE;
            break;

        case S_BAT:
            if (Inhell && is_bat(ptr))
                mon_adjust_speed(mtmp, 2, (struct obj *)0);
            break;
    }
    if ((ct = emits_light(mtmp->data)) > 0)
        new_light_source(mtmp->mx, mtmp->my, ct,
                LS_MONSTER, monst_to_any(mtmp));
    mitem = 0; /* extra inventory item for this monster */

    mtmp->cham = NON_PM; /* default is "not a shapechanger" */
    if (!Protection_from_shape_changers &&
        (mcham = pm_to_cham(mndx)) != NON_PM) {
        /* this is a shapechanger after all */
        mtmp->cham = mcham;
        /* Vlad stays in his normal shape so he can carry the Candelabrum */
        if (mndx != PM_VLAD_THE_IMPALER &&
            /* don't generate vampires polymorphed */
            !is_vampire(mtmp->data) &&
            /* Note:  shapechanger's initial form used to be chosen here
               with rndmonst(), yielding a monster which was appropriate
               to the level's difficulty but ignoring the changer's usual
               type selection, so was inappropriate for vampshifters.
               Let newcham() pick the shape. */
             newcham(mtmp, (struct permonst *) 0, FALSE, FALSE)) {
            allow_minvent = FALSE;
        }
    } else if (mndx == PM_WIZARD_OF_YENDOR) {
        mtmp->iswiz = TRUE;
        flags.no_of_wizards++;
        if (flags.no_of_wizards == 1 && Is_earthlevel(&u.uz)) {
            mitem = SPE_DIG;
        }
    } else if (mndx == PM_GHOST && !(mmflags & MM_NONAME)) {
        mtmp = christen_monst(mtmp, rndghostname());
    } else if (mndx == PM_CROESUS) {
        mitem = TWO_HANDED_SWORD;
    } else if (ptr->msound == MS_NEMESIS) {
        mitem = BELL_OF_OPENING;
    } else if (mndx == PM_PESTILENCE) {
        mitem = POT_SICKNESS;
    }
    if (mitem && allow_minvent) {
        (void) mongets(mtmp, mitem);
    }

    if (in_mklev) {
        if (((is_ndemon(ptr)) ||
                    (mndx == PM_WUMPUS) ||
                    (mndx == PM_LONG_WORM) ||
                    (mndx == PM_GIANT_EEL)) && !u.uhave.amulet && rn2(5))
            mtmp->msleeping = TRUE;
    } else {
        if (byyou) {
            newsym(mtmp->mx, mtmp->my);
            set_apparxy(mtmp);
        }
    }
    if (is_dprince(ptr) && ptr->msound == MS_BRIBE) {
        mtmp->mpeaceful = mtmp->minvis = mtmp->perminvis = 1;
        mtmp->mavenge = 0;
        if (uwep && (uwep->oartifact == ART_EXCALIBUR ||
                     uwep->oartifact == ART_DEMONBANE)) {
            mtmp->mpeaceful = mtmp->mtame = FALSE;
        }
    }
#ifndef DCC30_BUG
    if (mndx == PM_LONG_WORM && (mtmp->wormno = get_wormno()) != 0)
#else
        /* DICE 3.0 doesn't like assigning and comparing mtmp->wormno in the
         * same expression.
         */
        if (mndx == PM_LONG_WORM &&
                (mtmp->wormno = get_wormno(), mtmp->wormno != 0))
#endif
        {
            /* we can now create worms with tails - 11/91 */
            initworm(mtmp, rn2(5));
            if (count_wsegs(mtmp)) place_worm_tail_randomly(mtmp, x, y);
        }
    /* it's possible to create an ordinary monster of some special
       types; make sure their extended data is initialized to
       something sensible if caller hasn't specified MM_EPRI|MM_EMIN
       (when they're specified, caller intends to handle this itself) */
    if ((mndx == PM_ALIGNED_PRIEST || mndx == PM_HIGH_PRIEST) ? !(mmflags & (MM_EPRI | MM_EMIN)) :
        (mndx == PM_ANGEL && !(mmflags & MM_EMIN) && !rn2(3))) {
        struct emin *eminp;

        newemin(mtmp);
        eminp = EMIN(mtmp);

        mtmp->isminion = 1;            /* make priest be a roamer */
        eminp->min_align = rn2(3) - 1; /* no A_NONE */
        eminp->renegade = (boolean) ((mmflags & MM_ANGRY) ? 1 : !rn2(3));
        mtmp->mpeaceful = (eminp->min_align == u.ualign.type) ? !eminp->renegade : eminp->renegade;
    }

    set_malign(mtmp); /* having finished peaceful changes */
    if (anymon && !(mmflags & MM_NOGRP)) {
        if ((ptr->geno & G_SGROUP) && rn2(2)) {
            m_initsgrp(mtmp, mtmp->mx, mtmp->my, mmflags);
        } else if (ptr->geno & G_LGROUP) {
            if (rn2(3)) {
                m_initlgrp(mtmp, mtmp->mx, mtmp->my, mmflags);
            } else {
                m_initsgrp(mtmp, mtmp->mx, mtmp->my, mmflags);
            }
        }
    }

    if (allow_minvent) {
        if(is_armed(ptr))
            m_initweap(mtmp); /* equip with weapons / armor */
        m_initinv(mtmp); /* add on a few special items incl. more armor */
        m_dowear(mtmp, TRUE);

        /* domestic animals may get a saddle */
        if (!rn2(100) && can_saddle(mtmp) && is_domestic(ptr)) {
            struct obj *otmp = mksobj(SADDLE, FALSE, FALSE);
            (void) mpickobj(mtmp, otmp);
            mtmp->misc_worn_check |= W_SADDLE;
            otmp->owornmask = W_SADDLE;
            otmp->leashmon = mtmp->m_id;
            update_mon_intrinsics(mtmp, otmp, TRUE, FALSE);
        }
    } else {
        /* no initial inventory is allowed */
        if (mtmp->minvent) discard_minvent(mtmp);
        mtmp->minvent = (struct obj *)0; /* caller expects this */
    }
    if (ptr->mflags3 && !(mmflags & MM_NOWAIT)) {
        if (ptr->mflags3 & M3_WAITFORU)
            mtmp->mstrategy |= STRAT_WAITFORU;
        if (ptr->mflags3 & M3_CLOSE)
            mtmp->mstrategy |= STRAT_CLOSE;
        if (ptr->mflags3 & (M3_WAITMASK | M3_COVETOUS)) {
            mtmp->mstrategy |= STRAT_APPEARMSG;
        }
    }

    /* heaven or hell mode */
    if (heaven_or_hell_mode && !hell_and_hell_mode)
    {
        mtmp->mhpmax = 1;
        mtmp->mhp = 1;
    }

    if (allow_minvent && migrating_objs) {
        deliver_obj_to_mon(mtmp, 1, DF_NONE); /* in case of waiting items */
    }

    if (!in_mklev)
        newsym(mtmp->mx, mtmp->my); /* make sure the mon shows up */

    return(mtmp);
}

/* set up a new monster's initial level and hit points;
   used by newcham() as well as by makemon() */
void
newmonhp(mtmp, mndx)
struct monst *mtmp;
int mndx;
{
    struct permonst *ptr = &mons[mndx];

    mtmp->m_lev = adj_lev(ptr);
    if (is_golem(ptr)) {
        mtmp->mhpmax = mtmp->mhp = golemhp(mndx);
    } else if (is_rider(ptr)) {
        /* We want low HP, but a high mlevel so they can attack well
         *
         * DSR 10/31/09: What, are you nuts?  They're way too crunchy. */
        mtmp->mhpmax = mtmp->mhp = 100 + d(8, 8);
    } else if (ptr->mlevel > 49) {
        /* "special" fixed hp monster
         * the hit points are encoded in the mlevel in a somewhat strange
         * way to fit in the 50..127 positive range of a signed character
         * above the 1..49 that indicate "normal" monster levels */
        mtmp->mhpmax = mtmp->mhp = 2*(ptr->mlevel - 6);
        mtmp->m_lev = mtmp->mhp / 4; /* approximation */
    } else if (ptr->mlet == S_DRAGON && mndx >= PM_GRAY_DRAGON && In_endgame(&u.uz)) {
        /* dragons in the endgame are always at least average HP
         * note modified hit die here as well; they're MZ_GIGANTIC */
        mtmp->mhpmax = mtmp->mhp = 7 * mtmp->m_lev + d((int)mtmp->m_lev, 8);
    } else if (!mtmp->m_lev) {
        mtmp->mhpmax = mtmp->mhp = rnd(4); /* level 0 monsters are pathetic */
    } else if (ptr->msound == MS_LEADER) {
        /* Quest Leaders need to be fairly burly */
        mtmp->mhpmax = mtmp->mhp = 135 + rnd(30);
    } else {
        int mhitdie;
        /* plain old ordinary monsters; modify hit die based on size;
         * big-ass critters like mastodons should have big-ass HP, and
         * small things like bees and locusts should get less
         */
        switch (mtmp->data->msize) {
            case MZ_TINY:     mhitdie = 4; break;
            case MZ_SMALL:    mhitdie = 6; break;
            case MZ_LARGE:    mhitdie = 10; break;
            case MZ_HUGE:     mhitdie = 12; break;
            case MZ_GIGANTIC: mhitdie = 15; break;
            case MZ_MEDIUM:
            default:          mhitdie = 8;
        }
        mtmp->mhpmax = mtmp->mhp = d((int)mtmp->m_lev, mhitdie);
        if (is_home_elemental(ptr)) {
            mtmp->mhpmax = (mtmp->mhp *= 3);
        }
    }
}

struct mextra *
newmextra()
{
    struct mextra *mextra;

    mextra = (struct mextra *) alloc(sizeof(struct mextra));
    mextra->mname = 0;
    mextra->egd = 0;
    mextra->epri = 0;
    mextra->eshk = 0;
    mextra->emin = 0;
    mextra->edog = 0;
    mextra->mcorpsenm = NON_PM;
    return mextra;
}

/*
 * called with [x,y] = coordinates;
 *  [0,0] means anyplace
 *  [u.ux,u.uy] means: near player (if !in_mklev)
 *
 *  In case we make a monster group, only return the one at [x,y].
 */
struct monst *
makemon(ptr, x, y, mmflags)
register struct permonst *ptr;
register int x, y;
register int mmflags;
{
    use_mon_rng++;
    struct monst *mtmp = _makemon(ptr, x, y, mmflags);
    use_mon_rng--;
    return mtmp;
}

int
mbirth_limit(mndx)
int mndx;
{
    /* assert(MAXMONNO < 255); */
    return (mndx == PM_NAZGUL ? 9 : mndx == PM_ERINYS ? 3 :
            mndx == PM_WEEPING_ARCHANGEL ? 7 : MAXMONNO);
}

/* used for wand/scroll/spell of create monster */
/* returns TRUE iff you know monsters have been created */
boolean
create_critters(cnt, mptr)
int cnt;
struct permonst *mptr;  /* usually null; used for confused reading */
{
    coord c;
    int x, y;
    struct monst *mon;
    boolean known = FALSE;
#ifdef WIZARD
    boolean ask = wizard;
#endif

    while (cnt--) {
#ifdef WIZARD
        if (ask) {
            if (create_particular()) {
                known = TRUE;
                continue;
            }
            else ask = FALSE; /* ESC will shut off prompting */
        }
#endif
        x = u.ux,  y = u.uy;
        /* if in water, try to encourage an aquatic monster
           by finding and then specifying another wet location */
        if (!mptr && u.uinwater && enexto(&c, x, y, &mons[PM_GIANT_EEL]))
            x = c.x,  y = c.y;

        mon = makemon(mptr, x, y, NO_MM_FLAGS);
        if (mon && canspotmon(mon)) known = TRUE;
    }
    return known;
}

STATIC_OVL boolean
uncommon(mndx)
int mndx;
{
    if (mons[mndx].geno & (G_NOGEN | G_UNIQ)) return TRUE;
    if (mvitals[mndx].mvflags & G_GONE) return TRUE;
    if (Inhell && !Insheol) {
        return(mons[mndx].maligntyp > A_NEUTRAL);
    } else if (!((Insheol) && ((mons[mndx].geno & G_SHEOL) != 0))) {
        return((mons[mndx].geno & G_HELL) != 0);
    } else {
        return FALSE; /* G_SHEOL|G_HELL monster in Sheol */
    }
}

boolean
prohibited_by_generation_flags(struct permonst *ptr)
{
    if (Inhell && !Insheol) {
        /* In Gehennon, outside of the Sheol */
        if (ptr->geno & G_HELL) {
            return FALSE;
        }
        if (ptr->geno & G_NOHELL) {
            return TRUE;
        }
        if (ptr->geno & G_SHEOL) {
            return TRUE;
        }
        return FALSE;
    } else if (Insheol) {
        /* In Sheol */
        if (ptr->geno & G_SHEOL) {
            return FALSE;
        }
        if (ptr->geno & G_NOSHEOL) {
            return TRUE;
        }
        if (ptr->geno & G_HELL) {
            return TRUE;
        }
        return FALSE;
    } else {
        /* Outside of Gehennon and Sheol*/
        if (ptr->geno & G_SHEOL) {
            return TRUE;
        }
        if (ptr->geno & G_HELL) {
            return TRUE;
        }
        return FALSE;
    }
}

/*
 *  shift the probability of a monster's generation by
 *  comparing the dungeon alignment and monster alignment.
 *  return an integer in the range of 0-5.
 */
STATIC_OVL int
align_shift(ptr)
register struct permonst *ptr;
{
    static NEARDATA long oldmoves = 0L; /* != 1, starting value of moves */
    static NEARDATA s_level *lev;
    register int alshift;

    if (oldmoves != moves) {
        lev = Is_special(&u.uz);
        oldmoves = moves;
    }
    switch((lev) ? lev->flags.align : dungeons[u.uz.dnum].flags.align) {
        default: /* just in case */
        case AM_NONE:   alshift = 0;
                        break;
        case AM_LAWFUL: alshift = (ptr->maligntyp+20)/(2*ALIGNWEIGHT);
                        break;
        case AM_NEUTRAL:    alshift = (20 - abs(ptr->maligntyp))/ALIGNWEIGHT;
                            break;
        case AM_CHAOTIC:    alshift = (-(ptr->maligntyp-20))/(2*ALIGNWEIGHT);
                            break;
    }
    return alshift;
}

/** Returns the level of the weakest monster to make. */
int
min_monster_difficulty()
{
    int zlevel = level_difficulty();
    if (u.uevent.udemigod) {
        /* all hell breaks loose */
        return zlevel / 4;
    } else {
        return zlevel / 6;
    }
}

/** Returns the level of the strongest monster to make. */
int
max_monster_difficulty()
{
    int zlevel = level_difficulty();
    if (u.uevent.udemigod) {
        /* all hell breaks loose */
        return monstr[PM_DEMOGORGON];
    } else {
        return (zlevel + u.ulevel) / 2;
    }
}


struct permonst *
get_override_mon(override)
struct mon_gen_override *override;
{
    int chance, try = 100;
    struct mon_gen_tuple *mt;
    if (!override) return NULL;

    chance = rnd(override->total_mon_freq);
    do {
        mt = override->gen_chances;
        while (mt && ((chance -= mt->freq) > 0)) mt = mt->next;
        if (mt && (chance <= 0)) {
            if (mt->is_sym) {
                return (mkclass(mt->monid, 0));
            } else {
                if (!(mvitals[mt->monid].mvflags & G_GENOD))
                    return (&mons[mt->monid]);
            }
        }
    } while (--try > 0);
    return NULL;
}

static NEARDATA struct {
    int choice_count;
    char mchoices[SPECIAL_PM]; /* value range is 0..127 */
} rndmonst_state = { -1, {0} };

static
struct permonst *
_rndmonst()
{
    register struct permonst *ptr;
    register int mndx, ct;

    if (level.mon_gen &&
            (rn2(100) < level.mon_gen->override_chance) &&
            ((ptr = get_override_mon(level.mon_gen)) != 0)) {
        return ptr;
    }

    if (rndmonst_state.choice_count < 0) { /* need to recalculate */
        int minmlev, maxmlev;
        boolean elemlevel;
#ifdef REINCARNATION
        boolean upper;
#endif

        rndmonst_state.choice_count = 0;
        /* look for first common monster */
        for (mndx = LOW_PM; mndx < SPECIAL_PM; mndx++) {
            if (!uncommon(mndx)) break;
            rndmonst_state.mchoices[mndx] = 0;
        }
        if (mndx == SPECIAL_PM) {
            /* evidently they've all been exterminated */
#ifdef DEBUG
            pline("rndmonst: no common mons!");
#endif
            return (struct permonst *)0;
        } /* else `mndx' now ready for use below */
        minmlev = min_monster_difficulty();
        maxmlev = max_monster_difficulty();
#ifdef REINCARNATION
        upper = Is_rogue_level(&u.uz);
#endif
        elemlevel = In_endgame(&u.uz) && !Is_astralevel(&u.uz);

        /*
         *  Find out how many monsters exist in the range we have selected.
         */
        /* (`mndx' initialized above) */
        for ( ; mndx < SPECIAL_PM; mndx++) {
            ptr = &mons[mndx];
            rndmonst_state.mchoices[mndx] = 0;
            if (tooweak(mndx, minmlev) || toostrong(mndx, maxmlev))
                continue;
#ifdef REINCARNATION
            if (upper && !isupper(def_monsyms[(int)(ptr->mlet)])) continue;
#endif
            if (elemlevel && wrong_elem_type(ptr)) continue;
            if (uncommon(mndx)) continue;
#ifdef BLACKMARKET  /* SWD: pets are not allowed in the black market */
            if (is_domestic(ptr) && Is_blackmarket(&u.uz)) continue;
#endif
            if (prohibited_by_generation_flags(ptr)) continue;
            ct = (int)(ptr->geno & G_FREQ) + align_shift(ptr);
            /* Boost Sheol-Only(tm) monster generation in Sheol. */
            if (Insheol && (ptr->geno & G_SHEOL)) ct *= 2;
            if (ct < 0 || ct > 127)
                panic("rndmonst: bad count [#%d: %d]", mndx, ct);
            rndmonst_state.choice_count += ct;
            rndmonst_state.mchoices[mndx] = (char)ct;
        }
        /*
         *      Possible modification:  if choice_count is "too low",
         *      expand minmlev..maxmlev range and try again.
         */
    } /* choice_count+mchoices[] recalc */

    if (rndmonst_state.choice_count <= 0) {
        /* maybe no common mons left, or all are too weak or too strong */
#ifdef DEBUG
        Norep("rndmonst: choice_count=%d", rndmonst_state.choice_count);
#endif
        return (struct permonst *)0;
    }

    /*
     *  Now, select a monster at random.
     */
    ct = rnd(rndmonst_state.choice_count);
    for (mndx = LOW_PM; mndx < SPECIAL_PM; mndx++)
        if ((ct -= (int)rndmonst_state.mchoices[mndx]) <= 0) break;

    if (mndx == SPECIAL_PM || uncommon(mndx)) { /* shouldn't happen */
        warning("rndmonst: bad `mndx' [#%d]", mndx);
        return (struct permonst *)0;
    }
    return &mons[mndx];
}

/* select a random monster type */
struct permonst *
rndmonst()
{
    use_mon_rng++;
    struct permonst *tmp = _rndmonst();
    use_mon_rng--;
    return tmp;
}

/* called when you change level (experience or dungeon depth) or when
   monster species can no longer be created (genocide or extinction) */
void
reset_rndmonst(mndx)
int mndx; /* particular species that can no longer be created */
{
    /* cached selection info is out of date */
    if (mndx == NON_PM) {
        rndmonst_state.choice_count = -1; /* full recalc needed */
    } else if (mndx < SPECIAL_PM) {
        rndmonst_state.choice_count -= rndmonst_state.mchoices[mndx];
        rndmonst_state.mchoices[mndx] = 0;
    } /* note: safe to ignore extinction of unique monsters */
}

/* decide whether it's ok to generate a candidate monster by mkclass() */
STATIC_OVL boolean
mk_gen_ok(mndx, mvflagsmask, genomask)
int mndx, mvflagsmask, genomask;
{
    struct permonst *ptr = &mons[mndx];

    if (mvitals[mndx].mvflags & mvflagsmask)
        return FALSE;
    if (ptr->geno & genomask)
        return FALSE;
    if (is_placeholder(ptr))
        return FALSE;
#ifdef MAIL
    /* special levels might ask for random demon type; reject this one */
    if (ptr == &mons[PM_MAIL_DAEMON])
        return FALSE;
#endif
    return TRUE;
}

/*  The routine below is used to make one of the multiple types
 *  of a given monster class.  The second parameter specifies a
 *  special casing bit mask to allow the normal genesis
 *  masks to be deactivated.  Returns 0 if no monsters
 *  in that class can be made.
 */
struct permonst *
mkclass(class, spc)
char class;
int spc;
{
    return mkclass_aligned(class, spc, A_NONE);
}

/* mkclass() with alignment restrictions; used by ndemon() */
struct permonst *
mkclass_aligned(class, spc, atyp)
char class;
int spc;
aligntyp atyp;
{
    register int first, last, num = 0;
    int k, nums[SPECIAL_PM + 1]; /* +1: insurance for final return value */
    int maxmlev, mask = (G_NOGEN | G_UNIQ) & ~spc;

    (void) memset((genericptr_t) nums, 0, sizeof nums);
    maxmlev = level_difficulty() >> 1;
    if (class < 1 || class >= MAXMCLASSES) {
        warning("mkclass called with bad class!");
        return NULL;
    }
    /*  Assumption #1:  monsters of a given class are contiguous in the
     *                  mons[] array.  Player monsters and quest denizens
     *                  are an exception; mkclass() won't pick them.
     *                  SPECIAL_PM is long worm tail and separates the
     *                  regular monsters from the exceptions.
     */
    for (first = LOW_PM; first < SPECIAL_PM; first++) {
        if (mons[first].mlet == class) {
            break;
        }
    }
    if (first == SPECIAL_PM) {
        warning("mkclass found no class %d monsters", class);
        return (struct permonst *) 0;
    }

    /*  Assumption #2:  monsters of a given class are presented in ascending
     *                  order of strength.
     */
    for (last = first; last < SPECIAL_PM && mons[last].mlet == class; last++) {
        if (atyp != A_NONE && sgn(mons[last].maligntyp) != sgn(atyp)) {
            continue;
        }
        if (mk_gen_ok(last, G_GONE, mask)) {
            /* consider it; don't reject a toostrong() monster if we
               don't have anything yet (num==0) or if it is the same
               (or lower) difficulty as preceding candidate (non-zero
               'num' implies last > first so mons[last-1] is safe);
               sometimes accept it even if high difficulty */
            if (num && toostrong(last, maxmlev) &&
                    monstr[last] != monstr[last-1] && rn2(2)) break;
            if ((k = (mons[last].geno & G_FREQ)) > 0) {
                /* skew towards lower value monsters at lower exp. levels
                   (this used to be done in the next loop, but that didn't
                   work well when multiple species had the same level and
                   were followed by one that was past the bias threshold;
                   cited example was sucubus and incubus, where the bias
                   against picking the next demon resulted in incubus
                   being picked nearly twice as often as sucubus);
                   we need the '+1' in case the entire set is too high
                   level (really low level hero) */
                nums[last] = k + 1 - (adj_lev(&mons[last]) > (u.ulevel * 2));
                num += nums[last];
            }
        }
    }

    if (!num) {
        return (struct permonst *) 0;
    }

    /* the hard work has already been done; 'num' should hit 0 before
       first reaches last (which is actually one past our last candidate) */
    for (num = rnd(num); first < last; first++) {
        if ((num -= nums[first]) <= 0) {
            break;
        }
    }

    return nums[first] ? &mons[first] : (struct permonst *) 0;
}

/* like mkclass(), but excludes difficulty considerations; used when
   player with polycontrol picks a class instead of a specific type;
   genocided types are avoided but extinct ones are acceptable; we don't
   check polyok() here--caller accepts some choices !polyok() would reject */
int
mkclass_poly(class)
int class;
{
    register int first, last, num = 0;

    for (first = LOW_PM; first < SPECIAL_PM; first++) {
        if (mons[first].mlet == class) {
            break;
        }
    }
    if (first == SPECIAL_PM) {
        return NON_PM;
    }

    for (last = first; last < SPECIAL_PM && mons[last].mlet == class; last++) {
        if (mk_gen_ok(last, G_GENOD, (G_NOGEN | G_UNIQ))) {
            num += mons[last].geno & G_FREQ;
        }
    }
    if (!num) {
        return NON_PM;
    }

    for (num = rnd(num); num > 0; first++) {
        if (mk_gen_ok(first, G_GENOD, (G_NOGEN | G_UNIQ))) {
            num -= mons[first].geno & G_FREQ;
        }
    }
    first--; /* correct an off-by-one error */

    return first;
}

int
adj_lev(ptr) /* adjust strength of monsters based on u.uz and u.ulevel */
register struct permonst *ptr;
{
    int tmp, tmp2;

    if (ptr == &mons[PM_WIZARD_OF_YENDOR]) {
        /* does not depend on other strengths, but does get stronger
         * every time he is killed
         */
        tmp = ptr->mlevel + mvitals[PM_WIZARD_OF_YENDOR].died;
        if (tmp > 49) tmp = 49;
        return tmp;
    }

    if((tmp = ptr->mlevel) > 49) return(50); /* "special" demons/devils */
    tmp2 = (level_difficulty() - tmp);
    if(tmp2 < 0) tmp--; /* if mlevel > u.uz decrement tmp */
    else tmp += (tmp2 / 5); /* else increment 1 per five diff */

    tmp2 = (u.ulevel - ptr->mlevel); /* adjust vs. the player */
    if(tmp2 > 0) tmp += (tmp2 / 4); /* level as well */

    tmp2 = (3 * ((int) ptr->mlevel))/ 2; /* crude upper limit */
    if (tmp2 > 49) tmp2 = 49;   /* hard upper limit */
    return((tmp > tmp2) ? tmp2 : (tmp > 0 ? tmp : 0)); /* 0 lower limit */
}

/* monster earned experience and will gain some hit points; it might also
   grow into a bigger monster (baby to adult, soldier to officer, etc) */
struct permonst *
grow_up(mtmp, victim)
struct monst *mtmp, *victim;
{
    int oldtype, newtype, max_increase, cur_increase,
        lev_limit, hp_threshold;
    struct permonst *ptr = mtmp->data;

    /* monster died after killing enemy but before calling this function */
    /* currently possible if killing a gas spore */
    if (mtmp->mhp <= 0)
        return ((struct permonst *)0);

    /* note:  none of the monsters with special hit point calculations
       have both little and big forms */
    oldtype = monsndx(ptr);
    newtype = little_to_big(oldtype);
    if (newtype == PM_PRIEST && mtmp->female) newtype = PM_PRIESTESS;

    /* growth limits differ depending on method of advancement */
    if (victim) {   /* killed a monster */
        /*
         * The HP threshold is the maximum number of hit points for the
         * current level; once exceeded, a level will be gained.
         * Possible bug: if somehow the hit points are already higher
         * than that, monster will gain a level without any increase in HP.
         */
        hp_threshold = mtmp->m_lev * 8; /* normal limit */
        if (!mtmp->m_lev)
            hp_threshold = 4;
        else if (is_golem(ptr)) /* strange creatures */
            hp_threshold = ((mtmp->mhpmax / 10) + 1) * 10 - 1;
        else if (is_home_elemental(ptr))
            hp_threshold *= 3;
        lev_limit = 3 * (int)ptr->mlevel / 2; /* same as adj_lev() */
        /* If they can grow up, be sure the level is high enough for that */
        if (oldtype != newtype && mons[newtype].mlevel > lev_limit)
            lev_limit = (int)mons[newtype].mlevel;
        /* number of hit points to gain; unlike for the player, we put
           the limit at the bottom of the next level rather than the top */
        max_increase = rnd((int)victim->m_lev + 1);
        if (mtmp->mhpmax + max_increase > hp_threshold + 1)
            max_increase = max((hp_threshold + 1) - mtmp->mhpmax, 0);
        cur_increase = (max_increase > 1) ? rn2(max_increase) : 0;
    } else {
        /* a gain level potion or wraith corpse; always go up a level
           unless already at maximum (49 is hard upper limit except
           for demon lords, who start at 50 and can't go any higher) */
        max_increase = cur_increase = rnd(8);
        hp_threshold = 0; /* smaller than `mhpmax + max_increase' */
        lev_limit = 50; /* recalc below */
    }

    mtmp->mhpmax += max_increase;
    mtmp->mhp += cur_increase;
    if (mtmp->mhpmax <= hp_threshold)
        return ptr; /* doesn't gain a level */

    if (is_mplayer(ptr)) lev_limit = 30; /* same as player */
    else if (lev_limit < 5) lev_limit = 5; /* arbitrary */
    else if (lev_limit > 49) lev_limit = (ptr->mlevel > 49 ? 50 : 49);

    if ((int)++mtmp->m_lev >= mons[newtype].mlevel && newtype != oldtype) {
        ptr = &mons[newtype];
        /* new form might force gender change */
        int fem = is_male(ptr) ? 0 : is_female(ptr) ? 1 : mtmp->female;

        if (mvitals[newtype].mvflags & G_GENOD) { /* allow G_EXTINCT */
            if (canspotmon(mtmp)) {
                pline("As %s grows up into %s, %s %s!", mon_nam(mtmp),
                        an(ptr->mname), mhe(mtmp),
                        nonliving(ptr) ? "expires" : "dies");
            }
            set_mon_data(mtmp, ptr); /* keep mvitals[] accurate */
            mondied(mtmp);
            return (struct permonst *)0;
        } else if (canspotmon(mtmp)) {
            char buf[BUFSZ];

            /* 3.6.1:
             * Temporary (?) hack to fix growing into opposite gender.
             */
            Sprintf(buf, "%s%s",
                    /* deal with female gnome becoming a gnome lord */
                    (mtmp->female && !fem) ? "male "
                        /* or a male gnome becoming a gnome lady
                           (can't happen with 3.6.0 mons[], but perhaps
                           slightly less sexist if prepared for it...) */
                      : (fem && !mtmp->female) ? "female " : "",
                    ptr->mname);
            pline("%s %s %s.", upstart(y_monnam(mtmp)),
                  (fem != mtmp->female) ? "changes into"
                                        : humanoid(ptr) ? "becomes"
                                                        : "grows up into",
                  an(buf));
        }
        set_mon_data(mtmp, ptr);
        newsym(mtmp->mx, mtmp->my); /* color may change */
        lev_limit = (int)mtmp->m_lev; /* never undo increment */

        mtmp->female = fem; /* gender might be changing */
    }
    /* sanity checks */
    if ((int)mtmp->m_lev > lev_limit) {
        mtmp->m_lev--; /* undo increment */
        /* HP might have been allowed to grow when it shouldn't */
        if (mtmp->mhpmax == hp_threshold + 1) mtmp->mhpmax--;
    }
    if (mtmp->mhpmax > 50*8) mtmp->mhpmax = 50*8; /* absolute limit */
    if (mtmp->mhp > mtmp->mhpmax) mtmp->mhp = mtmp->mhpmax;

    return ptr;
}

int
mongets(mtmp, otyp)
struct monst *mtmp;
int otyp;
{
    register struct obj *otmp;
    int spe;

    if (!otyp) return 0;
    otmp = mksobj(otyp, TRUE, FALSE);
    if (otmp) {
        if (mtmp->data->mlet == S_DEMON) {
            /* demons never get blessed objects */
            if (otmp->blessed) curse(otmp);
        } else if (is_lminion(mtmp)) {
            /* lawful minions don't get cursed, bad, or rusting objects */
            otmp->cursed = FALSE;
            if(otmp->spe < 0) otmp->spe = 0;
            otmp->oerodeproof = TRUE;
        } else if (is_mplayer(mtmp->data) && is_sword(otmp)) {
            otmp->spe = (3 + rn2(4));
        }

        if (otmp->otyp == CANDELABRUM_OF_INVOCATION) {
            otmp->spe = 0;
            otmp->age = 0L;
            otmp->lamplit = FALSE;
            otmp->blessed = otmp->cursed = FALSE;
        } else if (otmp->otyp == BELL_OF_OPENING) {
            otmp->blessed = otmp->cursed = FALSE;
        } else if (otmp->otyp == SPE_BOOK_OF_THE_DEAD) {
            otmp->blessed = FALSE;
            otmp->cursed = TRUE;
        }

        /* leaders don't tolerate inferior quality battle gear */
        if (is_prince(mtmp->data)) {
            if (otmp->oclass == WEAPON_CLASS && otmp->spe < 1)
                otmp->spe = 1;
            else if (otmp->oclass == ARMOR_CLASS && otmp->spe < 0)
                otmp->spe = 0;
        }

        spe = otmp->spe;
        (void) mpickobj(mtmp, otmp); /* might free otmp */
        return(spe);
    } else return(0);
}

int
golemhp(type)
int type;
{
    switch(type) {
        case PM_STRAW_GOLEM: return 20;
        case PM_PAPER_GOLEM: return 20;
        case PM_WAX_GOLEM: return 20;
        case PM_ROPE_GOLEM: return 30;
        case PM_LEATHER_GOLEM: return 40;
        case PM_GOLD_GOLEM: return 40;
        case PM_WOOD_GOLEM: return 50;
        case PM_FLESH_GOLEM: return 40;
        case PM_CLAY_GOLEM: return 50;
        case PM_STONE_GOLEM: return 60;
        case PM_GLASS_GOLEM: return 60;
        case PM_IRON_GOLEM: return 80;
        case PM_ICE_GOLEM: return 130;
        case PM_CRYSTAL_ICE_GOLEM: return 160;
        default: return 0;
    }
}

/*
 *  Alignment vs. yours determines monster's attitude to you.
 *  ( some "animal" types are co-aligned, but also hungry )
 */
boolean
peace_minded(ptr)
register struct permonst *ptr;
{
    aligntyp mal = ptr->maligntyp, ual = u.ualign.type;

    if (always_peaceful(ptr)) return TRUE;
    if (always_hostile(ptr)) return FALSE;
    if (ptr->msound == MS_LEADER || ptr->msound == MS_GUARDIAN)
        return TRUE;
    if (ptr->msound == MS_NEMESIS) return FALSE;

    if (race_peaceful(ptr)) return TRUE;
    if (race_hostile(ptr)) return FALSE;

    /* the monster is hostile if its alignment is different from the
     * player's */
    if (sgn(mal) != sgn(ual)) return FALSE;

    /* Negative monster hostile to player with Amulet. */
    if (mal < A_NEUTRAL && u.uhave.amulet) return FALSE;

    /* minions are hostile to players that have strayed at all */
    if (is_minion(ptr)) return((boolean)(u.ualign.record >= 0));

    /* Last case:  a chance of a co-aligned monster being
     * hostile.  This chance is greater if the player has strayed
     * (u.ualign.record negative) or the monster is not strongly aligned.
     */
    return((boolean)(!!rn2(16 + (u.ualign.record < -15 ? -15 : u.ualign.record)) &&
                !!rn2(2 + abs(mal))));
}

/* Set malign to have the proper effect on player alignment if monster is
 * killed.  Negative numbers mean it's bad to kill this monster; positive
 * numbers mean it's good.  Since there are more hostile monsters than
 * peaceful monsters, the penalty for killing a peaceful monster should be
 * greater than the bonus for killing a hostile monster to maintain balance.
 * Rules:
 *   it's bad to kill peaceful monsters, potentially worse to kill always-
 *      peaceful monsters
 *   it's never bad to kill a hostile monster, although it may not be good
 */
void
set_malign(mtmp)
struct monst *mtmp;
{
    schar mal = mtmp->data->maligntyp;
    boolean coaligned;

    if (mtmp->ispriest || mtmp->isminion) {
        /* some monsters have individual alignments; check them */
        if (mtmp->ispriest && EPRI(mtmp)) {
            mal = EPRI(mtmp)->shralign;
        } else if (mtmp->isminion && EMIN(mtmp)) {
            mal = EMIN(mtmp)->min_align;
        }
        /* unless alignment is none, set mal to -5,0,5 */
        /* (see align.h for valid aligntyp values)     */
        if (mal != A_NONE) {
            mal *= 5;
        }
    }

    coaligned = (sgn(mal) == sgn(u.ualign.type));
    if (mtmp->data->msound == MS_LEADER) {
        mtmp->malign = -20;
    } else if (mal == A_NONE) {
        if (mtmp->mpeaceful)
            mtmp->malign = 0;
        else
            mtmp->malign = 20; /* really hostile */
    } else if (always_peaceful(mtmp->data)) {
        int absmal = abs(mal);
        if (mtmp->mpeaceful)
            mtmp->malign = -3*max(5, absmal);
        else
            mtmp->malign = 3*max(5, absmal); /* renegade */
    } else if (always_hostile(mtmp->data)) {
        int absmal = abs(mal);
        if (coaligned)
            mtmp->malign = 0;
        else
            mtmp->malign = max(5, absmal);
    } else if (coaligned) {
        int absmal = abs(mal);
        if (mtmp->mpeaceful)
            mtmp->malign = -3*max(3, absmal);
        else /* renegade */
            mtmp->malign = max(3, absmal);
    } else /* not coaligned and therefore hostile */
        mtmp->malign = abs(mal);
}

/* allocate a new mcorpsenm field for a monster; only need mextra itself */
void
newmcorpsenm(mtmp)
struct monst *mtmp;
{
    if (!mtmp->mextra) {
        mtmp->mextra = newmextra();
    }
    MCORPSENM(mtmp) = NON_PM; /* not initialized yet */
}

/* release monster's mcorpsenm field; basically a no-op */
void
freemcorpsenm(mtmp)
struct monst *mtmp;
{
    if (has_mcorpsenm(mtmp)) {
        MCORPSENM(mtmp) = NON_PM;
    }
}

static NEARDATA char syms[] = {
    MAXOCLASSES,  MAXOCLASSES+1, RING_CLASS,   WAND_CLASS,   WEAPON_CLASS,
    FOOD_CLASS,   COIN_CLASS,    SCROLL_CLASS, POTION_CLASS, ARMOR_CLASS,
    AMULET_CLASS, TOOL_CLASS,    ROCK_CLASS,   GEM_CLASS,    SPBOOK_CLASS,
    S_MIMIC_DEF,  S_MIMIC_DEF,
};

void
set_mimic_sym(mtmp) /* KAA, modified by ERS */
register struct monst *mtmp;
{
    int typ, roomno, rt;
    unsigned appear, ap_type;
    int s_sym;
    struct obj *otmp;
    int mx, my;

    if (!mtmp || Protection_from_shape_changers) {
        return;
    }
    mx = mtmp->mx; my = mtmp->my;
    typ = levl[mx][my].typ;
    /* only valid for INSIDE of room */
    roomno = levl[mx][my].roomno - ROOMOFFSET;
    if (roomno >= 0)
        rt = rooms[roomno].rtype;
#ifdef SPECIALIZATION
    else if (IS_ROOM(typ))
        rt = OROOM,  roomno = 0;
#endif
    else rt = 0; /* roomno < 0 case for GCC_WARN */

    if (OBJ_AT(mx, my)) {
        ap_type = M_AP_OBJECT;
        appear = level.objects[mx][my]->otyp;
    } else if (IS_DOOR(typ) || IS_WALL(typ) ||
            typ == SDOOR || typ == SCORR) {
        ap_type = M_AP_FURNITURE;
        /*
         *  If there is a wall to the left that connects to this
         *  location, then the mimic mimics a horizontal closed door.
         *  This does not allow doors to be in corners of rooms.
         */
        if (mx != 0 &&
                (levl[mx-1][my].typ == HWALL    ||
                 levl[mx-1][my].typ == TLCORNER ||
                 levl[mx-1][my].typ == TRWALL   ||
                 levl[mx-1][my].typ == BLCORNER ||
                 levl[mx-1][my].typ == TDWALL   ||
                 levl[mx-1][my].typ == CROSSWALL||
                 levl[mx-1][my].typ == TUWALL    ))
            appear = S_hcdoor;
        else
            appear = S_vcdoor;

    } else if (level.flags.is_maze_lev && !In_sokoban(&u.uz) && rn2(2)) {
        ap_type = M_AP_OBJECT;
        appear = STATUE;
    } else if (roomno < 0 && !t_at(mx, my)) {
        ap_type = M_AP_OBJECT;
        appear = BOULDER;
    } else if (rt == ZOO || rt == VAULT) {
        ap_type = M_AP_OBJECT;
        appear = GOLD_PIECE;
    } else if (rt == DELPHI) {
        if (rn2(2)) {
            ap_type = M_AP_OBJECT;
            appear = STATUE;
        } else {
            ap_type = M_AP_FURNITURE;
            appear = S_fountain;
        }
    } else if (rt == TEMPLE) {
        ap_type = M_AP_FURNITURE;
        appear = S_altar;
        /*
         * We won't bother with beehives, morgues, barracks, throne rooms
         * since they shouldn't contain too many mimics anyway...
         */
    } else if (rt >= SHOPBASE) {
        s_sym = get_shop_item(rt - SHOPBASE);
        if (s_sym < 0) {
            ap_type = M_AP_OBJECT;
            appear = -s_sym;
        } else {
            if (s_sym == RANDOM_CLASS)
                s_sym = syms[rn2((int)sizeof(syms)-2) + 2];
            goto assign_sym;
        }
    } else {
        s_sym = syms[rn2((int)sizeof(syms))];
assign_sym:
        if (s_sym == MAXOCLASSES || s_sym == MAXOCLASSES + 1) {
            ap_type = M_AP_FURNITURE;
            appear = s_sym == MAXOCLASSES ? S_upstair : S_dnstair;
        } else {
            ap_type = M_AP_OBJECT;
            if (s_sym == S_MIMIC_DEF) {
                appear = STRANGE_OBJECT;
            } else if (s_sym == COIN_CLASS) {
                appear = GOLD_PIECE;
            } else {
                otmp = mkobj( (char) s_sym, FALSE );
                appear = otmp->otyp;
                /* make sure container contents are free'ed */
                obfree(otmp, (struct obj *) 0);
            }
        }
    }
    mtmp->m_ap_type = ap_type;
    mtmp->mappearance = appear;

    /* when appearing as an object based on a monster type, pick a shape */
    if (ap_type == M_AP_OBJECT &&
         (appear == STATUE || appear == FIGURINE || appear == CORPSE || appear == EGG || appear == TIN)) {
        int mndx = rndmonnum();
        int nocorpse_ndx = (mvitals[mndx].mvflags & G_NOCORPSE);

        if (appear == CORPSE && nocorpse_ndx) {
            mndx = rn1(PM_WIZARD - PM_ARCHEOLOGIST + 1, PM_ARCHEOLOGIST);
        } else if ((appear == EGG && !can_be_hatched(mndx)) || (appear == TIN && nocorpse_ndx)) {
            mndx = NON_PM; /* revert to generic egg or empty tin */
        }

        newmcorpsenm(mtmp);
        MCORPSENM(mtmp) = mndx;
    } else if (ap_type == M_AP_OBJECT && appear == SLIME_MOLD) {
        newmcorpsenm(mtmp);
        MCORPSENM(mtmp) = current_fruit;
        /* if no objects of this fruit type have been created yet,
           context.current_fruit is available for re-use when the player
           assigns a new fruit name; override that--having a mimic as the
           current_fruit is equivalent to creating an instance of that
           fruit (no-op if a fruit of this type has actually been made) */
        flags.made_fruit = TRUE;
    } else if (ap_type == M_AP_FURNITURE && appear == S_altar) {
        int algn = rn2(3) - 1; /* -1 (A_Cha) or 0 (A_Neu) or +1 (A_Law) */

        newmcorpsenm(mtmp);
        MCORPSENM(mtmp) = (Inhell && rn2(3)) ? AM_NONE : Align2amask(algn);
    } else if (has_mcorpsenm(mtmp)) {
        /* don't retain stale value from a previously mimicked shape */
        MCORPSENM(mtmp) = NON_PM;
    }

    if (does_block(mx, my, &levl[mx][my])) {
        block_point(mx, my);
    }
}

/* Release a monster from a bag of tricks or
 * produce an interesting effect ... nda 5/13/2003 */
int
bagotricks(bag)
struct obj *bag;
{
    if (!bag || bag->otyp != BAG_OF_TRICKS) {
        warning("bad bag o' tricks");
    } else if (bag->spe < 1) {
        return use_container(&bag, 1, FALSE);
    } else {

        boolean gotone = TRUE;
        int cnt;
        struct monst *mtmp;
        struct obj *otmp = NULL;

        consume_obj_charge(bag, TRUE);

        switch(rn2(40)) {
            case 0:
            case 1:
                if (bag->recharged==0 && !bag->cursed) {
                    for (cnt=3; cnt>0 && (otmp = mkobj(RANDOM_CLASS, FALSE)); cnt--) {
                        if (otmp->owt<100 && !objects[otmp->otyp].oc_big)
                            break;
                        obj_extract_self(otmp);
                        obfree(otmp, (struct obj *)0);
                        otmp = (struct obj*)0;
                    }
                    if (!otmp) {
                        pline_The("bag coughs nervously.");
                        break;
                    }
                } else {
                    otmp = mksobj(IRON_CHAIN, FALSE, FALSE);
                }
                pline("%s spits %s out.", The(xname(bag)), something);
                otmp = hold_another_object(otmp, "It slips away from you.", (char*)0, (char*)0);
                break;
            case 2:
                pline_The("bag wriggles away from you!");
                dropx(bag);
                break;
            case 3:
                nomul(-1*(rnd(4)), "sucked by a bag");
                if (Hallucination) {
                    You("start climbing into the bag.");
                    nomovemsg = "You give up your attempt to climb into the bag.";
                } else {
                    pline("%s tries to pull you into the bag!", Something);
                    nomovemsg = "You manage to free yourself.";
                }
                break;
            case 4:
                if (Blind)
                    You_hear("a loud eructation.");
                else
                    pline_The("bag belches out %s.",
                            Hallucination ? "the alphabet" : "a noxious cloud");
                (void)create_gas_cloud(u.ux, u.uy, 2, 8, rn1(3, 2));
                break;
            case 5:
                if (Blind) {
                    if (breathless(youmonst.data))
                        You_feel("a puff of air.");
                    else
                        You("smell a musty odor.");
                } else {
                    pline_The("bag exhales a puff of spores.");
                }
                if (!breathless(youmonst.data))
                    (void) make_hallucinated(HHallucination + rn1(35, 10), TRUE, 0L);
                break;
            case 6:
                pline_The("bag yells \"%s\".", Hallucination ? "!ooB" : "Boo!");
                for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                    if (DEADMONSTER(mtmp)) continue;
                    if (cansee(mtmp->mx, mtmp->my)) {
                        if (!resist(mtmp, bag->oclass, 0, NOTELL))
                            monflee(mtmp, 0, FALSE, FALSE);
                    }
                }
                if ((ACURR(A_WIS)<rnd(20) && !bag->blessed) || bag->cursed) {
                    You("are startled into immobility.");
                    nomul(-1*rnd(3), "startled by a bag");
                    nomovemsg = "You regain your composure.";
                }
                break;
            case 7:
                pline_The("bag develops a huge set of %s you!",
                        Hallucination ? "lips and kisses" : "teeth and bites");
                cnt = rnd(10);
                if (Half_physical_damage) cnt = (cnt+1) / 2;
                losehp(cnt, Hallucination ? "amorous bag" : "carnivorous bag", KILLED_BY_AN);
                break;
            case 8:
                if (uwep || uswapwep) {
                    otmp = rn2(2) ? uwep : uswapwep;
                    if (!otmp) otmp = uwep ? uwep : uswapwep;
                    if (Blind)
                        pline("%s grabs %s away from you.", Something, yname(otmp));
                    else
                        pline_The("bag sprouts a tongue and flicks %s %s.",
                                yname(otmp),
                                (Is_airlevel(&u.uz) ||
                                 Is_waterlevel(&u.uz) ||
                                 levl[u.ux][u.uy].typ < IRONBARS ||
                                 levl[u.ux][u.uy].typ >= ICE) ?
                                "away from you" : "to the floor");
                    dropx(otmp);
                } else {
                    pline("%s licks your %s.",
                            Blind ? Something : "The bag sprouts a tongue and",
                            body_part(HAND));
                }
                break;
            default:
                cnt = 1;
                gotone = FALSE;
                if (!rn2(23)) cnt += rn1(7, 1);
                while (cnt-- > 0) {
                    if (makemon((struct permonst *)0, u.ux, u.uy, NO_MM_FLAGS))
                        gotone = TRUE;
                }
        }
        if (gotone) makeknown(BAG_OF_TRICKS);
    }
    return 1;
}

/** May create a camera demon emerging from camera around position x,y. */
void
create_camera_demon(camera, x, y)
struct obj *camera;
int x, y;
{
    struct monst *mtmp;

    if (!rn2(3) &&
            (mtmp = makemon(&mons[PM_HOMUNCULUS], x, y, NO_MM_FLAGS)) != 0) {
        pline("%s is released!", !canspotmon(mtmp) ?
                Something : Hallucination ?
                An(rndmonnam()) : "The picture-painting demon");
        mtmp->mpeaceful = !camera->cursed;
        set_malign(mtmp);
    }
}

/*makemon.c*/
