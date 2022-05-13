/*  SCCS Id: @(#)tutorial.c 3.4 2009/09/05  */
/*  Copyright 2009, Alex Smith        */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "quest.h"
#include "qtext.h"

/* Display a tutorial message, if it hasn't been displayed before.
   Returns TRUE if a tutorial message is output. */
boolean
check_tutorial_message(msgnum)
int msgnum;
{
    if (!flags.tutorial) return FALSE;
    if (pl_tutorial[msgnum - QT_T_FIRST] > 0) return FALSE;
    pl_tutorial[msgnum - QT_T_FIRST] = 1;
    flush_screen(1);
    com_pager(msgnum);
    return TRUE;
}

/* Displays a tutorial message pertaining to object class oclass, if
   it hasn't been shown already. Returns 1 if the object class is one
   for which tutorial messages exist, regardless of whether the
   message is shown or not. This assumes that the object classes haven't
   been customized too heavily (possibly a custom boulder, and that's it). */
int
check_tutorial_oclass(oclass)
int oclass;
{
    switch(oclass) {
    case WEAPON_CLASS: check_tutorial_message(QT_T_ITEM_WEAPON);   return 1;
    case FOOD_CLASS:   check_tutorial_message(QT_T_ITEM_FOOD);     return 1;
    case GEM_CLASS:    check_tutorial_message(QT_T_ITEM_GEM);      return 1;
    case TOOL_CLASS:   check_tutorial_message(QT_T_ITEM_TOOL);     return 1;
    case AMULET_CLASS: check_tutorial_message(QT_T_ITEM_AMULET);   return 1;
    case POTION_CLASS: check_tutorial_message(QT_T_ITEM_POTION);   return 1;
    case SCROLL_CLASS: check_tutorial_message(QT_T_ITEM_SCROLL);   return 1;
    case SPBOOK_CLASS: check_tutorial_message(QT_T_ITEM_BOOK);     return 1;
    case ARMOR_CLASS:  check_tutorial_message(QT_T_ITEM_ARMOR);    return 1;
    case WAND_CLASS:   check_tutorial_message(QT_T_ITEM_WAND);     return 1;
    case RING_CLASS:   check_tutorial_message(QT_T_ITEM_RING);     return 1;
    case ROCK_CLASS:   check_tutorial_message(QT_T_ITEM_STATUE);   return 1;
    case COIN_CLASS:   check_tutorial_message(QT_T_ITEM_GOLD);     return 1;
    default: return 0; /* venom/ball/chain/mimic don't concern us */
    }
}

/* Displays a tutorial message pertaining to the location at (lx, ly),
   if there is a message and it hasn't been shown already. Returns TRUE
   if a message is shown. */
int
check_tutorial_location(lx, ly, from_farlook)
int lx;
int ly;
boolean from_farlook;
{
    struct rm *l = &(levl[lx][ly]);
    if (!flags.tutorial) return FALSE; /* short-circuit */
    if (glyph_is_trap(l->glyph)) /* seen traps only */
        if (check_tutorial_message(QT_T_TRAP)) return TRUE;
    if (IS_DOOR(l->typ) && l->doormask >= D_ISOPEN)
        if (check_tutorial_message(QT_T_DOORS)) return TRUE;
    if (l->typ == CORR)
        if (check_tutorial_message(QT_T_CORRIDOR)) return TRUE;
    /* A freebie: we give away the location of a secret door or
       corridor, once. This is so that the advice to search will
       always end up coming good, to avoid confusing new players;
       it also deals with the horrific possibility of a player's
       first game having no visible exits from the first room (it
       can happen!) */
    if (l->typ == SCORR || l->typ == SDOOR)
        if (check_tutorial_message(QT_T_SECRETDOOR)) return TRUE;
    if (l->typ == POOL || l->typ == MOAT)
        if (check_tutorial_message(QT_T_POOLORMOAT)) return TRUE;
    if (l->typ == LAVAPOOL)
        if (check_tutorial_message(QT_T_LAVA)) return TRUE;
    if (l->typ == STAIRS) {
        /* In which direction? */
        if ((lx == xupstair   && ly == yupstair) ||
            (lx == xupladder  && ly == yupladder) ||
            (lx == sstairs.sx && ly == sstairs.sy && sstairs.up)) {
            if (u.uz.dlevel > 1) {
                if (check_tutorial_message(QT_T_STAIRS)) return TRUE;
                else if (from_farlook)
                    if (check_tutorial_message(QT_T_L1UPSTAIRS)) return TRUE;
            }
        } else if ((lx == xdnstair   && ly == ydnstair) ||
                   (lx == xdnladder  && ly == ydnladder) ||
                   (lx == sstairs.sx && ly == sstairs.sy && !sstairs.up)) {
            if (check_tutorial_message(QT_T_STAIRS)) return TRUE;
        } else impossible("Stairs go neither up nor down?");
    }
    if (l->typ == FOUNTAIN)
        if (check_tutorial_message(QT_T_FOUNTAIN)) return TRUE;
    if (l->typ == THRONE)
        if (check_tutorial_message(QT_T_THRONE)) return TRUE;
    if (l->typ == SINK)
        if (check_tutorial_message(QT_T_SINK)) return TRUE;
    if (l->typ == GRAVE)
        if (check_tutorial_message(QT_T_GRAVE)) return TRUE;
    if (l->typ == ALTAR)
        if (check_tutorial_message(QT_T_ALTAR)) return TRUE;
    if (IS_DRAWBRIDGE(l->typ))
        if (check_tutorial_message(QT_T_DRAWBRIDGE)) return TRUE;
    return FALSE;
}
/* Display tutorial messages that may result from farlook data. */
void
check_tutorial_farlook(x, y)
int x;
int y;
{
    int glyph = glyph_at(x, y);
    /* Monsters */
    if (!flags.tutorial) return; /* short-circuit */
    if (glyph_is_invisible(glyph)) {
        check_tutorial_message(QT_T_LOOK_INVISIBLE);
        return;
    }
    if (glyph_is_pet(glyph) || glyph_is_ridden_monster(glyph)) {
        check_tutorial_message(QT_T_LOOK_TAME);
        return;
    }
    if (glyph_is_monster(glyph)) {
        if (x == u.ux && y == u.uy) return; /* you aren't hostile */
        if (!MON_AT(x, y)) return; /* to prevent null pointer deref */
        if (m_at(x, y)->mpeaceful)
            check_tutorial_message(QT_T_LOOK_PEACEFUL);
        else if (m_at(x, y)->mtame) /* without showpet on */
            check_tutorial_message(QT_T_LOOK_TAME);
        else
            check_tutorial_message(QT_T_LOOK_HOSTILE);
        return;
    }
    /* Items */
    if (glyph_is_object(glyph)) {
        check_tutorial_oclass(objects[glyph_to_obj(glyph)].oc_class);
        return;
    }
    /* Terrain */
    if (glyph_is_cmap(glyph) || glyph_is_trap(glyph)) {
        check_tutorial_location(x, y, TRUE);
        return;
    }
}

#define CHECK_TUTORIAL_COMMAND_BUFSIZE 100
static int check_tutorial_command_message = 0;
static char check_tutorial_command_buffer[CHECK_TUTORIAL_COMMAND_BUFSIZE];
static int check_tutorial_command_pointer = 0;
static int check_tutorial_command_count = 0;
void
check_tutorial_command(c)
char c;
{
    int i, r;
    char lc;
    boolean travel = TRUE;
    boolean farmove = TRUE;
    boolean repeat = TRUE;
    boolean massunequip = TRUE;
    boolean look_reminder = TRUE;
    int secondwield = 0;
    if (c == 'm') {
        if (u.dx && u.dy) c = 'y'; else c = 'b';
    }
    check_tutorial_command_buffer[check_tutorial_command_pointer] = c;
    i = check_tutorial_command_pointer;
    check_tutorial_command_pointer++;
    check_tutorial_command_pointer %= CHECK_TUTORIAL_COMMAND_BUFSIZE;
    check_tutorial_command_count++;
    check_tutorial_command_message = 0;
    r = 0;
    lc = c;
    do
    {
        c = check_tutorial_command_buffer[i];
        if (lc != c) repeat = FALSE;
        if (c != 'y' && c != 'b' && c != 'G') travel = FALSE;
        if (c != 'y' && c != 'b') farmove = FALSE;
        if (c != 'R' && c != 'T') massunequip = FALSE;
        if (c == ';') look_reminder = FALSE;
        if (c == 'w') secondwield++;
        r++;
        if (r > check_tutorial_command_count) break;
        if (moves > 125 && r > 5 && farmove) {
            if (iflags.num_pad)
                check_tutorial_command_message = QT_T_FARMOVE_NUMPAD;
            else
                check_tutorial_command_message = QT_T_FARMOVE_VIKEYS;
            break;
        }
        if (moves > 125 && r > 30 && travel) {
            check_tutorial_command_message = QT_T_TRAVEL;
            break;
        }
        if (moves > 80 && r > 20 && c == 'b') {
            if (iflags.num_pad)
                check_tutorial_command_message = QT_T_DIAGONALS_NUM;
            else
                check_tutorial_command_message = QT_T_DIAGONALS_VI;
            break;
        }
        if (repeat && r > 5 && c == 's') {
            if (iflags.num_pad)
                check_tutorial_command_message = QT_T_REPEAT_NUMPAD;
            else
                check_tutorial_command_message = QT_T_REPEAT_VIKEYS;
            break;
        }
        if (moves > 45 && r >= 2 && massunequip) {
            check_tutorial_command_message = QT_T_MASSUNEQUIP;
            break;
        }
        if (moves > 45 && r >= 2 && repeat && c == 'd') {
            check_tutorial_command_message = QT_T_MULTIDROP;
            break;
        }
        if (moves > 45 && r >= 2 && repeat && c == 'I') {
            check_tutorial_command_message = QT_T_MASSINVENTORY;
            break;
        }
        if (moves > 45 && secondwield >= 3 && r == 50) {
            check_tutorial_command_message = QT_T_SECONDWIELD;
            break;
        }
        if (r >= 3 && repeat && c == 't') {
            check_tutorial_command_message = QT_T_FIRE;
            break;
        }
        i--;
        if (i == -1) i = CHECK_TUTORIAL_COMMAND_BUFSIZE - 1;
        lc = c;
    }
    while(r < 100);
    if (check_tutorial_command_message == 0 && look_reminder &&
        check_tutorial_command_count >= 100)
        check_tutorial_command_message = QT_T_LOOK_REMINDER;
}

/* Display tutorial messages based on the state of the character. */
void
maybe_tutorial()
{
    /* So that we don't get two different combats if a monster disappears */
    static int time_since_combat = 0;
    /* Don't give messages on the first combat of any reload-from-save, to
       avoid overloading the player with tutorial messages */
    static int firstcombat = 1;
    int old_time_since_combat;
    int i;
    struct monst *mtmp;
    static struct attribs oldattribs = {{0, 0, 0, 0, 0, 0}};
    static int oldac = 300; /* an impossible value */

    if (!flags.tutorial) return; /* short-circuit */

    /* Check to see if any tutorial triggers have occured.
       Stop checking once one message is output. */

    /* Welcome message: show immediately */
    if (check_tutorial_message(QT_T_WELCOME)) return;

    /* Terrain checks; these only occur if at least 3 turns have elapsed,
       to avoid overwhelming the player early on. */
    if (moves > 3) {
        int dx, dy;
        int monsterglyph = -1;
        for (dx = -1; dx <= 1; dx++)
            for (dy = -1; dy <= 1; dy++) {
                if (isok(u.ux+dx, u.uy+dy)) {
                    int lx = u.ux+dx;
                    int ly = u.uy+dy;
                    /* Terrain checks */
                    if (check_tutorial_location(lx, ly, FALSE)) return;
                    /* Some non-terrain checks in the same loop */
                    if (glyph_is_invisible(glyph_at(lx, ly)))
                        if (check_tutorial_message(QT_T_LOOK_INVISIBLE)) return;
                    if (glyph_is_monster(glyph_at(lx, ly))) {
                        if (monsterglyph == glyph_at(lx, ly))
                            if (check_tutorial_message(QT_T_CALLMONSTER)) return;
                        monsterglyph = glyph_at(lx, ly);
                    }
                }
            }
    }
    /* Check to see if we're in combat. */
    ++time_since_combat;
    old_time_since_combat = time_since_combat;
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
        if (!DEADMONSTER(mtmp) && cansee(mtmp->mx, mtmp->my) && !mtmp->mtame)
            time_since_combat = 0;
    /* Ambient messages that only come up during combat, and only one
       message per combat */
    if (!time_since_combat && old_time_since_combat > 5) {
        if (!firstcombat) {
            if (u.uz.dlevel >= 3)
                if (check_tutorial_message(QT_T_ELBERETH)) return;
            for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++) {
                if (spellid(i) == SPE_FORCE_BOLT) {
                    if (check_tutorial_message(QT_T_SPELLS)) return;
                }
            }
            if (check_tutorial_message(QT_T_MELEE)) return;
        } else {
            old_time_since_combat = 0;
            firstcombat = 0;
        }
    }
    /* Events. */
    if (spellid(1)) {
        if ((uarm && is_metallic(uarm)) ||
            (uarms && !Role_if(PM_PRIEST)) || /* priests start with shields */
            (uarmh && is_metallic(uarmh) && uarmh->otyp != HELM_OF_BRILLIANCE) ||
            (uarmg && is_metallic(uarmg)) ||
            (uarmf && is_metallic(uarmf)))
            if (check_tutorial_message(QT_T_CASTER_ARMOR)) return;
    }
    if (uwep && uwep != &zeroobj && weapon_dam_bonus(uwep) < 0 && !u.twoweap)
        if (check_tutorial_message(QT_T_WEAPON_SKILL)) return;
    if (u.ulevel >= 2)
        if (check_tutorial_message(QT_T_LEVELUP)) return;
    if (u.ulevel >= 3)
        if (check_tutorial_message(QT_T_RANKUP)) return;
    {
        int incdec = 0;
        boolean firstturn = oldattribs.a[0] == 0;
        for (i = 0; i < A_MAX; i++) {
            if (u.acurr.a[i] > oldattribs.a[i]) incdec = 1;
            if (u.acurr.a[i] < oldattribs.a[i]) incdec = -1;
            oldattribs.a[i] = u.acurr.a[i];
        }
        if (!firstturn) {
            if (incdec > 0)
                if (check_tutorial_message(QT_T_ABILUP)) return;
            if (incdec < 0)
                if (check_tutorial_message(QT_T_ABILDOWN)) return;
        }
    }
    if (u.uac < oldac && oldac != 300)
        if (check_tutorial_message(QT_T_ACIMPROVED)) return;
    oldac = u.uac;
    if (Confusion || Sick || Blind || Stunned || Hallucination || Slimed)
        if (check_tutorial_message(QT_T_STATUS)) return;
    if (u.uz.dlevel >= 2)
        if (check_tutorial_message(QT_T_DLEVELCHANGE)) return;
    if (u.uz.dlevel >= 10)
        if (check_tutorial_message(QT_T_DGN_OVERVIEW)) return;
    if (u.uhp < u.uhpmax)
        if (check_tutorial_message(QT_T_DAMAGED)) return;
    if (u.uen < u.uenmax)
        if (check_tutorial_message(QT_T_PWUSED)) return;
    if (u.uen < 5 && u.uenmax > 10)
        if (check_tutorial_message(QT_T_PWEMPTY)) return;
    if (u.umonster != u.umonnum)
        if (check_tutorial_message(QT_T_POLYSELF)) return;
    if (u.uexp > 0)
        if (check_tutorial_message(QT_T_GAINEDEXP)) return;
    if (u.uhs >= HUNGRY)
        if (check_tutorial_message(QT_T_HUNGER)) return;
    if (u.uhs <= SATIATED)
        if (check_tutorial_message(QT_T_SATIATION)) return;
    if (can_advance_something())
        if (check_tutorial_message(QT_T_ENHANCE)) return;
    if (u.uswallow)
        if (check_tutorial_message(QT_T_ENGULFED)) return;
    if (near_capacity() > UNENCUMBERED)
        if (check_tutorial_message(QT_T_BURDEN)) return;
    if (in_trouble() > 0 && can_pray(0) && !IS_ALTAR(levl[u.ux][u.uy].typ))
        if (check_tutorial_message(QT_T_MAJORTROUBLE)) return;
    if (inside_shop(u.ux, u.uy))
        if (check_tutorial_message(QT_T_SHOPENTRY)) return;
    if (u.uz.dnum == mines_dnum)
        if (check_tutorial_message(QT_T_MINES)) return;
    if (u.uz.dnum == sokoban_dnum)
        if (check_tutorial_message(QT_T_SOKOBAN)) return;
    if (check_tutorial_command_message == QT_T_FIRE)
        if (check_tutorial_message(QT_T_FIRE)) return;
    /* Item-dependent events. */
    {
        int projectile_groups = 0;
        int launcher_groups = 0;
        struct obj *otmp;
        for (otmp = invent; otmp; otmp = otmp->nobj) {
            if (otmp->bknown && otmp->cursed)
                if (check_tutorial_message(QT_T_EQUIPCURSE)) return;
#ifdef MAIL
            if (otmp->otyp == SCR_MAIL)
                if (check_tutorial_message(QT_T_MAILSCROLL)) return;
#endif
            if (otmp->oartifact)
                if (check_tutorial_message(QT_T_ARTIFACT)) return;
            if (otmp->unpaid)
                if (check_tutorial_message(QT_T_SHOPBUY)) return;
            if (!objects[otmp->otyp].oc_name_known)
                switch (objects[otmp->otyp].oc_class) {
                case POTION_CLASS:
                    if (otmp->otyp == POT_WATER) break;
                /* fall through */
                case SCROLL_CLASS:
                    if (otmp->otyp == SCR_BLANK_PAPER) break;
#ifdef MAIL
                    if (otmp->otyp == SCR_MAIL) break;
#endif
                /* fall through */
                case WAND_CLASS:
                case SPBOOK_CLASS:
                    if (otmp->otyp == SPE_BLANK_PAPER) break;
                    if (otmp->otyp == SPE_BOOK_OF_THE_DEAD) break;
                /* fall through */
                case RING_CLASS:
                case AMULET_CLASS:
                    if (check_tutorial_message(QT_T_RANDAPPEARANCE)) return;
                    break;
                default: break;
                }
            /* Containers; minor spoiler here, in that it doesn't trigger
               off a bag of tricks and a savvy player might notice that, but
               that's not a freebie I'm worried about. */
            switch (otmp->otyp) {
            case SACK:
                /* starting inventory for arcs/rogues, and we don't want to
                   give the message until a second container's listed */
                if(Role_if(PM_ARCHEOLOGIST)) break;
                if(Role_if(PM_ROGUE)) break;
                /* fall through */
            case LARGE_BOX:
            case CHEST:
            case ICE_BOX:
            case OILSKIN_SACK:
            case BAG_OF_HOLDING:
                if (check_tutorial_message(QT_T_ITEM_CONTAINER)) return;
                break;
            default: break;
            }
            /* Requiring a specific item during combat... */
            if (!time_since_combat && old_time_since_combat > 5) {
                switch (otmp->otyp) {
                /* Projectiles. */
                case ARROW:
                case ELVEN_ARROW:
                case ORCISH_ARROW:
                case SILVER_ARROW:
                case YA:
                    projectile_groups |= 0x1;
                    break;
                case CROSSBOW_BOLT:
                    projectile_groups |= 0x2;
                    break;
                case FLINT:
                case ROCK:
                    projectile_groups |= 0x4;
                    break;
                /* Launchers. */
                case BOW:
                case ELVEN_BOW:
                case ORCISH_BOW:
                case YUMI:
                    launcher_groups |= 0x1;
                    break;
                case CROSSBOW:
                    launcher_groups |= 0x2;
                    break;
                case SLING:
                    launcher_groups |= 0x4;
                    break;
                /* Thrown weapons. Don't count our wielded weapon in this. */
                case DART:
                case SHURIKEN:
                case BOOMERANG:
                case SPEAR:
                case ELVEN_SPEAR:
                case ORCISH_SPEAR:
                case DWARVISH_SPEAR:
                case JAVELIN:
                case DAGGER:
                case ELVEN_DAGGER:
                case ORCISH_DAGGER:
                case SILVER_DAGGER:
                    if (otmp == uwep) break;
                    if (check_tutorial_message(QT_T_THROWNWEAPONS)) return;
                    break;
                default: break;
                }
            }
        }
        if (projectile_groups & launcher_groups)
            if (check_tutorial_message(QT_T_PROJECTILES)) return;
    }
    /* Items on the current square. */
    {
        struct obj *otmp;
        for (otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere) {
            switch (otmp->otyp) {
            case LARGE_BOX:
            case CHEST:
            case ICE_BOX:
                if (check_tutorial_message(QT_T_ITEM_CONTAINER)) return;
                break;
            default: break;
            }
        }
    }
    /* Ambient messages that only come up outside combat */
    if (time_since_combat > 5) {
        if (check_tutorial_command_message > 0)
            if (check_tutorial_message(check_tutorial_command_message)) return;
        if (moves >= 10)
            if (check_tutorial_message(QT_T_VIEWTUTORIAL)) return;
        if (moves >= 30)
            if (check_tutorial_message(QT_T_CHECK_ITEMS)) return;
        if (moves >= 60)
            if (check_tutorial_message(QT_T_OBJECTIVE)) return;
        if (moves >= 100)
            if (check_tutorial_message(QT_T_SAVELOAD)) return;
        if (moves >= 150)
            if (check_tutorial_message(QT_T_MESSAGERECALL)) return;
    }
}

/* Redisplay tutorial messages. */
int
tutorial_redisplay()
{
    winid tempwin = create_nhwindow(NHW_MENU);
    anything i;
    int c;
    menu_item *s;
    start_menu(tempwin);
    for (i.a_int = QT_T_FIRST; i.a_int <= QT_T_MAX; i.a_int++) {
        if (pl_tutorial[i.a_int - QT_T_FIRST] > 0) {
            static char namebuf[80];
            const char* name;
            qt_com_firstline(i.a_int, namebuf);
            /* adding 10 to namebuf removes the 'Tutorial: ' at the start */
            name = *namebuf ? namebuf + 10 : "(not found)";
            add_menu(tempwin, NO_GLYPH, MENU_DEFCNT, &i, 0, 0, ATR_NONE, name, FALSE);
        }
    }
    end_menu(tempwin, "Which tutorial?");
    c = select_menu(tempwin, PICK_ONE, &s);
    if (c <= 0) {
        destroy_nhwindow(tempwin);
        return 0;
    }
    i.a_int = s[0].item.a_int;
    free((genericptr_t)s);
    flush_screen(1);
    com_pager(i.a_int);
    destroy_nhwindow(tempwin);
    return 0;
}

/* tutorial.c */
