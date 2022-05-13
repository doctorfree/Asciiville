/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef OBJ_H
#define OBJ_H

/* #define obj obj_nh */ /* uncomment for SCO UNIX, which has a conflicting
 * typedef for "obj" in <sys/types.h> */

union vptrs {
    struct obj *v_nexthere;     /* floor location lists */
    struct obj *v_ocontainer;       /* point back to container */
    struct monst *v_ocarry;     /* point back to carrying monst */
};

/****
 *** oextra -- collection of all object extensions
 ** (see the note at the bottom of this file before adding oextra fields)
 */
struct oextra {
    char *oname;          /* ptr to name of object */
    struct monst *omonst; /* ptr to attached monst struct */
    unsigned *omid;       /* ptr to m_id */
    long *olong;          /* ptr to misc long (temporary gold object) */
    char *omailcmd;       /* response_cmd for mail deliver */
};

struct obj {
    struct obj *nobj;
    union vptrs v;
#define nexthere    v.v_nexthere
#define ocontainer  v.v_ocontainer
#define ocarry      v.v_ocarry

    struct obj *cobj;   /* contents list for containers */
    size_t o_id;
    xchar ox, oy;
    short otyp;     /* object class number */
    unsigned owt;
    long quan;      /* number of items */

    schar spe;        /* quality of weapon, weptool, armor or ring (+ or -);
                         number of charges for wand or charged tool ( >= -1 );
                         number of candles attached to candelabrum;
                         marks your eggs, tin variety and spinach tins;
                         Schroedinger's Box (1) or royal coffers for a court (2);
                         tells which fruit a fruit is;
                         special for uball and amulet;
                         scroll of mail (normal==0, bones or wishing==1, written==2);
                         historic and gender for statues */
#define STATUE_HISTORIC 0x01
#define STATUE_MALE     0x02
#define STATUE_FEMALE   0x04
#define MAIL_MAIL 0
#define MAIL_JUNK 1
#define MAIL_HINT 2
    char oclass;        /* object class */
    char invlet;        /* designation in inventory */
    int oartifact;      /* artifact array index */
    int mrevived;       /* keep track of revivification from former monster */
    xchar where;        /* where the object thinks it is */
#define OBJ_FREE       0 /* object not attached to anything */
#define OBJ_FLOOR      1 /* object on floor */
#define OBJ_CONTAINED  2 /* object in a container */
#define OBJ_INVENT     3 /* object in the hero's inventory */
#define OBJ_MINVENT    4 /* object in a monster inventory */
#define OBJ_MIGRATING  5 /* object sent off to another level */
#define OBJ_BURIED     6 /* object buried */
#define OBJ_ONBILL     7 /* object on shk bill */
#define NOBJ_STATES    8
    xchar timed;        /* # of fuses (timers) attached to this obj */

    Bitfield(cursed, 1);
    Bitfield(blessed, 1);
    Bitfield(unpaid, 1);      /* on some bill */
    Bitfield(no_charge, 1);   /* if shk shouldn't charge for this */
    Bitfield(known, 1);       /* exact nature known */
    Bitfield(dknown, 1);      /* color or text known */
    Bitfield(bknown, 1);      /* blessing or curse known */
    Bitfield(rknown, 1);      /* rustproof or not known */

    Bitfield(oeroded, 2);     /* rusted/burnt weapon/armor */
    Bitfield(oeroded2, 2);    /* corroded/rotted weapon/armor */
#define greatest_erosion(otmp) (int)((otmp)->oeroded > (otmp)->oeroded2 ? (otmp)->oeroded : (otmp)->oeroded2)
#define MAX_ERODE 3
#define orotten oeroded       /* rotten food */
#define odiluted oeroded      /* diluted potions */
#define norevive oeroded2
    Bitfield(oerodeproof, 1); /* erodeproof weapon/armor */
    Bitfield(olocked, 1);     /* object is locked */
#define sokoprize olocked     /* special flag for sokoban prize */
#define orecursive olocked    /* special flag for preventing recursive calls */
    Bitfield(obroken, 1);     /* lock has been broken */
#define degraded_horn obroken /* unicorn horn will poly to non-magic */
    Bitfield(otrapped, 1);    /* container is trapped */
    /* or accidental tripped rolling boulder trap */
#define opoisoned otrapped    /* object (weapon) is coated with poison */

    Bitfield(recharged, 3);   /* number of times it's been recharged */
#define on_ice recharged      /* corpse on ice */
    Bitfield(lamplit, 1);     /* a light-source -- can be lit */
    Bitfield(globby, 1);      /* combines with like types on adjacent squares */
    Bitfield(greased, 1);     /* covered with grease */
    Bitfield(nomerge, 1);     /* set temporarily to prevent merging */

    Bitfield(in_use, 1);      /* for magic items before useup items */
    Bitfield(bypass, 1);      /* mark this as an object to be skipped by bhito() */
    Bitfield(cknown, 1);      /* contents of container assumed to be known */
    Bitfield(lknown, 1);      /* locked/unlocked status is known */
    Bitfield(was_thrown, 1);  /* thrown by the hero since last picked up */
    Bitfield(odrained, 1);    /* drained corpse */
    Bitfield(was_dropped, 1); /* dropped deliberately by the hero */
    Bitfield(was_in_starting_inventory, 1); /* hero started with this item */

    int corpsenm;           /* type of corpse is mons[corpsenm] */
#define leashmon  corpsenm  /* gets m_id of attached pet */
#define spestudied corpsenm /* # of times a spellbook has been studied */
#define fromsink  corpsenm  /* a potion from a sink */

#ifdef RECORD_ACHIEVE
    xchar record_achieve_special;
#endif
    int usecount;           /* overloaded for various things that tally */
    unsigned oeaten;        /* nutrition left in food, if partly eaten */
    long age;               /* creation date */
    long owornmask;
    long picked_up_turn;    /* keep track of when obj got picked up */
    long quiver_priority;   /* priority for automatic quivering of objects */
    struct oextra *oextra;  /* pointer to oextra struct */
};

#define newobj() (struct obj *) alloc(sizeof(struct obj))

/***
 ** oextra referencing and testing macros
 */

#define ONAME(o) ((o)->oextra->oname)
#define OMID(o) ((o)->oextra->omid)
#define OMONST(o) ((o)->oextra->omonst)
#define OLONG(o) ((o)->oextra->olong)
#define OMAILCMD(o) ((o)->oextra->omailcmd)

#define has_oname(o) ((o)->oextra && ONAME(o))
#define has_omid(o) ((o)->oextra && OMID(o))
#define has_omonst(o) ((o)->oextra && OMONST(o))
#define has_olong(o) ((o)->oextra && OLONG(o))
#define has_omailcmd(o) ((o)->oextra && OMAILCMD(o))

/* Weapons and weapon-tools */
/* KMH -- now based on skill categories.  Formerly:
 *  #define is_sword(otmp)  (otmp->oclass == WEAPON_CLASS && \
 *           objects[otmp->otyp].oc_wepcat == WEP_SWORD)
 *  #define is_blade(otmp)  (otmp->oclass == WEAPON_CLASS && \
 *           (objects[otmp->otyp].oc_wepcat == WEP_BLADE || \
 *            objects[otmp->otyp].oc_wepcat == WEP_SWORD))
 *  #define is_weptool(o)   ((o)->oclass == TOOL_CLASS && \
 *           objects[(o)->otyp].oc_weptool)
 *  #define is_multigen(otyp) (otyp <= SHURIKEN)
 *  #define is_poisonable(otyp) (otyp <= BEC_DE_CORBIN)
 */
#define is_blade(otmp)  (otmp->oclass == WEAPON_CLASS && \
                         objects[otmp->otyp].oc_skill >= P_DAGGER && \
                         objects[otmp->otyp].oc_skill <= P_SABER)
#define is_axe(otmp)    ((otmp->oclass == WEAPON_CLASS || \
                          otmp->oclass == TOOL_CLASS) && \
                         objects[otmp->otyp].oc_skill == P_AXE)
#define is_pick(otmp)   ((otmp->oclass == WEAPON_CLASS || \
                          otmp->oclass == TOOL_CLASS) && \
                         objects[otmp->otyp].oc_skill == P_PICK_AXE)
#define is_crystal_pick(otmp) ((otmp)->otyp == CRYSTAL_PICK)
#define is_sword(otmp)  (otmp->oclass == WEAPON_CLASS && \
                         objects[otmp->otyp].oc_skill >= P_SHORT_SWORD && \
                         objects[otmp->otyp].oc_skill <= P_SABER)
#define is_pole(otmp)   ((otmp->oclass == WEAPON_CLASS || \
                          otmp->oclass == TOOL_CLASS) && \
                         (objects[otmp->otyp].oc_skill == P_POLEARMS || \
                          objects[otmp->otyp].oc_skill == P_LANCE))
#define is_spear(otmp)  (otmp->oclass == WEAPON_CLASS && \
                         objects[otmp->otyp].oc_skill >= P_SPEAR && \
                         objects[otmp->otyp].oc_skill <= P_JAVELIN)
#define is_launcher(otmp)   (otmp->oclass == WEAPON_CLASS && \
                             objects[otmp->otyp].oc_skill >= P_BOW && \
                             objects[otmp->otyp].oc_skill <= P_CROSSBOW)
#define is_ammo(otmp)   ((otmp->oclass == WEAPON_CLASS || \
                          otmp->oclass == GEM_CLASS) && \
                         objects[otmp->otyp].oc_skill >= -P_CROSSBOW && \
                         objects[otmp->otyp].oc_skill <= -P_BOW)
#define matching_launcher(a, l) \
    ((l) && objects[(a)->otyp].oc_skill == -objects[(l)->otyp].oc_skill)
#define ammo_and_launcher(a, l) (is_ammo(a) && matching_launcher(a, l))
#define is_missile(otmp)    ((otmp->oclass == WEAPON_CLASS || \
                              otmp->oclass == TOOL_CLASS) && \
                             objects[otmp->otyp].oc_skill >= -P_BOOMERANG && \
                             objects[otmp->otyp].oc_skill <= -P_DART)
#define is_weptool(o)   ((o)->oclass == TOOL_CLASS && \
                         objects[(o)->otyp].oc_skill != P_NONE)
        /* towel is not a weptool:  spe isn't an enchantment, cursed towel
           doesn't weld to hand, and twoweapon won't work with one */
#define is_wet_towel(o) ((o)->otyp == TOWEL && (o)->spe > 0)
#define bimanual(otmp)  ((otmp->oclass == WEAPON_CLASS || \
                          otmp->oclass == TOOL_CLASS) && \
                         objects[otmp->otyp].oc_bimanual)
#define is_multigen(otmp)   (otmp->oclass == WEAPON_CLASS && \
                             objects[otmp->otyp].oc_skill >= -P_SHURIKEN && \
                             objects[otmp->otyp].oc_skill <= -P_BOW)
#define is_poisonable(otmp) (otmp->oclass == WEAPON_CLASS && \
                             objects[otmp->otyp].oc_skill >= -P_SHURIKEN && \
                             objects[otmp->otyp].oc_skill <= -P_BOW)
#define uslinging() (uwep && objects[uwep->otyp].oc_skill == P_SLING)

/* 'is_quest_artifact()' only applies to the current role's artifact */
#define any_quest_artifact(o) ((o)->oartifact >= ART_ITLACHIAYAQUE)

/* Armor */
#define is_shield(otmp) (otmp->oclass == ARMOR_CLASS && \
                         objects[otmp->otyp].oc_armcat == ARM_SHIELD)
#define is_helmet(otmp) (otmp->oclass == ARMOR_CLASS && \
                         objects[otmp->otyp].oc_armcat == ARM_HELM)
#define is_boots(otmp)  (otmp->oclass == ARMOR_CLASS && \
                         objects[otmp->otyp].oc_armcat == ARM_BOOTS)
#define is_gloves(otmp) (otmp->oclass == ARMOR_CLASS && \
                         objects[otmp->otyp].oc_armcat == ARM_GLOVES)
#define is_cloak(otmp)  (otmp->oclass == ARMOR_CLASS && \
                         objects[otmp->otyp].oc_armcat == ARM_CLOAK)
#define is_shirt(otmp)  (otmp->oclass == ARMOR_CLASS && \
                         objects[otmp->otyp].oc_armcat == ARM_SHIRT)
#define is_suit(otmp)   (otmp->oclass == ARMOR_CLASS && \
                         objects[otmp->otyp].oc_armcat == ARM_SUIT)

/* http://nethackwiki.com/wiki/Unofficial_conduct#Racial */
#define is_elven_armor(otmp)    ((otmp)->otyp == ELVEN_LEATHER_HELM \
                                 || (otmp)->otyp == ELVEN_MITHRIL_COAT \
                                 || (otmp)->otyp == ELVEN_CLOAK \
                                 || (otmp)->otyp == ELVEN_SHIELD \
                                 || (otmp)->otyp == ELVEN_BOOTS)
#define is_orcish_armor(otmp)   ((otmp)->otyp == ORCISH_HELM \
                                 || (otmp)->otyp == ORCISH_CHAIN_MAIL \
                                 || (otmp)->otyp == ORCISH_RING_MAIL \
                                 || (otmp)->otyp == ORCISH_CLOAK \
                                 || (otmp)->otyp == URUK_HAI_SHIELD \
                                 || (otmp)->otyp == ORCISH_SHIELD)
#define is_dwarvish_armor(otmp) ((otmp)->otyp == DWARVISH_IRON_HELM \
                                 || (otmp)->otyp == DWARVISH_MITHRIL_COAT \
                                 || (otmp)->otyp == DWARVISH_CLOAK \
                                 || (otmp)->otyp == DWARVISH_ROUNDSHIELD)
#define is_gnomish_armor(otmp)  (FALSE)


/* Eggs and other food */
#define MAX_EGG_HATCH_TIME 200  /* longest an egg can remain unhatched */
#define stale_egg(egg)  ((monstermoves - (egg)->age) > (2*MAX_EGG_HATCH_TIME))
#define ofood(o) ((o)->otyp == CORPSE || (o)->otyp == EGG || (o)->otyp == TIN)
#define polyfodder(obj) (ofood(obj) && \
                         pm_to_cham((obj)->corpsenm) != NON_PM)
#define mlevelgain(obj) (ofood(obj) && (obj)->corpsenm == PM_WRAITH)
#define mhealup(obj)    (ofood(obj) && (obj)->corpsenm == PM_NURSE)
#define drainlevel(corpse) (mons[(corpse)->corpsenm].cnutrit*4/5)

/* Containers */
#define carried(o)  ((o)->where == OBJ_INVENT)
#define mcarried(o) ((o)->where == OBJ_MINVENT)
#define Has_contents(o) (/* (Is_container(o) || (o)->otyp == STATUE) && */ \
        (o)->cobj != (struct obj *)0)
#define Is_container(o) ((o)->otyp >= LARGE_BOX && (o)->otyp <= BAG_OF_TRICKS)
#define Is_box(otmp)    (otmp->otyp == LARGE_BOX || otmp->otyp == CHEST || \
                         otmp->otyp == IRON_SAFE)
#define Is_mbag(otmp)   (otmp->otyp == BAG_OF_HOLDING || \
                         (otmp->otyp == BAG_OF_TRICKS && otmp->spe > 0))
#define SchroedingersBox(o) ((o)->otyp == LARGE_BOX && (o)->spe == 1)
/* usually waterproof; random chance to be subjected to leakage if cursed;
   excludes statues, which aren't vulernable to water even when cursed */
#define Is_waterproof_container(o) \
    ((o)->otyp == OILSKIN_SACK || (o)->otyp == ICE_BOX || Is_box(o))

#define Is_sokoprize(otmp)  ((otmp)->sokoprize && !Is_box(otmp))

/* dragon gear */
#define Is_dragon_scales(idx)   (idx >= GRAY_DRAGON_SCALES && \
                                 idx <= CHROMATIC_DRAGON_SCALES)
#define Is_dragon_mail(idx) (idx >= GRAY_DRAGON_SCALE_MAIL && \
                             idx <= CHROMATIC_DRAGON_SCALE_MAIL)
#define Is_dragon_armor(idx)    (Is_dragon_scales(idx) || Is_dragon_mail(idx))
#define Is_glowing_dragon_armor(idx)    (Is_dragon_armor(idx) && \
                                         (OBJ_DESCR(objects[idx])) && \
                                         (!strncmp(OBJ_DESCR(objects[idx]), "glowing ", 5)))
#define Dragon_scales_to_mail(idx) (idx - GRAY_DRAGON_SCALES + GRAY_DRAGON_SCALE_MAIL)
#define Dragon_mail_to_scales(idx) (idx - GRAY_DRAGON_SCALE_MAIL + GRAY_DRAGON_SCALES)
#define Dragon_scales_to_pm(obj) &mons[PM_GRAY_DRAGON + (obj)->otyp \
                                       - GRAY_DRAGON_SCALES]
#define Dragon_mail_to_pm(obj)  &mons[PM_GRAY_DRAGON + (obj)->otyp \
                                      - GRAY_DRAGON_SCALE_MAIL]
#define Dragon_to_scales(pm)    (GRAY_DRAGON_SCALES + \
        (monsndx(pm) >= PM_GRAY_DRAGON ? (monsndx(pm) - PM_GRAY_DRAGON) : \
         (monsndx(pm) - PM_BABY_GRAY_DRAGON)))

/* http://nethackwiki.com/wiki/Unofficial_conduct#Racial */
/* Elven gear */
#define is_elven_weapon(otmp)   ((otmp)->otyp == ELVEN_ARROW \
                                 || (otmp)->otyp == ELVEN_SPEAR \
                                 || (otmp)->otyp == ELVEN_DAGGER \
                                 || (otmp)->otyp == ELVEN_SHORT_SWORD \
                                 || (otmp)->otyp == ELVEN_BROADSWORD \
                                 || (otmp)->otyp == ELVEN_BOW)
#define is_elven_obj(otmp)  (is_elven_armor(otmp) || is_elven_weapon(otmp))

/* Orcish gear */
#define is_orcish_obj(otmp) (is_orcish_armor(otmp) \
                             || (otmp)->otyp == ORCISH_ARROW \
                             || (otmp)->otyp == ORCISH_SPEAR \
                             || (otmp)->otyp == ORCISH_DAGGER \
                             || (otmp)->otyp == ORCISH_SHORT_SWORD \
                             || (otmp)->otyp == ORCISH_BOW)

/* Dwarvish gear */
#define is_dwarvish_obj(otmp)   (is_dwarvish_armor(otmp) \
                                 || (otmp)->otyp == DWARVISH_SPEAR \
                                 || (otmp)->otyp == DWARVISH_SHORT_SWORD \
                                 || (otmp)->otyp == DWARVISH_MATTOCK)

/* Gnomish gear */
#define is_gnomish_obj(otmp)    (is_gnomish_armor(otmp))

/* Light sources */
#define Is_candle(otmp) (otmp->otyp == TALLOW_CANDLE || \
                         otmp->otyp == WAX_CANDLE)
#define MAX_OIL_IN_FLASK 400    /* maximum amount of oil in a potion of oil */

#define MAX_LAMP_FUEL 1500 /* maximum amount of fuel in lamps or lanterns */

/* MAGIC_LAMP intentionally excluded below */
/* age field of this is relative age rather than absolute */
#define age_is_relative(otmp)   ((otmp)->otyp == BRASS_LANTERN \
                                 || (otmp)->otyp == OIL_LAMP \
                                 || (otmp)->otyp == CANDELABRUM_OF_INVOCATION \
                                 || (otmp)->otyp == TALLOW_CANDLE \
                                 || (otmp)->otyp == WAX_CANDLE \
                                 || (otmp)->otyp == POT_OIL)
/* object can be ignited */
#define ignitable(otmp) ((otmp)->otyp == BRASS_LANTERN \
                         || (otmp)->otyp == OIL_LAMP \
                         || (otmp)->otyp == CANDELABRUM_OF_INVOCATION \
                         || (otmp)->otyp == TALLOW_CANDLE \
                         || (otmp)->otyp == WAX_CANDLE \
                         || (otmp)->otyp == POT_OIL)

/* unique items */
#define is_unique(obj)      ((obj)->otyp == AMULET_OF_YENDOR \
                             || (obj)->otyp == BELL_OF_OPENING \
                             || (obj)->otyp == CANDELABRUM_OF_INVOCATION \
                             || (obj)->otyp == SPE_BOOK_OF_THE_DEAD)

/* special stones */
#define is_graystone(obj)   ((obj)->otyp == LUCKSTONE || \
                             (obj)->otyp == LOADSTONE || \
                             (obj)->otyp == FLINT     || \
                             (obj)->otyp == TOUCHSTONE)
#define is_stone(obj)       ((obj)->otyp >= LUCKSTONE && (obj)->otyp <= ROCK)

/* misc */
#ifdef KOPS
#define is_flimsy(otmp)     (objects[(otmp)->otyp].oc_material <= LEATHER || \
                             (otmp)->otyp == RUBBER_HOSE)
#else
#define is_flimsy(otmp)     (objects[(otmp)->otyp].oc_material <= LEATHER)
#endif
#ifdef WEBB_DISINT
/* note: worn amulet of life saving must be preserved in order to operate */
# define oresist_disintegration(obj)                                       \
    (objects[obj->otyp].oc_oprop == DISINT_RES || \
     obj_resists(obj, 5, 50) || \
     is_quest_artifact(obj))
# define weight_dmg(i) {  \
        i = (i<=100) ? 1 : i/100; \
        i = rnd(i); \
        if(i > 6) i = 6; \
        }

#endif
/* helpers, simple enough to be macros */
#define is_plural(o)    ((o)->quan > 1 || \
                         (o)->oartifact == ART_EYES_OF_THE_OVERWORLD)
#define pair_of(o) ((o)->otyp == LENSES || is_gloves(o) || is_boots(o))

/* Flags for get_obj_location(). */
#define CONTAINED_TOO 0x1
#define BURIED_TOO    0x2

/* object erosion types */
#define ERODE_BURN    0
#define ERODE_RUST    1
#define ERODE_ROT     2
#define ERODE_CORRODE 3

/* erosion flags for erode_obj() */
#define EF_NONE    0x0
#define EF_GREASE  0x1 /* check for a greased object */
#define EF_DESTROY 0x2 /* potentially destroy the object */
#define EF_VERBOSE 0x4 /* print extra messages */
#define EF_PAY     0x8 /* it's the player's fault */

/* erosion return values for erode_obj(), water_damage() */
#define ER_NOTHING   0 /* nothing happened */
#define ER_GREASED   1 /* protected by grease */
#define ER_DAMAGED   2 /* object was damaged in some way */
#define ER_DESTROYED 3 /* object was destroyed */

/* propeller method for potionhit() */
#define POTHIT_HERO_BASH   0 /* wielded by hero */
#define POTHIT_HERO_THROW  1 /* thrown by hero */
#define POTHIT_MONST_THROW 2 /* thrown by a monster */
#define POTHIT_OTHER_THROW 3 /* propelled by some other means [scatter()] */

/*
 *  Notes for adding new oextra structures:
 *
 *   1. Add the structure definition and any required macros in an
 *          appropriate header file that precedes this one.
 *   2. Add a pointer to your new struct to oextra struct in this file.
 *   3. Add a referencing macro to this file after the newobj macro above
 *      (see ONAME, OMONST, OMIN, OLONG, or OMAILCMD for examples).
 *   4. Add a testing macro after the set of referencing macros
 *      (see has_oname(), has_omonst(), has_omin(), has_olong(),
 *      has_omailcmd() for examples).
 *   5. Create newXX(otmp) function and possibly free_XX(otmp) function
 *      in an appropriate new or existing source file and add a prototype
 *      for it to include/extern.h.  The majority of these are currently
 *      located in mkobj.c for convenience.
 *
 *      void FDECL(newXX, (struct obj *));
 *          void FDECL(free_XX, (struct obj *));
 *
 *            void
 *            newxx(otmp)
 *            struct obj *otmp;
 *            {
 *                if (!otmp->oextra) otmp->oextra = newoextra();
 *                if (!XX(otmp)) {
 *                    XX(otmp) = (struct XX *)alloc(sizeof(struct xx));
 *                    (void) memset((genericptr_t) XX(otmp),
 *                               0, sizeof(struct xx));
 *                }
 *            }
 *
 *   6. Adjust size_obj() in src/cmd.c appropriately.
 *   7. Adjust dealloc_oextra() in src/mkobj.c to clean up
 *      properly during obj deallocation.
 *   8. Adjust copy_oextra() in src/mkobj.c to make duplicate
 *      copies of your struct or data onto another obj struct.
 *   9. Adjust restobj() in src/restore.c to deal with your
 *      struct or data during a restore.
 *  10. Adjust saveobj() in src/save.c to deal with your
 *      struct or data during a save.
 */

/* This struct holds detailed stats about how an object's damage is
 * constructed.
 * Filled in dmgval_core() and used in the pokedex.
 */
struct damage_info_t {
    int damage_small;
    const char* bonus_small;
    int damage_large;
    const char* bonus_large;

    /* various bonus damage conditions */
    const char* blessed_damage;
    const char* axe_damage;
    const char* silver_damage;
    const char* light_damage;
};

#endif /* OBJ_H */
