/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* KMH -- Copied from pray.c; this really belongs in a header file */
#define DEVOUT 14
#define STRIDENT 4

#define Your_Own_Role(mndx) \
    ((mndx) == urole.malenum || \
     (urole.femalenum != NON_PM && (mndx) == urole.femalenum))
#define Your_Own_Race(mndx) \
    ((mndx) == urace.malenum || \
     (urace.femalenum != NON_PM && (mndx) == urace.femalenum))

boolean known;

static NEARDATA const char readable[] =
{ COIN_CLASS, ALL_CLASSES, SCROLL_CLASS, SPBOOK_CLASS, 0 };
static const char all_count[] = { ALLOW_COUNT, ALL_CLASSES, 0 };

static void FDECL(wand_explode, (struct obj *));
#if 0
static void NDECL(do_class_genocide);
#endif
static void FDECL(stripspe, (struct obj *));
static void FDECL(p_glow1, (struct obj *));
static void FDECL(p_glow2, (struct obj *, const char *));
static void FDECL(randomize, (int *, int));
static void FDECL(forget_single_object, (int));
#if 0
static void FDECL(forget, (int));
#endif
static int FDECL(maybe_tame, (struct monst *, struct obj *));

STATIC_PTR void FDECL(do_flood, (int, int, genericptr_t));
STATIC_PTR void FDECL(undo_flood, (int, int, genericptr_t));
STATIC_PTR void FDECL(set_lit, (int, int, genericptr_t));

static boolean
learn_scroll_typ(short scrolltyp, boolean verbose)
{
    if (!objects[scrolltyp].oc_name_known) {
        if (verbose) {
            makeknown_msg(scrolltyp);
        } else {
            makeknown(scrolltyp);
        }
        more_experienced(0, 0, 10);
        return TRUE;
    } else {
        return FALSE;
    }
}

/* also called from teleport.c for scroll of teleportation */
void
learn_scroll(sobj)
struct obj *sobj;
{
    /* it's implied that sobj->dknown is set;
       we couldn't be reading this scroll otherwise */
    if (sobj->oclass != SPBOOK_CLASS) {
        (void) learn_scroll_typ(sobj->otyp, FALSE);
    }
}

static char *
erode_obj_text(otmp, buf)
struct obj *otmp;
char *buf;
{
    int erosion = greatest_erosion(otmp);

    if (erosion) {
        wipeout_text(buf,
                     (strlen(buf) * erosion / (2 * MAX_ERODE)),
                     otmp->o_id ^ (unsigned)game_seed);
    }
    return buf;
}

char *
tshirt_text(tshirt, buf)
struct obj *tshirt;
char *buf;
{
    static const char *shirt_msgs[] = {
        /* Scott Bigham */
        "I explored the Dungeons of Doom and all I got was this lousy T-shirt!",
        "Is that Mjollnir in your pocket or are you just happy to see me?",
        "It's not the size of your sword, it's how #enhance'd you are with it.",
        "Madame Elvira's House O' Succubi Lifetime Customer",
        "Madame Elvira's House O' Succubi Employee of the Month",
        "Ludios Vault Guards Do It In Small, Dark Rooms",
        "Yendor Military Soldiers Do It In Large Groups",
        "I survived Yendor Military Boot Camp",
        "Ludios Accounting School Intra-Mural Lacrosse Team",
        "Oracle(TM) Fountains 10th Annual Wet T-Shirt Contest",
        "Hey, black dragon!  Disintegrate THIS!",
        "I'm With Stupid -->",
        "Don't blame me, I voted for Izchak!",
        "Frodo Lives!",
        "Actually, I AM a quantum mechanic.",
        "I beat the Sword Master",  /* Monkey Island */
        "Don't Panic",      /* HHGTTG */
        "Furinkan High School Athletic Dept.", /* Ranma 1/2 */
        "Hel-LOOO, Nurse!", /* Animaniacs */
        /* NAO */
        "=^.^=",
        "100% goblin hair - do not wash",
        "Aim >>> <<< here",
        "cK -- Cockatrice touches the Kop",
        "Don't ask me, I only adventure here",
        "Down With Pants!",
        "Gehennoms Angels",
        "Glutton For Punishment",
        "Go Team Ant!",
        "Got Newt?",
        "Heading for Godhead",
        "Hello, my darlings!", /* Charlie Drake */
        "Hey! Nymphs! Steal This T-Shirt!",
        "I <3 Dungeon of Doom",
        "I am a Valkyrie. If you see me running, try to keep up.",
        "I Am Not a Pack Rat - I Am a Collector",
        "I bounced off a rubber tree",
        "If you can read this, I can hit you with my polearm",
        "I Support Single Succubi",
        "I want to live forever or die in the attempt.",
        "Kop Killaz",
        "Lichen Park",
        "LOST IN THOUGHT - please send search party",
        "Minetown Watch",
        "Ms. Palm's House of Negotiable Affection -- A Very Reputable House Of Disrepute",
        "^^  My eyes are up there!  ^^",
        "Next time you wave at me, use more than one finger, please.",
        "Objects In This Shirt Are Closer Than They Appear",
        "Protection Racketeer",
        "P Happens",
        "Real men love Crom",
        "Sokoban Gym -- Get Strong or Die Trying",
        "Somebody stole my Mojo!",
        "The Hellhound Gang",
        "The Werewolves",
        "They Might Be Storm Giants",
        "Weapons don't kill people, I kill people",
        "White Zombie",
        "Worship me",
        "You laugh because I'm different, I laugh because you're about to die",
        "You should hear what the voices in my head are saying about you.",
        "Anhur State University - Home of the Fighting Fire Ants!",
        "FREE HUGS",
        "Serial Ascender",
        "I couldn't afford this T-shirt so I stole it!",
        "End Mercantile Opacity Discrimination Now: Let Invisible Customers Shop!",
        "Elvira's House O'Succubi, granting the gift of immorality!",
        "Mind Flayers Suck",
        "I'm not wearing any pants",
        "Newt Fodder",
        "Pudding Farmer",
        "Vegetarian",
        "I plan to go to Astral",
        /* NetHack */
        "I made a NetHack fork and all I got was this lousy T-shirt!", /* galehar */
    };

    Strcpy(buf, shirt_msgs[tshirt->o_id % SIZE(shirt_msgs)]);
    return erode_obj_text(tshirt, buf);
}

char *
hawaiian_motif(struct obj *shirt, char *buf)
{
    static const char *hawaiian_motifs[] = {
        /* birds */
        "flamingo",
        "parrot",
        "toucan",
        "bird of paradise", /* could be a bird or a flower */
        /* sea creatures */
        "sea turtle",
        "tropical fish",
        "jellyfish",
        "giant eel",
        "water nymph",
        /* plants */
        "plumeria",
        "orchid",
        "hibiscus flower",
        "palm tree",
        /* other */
        "hula dancer",
        "sailboat",
        "ukulele",
    };

    /* a tourist's starting shirt always has the same o_id; we need some
       additional randomness or else its design will never differ */
    unsigned motif = shirt->o_id ^ (unsigned) game_seed;

    Strcpy(buf, hawaiian_motifs[motif % SIZE(hawaiian_motifs)]);
    return buf;
}

static char *
hawaiian_design(struct obj *shirt, char *buf)
{
    static const char *hawaiian_bgs[] = {
        /* solid colors */
        "purple",
        "yellow",
        "red",
        "blue",
        "orange",
        "black",
        "green",
        /* adjectives */
        "abstract",
        "geometric",
        "patterned",
        "naturalistic",
    };

    /* This hash method is slightly different than the one in hawaiian_motif;
       using the same formula in both cases may lead to some shirt combos
       never appearing, if the sizes of the two lists have common factors. */
    unsigned bg = shirt->o_id ^ (unsigned) ~game_seed;

    Sprintf(buf, "%s on %s background",
            makeplural(hawaiian_motif(shirt, buf)),
            an(hawaiian_bgs[bg % SIZE(hawaiian_bgs)]));
    return buf;
}

char *
apron_text(struct obj* apron, char* buf)
{
    static const char *apron_msgs[] = {
        "Kiss the cook",
        "I'm making SCIENCE!",
        "Don't mess with the chef",
        "Don't make me poison you",
        "Gehennom's Kitchen",
        "Rat: The other white meat",
        "If you can't stand the heat, get out of Gehennom!",
        "If we weren't meant to eat animals, why are they made out of meat?",
        "If you don't like the food, I'll stab you",
        /* In the movie "The Sum of All Fears", a Russian worker in a weapons
           facility wears a T-shirt that a translator says reads, "I am a
           bomb technician, if you see me running ... try to catch up."
           In nethack, the quote is far more suitable to an alchemy smock
           (particularly since so many of these others are about cooking)
           than a T-shirt and is paraphrased to simplify/shorten it.
           [later... turns out that this is already a T-shirt message:
            "I am a Valkyrie.  If you see me running, try to keep up."
           so this one has been revised a little:  added alchemist prefix,
           changed "keep up" to original source's "catch up"] */
        "I am an alchemist; if you see me running, try to catch up...",
    };

    Strcpy(buf, apron_msgs[apron->o_id % SIZE(apron_msgs)]);
    return erode_obj_text(apron, buf);
}

int
doread()
{
    register struct obj *scroll;
    register boolean confused;

    known = FALSE;
    if (check_capacity((char *)0)) {
        return 0;
    }
    scroll = getobj(readable, "read");
    if (!scroll) {
        return 0;
    }

    /* outrumor has its own blindness check */
    if (scroll->otyp == FORTUNE_COOKIE) {
        if (flags.verbose) {
            You("break up the cookie and throw away the pieces.");
        }
        if (u.roleplay.illiterate) {
            pline("This cookie has a scrap of paper inside.");
            pline("What a pity that you cannot read!");
        } else {
            outrumor(bcsign(scroll), BY_COOKIE);
            if (!Blind) {
                if (successful_cdt(CONDUCT_ILLITERACY)) {
                    livelog_printf(LL_CONDUCT, "became literate by reading a fortune cookie");
                }
                violated(CONDUCT_ILLITERACY);
            }
        }
        useup(scroll);
        return 1;
#ifdef TOURIST
    } else if (scroll->otyp == T_SHIRT ||
               scroll->otyp == ALCHEMY_SMOCK ||
               scroll->otyp == HAWAIIAN_SHIRT) {
        char buf[BUFSZ];

        if (Blind) {
            You_cant("feel any Braille writing.");
            return 0;
        }
        if (u.roleplay.illiterate) {
            pline("Unfortunately you cannot read!");
            return 0;
        }
        /* can't read shirt worn under suit (under cloak is ok though) */
        if ((scroll->otyp == T_SHIRT || scroll->otyp == HAWAIIAN_SHIRT) &&
             uarm && scroll == uarmu) {
            pline("%s shirt is obscured by %s %s.",
                  scroll->unpaid ? "That" : "Your", shk_your(buf, uarm),
                  suit_simple_name(uarm));
            return 0;
        }

        if (successful_cdt(CONDUCT_ILLITERACY)) {
            livelog_printf(LL_CONDUCT, "became literate by reading %s",
                           an(dump_typename(scroll->otyp)));
        }

        if (scroll->otyp == HAWAIIAN_SHIRT) {
            pline("%s features %s.", flags.verbose ? "The design" : "It", hawaiian_design(scroll, buf));
            return 1;
        }
        violated(CONDUCT_ILLITERACY);

        /* populate 'buf[]' */
        char *mesg = (scroll->otyp == T_SHIRT) ? tshirt_text(scroll, buf) : apron_text(scroll, buf);
        const char *endpunct = "";
        if (flags.verbose) {
            int ln = (int) strlen(mesg);

            /* we will be displaying a sentence; need ending punctuation */
            if (ln > 0 && !index(".!?", mesg[ln - 1])) {
                endpunct = ".";
            }
            pline("It reads:");
        }
        pline("\"%s\"%s", mesg, endpunct);
        return 1;
    } else if (scroll->otyp == CREDIT_CARD) {
        static const char *card_msgs[] = {
            "Leprechaun Gold Tru$t - Shamrock Card",
            "Magic Memory Vault Charge Card",
            "Larn National Bank", /* Larn */
            "First Bank of Omega", /* Omega */
            "Bank of Zork - Frobozz Magic Card", /* Zork */
            "Ankh-Morpork Merchant's Guild Barter Card",
            "Ankh-Morpork Thieves' Guild Unlimited Transaction Card",
            "Ransmannsby Moneylenders Association",
            "Bank of Gehennom - 99% Interest Card",
            "Yendorian Express - Copper Card",
            "Yendorian Express - Silver Card",
            "Yendorian Express - Gold Card",
            "Yendorian Express - Mithril Card",
            "Yendorian Express - Platinum Card",
        };
        if (Blind) {
            You("feel the embossed numbers:");
        } else {
            if (flags.verbose)
                pline("It reads:");
            pline("\"%s\"", scroll->oartifact ? card_msgs[SIZE(card_msgs)-1]
                  : card_msgs[scroll->o_id % (SIZE(card_msgs)-1)]);
        }
        /* Make a credit card number */
        pline("\"%d0%d %d%d1 0%d%d0\"", (int)((scroll->o_id % 89)+10), (int)(scroll->o_id % 4),
              (int)(((scroll->o_id * 499) % 899999) + 100000), (int)(scroll->o_id % 10),
              (int)(!(scroll->o_id % 3)), (int)((scroll->o_id * 7) % 10));

        if (successful_cdt(CONDUCT_ILLITERACY)) {
            livelog_printf(LL_CONDUCT, "became literate by reading %s", an(dump_typename(scroll->otyp)));
        }
        violated(CONDUCT_ILLITERACY);
        return 1;
#endif  /* TOURIST */

    } else if ((scroll->otyp == TIN) ||
               (scroll->otyp == CAN_OF_GREASE) ||
               (scroll->otyp == CANDY_BAR)) {
        pline("This %s has no %s.", singular(scroll, xname),
              (scroll->otyp == CANDY_BAR) ? "wrapper" : "label");
        return(0);

    } else if (scroll->otyp == MAGIC_MARKER) {
        if (Blind) {
            You_cant("feel any Braille writing.");
            return 0;
        }
        if (flags.verbose)
            pline("It reads:");
        pline("\"Magic Marker(TM) Red Ink Marker Pen.  Water Soluble.\"");

        if (successful_cdt(CONDUCT_ILLITERACY)) {
            livelog_printf(LL_CONDUCT, "became literate by reading %s", an(dump_typename(scroll->otyp)));
        }
        violated(CONDUCT_ILLITERACY);
        return 1;

    } else if (scroll->oclass == COIN_CLASS) {
        if (Blind)
            You("feel the embossed words:");
        else if (flags.verbose)
            You("read:");
        pline("\"1 Zorkmid. 857 GUE.  In Frobs We Trust.\"");

        if (successful_cdt(CONDUCT_ILLITERACY)) {
            livelog_printf(LL_CONDUCT, "became literate by reading a coin's engravings");
        }
        violated(CONDUCT_ILLITERACY);
        return 1;

    } else if (scroll->oartifact == ART_ORB_OF_FATE) {
        if (Blind)
            You("feel the engraved signature:");
        else pline("It is signed:");
        pline("\"Odin.\"");

        if (successful_cdt(CONDUCT_ILLITERACY)) {
            livelog_printf(LL_CONDUCT, "became literate by reading the divine signature of Odin");
        }
        violated(CONDUCT_ILLITERACY);
        return 1;

    } else if (OBJ_DESCR(objects[scroll->otyp]) &&
               !strncmp(OBJ_DESCR(objects[scroll->otyp]), "runed", 5)) {
        if (scroll->otyp == RUNESWORD) {
            You_cant("decipher the arcane runes.");
            return 0;
        } if (!Race_if(PM_ELF) && !Role_if(PM_ARCHEOLOGIST)) {
            You_cant("decipher the Elvish runes.");
            return 0;
        }

        if (successful_cdt(CONDUCT_ILLITERACY)) {
            livelog_printf(LL_CONDUCT, "became literate by reading Elvish runes");
        }
        violated(CONDUCT_ILLITERACY);

        if (objects[scroll->otyp].oc_merge) {
            if (Blind)
                You("feel the engraved runes:");
            else if (flags.verbose) pline("The runes read:");
            pline("\"Made in Elfheim.\"");
            return 1;
        } else {
            /* "Avoid any artifact with Runes on it, even if the Runes
             *  prove only to spell the maker's name." -- Diana Wynne Jones
             */
            /* Elf name fragments courtesy of ToME */
            static const char *elf_syllable1[] = {
                "Al", "An", "Bal", "Bel", "Cal", "Cel", "El", "Elr", "Elv", "Eow",
                "Ear", "F", "Fal", "Fel", "Fin", "G", "Gal", "Gel", "Gl", "Is", "Lan",
                "Leg", "Lom", "N", "Nal", "Nel", "S", "Sal", "Sel", "T", "Tal", "Tel",
                "Thr", "Tin",
            };
            static const char *elf_syllable2[] = {
                "a", "adrie", "ara", "e", "ebri", "ele", "ere", "i", "io", "ithra",
                "ilma", "il-Ga", "ili", "o", "orfi", "u", "y",
            };
            static const char *elf_syllable3[] = {
                "l", "las", "lad", "ldor", "ldur", "linde", "lith", "mir", "n", "nd",
                "ndel", "ndil", "ndir", "nduil", "ng", "mbor", "r", "rith", "ril",
                "riand", "rion", "s", "thien", "viel", "wen", "wyn",
            };
            if (Blind)
                You("feel the engraved signature:");
            else
                pline("It is signed:");
            pline("\"%s%s%s\"",
                  elf_syllable1[scroll->o_id % SIZE(elf_syllable1)],
                  elf_syllable2[scroll->o_id % SIZE(elf_syllable2)],
                  elf_syllable3[scroll->o_id % SIZE(elf_syllable3)]);
            return 1;
        }
    } else if (scroll->oclass != SCROLL_CLASS
               && scroll->oclass != SPBOOK_CLASS) {
        pline(silly_thing_to, "read");
        return(0);
    } else if (u.roleplay.illiterate && (scroll->otyp != SPE_BOOK_OF_THE_DEAD)) {
        pline("Unfortunately you cannot read.");
        return(0);
    } else if (Blind && (scroll->otyp != SPE_BOOK_OF_THE_DEAD)) {
        const char *what = 0;
        if (scroll->oclass == SPBOOK_CLASS)
            what = "mystic runes";
        else if (!scroll->dknown)
            what = "formula on the scroll";
        if (what) {
            pline("Being blind, you cannot read the %s.", what);
            return(0);
        }
    }

    /* Actions required to win the game aren't counted towards conduct */
    if (scroll->otyp != SPE_BOOK_OF_THE_DEAD &&
        scroll->otyp != SPE_BLANK_PAPER &&
        scroll->otyp != SCR_BLANK_PAPER
#ifdef MAIL
        && scroll->otyp != SCR_MAIL
#endif
        ) {
        if (successful_cdt(CONDUCT_ILLITERACY)) {
            livelog_printf(LL_CONDUCT, "became literate by reading %s",
                           scroll->oclass == SPBOOK_CLASS ? "a book" :
                           scroll->oclass == SCROLL_CLASS ? "a scroll" : "something");
        }
        violated(CONDUCT_ILLITERACY);
    }

    confused = (Confusion != 0);
#ifdef MAIL
    if (scroll->otyp == SCR_MAIL) confused = FALSE;
#endif
    if(scroll->oclass == SPBOOK_CLASS) {
        return(study_book(scroll));
    }
    scroll->in_use = TRUE; /* scroll, not spellbook, now being read */
    if (scroll->otyp != SCR_BLANK_PAPER) {
        boolean silently = !can_chant(&youmonst);

        /* a few scroll feedback messages describe something happening
           to the scroll itself, so avoid "it disappears" for those */
        boolean nodisappear = (scroll->otyp == SCR_FIRE ||
                               (scroll->otyp == SCR_REMOVE_CURSE && scroll->cursed));
        if (Blind) {
            pline(nodisappear ? "You %s the formula on the scroll." :
                                 "As you %s the formula on it, the scroll disappears.",
                  silently ? "cogitate" : "pronounce");
        } else {
            pline(nodisappear ? "You read the scroll." : "As you read the scroll, it disappears.");
        }
        if (confused) {
            if (Hallucination)
                pline("Being so trippy, you screw up...");
            else
                pline("Being confused, you mis%s the magic words...",
                      silently ? "understand" : "pronounce");
        }
    }
    if (!seffects(scroll))  {
        if (!objects[scroll->otyp].oc_name_known) {
            if (known) {
                learn_scroll(scroll);
            } else if (!objects[scroll->otyp].oc_uname)
                docall(scroll);
        }
        scroll->in_use = FALSE;
        if (scroll->otyp != SCR_BLANK_PAPER) {
            useup(scroll);
        }
    }
    return(1);
}

static void
stripspe(obj)
register struct obj *obj;
{
    if (obj->blessed || obj->spe <= 0) {
        pline("%s", nothing_happens);
    } else {
        /* order matters: message, shop handling, actual transformation */
        pline("%s briefly.", Yobjnam2(obj, "vibrate"));
        costly_alteration(obj, COST_UNCHRG);
        obj->spe = 0;
        if (obj->otyp == OIL_LAMP || obj->otyp == BRASS_LANTERN) {
            obj->age = 0;
        }
    }
}

static void
p_glow1(otmp)
register struct obj *otmp;
{
    Your("%s %s briefly.", xname(otmp),
         otense(otmp, Blind ? "vibrate" : "glow"));
}

static void
p_glow2(otmp, color)
register struct obj *otmp;
register const char *color;
{
    Your("%s %s%s%s for a moment.",
         xname(otmp),
         otense(otmp, Blind ? "vibrate" : "glow"),
         Blind ? "" : " ",
         Blind ? nul : hcolor(color));
}

/* Is the object chargeable?  For purposes of inventory display; it is */
/* possible to be able to charge things for which this returns FALSE. */
boolean
is_chargeable(obj)
struct obj *obj;
{
    if (obj->oclass == WAND_CLASS) return TRUE;
    /* known && !oc_name_known is possible after amnesia/mind flayer */
    if (obj->oclass == RING_CLASS) {
        return (objects[obj->otyp].oc_charged &&
                (obj->known ||
                 (obj->dknown && objects[obj->otyp].oc_name_known)));
    }
    /* specific check before general tools */
    if (is_weptool(obj)) {
        return FALSE;
    }
    if (obj->oclass == TOOL_CLASS) {
        if (obj->otyp == BRASS_LANTERN ||
            (obj->otyp == OIL_LAMP) ||
            /* only list magic lamps if they are not identified yet */
            (obj->otyp == MAGIC_LAMP && !objects[MAGIC_LAMP].oc_name_known)) {
            return TRUE;
        }
        return objects[obj->otyp].oc_charged;
    }
    return FALSE; /* why are weapons/armor considered charged anyway? */
}

/*
 * recharge an object; curse_bless is -1 if the recharging implement
 * was cursed, +1 if blessed, 0 otherwise.
 */
void
recharge(obj, curse_bless)
struct obj *obj;
int curse_bless;
{
    register int n;
    boolean is_cursed, is_blessed;

    is_cursed = curse_bless < 0;
    is_blessed = curse_bless > 0;

    /* Scrolls of charging now ID charge count when blessed */
    if (is_blessed) obj->known = 1;

    if (obj->oclass == WAND_CLASS) {
        /* undo any prior cancellation, even when is_cursed */
        if (obj->spe == -1) obj->spe = 0;

        /*
         * Recharging might cause wands to explode.
         *      v = number of previous recharges
         *            v = percentage chance to explode on this attempt
         *                    v = cumulative odds for exploding
         *      0 :   0       0
         *      1 :   0.29    0.29
         *      2 :   2.33    2.62
         *      3 :   7.87   10.28
         *      4 :  18.66   27.02
         *      5 :  36.44   53.62
         *      6 :  62.97   82.83
         *      7 : 100     100
         */
        n = (int)obj->recharged;
        if (n > 0 && (obj->otyp == WAN_WISHING ||
                      (n * n * n > rn2(7*7*7)))) { /* recharge_limit */
            wand_explode(obj);
            return;
        }
        /* didn't explode, so increment the recharge count */
        obj->recharged = (unsigned)(n + 1);

        /* now handle the actual recharging */
        if (is_cursed) {
            stripspe(obj);
        } else {
            int lim = (obj->otyp == WAN_WISHING) ? 3 :
                      (objects[obj->otyp].oc_dir != NODIR) ? 8 : 15;

            n = (lim == 3) ? 3 : rn1(5, lim + 1 - 5);
            if (!is_blessed) n = rnd(n);

            if (obj->spe < n) obj->spe = n;
            else obj->spe++;
            if (obj->otyp == WAN_WISHING && obj->spe > 3) {
                wand_explode(obj);
                return;
            }
            if (obj->spe >= lim) p_glow2(obj, NH_BLUE);
            else p_glow1(obj);
        }

    } else if (obj->oclass == RING_CLASS &&
               objects[obj->otyp].oc_charged) {
        /* charging does not affect ring's curse/bless status */
        int s = is_blessed ? rnd(3) : is_cursed ? -rnd(2) : 1;
        boolean is_on = (obj == uleft || obj == uright);

        /* destruction depends on current state, not adjustment */
        if (obj->spe > rn2(7) || obj->spe <= -5) {
            pline("%s momentarily, then %s!", Yobjnam2(obj, "pulsate"),
                  otense(obj, "explode"));
            if (is_on) Ring_gone(obj);
            s = rnd(3 * abs(obj->spe)); /* amount of damage */
            useup(obj);
            losehp(Maybe_Half_Phys(s), "exploding ring", KILLED_BY_AN);
        } else {
            long mask = is_on ? (obj == uleft ? LEFT_RING :
                                 RIGHT_RING) : 0L;
            pline("%s spins %sclockwise for a moment.", Yname2(obj), s < 0 ? "counter" : "");
            if (s < 0) {
                costly_alteration(obj, COST_DECHNT);
            }
            /* cause attributes and/or properties to be updated */
            if (is_on) Ring_off(obj);
            obj->spe += s; /* update the ring while it's off */
            if (is_on) setworn(obj, mask), Ring_on(obj);
            /* oartifact: if a touch-sensitive artifact ring is
               ever created the above will need to be revised  */
            /* update shop bill to reflect new higher price */
            if (s > 0 && obj->unpaid) {
                alter_cost(obj, 0L);
            }
        }

    } else if (obj->oclass == TOOL_CLASS) {
        int rechrg = (int)obj->recharged;

        if (objects[obj->otyp].oc_charged) {
            /* tools don't have a limit, but the counter used does */
            if (rechrg < 7) /* recharge_limit */
                obj->recharged++;
        }
        switch (obj->otyp) {
        case BELL_OF_OPENING:
            if (is_cursed) stripspe(obj);
            else if (is_blessed) obj->spe += rnd(3);
            else obj->spe += 1;
            if (obj->spe > 5) obj->spe = 5;
            break;

        case MAGIC_MARKER:
        case TINNING_KIT:
#ifdef TOURIST
        case EXPENSIVE_CAMERA:
#endif
            if (is_cursed) stripspe(obj);
            else if (rechrg && obj->otyp == MAGIC_MARKER) { /* previously recharged */
                obj->recharged = 1; /* override increment done above */
                if (obj->spe < 3)
                    Your("marker seems permanently dried out.");
                else
                    pline("%s", nothing_happens);
            } else if (is_blessed) {
                n = rn1(16, 15); /* 15..30 */
                if (obj->spe + n <= 50)
                    obj->spe = 50;
                else if (obj->spe + n <= 75)
                    obj->spe = 75;
                else {
                    int chrg = (int)obj->spe;
                    if ((chrg + n) > 127)
                        obj->spe = 127;
                    else
                        obj->spe += n;
                }
                p_glow2(obj, NH_BLUE);
            } else {
                n = rn1(11, 10); /* 10..20 */
                if (obj->spe + n <= 50)
                    obj->spe = 50;
                else {
                    int chrg = (int)obj->spe;
                    if ((chrg + n) > 127)
                        obj->spe = 127;
                    else
                        obj->spe += n;
                }
                p_glow2(obj, NH_WHITE);
            }
            break;

        case OIL_LAMP:
        case BRASS_LANTERN:
            if (is_cursed) {
                stripspe(obj);
                if (obj->lamplit) {
                    if (!Blind)
                        pline("%s out!", Tobjnam(obj, "go"));
                    end_burn(obj, TRUE);
                }
            } else if (is_blessed) {
                obj->spe = 1;
                obj->age = MAX_LAMP_FUEL;
                obj->cknown = FALSE;
                p_glow2(obj, NH_BLUE);
            } else {
                obj->spe = 1;
                obj->age += MAX_LAMP_FUEL/2;
                obj->cknown = FALSE;
                if (obj->age > MAX_LAMP_FUEL) {
                    obj->age = MAX_LAMP_FUEL;
                }
                p_glow1(obj);
            }
            /* give some indication if obj is full */
            if (obj->age == MAX_LAMP_FUEL) {
                if (obj->otyp == BRASS_LANTERN) {
                    if (Blind) {
                        You("smell %s.", Hallucination ? "unicorn farts" : "ozone");
                    } else {
                        You_see("%s from %s.",
                                Hallucination ? "unicorn farts escape" : "sparks fly",
                                yname(obj));
                    }
                } else if (obj->otyp == OIL_LAMP) {
                    pline("%s is filled to the brim.", Yname2(obj));
                }
            }
            break;

        case CRYSTAL_BALL:
            if (is_cursed) stripspe(obj);
            else if (is_blessed) {
                obj->spe = 6;
                p_glow2(obj, NH_BLUE);
            } else {
                if (obj->spe < 5) {
                    obj->spe++;
                    p_glow1(obj);
                } else pline("%s", nothing_happens);
            }
            break;

        case BAG_OF_TRICKS:
            /* if there are any objects inside the bag, devour them */
            if (!is_cursed) {
                struct obj *curr, *otmp;
                struct monst *shkp;
                int lcnt = 0;
                long loss = 0L;

                makeknown(BAG_OF_TRICKS);
                for (curr = obj->cobj; curr; curr = otmp) {
                    otmp = curr->nobj;
                    obj_extract_self(curr);
                    lcnt++;
                    if (*u.ushops && (shkp = shop_keeper(*u.ushops)) != 0) {
                        if (curr->unpaid)
                            loss += stolen_value(curr, u.ux, u.uy,
                                                 (boolean)shkp->mpeaceful, TRUE);
                    }
                    /* obfree() will free all contained objects */
                    obfree(curr, (struct obj *) 0);
                }

                if (lcnt)
                    You_hear("loud crunching sounds from inside %s.", yname(obj));
                if (lcnt && loss)
                    You("owe %ld %s for lost item%s.",
                        loss, currency(loss), lcnt > 1 ? "s" : "");
            }
        /* fall through */

        case HORN_OF_PLENTY:
        case CAN_OF_GREASE:
            if (is_cursed) stripspe(obj);
            else if (is_blessed) {
                if (obj->spe <= 10)
                    obj->spe += rn1(10, 6);
                else obj->spe += rn1(5, 6);
                if (obj->spe > 50) obj->spe = 50;
                p_glow2(obj, NH_BLUE);
            } else {
                obj->spe += rnd(5);
                if (obj->spe > 50) obj->spe = 50;
                p_glow1(obj);
            }
            break;

        case MAGIC_FLUTE:
        case MAGIC_HARP:
        case FROST_HORN:
        case FIRE_HORN:
        case DRUM_OF_EARTHQUAKE:
            if (is_cursed) {
                stripspe(obj);
            } else if (is_blessed) {
                obj->spe += d(2, 4);
                if (obj->spe > 20) obj->spe = 20;
                p_glow2(obj, NH_BLUE);
            } else {
                obj->spe += rnd(4);
                if (obj->spe > 20) obj->spe = 20;
                p_glow1(obj);
            }
            break;

        default:
            goto not_chargable;
            /*NOTREACHED*/
            break;
        } /* switch */

    } else {
not_chargable:
        You("have a feeling of loss.");
    }
}

/* Forget known information about this object class. */
static void
forget_single_object(obj_id)
int obj_id;
{
    objects[obj_id].oc_name_known = 0;
    objects[obj_id].oc_pre_discovered = 0;  /* a discovery when relearned */
    if (objects[obj_id].oc_uname) {
        free((genericptr_t)objects[obj_id].oc_uname);
        objects[obj_id].oc_uname = 0;
    }
    undiscover_object(obj_id);  /* after clearing oc_name_known */

    if (Is_dragon_scales(obj_id) &&
        objects[Dragon_scales_to_mail(obj_id)].oc_name_known)
        forget_single_object(Dragon_scales_to_mail(obj_id));
    else if (Is_dragon_mail(obj_id) &&
             objects[Dragon_mail_to_scales(obj_id)].oc_name_known)
        forget_single_object(Dragon_mail_to_scales(obj_id));

    /* clear & free object names from matching inventory items too? */
}


#if 0   /* here if anyone wants it.... */
/* Forget everything known about a particular object class. */
static void
forget_objclass(oclass)
int oclass;
{
    int i;

    for (i=bases[oclass];
         i < NUM_OBJECTS && objects[i].oc_class==oclass; i++)
        forget_single_object(i);
}
#endif


/* randomize the given list of numbers  0 <= i < count */
static void
randomize(indices, count)
int *indices;
int count;
{
    int i, iswap, temp;

    for (i = count - 1; i > 0; i--) {
        if ((iswap = rn2(i + 1)) == i) continue;
        temp = indices[i];
        indices[i] = indices[iswap];
        indices[iswap] = temp;
    }
}


/* Forget % of known objects. */
void
forget_objects(percent)
int percent;
{
    int i, count;
    int indices[NUM_OBJECTS];

    if (percent == 0) return;
    if (percent <= 0 || percent > 100) {
        impossible("forget_objects: bad percent %d", percent);
        return;
    }

    indices[0] = 0; /* lint suppression */
    for (count = 0, i = 1; i < NUM_OBJECTS; i++)
        if (OBJ_DESCR(objects[i]) &&
            (objects[i].oc_name_known || objects[i].oc_uname))
            indices[count++] = i;

    if (count > 0) {
        randomize(indices, count);

        /* forget first % of randomized indices */
        count = ((count * percent) + rn2(100)) / 100;
        for (i = 0; i < count; i++) {
            forget_single_object(indices[i]);
        }
    }
}

/* Forget some or all of map (depends on parameters). */
void
forget_map(howmuch)
int howmuch;
{
    register int zx, zy;

    if (In_sokoban(&u.uz))
        return;

    known = TRUE;
    for(zx = 0; zx < COLNO; zx++) for(zy = 0; zy < ROWNO; zy++)
            if (howmuch & ALL_MAP || rn2(7)) {
                /* Zonk all memory of this location. */
                levl[zx][zy].seenv = 0;
                levl[zx][zy].stepped_on = 0;
                levl[zx][zy].waslit = 0;
                levl[zx][zy].glyph = cmap_to_glyph(S_stone);
                levl[zx][zy].styp = STONE;
            }

    /* forget overview data for this level */
    forget_mapseen(ledger_no(&u.uz));
}

/* Forget all traps on the level. */
void
forget_traps()
{
    register struct trap *trap;

    /* forget all traps (except the one the hero is in :-) */
    for (trap = ftrap; trap; trap = trap->ntrap)
        if ((trap->tx != u.ux || trap->ty != u.uy) && (trap->ttyp != HOLE))
            trap->tseen = 0;
}

/*
 * Forget given % of all levels that the hero has visited and not forgotten,
 * except this one.
 */
void
forget_levels(percent)
int percent;
{
    int i, count;
    xchar maxl, this_lev;
    int indices[MAXLINFO];

    if (percent == 0) return;

    if (percent <= 0 || percent > 100) {
        impossible("forget_levels: bad percent %d", percent);
        return;
    }

    this_lev = ledger_no(&u.uz);
    maxl = maxledgerno();

    /* count & save indices of non-forgotten visited levels */
    /* Sokoban levels are pre-mapped for the player, and should stay
     * so, or they become nearly impossible to solve.  But try to
     * shift the forgetting elsewhere by fiddling with percent
     * instead of forgetting fewer levels.
     */
    indices[0] = 0; /* lint suppression */
    for (count = 0, i = 0; i <= maxl; i++)
        if ((level_info[i].flags & VISITED) &&
            !(level_info[i].flags & FORGOTTEN) && i != this_lev) {
            if (ledger_to_dnum(i) == sokoban_dnum)
                percent += 2;
            else
                indices[count++] = i;
        }

    if (percent > 100) percent = 100;

    if (count > 0) {
        randomize(indices, count);

        /* forget first % of randomized indices */
        count = ((count * percent) + 50) / 100;
        for (i = 0; i < count; i++) {
            level_info[indices[i]].flags |= FORGOTTEN;
            forget_mapseen(indices[i]);
        }
    }
}

#if 0
/*
 * Forget some things (e.g. after reading a scroll of amnesia).  When called,
 * the following are always forgotten:
 *
 *  - felt ball & chain
 *  - traps
 *  - part (6 out of 7) of the map
 *
 * Other things are subject to flags:
 *
 *  howmuch & ALL_MAP   = forget whole map
 *  howmuch & ALL_SPELLS    = forget all spells
 */
static void
forget(howmuch)
int howmuch;
{

    if (Punished) u.bc_felt = 0;    /* forget felt ball&chain */

    forget_map(howmuch);
    forget_traps();

    /* 1 in 3 chance of forgetting some levels */
    if (!rn2(3)) forget_levels(rn2(25));

    /* 1 in 3 chance of forgeting some objects */
    if (!rn2(3)) forget_objects(rn2(25));

    if (howmuch & ALL_SPELLS) losespells();
    /*
     * Make sure that what was seen is restored correctly.  To do this,
     * we need to go blind for an instant --- turn off the display,
     * then restart it.  All this work is needed to correctly handle
     * walls which are stone on one side and wall on the other.  Turning
     * off the seen bits above will make the wall revert to stone,  but
     * there are cases where we don't want this to happen.  The easiest
     * thing to do is to run it through the vision system again, which
     * is always correct.
     */
    docrt();        /* this correctly will reset vision */
}
#endif

/* monster is hit by scroll of taming's effect */
static int
maybe_tame(mtmp, sobj)
struct monst *mtmp;
struct obj *sobj;
{
    int was_tame = mtmp->mtame;
    unsigned was_peaceful = mtmp->mpeaceful;
#ifdef BLACKMARKET
    if (sobj->cursed || Is_blackmarket(&u.uz)) {
#else
    if (sobj->cursed) {
#endif
        setmangry(mtmp, FALSE);
    } else {
        if (mtmp->isshk)
            make_happy_shk(mtmp, FALSE);
        else if (!resist(mtmp, sobj->oclass, 0, NOTELL))
            (void) tamedog(mtmp, (struct obj *) 0);
        if ((!was_peaceful && mtmp->mpeaceful) || (!was_tame && mtmp->mtame)) {
            return 1;
        }
    }
    return 0;
}

/** Remove water tile at x,y. */
STATIC_PTR void
undo_flood(x, y, roomcnt)
int x, y;
genericptr_t roomcnt;
{
    if ((levl[x][y].typ != POOL) &&
        (levl[x][y].typ != MOAT) &&
        (levl[x][y].typ != WATER) &&
        (levl[x][y].typ != FOUNTAIN))
        return;

    (*(int *)roomcnt)++;

    /* Get rid of a pool at x, y */
    levl[x][y].typ = ROOM;
    newsym(x, y);
}

STATIC_PTR void
do_flood(x, y, poolcnt)
int x, y;
genericptr_t poolcnt;
{
    register struct monst *mtmp;
    register struct trap *ttmp;

    if (nexttodoor(x, y) || (rn2(1 + distmin(u.ux, u.uy, x, y))) ||
        (sobj_at(BOULDER, x, y)) || (levl[x][y].typ != ROOM))
        return;

    if ((ttmp = t_at(x, y)) != 0 && !delfloortrap(ttmp))
        return;

    (*(int *)poolcnt)++;

    if (!((*(int *)poolcnt) && (x == u.ux) && (y == u.uy))) {
        /* Put a pool at x, y */
        levl[x][y].typ = POOL;
        del_engr_at(x, y);
        water_damage_chain(level.objects[x][y], FALSE);

        if ((mtmp = m_at(x, y)) != 0) {
            (void) minliquid(mtmp);
        } else {
            newsym(x, y);
        }
    } else if ((x == u.ux) && (y == u.uy)) {
        (*(int *)poolcnt)--;
    }

}

static boolean
get_valid_stinking_cloud_pos(x,y)
int x,y;
{
    return (!(!isok(x,y) ||
              !cansee(x, y) ||
              !ACCESSIBLE(levl[x][y].typ) ||
              distu(x, y) >= 32));
}

static boolean
is_valid_stinking_cloud_pos(x, y, showmsg)
int x, y;
boolean showmsg;
{
    if (!get_valid_stinking_cloud_pos(x,y)) {
        if (showmsg) {
            You("smell rotten eggs.");
        }
        return FALSE;
    }
    return TRUE;
}

static void
display_stinking_cloud_positions(state)
int state;
{
    if (state == 0) {
        tmp_at(DISP_BEAM, cmap_to_glyph(S_goodpos));
    } else if (state == 1) {
        int x, y, dx, dy;
        int dist = 6;

        for (dx = -dist; dx <= dist; dx++) {
            for (dy = -dist; dy <= dist; dy++) {
                x = u.ux + dx;
                y = u.uy + dy;
                if (get_valid_stinking_cloud_pos(x,y)) {
                    tmp_at(x, y);
                }
            }
        }
    } else {
        tmp_at(DISP_END, 0);
    }
}

/* scroll effects; return 1 if we use up the scroll and possibly make it
   become discovered, 0 if caller should take care of those side-effects */
int
seffects(sobj)
struct obj *sobj; /* scroll, or fake spellbook object for scroll-like spell */
{
    int cval, otyp = sobj->otyp;
    boolean confused = (Confusion != 0);
    boolean sobj_blessed = sobj->blessed, sobj_cursed = sobj->cursed;
    boolean already_known, old_erodeproof, new_erodeproof;
    struct obj *otmp;

    if (objects[sobj->otyp].oc_magic) {
        exercise(A_WIS, TRUE); /* just for trying */
    }

    already_known = (sobj->oclass == SPBOOK_CLASS || /* spell */
                     objects[otyp].oc_name_known);

    switch (sobj->otyp) {
#ifdef MAIL
    case SCR_MAIL:
        known = TRUE;
        if (sobj->spe == MAIL_HINT) {
            read_hint(sobj);
        } else if (sobj->spe == 2) {
            /* "stamped scroll" created via magic marker--without a stamp */
            pline("This scroll is marked \"postage due\".");
        } else if (sobj->spe == MAIL_JUNK) {
            pline("This seems to be junk mail addressed to the finder of the Eye of Larn.");
            /* note to the puzzled: the game Larn actually sends you junk
             * mail if you win!
             */
        } else {
            readmail(sobj);
        }
        break;
#endif
    case SCR_ENCHANT_ARMOR:
    {
        register schar s;
        boolean special_armor;
        boolean same_color;

        otmp = some_armor(&youmonst);
        if (!otmp) {
            strange_feeling(sobj,
                            !Blind ? "Your skin glows then fades." :
                            "Your skin feels warm for a moment.");
            sobj = NULL; /* useup() in strange_feeling() */
            exercise(A_CON, !sobj_cursed);
            exercise(A_STR, !sobj_cursed);
            break;
        }
        if (confused) {
            old_erodeproof = otmp->oerodeproof;
            new_erodeproof = !sobj_cursed;
            otmp->oerodeproof = 0; /* for messages */
            if (Blind) {
                otmp->rknown = sobj->bknown;
                pline("%s warm for a moment.", Yobjnam2(otmp, "feel"));
            } else {
                otmp->rknown = TRUE;
                pline("%s covered by a %s %s %s!", Yobjnam2(otmp, "are"),
                     sobj_cursed ? "mottled" : "shimmering",
                     hcolor(sobj_cursed ? NH_BLACK : NH_GOLDEN),
                     sobj_cursed ? "glow" :
                     (is_shield(otmp) ? "layer" : "shield"));
            }
            if (new_erodeproof && (otmp->oeroded || otmp->oeroded2)) {
                otmp->oeroded = otmp->oeroded2 = 0;
                pline("%s as good as new!", Yobjnam2(otmp, Blind ? "feel" : "look"));
            }
            if (old_erodeproof && !new_erodeproof) {
                /* restore old_erodeproof before shop charges */
                otmp->oerodeproof = 1;
                costly_alteration(otmp, COST_DEGRD);
            }
            otmp->oerodeproof = new_erodeproof ? 1 : 0;
            break;
        }
        /* elven armor vibrates warningly when enchanted beyond a limit */
        special_armor = is_elven_armor(otmp) ||
                        (Role_if(PM_WIZARD) && otmp->otyp == CORNUTHAUM) ||
                        (Role_if(PM_ARCHEOLOGIST) && otmp->otyp == FEDORA);
        if (sobj_cursed) {
            same_color =
                (otmp->otyp == BLACK_DRAGON_SCALE_MAIL ||
                 otmp->otyp == BLACK_DRAGON_SCALES);
        } else {
            same_color =
                (otmp->otyp == SILVER_DRAGON_SCALE_MAIL ||
                 otmp->otyp == SILVER_DRAGON_SCALES ||
                 otmp->otyp == SHIELD_OF_REFLECTION);
        }
        if (Blind) same_color = FALSE;

        /* KMH -- catch underflow */
        s = sobj_cursed ? -otmp->spe : otmp->spe;
        if (s > (special_armor ? 5 : 3) && rn2(s)) {
            otmp->in_use = TRUE;
            pline("%s violently %s%s%s for a while, then %s.", Yname2(otmp),
                 otense(otmp, Blind ? "vibrate" : "glow"),
                 (!Blind && !same_color) ? " " : nul,
                 (Blind || same_color) ? nul :
                 hcolor(sobj_cursed ? NH_BLACK : NH_SILVER),
                 otense(otmp, "evaporate"));
            remove_worn_item(otmp, FALSE);
            useup(otmp);
            break;
        }
        s = sobj_cursed ? -1 :
            otmp->spe >= 9 ? (rn2(otmp->spe) == 0) :
            sobj_blessed ? rnd(3-otmp->spe/3) : 1;
        if (s >= 0 && otmp->otyp >= GRAY_DRAGON_SCALES &&
            otmp->otyp <= CHROMATIC_DRAGON_SCALES) {
            /* dragon scales get turned into dragon scale mail */
            Your("%s merges and hardens!", xname(otmp));
            setworn((struct obj *)0, W_ARM);
            /* assumes same order */
            otmp->otyp = GRAY_DRAGON_SCALE_MAIL +
                         otmp->otyp - GRAY_DRAGON_SCALES;
            if (sobj_blessed) {
                otmp->spe++;
                if (!otmp->blessed) {
                    bless(otmp);
                }
            } else if (otmp->cursed) {
                uncurse(otmp);
            }
            otmp->known = 1;
            setworn(otmp, W_ARM);
            if (otmp->unpaid) {
                alter_cost(otmp, 0L); /* shop bill */
            }
            break;
        }
        pline("%s %s%s%s%s for a %s.",
             Yname2(otmp),
             s == 0 ? "violently " : nul,
             otense(otmp, Blind ? "vibrate" : "glow"),
             (!Blind && !same_color) ? " " : nul,
             (Blind || same_color) ? nul : hcolor(sobj_cursed ? NH_BLACK : NH_SILVER),
             (s*s>1) ? "while" : "moment");
        /* [this cost handling will need updating if shop pricing is
           ever changed to care about curse/bless status of armor] */
        if (s < 0) {
            costly_alteration(otmp, COST_DECHNT);
        }
        if (sobj_cursed && !otmp->cursed) {
            curse(otmp);
        } else if (sobj_blessed && !otmp->blessed) {
            bless(otmp);
        } else if (!sobj_cursed && otmp->cursed) {
            uncurse(otmp);
        }
        if (s) {
            otmp->spe += s;
            adj_abon(otmp, s);
            known = otmp->known;
            /* update shop bill to reflect new higher price */
            if (s > 0 && otmp->unpaid) {
                alter_cost(otmp, 0L);
            }
        }

        if ((otmp->spe > (special_armor ? 5 : 3)) &&
            (special_armor || !rn2(7))) {
            pline("%s %s.", Yobjnam2(otmp, "suddenly vibrate"),
                  Blind ? "again" : "unexpectedly");
        }
        break;
    }
    case SCR_DESTROY_ARMOR:
    {
        otmp = some_armor(&youmonst);
        if (confused) {
            if (!otmp) {
                strange_feeling(sobj, "Your bones itch.");
                sobj = 0; /* useup() in strange_feeling() */
                exercise(A_STR, FALSE);
                exercise(A_CON, FALSE);
                break;
            }
            old_erodeproof = (otmp->oerodeproof != 0);
            new_erodeproof = sobj_cursed;
            otmp->oerodeproof = 0; /* for messages */
            p_glow2(otmp, NH_PURPLE);
            if (old_erodeproof && !new_erodeproof) {
                /* restore old_erodeproof before shop charges */
                otmp->oerodeproof = 1;
                costly_alteration(otmp, COST_DEGRD);
            }
            otmp->oerodeproof = new_erodeproof ? 1 : 0;
            break;
        }
        if (!sobj_cursed || !otmp || !otmp->cursed) {
            if (!destroy_arm(otmp)) {
                strange_feeling(sobj, "Your skin itches.");
                sobj = 0; /* useup() in strange_feeling() */
                exercise(A_STR, FALSE);
                exercise(A_CON, FALSE);
                break;
            } else
                known = TRUE;
        } else {
            /* armor and scroll both cursed */
            pline("%s.", Yobjnam2(otmp, "vibrate"));
            if (otmp->spe >= -6) {
                otmp->spe--;
                adj_abon(otmp, -1);
            }
            make_stunned(HStun + rn1(10, 10), TRUE);
        }
    }
    break;

    case SCR_CONFUSE_MONSTER:
    case SPE_CONFUSE_MONSTER:
        if (youmonst.data->mlet != S_HUMAN || sobj_cursed) {
            if(!HConfusion) You_feel("confused.");
            make_confused(HConfusion + rnd(100), FALSE);
        } else if (confused) {
            if (!sobj_blessed) {
                Your("%s begin to %s%s.",
                     makeplural(body_part(HAND)),
                     Blind ? "tingle" : "glow ",
                     Blind ? nul : hcolor(NH_PURPLE));
                make_confused(HConfusion + rnd(100), FALSE);
            } else {
                pline("A %s%s surrounds your %s.",
                      Blind ? nul : hcolor(NH_RED),
                      Blind ? "faint buzz" : " glow",
                      body_part(HEAD));
                make_confused(0L, TRUE);
            }
        } else {
            if (!sobj_blessed) {
                Your("%s%s %s%s.",
                     makeplural(body_part(HAND)),
                     Blind ? "" : " begin to glow",
                     Blind ? (const char *)"tingle" : hcolor(NH_RED),
                     u.umconf ? " even more" : "");
                u.umconf++;
            } else {
                if (Blind)
                    Your("%s tingle %s sharply.",
                         makeplural(body_part(HAND)),
                         u.umconf ? "even more" : "very");
                else
                    Your("%s glow a%s brilliant %s.",
                         makeplural(body_part(HAND)),
                         u.umconf ? "n even more" : "",
                         hcolor(NH_RED));
                /* after a while, repeated uses become less effective */
                if (u.umconf >= 40)
                    u.umconf++;
                else
                    u.umconf += rn1(8, 2);
            }
        }
        break;

    case SCR_SCARE_MONSTER:
    case SPE_CAUSE_FEAR:
    {   register int ct = 0;
        register struct monst *mtmp;

        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp)) continue;
            if (cansee(mtmp->mx, mtmp->my)) {
                if (confused || sobj_cursed) {
                    mtmp->mflee = mtmp->mfrozen = mtmp->msleeping = 0;
                    mtmp->mcanmove = 1;
                } else
                if (!resist(mtmp, sobj->oclass, 0, NOTELL))
                    monflee(mtmp, 0, FALSE, FALSE);
                if(!mtmp->mtame) ct++; /* pets don't laugh at you */
            }
        }
        if(!ct)
            You_hear("%s in the distance.",
                     (confused || sobj_cursed) ? "sad wailing" :
                     "maniacal laughter");
        else if(sobj->otyp == SCR_SCARE_MONSTER)
            You_hear("%s close by.",
                     (confused || sobj_cursed) ? "sad wailing" :
                     "maniacal laughter");
        break;}

    case SCR_BLANK_PAPER:
        if (Blind)
            You("don't remember there being any magic words on this scroll.");
        else
            pline("This scroll seems to be blank.");
        known = TRUE;
        break;

    case SCR_REMOVE_CURSE:
    case SPE_REMOVE_CURSE:
    {   register struct obj *obj;
        if(confused)
            if (Hallucination)
                You_feel("the power of the Force against you!");
            else
                You_feel("like you need some help.");
        else
        if (Hallucination)
            You_feel("in touch with the Universal Oneness.");
        else
            You_feel("like someone is helping you.");

        if (sobj_cursed) {
            pline_The("scroll disintegrates.");
        } else {
            for (obj = invent; obj; obj = obj->nobj) {
                long wornmask;

                /* gold isn't subject to cursing and blessing */
                if (obj->oclass == COIN_CLASS) continue;

                /* hide current scroll from itself so that perm_invent won't
                   show known blessed scroll losing bknown when confused */
                if (obj == sobj && obj->quan == 1L) {
                    continue;
                }

                wornmask = (obj->owornmask & ~(W_BALL|W_ART|W_ARTI));
                if (wornmask && !sobj_blessed) {
                    /* handle a couple of special cases; we don't
                       allow auxiliary weapon slots to be used to
                       artificially increase number of worn items */
                    if (obj == uswapwep) {
                        if (!u.twoweap) wornmask = 0L;
                    } else if (obj == uquiver) {
                        if (obj->oclass == WEAPON_CLASS) {
                            /* mergeable weapon test covers ammo,
                               missiles, spears, daggers & knives */
                            if (!objects[obj->otyp].oc_merge)
                                wornmask = 0L;
                        } else if (obj->oclass == GEM_CLASS) {
                            /* possibly ought to check whether
                               alternate weapon is a sling... */
                            if (!uslinging()) wornmask = 0L;
                        } else {
                            /* weptools don't merge and aren't
                               reasonable quivered weapons */
                            wornmask = 0L;
                        }
                    }
                }
                if (sobj_blessed || wornmask ||
                     obj->otyp == LOADSTONE ||
                     (obj->otyp == LEASH && obj->leashmon)) {
                    /* water price varies by curse/bless status */
                    boolean shop_h2o = (obj->unpaid && obj->otyp == POT_WATER);

                    if (confused) {
                        blessorcurse(obj, 2);
                        /* lose knowledge of this object's curse/bless
                           state (even if it didn't actually change) */
                        obj->bknown = 0;
                        /* blessorcurse() only affects uncursed items
                           so no need to worry about price of water
                           going down (hence no costly_alteration) */
                        if (shop_h2o && (obj->cursed || obj->blessed)) {
                            alter_cost(obj, 0L); /* price goes up */
                        }
                    } else if (obj->cursed) {
                        if (shop_h2o) {
                            costly_alteration(obj, COST_UNCURS);
                        }
                        uncurse(obj);
                        /* if the object was known to be cursed and is now known not to be,
                           make the scroll known; it's trivial to identify anyway by comparing
                           inventory before and after */
                        if (obj->bknown && otyp == SCR_REMOVE_CURSE) {
                            (void) learn_scroll_typ(SCR_REMOVE_CURSE, TRUE);
                        }
                    }
                }
            }
        }
        if (Punished && !confused) {
            unpunish();
        }
        if (u.utrap && u.utraptype == TT_BURIEDBALL) {
            buried_ball_to_freedom();
            pline_The("clasp on your %s vanishes.", body_part(LEG));
        }
        update_inventory();
        break;}

    case SCR_CREATE_MONSTER:
    case SPE_CREATE_MONSTER: {
        int count = 1 +
            ((confused || sobj_cursed) ? 12 : 0) +
            ((sobj_blessed || rn2(73)) ? 0 : rnd(4));
        struct permonst *what_monster = (confused ? &mons[PM_ACID_BLOB] : NULL);
        if (create_critters(count, what_monster)) {
            known = TRUE;
        }
        /* no need to flush monsters; we ask for identification only if the
         * monsters are not visible
         */
        break;
    }

    case SCR_ENCHANT_WEAPON:
        /* [What about twoweapon mode?  Proofing/repairing/enchanting both
           would be too powerful, but shouldn't we choose randomly between
           primary and secondary instead of always acting on primary?] */
        if (confused && uwep && erosion_matters(uwep) && uwep->oclass != ARMOR_CLASS) {
            old_erodeproof = (uwep->oerodeproof != 0);
            new_erodeproof = !sobj_cursed;
            uwep->oerodeproof = 0; /* for messages */
            if (Blind) {
                uwep->rknown = sobj->bknown;
                Your("weapon feels warm for a moment.");
            } else {
                uwep->rknown = TRUE;
                pline("%s covered by a %s %s %s!",
                      Yobjnam2(uwep, "are"),
                      sobj_cursed ? "mottled" : "shimmering",
                      hcolor(sobj_cursed ? NH_PURPLE : NH_GOLDEN),
                      sobj_cursed ? "glow" : "shield");
            }
            if (new_erodeproof && (uwep->oeroded || uwep->oeroded2)) {
                uwep->oeroded = uwep->oeroded2 = 0;
                pline("%s as good as new!",
                      Yobjnam2(uwep, Blind ? "feel" : "look"));
            }
            if (old_erodeproof && !new_erodeproof) {
                /* restore old_erodeproof before shop charges */
                uwep->oerodeproof = 1;
                costly_alteration(uwep, COST_DEGRD);
            }
            uwep->oerodeproof = new_erodeproof ? 1 : 0;
            break;
        }
        if (!chwepon(sobj, sobj_cursed ? -1 :
                           !uwep ? 1 :
                           uwep->spe >= 9 ? (rn2(uwep->spe) == 0) :
                           sobj_blessed ? rnd(3-uwep->spe/3) : 1)) {
            sobj = 0; /* nothing enchanted: strange_feeling -> useup */
        }
        break;

    case SCR_TAMING:
    case SPE_CHARM_MONSTER: {
        int candidates, res, results, vis_results;

        if (u.uswallow) {
            candidates = 1;
            results = vis_results = maybe_tame(u.ustuck, sobj);
        } else {
            int i, j, bd = confused ? 5 : 1;
            struct monst *mtmp;

            /* note: maybe_tame() can return either positive or
               negative values, but not both for the same scroll */
            candidates = results = vis_results = 0;
            for (i = -bd; i <= bd; i++) {
                for (j = -bd; j <= bd; j++) {
                    if (!isok(u.ux + i, u.uy + j)) {
                        continue;
                    }
                    if ((mtmp = m_at(u.ux + i, u.uy + j)) ||
                        (!i && !j && (mtmp = u.usteed))) {
                        ++candidates;
                        res = maybe_tame(mtmp, sobj);
                        results += res;
                        if (canspotmon(mtmp)) {
                            vis_results += res;
                        }
                    }
                }
            }
        }
        if (!results) {
            pline("Nothing interesting %s.", !candidates ? "happens" : "seems to happen");
        } else {
            pline_The("neighborhood %s %sfriendlier.",
                      vis_results ? "is" : "seems",
                      (results < 0) ? "un" : "");
            if (vis_results > 0) {
                known = TRUE;
            }
        }
        break;
    }

    case SCR_FLOOD:
        if (confused) {
            /* remove water from vicinity of player */
            int maderoom = 0;
            do_clear_area(u.ux, u.uy, 4+2*bcsign(sobj),
                          undo_flood, (genericptr_t)&maderoom);
            if (maderoom) {
                known = TRUE;
                You("are suddenly very dry!");
            }
        } else {
            int madepool = 0;
            int stilldry = -1;
            int x, y, safe_pos=0;
            if (!sobj_cursed) {
                do_clear_area(u.ux, u.uy, 5, do_flood,
                              (genericptr_t)&madepool);
            }

            /* check if there are safe tiles around the player */
            for (x = u.ux-1; x <= u.ux+1; x++) {
                for (y = u.uy - 1; y <= u.uy + 1; y++) {
                    if (x != u.ux && y != u.uy &&
                        goodpos(x, y, &youmonst, 0)) {
                        safe_pos++;
                    }
                }
            }

            /* cursed and uncursed might put a water tile on
             * player's position */
            if (!sobj_blessed && safe_pos > 0) {
                do_flood(u.ux, u.uy, (genericptr_t)&stilldry);
            }
            if (!madepool && stilldry)
                break;
            if (madepool)
                pline(Hallucination ?
                      "A totally gnarly wave comes in!" :
                      "A flood surges through the area!" );
            if (!stilldry && !Wwalking && !Flying && !Levitation)
                drown();
            known = TRUE;
            break;
        }
        break;

    case SCR_GENOCIDE:
        if (!already_known) {
            You("have found a scroll of genocide!");
        }
        known = TRUE;
        do_genocide((!sobj_cursed) | (2 * !!Confusion), !sobj_blessed);
        break;

    case SCR_LIGHT:
        if (!confused) {
            if (!Blind) known = TRUE;
            litroom(!sobj_cursed, sobj);
        } else {
            /* confused reading summons lights */
            int i;
            if (sobj_cursed) {
                for (i = 0; i < rn1(6, 5); i++)
                    makemon(&mons[PM_BLACK_LIGHT], 0, 0, NO_MM_FLAGS);
            } else {
                for (i = 0; i < rnd(15); i++)
                    makemon(&mons[PM_YELLOW_LIGHT], 0, 0, NO_MM_FLAGS);
            }
        }
        break;

    case SCR_TELEPORTATION:
        if (confused || sobj_cursed) {
            level_tele();
        } else {
            if (sobj_blessed && !Teleport_control) {
                known = TRUE;
                if (yn("Do you wish to teleport?")=='n')
                    break;
            }
            tele();
            if (Teleport_control || !couldsee(u.ux0, u.uy0) ||
                (distu(u.ux0, u.uy0) >= 16))
                known = TRUE;
        }
        break;

    case SCR_GOLD_DETECTION:
        if (confused) {
            /* detect random item class */
            int random_classes[] = { WEAPON_CLASS, ARMOR_CLASS, RING_CLASS, AMULET_CLASS,
                                     TOOL_CLASS, FOOD_CLASS, POTION_CLASS, SCROLL_CLASS, SPBOOK_CLASS,
                                     WAND_CLASS, COIN_CLASS, GEM_CLASS };
            /* We don't want detection behavior depending on the scroll's properties, so object_detect() is
             * called with a NULL pointer. But then we need to call strange_feeling() manually. */
            if (object_detect((struct obj *)0, random_classes[rn2(SIZE(random_classes))], FALSE)) {
                strange_feeling(sobj, (char *) 0);
                sobj = 0; /* nothing detected: strange_feeling -> useup */
            }
        } else if (sobj_cursed) {
            if (trap_detect(sobj)) {
                sobj = 0; /* nothing detected: strange_feeling -> useup */
            }
        } else {
            if (gold_detect(sobj)) {
                sobj = 0; /* nothing detected: strange_feeling -> useup */
            }
        }
        break;

    case SCR_FOOD_DETECTION:
    case SPE_DETECT_FOOD:
        if (food_detect(sobj)) {
            sobj = 0; /* nothing detected: strange_feeling -> useup */
        }
        break;

    case SCR_IDENTIFY:
        /* known = TRUE; -- handled inline here */
        /* use up the scroll first, before makeknown() performs a
           perm_invent update; also simplifies empty invent check */
        useup(sobj);
        sobj = 0; /* it's gone */
        if (confused) {
            You("identify this as an identify scroll.");
        } else if (!already_known || !invent) {
            /* force feedback now if invent became
               empty after using up this scroll */
            pline("This is an identify scroll.");
        }
        if (!already_known) {
            (void) learn_scroll_typ(SCR_IDENTIFY, FALSE);
        }
        /* fall through */
    case SPE_IDENTIFY:
        cval = 1;
        if (sobj_blessed || (!sobj_cursed && !rn2(5))) {
            cval = rn2(5);
            /* note: if cval==0, identify all items */
            if (cval == 1 && sobj_blessed && Luck > 0) {
                ++cval;
            }
        } else {
            cval = 1;
        }
        if (invent && !confused) {
            identify_pack(cval, !already_known);
        } else if (otyp == SPE_IDENTIFY) {
            /* when casting a spell we know we're not confused,
               so inventory must be empty (another message has
               already been given above if reading a scroll) */
            pline("You're not carrying anything to be identified.");
        }
        break;

    case SCR_CHARGING:
        if (confused) {
            if (sobj_cursed) {
                You_feel("discharged.");
                u.uen = 0;
            } else {
                You_feel("charged up!");
                u.uen += d(sobj_blessed ? 6 : 4, 4);
                if (u.uen > u.uenmax) {
                    /* if current energy is already at   */
                    /* or near maximum, increase maximum */
                    u.uenmax = u.uen;
                } else {
                    u.uen = u.uenmax; /* otherwise restore current to max  */
                }
            }
            flags.botl = 1;
            break;
        }
        /* known = TRUE; -- handled inline here */
        if (!already_known) {
            pline("This is a charging scroll.");
            learn_scroll(sobj);
        }
        /* use it up now to prevent it from showing in the
           getobj picklist because the "disappears" message
           was already delivered */
        useup(sobj);
        sobj = 0; /* it's gone */
        otmp = getobj(all_count, "charge");
        if (!otmp) break;
        recharge(otmp, sobj_cursed ? -1 : (sobj_blessed ? 1 : 0));
        break;

    case SCR_MAGIC_MAPPING:
        if (level.flags.nommap) {
            Your("mind is filled with crazy lines!");
            if (Hallucination)
                pline("Wow!  Modern art.");
            else
                Your("%s spins in bewilderment.", body_part(HEAD));
            make_confused(HConfusion + rnd(30), FALSE);
            break;
        }
        /* reveal secret doors for uncursed and blessed scrolls */
        if (!sobj_cursed) {
            register int x, y;

            for (x = 1; x < COLNO; x++)
                for (y = 0; y < ROWNO; y++)
                    if (levl[x][y].typ == SDOOR)
                        cvt_sdoor_to_door(&levl[x][y]);
            /* do_mapping() already reveals secret passages */
        }
        known = TRUE;
        /* fall through */

case SPE_MAGIC_MAPPING:
        if (level.flags.nommap) {
            Your("%s spins as %s blocks the spell!", body_part(HEAD), something);
            make_confused(HConfusion + rnd(30), FALSE);
            break;
        }
        pline("A map coalesces in your mind!");
        cval = (sobj_cursed && !confused);
        if(cval) HConfusion = 1;    /* to screw up map */
        do_mapping();
        /* objects, too, pal! */
        if (sobj_blessed && !cval) {
            object_detect(sobj, 0, TRUE);
        }
        if (cval) {
            HConfusion = 0; /* restore */
            pline("Unfortunately, you can't grasp the details.");
        }
        break;

#ifdef SCR_AMNESIA
    case SCR_AMNESIA:
        known = TRUE;
        forget( (!sobj_blessed ? ALL_SPELLS : 0) |
                (!confused || sobj_cursed ? ALL_MAP : 0) );
        if (Hallucination) /* Ommmmmm! */
            Your("mind releases itself from mundane concerns.");
        else if (!strncmpi(plname, "Maud", 4))
            pline("As your mind turns inward on itself, you forget everything else.");
        else if (rn2(2))
            pline("Who was that Maud person anyway?");
        else
            pline("Thinking of Maud you forget everything else.");
        exercise(A_WIS, FALSE);
        break;
#endif

    case SCR_FIRE:

        cval = bcsign(sobj);
        int dam = (2 * (rn1(3, 3) + 2 * cval) + 1) / 3;
        coord cc;
        cc.x = u.ux;
        cc.y = u.uy;
        useup(sobj);
        sobj = 0; /* it's gone */
        if (!already_known) {
            (void) learn_scroll_typ(SCR_FIRE, FALSE);
        }
        if (confused) {
            if (Fire_resistance) {
                shieldeff(u.ux, u.uy);
                if (!Blind) {
                    pline("Oh, look, what a pretty fire in your %s.",
                          makeplural(body_part(HAND)));
                } else {
                    You_feel("a pleasant warmth in your %s.",
                             makeplural(body_part(HAND)));
                }
            } else {
                pline_The("scroll catches fire and you burn your %s.",
                          makeplural(body_part(HAND)));
                losehp(1, "scroll of fire", KILLED_BY_AN);
            }
            break;
        }
        if (Underwater) {
            pline_The("%s around you vaporizes violently!", hliquid("water"));
        } else {
            if (sobj_blessed) {
                if (!already_known) {
                    pline("This is a scroll of fire!");
                }
                dam *= 5;
                pline("Where do you want to center the explosion?");
                getpos_sethilite(display_stinking_cloud_positions,
                                 get_valid_stinking_cloud_pos);
                (void) getpos(&cc, TRUE, "the desired position");
                if (!is_valid_stinking_cloud_pos(cc.x, cc.y, FALSE)) {
                    /* try to reach too far, get burned */
                    cc.x = u.ux;
                    cc.y = u.uy;
                }
            }
            if (cc.x == u.ux && cc.y == u.uy) {
                pline_The("scroll erupts in a tower of flame!");
                iflags.last_msg = PLNMSG_TOWER_OF_FLAME; /* for explode() */
                burn_away_slime();
            }
        }
        explode(cc.x, cc.y, 11, dam, SCROLL_CLASS, EXPL_FIERY);
        break;

    case SCR_EARTH:
        /* TODO: handle steeds */
        if (
#ifdef REINCARNATION
            !Is_rogue_level(&u.uz) &&
#endif
            (!In_endgame(&u.uz) || Is_earthlevel(&u.uz))) {
            register int x, y;
            int boulder_created = 0;

            /* Identify the scroll */
            if (u.uswallow) {
                You_hear("rumbling.");
            } else {
                pline_The("%s rumbles %s you!", ceiling(u.ux, u.uy),
                      sobj_blessed ? "around" : "above");
            }
            known = 1;
            if (In_sokoban(&u.uz))
                sokoban_trickster();    /* Sokoban guilt */

            /* Loop through the surrounding squares */
            if (!sobj_cursed) for (x = u.ux-1; x <= u.ux+1; x++) {
                    for (y = u.uy-1; y <= u.uy+1; y++) {
                        /* Is this a suitable spot? */
                        if (isok(x, y) && !closed_door(x, y) &&
                            !IS_ROCK(levl[x][y].typ) &&
                            !IS_AIR(levl[x][y].typ) &&
                            (x != u.ux || y != u.uy)) {
                            boulder_created += drop_boulder_on_monster(x, y, confused, TRUE);
                        }
                    }
                }
            /* Attack the player */
            if (!sobj_blessed) {
                drop_boulder_on_player(confused, !sobj_cursed, TRUE, FALSE);
            } else {
                if (boulder_created == 0)
                    pline("But nothing else happens.");
            }
        } else if (In_endgame(&u.uz)) {
            You_hear("the %s rumbling below you!", surface(u.ux, u.uy));
        }
        break;

    case SCR_PUNISHMENT:
        known = TRUE;
        if (sobj_blessed) {
            You_feel("guilty.");
            break;
        } else if (confused) {
            makemon(&mons[PM_PUNISHER], 0, 0, NO_MM_FLAGS);
            break;
        }
        punish(sobj);
        break;

    case SCR_STINKING_CLOUD: {
        coord cc;

        if (!already_known) {
            You("have found a scroll of stinking cloud!");
        }
        known = TRUE;
        pline("Where do you want to center the %scloud?", already_known ? "stinking " : "");
        cc.x = u.ux;
        cc.y = u.uy;
        getpos_sethilite(display_stinking_cloud_positions,
                         get_valid_stinking_cloud_pos);
        if (getpos(&cc, TRUE, "the desired position") < 0) {
            pline("%s", Never_mind);
            break;
        }
        if (!confused) {
            if (!cansee(cc.x, cc.y) || distu(cc.x, cc.y) >= 32) {
                You("smell rotten eggs.");
            } else {
                (void) create_gas_cloud(cc.x, cc.y, 3+bcsign(sobj),
                                        8+4*bcsign(sobj), rn1(3, 4));
            }
        } else {
            int i;
            if (!sobj_blessed) {
                for (i = 0; i < 30 + rn2(70); i++)
                    makemon(&mons[PM_GAS_SPORE], 0, 0, NO_MM_FLAGS);
            } else {
                for (i = 0; i < rnd(3); i++)
                    makemon(&mons[PM_FERN_SPORE], 0, 0, NO_MM_FLAGS);
            }
        }
        break;
    }
    default:
        impossible("What weird effect is this? (%u)", sobj->otyp);
    }
    /* if sobj is gone, we've already called useup() above and the
       update_inventory() that it performs might have come too soon
       (before charging an item, for instance) */
    if (!sobj) {
        update_inventory();
    }
    return sobj ? 0 : 1;
}

static void
wand_explode(obj)
register struct obj *obj;
{
    obj->in_use = TRUE; /* in case losehp() is fatal */
    Your("%s vibrates violently, and explodes!", xname(obj));
    nhbell();
    losehp(rnd(2*(u.uhpmax+1)/3), "exploding wand", KILLED_BY_AN);
    useup(obj);
    exercise(A_STR, FALSE);
}

/* used to collect gremlins being hit by light so that they can be processed
   after vision for the entire lit area has been brought up to date */
struct litmon {
    struct monst *mon;
    struct litmon *nxt;
};
static struct litmon *gremlins = 0;

/*
 * Low-level lit-field update routine.
 */
STATIC_PTR void
set_lit(x, y, val)
int x, y;
genericptr_t val;
{
    struct monst *mtmp;
    struct litmon *gremlin;

    if (val) {
        levl[x][y].lit = 1;
        if ((mtmp = m_at(x, y)) != 0 && mtmp->data == &mons[PM_GREMLIN]) {
            gremlin = (struct litmon *) alloc(sizeof *gremlin);
            gremlin->mon = mtmp;
            gremlin->nxt = gremlins;
            gremlins = gremlin;
        }
    } else {
        levl[x][y].lit = 0;
        snuff_light_source(x, y);
    }
}

void
litroom(on, obj)
register boolean on;
struct obj *obj;
{
    char is_lit;    /* value is irrelevant; we use its address
                       as a `not null' flag for set_lit() */

    /* first produce the text (provided you're not blind) */
    if(!on) {
        register struct obj *otmp;

        if (!Blind) {
            if(u.uswallow) {
                pline("It seems even darker in here than before.");
                return;
            }
            if (uwep && artifact_light(uwep) && uwep->lamplit)
                pline("Suddenly, the only light left comes from %s!",
                      the(xname(uwep)));
            else
                You("are surrounded by darkness!");
        }

        /* the magic douses lamps, et al, too */
        for(otmp = invent; otmp; otmp = otmp->nobj)
            if (otmp->lamplit)
                (void) snuff_lit(otmp);
        if (Blind) goto do_it;
    } else {
        if (Blind) goto do_it;
        if(u.uswallow) {
            if (is_animal(u.ustuck->data))
                pline("%s %s is lit.",
                      s_suffix(Monnam(u.ustuck)),
                      mbodypart(u.ustuck, STOMACH));
            else
            if (is_whirly(u.ustuck->data))
                pline("%s shines briefly.",
                      Monnam(u.ustuck));
            else
                pline("%s glistens.", Monnam(u.ustuck));
            return;
        }
        pline("A lit field surrounds you!");
    }

do_it:
    /* No-op in water - can only see the adjacent squares and that's it! */
    if (Underwater || Is_waterlevel(&u.uz)) return;
    /*
     *  If we are darkening the room and the hero is punished but not
     *  blind, then we have to pick up and replace the ball and chain so
     *  that we don't remember them if they are out of sight.
     */
    if (Punished && !on && !Blind)
        move_bc(1, 0, uball->ox, uball->oy, uchain->ox, uchain->oy);

#ifdef REINCARNATION
    if (Is_rogue_level(&u.uz)) {
        /* Can't use do_clear_area because MAX_RADIUS is too small */
        /* rogue lighting must light the entire room */
        int rnum = levl[u.ux][u.uy].roomno - ROOMOFFSET;
        int rx, ry;
        if(rnum >= 0) {
            for(rx = rooms[rnum].lx-1; rx <= rooms[rnum].hx+1; rx++)
                for(ry = rooms[rnum].ly-1; ry <= rooms[rnum].hy+1; ry++)
                    set_lit(rx, ry,
                            (genericptr_t)(on ? &is_lit : (char *)0));
            rooms[rnum].rlit = on;
        }
        /* hallways remain dark on the rogue level */
    } else
#endif
    do_clear_area(u.ux, u.uy,
                  (obj && obj->oclass==SCROLL_CLASS && obj->blessed) ? 9 : 5,
                  set_lit, (genericptr_t)(on ? &is_lit : (char *)0));

    /*
     *  If we are not blind, then force a redraw on all positions in sight
     *  by temporarily blinding the hero.  The vision recalculation will
     *  correctly update all previously seen positions *and* correctly
     *  set the waslit bit [could be messed up from above].
     */
    if (!Blind) {
        vision_recalc(2);

        /* replace ball&chain */
        if (Punished && !on)
            move_bc(0, 0, uball->ox, uball->oy, uchain->ox, uchain->oy);
    }

    vision_full_recalc = 1; /* delayed vision recalculation */
    if (gremlins) {
        struct litmon *gremlin;

        /* can't delay vision recalc after all */
        vision_recalc(0);
        /* after vision has been updated, monsters who are affected
           when hit by light can now be hit by it */
        do {
            gremlin = gremlins;
            gremlins = gremlin->nxt;
            light_hits_gremlin(gremlin->mon, rnd(5));
            free(gremlin);
        } while (gremlins);
    }
}

#if 0
static void
do_class_genocide()
{
    int i, j, immunecnt, gonecnt, goodcnt, class, feel_dead = 0;
    char buf[BUFSZ];
    boolean gameover = FALSE;   /* true iff killed self */

    for(j=0; ; j++) {
        if (j >= 5) {
            pline("%s", thats_enough_tries);
            return;
        }
        do {
            getlin("What class of monsters do you wish to genocide?",
                   buf);
            (void)mungspaces(buf);
        } while (buf[0]=='\033' || !buf[0]);
        /* choosing "none" preserves genocideless conduct */
        if (!strcmpi(buf, "none") ||
            !strcmpi(buf, "nothing")) return;

        if (strlen(buf) == 1) {
            if (buf[0] == ILLOBJ_SYM)
                buf[0] = def_monsyms[S_MIMIC];
            class = def_char_to_monclass(buf[0]);
        } else {
            char buf2[BUFSZ];

            class = 0;
            Strcpy(buf2, makesingular(buf));
            Strcpy(buf, buf2);
        }
        immunecnt = gonecnt = goodcnt = 0;
        for (i = LOW_PM; i < NUMMONS; i++) {
            if (class == 0 &&
                strstri(monexplain[(int)mons[i].mlet], buf) != 0)
                class = mons[i].mlet;
            if (mons[i].mlet == class) {
                if (!(mons[i].geno & G_GENO)) immunecnt++;
                else if(mvitals[i].mvflags & G_GENOD) gonecnt++;
                else goodcnt++;
            }
        }
        /*
         * TODO[?]: If user's input doesn't match any class
         *      description, check individual species names.
         */
        if (!goodcnt && class != mons[urole.malenum].mlet &&
            class != mons[urace.malenum].mlet) {
            if (gonecnt)
                pline("All such monsters are already nonexistent.");
            else if (immunecnt ||
                     (buf[0] == DEF_INVISIBLE && buf[1] == '\0'))
                You("aren't permitted to genocide such monsters.");
            else
#ifdef WIZARD   /* to aid in topology testing; remove pesky monsters */
            if (wizard && buf[0] == '*') {
                register struct monst *mtmp, *mtmp2;

                gonecnt = 0;
                for (mtmp = fmon; mtmp; mtmp = mtmp2) {
                    mtmp2 = mtmp->nmon;
                    if (DEADMONSTER(mtmp)) continue;
                    mongone(mtmp);
                    gonecnt++;
                }
                pline("Eliminated %d monster%s.", gonecnt, plur(gonecnt));
                return;
            } else
#endif
                pline("That symbol does not represent any monster.");
            continue;
        }

        for (i = LOW_PM; i < NUMMONS; i++) {
            if(mons[i].mlet == class) {
                char nam[BUFSZ];

                Strcpy(nam, makeplural(mons[i].mname));
                /* Although "genus" is Latin for race, the hero benefits
                 * from both race and role; thus genocide affects either.
                 */
                if (Your_Own_Role(i) || Your_Own_Race(i) ||
                    ((mons[i].geno & G_GENO)
                     && !(mvitals[i].mvflags & G_GENOD))) {
                    /* This check must be first since player monsters might
                     * have G_GENOD or !G_GENO.
                     */
                    mvitals[i].mvflags |= (G_GENOD|G_NOCORPSE);
                    reset_rndmonst(i);
                    kill_genocided_monsters();
                    update_inventory(); /* eggs & tins */
                    pline("Wiped out all %s.", nam);
                    if (Upolyd && i == u.umonnum) {
                        u.mh = -1;
                        if (Unchanging) {
                            if (!feel_dead++) You("die.");
                            /* finish genociding this class of
                               monsters before ultimately dying */
                            gameover = TRUE;
                        } else
                            rehumanize();
                    }
                    /* Self-genocide if it matches either your race
                       or role.  Assumption:  male and female forms
                       share same monster class. */
                    if (i == urole.malenum || i == urace.malenum) {
                        u.uhp = -1;
                        if (Upolyd) {
                            if (!feel_dead++) {
                                You_feel("%s inside.", udeadinside());
                            }
                        } else {
                            if (!feel_dead++) You("die.");
                            gameover = TRUE;
                        }
                    }
                } else if (mvitals[i].mvflags & G_GENOD) {
                    if (!gameover)
                        pline("All %s are already nonexistent.", nam);
                } else if (!gameover) {
                    /* suppress feedback about quest beings except
                       for those applicable to our own role */
                    if ((mons[i].msound != MS_LEADER ||
                         quest_info(MS_LEADER) == i)
                        && (mons[i].msound != MS_NEMESIS ||
                            quest_info(MS_NEMESIS) == i)
                        && (mons[i].msound != MS_GUARDIAN ||
                            quest_info(MS_GUARDIAN) == i)
                        /* non-leader/nemesis/guardian role-specific monster */
                        && (i != PM_NINJA || /* nuisance */
                            Role_if(PM_SAMURAI))) {
                        boolean named, uniq;

                        named = type_is_pname(&mons[i]) ? TRUE : FALSE;
                        uniq = (mons[i].geno & G_UNIQ) ? TRUE : FALSE;
                        /* one special case */
                        if (i == PM_HIGH_PRIEST) uniq = FALSE;

                        You("aren't permitted to genocide %s%s.",
                            (uniq && !named) ? "the " : "",
                            (uniq || named) ? mons[i].mname : nam);
                    }
                }
            }
        }
        if (gameover || u.uhp == -1) {
            killer.format = KILLED_BY_AN;
            Strcpy(killer.name, "scroll of genocide");
            if (gameover) done(GENOCIDED);
        }
        return;
    }
}
#endif

#define REALLY 1
#define PLAYER 2
#define ONTHRONE 4
void
do_genocide(how, only_on_level)
int how;
boolean only_on_level; /**< if TRUE only genocide monsters on current level,
                            not in the complete dungeon */
/* 0 = no genocide; create monsters (cursed scroll) */
/* 1 = normal genocide */
/* 3 = forced genocide of player */
/* 5 (4 | 1) = normal genocide from throne */
{
    char buf[BUFSZ];
    register int i, killplayer = 0;
    register int mndx;
    register struct permonst *ptr;
    const char *which;
    const char *on_this_level;

    if (how & PLAYER) {
        mndx = u.umonster;  /* non-polymorphed mon num */
        ptr = &mons[mndx];
        Strcpy(buf, ptr->mname);
        killplayer++;
    } else {
        for(i = 0; ; i++) {
            if (i >= 5) {
                /* cursed effect => no free pass (unless rndmonst() fails) */
                if (!(how & REALLY) && (ptr = rndmonst()) != 0) {
                    break;
                }
                pline("%s", thats_enough_tries);
                return;
            }
            getlin("What monster do you want to genocide? [type the name]",
                   buf);
            (void)mungspaces(buf);
            /* choosing "none" preserves genocideless conduct */
            if (*buf == '\033' || !strcmpi(buf, "none") || !strcmpi(buf, "nothing")) {
                /* ... but no free pass if cursed */
                if (!(how & REALLY) && (ptr = rndmonst()) != 0) {
                    break; /* remaining checks don't apply */
                }
                return;
            }

#ifdef WIZARD   /* to aid in topology testing; remove pesky monsters */
            /* copy from do_class_genocide */
            if (wizard && buf[0] == '*') {
                register struct monst *mtmp, *mtmp2;

                int gonecnt = 0;
                for (mtmp = fmon; mtmp; mtmp = mtmp2) {
                    mtmp2 = mtmp->nmon;
                    if (DEADMONSTER(mtmp)) continue;
                    mongone(mtmp);
                    gonecnt++;
                }
                pline("Eliminated %d monster%s.", gonecnt, plur(gonecnt));
                return;
            }
#endif
            mndx = name_to_mon(buf);
            if (mndx == NON_PM || (mvitals[mndx].mvflags & G_GENOD)) {
                pline("Such creatures %s exist in this world.",
                      (mndx == NON_PM) ? "do not" : "no longer");
                continue;
            }
            ptr = &mons[mndx];
            /* Although "genus" is Latin for race, the hero benefits
             * from both race and role; thus genocide affects either.
             */
            if (Your_Own_Role(mndx) || Your_Own_Race(mndx)) {
                killplayer++;
                break;
            }
            if (is_human(ptr)) adjalign(-sgn(u.ualign.type));
            if (is_demon(ptr)) adjalign(sgn(u.ualign.type));

            if (!(ptr->geno & G_GENO)) {
                if (!Deaf) {
                    /* FIXME: unconditional "caverns" will be silly in some
                     * circumstances.  Who's speaking?  Divine pronouncements
                     * aren't supposed to be hampered by deafness....
                     */
                    if(flags.verbose)
                        pline("A thunderous voice booms through the caverns:");
                    verbalize("No, mortal!  That will not be done.");
                }
                continue;
            }
            /* KMH -- Unchanging prevents rehumanization */
            if (Unchanging && ptr == youmonst.data)
                killplayer++;
            break;
        }
        mndx = monsndx(ptr); /* needed for the 'no free pass' cases */
    }

    on_this_level = only_on_level ? " on this level" : "";
    which = "all ";
    if (Hallucination) {
        if (Upolyd)
            Strcpy(buf, youmonst.data->mname);
        else {
            Strcpy(buf, (flags.female && urole.name.f) ?
                   urole.name.f : urole.name.m);
            buf[0] = lowc(buf[0]);
        }
    } else {
        Strcpy(buf, ptr->mname); /* make sure we have standard singular */
        if ((ptr->geno & G_UNIQ) && ptr != &mons[PM_HIGH_PRIEST])
            which = !type_is_pname(ptr) ? "the " : "";
    }
    if (how & REALLY) {
        char* genocided_name = (*which != 'a') ? buf : makeplural(buf);
#ifdef LIVELOGFILE
        livelog_genocide(genocided_name, only_on_level);
#endif
        /* setting no-corpse affects wishing and random tin generation */
        if (!only_on_level) { mvitals[mndx].mvflags |= (G_GENOD | G_NOCORPSE); }
        pline("Wiped out %s%s%s.", which,
              genocided_name,
              on_this_level);
        if (!only_on_level && mons[mndx].mlet == S_TROLL) {
            pline("*plonk*");
        }

        if (killplayer) {
            /* might need to wipe out dual role */
            if (!only_on_level) {
                if (urole.femalenum != NON_PM && mndx == urole.malenum)
                    mvitals[urole.femalenum].mvflags |= (G_GENOD | G_NOCORPSE);
                if (urole.femalenum != NON_PM && mndx == urole.femalenum)
                    mvitals[urole.malenum].mvflags |= (G_GENOD | G_NOCORPSE);
                if (urace.femalenum != NON_PM && mndx == urace.malenum)
                    mvitals[urace.femalenum].mvflags |= (G_GENOD | G_NOCORPSE);
                if (urace.femalenum != NON_PM && mndx == urace.femalenum)
                    mvitals[urace.malenum].mvflags |= (G_GENOD | G_NOCORPSE);
            }

            u.uhp = -1;
            if (how & PLAYER) {
                killer.format = KILLED_BY;
                Strcpy(killer.name, "genocidal confusion");
            } else if (how & ONTHRONE) {
                /* player selected while on a throne */
                killer.format = KILLED_BY_AN;
                Strcpy(killer.name, "imperious order");
            } else { /* selected player deliberately, not confused */
                killer.format = KILLED_BY_AN;
                Strcpy(killer.name, "scroll of genocide");
            }

            /* Polymorphed characters will die as soon as they're rehumanized. */
            /* KMH -- Unchanging prevents rehumanization */
            if (Upolyd && ptr != youmonst.data) {
                delayed_killer(POLYMORPH, killer.format, killer.name);
                You_feel("%s inside.", udeadinside());
            } else
                done(GENOCIDED);
        } else if (ptr == youmonst.data) {
            rehumanize();
        }

        if (only_on_level) {
            kill_monster_on_level(mndx);
        } else {
            reset_rndmonst(mndx);
            kill_genocided_monsters();
            update_inventory(); /* in case identified eggs were affected */
        }
    } else {
        int cnt = 0, census = monster_census(FALSE);

        if (!(mons[mndx].geno & G_UNIQ) &&
            !(mvitals[mndx].mvflags & (G_GENOD | G_EXTINCT)))
            for (i = rn1(3, 4); i > 0; i--) {
                if (!makemon(ptr, u.ux, u.uy, NO_MINVENT))
                    break; /* couldn't make one */
                ++cnt;
                if (mvitals[mndx].mvflags & G_EXTINCT)
                    break; /* just made last one */
            }
        if (cnt) {
            /* accumulated 'cnt' doesn't take groups into account;
               assume bringing in new mon(s) didn't remove any old ones */
            cnt = monster_census(FALSE) - census;
            if (!Hallucination && mons[mndx].mlet == S_TROLL) {
                pline("S3n7 1n %s!!!",
                      (cnt > 1) ? "s0m3 7r0llz" : "4 7r0ll");
            } else {
                pline("Sent in %s%s.",
                      (cnt > 1) ? "some " : "",
                      (cnt > 1) ? makeplural(buf) : an(buf));
            }
        } else {
            pline("%s", nothing_happens);
        }
    }
}

void
punish(sobj)
register struct obj *sobj;
{
#ifdef CONVICT
    struct obj *otmp;
#endif /* CONVICT */
    struct obj *reuse_ball = (sobj && sobj->otyp == HEAVY_IRON_BALL) ? sobj : (struct obj *) 0;

    /* KMH -- Punishment is still okay when you are riding */
    if (!reuse_ball) {
        You("are being punished for your misbehavior!");
    }
    if (Punished) {
        Your("iron ball gets heavier.");
        uball->owt += IRON_BALL_W_INCR * (1 + (sobj ? sobj->cursed : 0));
        return;
    }
    if (amorphous(youmonst.data) || is_whirly(youmonst.data) || unsolid(youmonst.data)) {
        if (!reuse_ball) {
            pline("A ball and chain appears, then falls away.");
            dropy(mkobj(BALL_CLASS, TRUE));
        } else {
            dropy(reuse_ball);
        }
        return;
    }
    setworn(mkobj(CHAIN_CLASS, TRUE), W_CHAIN);
#ifdef CONVICT
    if (((otmp = carrying(HEAVY_IRON_BALL)) != 0) &&(otmp->oartifact ==
                                                     ART_IRON_BALL_OF_LIBERATION)) {
        setworn(otmp, W_BALL);
        Your("%s chains itself to you!", xname(otmp));
    } else {
#endif
        if (!reuse_ball) {
            setworn(mkobj(BALL_CLASS, TRUE), W_BALL);
        } else {
            setworn(reuse_ball, W_BALL);
        }
#ifdef CONVICT
    }
#endif /* CONVICT */
    uball->spe = 1;     /* special ball (see save) */

    /*
     *  Place ball & chain if not swallowed.  If swallowed, the ball &
     *  chain variables will be set at the next call to placebc().
     */
    if (!u.uswallow) {
        placebc();
        if (Blind) set_bc(1);   /* set up ball and chain variables */
        newsym(u.ux, u.uy);      /* see ball&chain if can't see self */
    }
}

/* remove the ball and chain */
void
unpunish()
{
    struct obj *savechain = uchain;

    /* chain goes away */
    obj_extract_self(uchain);
    newsym(uchain->ox, uchain->oy);
    setworn((struct obj *) 0, W_CHAIN); /* sets 'uchain' to Null */
    dealloc_obj(savechain);
    /* ball persists */
    uball->spe = 0;
    setworn((struct obj *) 0, W_BALL); /* sets 'uball' to Null */
}

/* some creatures have special data structures that only make sense in their
 * normal locations -- if the player tries to create one elsewhere, or to revive
 * one, the disoriented creature becomes a zombie
 */
boolean
cant_revive(mtype, revival, from_obj)
int *mtype;
boolean revival;
struct obj *from_obj;
{
    /* SHOPKEEPERS can be revived now */
    if ((*mtype == PM_GUARD) ||
        (*mtype == PM_SHOPKEEPER && !revival) ||
        (*mtype == PM_HIGH_PRIEST) ||
        (*mtype == PM_ALIGNED_PRIEST) ||
        (*mtype == PM_ANGEL)) {
        *mtype = PM_HUMAN_ZOMBIE;
        return TRUE;
    } else if (*mtype == PM_LONG_WORM_TAIL) { /* for create_particular() */
        *mtype = PM_LONG_WORM;
        return TRUE;
    } else if (unique_corpstat(&mons[*mtype])
               && (!from_obj || !has_omonst(from_obj))) {
        /* unique corpses (from bones or wizard mode wish) or
           statues (bones or any wish) end up as shapechangers */
        *mtype = PM_DOPPELGANGER;
        return TRUE;
    }
    return FALSE;
}

#ifdef WIZARD
struct _create_particular_data {
    int quan;
    int which;
    int fem;
    char monclass;
    boolean randmonst;
    boolean maketame, makepeaceful, makehostile;
    boolean sleeping, saddled, invisible, hidden;
};

static boolean
create_particular_parse(str, d)
char *str;
struct _create_particular_data *d;
{
    char *bufp = str;
    char *tmpp;

    d->quan = 1 + ((multi > 0) ? multi : 0);
    d->monclass = MAXMCLASSES;
    d->which = urole.malenum; /* an arbitrary index into mons[] */
    d->fem = -1; /* gender not specified */
    d->randmonst = FALSE;
    d->maketame = d->makepeaceful = d->makehostile = FALSE;
    d->sleeping = d->saddled = d->invisible = d->hidden = FALSE;

    /* quantity */
    if (digit(*bufp)) {
        d->quan = atoi(bufp);
        while (digit(*bufp)) {
            bufp++;
        }
        while (*bufp == ' ') {
            bufp++;
        }
    }
#define QUAN_LIMIT (ROWNO * (COLNO - 1))
    /* maximum possible quantity is one per cell: (0..ROWNO-1) x (1..COLNO-1)
       [21*79==1659 for default map size; could subtract 1 for hero's spot] */
    if (d->quan < 1 || d->quan > QUAN_LIMIT) {
        d->quan = QUAN_LIMIT - monster_census(FALSE);
    }
#undef QUAN_LIMIT
    /* gear -- extremely limited number of possibilities supported */
    if ((tmpp = strstri(bufp, "saddled ")) != 0) {
        d->saddled = TRUE;
        (void) memset(tmpp, ' ', sizeof "saddled " - 1);
    }
    /* state -- limited number of possibilitie supported */
    if ((tmpp = strstri(bufp, "sleeping ")) != 0) {
        d->sleeping = TRUE;
        (void) memset(tmpp, ' ', sizeof "sleeping " - 1);
    }
    if ((tmpp = strstri(bufp, "invisible ")) != 0) {
        d->invisible = TRUE;
        (void) memset(tmpp, ' ', sizeof "invisible " - 1);
    }
    if ((tmpp = strstri(bufp, "hidden ")) != 0) {
        d->hidden = TRUE;
        (void) memset(tmpp, ' ', sizeof "hidden " - 1);
    }
    /* check "female" before "male" to avoid false hit mid-word */
    if ((tmpp = strstri(bufp, "female ")) != 0) {
        d->fem = 1;
        (void) memset(tmpp, ' ', sizeof "female " - 1);
    }
    if ((tmpp = strstri(bufp, "male ")) != 0) {
        d->fem = 0;
        (void) memset(tmpp, ' ', sizeof "male " - 1);
    }
    bufp = mungspaces(bufp); /* after potential memset(' ') */
    /* allow the initial disposition to be specified */
    if (!strncmpi(bufp, "tame ", 5)) {
        bufp += 5;
        d->maketame = TRUE;
    } else if (!strncmpi(bufp, "peaceful ", 9)) {
        bufp += 9;
        d->makepeaceful = TRUE;
    } else if (!strncmpi(bufp, "hostile ", 8)) {
        bufp += 8;
        d->makehostile = TRUE;
    }
    /* decide whether a valid monster was chosen */
    if (wizard && (!strcmp(bufp, "*") || !strcmp(bufp, "random"))) {
        d->randmonst = TRUE;
        return TRUE;
    }
    d->which = name_to_mon(bufp);
    if (d->which >= LOW_PM) {
        return TRUE; /* got one */
    }
    d->monclass = name_to_monclass(bufp, &d->which);

    if (d->which >= LOW_PM) {
        d->monclass = MAXMCLASSES; /* matters below */
        return TRUE;
#ifdef NEXT_VERSION
    } else if (d->monclass == S_invisible) { /* not an actual monster class */
        d->which = PM_STALKER;
        d->monclass = MAXMCLASSES;
        return TRUE;
#endif
    } else if (d->monclass == S_WORM_TAIL) { /* empty monster class */
        d->which = PM_LONG_WORM;
        d->monclass = MAXMCLASSES;
        return TRUE;
    } else if (d->monclass > 0) {
        d->which = urole.malenum; /* reset from NON_PM */
        return TRUE;
    }
    return FALSE;
}

static boolean
create_particular_creation(str, d)
char *str;
struct _create_particular_data *d;
{
    struct permonst *whichpm = NULL;
    int i, mx, my, firstchoice = NON_PM;
    struct monst *mtmp;
    boolean madeany = FALSE;

    if (!d->randmonst) {
        firstchoice = d->which;
        if (cant_revive(&d->which, FALSE, (struct obj *) 0) && firstchoice != PM_LONG_WORM_TAIL) {
            /* wizard mode can override handling of special monsters */
            char buf[BUFSZ];
            /* if str contains shapechanger name, create shapeshanger */
            if (!strstri(str, mons[d->which].mname)) {
                /* otherwise overwrite with firstchoice */
                d->which = firstchoice;
            }
        }
        whichpm = &mons[d->which];
    }
    for (i = 0; i < d->quan; i++) {
        if (d->monclass != MAXMCLASSES) {
            whichpm = mkclass(d->monclass, 0);
        } else if (d->randmonst) {
            whichpm = rndmonst();
        }
        mtmp = makemon(whichpm, u.ux, u.uy, NO_MM_FLAGS);
        if (!mtmp) {
            /* quit trying if creation failed and is going to repeat */
            if (d->monclass == MAXMCLASSES && !d->randmonst) {
                break;
            }
            /* otherwise try again */
            continue;
        }
        mx = mtmp->mx, my = mtmp->my;
        /* 'is_FOO()' ought to be called 'always_FOO()' */
        if (d->fem != -1 && !is_male(mtmp->data) && !is_female(mtmp->data)) {
            mtmp->female = d->fem; /* ignored for is_neuter() */
        }
        if (d->maketame) {
            (void) tamedog(mtmp, (struct obj *) 0);
        } else if (d->makepeaceful || d->makehostile) {
            mtmp->mtame = 0; /* sanity precaution */
            mtmp->mpeaceful = d->makepeaceful ? 1 : 0;
            set_malign(mtmp);
        }
        if (d->saddled && can_saddle(mtmp) && !which_armor(mtmp, W_SADDLE)) {
            struct obj *otmp = mksobj(SADDLE, TRUE, FALSE);

            put_saddle_on_mon(otmp, mtmp);
        }
        if (d->invisible) {
            mon_set_minvis(mtmp);
            if (does_block(mx, my, &levl[mx][my])) {
                block_point(mx, my);
            } else {
                unblock_point(mx, my);
            }
        }
       if (d->hidden &&
           ((is_hider(mtmp->data) && mtmp->data->mlet != S_MIMIC) ||
            (hides_under(mtmp->data) && OBJ_AT(mx, my)) ||
            (mtmp->data->mlet == S_EEL && is_pool(mx, my)))) {
            mtmp->mundetected = 1;
       }
        if (d->sleeping) {
            mtmp->msleeping = 1;
        }
        /* iff asking for 'hidden', show location of every created monster
           that can't be seen--whether that's due to successfully hiding
           or vision issues (line-of-sight, invisibility, blindness) */
        if (d->hidden && !canspotmon(mtmp)) {
            int count = couldsee(mx, my) ? 8 : 4;
            char saveviz = viz_array[my][mx];

            if (!flags.sparkle) {
                count /= 2;
            }
            viz_array[my][mx] |= (IN_SIGHT | COULD_SEE);
            flash_glyph_at(mx, my, mon_to_glyph(mtmp), count);
            viz_array[my][mx] = saveviz;
            newsym(mx, my);
        }
        madeany = TRUE;
        /* in case we got a doppelganger instead of what was asked
           for, make it start out looking like what was asked for */
        if (mtmp->cham != NON_PM && firstchoice != NON_PM && mtmp->cham != firstchoice) {
            (void) newcham(mtmp, &mons[firstchoice], FALSE, FALSE);
        }
    }
    return madeany;
}

/*
 * Make a new monster with the type controlled by the user.
 *
 * Note:  when creating a monster by class letter, specifying the
 * "strange object" (']') symbol produces a random monster rather
 * than a mimic.  This behavior quirk is useful so don't "fix" it
 * (use 'm'--or "mimic"--to create a random mimic).
 *
 * Used in wizard mode only (for ^G command and for scroll or spell
 * of create monster).  Once upon a time, an earlier incarnation of
 * this code was also used for the scroll/spell in explore mode.
 */
boolean
create_particular()
{
    char buf[BUFSZ] = DUMMY, *bufp;
    int  tryct = 5;
    struct _create_particular_data d;

    do {
        getlin("Create what kind of monster? [type the name or symbol]", buf);
        bufp = mungspaces(buf);
        if (*bufp == '\033') {
            return FALSE;
        }

        if (create_particular_parse(bufp, &d)) {
            break;
        }

        /* no good; try again... */
        pline("I've never heard of such monsters.");
    } while (--tryct > 0);

    if (!tryct) {
        pline("%s", thats_enough_tries);
    } else {
        return create_particular_creation(bufp, &d);
    }

    return FALSE;
}

boolean
create_particular_from_buffer(const char* bufp)
{
    struct _create_particular_data d;

    if (create_particular_parse(bufp, &d)) {
        return create_particular_creation(bufp, &d);
    }

    return FALSE;
}
#endif /* WIZARD */

void
drop_boulder_on_player(confused, helmet_protects, by_player, drop_directly_to_floor)
boolean confused;
boolean helmet_protects; /**< if player is protected by a hard helmet */
boolean by_player; /**< is boulder creation caused by player */
boolean drop_directly_to_floor; /**< don't check if player is swallowed by a monster */
{
    int dmg;
    struct obj *otmp2;

    /* hit monster if swallowed */
    if (u.uswallow && !drop_directly_to_floor) {
        drop_boulder_on_monster(u.ux, u.uy, confused, by_player);
        return;
    }

    /* Okay, _you_ write this without repeating the code */
    otmp2 = mksobj(confused ? ROCK : BOULDER,
                   FALSE, FALSE);
    if (!otmp2) return;
    otmp2->quan = confused ? rn1(5, 2) : 1;
    otmp2->owt = weight(otmp2);
    if (!amorphous(youmonst.data) &&
        !Passes_walls &&
        !noncorporeal(youmonst.data) &&
        !unsolid(youmonst.data)) {
        You("are hit by %s!", doname(otmp2));
        dmg = dmgval(otmp2, &youmonst) * otmp2->quan;
        if (uarmh && helmet_protects) {
            if (is_metallic(uarmh)) {
                pline("Fortunately, you are wearing a hard helmet.");
                if (dmg > 2) dmg = 2;
            } else if (flags.verbose) {
                Your("%s does not protect you.",
                     xname(uarmh));
            }
        }
    } else {
        dmg = 0;
    }

    wake_nearto(u.ux, u.uy, 4*4);

    /* Must be before the losehp(), for bones files */
    if (!flooreffects(otmp2, u.ux, u.uy, "fall")) {
        place_object(otmp2, u.ux, u.uy);
        stackobj(otmp2);
        newsym(u.ux, u.uy);
    }
    if (dmg) {
        losehp(Maybe_Half_Phys(dmg), "scroll of earth", KILLED_BY_AN);
    }
}

int
drop_boulder_on_monster(x, y, confused, by_player)
int x, y;
boolean confused;
boolean by_player; /**< is boulder creation caused by player */
{
    register struct obj *otmp2;
    register struct monst *mtmp2;

    /* Make the object(s) */
    otmp2 = mksobj(confused ? ROCK : BOULDER,
                   FALSE, FALSE);
    if (!otmp2) return 0;                      /* Shouldn't happen */
    otmp2->quan = confused ? rn1(5, 2) : 1;
    otmp2->owt = weight(otmp2);

    /* Find the monster here (should not be player) */
    mtmp2 = m_at(x, y);
    if (mtmp2 && !amorphous(mtmp2->data) &&
        !passes_walls(mtmp2->data) &&
        !noncorporeal(mtmp2->data) &&
        !unsolid(mtmp2->data)) {
        struct obj *helmet = which_armor(mtmp2, W_ARMH);
        int mdmg;

        if (cansee(mtmp2->mx, mtmp2->my)) {
            pline("%s is hit by %s!", Monnam(mtmp2),
                  doname(otmp2));
            if (mtmp2->minvis && !canspotmon(mtmp2))
                map_invisible(mtmp2->mx, mtmp2->my);
        } else if (u.uswallow && mtmp2 == u.ustuck) {
            if (flags.soundok) You_hear("something hit %s %s over your %s!",
                                        s_suffix(mon_nam(mtmp2)),
                                        mbodypart(mtmp2, STOMACH),
                                        body_part(HEAD));
        }
        mdmg = dmgval(otmp2, mtmp2) * otmp2->quan;
        if (helmet) {
            if (is_metallic(helmet)) {
                if (canspotmon(mtmp2))
                    pline("Fortunately, %s is wearing a hard helmet.", mon_nam(mtmp2));
                else if (flags.soundok)
                    You_hear("a clanging sound.");
                if (mdmg > 2) mdmg = 2;
            } else {
                if (canspotmon(mtmp2))
                    pline("%s's %s does not protect %s.",
                          Monnam(mtmp2), xname(helmet),
                          mhim(mtmp2));
            }
        }
        mtmp2->mhp -= mdmg;
        if (mtmp2->mhp <= 0) {
            if (by_player) {
                xkilled(mtmp2, 1);
            } else {
                pline("%s is killed.", Monnam(mtmp2));
                mondied(mtmp2);
            }
        } else {
            wakeup(mtmp2, by_player);
        }
        wake_nearto(x, y, 4 * 4);
    } else if (u.uswallow && mtmp2 == u.ustuck) {
        obfree(otmp2, (struct obj *)0);
        /* fall through to player */
        drop_boulder_on_player(confused, TRUE, FALSE, TRUE);
        return 1;
    }
    /* Drop the rock/boulder to the floor */
    if (!flooreffects(otmp2, x, y, "fall")) {
        place_object(otmp2, x, y);
        stackobj(otmp2);
        newsym(x, y); /* map the rock */
    }
    return 1;
}
/*read.c*/
