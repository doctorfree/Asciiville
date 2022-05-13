/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* "an [uncursed] greased [very rotten] partly eaten guardian naga hatchling (corpse)" */
#define PREFIX  80  /* (73) */
#define SCHAR_LIM 127
#define NUMOBUF 12

STATIC_DCL char *FDECL(strprepend, (char *, const char *));
static boolean FDECL(wishymatch, (const char *, const char *, BOOLEAN_P));
static char *NDECL(nextobuf);
STATIC_DCL void FDECL(releaseobuf, (char *));
static char *FDECL(minimal_xname, (struct obj *));
static void FDECL(add_erosion_words, (struct obj *, char *, BOOLEAN_P));
static char *FDECL(just_an, (char *str, const char *));
static char *FDECL(xname_flags, (struct obj *, unsigned));
static boolean FDECL(badman, (const char *, BOOLEAN_P));

struct Jitem {
    int item;
    const char *name;
};

#define BSTRCMP(base, ptr, string) ((ptr) < base || strcmp((ptr), string))
#define BSTRCMPI(base, ptr, string) ((ptr) < base || strcmpi((ptr), string))
#define BSTRNCMP(base, ptr, string, num) ((ptr)<base || strncmp((ptr), string, num))
#define BSTRNCMPI(base, ptr, string, num) ((ptr)<base||strncmpi((ptr), string, num))
#define Strcasecpy(dst, src) (void) strcasecpy(dst, src)

/* true for gems/rocks that should have " stone" appended to their names */
#define GemStone(typ)   (typ == FLINT ||                          \
                         (objects[typ].oc_material == GEMSTONE &&   \
                          (typ != DILITHIUM_CRYSTAL &&              \
                           typ != RUBY &&                           \
                           typ != DIAMOND &&                        \
                           typ != SAPPHIRE &&                       \
                           typ != BLACK_OPAL &&                     \
                           typ != EMERALD &&                        \
                           typ != OPAL)))

STATIC_OVL struct Jitem Japanese_items[] = {
    { SHORT_SWORD, "wakizashi" },
    { BROADSWORD, "ninja-to" },
    { FLAIL, "nunchaku" },
    { GLAIVE, "naginata" },
    { LOCK_PICK, "osaku" },
    { WOODEN_HARP, "koto" },
    { KNIFE, "shito" },
    { PLATE_MAIL, "tanko" },
    { HELMET, "kabuto" },
    { LEATHER_GLOVES, "yugake" },
    { FOOD_RATION, "gunyoki" },
    { POT_BOOZE, "sake" },
    {0, "" }
};

STATIC_DCL const char *FDECL(Japanese_item_name, (int i));

STATIC_OVL char *
strprepend(s, pref)
char *s;
const char *pref;
{
    char *prefixed = nextobuf();

    snprintf(prefixed, BUFSZ, "%s%s", pref, s);
    return prefixed;
}

/* manage a pool of BUFSZ buffers, so callers don't have to */
static char NEARDATA obufs[NUMOBUF][BUFSZ];
static int obufidx = 0;

/* manage a pool of BUFSZ buffers, so callers don't have to */
STATIC_OVL char *
nextobuf()
{
    obufidx = (obufidx + 1) % NUMOBUF;
    obufs[obufidx][0] = '\0';
    return obufs[obufidx];
}

/* put the most recently allocated buffer back if possible */
STATIC_OVL void
releaseobuf(bufp)
char *bufp;
{
    /* caller may not know whether bufp is the most recently allocated
       buffer; if it isn't, do nothing; note that because of the somewhat
       obscure PREFIX handling for object name formatting by xname(),
       the pointer our caller has and is passing to us might be into the
       middle of an obuf rather than the address returned by nextobuf() */
    if (bufp >= obufs[obufidx]
        && bufp < obufs[obufidx] + sizeof obufs[obufidx]) /* obufs[][BUFSZ] */
        obufidx = (obufidx - 1 + NUMOBUF) % NUMOBUF;
}

char *
obj_typename(otyp)
register int otyp;
{
    char *buf = nextobuf();
    struct objclass *ocl = &objects[otyp];
    const char *actualn = OBJ_NAME(*ocl);
    const char *dn = OBJ_DESCR(*ocl);
    const char *un = ocl->oc_uname;
    int nn = ocl->oc_name_known;

    if (Role_if(PM_SAMURAI) && Japanese_item_name(otyp))
        actualn = Japanese_item_name(otyp);
    switch(ocl->oc_class) {
    case COIN_CLASS:
        Strcpy(buf, "coin");
        break;
    case POTION_CLASS:
        Strcpy(buf, "potion");
        break;
    case SCROLL_CLASS:
        Strcpy(buf, "scroll");
        break;
    case WAND_CLASS:
        Strcpy(buf, "wand");
        break;
    case SPBOOK_CLASS:
        Strcpy(buf, "spellbook");
        break;
    case RING_CLASS:
        Strcpy(buf, "ring");
        break;
    case AMULET_CLASS:
        if(nn)
            Strcpy(buf, actualn);
        else
            Strcpy(buf, "amulet");
        if(un)
            Sprintf(eos(buf), " called %s", un);
        if(dn)
            Sprintf(eos(buf), " (%s)", dn);
        return(buf);
    default:
        if(nn) {
            Strcpy(buf, actualn);
            if (GemStone(otyp))
                Strcat(buf, " stone");
            if(un)
                Sprintf(eos(buf), " called %s", un);
            if(dn)
                Sprintf(eos(buf), " (%s)", dn);
        } else {
            Strcpy(buf, dn ? dn : actualn);
            if(ocl->oc_class == GEM_CLASS)
                Strcat(buf, (ocl->oc_material == MINERAL) ?
                       " stone" : " gem");
            if(un)
                Sprintf(eos(buf), " called %s", un);
        }
        return(buf);
    }
    /* here for ring/scroll/potion/wand */
    if(nn) {
        if (ocl->oc_unique)
            Strcpy(buf, actualn); /* avoid spellbook of Book of the Dead */
        else
            Sprintf(eos(buf), " of %s", actualn);
    }
    if(un)
        Sprintf(eos(buf), " called %s", un);
    if(dn)
        Sprintf(eos(buf), " (%s)", dn);
    return(buf);
}

/* less verbose result than obj_typename(); either the actual name
   or the description (but not both); user-assigned name is ignored */
char *
simple_typename(otyp)
int otyp;
{
    char *bufp, *pp, *save_uname = objects[otyp].oc_uname;

    objects[otyp].oc_uname = 0; /* suppress any name given by user */
    bufp = obj_typename(otyp);
    objects[otyp].oc_uname = save_uname;
    if ((pp = strstri(bufp, " (")) != 0)
        *pp = '\0'; /* strip the appended description */
    return bufp;
}

/* less verbose result than obj_typename(); either the actual name
   or the description (but not both); user-assigned name is ignored */
char *
dump_typename(otyp)
int otyp;
{
    char *p;
    int saved_name_known = objects[otyp].oc_name_known;
    objects[otyp].oc_name_known = 1;
    p = simple_typename(otyp);
    objects[otyp].oc_name_known = saved_name_known;
    return p;
}

/* typename for debugging feedback where data involved might be suspect */
char *
safe_typename(otyp)
int otyp;
{
    unsigned save_nameknown;
    char *res = 0;

    if (otyp < STRANGE_OBJECT || otyp >= NUM_OBJECTS
        || !OBJ_NAME(objects[otyp])) {
        res = nextobuf();
        Sprintf(res, "glorkum[%d]", otyp);
    } else {
        /* force it to be treated as fully discovered */
        save_nameknown = objects[otyp].oc_name_known;
        objects[otyp].oc_name_known = 1;
        res = simple_typename(otyp);
        objects[otyp].oc_name_known = save_nameknown;
    }
    return res;
}

boolean
obj_is_pname(obj)
struct obj *obj;
{
    if (!obj->oartifact || !has_oname(obj)) {
        return FALSE;
    }
    if (!program_state.gameover && !iflags.override_ID) {
        if (not_fully_identified(obj)) {
            return FALSE;
        }
    }
    return TRUE;
}

/* used by distant_name() to pass extra information to xname_flags();
   it would be much cleaner if this were a parameter, but that would
   require all of the xname() and doname() calls to be modified */
static int distantname = 0;

/* Give the name of an object seen at a distance.  Unlike xname/doname,
 * we don't want to set dknown if it's not set already.
 */
char *
distant_name(obj, func)
struct obj *obj;
char *FDECL((*func), (OBJ_P));
{
    char *str;

    /* 3.6.1: this used to save Blind, set it, make the call, then restore
     * the saved value; but the Eyes of the Overworld override blindness
     * and let characters wearing them get dknown set for distant items.
     *
     * TODO? if the hero is wearing those Eyes, figure out whether the
     * object is within X-ray radius and only treat it as distant when
     * beyond that radius.  Logic is iffy but result might be interesting.
     */
    ++distantname;
    str = (*func)(obj);
    --distantname;
    return str;
}

/* convert player specified fruit name into corresponding fruit juice name
   ("slice of pizza" -> "pizza juice" rather than "slice of pizza juice") */
char *
fruitname(juice)
boolean juice; /* whether or not to append " juice" to the name */
{
    char *buf = nextobuf();
    const char *fruit_nam = strstri(pl_fruit, " of ");

    if (fruit_nam)
        fruit_nam += 4; /* skip past " of " */
    else
        fruit_nam = pl_fruit; /* use it as is */

    Sprintf(buf, "%s%s", makesingular(fruit_nam), juice ? " juice" : "");
    return buf;
}

/* look up a named fruit by index (1..127) */
struct fruit *
fruit_from_indx(indx)
int indx;
{
    struct fruit *f;

    for (f = ffruit; f; f = f->nextf) {
        if (f->fid == indx) {
            break;
        }
    }
    return f;
}

/* look up a named fruit by name */
struct fruit *
fruit_from_name(fname, exact, highest_fid)
const char *fname;
boolean exact; /* False => prefix or exact match, True = exact match only */
int *highest_fid; /* optional output; only valid if 'fname' isn't found */
{
    struct fruit *f, *tentativef;
    char *altfname;
    unsigned k;
    /*
     * note: named fruits are case-senstive...
     */

    if (highest_fid) {
        *highest_fid = 0;
    }
    /* first try for an exact match */
    for (f = ffruit; f; f = f->nextf) {
        if (!strcmp(f->fname, fname)) {
            return f;
        } else if (highest_fid && f->fid > *highest_fid) {
            *highest_fid = f->fid;
        }
    }

    /* didn't match as-is; if caller is willing to accept a prefix
       match, try to find one; we want to find the longest prefix that
       matches, not the first */
    if (!exact) {
        tentativef = 0;
        for (f = ffruit; f; f = f->nextf) {
            k = strlen(f->fname);
            if (!strncmp(f->fname, fname, k) &&
                 (!fname[k] || fname[k] == ' ') &&
                 (!tentativef || k > strlen(tentativef->fname))) {
                tentativef = f;
            }
        }
        f = tentativef;
    }
    /* if we still don't have a match, try singularizing the target;
       for exact match, that's trivial, but for prefix, it's hard */
    if (!f) {
        altfname = makesingular(fname);
        for (f = ffruit; f; f = f->nextf) {
            if (!strcmp(f->fname, altfname)) {
                break;
            }
        }
        releaseobuf(altfname);
    }
    if (!f && !exact) {
        char fnamebuf[BUFSZ], *p;
        unsigned fname_k = strlen(fname); /* length of assumed plural fname */

        tentativef = 0;
        for (f = ffruit; f; f = f->nextf) {
            k = strlen(f->fname);
            /* reload fnamebuf[] each iteration in case it gets modified;
               there's no need to recalculate fname_k */
            Strcpy(fnamebuf, fname);
            /* bug? if singular of fname is longer than plural,
               failing the 'fname_k > k' test could skip a viable
               candidate; unfortunately, we can't singularize until
               after stripping off trailing stuff and we can't get
               accurate fname_k until fname has been singularized;
               compromise and use 'fname_k >= k' instead of '>',
               accepting 1 char length discrepancy without risking
               false match (I hope...) */
            if (fname_k >= k && (p = index(&fnamebuf[k], ' ')) != 0) {
                *p = '\0'; /* truncate at 1st space past length of f->fname */
                altfname = makesingular(fnamebuf);
                k = strlen(altfname); /* actually revised 'fname_k' */
                if (!strcmp(f->fname, altfname) &&
                     (!tentativef || k > strlen(tentativef->fname))) {
                    tentativef = f;
                }
                releaseobuf(altfname); /* avoid churning through all obufs */
            }
        }
        f = tentativef;
    }
    return f;
}

/* sort the named-fruit linked list by fruit index number */
void
reorder_fruit(forward)
boolean forward;
{
    struct fruit *f, *allfr[1 + 127];
    int i, j, k = SIZE(allfr);

    for (i = 0; i < k; ++i) {
        allfr[i] = (struct fruit *) 0;
    }
    for (f = ffruit; f; f = f->nextf) {
        /* without sanity checking, this would reduce to 'allfr[f->fid]=f' */
        j = f->fid;
        if (j < 1 || j >= k) {
            impossible("reorder_fruit: fruit index (%d) out of range", j);
            return; /* don't sort after all; should never happen... */
        } else if (allfr[j]) {
            impossible("reorder_fruit: duplicate fruit index (%d)", j);
            return;
        }
        allfr[j] = f;
    }
    ffruit = 0; /* reset linked list; we're rebuilding it from scratch */
    /* slot [0] will always be empty; must start 'i' at 1 to avoid
       [k - i] being out of bounds during first iteration */
    for (i = 1; i < k; ++i) {
        /* for forward ordering, go through indices from high to low;
           for backward ordering, go from low to high */
        j = forward ? (k - i) : i;
        if (allfr[j]) {
            allfr[j]->nextf = ffruit;
            ffruit = allfr[j];
        }
    }
}

char *
xname(obj)
struct obj *obj;
{
    return xname_flags(obj, CXN_NORMAL);
}

static char *
xname_flags(obj, cxn_flags)
register struct obj *obj;
unsigned cxn_flags; /* bitmask of CXN_xxx values */
{
    char *buf;
    int typ = obj->otyp;
    struct objclass *ocl = &objects[typ];
    int nn = ocl->oc_name_known ||
                      /* only reveal Sokoban prizes when in sight */
                      (Is_sokoprize(obj) &&
                       (cansee(obj->ox, obj->oy) ||
                        /* even reveal when Sokoban prize only felt */
                        (u.ux == obj->ox && u.uy == obj->oy)));
    int omndx = obj->corpsenm;
    const char *actualn = OBJ_NAME(*ocl);
    const char *dn = OBJ_DESCR(*ocl);
    const char *un = ocl->oc_uname;
    boolean pluralize = (obj->quan != 1L) && !(cxn_flags & CXN_SINGULAR);
    boolean known, dknown, bknown;

    buf = nextobuf();
    if (Role_if(PM_SAMURAI) && Japanese_item_name(typ))
        actualn = Japanese_item_name(typ);

    /* As of 3.6.2: this used to be part of 'dn's initialization, but it
       needs to come after possibly overriding 'actualn' */
    if (!dn) {
        dn = actualn;
    }

    buf[0] = '\0';
    /*
     * clean up known when it's tied to oc_name_known, eg after AD_DRIN
     * This is only required for unique objects since the article
     * printed for the object is tied to the combination of the two
     * and printing the wrong article gives away information.
     */
    if (!nn && ocl->oc_uses_known && ocl->oc_unique) {
        obj->known = 0;
    }
    if (!Blind && !distantname) {
        obj->dknown = 1;
    }
    /* needed, otherwise BoH shows only up as "bag" when blind */
    if (Is_sokoprize(obj)) {
        obj->dknown = 1;
    }

    if (Role_if(PM_PRIEST)) {
        obj->bknown = 1; /* actively avoid set_bknown();
                          * we mustn't call update_inventory() now because
                          * it would call xname() (via doname()) recursively
                          * and could end up clobbering all the obufs... */
    }

    if (iflags.override_ID) {
        known = dknown = bknown = TRUE;
        nn = 1;
    } else if (cxn_flags & CXN_UNIDENTIFIED) {
        /* force unidentified */
        known = FALSE;
        dknown = TRUE;
        bknown = FALSE;
        nn = FALSE;
    } else {
        known = obj->known;
        dknown = obj->dknown;
        bknown = obj->bknown;
    }

    if (obj_is_pname(obj))
        goto nameit;

    switch (obj->oclass) {
    case AMULET_CLASS:
        if (!dknown) {
            Strcpy(buf, "amulet");
        } else if (typ == AMULET_OF_YENDOR ||
                   typ == FAKE_AMULET_OF_YENDOR) {
            /* each must be identified individually */
            Strcpy(buf, obj->known ? actualn : dn);
        } else if (nn) {
            Strcpy(buf, actualn);
        } else if (un) {
            Sprintf(buf, "amulet called %s", un);
        } else if (Is_sokoprize(obj)) {
            Strcpy(buf, "sokoban amulet");
        } else {
            Sprintf(buf, "%s amulet", dn);
        }
        break;

    case WEAPON_CLASS:
        if (is_poisonable(obj) && obj->opoisoned)
            Strcpy(buf, "poisoned ");
        /* fall through */

    case VENOM_CLASS:
    case TOOL_CLASS:
        if (typ == LENSES) {
            Strcpy(buf, "pair of ");
        } else if (is_wet_towel(obj)) {
            Strcpy(buf, (obj->spe < 3) ? "moist " : "wet ");
        }

        if (!dknown) {
            Strcat(buf, dn);
        } else if (nn) {
            Strcat(buf, actualn);
        } else if (un) {
            Strcat(buf, dn);
            Strcat(buf, " called ");
            Strcat(buf, un);
        } else if (Is_sokoprize(obj)) {
            Strcpy(buf, "sokoban bag");
        } else {
            Strcat(buf, dn);
        }
        if (typ == FIGURINE && omndx != NON_PM) {
            char anbuf[10]; /* [4] would be enough: 'a','n',' ','\0' */

            Sprintf(eos(buf), " of %s%s",
                    just_an(anbuf, mons[omndx].mname),
                    mons[omndx].mname);
        } else if (is_wet_towel(obj)) {
            if (wizard) {
                Sprintf(eos(buf), " (%d)", obj->spe);
            }
        }
        break;

    case ARMOR_CLASS:
        if (Is_dragon_scales(obj->otyp)) {
            Strcat(buf, "set of ");
        }
        if (is_boots(obj) || is_gloves(obj)) {
            Strcpy(buf, "pair of ");
        }
        if (obj->otyp >= ELVEN_SHIELD && obj->otyp <= ORCISH_SHIELD && !dknown) {
            Strcpy(buf, "shield");
            break;
        }
        if (obj->otyp == SHIELD_OF_REFLECTION && !dknown) {
            Strcpy(buf, "smooth shield");
            break;
        }

        if (nn) {
            Strcat(buf, actualn);
        } else if (un) {
            if (is_boots(obj)) {
                Strcat(buf, "boots");
            } else if(is_gloves(obj)) {
                Strcat(buf, "gloves");
            } else if(is_cloak(obj)) {
                Strcpy(buf, "cloak");
            } else if(is_helmet(obj)) {
                Strcpy(buf, "helmet");
            } else if(is_shield(obj)) {
                Strcpy(buf, "shield");
            } else {
                Strcpy(buf, "armor");
            }
            Strcat(buf, " called ");
            Strcat(buf, un);
        } else if (Is_sokoprize(obj)) {
            Strcpy(buf, "sokoban cloak");
        } else {
            Strcat(buf, dn);
        }
        break;

    case FOOD_CLASS:
        if (typ == SLIME_MOLD) {
            struct fruit *f = fruit_from_indx(obj->spe);

            if (!f) {
                impossible("Bad fruit #%d?", obj->spe);
                Strcpy(buf, "fruit");
            } else {
                Strcpy(buf, f->fname);
                if (pluralize) {
                    /* ick; already pluralized fruit names
                       are allowed--we want to try to avoid
                       adding a redundant plural suffix */
                    Strcpy(buf, makeplural(makesingular(buf)));
                    pluralize = FALSE;
                }
            }
            break;
        } else if (typ == CREAM_PIE && piday()) {
            Strcpy(buf, "irrational pie");
            break;
        }

        Strcpy(buf, actualn);
        if (typ == TIN && known) {
            tin_details(obj, omndx, buf);
        }
        break;

    case COIN_CLASS:
    case CHAIN_CLASS:
        Strcpy(buf, actualn);
        break;

    case ROCK_CLASS:
        if (typ == STATUE && omndx != NON_PM) {
            char anbuf[10];

            Sprintf(buf, "%s%s of %s%s",
                    (Role_if(PM_ARCHEOLOGIST) && (obj->spe & STATUE_HISTORIC)) ? "historic " : "",
                    actualn,
                    type_is_pname(&mons[omndx]) ? "" :
                    the_unique_pm(&mons[omndx]) ? "the " : just_an(anbuf, mons[omndx].mname),
                    mons[omndx].mname);
        } else {
            Strcpy(buf, actualn);
        }
        break;

    case BALL_CLASS:
        Sprintf(buf, "%sheavy iron ball",
                (obj->owt > ocl->oc_weight) ? "very " : "");
        break;

    case POTION_CLASS:
        if (dknown && obj->odiluted) {
            Strcpy(buf, "diluted ");
        }
        if (nn || un || !dknown) {
            Strcat(buf, "potion");
            if (!dknown) {
                break;
            }
            if (nn) {
                Strcat(buf, " of ");
                if (typ == POT_WATER &&
                     bknown && (obj->blessed || obj->cursed)) {
                    Strcat(buf, obj->blessed ? "holy " : "unholy ");
                }
                /* work around for potion alchemy bug that lets one
                 * alchemize potions in unused range */
                if (actualn) {
                    Strcat(buf, actualn);
                } else {
                    warning("inexistant potion %d", obj->otyp);
                    Strcat(buf, "inexistant");
                }
            } else {
                Strcat(buf, " called ");
                Strcat(buf, un);
            }
        } else {
            Strcat(buf, dn);
            Strcat(buf, " potion");
        }
        break;

    case SCROLL_CLASS:
        Strcpy(buf, "scroll");
        if (!dknown) {
            break;
        }
        if (nn) {
            Strcat(buf, " of ");
            Strcat(buf, actualn);
        } else if (un) {
            Strcat(buf, " called ");
            Strcat(buf, un);
        } else if (ocl->oc_magic) {
            Strcat(buf, " labeled ");
            Strcat(buf, dn);
        } else {
            Strcpy(buf, dn);
            Strcat(buf, " scroll");
        }
        break;

    case WAND_CLASS:
        if (!dknown) {
            Strcpy(buf, "wand");
        } else if (nn) {
            Sprintf(buf, "wand of %s", actualn);
        } else if (un) {
            Sprintf(buf, "wand called %s", un);
        } else {
            Sprintf(buf, "%s wand", dn);
        }
        break;

    case SPBOOK_CLASS:
        if (!dknown) {
            Strcpy(buf, "spellbook");
        } else if (nn) {
            if (typ != SPE_BOOK_OF_THE_DEAD) {
                Strcpy(buf, "spellbook of ");
            }
            Strcat(buf, actualn);
        } else if (un) {
            Sprintf(buf, "spellbook called %s", un);
        } else
            Sprintf(buf, "%s spellbook", dn);
        break;

    case RING_CLASS:
        if (!dknown) {
            Strcpy(buf, "ring");
        } else if (nn) {
            Sprintf(buf, "ring of %s", actualn);
        } else if (un) {
            Sprintf(buf, "ring called %s", un);
        } else {
            Sprintf(buf, "%s ring", dn);
        }
        break;

    case GEM_CLASS:
    {
        const char *rock = (ocl->oc_material == MINERAL) ? "stone" : "gem";

        if (!dknown) {
            Strcpy(buf, rock);
        } else if (!nn) {
            if (un) {
                Sprintf(buf, "%s called %s", rock, un);
            } else {
                Sprintf(buf, "%s %s", dn, rock);
            }
        } else {
            Strcpy(buf, actualn);
            if (GemStone(typ)) {
                Strcat(buf, " stone");
            }
        }
        break;
    }
    default:
        Sprintf(buf, "glorkum %d %d %d", obj->oclass, typ, obj->spe);
    }
    if (pluralize) {
        Strcpy(buf, makeplural(buf));
    }

    if (program_state.gameover) {
        char tmpbuf[BUFSZ];

        /* disclose without breaking illiterate conduct, but mainly tip off
           players who aren't aware that something readable is present */
        switch (obj->otyp) {
        case T_SHIRT:
            Sprintf(eos(buf), " with text \"%s\"", tshirt_text(obj, tmpbuf));
            break;

        case ALCHEMY_SMOCK:
            Sprintf(eos(buf), " with text \"%s\"", apron_text(obj, tmpbuf));
            break;

        case HAWAIIAN_SHIRT:
            Sprintf(eos(buf), " with %s motif", an(hawaiian_motif(obj, tmpbuf)));
            break;

        default:
            break;
        }
    }

    if (has_oname(obj) && dknown) {
        Strcat(buf, " named ");
nameit:
        Strcat(buf, ONAME(obj));
    }

    if (!strncmpi(buf, "the ", 4)) {
        buf += 4;
    }
    return buf;
}

/* similar to simple_typename but minimal_xname operates on a particular
   object rather than its general type; it formats the most basic info:
     potion                     -- if description not known
     brown potion               -- if oc_name_known not set
     potion of object detection -- if discovered
 */
static char *
minimal_xname(obj)
struct obj *obj;
{
    char *bufp;
    struct obj bareobj;
    struct objclass saveobcls;
    int otyp = obj->otyp;

    /* suppress user-supplied name */
    saveobcls.oc_uname = objects[otyp].oc_uname;
    objects[otyp].oc_uname = 0;
    /* suppress actual name if object's description is unknown */
    saveobcls.oc_name_known = objects[otyp].oc_name_known;
    if (!obj->dknown) {
        objects[otyp].oc_name_known = 0;
    }

    /* caveat: this makes a lot of assumptions about which fields
       are required in order for xname() to yield a sensible result */
    bareobj = zeroobj;
    bareobj.otyp = otyp;
    bareobj.oclass = obj->oclass;
    bareobj.dknown = obj->dknown;
    /* suppress known except for amulets (needed for fakes and real A-of-Y) */
    bareobj.known = (obj->oclass == AMULET_CLASS)
                        ? obj->known
                        /* default is "on" for types which don't use it */
                        : !objects[otyp].oc_uses_known;
    bareobj.quan = 1L;         /* don't want plural */
    bareobj.corpsenm = NON_PM; /* suppress statue and figurine details */
    /* but suppressing fruit details leads to "bad fruit #0"
       [perhaps we should force "slime mold" rather than use xname?] */
    if (obj->otyp == SLIME_MOLD) {
        bareobj.spe = obj->spe;
    }

    bufp = distant_name(&bareobj, xname); /* xname(&bareobj) */
    if (!strncmp(bufp, "uncursed ", 9)) {
        bufp += 9; /* Role_if(PM_PRIEST) */
    }

    objects[otyp].oc_uname = saveobcls.oc_uname;
    objects[otyp].oc_name_known = saveobcls.oc_name_known;
    return bufp;
}

/* xname() output augmented for multishot missile feedback */
char *
mshot_xname(obj)
struct obj *obj;
{
    char tmpbuf[BUFSZ];
    char *onm = xname(obj);

    if (m_shot.n > 1 && m_shot.o == obj->otyp) {
        /* "the Nth arrow"; value will eventually be passed to an() or
           The(), both of which correctly handle this "the " prefix */
        Sprintf(tmpbuf, "the %d%s ", m_shot.i, ordin(m_shot.i));
        onm = strprepend(onm, tmpbuf);
    }

    return onm;
}

/* used for naming "the unique_item" instead of "a unique_item" */
boolean
the_unique_obj(obj)
struct obj *obj;
{
    boolean known = (obj->known || iflags.override_ID);

    if (!obj->dknown && !iflags.override_ID) {
        return FALSE;
    } else if (obj->otyp == FAKE_AMULET_OF_YENDOR && !known) {
        return TRUE; /* lie */
    } else {
        return (boolean)(objects[obj->otyp].oc_unique &&
                         (known || obj->otyp == AMULET_OF_YENDOR));
    }
}

/** should monster type be prefixed with "the"? (mostly used for corpses) */
boolean
the_unique_pm(ptr)
struct permonst *ptr;
{
    boolean uniq;

    /* even though monsters with personal names are unique, we want to
       describe them as "Name" rather than "the Name" */
    if (type_is_pname(ptr)) {
        return FALSE;
    }

    uniq = (ptr->geno & G_UNIQ) ? TRUE : FALSE;
    /* high priest is unique if it includes "of <deity>", otherwise not
       (caller needs to handle the 1st possibility; we assume the 2nd);
       worm tail should be irrelevant but is included for completeness */
    if (ptr == &mons[PM_HIGH_PRIEST] || ptr == &mons[PM_LONG_WORM_TAIL]) {
        uniq = FALSE;
    }
    /* Wizard no longer needs this; he's flagged as unique these days */
    if (ptr == &mons[PM_WIZARD_OF_YENDOR]) {
        uniq = TRUE;
    }
    return uniq;
}

static void
add_erosion_words(obj, prefix, in_final_dump)
struct obj *obj;
char *prefix;
boolean in_final_dump;
{
    boolean iscrys = (obj->otyp == CRYSKNIFE);
    boolean rknown = (iflags.override_ID == 0) ? obj->rknown : TRUE;

    if (!is_damageable(obj) && !iscrys) return;

    /* The only cases where any of these bits do double duty are for
     * rotted food and diluted potions, which are all not is_damageable().
     */
    if (obj->oeroded && !iscrys) {
        switch (obj->oeroded) {
        case 2: Strcat(prefix, "very "); break;
        case 3: Strcat(prefix, "thoroughly "); break;
        }
        Strcat(prefix, is_rustprone(obj) ? "rusty " : "burnt ");
    }
    if (obj->oeroded2 && !iscrys) {
        switch (obj->oeroded2) {
        case 2: Strcat(prefix, "very "); break;
        case 3: Strcat(prefix, "thoroughly "); break;
        }
        Strcat(prefix, is_corrodeable(obj) ? "corroded " :
               "rotted ");
    }
    if (obj->oerodeproof && (in_final_dump || rknown)) {
        Sprintf(eos(prefix), "%s%s%s ",
                rknown ? "" : "[",
                iscrys ? "fixed" :
                is_rustprone(obj) ? "rustproof" :
                is_corrodeable(obj) ? "corrodeproof" : /* "stainless"? */
                is_flammable(obj) ? "fireproof" : "",
                rknown ? "" : "]");
    }
}

/* used to prevent rust on items where rust makes no difference */
boolean
erosion_matters(obj)
struct obj *obj;
{
    switch (obj->oclass) {
    case TOOL_CLASS:
        /* it's possible for a rusty weptool to be polymorphed into some
           non-weptool iron tool, in which case the rust implicitly goes
           away, but it's also possible for it to be polymorphed into a
           non-iron tool, in which case rust also implicitly goes away,
           so there's no particular reason to try to handle the first
           instance differently [this comment belongs in poly_obj()...] */
        return is_weptool(obj) ? TRUE : FALSE;
    case WEAPON_CLASS:
    case ARMOR_CLASS:
    case BALL_CLASS:
    case CHAIN_CLASS:
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

static char *
doname_base(obj, with_price)
register struct obj *obj;
boolean with_price;
{
    boolean ispoisoned = FALSE;
    boolean weightshown = FALSE;
    boolean known, dknown, cknown, bknown, lknown;
    int omndx = obj->corpsenm;
    char prefix[PREFIX];
    /* when we have to add something at the start of prefix instead of the
     * end (Strcat is used on the end)
     */
    char tmpbuf[PREFIX+1];

    /* tourists get a special identification service for shop items */
    if (Role_if(PM_TOURIST)) {
        long price = get_cost_of_shop_item(obj);
        if (price > 0) {
            discover_object(obj->otyp, TRUE, FALSE);
        }
    }

    register char *bp = xname(obj), *tmp;

    int dump_ID_flag = program_state.gameover;
    /* display ID in addition to appearance */
    boolean do_ID = dump_ID_flag && !objects[obj->otyp].oc_name_known;
    boolean do_known = dump_ID_flag && !obj->known;
    boolean do_dknown = dump_ID_flag && !obj->dknown;
    boolean do_bknown = dump_ID_flag && !obj->bknown;
    boolean do_lknown = dump_ID_flag && !obj->lknown;
    boolean do_cknown = dump_ID_flag && !obj->cknown;
    /*boolean do_rknown = dump_ID_flag && !obj->rknown;*/

    if (!dump_ID_flag)
        ; /* early exit */
    else if (exist_artifact(obj->otyp, (tmp = (char *)safe_oname(obj)))) {
        if (do_dknown || do_known) {
            Sprintf(eos(bp), " [%s]", tmp);
        }
        ; /* if already known as an artifact, don't bother showing the base type */
    }
    else if (obj->otyp == EGG && obj->corpsenm >= LOW_PM &&
             !(obj->known || mvitals[obj->corpsenm].mvflags & MV_KNOWS_EGG))
        Sprintf(bp, "[%s] egg%s", mons[obj->corpsenm].mname, obj->quan > 1 ? "s" : "");

    else if (do_ID || do_dknown) {
        char *cp = nextobuf();
        if (Role_if(PM_SAMURAI) && (tmp = (char*)Japanese_item_name(obj->otyp)))
            Strcpy(cp, tmp);
        else if (obj->otyp == POT_WATER && (obj->blessed || obj->cursed))
            Sprintf(cp, "%sholy water", obj->blessed ? "" : "un");
        else {
            Strcpy(cp, OBJ_NAME(objects[obj->otyp]));
        }

        /* Hideous post-processing: try to merge the ID and appearance naturally
           The cases are significant, to avoid matching fruit names.
           General rules, barring bugs:
             thing of foo [thing of actual] -> thing of foo [of actual]
               (no such objects)
             foo thing [thing of actual] -> foo thing [of actual]
               eg. square amulet [of strangulation]
             thing of foo [actual thing] -> thing of foo [of actual]
               eg. scroll labeled DUAM XNAHT [of amnesia]
             foo thing [actual thing] -> foo thing [actual]
               eg. mud boots [speed boots]
             thing [thing of actual] -> thing [of actual]
               eg. bag [of holding]
             thing [actual thing] -> [actual] thing
               eg. [wax] candle
         */
        switch(obj->oclass) {
        case COIN_CLASS:
            *cp = '\0';
            break;
        case AMULET_CLASS:
            if(obj->otyp == AMULET_VERSUS_POISON) cp += sizeof("amulet");     /* versus poison */
            else if(obj->otyp == FAKE_AMULET_OF_YENDOR) *strstr(cp, " of the Amulet of Yendor") = '\0';     /* cheap plastic imitation */
            else if(obj->otyp == AMULET_OF_YENDOR) *cp = '\0';     /* is its own description */
            else cp += sizeof("amulet");
            break;
        case WEAPON_CLASS:
            if ((tmp = strstr(cp, " dagger"))) *tmp = '\0';
            else if ((tmp = strstr(cp, " bow"))) *tmp = '\0';
            else if ((tmp = strstr(cp, " arrow"))) *tmp = '\0';
            else if ((tmp = strstr(cp, " short sword"))) *tmp = '\0';
            else if ((tmp = strstr(cp, " broadsword"))) *tmp = '\0';
            else if ((tmp = strstr(cp, " spear"))) *tmp = '\0';
            break;
        case ARMOR_CLASS:
            if (obj->otyp == DWARVISH_CLOAK) Strcpy(cp, "dwarvish");
            /* only remove "cloak" if unIDed is already "opera cloak" */
            else if (strstr(bp, "cloak")) {
                if ((tmp = strstr(cp, " cloak"))) *tmp = '\0';     /* elven */
                else if (strstr(cp, "cloak of ")) cp += sizeof("cloak");     /* other */
            }
            else if (obj->otyp == LEATHER_GLOVES) Strcpy(cp, "leather");
            else if ((tmp = strstr(cp, " gloves"))) *tmp = '\0';     /* other */
            else if ((tmp = strstr(cp, " boots"))) *tmp = '\0';
            /* else if((tmp = strstr(cp, " boots"))) {
             * tmp = '\0';
                memmove(cp + 3, cp, strlen(cp) + 1);
                strncpy(cp, "of ", 3);
               } foo boots [actual boots] -> foo boots [of actual] */
            else if ((tmp = strstr(cp, " shoes"))) *tmp = '\0';     /* iron */
            else if (strstr(cp, "helm of ")) cp += sizeof("helm");
            else if (strstr(cp, "shield of ")) cp += sizeof("shield");     /* of reflection */
            else if ((tmp = strstr(cp, " shield"))) *tmp = '\0';
            else if ((tmp = strstr(cp, " ring mail"))) *tmp = '\0';     /* orcish */
            else if ((tmp = strstr(cp, " chain mail"))) *tmp = '\0';     /* orcish */
            break;
        case TOOL_CLASS:
            /* thing [actual thing] -> [actual] thing */
            if ((tmp = strstr(cp, " candle")) ||
                (tmp = strstr(cp, " horn")) ||
                (tmp = strstr(cp, " lamp")) ||
                (tmp = strstr(cp, " flute")) ||
                (tmp = strstr(cp, " harp")) ||
                (tmp = strstr(cp, " whistle"))) {
                *tmp = '\0';
                memmove(cp + 1, cp, strlen(cp) + 1);
                *cp = '[';
                Strcat(cp, "] ");
                bp = strprepend(bp, cp);
                *cp = '\0';
            }
            else if (strstr(cp, "horn of ")) cp += sizeof("horn");     /* of plenty */
            else if (obj->otyp == LEATHER_DRUM) Strcpy(cp, "leather");
            else if (obj->otyp == DRUM_OF_EARTHQUAKE) Strcpy(cp, "of earthquake");
            else if ((tmp = strstr(cp, "bag of "))) cp += sizeof("bag");
            break;
        case GEM_CLASS:
            if (strstr(cp, "worthless piece")) Strcpy(cp, "worthless glass");
            break;
        case VENOM_CLASS:
            /* technically, this doesn't follow the rules... if anyone cares. */
            if ((tmp = strstr(cp, " venom"))) *tmp = '\0';
            break;
        }
        /* end post-processing */

        if (strlen(cp) > 0) {
            if (obj->oclass == POTION_CLASS || obj->oclass == SCROLL_CLASS
                || (obj->oclass == SPBOOK_CLASS && obj->otyp != SPE_BOOK_OF_THE_DEAD)
                || obj->oclass == WAND_CLASS || obj->oclass == RING_CLASS)
                Sprintf(eos(bp), " [of %s]", cp);
            else
                Sprintf(eos(bp), " [%s]", cp);
        }
    }
    else if (obj->otyp == TIN && do_known) {
        if (obj->spe > 0)
            Strcat(bp, " [of spinach]");
        else if (obj->corpsenm == NON_PM)
            Strcat(bp, " [empty]");
        else if (vegetarian(&mons[obj->corpsenm]))
            Sprintf(eos(bp), " [of %s]", mons[obj->corpsenm].mname);
        else
            Sprintf(eos(bp), " [of %s meat]", mons[obj->corpsenm].mname);
    }
    else if (obj->otyp == POT_WATER &&
             (obj->blessed || obj->cursed) && do_bknown) {
        Sprintf(bp, "potion of [%sholy] water", obj->cursed ? "un" : "");
    }

    if (iflags.override_ID) {
        known = dknown = cknown = bknown = lknown = TRUE;
    } else {
        known = obj->known;
        dknown = obj->dknown;
        cknown = obj->cknown;
        bknown = obj->bknown;
        lknown = obj->lknown;
    }

    /* When using xname, we want "poisoned arrow", and when using
     * doname, we want "poisoned +0 arrow".  This kludge is about the only
     * way to do it, at least until someone overhauls xname() and doname(),
     * combining both into one function taking a parameter.
     */
    /* must check opoisoned--someone can have a weirdly-named fruit */
    if (!strncmp(bp, "poisoned ", 9) && obj->opoisoned) {
        bp += 9;
        ispoisoned = TRUE;
    }

    if (obj->quan != 1L) {
        Sprintf(prefix, "%ld ", obj->quan);
    } else if (obj->otyp == CORPSE) {
        /* skip article prefix for corpses [else corpse_xname()
           would have to be taught how to strip it off again] */
        *prefix = '\0';
    } else if (obj_is_pname(obj) || the_unique_obj(obj)) {
        if (!strncmpi(bp, "the ", 4))
            bp += 4;
        Strcpy(prefix, "the ");
    } else {
        Strcpy(prefix, "a ");
    }

#ifdef INVISIBLE_OBJECTS
    if (obj->oinvis) Strcat(prefix, "invisible ");
#endif

    /* "empty" goes at the beginning, but item count goes at the end */
    if (cknown &&
        /* bag of tricks: include "empty" prefix if it's known to
           be without charges and empty */
        ((obj->otyp == BAG_OF_TRICKS) ? (obj->spe == 0 && !Has_contents(obj)) :
             /* not bag of tricks: empty if container which has no contents */
           ((Is_container(obj) || obj->otyp == STATUE) && !Has_contents(obj)))) {
        Strcat(prefix, "empty ");
    }

    if (cknown &&
        ((obj->otyp == OIL_LAMP) || (obj->otyp == BRASS_LANTERN)) &&
        (obj->age == 0)) {
        Strcat(prefix, "empty ");
    }

    if ((bknown || do_bknown) &&
        obj->oclass != COIN_CLASS &&
        (obj->otyp != POT_WATER || !objects[POT_WATER].oc_name_known
         || (!obj->cursed && !obj->blessed))) {
        /* allow 'blessed clear potion' if we don't know it's holy water;
         * always allow "uncursed potion of water"
         */
        if (obj->cursed)
            Strcat(prefix, do_bknown ? "[cursed] " : "cursed ");
        else if (obj->blessed)
            Strcat(prefix, do_bknown ? "[blessed] " : "blessed ");
        else if (iflags.show_buc || ((!known || !objects[obj->otyp].oc_charged ||
                                      (obj->oclass == ARMOR_CLASS ||
                                       obj->oclass == RING_CLASS))
                                     /* For most items with charges or +/-, if you know how many
                                      * charges are left or what the +/- is, then you must have
                                      * totally identified the item, so "uncursed" is unneccesary,
                                      * because an identified object not described as "blessed" or
                                      * "cursed" must be uncursed.
                                      *
                                      * If the charges or +/- is not known, "uncursed" must be
                                      * printed to avoid ambiguity between an item whose curse
                                      * status is unknown, and an item known to be uncursed.
                                      */
#ifdef MAIL
                                     && obj->otyp != SCR_MAIL
#endif
                                     && obj->otyp != FAKE_AMULET_OF_YENDOR
                                     && obj->otyp != AMULET_OF_YENDOR
                                     && !Role_if(PM_PRIEST)))
            Strcat(prefix, do_bknown ? "[uncursed] " : "uncursed ");
    }

    if ((lknown || do_lknown) && Is_box(obj)) {
        if (obj->obroken) {
            /* 3.6.0 used "unlockable" here but that could be misunderstood
               to mean "capable of being unlocked" rather than the intended
               "not capable of being locked" */
            Strcat(prefix, do_lknown ? "[broken] " : "broken ");
        } else if (obj->olocked) {
            Strcat(prefix, do_lknown ? "[locked] " : "locked ");
        } else {
            Strcat(prefix, do_lknown ? "[unlocked] " : "unlocked ");
        }
    }

    if (obj->greased) Strcat(prefix, "greased ");

    if ((cknown || do_cknown) && Has_contents(obj)) {
        /* we count the number of separate stacks, which corresponds
           to the number of inventory slots needed to be able to take
           everything out if no merges occur */
        long itemcount = count_contents(obj, FALSE, FALSE, TRUE, FALSE);

        if (do_cknown) {
            Sprintf(eos(bp), " [containing %ld item%s]", itemcount, plur(itemcount));
        } else {
            Sprintf(eos(bp), " (containing %ld item%s)", itemcount, plur(itemcount));
        }
    }

    switch (is_weptool(obj) ? WEAPON_CLASS : obj->oclass) {
    case AMULET_CLASS:
        if(obj->owornmask & W_AMUL)
            Strcat(bp, " (being worn)");
        break;

    case ARMOR_CLASS:
        if (obj->owornmask & W_ARMOR) {
            Strcat(bp, (obj == uskin) ? " (embedded in your skin)" :
                       /* in case of perm_invent update while Wear/Takeoff
                          is in progress; check doffing() before donning()
                          because donning() returns True for both cases */
                       doffing(obj) ? " (being doffed)" :
                       donning(obj) ? " (being donned)" :
                       " (being worn)");
            /* slippery fingers is an intrinsic condition of the hero
               rather than extrinsic condition of objects, but gloves
               are described as slippery when hero has slippery fingers */
            if (obj == uarmg && Glib) {
                /* just appended "(something)",
                 * change to "(something; slippery)" */
                Strcpy(rindex(bp, ')'), "; slippery)");
            }
        }
        /* fall through */

    case WEAPON_CLASS:
        if (ispoisoned) {
            Strcat(prefix, "poisoned ");
        }
        add_erosion_words(obj, prefix, dump_ID_flag);
        if (known || do_known) {
            Sprintf(eos(prefix), "%s%s%s ",
                    do_known ? "[" : "", sitoa(obj->spe), do_known ? "]" : "");
        }
        break;

    case TOOL_CLASS:
        if(obj->owornmask & (W_TOOL /* blindfold */
#ifdef STEED
                             | W_SADDLE
#endif
                             )) {
            Strcat(bp, " (being worn)");
            break;
        }
        if (obj->otyp == LEASH && obj->leashmon != 0) {
            struct monst *mlsh = find_mid(obj->leashmon, FM_FMON);

            if (!mlsh) {
                impossible("leashed monster not on this level");
                obj->leashmon = 0;
            } else {
                Sprintf(eos(bp), " (attached to %s)",
                        noit_mon_nam(mlsh));
            }
            break;
        }
        if (obj->otyp == CANDELABRUM_OF_INVOCATION) {
            if (!obj->spe)
                Strcpy(tmpbuf, "no");
            else
                Sprintf(tmpbuf, "%d", obj->spe);
            Sprintf(eos(bp), " (%s candle%s%s)",
                    tmpbuf, plur(obj->spe),
                    !obj->lamplit ? " attached" : ", lit");
            break;
        } else if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
                   obj->otyp == BRASS_LANTERN || Is_candle(obj)) {
            if (Is_candle(obj) &&
                obj->age < 20L * (long)objects[obj->otyp].oc_cost)
                Strcat(prefix, "partly used ");
            if(obj->lamplit)
                Strcat(bp, " (lit)");
            break;
        }
        if(objects[obj->otyp].oc_charged)
            goto charges;
        break;

    case SPBOOK_CLASS:
#define MAX_SPELL_STUDY 3 /* spell.c */
        if(dump_ID_flag && obj->spestudied > MAX_SPELL_STUDY / 2)
            Strcat(prefix, "[faint] ");
        break;

    case WAND_CLASS:
        add_erosion_words(obj, prefix, dump_ID_flag);
charges:
        if (known || do_known) {
            Sprintf(eos(bp), " %s%d:%d%s", do_known ? "[(" : "(",
                    (int)obj->recharged, obj->spe, do_known ? ")]" : ")");
        }
        break;

    case POTION_CLASS:
        if (obj->otyp == POT_OIL && obj->lamplit)
            Strcat(bp, " (lit)");
        break;

    case RING_CLASS:
        add_erosion_words(obj, prefix, dump_ID_flag);
ring:
        if(obj->owornmask & W_RINGR) Strcat(bp, " (on right ");
        if(obj->owornmask & W_RINGL) Strcat(bp, " (on left ");
        if(obj->owornmask & W_RING) {
            Strcat(bp, body_part(HAND));
            Strcat(bp, ")");
        }
        if ((known || do_known) && objects[obj->otyp].oc_charged) {
            Sprintf(eos(prefix), "%s%s%s ",
                    do_known ? "[" : "", sitoa(obj->spe), do_known ? "]" : "");
        }
        break;

    case FOOD_CLASS:
        /* eat.c: edibility_prompts() */
        if (dump_ID_flag && obj->otyp == CORPSE && obj->corpsenm != PM_ACID_BLOB &&
            obj->corpsenm != PM_LIZARD && obj->corpsenm != PM_LICHEN) {
            long age = monstermoves - peek_at_iced_corpse_age(obj);
            long bucmod = obj->cursed ? 2 : obj->blessed ? -2 : 0;
            long mayberot = age / 10L + bucmod, surerot = age / 29L + bucmod;
            if (surerot > 5L) Strcat(prefix, "[very rotten] ");
            else if (mayberot > 5L) Strcat(prefix, "[rotten] ");
        }
        if (obj->otyp == CORPSE && obj->odrained) {
#ifdef WIZARD
            if (wizard && obj->oeaten < drainlevel(obj))
                Strcpy(tmpbuf, "over-drained ");
            else
#endif
            Sprintf(tmpbuf, "%sdrained ",
                    (obj->oeaten > drainlevel(obj)) ? "partly " : "");
        }
        else if (obj->oeaten)
            Strcpy(tmpbuf, "partly eaten ");
        else
            tmpbuf[0] = '\0';
        Strcat(prefix, tmpbuf);
        if (obj->otyp == CORPSE) {
            /* (quan == 1) => want corpse_xname() to supply article,
               (quan != 1) => already have count or "some" as prefix;
               "corpse" is already in the buffer returned by xname() */
            unsigned cxarg = (((obj->quan != 1L) ? 0 : CXN_ARTICLE) | CXN_NOCORPSE);
            char *cxstr = corpse_xname(obj, prefix, cxarg);
            Sprintf(prefix, "%s ", cxstr);
            /* avoid having doname(corpse) consume an extra obuf */
            releaseobuf(cxstr);
        } else if (obj->otyp == EGG) {
            if (dump_ID_flag && stale_egg(obj))
                Strcat(prefix, "[stale] ");
            if (omndx >= LOW_PM &&
                (known || mvitals[omndx].mvflags & MV_KNOWS_EGG)) {
                Strcat(prefix, mons[omndx].mname);
                Strcat(prefix, " ");
                if (obj->spe)
                    Strcat(bp, " (laid by you)");
            }
        }
        if (obj->otyp == MEAT_RING) goto ring;
        break;

    case BALL_CLASS:
    case CHAIN_CLASS:
        add_erosion_words(obj, prefix, dump_ID_flag);
        if(obj->owornmask & W_BALL)
            Strcat(bp, " (chained to you)");
        break;
    }

    if((obj->owornmask & W_WEP) && !mrg_to_wielded) {
        if (obj->quan != 1L) {
            Strcat(bp, " (wielded)");
        } else {
            const char *hand_s = body_part(HAND);

            if (bimanual(obj)) hand_s = makeplural(hand_s);
            /* note: Sting's glow message, if added, will insert text
               in front of "(weapon in hand)"'s closing paren */
            Sprintf(eos(bp), " (%sweapon in %s)",
                    (obj->otyp == AKLYS) ? "tethered " : "", hand_s);

            if (warn_obj_cnt && obj == uwep && (EWarn_of_mon & W_WEP)) {
                if (!Blind) {
                    /* we know bp[] ends with ')'; overwrite that */
                    Sprintf(eos(bp)-1, ", %s %s)",
                            glow_verb(warn_obj_cnt, TRUE),
                            glow_color(obj->oartifact));
                }
            }
        }
    }
    if (obj->owornmask & W_SWAPWEP) {
        if (u.twoweap)
            Sprintf(eos(bp), " (wielded in other %s)",
                    body_part(HAND));
        else
            Strcat(bp, " (alternate weapon; not wielded)");
    }
    if (obj->owornmask & W_QUIVER) {
        switch (obj->oclass) {
        case WEAPON_CLASS:
            if (is_ammo(obj)) {
                if (objects[obj->otyp].oc_skill == -P_BOW) {
                    /* Ammo for a bow */
                    Strcat(bp, " (in quiver)");
                    break;
                } else {
                    /* Ammo not for a bow */
                    Strcat(bp, " (in quiver pouch)");
                    break;
                }
            } else {
                /* Weapons not considered ammo */
                Strcat(bp, " (quivered)");
                break;
            }
        /* Small things and ammo not for a bow */
        case RING_CLASS:
        case AMULET_CLASS:
        case WAND_CLASS:
        case COIN_CLASS:
        case GEM_CLASS:
            Strcat(bp, " (in quiver pouch)");
            break;
        default: /* odd things */
            Strcat(bp, " (quivered)");
        }
    }

    /* treat 'restoring' like suppress_price because shopkeeper and
       bill might not be available yet while restore is in progress
       (objects won't normally be formatted during that time, but if
       'perm_invent' is enabled then they might be) */
    if (iflags.suppress_price || restoring) {
        ; /* don't attempt to obtain any stop pricing, even if 'with_price' */
    } else if (obj->unpaid) {
        xchar ox, oy;
        long quotedprice = unpaid_cost(obj, TRUE);
        struct monst *shkp = (struct monst *)0;

        Sprintf(eos(bp), " (unpaid, %ld %s)",
                quotedprice, currency(quotedprice));
    } else if (with_price) {
        /* price needs to be recalculated in case identification
         * changes the price e.g. with worthless glass */
        long price = get_cost_of_shop_item(obj);
        if (price > 0) {
            Sprintf(eos(bp), " (%ld %s)", price, currency(price));
        }
    }
    if (!strncmp(prefix, "a ", 2) &&
        (index(vowels, prefix[2] ? prefix[2] : *bp)
         || (dump_ID_flag && !strncmp(prefix+2, "[uncursed", 9)))
        && (*(prefix+2) || (strncmp(bp, "uranium", 7)
                            && strncmp(bp, "unicorn", 7)
                            && strncmp(bp, "eucalyptus", 10)))) {
        Strcpy(tmpbuf, prefix);
        Strcpy(prefix, "an ");
        Strcpy(prefix+3, tmpbuf+2);
    }
    /* merge bracketed attribs
       eg. [rustproof] [+1] -> [rustproof +1] */
    tmp = prefix;
    while ((tmp = strstr(tmp, "] ["))) {
        *tmp = ' ';
        memmove(tmp + 1, tmp + 3, strlen(tmp + 3) + 1);
    }
    bp = strprepend(bp, prefix);
    if (obj->otyp != SLIME_MOLD) {
        tmp = bp;
        while ((tmp = strstr(tmp, "] ["))) {
            *tmp = ' ';
            memmove(tmp + 1, tmp + 3, strlen(tmp + 3) + 1);
        }
        /* turn [(n:n)] wand charges into [n:n] */
        if ((tmp = strstr(bp, "[("))) {
            char *tmp2 = strstr(tmp, ")]");
            if (tmp2) {
                memmove(tmp2, tmp2 + 1, strlen(tmp2 + 1) + 1);
                memmove(tmp + 1, tmp + 2, strlen(tmp + 2) + 1);
            }
        }
    }
    long weight = display_weight(obj);
    if (weight > 0) {
        /* "aum" is stolen from Crawl's "Arbitrary Unit of Measure" */
        Sprintf (eos(bp), " (%d aum)", obj->owt);
    }

    return bp;
}

long
display_weight(struct obj *obj)
{
#ifdef SHOW_WEIGHT
    if (!flags.invweight) {
        return 0;
    }

    /* [max] weight inventory */
    if ((obj->otyp != BOULDER) || !throws_rocks (youmonst.data)) {
        if (is_stone(obj) &&
            (!objects[LOADSTONE].oc_name_known) &&
            (!objects[obj->otyp].oc_name_known || !obj->dknown)) {
            return 0;
        }
        if (Is_container(obj) && (!obj->cknown || !obj->dknown)) {
            return 0;
        }
        if (!Hallucination && flags.invweight) {
            return obj->owt;
        }
    }
#endif
    return 0;
}

/** Wrapper function for vanilla behaviour. */
char *
doname(obj)
register struct obj *obj;
{
    return doname_base(obj, FALSE);
}

/** Name of object including price. */
char *
doname_with_price(obj)
register struct obj *obj;
{
    return doname_base(obj, TRUE);
}

/* used from invent.c */
boolean
not_fully_identified(otmp)
struct obj *otmp;
{
    /* gold doesn't have any interesting attributes [yet?] */
    if (otmp->oclass == COIN_CLASS) return FALSE;   /* always fully ID'd */

    /* check fundamental ID hallmarks first */
    if (!otmp->known || !otmp->dknown ||
#ifdef MAIL
        (!otmp->bknown && otmp->otyp != SCR_MAIL) ||
#else
        !otmp->bknown ||
#endif
        !objects[otmp->otyp].oc_name_known) /* ?redundant? */
        return TRUE;
    if ((!otmp->cknown && (Is_container(otmp) || otmp->otyp == STATUE)) ||
        (!otmp->lknown && Is_box(otmp))) {
        return TRUE;
    }
    if (otmp->oartifact && undiscovered_artifact(otmp->oartifact))
        return TRUE;
    /* otmp->rknown is the only item of interest if we reach here */
    /*
     *  Note:  if a revision ever allows scrolls to become fireproof or
     *  rings to become shockproof, this checking will need to be revised.
     *  `rknown' ID only matters if xname() will provide the info about it.
     */
    if (otmp->rknown || (otmp->oclass != ARMOR_CLASS &&
                         otmp->oclass != WEAPON_CLASS &&
                         !is_weptool(otmp) && /* (redunant) */
                         otmp->oclass != BALL_CLASS)) /* (useless) */
        return FALSE;
    else    /* lack of `rknown' only matters for vulnerable objects */
        return (boolean)(is_rustprone(otmp) ||
                         is_corrodeable(otmp) ||
                         is_flammable(otmp));
}

/** format a corpse name (xname() omits monster type; doname() calls us);
   eatcorpse() also uses us for death reason when eating tainted glob */
char *
corpse_xname(otmp, adjective, cxn_flags)
struct obj *otmp;
const char *adjective;
unsigned cxn_flags; /* bitmask of CXN_xxx values */
{
    char *nambuf = nextobuf();
    int omndx = otmp->corpsenm;
    boolean ignore_quan = (cxn_flags & CXN_SINGULAR);
    /* suppress "the" from "the unique monster corpse" */
    boolean no_prefix = (cxn_flags & CXN_NO_PFX);
    /* include "the" for "the woodchuck corpse */
    boolean the_prefix = (cxn_flags & CXN_PFX_THE);
    /* include "an" for "an ogre corpse */
    boolean any_prefix = (cxn_flags & CXN_ARTICLE);
    /* leave off suffix (do_name() appends "corpse" itself) */
    boolean omit_corpse = (cxn_flags & CXN_NOCORPSE);
    boolean possessive = FALSE;
    boolean glob = (otmp->otyp != CORPSE && otmp->globby);
    const char *mname;

    if (glob) {
        mname = OBJ_NAME(objects[otmp->otyp]); /* "glob of <monster>" */
    } else if (omndx == NON_PM) { /* paranoia */
        mname = "thing";
        /* [Possible enhancement:  check whether corpse has monster traits
            attached in order to use priestname() for priests and minions.] */
    } else if (omndx == PM_ALIGNED_PRIEST) {
        /* avoid "aligned priest"; it just exposes internal details */
        mname = "priest";
    } else {
        mname = mons[omndx].mname;
        if (the_unique_pm(&mons[omndx]) || type_is_pname(&mons[omndx])) {
            mname = s_suffix(mname);
            possessive = TRUE;
            /* don't precede personal name like "Medusa" with an article */
            if (type_is_pname(&mons[omndx])) {
                no_prefix = TRUE;
            } else if (the_unique_pm(&mons[omndx]) && !no_prefix) {
                /* always precede non-personal unique monster name like
                   "Oracle" with "the" unless explicitly overridden */
                the_prefix = TRUE;
            }
        }
    }
    if (no_prefix) {
        the_prefix = any_prefix = FALSE;
    } else if (the_prefix) {
        any_prefix = FALSE; /* mutually exclusive */
    }

    *nambuf = '\0';
    /* can't use the() the way we use an() below because any capitalized
       Name causes it to assume a personal name and return Name as-is;
       that's usually the behavior wanted, but here we need to force "the"
       to precede capitalized unique monsters (pnames are handled above) */
    if (the_prefix) {
        Strcat(nambuf, "the ");
    }

    if (!adjective || !*adjective) {
        /* normal case:  newt corpse */
        Strcat(nambuf, mname);
    } else {
        /* adjective positioning depends upon format of monster name */
        if (possessive) {
            /* Medusa's cursed partly eaten corpse */
            Sprintf(eos(nambuf), "%s %s", mname, adjective);
        } else {
            /* cursed partly eaten troll corpse */
            Sprintf(eos(nambuf), "%s %s", adjective, mname);
        }
        /* in case adjective has a trailing space, squeeze it out */
        mungspaces(nambuf);
        /* doname() might include a count in the adjective argument;
           if so, don't prepend an article */
        if (digit(*adjective)) {
            any_prefix = FALSE;
        }
    }

    if (glob) {
        ; /* omit_corpse doesn't apply; quantity is always 1 */
    } else if (!omit_corpse) {
        Strcat(nambuf, " corpse");
        /* makeplural(nambuf) => append "s" to "corpse" */
        if (otmp->quan > 1L && !ignore_quan) {
            Strcat(nambuf, "s");
            any_prefix = FALSE; /* avoid "a newt corpses" */
        }
    }

    /* it's safe to overwrite our nambuf after an() has copied
       its old value into another buffer */
    if (any_prefix) {
        Strcpy(nambuf, an(nambuf));
    }

    return nambuf;
}

/* xname, unless it's a corpse, then corpse_xname(obj, FALSE) */
char *
cxname(obj)
struct obj *obj;
{
    if (obj->otyp == CORPSE) {
        return corpse_xname(obj, (const char *) 0, CXN_NORMAL);
    }

    return xname(obj);
}
#ifdef SORTLOOT
/** like cxname, but ignores quantity */
char *
cxname_singular(obj)
struct obj *obj;
{
    if (obj->otyp == CORPSE) {
        return corpse_xname(obj, (const char *) 0, CXN_SINGULAR);
    }
    return xname_flags(obj, CXN_SINGULAR);
}
#endif /* SORTLOOT */

/** Returns the unidentified name of obj. */
char *
cxname_unidentified(obj)
struct obj *obj;
{
    int cxn_flags = CXN_UNIDENTIFIED | CXN_SINGULAR;
    if (obj->otyp == CORPSE) {
        return corpse_xname(obj, (const char *) 0, cxn_flags);
    }

    return xname_flags(obj, cxn_flags);
}

/* treat an object as fully ID'd when it might be used as reason for death */
char *
killer_xname(obj)
struct obj *obj;
{
    struct obj save_obj;
    unsigned save_ocknown;
    char *buf, *save_ocuname, *save_oname = (char *) 0;

    /* bypass object twiddling for artifacts */
    if (obj->oartifact) {
        return bare_artifactname(obj);
    }

    /* remember original settings for core of the object;
       oname and oattached extensions don't matter here--since they
       aren't modified they don't need to be saved and restored */
    save_obj = *obj;
    if (has_oname(obj)) {
        save_oname = ONAME(obj);
    }
    /* killer name should be more specific than general xname; however, exact
       info like blessed/cursed and rustproof makes things be too verbose */
    obj->known = obj->dknown = 1;
    obj->bknown = obj->rknown = obj->greased = 0;
    /* if character is a priest[ess], bknown will get toggled back on */
    if (obj->otyp != POT_WATER) {
        obj->blessed = obj->cursed = 0;
    } else {
        obj->bknown = 1; /* describe holy/unholy water as such */
    }
    /* "killed by poisoned <obj>" would be misleading when poison is
       not the cause of death and "poisoned by poisoned <obj>" would
       be redundant when it is, so suppress "poisoned" prefix */
    obj->opoisoned = 0;
    /* strip user-supplied name; artifacts keep theirs */
    if (!obj->oartifact && save_oname) {
        ONAME(obj) = (char *) 0;
    }
    /* temporarily identify the type of object */
    save_ocknown = objects[obj->otyp].oc_name_known;
    objects[obj->otyp].oc_name_known = 1;
    save_ocuname = objects[obj->otyp].oc_uname;
    objects[obj->otyp].oc_uname = 0; /* avoid "foo called bar" */

    /* format the object */
    if (obj->otyp == CORPSE) {
        buf = corpse_xname(obj, (const char *) 0, CXN_NORMAL);
    } else if (obj->otyp == SLIME_MOLD) {
        /* concession to "most unique deaths competition" in the annual
           devnull tournament, suppress player supplied fruit names because
           those can be used to fake other objects and dungeon features */
        buf = nextobuf();
        Sprintf(buf, "deadly slime mold%s", plur(obj->quan));
    } else {
        buf = xname(obj);
    }
    /* apply an article if appropriate; caller should always use KILLED_BY */
    if (obj->quan == 1L && !strstri(buf, "'s ") && !strstri(buf, "s' ")) {
        buf = (obj_is_pname(obj) || the_unique_obj(obj)) ? the(buf) : an(buf);
    }

    objects[obj->otyp].oc_name_known = save_ocknown;
    objects[obj->otyp].oc_uname = save_ocuname;
    *obj = save_obj; /* restore object's core settings */
    if (!obj->oartifact && save_oname) {
        ONAME(obj) = save_oname;
    }

    return buf;
}

/* xname,doname,&c with long results reformatted to omit some stuff */
char *
short_oname(obj, func, altfunc, lenlimit)
struct obj *obj;
char *FDECL((*func), (OBJ_P)),    /* main formatting routine */
     *FDECL((*altfunc), (OBJ_P)); /* alternate for shortest result */
unsigned lenlimit;
{
    struct obj save_obj;
    char unamebuf[12], onamebuf[12], *save_oname, *save_uname, *outbuf;

    outbuf = (*func)(obj);
    if ((unsigned) strlen(outbuf) <= lenlimit)
        return outbuf;

    /* shorten called string to fairly small amount */
    save_uname = objects[obj->otyp].oc_uname;
    if (save_uname && strlen(save_uname) >= sizeof unamebuf) {
        (void) strncpy(unamebuf, save_uname, sizeof unamebuf - 4);
        Strcpy(unamebuf + sizeof unamebuf - 4, "...");
        objects[obj->otyp].oc_uname = unamebuf;
        releaseobuf(outbuf);
        outbuf = (*func)(obj);
        objects[obj->otyp].oc_uname = save_uname; /* restore called string */
        if ((unsigned) strlen(outbuf) <= lenlimit)
            return outbuf;
    }

    /* shorten named string to fairly small amount */
    save_oname = has_oname(obj) ? ONAME(obj) : 0;
    if (save_oname && strlen(save_oname) >= sizeof onamebuf) {
        (void) strncpy(onamebuf, save_oname, sizeof onamebuf - 4);
        Strcpy(onamebuf + sizeof onamebuf - 4, "...");
        ONAME(obj) = onamebuf;
        releaseobuf(outbuf);
        outbuf = (*func)(obj);
        ONAME(obj) = save_oname; /* restore named string */
        if ((unsigned) strlen(outbuf) <= lenlimit)
            return outbuf;
    }

    /* shorten both called and named strings;
       unamebuf and onamebuf have both already been populated */
    if (save_uname && strlen(save_uname) >= sizeof unamebuf && save_oname
        && strlen(save_oname) >= sizeof onamebuf) {
        objects[obj->otyp].oc_uname = unamebuf;
        ONAME(obj) = onamebuf;
        releaseobuf(outbuf);
        outbuf = (*func)(obj);
        if ((unsigned) strlen(outbuf) <= lenlimit) {
            objects[obj->otyp].oc_uname = save_uname;
            ONAME(obj) = save_oname;
            return outbuf;
        }
    }

    /* still long; strip several name-lengthening attributes;
       called and named strings are still in truncated form */
    save_obj = *obj;
    obj->bknown = obj->rknown = obj->greased = 0;
    obj->oeroded = obj->oeroded2 = 0;
    releaseobuf(outbuf);
    outbuf = (*func)(obj);
    if (altfunc && (unsigned) strlen(outbuf) > lenlimit) {
        /* still long; use the alternate function (usually one of
           the jackets around minimal_xname()) */
        releaseobuf(outbuf);
        outbuf = (*altfunc)(obj);
    }
    /* restore the object */
    *obj = save_obj;
    if (save_oname)
        ONAME(obj) = save_oname;
    if (save_uname)
        objects[obj->otyp].oc_uname = save_uname;

    /* use whatever we've got, whether it's too long or not */
    return outbuf;
}

/*
 * Used if only one of a collection of objects is named (e.g. in eat.c).
 */
const char *
singular(otmp, func)
register struct obj *otmp;
char *FDECL((*func), (OBJ_P));
{
    long savequan;
#ifdef SHOW_WEIGHT
    unsigned saveowt;
#endif
    char *nam;

    /* Note: using xname for corpses will not give the monster type */
    if (otmp->otyp == CORPSE && func == xname) {
        func = cxname;
    }

    savequan = otmp->quan;
    otmp->quan = 1L;
#ifdef SHOW_WEIGHT
    saveowt = otmp->owt;
    otmp->owt = weight(otmp);
#endif
    nam = (*func)(otmp);
    otmp->quan = savequan;
#ifdef SHOW_WEIGHT
    otmp->owt = saveowt;
#endif
    return nam;
}

/** pick "", "a ", or "an " as article for 'str'; used by an() and doname() */
static char *
just_an(outbuf, str)
char *outbuf;
const char *str;
{
    char c0;

    *outbuf = '\0';
    c0 = lowc(*str);
    if (!str[1]) {
        /* single letter; might be used for named fruit */
        Strcpy(outbuf, index("aefhilmnosx", c0) ? "an " : "a ");
    } else if (!strncmpi(str, "the ", 4) ||
               !strcmpi(str, "molten lava") ||
               !strcmpi(str, "iron bars") ||
               !strcmpi(str, "ice")) {
        ; /* no article */
    } else {
        if ((index(vowels, c0) &&
             strncmpi(str, "one-", 4) &&
             strncmpi(str, "eucalyptus", 10) &&
             strncmpi(str, "unicorn", 7) &&
             strncmpi(str, "uranium", 7) &&
             strncmpi(str, "useful", 6)) ||
            (index("x", c0) &&
             !index(vowels, lowc(str[1])))) {
            Strcpy(outbuf, "an ");
        } else {
            Strcpy(outbuf, "a ");
        }
    }
    return outbuf;
}

char *
an(str)
const char *str;
{
    char *buf = nextobuf();

    if (!str || !*str) {
        impossible("Alphabet soup: 'an(%s)'.", str ? "\"\"" : "<null>");
        return strcpy(buf, "an []");
    }
    (void) just_an(buf, str);
    return strcat(buf, str);
}

char *
An(str)
const char *str;
{
    char *tmp = an(str);

    *tmp = highc(*tmp);
    return tmp;
}

/*
 * Prepend "the" if necessary; assumes str is a subject derived from xname.
 * Use type_is_pname() for monster names, not the().  the() is idempotent.
 */
char *
the(str)
const char *str;
{
    char *buf = nextobuf();
    boolean insert_the = FALSE;

    if (!str || !*str) {
        impossible("Alphabet soup: 'the(%s)'.", str ? "\"\"" : "<null>");
        return strcpy(buf, "the []");
    }
    if (!strncmpi(str, "the ", 4)) {
        buf[0] = lowc(*str);
        Strcpy(&buf[1], str+1);
        return buf;
    } else if (*str < 'A' || *str > 'Z' ||
               /* treat named fruit as not a proper name, even if player
                  has assigned a capitalized proper name as his/her fruit */
               fruit_from_name(str, TRUE, (int *) 0)) {
        /* not a proper name, needs an article */
        insert_the = TRUE;
    } else {
        /* Probably a proper name, might not need an article */
        register char *tmp, *named, *called;
        int l;

        /* some objects have capitalized adjectives in their names */
        if (((tmp = rindex(str, ' ')) || (tmp = rindex(str, '-'))) &&
           (tmp[1] < 'A' || tmp[1] > 'Z')) {
            insert_the = TRUE;
        } else if (tmp && index(str, ' ') < tmp) { /* has spaces */
            /* it needs an article if the name contains "of" */
            tmp = strstri(str, " of ");
            named = strstri(str, " named ");
            called = strstri(str, " called ");
            if (called && (!named || called < named)) named = called;

            if (tmp && (!named || tmp < named)) /* found an "of" */
                insert_the = TRUE;
            /* stupid special case: lacks "of" but needs "the" */
            else if (!named && (l = strlen(str)) >= 31 &&
                     !strcmp(&str[l - 31], "Platinum Yendorian Express Card"))
                insert_the = TRUE;
        }
    }
    if (insert_the)
        Strcpy(buf, "the ");
    else
        buf[0] = '\0';
    Strcat(buf, str);

    return buf;
}

char *
The(str)
const char *str;
{
    char *tmp = the(str);

    *tmp = highc(*tmp);
    return tmp;
}

/* returns "count cxname(otmp)" or just cxname(otmp) if count == 1 */
char *
aobjnam(otmp, verb)
struct obj *otmp;
const char *verb;
{
    char *bp = cxname(otmp);
    char prefix[PREFIX];

    if (otmp->quan != 1L) {
        Sprintf(prefix, "%ld ", otmp->quan);
        bp = strprepend(bp, prefix);
    }

    if (verb) {
        Strcat(bp, " ");
        Strcat(bp, otense(otmp, verb));
    }
    return(bp);
}

/* combine yname and aobjnam eg "your count cxname(otmp)" */
char *
yobjnam(obj, verb)
struct obj *obj;
const char *verb;
{
    char *s = aobjnam(obj, verb);

    /* leave off "your" for most of your artifacts, but prepend
     * "your" for unique objects and "foo of bar" quest artifacts */
    if (!carried(obj) || !obj_is_pname(obj)
        || obj->oartifact >= ART_HEART_OF_AHRIMAN) {
        char *outbuf = shk_your(nextobuf(), obj);
        size_t outbuf_len = strlen(outbuf);
        if ((outbuf[outbuf_len] != ' ') && (outbuf_len < BUFSZ-2) &&
            (s[0] && s[0] != ' ')) {
            strncat(outbuf, " ", 1);
        }
        int space_left = BUFSZ - 1 - strlen(outbuf);

        s = strncat(outbuf, s, space_left);
    }
    return s;
}

/* combine Yname2 and aobjnam eg "Your count cxname(otmp)" */
char *
Yobjnam2(obj, verb)
struct obj *obj;
const char *verb;
{
    register char *s = yobjnam(obj, verb);

    *s = highc(*s);
    return s;
}

/* like aobjnam, but prepend "The", not count, and use xname */
char *
Tobjnam(otmp, verb)
struct obj *otmp;
const char *verb;
{
    char *bp = The(xname(otmp));

    if (verb) {
        Strcat(bp, " ");
        Strcat(bp, otense(otmp, verb));
    }
    return(bp);
}

/* return form of the verb (input plural) if xname(otmp) were the subject */
char *
otense(otmp, verb)
struct obj *otmp;
const char *verb;
{
    char *buf;

    /*
     * verb is given in plural (without trailing s).  Return as input
     * if the result of xname(otmp) would be plural.  Don't bother
     * recomputing xname(otmp) at this time.
     */
    if (!is_plural(otmp))
        return vtense((char *)0, verb);

    buf = nextobuf();
    Strcpy(buf, verb);
    return buf;
}

/* various singular words that vtense would otherwise categorize as plural */
static const char * const special_subjs[] = {
    "erinys",
    "manes",        /* this one is ambiguous */
    "Cyclops",
    "Hippocrates",
    "Pelias",
    "aklys",
    "amnesia",
    "paralysis",
    "detect monsters",
    "shape changers",
    "nemesis",
    0
    /* note: "detect monsters" and "shape changers" are normally
       caught via "<something>(s) of <whatever>", but they can be
       wished for using the shorter form, so we include them here
       to accommodate usage by makesingular during wishing */
};

/* return form of the verb (input plural) for present tense 3rd person subj */
char *
vtense(subj, verb)
const char *subj;
const char *verb;
{
    char *buf = nextobuf();
    int len, ltmp;
    const char *sp, *spot;
    const char * const *spec;

    /*
     * verb is given in plural (without trailing s).  Return as input
     * if subj appears to be plural.  Add special cases as necessary.
     * Many hard cases can already be handled by using otense() instead.
     * If this gets much bigger, consider decomposing makeplural.
     * Note: monster names are not expected here (except before corpse).
     *
     * special case: allow null sobj to get the singular 3rd person
     * present tense form so we don't duplicate this code elsewhere.
     */
    if (subj) {
        if (!strncmpi(subj, "a ", 2) || !strncmpi(subj, "an ", 3))
            goto sing;
        spot = (const char *)0;
        for (sp = subj; (sp = index(sp, ' ')) != 0; ++sp) {
            if (!strncmp(sp, " of ", 4) ||
                !strncmp(sp, " from ", 6) ||
                !strncmp(sp, " called ", 8) ||
                !strncmp(sp, " named ", 7) ||
                !strncmp(sp, " labeled ", 9)) {
                if (sp != subj) spot = sp - 1;
                break;
            }
        }
        len = (int) strlen(subj);
        if (!spot) spot = subj + len - 1;

        /*
         * plural: anything that ends in 's', but not '*us' or '*ss'.
         * Guess at a few other special cases that makeplural creates.
         */
        if ((*spot == 's' && spot != subj &&
             (*(spot-1) != 'u' && *(spot-1) != 's')) ||
            ((spot - subj) >= 4 && !strncmp(spot-3, "eeth", 4)) ||
            ((spot - subj) >= 3 && !strncmp(spot-3, "feet", 4)) ||
            ((spot - subj) >= 2 && !strncmp(spot-1, "ia", 2)) ||
            ((spot - subj) >= 2 && !strncmp(spot-1, "ae", 2))) {
            /* check for special cases to avoid false matches */
            len = (int)(spot - subj) + 1;
            for (spec = special_subjs; *spec; spec++) {
                ltmp = strlen(*spec);
                if (len == ltmp && !strncmpi(*spec, subj, len)) goto sing;
                /* also check for <prefix><space><special_subj>
                   to catch things like "the invisible erinys" */
                if (len > ltmp && *(spot - ltmp) == ' ' &&
                    !strncmpi(*spec, spot - ltmp + 1, ltmp)) goto sing;
            }

            return strcpy(buf, verb);
        }
        /*
         * 3rd person plural doesn't end in telltale 's';
         * 2nd person singular behaves as if plural.
         */
        if (!strcmpi(subj, "they") || !strcmpi(subj, "you"))
            return strcpy(buf, verb);
    }

sing:
    len = strlen(verb);
    spot = verb + len - 1;

    if (!strcmp(verb, "are"))
        Strcpy(buf, "is");
    else if (!strcmp(verb, "have"))
        Strcpy(buf, "has");
    else if (index("zxs", *spot) ||
             (len >= 2 && *spot=='h' && index("cs", *(spot-1))) ||
             (len == 2 && *spot == 'o')) {
        /* Ends in z, x, s, ch, sh; add an "es" */
        Strcpy(buf, verb);
        Strcat(buf, "es");
    } else if (*spot == 'y' && (!index(vowels, *(spot-1)))) {
        /* like "y" case in makeplural */
        Strcpy(buf, verb);
        Strcpy(buf + len - 1, "ies");
    } else {
        Strcpy(buf, verb);
        Strcat(buf, "s");
    }

    return buf;
}

/* capitalized variant of doname() */
char *
Doname2(obj)
register struct obj *obj;
{
    register char *s = doname(obj);

    *s = highc(*s);
    return(s);
}

/* returns "your xname(obj)" or "Foobar's xname(obj)" or "the xname(obj)" */
char *
yname(obj)
struct obj *obj;
{
    char *outbuf = nextobuf();
    char *s = shk_your(outbuf, obj);    /* assert( s == outbuf ); */
    int space_left = BUFSZ - strlen(s) - sizeof " ";

    return strncat(strcat(s, " "), cxname(obj), space_left);
}

/* capitalized variant of yname() */
char *
Yname2(obj)
struct obj *obj;
{
    char *s = yname(obj);

    *s = highc(*s);
    return s;
}

/* returns "your minimal_xname(obj)"
 * or "Foobar's minimal_xname(obj)"
 * or "the minimal_xname(obj)"
 */
char *
ysimple_name(obj)
struct obj *obj;
{
    char *outbuf = nextobuf();
    char *s = shk_your(outbuf, obj); /* assert( s == outbuf ); */
    int space_left = BUFSZ - 1 - strlen(s);

    return strncat(strncat(s, " ", 1), minimal_xname(obj), space_left);
}

/* capitalized variant of ysimple_name() */
char *
Ysimple_name2(obj)
struct obj *obj;
{
    char *s = ysimple_name(obj);

    *s = highc(*s);
    return s;
}

/* "scroll" or "scrolls" */
char *
simpleonames(obj)
struct obj *obj;
{
    char *simpleoname = minimal_xname(obj);

    if (obj->quan != 1L) {
        simpleoname = makeplural(simpleoname);
    }
    return simpleoname;
}

/* "a scroll" or "scrolls"; "a silver bell" or "the Bell of Opening" */
char *
ansimpleoname(obj)
struct obj *obj;
{
    char *simpleoname = simpleonames(obj);
    int otyp = obj->otyp;

    /* prefix with "the" if a unique item, or a fake one imitating same,
       has been formatted with its actual name (we let typename() handle
       any `known' and `dknown' checking necessary) */
    if (otyp == FAKE_AMULET_OF_YENDOR) {
        otyp = AMULET_OF_YENDOR;
    }
    if (objects[otyp].oc_unique
        && !strcmp(simpleoname, OBJ_NAME(objects[otyp]))) {
        return the(simpleoname);
    }

    /* simpleoname is singular if quan==1, plural otherwise */
    if (obj->quan == 1L) {
        simpleoname = an(simpleoname);
    }
    return simpleoname;
}

/* "the scroll" or "the scrolls" */
char *
thesimpleoname(obj)
struct obj *obj;
{
    char *simpleoname = simpleonames(obj);

    return the(simpleoname);
}

/* artifact's name without any object type or known/dknown/&c feedback */
char *
bare_artifactname(obj)
struct obj *obj;
{
    char *outbuf;

    if (obj->oartifact) {
        outbuf = nextobuf();
        Strcpy(outbuf, artiname(obj->oartifact));
        if (!strncmp(outbuf, "The ", 4))
            outbuf[0] = lowc(outbuf[0]);
    } else {
        outbuf = xname(obj);
    }
    return outbuf;
}

static const char *wrp[] = {
    "wand", "ring", "potion", "scroll", "gem", "amulet",
    "spellbook", "spell book",
    /* for non-specific wishes */
    "weapon", "armor", "armour", "tool", "food", "comestible",
};
static const char wrpsym[] = {
    WAND_CLASS, RING_CLASS, POTION_CLASS, SCROLL_CLASS, GEM_CLASS,
    AMULET_CLASS, SPBOOK_CLASS, SPBOOK_CLASS,
    WEAPON_CLASS, ARMOR_CLASS, ARMOR_CLASS, TOOL_CLASS, FOOD_CLASS,
    FOOD_CLASS
};

struct sing_plur {
    const char *sing, *plur;
};

/* word pairs that don't fit into formula-based transformations;
   also some suffices which have very few--often one--matches or
   which aren't systematically reversible (knives, staves) */
static struct sing_plur one_off[] = {
    { "child",
      "children" },      /* (for wise guys who give their food funny names) */
    { "cubus", "cubi" }, /* in-/suc-cubus */
    { "culus", "culi" }, /* homunculus */
    { "djinni", "djinn" },
    { "erinys", "erinyes" },
    { "foot", "feet" },
    { "fungus", "fungi" },
    { "goose", "geese" },
    { "knife", "knives" },
    { "labrum", "labra" }, /* candelabrum */
    { "louse", "lice" },
    { "mouse", "mice" },
    { "mumak", "mumakil" },
    { "nemesis", "nemeses" },
    { "ovum", "ova" },
    { "ox", "oxen" },
    { "passerby", "passersby" },
    { "rtex", "rtices" }, /* vortex */
    { "serum", "sera" },
    { "staff", "staves" },
    { "tooth", "teeth" },
    { 0, 0 }
};

static const char *const as_is[] = {
    /* makesingular() leaves these plural due to how they're used */
    "boots",   "shoes",     "gloves",    "lenses",   "scales",
    "eyes",    "gauntlets", "iron bars",
    /* both singular and plural are spelled the same */
    "bison",   "deer",      "elk",       "fish",      "fowl",
    "tuna",    "yaki",      "-hai",      "krill",     "manes",
    "moose",   "ninja",     "sheep",     "ronin",     "roshi",
    "shito",   "tengu",     "ki-rin",    "Nazgul",    "gunyoki",
    "piranha", "samurai",   "shuriken", 0,
    /* Note:  "fish" and "piranha" are collective plurals, suitable
       for "wiped out all <foo>".  For "3 <foo>", they should be
       "fishes" and "piranhas" instead.  We settle for collective
       variant instead of attempting to support both. */
};

/* singularize/pluralize decisions common to both makesingular & makeplural */
static boolean
singplur_lookup(basestr, endstring, to_plural, alt_as_is)
char *basestr, *endstring;    /* base string, pointer to eos(string) */
boolean to_plural;            /* true => makeplural, false => makesingular */
const char *const *alt_as_is; /* another set like as_is[] */
{
    const struct sing_plur *sp;
    const char *same, *other, *const *as;
    int al;
    int baselen = strlen(basestr);

    for (as = as_is; *as; ++as) {
        al = (int) strlen(*as);
        if (!BSTRCMPI(basestr, endstring - al, *as)) {
            return TRUE;
        }
    }
    if (alt_as_is) {
        for (as = alt_as_is; *as; ++as) {
            al = (int) strlen(*as);
            if (!BSTRCMPI(basestr, endstring - al, *as)) {
                return TRUE;
            }
        }
    }

   /* Leave "craft" as a suffix as-is (aircraft, hovercraft);
      "craft" itself is (arguably) not included in our likely context */
   if ((baselen > 5) && (!BSTRCMPI(basestr, endstring - 5, "craft"))) {
       return TRUE;
   }
   /* avoid false hit on one_off[].plur == "lice" or .sing == "goose";
       if more of these turn up, one_off[] entries will need to flagged
       as to which are whole words and which are matchable as suffices
       then matching in the loop below will end up becoming more complex */
    if (!strcmpi(basestr, "slice") || !strcmpi(basestr, "mongoose")) {
        if (to_plural) {
            Strcasecpy(endstring, "s");
        }
        return TRUE;
    }
    /* skip "ox" -> "oxen" entry when pluralizing "<something>ox"
       unless it is muskox */
    if (to_plural &&
         baselen > 2 &&
         !strcmpi(endstring - 2, "ox") &&
         !(baselen > 5 && !strcmpi(endstring - 6, "muskox"))) {
        /* "fox" -> "foxes" */
        Strcasecpy(endstring, "es");
        return TRUE;
    }
    if (to_plural) {
        if (baselen > 2 && !strcmpi(endstring - 3, "man") && badman(basestr, to_plural)) {
            Strcasecpy(endstring, "s");
            return TRUE;
        }
    } else {
        if (baselen > 2 && !strcmpi(endstring - 3, "men") && badman(basestr, to_plural)) {
            return TRUE;
        }
    }
    for (sp = one_off; sp->sing; sp++) {
        /* check whether endstring already matches */
        same = to_plural ? sp->plur : sp->sing;
        al = (int) strlen(same);
        if (!BSTRCMPI(basestr, endstring - al, same)) {
            return TRUE; /* use as-is */
        }
        /* check whether it matches the inverse; if so, transform it */
        other = to_plural ? sp->sing : sp->plur;
        al = (int) strlen(other);
        if (!BSTRCMPI(basestr, endstring - al, other)) {
            Strcasecpy(endstring - al, same);
            return TRUE; /* one_off[] transformation */
        }
    }
    return FALSE;
}

/* searches for common compounds, ex. lump of royal jelly */
static char *
singplur_compound(str)
char *str;
{
    /* if new entries are added, be sure to keep compound_start[] in sync */
    static const char *const compounds[] =
        {
          " of ",     " labeled ", " called ",
          " named ",  " above", /* lurkers above */
          " versus ", " from ",    " in ",
          " on ",     " a la ",    " with", /* " with "? */
          " de ",     " d'",       " du ",
          "-in-",     "-at-",      0
        }, /* list of first characters for all compounds[] entries */
        compound_start[] = " -";

    const char *const *cmpd;
    char *p;

    for (p = str; *p; ++p) {
        /* substring starting at p can only match if *p is found
           within compound_start[] */
        if (!index(compound_start, *p)) {
            continue;
        }

        /* check current substring against all words in the compound[] list */
        for (cmpd = compounds; *cmpd; ++cmpd) {
            if (!strncmpi(p, *cmpd, (int) strlen(*cmpd))) {
                return p;
            }
        }
    }
    /* wasn't recognized as a compound phrase */
    return 0;
}

/* Plural routine; once upon a time it may have been chiefly used for
 * user-defined fruits, but it is now used extensively throughout the
 * program.
 *
 * For fruit, we have to try to account for everything reasonable the
 * player has; something unreasonable can still break the code.
 * However, it's still a lot more accurate than "just add an 's' at the
 * end", which Rogue uses...
 *
 * Also used for plural monster names ("Wiped out all homunculi." or the
 * vanquished monsters list) and body parts.  A lot of unique monsters have
 * names which get mangled by makeplural and/or makesingular.  They're not
 * genocidable, and vanquished-mon handling does its own special casing
 * (for uniques who've been revived and re-killed), so we don't bother
 * trying to get those right here.
 *
 * Also misused by muse.c to convert 1st person present verbs to 2nd person.
 * 3.6.0: made case-insensitive.
 */
char *
makeplural(oldstr)
const char *oldstr;
{
    /* Note: cannot use strcmpi here -- it'd give MATZot, CAVEMeN,... */
    register char *spot;
    char *str = nextobuf();
    char lo_c;
    const char *excess = (char *)0;
    int len;

    if (oldstr) {
        while (*oldstr==' ') oldstr++;
    }
    if (!oldstr || !*oldstr) {
        warning("plural of null?");
        Strcpy(str, "s");
        return str;
    }
    Strcpy(str, oldstr);

    /*
     * Skip changing "pair of" to "pairs of".  According to Webster, usual
     * English usage is use pairs for humans, e.g. 3 pairs of dancers,
     * and pair for objects and non-humans, e.g. 3 pair of boots.  We don't
     * refer to pairs of humans in this game so just skip to the bottom.
     */
    if (!strncmpi(str, "pair of ", 8)) {
        goto bottom;
    }

    /* look for "foo of bar" so that we can focus on "foo" */
    if ((spot = singplur_compound(str)) != 0) {
        excess = oldstr + (int) (spot - str);
        *spot = '\0';
    } else {
        spot = eos(str);
    }

    spot--;
    while (spot > str && *spot == ' ') {
        spot--; /* Strip blanks from end */
    }
    *(spot + 1) = '\0';
    /* Now spot is the last character of the string */

    len = strlen(str);

    /* Single letters */
    if (len==1 || !letter(*spot)) {
        Strcpy(spot+1, "'s");
        goto bottom;
    }

    /* dispense with some words which don't need pluralization */
    static const char *const already_plural[] = {
        "ae",  /* algae, larvae, &c */
        "matzot", 0,
    };

    /* spot+1: synch up with makesingular's usage */
    if (singplur_lookup(str, spot+1, TRUE, already_plural)) {
        goto bottom;
    }

    /* more of same, but not suitable for blanket loop checking */
    if ((len == 2 && !strcmpi(str, "ya")) || (len >= 3 && !strcmpi(spot - 2, " ya"))) {
        goto bottom;
    }

    /* man/men ("Wiped out all cavemen.") */
    if (len >= 3 && !strcmpi(spot-2, "man") &&
        /* exclude shamans and humans etc */
         !badman(str, TRUE)) {
        Strcasecpy(spot-1, "en");
        goto bottom;
    }
    /* (staff handled via one_off[]) */
    if (lowc(*spot) == 'f') {
        lo_c = lowc(*(spot - 1));
        if (len >= 3 && !strcmpi(spot - 2, "erf")) {
            /* avoid "nerf" -> "nerves", "serf" -> "serves" */
            ; /* fall through to default (append 's') */
        } else if (index("lr", lo_c) || index(vowels, lo_c)) {
            /* [aeioulr]f to [aeioulr]ves */
            Strcasecpy(spot, "ves");
            goto bottom;
        }
    }
    /* ium/ia (mycelia, baluchitheria) */
    if (len >= 3 && !strcmpi(spot-2, "ium")) {
        Strcasecpy(spot - 2, "ia");
        goto bottom;
    }
    /* algae, larvae, hyphae (another fungus part) */
    if ((len >= 4 && !strcmpi(spot-3, "alga")) ||
        (len >= 5 && (!strcmpi(spot-4, "hypha") ||
                      !strcmpi(spot-4, "larva"))) ||
        (len >= 6 && !strcmpi(spot-5, "amoeba")) ||
        (len >= 8 && (!strcmpi(spot-7, "vertebra")))) {
        /* a to ae */
        Strcasecpy(spot + 1, "e");
        goto bottom;
    }
    /* fungus/fungi, homunculus/homunculi, but buses, lotuses, wumpuses */
    if (len > 3 && !strcmpi(spot-1, "us") &&
        (len < 5 || (strcmpi(spot-4, "lotus") &&
                     (len < 6 || strcmpi(spot-5, "wumpus"))))) {
        Strcasecpy(spot - 1, "i");
        goto bottom;
    }
    /* sis/ses (nemesis) */
    if (len >= 3 && !strcmpi(spot-2, "sis")) {
        Strcasecpy(spot-1, "es");
        goto bottom;
    }
    /* matzoh/matzot, possible food name */
    if (len >= 6 &&
         (!strcmpi(spot-5, "matzoh") || !strcmpi(spot-5, "matzah"))) {
        Strcasecpy(spot - 1, "ot"); /* oh/ah -> ot */
        goto bottom;
    }
    if (len >= 5 &&
         (!strcmpi(spot-4, "matzo") || !strcmpi(spot-4, "matza"))) {
        Strcasecpy(spot, "ot"); /* o/a -> ot */
        goto bottom;
    }

    /* note: -eau/-eaux (gateau, bordeau...) */
    /* note: ox/oxen, VAX/VAXen, goose/geese */

    lo_c = lowc(*spot);

    /* codex/spadix/neocortex and the like */
    if (len >= 5 &&
         (!strcmpi(spot-2, "dex") ||
          !strcmpi(spot-2, "dix") ||
          !strcmpi(spot-2, "tex")) &&
           /* indices would have been ok too, but stick with indexes */
         (strcmpi(spot-4,"index") != 0)) {
        Strcasecpy(spot-1, "ices"); /* ex|ix -> ices */
        goto bottom;
    }
    /* Ends in z, x, s, ch, sh; add an "es" */
    if (index("zxs", lo_c) ||
         (len >= 2 && lo_c == 'h' && index("cs", lowc(*(spot-1))) &&
            /* 21st century k-sound */
             !(len >= 4 &&
                 ((lowc(*(spot-2)) == 'e' &&
                    index("mt", lowc(*(spot-3)))) ||
                  (lowc(*(spot-2)) == 'o' &&
                    index("lp", lowc(*(spot-3))))))) ||
        /* Kludge to get "tomatoes" and "potatoes" right */
         (len >= 4 && !strcmpi(spot-2, "ato")) ||
         (len >= 5 && !strcmpi(spot-4, "dingo"))) {
        Strcasecpy(spot+1, "es"); /* append es */
        goto bottom;
    }
    /* Ends in y preceded by consonant (note: also "qu") change to "ies" */
    if (*spot == 'y' &&
         (!index(vowels, lowc(*(spot-1))))) {
        Strcasecpy(spot, "ies"); /* y -> ies */
        goto bottom;
    }

    /* Default: append an 's' */
    Strcasecpy(spot + 1, "s");

bottom:
    if (excess) {
        Strcat(str, excess);
    }
    return str;
}

struct o_range {
    const char *name, oclass;
    int f_o_range, l_o_range;
};

/* wishable subranges of objects */
STATIC_OVL NEARDATA const struct o_range o_ranges[] = {
    { "bag",    TOOL_CLASS,   SACK,       BAG_OF_TRICKS },
    { "lamp",   TOOL_CLASS,   OIL_LAMP,       MAGIC_LAMP },
    { "candle", TOOL_CLASS,   TALLOW_CANDLE,  WAX_CANDLE },
    { "horn",   TOOL_CLASS,   TOOLED_HORN,    HORN_OF_PLENTY },
    { "shield", ARMOR_CLASS,  SMALL_SHIELD,   SHIELD_OF_REFLECTION },
    { "hat", ARMOR_CLASS, FEDORA, DUNCE_CAP },
    { "helm",   ARMOR_CLASS,  ELVEN_LEATHER_HELM, HELM_OF_TELEPATHY },
    { "helmet", ARMOR_CLASS,  ELVEN_LEATHER_HELM, HELM_OF_TELEPATHY },
    { "gloves", ARMOR_CLASS,  LEATHER_GLOVES, GAUNTLETS_OF_DEXTERITY },
    { "gauntlets",  ARMOR_CLASS,  LEATHER_GLOVES, GAUNTLETS_OF_DEXTERITY },
    { "boots",  ARMOR_CLASS,  LOW_BOOTS,      LEVITATION_BOOTS },
    { "shoes",  ARMOR_CLASS,  LOW_BOOTS,      IRON_SHOES },
    { "cloak",  ARMOR_CLASS,  MUMMY_WRAPPING, CLOAK_OF_DISPLACEMENT },
#ifdef TOURIST
    { "shirt",  ARMOR_CLASS,  HAWAIIAN_SHIRT, T_SHIRT},
#endif
    { "dragon scales",
      ARMOR_CLASS,  GRAY_DRAGON_SCALES, YELLOW_DRAGON_SCALES },
    { "dragon scale mail",
      ARMOR_CLASS,  GRAY_DRAGON_SCALE_MAIL, YELLOW_DRAGON_SCALE_MAIL },
    { "sword",  WEAPON_CLASS, SHORT_SWORD,    KATANA },
#ifdef WIZARD
    { "venom",  VENOM_CLASS,  BLINDING_VENOM, FREEZING_ICE },
#endif
    { "gray stone", GEM_CLASS,    LUCKSTONE,      FLINT },
    { "grey stone", GEM_CLASS,    LUCKSTONE,      FLINT },
};

/*
 * Singularize a string the user typed in; this helps reduce the complexity
 * of readobjnam, and is also used in pager.c to singularize the string
 * for which help is sought.
 *
 * "Manes" is ambiguous: monster type (keep s), or horse body part (drop s)?
 * Its inclusion in as_is[]/special_subj[] makes it get treated as the former.
 *
 * A lot of unique monsters have names ending in s; plural, or singular
 * from plural, doesn't make much sense for them so we don't bother trying.
 * 3.6.0: made case-insensitive.
 */
char *
makesingular(oldstr)
const char *oldstr;
{
    register char *p, *bp;
    const char *excess = 0;
    char *str = nextobuf();

    if (oldstr) {
        while (*oldstr == ' ') {
            oldstr++;
        }
    }
    if (!oldstr || !*oldstr) {
        warning("singular of null?");
        str[0] = 0;
        return str;
    }

    bp = strcpy(str, oldstr);

    /* check for "foo of bar" so that we can focus on "foo" */
    if ((p = singplur_compound(bp)) != 0) {
        excess = oldstr + (int) (p - bp);
        *p = '\0';
    } else {
        p = eos(bp);
    }

    /* dispense with some words which don't need singularization */
    if (singplur_lookup(bp, p, FALSE, special_subjs)) {
        goto bottom;
    }

    /* remove -s or -es (boxes) or -ies (rubies) */
    if (p >= bp+1 && lowc(p[-1]) == 's') {
        if (p >= bp+2 && lowc(p[-2]) == 'e') {
            if (p >= bp+3 && lowc(p[-3]) == 'i') {
                if (!BSTRCMPI(bp, p-7, "cookies") ||
                    (!BSTRCMPI(bp, p-4, "pies") &&
                     /* avoid false match for "harpies" */
                     (p-4 == bp || p[-5] == ' ')) ||
                    /* alternate djinni/djinn spelling; not really needed */
                     (!BSTRCMPI(bp, p-6, "genies") &&
                      /* avoid false match for "progenies" */
                      (p-6 == bp || p[-7] == ' ')) ||
                     /* zombie */
                     !BSTRCMPI(bp, p-5, "mbies") ||
                     /* valkyrie */
                     !BSTRCMPI(bp, p-5, "yries")) {
                    goto mins;
                   }
                Strcasecpy(p-3, "y"); /* ies -> y */
                goto bottom;
            }
            /* wolves, but f to ves isn't fully reversible */
            if (p-4 >= bp &&
                 (index("lr", lowc(*(p-4))) || index(vowels, lowc(*(p-4)))) &&
                 !BSTRCMPI(bp, p-3, "ves")) {
                if (!BSTRCMPI(bp, p-6, "cloves") ||
                     !BSTRCMPI(bp, p-6, "nerves")) {
                    goto mins;
                }
                Strcasecpy(p-3, "f"); /* ves -> f */
                goto bottom;
            }
            /* note: nurses, axes but boxes */
            if (!BSTRCMPI(bp, p-4, "eses") ||
                 !BSTRCMPI(bp, p-4, "oxes") || /* boxes, foxes */
                 !BSTRCMPI(bp, p-4, "nxes") ||  /* lynxes */
                 !BSTRCMPI(bp, p-4, "ches") ||
                 !BSTRCMPI(bp, p-4, "uses") ||  /* lotuses */
                 !BSTRCMPI(bp, p-4, "sses") ||  /* priestesses */
                 !BSTRCMPI(bp, p-5, "atoes") ||  /* tomatoes */
                 !BSTRCMPI(bp, p-7, "dingoes") ||
                 !BSTRCMPI(bp, p-7, "Aleaxes")) {
                *(p-2) = '\0'; /* drop es */
                goto bottom;
            } /* else fall through to mins */


            /* ends in 's' but not 'es' */
        } else if (!BSTRCMPI(bp, p-2, "us")) { /* lotus, fungus... */
            /* but not these... */
            if (BSTRCMPI(bp, p-6, "tengus") &&
                 BSTRCMPI(bp, p-7, "hezrous"))
                goto bottom;
        } else if (!BSTRCMPI(bp, p-2, "ss")
                   || !BSTRCMPI(bp, p-5, " lens")
                   || (p-4 == bp && !strcmpi(p-4, "lens"))) {
            goto bottom;
        }
mins:
        p[-1] = '\0';

    } else {
        /* input doesn't end in 's' */

        if (!BSTRCMPI(bp, p-3, "men") && !badman(bp, FALSE)) {
            Strcasecpy(p-2, "an");
            goto bottom;
        }
        /* matzot -> matzo, algae -> alga */
        if (!BSTRCMPI(bp, p-6, "matzot") || !BSTRCMPI(bp, p-2, "ae")) {
            *(p-1) = '\0'; /* drop t/e */
            goto bottom;
        }
        /* balactheria -> balactherium */
        if (p-4 >= bp && !strcmpi(p-2, "ia") &&
             index("lr", lowc(*(p-3))) &&
             lowc(*(p-4)) == 'e') {
            Strcasecpy(p-1, "um"); /* a -> um */
        }

        /* here we cannot find the plural suffix */
    }
 bottom:
    /* if we stripped off a suffix (" of bar" from "foo of bar"),
       put it back now [strcat() isn't actually 100% safe here...] */
    if (excess) {
        Strcat(bp, excess);
    }

    return bp;
}

static boolean
badman(basestr, to_plural)
const char *basestr;
boolean to_plural; /* true => makeplural, false => makesingular */
{
    /* these are all the prefixes for *man that don't have a *men plural */
    static const char *no_men[] = {
        "albu", "antihu", "anti", "ata", "auto", "bildungsro", "cai", "cay",
        "ceru", "corner", "decu", "des", "dura", "fir", "hanu", "het",
        "infrahu", "inhu", "nonhu", "otto", "out", "prehu", "protohu",
        "subhu", "superhu", "talis", "unhu", "sha",
        "hu", "un", "le", "re", "so", "to", "at", "a",
    };
    /* these are all the prefixes for *men that don't have a *man singular */
    static const char *no_man[] = {
        "abdo", "acu", "agno", "ceru", "cogno", "cycla", "fleh", "grava",
        "hegu", "preno", "sonar", "speci", "dai", "exa", "fla", "sta", "teg",
        "tegu", "vela", "da", "hy", "lu", "no", "nu", "ra", "ru", "se", "vi",
        "ya", "o", "a",
    };
    int i, al;
    const char *endstr, *spot;

    if (!basestr || strlen(basestr) < 4) {
        return FALSE;
    }

    endstr = eos((char *) basestr);

    if (to_plural) {
        for (i = 0; i < SIZE(no_men); i++) {
            al = (int) strlen(no_men[i]);
            spot = endstr - (al + 3);
            if (!BSTRNCMPI(basestr, spot, no_men[i], al) &&
                 (spot == basestr || *(spot - 1) == ' ')) {
                return TRUE;
            }
        }
    } else {
        for (i = 0; i < SIZE(no_man); i++) {
            al = (int) strlen(no_man[i]);
            spot = endstr - (al + 3);
            if (!BSTRNCMPI(basestr, spot, no_man[i], al) &&
                 (spot == basestr || *(spot - 1) == ' ')) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/* compare user string against object name string using fuzzy matching */
static boolean
wishymatch(u_str, o_str, retry_inverted)
const char *u_str;      /* from user, so might be variant spelling */
const char *o_str;      /* from objects[], so is in canonical form */
boolean retry_inverted; /* optional extra "of" handling */
{
    static NEARDATA const char detect_SP[] = "detect ",
                               SP_detection[] = " detection";
    char *p, buf[BUFSZ];

    /* ignore spaces & hyphens and upper/lower case when comparing */
    if (fuzzymatch(u_str, o_str, " -", TRUE)) return TRUE;

    if (retry_inverted) {
        const char *u_of, *o_of;

        /* when just one of the strings is in the form "foo of bar",
           convert it into "bar foo" and perform another comparison */
        u_of = strstri(u_str, " of ");
        o_of = strstri(o_str, " of ");
        if (u_of && !o_of) {
            Strcpy(buf, u_of+4);
            p = eos(strcat(buf, " "));
            while (u_str < u_of) *p++ = *u_str++;
            *p = '\0';
            return fuzzymatch(buf, o_str, " -", TRUE);
        } else if (o_of && !u_of) {
            Strcpy(buf, o_of+4);
            p = eos(strcat(buf, " "));
            while (o_str < o_of) *p++ = *o_str++;
            *p = '\0';
            return fuzzymatch(u_str, buf, " -", TRUE);
        }
    }

    /* [note: if something like "elven speed boots" ever gets added, these
       special cases should be changed to call wishymatch() recursively in
       order to get the "of" inversion handling] */
    if (!strncmp(o_str, "dwarvish ", 9)) {
        if (!strncmpi(u_str, "dwarven ", 8))
            return fuzzymatch(u_str+8, o_str+9, " -", TRUE);
    } else if (!strncmp(o_str, "elven ", 6)) {
        if (!strncmpi(u_str, "elvish ", 7))
            return fuzzymatch(u_str+7, o_str+6, " -", TRUE);
        else if (!strncmpi(u_str, "elfin ", 6))
            return fuzzymatch(u_str+6, o_str+6, " -", TRUE);
    } else if (!strcmp(o_str, "aluminum")) {
        /* this special case doesn't really fit anywhere else... */
        /* (note that " wand" will have been stripped off by now) */
        if (!strcmpi(u_str, "aluminium")) {
            return fuzzymatch(u_str+9, o_str+8, " -", TRUE);
        }
    }

    return FALSE;
}

/* alternate spellings; if the difference is only the presence or
   absence of spaces and/or hyphens (such as "pickaxe" vs "pick axe"
   vs "pick-axe") then there is no need for inclusion in this list;
   likewise for ``"of" inversions'' ("boots of speed" vs "speed boots") */
struct alt_spellings {
    const char *sp;
    int ob;
} spellings[] = {
    { "pickax", PICK_AXE },
    { "whip", BULLWHIP },
    { "saber", SILVER_SABER },
    { "silver sabre", SILVER_SABER },
    { "smooth shield", SHIELD_OF_REFLECTION },
    { "grey dragon scale mail", GRAY_DRAGON_SCALE_MAIL },
    { "grey dragon scales", GRAY_DRAGON_SCALES },
    { "enchant armour", SCR_ENCHANT_ARMOR },
    { "destroy armour", SCR_DESTROY_ARMOR },
    { "scroll of enchant armour", SCR_ENCHANT_ARMOR },
    { "scroll of destroy armour", SCR_DESTROY_ARMOR },
    { "leather armour", LEATHER_ARMOR },
    { "studded leather armour", STUDDED_LEATHER_ARMOR },
    { "iron ball", HEAVY_IRON_BALL },
    { "lantern", BRASS_LANTERN },
    { "mattock", DWARVISH_MATTOCK },
    { "amulet of poison resistance", AMULET_VERSUS_POISON },
    { "potion of sleep", POT_SLEEPING },
    { "stone", ROCK },
#ifdef TOURIST
    { "camera", EXPENSIVE_CAMERA },
    { "tee shirt", T_SHIRT },
#endif
    { "can", TIN },
    { "can opener", TIN_OPENER },
    { "kelp", KELP_FROND },
    { "eucalyptus", EUCALYPTUS_LEAF },
    { "royal jelly", LUMP_OF_ROYAL_JELLY },
    { "lembas", LEMBAS_WAFER },
    { "cookie", FORTUNE_COOKIE },
    { "pie", CREAM_PIE },
    { "marker", MAGIC_MARKER },
    { "hook", GRAPPLING_HOOK },
    { "grappling iron", GRAPPLING_HOOK },
    { "grapnel", GRAPPLING_HOOK },
    { "grapple", GRAPPLING_HOOK },
    { "helmet of opposite alignment", HELM_OF_OPPOSITE_ALIGNMENT },
    { "protection from shape shifters", RIN_PROTECTION_FROM_SHAPE_CHAN },
    /* if we ever add other sizes, move this to o_ranges[] with "bag" */
    { "box", LARGE_BOX },
    { "luck stone", LUCKSTONE },
    { "load stone", LOADSTONE },
    { "touch stone", TOUCHSTONE },
    { "flintstone", FLINT },
    { "crystall ball", CRYSTAL_BALL },
    { "crystall orb", CRYSTAL_BALL },
    { "crystal orb", CRYSTAL_BALL },
    { "crystall pick", CRYSTAL_PICK },
    { (const char *)0, 0 },
};

static short
rnd_otyp_by_wpnskill(skill)
schar skill;
{
    int i, n = 0;
    short otyp = STRANGE_OBJECT;

    for (i = bases[WEAPON_CLASS]; i < NUM_OBJECTS && objects[i].oc_class == WEAPON_CLASS; i++) {
        if (objects[i].oc_skill == skill) {
            n++;
            otyp = i;
        }
    }
    if (n > 0) {
        n = rn2(n);
        for (i = bases[WEAPON_CLASS]; i < NUM_OBJECTS && objects[i].oc_class == WEAPON_CLASS; i++) {
            if (objects[i].oc_skill == skill) {
                if (--n < 0) {
                    return i;
                }
            }
        }
    }

    return otyp;
}

static short
rnd_otyp_by_namedesc(name, oclass, xtra_prob)
const char *name;
char oclass;
int xtra_prob; /* to force 0% random generation items to also be considered */
{
    int i, n = 0;
    short validobjs[NUM_OBJECTS];
    register const char *zn;
    int prob, maxprob = 0;

    if (!name || !*name) {
        return STRANGE_OBJECT;
    }

    memset(validobjs, 0, sizeof validobjs);

    /* FIXME:
     * When this spans classes (the !oclass case), the item
     * probabilities are not very useful because they don't take
     * the class generation probability into account.  [If 10%
     * of spellbooks were blank and 1% of scrolls were blank,
     * "blank" would have 10/11 chance to yield a book even though
     * scrolls are supposed to be much more common than books.]
     */
    i = oclass ? bases[(int) oclass] : STRANGE_OBJECT+1;
    while (i < NUM_OBJECTS && (!oclass || objects[i].oc_class == oclass)) {
        /* don't match extra descriptions (w/o real name) */
        if ((zn = OBJ_NAME(objects[i])) == 0) {
            i++;
            continue;
        }
        if (wishymatch(name, zn, TRUE) ||
             ((zn = OBJ_DESCR(objects[i])) && wishymatch(name, zn, FALSE)) ||
             ((zn = objects[i].oc_uname) && wishymatch(name, zn, FALSE))) {
            validobjs[n++] = (short) i;
            maxprob += (objects[i].oc_prob + xtra_prob);
        }
        i++;
    }

    if (n > 0 && maxprob) {
        prob = rn2(maxprob);
        for (i = 0; i < n - 1; i++) {
            if ((prob -= (objects[validobjs[i]].oc_prob + xtra_prob)) < 0) {
                break;
            }
        }
        return validobjs[i];
    }
    return STRANGE_OBJECT;
}

int
shiny_obj(oclass)
char oclass;
{
    return (int) rnd_otyp_by_namedesc("shiny", oclass, 0);
}

/*
 * Given a user-supplied string, try to match it to an object type.
 * Very similar to rnd_otyp_by_namedesc, except it doesn't fall back on picking
 * a random object if it can't find an appropriate one. Intended to be the
 * object counterpart to name_to_mon.
 * Only works on exact object names, since allowing it to work on randomized
 * descriptions or user-called names would leak information to object lookup.
 */
short
name_to_otyp(in_str)
const char * in_str;
{
    short otyp;
    int i;
    char oclass = 0;

    /* Search for class names: XXXXX potion, scroll of XXXXX.  Avoid */
    /* false hits on, e.g., rings for "ring mail".
     * This is lifted from readobjnam, and should probably be refactored into
     * its own function but the existing logic in there is too tied up with
     * readobjnam variables at the moment. */
    if (strncmpi(in_str, "enchant ", 8) &&
         strncmpi(in_str, "destroy ", 8) &&
         strncmpi(in_str, "detect food", 11) &&
         strncmpi(in_str, "food detection", 14) &&
         strncmpi(in_str, "ring mail", 9) &&
         strncmpi(in_str, "studded armor", 21) &&
         strncmpi(in_str, "light armor", 13) &&
         strncmpi(in_str, "tooled horn", 11) &&
         strncmpi(in_str, "food ration", 11) &&
         strncmpi(in_str, "meat ring", 9)) {
        for (i = 0; i < (int) (sizeof wrpsym); i++) {
            int j = strlen(wrp[i]);
            if (!strncmpi(in_str, wrp[i], j)) {
                oclass = wrpsym[i];
                if (oclass != AMULET_CLASS) {
                    /* amulets don't consistently use "amulet of" */
                    in_str += j;
                    if (!strncmpi(in_str, " of ", 4)) {
                        in_str += 4;
                    }
                }
                break;
            }
        }
    }
    /* if the player asked only for "ring", etc, that's not going to resolve to
     * anything in this function, so safe to say the string matches no otyp. */
    if (!(*in_str)) {
        return STRANGE_OBJECT;
    }

    for (otyp = STRANGE_OBJECT + 1; otyp < NUM_OBJECTS; ++otyp) {
        if (!OBJ_NAME(objects[otyp])) {
            /* obj is nonexistent in this game */
            continue;
        } else if (oclass && objects[otyp].oc_class != oclass) {
            /* name might match, but the class is wrong, e.g. "scroll of light"
             * becomes "light" which matches "wand of light" */
            continue;
        }
        if (wishymatch(in_str, OBJ_NAME(objects[otyp]), TRUE)) {
            return otyp;
        }
    }
    /* try alternate spellings */
    const struct alt_spellings *as;

    for (as = spellings; as->sp != 0; as++) {
        if (!strcmpi(in_str, as->sp)) {
            return as->ob;
        }
    }
    /* try Japanese names */
    struct Jitem *j;
    for (j = Japanese_items; j->item != 0; j++) {
        if (!strcmpi(in_str, j->name)) {
            return j->item;
        }
    }
    /* try fruits */
    if (fruit_from_name(in_str, FALSE, NULL)) {
        return SLIME_MOLD;
    }

    /* try dragon scales and dragon scale mail */
    if (!strncmpi(in_str, "set of ", 7)) {
        in_str += 7;
    }
    for (int i = PM_GRAY_DRAGON; i <= PM_CHROMATIC_DRAGON; i++) {
        struct objclass oclass = objects[GRAY_DRAGON_SCALES + i - PM_GRAY_DRAGON];
        if ((!strcmpi(OBJ_DESCR(oclass), in_str)) ||
            (!strcmpi(OBJ_NAME(oclass), in_str))) {
            return oclass.oc_name_idx;
        }

        oclass = objects[GRAY_DRAGON_SCALE_MAIL + i - PM_GRAY_DRAGON];
        if ((!strcmpi(OBJ_DESCR(oclass), in_str)) ||
            (!strcmpi(OBJ_NAME(oclass), in_str))) {
            return oclass.oc_name_idx;
        }
    }

    return STRANGE_OBJECT;
}

/*
 * Return something wished for.  Specifying a null pointer for
 * the user request string results in a random object.  Otherwise,
 * if asking explicitly for "nothing" (or "nil") return no_wish;
 * if not an object return &zeroobj; if an error (no matching object),
 * return null.
 */
struct obj *
readobjnam(bp, no_wish)
register char *bp;
struct obj *no_wish;
{
    char *p;
    int i;
    struct obj *otmp;
    int cnt, spe, spesgn, typ, very, rechrg;
    int blessed, uncursed, iscursed, ispoisoned, isgreased, isdrained;
    int eroded, eroded2, erodeproof, locked, unlocked, broken;
#ifdef INVISIBLE_OBJECTS
    int isinvisible;
#endif
    int halfeaten, halfdrained, mntmp, contents;
    int islit, unlabeled, ishistoric, isdiluted, trapped;
    int tmp, tinv, tvariety;
    int wetness, gsize = 0;
    boolean zombifying;
    struct fruit *f;
    int ftype = current_fruit;
    char fruitbuf[BUFSZ];
    /* Fruits may not mess up the ability to wish for real objects (since
     * you can leave a fruit in a bones file and it will be added to
     * another person's game), so they must be checked for last, after
     * stripping all the possible prefixes and seeing if there's a real
     * name in there.  So we have to save the full original name.  However,
     * it's still possible to do things like "uncursed burnt Alaska",
     * or worse yet, "2 burned 5 course meals", so we need to loop to
     * strip off the prefixes again, this time stripping only the ones
     * possible on food.
     * We could get even more detailed so as to allow food names with
     * prefixes that _are_ possible on food, so you could wish for
     * "2 3 alarm chilis".  Currently this isn't allowed; options.c
     * automatically sticks 'candied' in front of such names.
     */

    char oclass;
    char *un, *dn, *actualn, *origbp = bp;
    const char *name=0;
    /** true if object has been found by its actual name */
    boolean found_by_actualn = FALSE;

    cnt = spe = spesgn = typ = very = rechrg =
        blessed = uncursed = iscursed = isdrained = halfdrained =
#ifdef INVISIBLE_OBJECTS
            isinvisible =
#endif
            ispoisoned = isgreased = eroded = eroded2 = erodeproof =
                halfeaten = islit = unlabeled = ishistoric = isdiluted = 0;
    trapped = locked = unlocked = broken = 0;

    tvariety = RANDOM_TIN;
    mntmp = NON_PM;
#define UNDEFINED 0
#define EMPTY 1
#define SPINACH 2
    contents = UNDEFINED;
    oclass = 0;
    actualn = dn = un = 0;
    wetness = 0;
    zombifying = FALSE;

    if (!bp) goto any;
    /* first, remove extra whitespace they may have typed */
    (void)mungspaces(bp);
    /* allow wishing for "nothing" to preserve wishless conduct...
       [now requires "wand of nothing" if that's what was really wanted] */
    if (!strcmpi(bp, "nothing") || !strcmpi(bp, "nil") ||
        !strcmpi(bp, "none")) return no_wish;
    /* save the [nearly] unmodified choice string */
    Strcpy(fruitbuf, bp);

    for (;;) {
        int l;

        if (!bp || !*bp) goto any;
        if (!strncmpi(bp, "an ", l=3) ||
            !strncmpi(bp, "a ", l=2)) {
            cnt = 1;
        } else if (!strncmpi(bp, "the ", l=4)) {
            ;   /* just increment `bp' by `l' below */
        } else if (!cnt && digit(*bp) && strcmp(bp, "0")) {
            cnt = atoi(bp);
            while(digit(*bp)) bp++;
            while(*bp == ' ') bp++;
            l = 0;
        } else if (*bp == '+' || *bp == '-') {
            spesgn = (*bp++ == '+') ? 1 : -1;
            spe = atoi(bp);
            while(digit(*bp)) bp++;
            while(*bp == ' ') bp++;
            l = 0;
        } else if (!strncmpi(bp, "blessed ", l=8) ||
                   !strncmpi(bp, "holy ", l=5)) {
            blessed = 1;
        } else if (!strncmpi(bp, "moist ", l = 6) ||
                   !strncmpi(bp, "wet ", l = 4)) {
            if (!strncmpi(bp, "wet ", 4)) {
                wetness = rn2(3) + 3;
            } else {
                wetness = rnd(2);
            }
        } else if (!strncmpi(bp, "cursed ", l=7) ||
                   !strncmpi(bp, "unholy ", l=7)) {
            iscursed = 1;
        } else if (!strncmpi(bp, "uncursed ", l=9)) {
            uncursed = 1;
#ifdef INVISIBLE_OBJECTS
        } else if (!strncmpi(bp, "invisible ", l=10)) {
            isinvisible = 1;
#endif
        } else if (!strncmpi(bp, "rustproof ", l=10) ||
                   !strncmpi(bp, "erodeproof ", l=11) ||
                   !strncmpi(bp, "corrodeproof ", l=13) ||
                   !strncmpi(bp, "fixed ", l=6) ||
                   !strncmpi(bp, "fireproof ", l=10) ||
                   !strncmpi(bp, "rotproof ", l=9)) {
            erodeproof = 1;
        } else if (!strncmpi(bp, "lit ", l=4) ||
                   !strncmpi(bp, "burning ", l=8)) {
            islit = 1;
        } else if (!strncmpi(bp, "unlit ", l=6) ||
                   !strncmpi(bp, "extinguished ", l=13)) {
            islit = 0;
            /* "unlabeled" and "blank" are synonymous */
        } else if (!strncmpi(bp, "unlabeled ", l=10) ||
                   !strncmpi(bp, "unlabelled ", l=11) ||
                   !strncmpi(bp, "blank ", l=6)) {
            unlabeled = 1;
        } else if(!strncmpi(bp, "poisoned ", l=9)
                  || (wizard && !strncmpi(bp, "trapped ", l=8))
                  ) {
            ispoisoned=1;
        } else if (!strncmpi(bp, "trapped ", l = 8)) {
            /* "trapped" recognized but not honored outside wizard mode */
            trapped = 0; /* undo any previous "untrapped" */
#ifdef WIZARD
            if (wizard) {
                trapped = 1;
            }
#endif
        } else if (!strncmpi(bp, "untrapped ", l = 10)) {
            trapped = 2; /* not trapped */
        /* locked, unlocked, broken: box/chest lock states */
        } else if (!strncmpi(bp, "locked ", l = 7)) {
            locked = 1;
            unlocked = broken = 0;
        } else if (!strncmpi(bp, "unlocked ", l = 9)) {
            unlocked = 1;
            locked = broken = 0;
        } else if (!strncmpi(bp, "broken ", l = 7)) {
            broken = 1;
            locked = unlocked = 0;
        } else if (!strncmpi(bp, "greased ", l=8)) {
            isgreased = 1;
        } else if (!strncmpi(bp, "zombifying ", l = 11)) {
            zombifying = TRUE;
        } else if (!strncmpi(bp, "very ", l=5)) {
            /* very rusted very heavy iron ball */
            very = 1;
        } else if (!strncmpi(bp, "thoroughly ", l=11)) {
            very = 2;
        } else if (!strncmpi(bp, "rusty ", l=6) ||
                   !strncmpi(bp, "rusted ", l=7) ||
                   !strncmpi(bp, "burnt ", l=6) ||
                   !strncmpi(bp, "burned ", l=7)) {
            eroded = 1 + very;
            very = 0;
        } else if (!strncmpi(bp, "corroded ", l=9) ||
                   !strncmpi(bp, "rotted ", l=7)) {
            eroded2 = 1 + very;
            very = 0;
        } else if (!strncmpi(bp, "partly drained ", l=15)) {
            isdrained = 1;
            halfdrained = 1;
        } else if (!strncmpi(bp, "drained ", l=8)) {
            isdrained = 1;
            halfdrained = 0;
        } else if (!strncmpi(bp, "partly eaten ", l = 13) ||
                   !strncmpi(bp, "partially eaten ", l = 16)) {
            halfeaten = 1;
        } else if (!strncmpi(bp, "historic ", l=9)) {
            ishistoric = 1;
        } else if (!strncmpi(bp, "diluted ", l=8)) {
            isdiluted = 1;
        } else if (!strncmpi(bp, "empty ", l=6)) {
            contents = EMPTY;
        } else break;
        bp += l;
    }
    if(!cnt) cnt = 1;       /* %% what with "gems" etc. ? */
    if (strlen(bp) > 1 && (p = rindex(bp, '(')) != 0) {
        boolean keeptrailingchars = TRUE;

        p[(p > bp && p[-1] == ' ') ? -1 : 0] = '\0'; /*terminate bp */
        p++; /* advance past '(' */
        if (!strncmpi(p, "lit)", 4)) {
            islit = 1;
            p += 4 - 1; /* point at ')' */
        } else {
            spe = atoi(p);
            while (digit(*p)) {
                p++;
            }
            if (*p == ':') {
                p++;
                rechrg = spe;
                spe = atoi(p);
                while (digit(*p)) {
                    p++;
                }
            }
            if (*p != ')') {
                spe = rechrg = 0;
                /* mis-matched parentheses; rest of string will be ignored
                 * [probably we should restore everything back to '('
                 * instead since it might be part of "named ..."]
                 */
                keeptrailingchars = FALSE;
            } else {
                spesgn = 1;
            }
        }
        if (keeptrailingchars) {
            char *pp = eos(bp);

            /* 'pp' points at 'pb's terminating '\0',
               'p' points at ')' and will be incremented past it */
            do {
                *pp++ = *++p;
            } while (*p);
        }
    }
/*
   otmp->spe is type schar; so we don't want spe to be any bigger or smaller.
   also, spe should always be positive  -- some cheaters may try to confuse
   atoi()
 */
    if (spe < 0) {
        spesgn = -1;    /* cheaters get what they deserve */
        spe = abs(spe);
    }
    if (spe > SCHAR_LIM)
        spe = SCHAR_LIM;
    if (rechrg < 0 || rechrg > 7) rechrg = 7;   /* recharge_limit */

    /* now we have the actual name, as delivered by xname, say
        green potions called whisky
        scrolls labeled "QWERTY"
        egg
        fortune cookies
        very heavy iron ball named hoei
        wand of wishing
        elven cloak
     */
    if ((p = strstri(bp, " named ")) != 0) {
        *p = 0;
        name = p+7;
    }
    if ((p = strstri(bp, " called ")) != 0) {
        *p = 0;
        un = p+8;
        /* "helmet called telepathy" is not "helmet" (a specific type)
         * "shield called reflection" is not "shield" (a general type)
         */
        for(i = 0; i < SIZE(o_ranges); i++)
            if(!strcmpi(bp, o_ranges[i].name)) {
                oclass = o_ranges[i].oclass;
                goto srch;
            }
    }
    if ((p = strstri(bp, " labeled ")) != 0) {
        *p = 0;
        dn = p+9;
    } else if ((p = strstri(bp, " labelled ")) != 0) {
        *p = 0;
        dn = p+10;
    }
    if ((p = strstri(bp, " of spinach")) != 0) {
        *p = 0;
        contents = SPINACH;
    }

    /*
       Skip over "pair of ", "pairs of", "set of" and "sets of".

       Accept "3 pair of boots" as well as "3 pairs of boots". It is valid
       English either way.  See makeplural() for more on pair/pairs.

       We should only double count if the object in question is not
       refered to as a "pair of".  E.g. We should double if the player
       types "pair of spears", but not if the player types "pair of
       lenses".  Luckily (?) all objects that are refered to as pairs
       -- boots, gloves, and lenses -- are also not mergable, so cnt is
       ignored anyway.
     */
    if(!strncmpi(bp, "pair of ", 8)) {
        bp += 8;
        cnt *= 2;
    } else if(cnt > 1 && !strncmpi(bp, "pairs of ", 9)) {
        bp += 9;
        if (cnt > 1) {
            cnt *= 2;
        }
    } else if (!strncmpi(bp, "set of ", 7)) {
        bp += 7;
    } else if (!strncmpi(bp, "sets of ", 8)) {
        bp += 8;
    }

    /*
     * Find corpse type using "of" (figurine of an orc, tin of orc meat)
     * Don't check if it's a wand or spellbook.
     * (avoid "wand/finger of death" confusion).
     * (ALI "potion of vampire blood" also).
     */
    if (!strstri(bp, "wand ")
        && !strstri(bp, "spellbook ")
        && !strstri(bp, "potion ")
        && !strstri(bp, "potions ")
        && !strstri(bp, "finger ")) {
        if ((p = strstri(bp, " of ")) != 0
            && (mntmp = name_to_mon(p+4)) >= LOW_PM)
            *p = 0;
    }
    /* Find corpse type w/o "of" (red dragon scale mail, yeti corpse) */
    if (strncmpi(bp, "samurai sword", 13)) /* not the "samurai" monster! */
        if (strncmpi(bp, "wizard lock", 11)) /* not the "wizard" monster! */
            if (strncmpi(bp, "ninja-to", 8)) /* not the "ninja" rank */
                if (strncmpi(bp, "master key", 10)) /* not the "master" rank */
                    if (strncmpi(bp, "Thiefbane", 9)) /* not the "thief" rank */
                        if (strncmpi(bp, "Ogresmasher", 11)) /* not the "ogre" monster */
                            if (strncmpi(bp, "magenta", 7)) /* not the "mage" rank */
                                if (strncmpi(bp, "vampire blood", 13)) /* not the "vampire" monster */
                                    if (mntmp < LOW_PM && strlen(bp) > 2 &&
                                        (mntmp = name_to_mon(bp)) >= LOW_PM) {
                                        int mntmptoo, mntmplen; /* double check for rank title */
                                        char *obp = bp;
                                        mntmptoo = title_to_mon(bp, (int *)0, &mntmplen);
                                        bp += mntmp != mntmptoo ? (int)strlen(mons[mntmp].mname) : mntmplen;
                                        if (*bp == ' ') bp++;
                                        else if (!strncmpi(bp, "s ", 2)) bp += 2;
                                        else if (!strncmpi(bp, "es ", 3)) bp += 3;
                                        else if (!*bp && !actualn && !dn && !un && !oclass) {
                                            /* no referent; they don't really mean a monster type */
                                            bp = obp;
                                            mntmp = NON_PM;
                                        }
                                    }

    /* disallow wishing for chromatic dragon stuff */
    if (mntmp == PM_CHROMATIC_DRAGON && !wizard) {
        mntmp = rn2(YELLOW_DRAGON_SCALES-GRAY_DRAGON_SCALES) + PM_GRAY_DRAGON;
    }

    /* first change to singular if necessary */
    if (*bp) {
        char *sng = makesingular(bp);
        if (strcmp(bp, sng)) {
            if (cnt == 1) cnt = 2;
            Strcpy(bp, sng);
        }
    }

    /* Alternate spellings (pick-ax, silver sabre, &c) */
    {
        const struct alt_spellings *as = spellings;

        while (as->sp) {
            if (fuzzymatch(bp, as->sp, " -", TRUE)) {
                typ = as->ob;
                goto typfnd;
            }
            as++;
        }
        /* can't use spellings list for this one due to shuffling */
        if (!strncmpi(bp, "grey spell", 10))
            *(bp + 2) = 'a';

        if ((p = strstri(bp, "armour")) != 0) {
            /* skip past "armo", then copy remainder beyond "u" */
            p += 4;
            while ((*p = *(p + 1)) != '\0') {
                p++; /* self terminating */
            }
        }
    }

    /* dragon scales - assumes order of dragons */
    if ((!strcmpi(bp, "scales") || !strcmpi(bp, "dragon scales")) &&
        mntmp != NON_PM &&
        mntmp >= PM_GRAY_DRAGON && mntmp <= PM_CHROMATIC_DRAGON) {
        if (mntmp == PM_CHROMATIC_DRAGON && !wizard) {
            /* random scales when asking for chromatic dragon scales */
            typ = rn2(YELLOW_DRAGON_SCALES-GRAY_DRAGON_SCALES) +
                  GRAY_DRAGON_SCALES;
        } else {
            typ = GRAY_DRAGON_SCALES + mntmp - PM_GRAY_DRAGON;
        }
        mntmp = NON_PM; /* no monster */
        goto typfnd;
    }

    p = eos(bp);
    if (!BSTRCMPI(bp, p-10, "holy water")) {
        typ = POT_WATER;
        if ((p-bp) >= 12 && *(p-12) == 'u')
            iscursed = 1; /* unholy water */
        else blessed = 1;
        goto typfnd;
    }
    if (unlabeled && !BSTRCMPI(bp, p-6, "scroll")) {
        typ = SCR_BLANK_PAPER;
        goto typfnd;
    }
    if (unlabeled && !BSTRCMPI(bp, p-9, "spellbook")) {
        typ = SPE_BLANK_PAPER;
        goto typfnd;
    }
    /* specific food rather than color of gem/potion/spellbook[/scales] */
    if (!BSTRCMPI(bp, p-6, "orange") && mntmp == NON_PM) {
        typ = ORANGE;
        goto typfnd;
    }
    /*
     * NOTE: Gold pieces are handled as objects nowadays, and therefore
     * this section should probably be reconsidered as well as the entire
     * gold/money concept.  Maybe we want to add other monetary units as
     * well in the future. (TH)
     */
    if(!BSTRCMPI(bp, p-10, "gold piece") || !BSTRCMPI(bp, p-7, "zorkmid") ||
       !strcmpi(bp, "gold") || !strcmpi(bp, "money") ||
       !strcmpi(bp, "coin") || *bp == GOLD_SYM) {
        if (cnt > 5000
#ifdef WIZARD
            && !wizard
#endif
            ) cnt=5000;
        if (cnt < 1) cnt=1;
        otmp = mksobj(GOLD_PIECE, FALSE, FALSE);
        otmp->quan = cnt;
        otmp->owt = weight(otmp);
        flags.botl=1;
        return (otmp);
    }

    /* check for single character object class code ("/" for wand, &c) */
    if (strlen(bp) == 1 &&
        (i = def_char_to_objclass(*bp)) < MAXOCLASSES && i > ILLOBJ_CLASS
#ifdef WIZARD
        && (wizard || i != VENOM_CLASS)
#else
        && i != VENOM_CLASS
#endif
        ) {
        oclass = i;
        goto any;
    }

    /* Search for class names: XXXXX potion, scroll of XXXXX.  Avoid */
    /* false hits on, e.g., rings for "ring mail". */
    if (strncmpi(bp, "enchant ", 8) &&
        strncmpi(bp, "destroy ", 8) &&
        strncmpi(bp, "detect food", 11) &&
        strncmpi(bp, "food detection", 14) &&
        strncmpi(bp, "ring mail", 9) &&
        strncmpi(bp, "studded leather arm", 19) &&
        strncmpi(bp, "leather arm", 11) &&
        strncmpi(bp, "tooled horn", 11) &&
        strncmpi(bp, "food ration", 11) &&
        strncmpi(bp, "meat ring", 9)
       ) {
        for (i = 0; i < (int)(sizeof wrpsym); i++) {
            int j = strlen(wrp[i]);

            /* check for "<class> [ of ] something" */
            if (!strncmpi(bp, wrp[i], j)) {
                oclass = wrpsym[i];
                if (oclass != AMULET_CLASS) {
                    bp += j;
                    if(!strncmpi(bp, " of ", 4)) actualn = bp+4;
                    /* else if(*bp) ?? */
                } else
                    actualn = bp;
                goto srch;
            }
            /* check for "something <class>" */
            if (!BSTRCMPI(bp, p-j, wrp[i])) {
                oclass = wrpsym[i];
                /* for "foo amulet", leave the class name so that
                   wishymatch() can do "of inversion" to try matching
                   "amulet of foo"; other classes don't include their
                   class name in their full object names (where
                   "potion of healing" is just "healing", for instance) */
                if (oclass != AMULET_CLASS) {
                    p -= j;
                    *p = '\0';
                    if (p > bp && p[-1] == ' ') {
                        p[-1] = '\0';
                    }
                } else {
                    /* amulet without "of"; convoluted wording but better a
                       special case that's handled than one that's missing */
                    if (!strncmpi(bp, "versus poison ", 14)) {
                        typ = AMULET_VERSUS_POISON;
                        goto typfnd;
                    }
                }
                actualn = dn = bp;
                goto srch;
            }
        }
    }

    /* Wishing in wizard mode can create traps and furniture.
     * Part I:  distinguish between trap and object for the two
     * types of traps which have corresponding objects:  bear trap
     * and land mine.  "beartrap" (object) and "bear trap" (trap)
     * have a difference in spelling which we used to exploit by
     * adding a special case in wishymatch(), but "land mine" is
     * spelled the same either way so needs different handing.
     * Since we need something else for land mine, we've dropped
     * the bear trap hack so that both are handled exactly the
     * same.  To get an armed trap instead of a disarmed object,
     * the player can prefix either the object name or the trap
     * name with "trapped " (which ordinarily applies to chests
     * and tins), or append something--anything at all except for
     * " object", but " trap" is suggested--to either the trap
     * name or the object name.
     */
    if (wizard && (!strncmpi(bp, "bear", 4) || !strncmpi(bp, "land", 4))) {
        boolean beartrap = (lowc(*bp) == 'b');
        char *zp = bp + 4; /* skip "bear"/"land" */

        if (*zp == ' ') {
            ++zp; /* embedded space is optional */
        }
        if (!strncmpi(zp, beartrap ? "trap" : "mine", 4)) {
            zp += 4;
            if (trapped == 2 || !strcmpi(zp, " object")) {
                /* "untrapped <foo>" or "<foo> object" */
                typ = beartrap ? BEARTRAP : LAND_MINE;
                goto typfnd;
            } else if (trapped == 1 || *zp != '\0') {
                /* "trapped <foo>" or "<foo> trap" (actually "<foo>*") */
                int idx = trap_to_defsym(beartrap ? BEAR_TRAP : LANDMINE);

                /* use canonical trap spelling, skip object matching */
                Strcpy(bp, defsyms[idx].explanation);
                goto wiztrap;
            }
            /* [no prefix or suffix; we're going to end up matching
               the object name and getting a disarmed trap object] */
        }
    }

retry:
    /* "grey stone" check must be before general "stone" */
    for (i = 0; i < SIZE(o_ranges); i++) {
        if (!strcmpi(bp, o_ranges[i].name)) {
            typ = rnd_class(o_ranges[i].f_o_range, o_ranges[i].l_o_range);
            goto typfnd;
        }
    }

    if (!BSTRCMPI(bp, p-6, " stone") || !BSTRCMPI(bp, p-4, " gem")) {
        p[!strcmpi(p-4, " gem") ? -4 : -6] = '\0';
        oclass = GEM_CLASS;
        dn = actualn = bp;
        goto srch;
    } else if (!strcmpi(bp, "looking glass")) {
        ; /* avoid false hit on "* glass" */
    } else if (!BSTRCMPI(bp, p-6, " glass") || !strcmpi(bp, "glass")) {
        register char *g = bp;

        /* treat "broken glass" as a non-existent item; since "broken" is
           also a chest/box prefix it might have been stripped off above */
        if (broken || strstri(g, "broken")) {
            return (struct obj *) 0;
        }
        if (!strncmpi(g, "worthless ", 10)) g += 10;
        if (!strncmpi(g, "piece of ", 9)) g += 9;
        if (!strncmpi(g, "colored ", 8)) g += 8;
        else if (!strncmpi(g, "coloured ", 9)) g += 9;
        if (!strcmpi(g, "glass")) { /* choose random color */
            /* 9 different kinds */
            typ = LAST_GEM + rnd(9);
            if (objects[typ].oc_class == GEM_CLASS) goto typfnd;
            else typ = 0;   /* somebody changed objects[]? punt */
        } else {        /* try to construct canonical form */
            char tbuf[BUFSZ];
            Strcpy(tbuf, "worthless piece of ");
            Strcat(tbuf, g);  /* assume it starts with the color */
            Strcpy(bp, tbuf);
        }
    }

    actualn = bp;
    if (!dn) dn = actualn; /* ex. "skull cap" */
srch:
    /* check real names of gems first */
    if (!oclass && actualn) {
        for (i = bases[GEM_CLASS]; i <= LAST_GEM; i++) {
            register const char *zn;

            if ((zn = OBJ_NAME(objects[i])) && !strcmpi(actualn, zn)) {
                typ = i;
                goto typfnd;
            }
        }
        /* "tin of foo" would be caught above, but plain "tin" has
           a random chance of yielding "tin wand" unless we do this */
        if (!strcmpi(actualn, "tin")) {
            typ = TIN;
            goto typfnd;
        }
    }

    if (((typ = rnd_otyp_by_namedesc(actualn, oclass, 1)) != STRANGE_OBJECT) ||
        ((typ = rnd_otyp_by_namedesc(dn, oclass, 1)) != STRANGE_OBJECT) ||
        ((typ = rnd_otyp_by_namedesc(un, oclass, 1)) != STRANGE_OBJECT) ||
        ((typ = rnd_otyp_by_namedesc(origbp, oclass, 1)) != STRANGE_OBJECT)) {
        goto typfnd;
    }
    typ = 0;

    if (actualn) {
        struct Jitem *j = Japanese_items;

        while (j->item) {
            if (actualn && !strcmpi(actualn, j->name)) {
                typ = j->item;
                goto typfnd;
            }
            j++;
        }
    }
    /* if we've stripped off "armor" and failed to match anything
       in objects[], append "mail" and try again to catch misnamed
       requests like "plate armor" and "yellow dragon scale armor" */
    if (oclass == ARMOR_CLASS && !strstri(bp, "mail")) {
        /* modifying bp's string is ok; we're about to resort
           to random armor if this also fails to match anything */
        Strcat(bp, " mail");
        goto retry;
    }

    if (!strcmpi(bp, "spinach")) {
        contents = SPINACH;
        typ = TIN;
        goto typfnd;
    }
    /* Note: not strcmpi.  2 fruits, one capital, one not, are possible.
       Also not strncmp.  We used to ignore trailing text with it, but
       that resulted in "grapefruit" matching "grape" if the latter came
       earlier than the former in the fruit list. */
    {
        char *fp;
        int l, cntf;
        int blessedf, iscursedf, uncursedf, halfeatenf;

        blessedf = iscursedf = uncursedf = halfeatenf = 0;
        cntf = 0;

        fp = fruitbuf;
        for (;;) {
            if (!fp || !*fp) break;
            if (!strncmpi(fp, "an ", l=3) ||
                !strncmpi(fp, "a ", l=2)) {
                cntf = 1;
            } else if (!cntf && digit(*fp)) {
                cntf = atoi(fp);
                while(digit(*fp)) fp++;
                while(*fp == ' ') fp++;
                l = 0;
            } else if (!strncmpi(fp, "blessed ", l=8)) {
                blessedf = 1;
            } else if (!strncmpi(fp, "cursed ", l=7)) {
                iscursedf = 1;
            } else if (!strncmpi(fp, "uncursed ", l=9)) {
                uncursedf = 1;
            } else if (!strncmpi(fp, "partly eaten ", l = 13) ||
                       !strncmpi(fp, "partially eaten ", l = 16)) {
                halfeatenf = 1;
            } else break;
            fp += l;
        }

        for (f=ffruit; f; f = f->nextf) {
            /* match type: 0=none, 1=exact, 2=singular, 3=plural */
            int ftyp = 0;

            if (!strcmp(fp, f->fname)) {
                ftyp = 1;
            } else if (!strcmp(fp, makesingular(f->fname))) {
                ftyp = 2;
            } else if (!strcmp(fp, makeplural(f->fname))) {
                ftyp = 3;
            }
            if (ftyp) {
                typ = SLIME_MOLD;
                blessed = blessedf;
                iscursed = iscursedf;
                uncursed = uncursedf;
                halfeaten = halfeatenf;
                /* adjust count if user explicitly asked for
                   singular amount (can't happen unless fruit
                   has been given an already pluralized name)
                   or for plural amount */
                if (ftyp == 2 && !cntf) {
                    cntf = 1;
                } else if (ftyp == 3 && !cntf) {
                    cntf = 2;
                }
                cnt = cntf;
                ftype = f->fid;
                goto typfnd;
            }
        }
    }

    if (!oclass && actualn) {
        short objtyp;

        /* Perhaps it's an artifact specified by name, not type */
        name = artifact_name(actualn, &objtyp);
        if (name) {
            typ = objtyp;
            goto typfnd;
        }
    }
#ifdef WIZARD
    /*
     * Let wizards wish for traps and furniture.
     * Must come after objects check so wizards can still wish for
     * trap objects like beartraps.
     * Disallow such topology tweaks for WIZKIT startup wishes.
     */
wiztrap:
    if (wizard && !program_state.wizkit_wishing) {
        struct rm *lev;
        boolean madeterrain = FALSE;
        int trap, x = u.ux, y = u.uy;

        for (trap = NO_TRAP + 1; trap < TRAPNUM; trap++) {
            struct trap *t;
            const char *tname;

            tname = defsyms[trap_to_defsym(trap)].explanation;
            if (strncmpi(tname, bp, strlen(tname))) {
                continue;
            }
            /* found it; avoid stupid mistakes */
            if (is_hole(trap) && !Can_fall_thru(&u.uz)) {
                trap = ROCKTRAP;
            }
            if ((t = maketrap(x, y, trap)) != 0) {
                trap = t->ttyp;
                tname = defsyms[trap_to_defsym(trap)].explanation;
                pline("%s%s.", An(tname), (trap != MAGIC_PORTAL) ? "" : " to nowhere");
            } else {
                pline("Creation of %s failed.", an(tname));
            }
            return (struct obj *) &zeroobj;
        }

        /* furniture and terrain (use at your own risk; can clobber stairs
           or place furniture on existing traps which shouldn't be allowed) */
        p = eos(bp);
        if (!BSTRCMP(bp, p-8, "fountain")) {
            levl[u.ux][u.uy].typ = FOUNTAIN;
            level.flags.nfountains++;
            if(!strncmpi(bp, "magic ", 6))
                levl[u.ux][u.uy].blessedftn = 1;
            pline("A %sfountain.",
                  levl[u.ux][u.uy].blessedftn ? "magic " : "");
            madeterrain = TRUE;
        }
        if (!BSTRCMP(bp, p-6, "throne")) {
            levl[u.ux][u.uy].typ = THRONE;
            pline("A throne.");
            madeterrain = TRUE;
        }
# ifdef SINKS
        if (!BSTRCMP(bp, p-4, "sink")) {
            levl[u.ux][u.uy].typ = SINK;
            level.flags.nsinks++;
            pline("A sink.");
            madeterrain = TRUE;
        }
# endif
        /* ("water" matches "potion of water" rather than terrain) */
        if (!BSTRCMP(bp, p-4, "pool")) {
            levl[u.ux][u.uy].typ = POOL;
            del_engr_at(u.ux, u.uy);
            pline("A pool.");
            /* Must manually make kelp! */
            water_damage_chain(level.objects[u.ux][u.uy], TRUE);
            madeterrain = TRUE;
        }
        if (!BSTRCMP(bp, p-4, "lava")) {  /* also matches "molten lava" */
            levl[u.ux][u.uy].typ = LAVAPOOL;
            del_engr_at(u.ux, u.uy);
            pline("A pool of molten lava.");
            if (!(Levitation || Flying)) (void) lava_effects();
            madeterrain = TRUE;
        }

        if (!BSTRCMP(bp, p-5, "swamp")) {
            levl[u.ux][u.uy].typ = BOG;
            del_engr_at(u.ux, u.uy);
            pline("A muddy swamp.");
            if (!(Levitation || Flying || Wwalking)) (void) swamp_effects();
            madeterrain = TRUE;
        }

        if (!BSTRCMP(bp, p-5, "altar")) {
            aligntyp al;

            levl[u.ux][u.uy].typ = ALTAR;
            if(!strncmpi(bp, "chaotic ", 8))
                al = A_CHAOTIC;
            else if(!strncmpi(bp, "neutral ", 8))
                al = A_NEUTRAL;
            else if(!strncmpi(bp, "lawful ", 7))
                al = A_LAWFUL;
            else if(!strncmpi(bp, "unaligned ", 10))
                al = A_NONE;
            else /* -1 - A_CHAOTIC, 0 - A_NEUTRAL, 1 - A_LAWFUL */
                al = (!rn2(6)) ? A_NONE : rn2((int)A_LAWFUL+2) - 1;
            levl[u.ux][u.uy].altarmask = Align2amask( al );
            pline("%s altar.", An(align_str(al)));
            madeterrain = TRUE;
        }

        if (!BSTRCMP(bp, p-5, "grave") || !BSTRCMP(bp, p-9, "headstone")) {
            make_grave(u.ux, u.uy, (char *) 0);
            pline("A grave.");
            pline("%s.", IS_GRAVE(levl[u.ux][u.uy].typ) ? "A grave" : "Can't place a grave here");
            madeterrain = TRUE;
        }

        if (!BSTRCMP(bp, p-9, "dead tree")) {
            levl[u.ux][u.uy].typ = DEADTREE;
            pline("A dead tree.");
            block_point(u.ux, u.uy);
            madeterrain = TRUE;
        }

        if (!BSTRCMP(bp, p-4, "tree") && !madeterrain) {
            levl[u.ux][u.uy].typ = TREE;
            pline("A tree.");
            block_point(u.ux, u.uy);
            madeterrain = TRUE;
        }

        if (!BSTRCMP(bp, p-4, "bars")) {
            levl[u.ux][u.uy].typ = IRONBARS;
            pline("Iron bars.");
            madeterrain = TRUE;
        }

        if (madeterrain) {
            feel_newsym(x, y); /* map the spot where the wish occurred */
            /* hero started at <x,y> but might not be there anymore (create
               lava, decline to die, and get teleported away to safety) */
            if (u.uinwater && !is_pool(u.ux, u.uy)) {
                u.uinwater = 0; /* leave the water */
                docrt();
                vision_full_recalc = 1;
            } else if (u.utrap && u.utraptype == TT_LAVA && !is_lava(u.ux, u.uy)) {
                reset_utrap(FALSE);
            }
            /* cast 'const' away; caller won't modify this */
            return (struct obj *) &zeroobj;
        }
    }
#endif

    if (!oclass && !typ) {
        if (!strncmpi(bp, "polearm", 7)) {
            typ = rnd_otyp_by_wpnskill(P_POLEARMS);
            goto typfnd;
        } else if (!strncmpi(bp, "hammer", 6)) {
            typ = rnd_otyp_by_wpnskill(P_HAMMER);
            goto typfnd;
        }
    }

    if(!oclass) return((struct obj *)0);
any:
    if(!oclass) oclass = wrpsym[rn2((int)sizeof(wrpsym))];
typfnd:
    if (typ) oclass = objects[typ].oc_class;

    /* return a random dragon armor if user asks for an unknown
       dragon armor with its actual name */
    if (typ && found_by_actualn &&
        Is_dragon_armor(typ) &&
#ifdef WIZARD
        !wizard &&
#endif
        !objects[typ].oc_name_known) {
        typ = rn2(YELLOW_DRAGON_SCALES-GRAY_DRAGON_SCALES);
        if (Is_dragon_scales(typ))
            typ += GRAY_DRAGON_SCALES;
        else
            typ += GRAY_DRAGON_SCALE_MAIL;
    }

    /* check for some objects that are not allowed */
    if (typ && objects[typ].oc_unique) {
#ifdef WIZARD
        if (wizard)
            ; /* allow unique objects */
        else
#endif
        switch (typ) {
        case AMULET_OF_YENDOR:
            typ = FAKE_AMULET_OF_YENDOR;
            break;
        case CANDELABRUM_OF_INVOCATION:
            typ = rnd_class(TALLOW_CANDLE, WAX_CANDLE);
            break;
        case BELL_OF_OPENING:
            typ = BELL;
            break;
        case SPE_BOOK_OF_THE_DEAD:
            typ = SPE_BLANK_PAPER;
            break;
        }
    }

    /* catch any other non-wishable objects */
    if (objects[typ].oc_nowish
#ifdef WIZARD
        && !wizard
#endif
        )
        return((struct obj *)0);

    /*
     * Create the object, then fine-tune it.
     */
    if (typ) {
        otmp = mksobj(typ, TRUE, FALSE);
    } else {
        otmp = mkobj(oclass, FALSE);
    }
    /* what we actually got */
    typ = otmp->otyp;
    oclass = otmp->oclass;

    if (islit &&
        (typ == OIL_LAMP || typ == MAGIC_LAMP || typ == BRASS_LANTERN ||
         Is_candle(otmp) || typ == POT_OIL)) {
        place_object(otmp, u.ux, u.uy);  /* make it viable light source */
        begin_burn(otmp, FALSE);
        obj_extract_self(otmp);  /* now release it for caller's use */
    }

    /* if player specified a reasonable count, maybe honor it */
    if (cnt > 0 && objects[typ].oc_merge && oclass != SPBOOK_CLASS) {
        /* at most 10, with a mean of 5 */
        int max_amt = wizard ? cnt : rnf(1, 100) ? 10 : d(4, 3)-3;
        int min_amt = 1;

        if (max_amt <= 7 && Is_candle(otmp)) {
            /* the player might need them later */
            max_amt = 7;
        } else if ((oclass == WEAPON_CLASS && is_ammo(otmp)) ||
                    typ == ROCK ||
                    is_missile(otmp)) {
            /* up to about 100 projectiles */
            min_amt = 15 + rn2(11);
            max_amt = min_amt + rn2(80);
        }

        if (rnl(4) || wizard) {
            /* bad luck gets you only the minimum amount of items
             * if you wish for too many items */
            otmp->quan = (cnt <= max_amt) ? cnt : min_amt;
        } else {
            otmp->quan = (cnt <= max_amt) ? cnt : max_amt;
        }
    }

#ifdef WIZARD
    if (oclass == VENOM_CLASS) otmp->spe = 1;
#endif

    if (spesgn == 0) spe = otmp->spe;
#ifdef WIZARD
    else if (wizard) /* no alteration to spe */;
#endif
    else if (oclass == ARMOR_CLASS || oclass == WEAPON_CLASS ||
             is_weptool(otmp) ||
             (oclass==RING_CLASS && objects[typ].oc_charged)) {
        if(spe > rnd(5) && spe > otmp->spe) spe = 0;
        if(spe > 2 && Luck < 0) spesgn = -1;
    } else {
        if (oclass == WAND_CLASS) {
            if (spe > 1 && spesgn == -1) spe = 1;
        } else {
            if (spe > 0 && spesgn == -1) spe = 0;
        }
        if (spe > otmp->spe) spe = otmp->spe;
    }

    if (spesgn == -1) spe = -spe;

    /* set otmp->spe.  This may, or may not, use spe... */
    switch (typ) {
    case TIN: if (contents==EMPTY) {
            otmp->corpsenm = NON_PM;
            otmp->spe = 0;
    } else if (contents==SPINACH) {
            otmp->corpsenm = NON_PM;
            otmp->spe = 1;
    }
        break;
    case TOWEL:
        if (wetness) {
            otmp->spe = wetness;
        }
        break;
    case SLIME_MOLD: otmp->spe = ftype;
    /* Fall through */
    case SKELETON_KEY: case CHEST: case LARGE_BOX: case IRON_SAFE:
    case HEAVY_IRON_BALL: case IRON_CHAIN: case STATUE:
        /* otmp->cobj already done in mksobj() */
        break;
#ifdef MAIL
    case SCR_MAIL: otmp->spe = 1; break;
#endif
    case WAN_WISHING:
#ifdef WIZARD
        if (!wizard) {
#endif
        otmp->spe = (rn2(10) ? -1 : 0);
        break;
#ifdef WIZARD
        }
#endif
        /* fall through */
    default: otmp->spe = spe;
    }

    /* set otmp->corpsenm or dragon scale [mail] */
    if (mntmp >= LOW_PM) {
        if (mntmp == PM_LONG_WORM_TAIL) mntmp = PM_LONG_WORM;

        switch (typ) {
        case TIN:
            otmp->spe = 0; /* No spinach */
            if (dead_species(mntmp, FALSE)) {
                otmp->corpsenm = NON_PM;    /* it's empty */
            } else if (!(mons[mntmp].geno & G_UNIQ) &&
                       !(mvitals[mntmp].mvflags & G_NOCORPSE) &&
                       mons[mntmp].cnutrit != 0) {
                otmp->corpsenm = mntmp;
            }
            break;

        case CORPSE:
            if (!(mons[mntmp].geno & G_UNIQ) &&
                !(mvitals[mntmp].mvflags & G_NOCORPSE)) {
                if (mons[mntmp].msound == MS_GUARDIAN) {
                    mntmp = genus(mntmp, 1);
                }
                set_corpsenm(otmp, mntmp);
            }
            if (zombifying && zombie_form(&mons[mntmp])) {
                (void) start_timer(rn1(5, 10), TIMER_OBJECT, ZOMBIFY_MON, obj_to_any(otmp));
            }
            break;

        case FIGURINE:
            if (!(mons[mntmp].geno & G_UNIQ)
                && !is_human(&mons[mntmp])
#ifdef MAIL
                && mntmp != PM_MAIL_DAEMON
#endif
                )
                otmp->corpsenm = mntmp;
            break;

        case EGG:
            mntmp = can_be_hatched(mntmp);
            /* don't allow wishes for chromatic dragon eggs */
            if (mntmp == PM_CHROMATIC_DRAGON && !wizard) {
                mntmp = rn2(YELLOW_DRAGON_SCALES-GRAY_DRAGON_SCALES) + PM_GRAY_DRAGON;
            }
            set_corpsenm(otmp, mntmp);
            break;

        case STATUE: otmp->corpsenm = mntmp;
            if (Has_contents(otmp) && verysmall(&mons[mntmp]))
                delete_contents(otmp);  /* no spellbook */
            otmp->spe = ishistoric ? STATUE_HISTORIC : 0;
            break;
        case SCALE_MAIL:
            /* Dragon mail - depends on the order of objects */
            /*       & dragons.          */
            if (mntmp >= PM_GRAY_DRAGON &&
                mntmp <= PM_CHROMATIC_DRAGON) {
                if (mntmp == PM_CHROMATIC_DRAGON && !wizard) {
                    /* random dsm when asking for chromatic dsm */
                    otmp->otyp = rn2(YELLOW_DRAGON_SCALES-GRAY_DRAGON_SCALES) +
                                 GRAY_DRAGON_SCALE_MAIL;
                } else {
                    otmp->otyp = GRAY_DRAGON_SCALE_MAIL +
                                 mntmp - PM_GRAY_DRAGON;
                }
            }
            break;
        }
    }

    /* set blessed/cursed -- setting the fields directly is safe
     * since weight() is called below and addinv() will take care
     * of luck */
    if (iscursed) {
        curse(otmp);
    } else if (uncursed) {
        otmp->blessed = 0;
        otmp->cursed = (Luck < 0
#ifdef WIZARD
                        && !wizard
#endif
                        );
    } else if (blessed) {
        otmp->blessed = (Luck >= 0
#ifdef WIZARD
                         || wizard
#endif
                         );
        otmp->cursed = (Luck < 0
#ifdef WIZARD
                        && !wizard
#endif
                        );
    } else if (spesgn < 0) {
        curse(otmp);
    }

#ifdef INVISIBLE_OBJECTS
    if (isinvisible) otmp->oinvis = 1;
#endif

    /* set eroded and erodeproof */
    if (erosion_matters(otmp)) {
        if (eroded && (is_flammable(otmp) || is_rustprone(otmp))) {
            otmp->oeroded = eroded;
        }
        if (eroded2 && (is_corrodeable(otmp) || is_rottable(otmp))) {
            otmp->oeroded2 = eroded2;
        }
        /*
         * 3.6.1: earlier versions included `&& !eroded && !eroded2' here,
         * but damageproof combined with damaged is feasible (eroded
         * armor modified by confused reading of cursed destroy armor)
         * so don't prevent player from wishing for such a combination.
         */
        if (erodeproof && (is_damageable(otmp) || otmp->otyp == CRYSKNIFE)) {
            otmp->oerodeproof = (Luck >= 0 || wizard);
        }
    }

    /* set otmp->recharged */
    if (oclass == WAND_CLASS) {
        /* prevent wishing abuse */
        if (otmp->otyp == WAN_WISHING
#ifdef WIZARD
            && !wizard
#endif
            ) rechrg = 1;
        otmp->recharged = (unsigned)rechrg;
    }

    /* set poisoned */
    if (ispoisoned) {
        if (is_poisonable(otmp))
            otmp->opoisoned = (Luck >= 0);
        else if (oclass == FOOD_CLASS)
            /* try to taint by making it as old as possible */
            otmp->age = 1L;
    }
    /* and [un]trapped */
    if (trapped) {
        if (Is_box(otmp) || typ == TIN) {
            otmp->otrapped = (trapped == 1);
        }
    }
    /* empty for containers rather than for tins */
    if (contents == EMPTY) {
        if (otmp->otyp == BAG_OF_TRICKS || otmp->otyp == HORN_OF_PLENTY) {
            if (otmp->spe > 0) {
                otmp->spe = 0;
            }
        } else if (Has_contents(otmp)) {
            /* this assumes that artifacts can't be randomly generated
               inside containers */
            delete_contents(otmp);
            otmp->owt = weight(otmp);
        }
    }
    /* set locked/unlocked/broken */
    if (Is_box(otmp)) {
        if (locked) {
            otmp->olocked = 1, otmp->obroken = 0;
        } else if (unlocked) {
            otmp->olocked = 0, otmp->obroken = 0;
        } else if (broken) {
            otmp->olocked = 0, otmp->obroken = 1;
        }
    }

    if (isgreased) otmp->greased = 1;

    if (isdiluted && otmp->oclass == POTION_CLASS &&
        otmp->otyp != POT_WATER)
        otmp->odiluted = 1;

    /* set tin variety */
    if (otmp->otyp == TIN && tvariety >= 0 && (rn2(4) || wizard)) {
        set_tin_variety(otmp, tvariety);
    }

    if (name) {
        const char *aname;
        short objtyp;

        /* an artifact name might need capitalization fixing */
        aname = artifact_name(name, &objtyp);
        if (aname && objtyp == otmp->otyp) name = aname;

        otmp = oname(otmp, name);
        /* name==aname => wished for artifact (otmp->oartifact => got it) */
        if (otmp->oartifact || name == aname) {
            otmp->quan = 1L;
            u.uconduct.wisharti++;  /* KMH, conduct */
        }
    }

    /* more wishing abuse: don't allow wishing for certain artifacts */
    /* and make them pay; charge them for the wish anyway! */
    if ((is_quest_artifact(otmp) ||
         (otmp->oartifact && otmp->oartifact == ART_THIEFBANE))
#ifdef WIZARD
        && !wizard
#endif
        ) {
        artifact_exists(otmp, ONAME(otmp), FALSE);
        obfree(otmp, (struct obj *) 0);
        otmp = &zeroobj;
        pline("For a moment, you feel %s in your %s, but it disappears!",
              something,
              makeplural(body_part(HAND)));
        return otmp;
    }

    if (halfeaten && otmp->oclass == FOOD_CLASS) {
        if (otmp->otyp == CORPSE)
            otmp->oeaten = mons[otmp->corpsenm].cnutrit;
        else otmp->oeaten = objects[otmp->otyp].oc_nutrition;
        /* (do this adjustment before setting up object's weight) */
        consume_oeaten(otmp, 1);
    }
    if (isdrained && otmp->otyp == CORPSE && mons[otmp->corpsenm].cnutrit) {
        int amt;
        otmp->odrained = 1;
        amt = mons[otmp->corpsenm].cnutrit - drainlevel(otmp);
        if (halfdrained) {
            amt /= 2;
            if (amt == 0)
                amt++;
        }
        /* (do this adjustment before setting up object's weight) */
        consume_oeaten(otmp, -amt);
    }
    otmp->owt = weight(otmp);
    if (very && otmp->otyp == HEAVY_IRON_BALL) {
        otmp->owt += IRON_BALL_W_INCR;
    }

    return(otmp);
}

int
rnd_class(first, last)
int first, last;
{
    int i, x, sum=0;

    if (first == last)
        return (first);
    for(i=first; i<=last; i++)
        sum += objects[i].oc_prob;
    if (!sum) /* all zero */
        return first + rn2(last-first+1);
    x = rnd(sum);
    for(i=first; i<=last; i++)
        if (objects[i].oc_prob && (x -= objects[i].oc_prob) <= 0)
            return i;
    return 0;
}

STATIC_OVL const char *
Japanese_item_name(i)
int i;
{
    struct Jitem *j = Japanese_items;

    while(j->item) {
        if (i == j->item)
            return j->name;
        j++;
    }
    return (const char *)0;
}

const char *
suit_simple_name(suit)
struct obj *suit;
{
    if (suit) {
        if (Is_dragon_mail(suit->otyp)) {
            return "dragon mail"; /* <color> dragon scale mail */
        } else if (Is_dragon_scales(suit->otyp)) {
            return "dragon scales";
        }

        const char *suitnm = OBJ_NAME(objects[suit->otyp]);
        const char *esuitp = eos((char *) suitnm);
        if (strlen(suitnm) > 5 && !strcmp(esuitp - 5, " mail")) {
            return "mail"; /* most suits fall into this category */
        } else if (strlen(suitnm) > 7 && !strcmp(esuitp - 7, " jacket")) {
            return "jacket"; /* leather jacket */
        }
    }
    /* "suit" is lame but "armor" is ambiguous and "body armor" is absurd */
    return "suit";
}

const char *
cloak_simple_name(cloak)
struct obj *cloak;
{
    if (cloak) {
        switch (cloak->otyp) {
        case ROBE:
            return "robe";
        case MUMMY_WRAPPING:
            return "wrapping";
        case ALCHEMY_SMOCK:
            return (objects[cloak->otyp].oc_name_known &&
                    cloak->dknown) ? "smock" : "apron";
        default:
            break;
        }
    }
    return "cloak";
}

/* helm vs hat for messages */
const char *
helm_simple_name(helmet)
struct obj *helmet;
{
    /*
     *  There is some wiggle room here; the result has been chosen
     *  for consistency with the "protected by hard helmet" messages
     *  given for various bonks on the head:  headgear that provides
     *  such protection is a "helm", that which doesn't is a "hat".
     *
     *      elven leather helm / leather hat    -> hat
     *      dwarvish iron helm / hard hat       -> helm
     *  The rest are completely straightforward:
     *      fedora, cornuthaum, dunce cap       -> hat
     *      all other types of helmets          -> helm
     */
    return (helmet && !is_metallic(helmet)) ? "hat" : "helm";
}

/* gloves vs gauntlets; depends upon discovery state */
const char *
gloves_simple_name(gloves)
struct obj *gloves;
{
    static const char gauntlets[] = "gauntlets";

    if (gloves && gloves->dknown) {
        int otyp = gloves->otyp;
        struct objclass *ocl = &objects[otyp];
        const char *actualn = OBJ_NAME(*ocl);
        const char *descrpn = OBJ_DESCR(*ocl);

        if (strstri(objects[otyp].oc_name_known ? actualn : descrpn, gauntlets)) {
            return gauntlets;
        }
    }
    return "gloves";
}

const char *
mimic_obj_name(mtmp)
struct monst *mtmp;
{
    if (M_AP_TYPE(mtmp) == M_AP_OBJECT) {
        if (mtmp->mappearance == GOLD_PIECE) {
            return "gold";
        }
        if (mtmp->mappearance != STRANGE_OBJECT) {
            return simple_typename(mtmp->mappearance);
        }
    }
    return "whatcha-may-callit";
}

/*
 * Construct a query prompt string, based around an object name, which is
 * guaranteed to fit within [QBUFSZ].  Takes an optional prefix, three
 * choices for filling in the middle (two object formatting functions and a
 * last resort literal which should be very short), and an optional suffix.
 */
char *
safe_qbuf(qbuf, qprefix, qsuffix, obj, func, altfunc, lastR)
char *qbuf; /* output buffer */
const char *qprefix, *qsuffix;
struct obj *obj;
char *FDECL((*func), (OBJ_P)), *FDECL((*altfunc), (OBJ_P));
const char *lastR;
{
    char *bufp, *endp;
    /* convert size_t (or int for ancient systems) to ordinary unsigned */
    unsigned len, lenlimit,
        len_qpfx = (unsigned) (qprefix ? strlen(qprefix) : 0),
        len_qsfx = (unsigned) (qsuffix ? strlen(qsuffix) : 0),
        len_lastR = (unsigned) strlen(lastR);

    lenlimit = QBUFSZ - 1;
    endp = qbuf + lenlimit;
    /* sanity check, aimed mainly at paniclog (it's conceivable for
       the result of short_oname() to be shorter than the length of
       the last resort string, but we ignore that possibility here) */
    if (len_qpfx > lenlimit)
        impossible("safe_qbuf: prefix too long (%u characters).", len_qpfx);
    else if (len_qpfx + len_qsfx > lenlimit)
        impossible("safe_qbuf: suffix too long (%u + %u characters).",
                   len_qpfx, len_qsfx);
    else if (len_qpfx + len_lastR + len_qsfx > lenlimit)
        impossible("safe_qbuf: filler too long (%u + %u + %u characters).",
                   len_qpfx, len_lastR, len_qsfx);

    /* the output buffer might be the same as the prefix if caller
       has already partially filled it */
    if (qbuf == qprefix) {
        /* prefix is already in the buffer */
        *endp = '\0';
    } else if (qprefix) {
        /* put prefix into the buffer */
        (void) strncpy(qbuf, qprefix, lenlimit);
        *endp = '\0';
    } else {
        /* no prefix; output buffer starts out empty */
        qbuf[0] = '\0';
    }
    len = (unsigned) strlen(qbuf);

    if (len + len_lastR + len_qsfx > lenlimit) {
        /* too long; skip formatting, last resort output is truncated */
        if (len < lenlimit) {
            (void) strncpy(&qbuf[len], lastR, lenlimit - len);
            *endp = '\0';
            len = (unsigned) strlen(qbuf);
            if (qsuffix && len < lenlimit) {
                (void) strncpy(&qbuf[len], qsuffix, lenlimit - len);
                *endp = '\0';
                /* len = (unsigned) strlen(qbuf); */
            }
        }
    } else {
        /* suffix and last resort are guaranteed to fit */
        len += len_qsfx; /* include the pending suffix */
        /* format the object */
        bufp = short_oname(obj, func, altfunc, lenlimit - len);
        if (len + strlen(bufp) <= lenlimit)
            Strcat(qbuf, bufp); /* formatted name fits */
        else
            Strcat(qbuf, lastR); /* use last resort */
        releaseobuf(bufp);

        if (qsuffix)
            Strcat(qbuf, qsuffix);
    }
    /* assert( strlen(qbuf) < QBUFSZ ); */
    return qbuf;
}

/*objnam.c*/
