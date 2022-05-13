/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"    /* save & restore info */

STATIC_DCL void FDECL(setgemprobs, (d_level*));
STATIC_DCL void FDECL(shuffle, (int, int, BOOLEAN_P));
STATIC_DCL void NDECL(shuffle_all);
STATIC_DCL boolean FDECL(interesting_to_discover, (int));
STATIC_DCL void FDECL(swap_armor, (int, int, int));
static char *FDECL(oclass_to_name, (CHAR_P, char *));

static NEARDATA short disco[NUM_OBJECTS] = DUMMY;

#ifdef USE_TILES
STATIC_DCL void NDECL(shuffle_tiles);
extern short glyph2tile[];  /* from tile.c */

/* Shuffle tile assignments to match descriptions, so a red potion isn't
 * displayed with a blue tile and so on.
 *
 * Tile assignments are not saved, and shouldn't be so that a game can
 * be resumed on an otherwise identical non-tile-using binary, so we have
 * to reshuffle the assignments from oc_descr_idx information when a game
 * is restored.  So might as well do that the first time instead of writing
 * another routine.
 */
STATIC_OVL void
shuffle_tiles()
{
    int i;
    short tmp_tilemap[NUM_OBJECTS];

    for (i = 0; i < NUM_OBJECTS; i++)
        tmp_tilemap[i] =
            glyph2tile[objects[i].oc_descr_idx + GLYPH_OBJ_OFF];

    for (i = 0; i < NUM_OBJECTS; i++)
        glyph2tile[i + GLYPH_OBJ_OFF] = tmp_tilemap[i];
}
#endif  /* USE_TILES */

STATIC_OVL void
setgemprobs(dlev)
d_level *dlev;
{
    int j, first, lev;

    if (dlev)
        lev = (ledger_no(dlev) > maxledgerno())
              ? maxledgerno() : ledger_no(dlev);
    else
        lev = 0;
    first = bases[GEM_CLASS];

    for(j = 0; j < 9-lev/3; j++)
        objects[first+j].oc_prob = 0;
    first += j;
    if (first > LAST_GEM || objects[first].oc_class != GEM_CLASS ||
        OBJ_NAME(objects[first]) == (char *)0) {
        raw_printf("Not enough gems? - first=%d j=%d LAST_GEM=%d",
                   first, j, LAST_GEM);
        wait_synch();
    }
    for (j = first; j <= LAST_GEM; j++)
        objects[j].oc_prob = (171+j-first)/(LAST_GEM+1-first);
}

/* shuffle descriptions on objects o_low to o_high */
STATIC_OVL void
shuffle(o_low, o_high, domaterial)
int o_low, o_high;
boolean domaterial;
{
    int i, j, num_to_shuffle;
    short sw;
    int color;

    for (num_to_shuffle = 0, j=o_low; j <= o_high; j++)
        if (!objects[j].oc_name_known) num_to_shuffle++;
    if (num_to_shuffle < 2) return;

    for (j=o_low; j <= o_high; j++) {
        if (objects[j].oc_name_known) continue;
        do
            i = j + rn2(o_high-j+1);
        while (objects[i].oc_name_known);
        sw = objects[j].oc_descr_idx;
        objects[j].oc_descr_idx = objects[i].oc_descr_idx;
        objects[i].oc_descr_idx = sw;
        sw = objects[j].oc_tough;
        objects[j].oc_tough = objects[i].oc_tough;
        objects[i].oc_tough = sw;
        color = objects[j].oc_color;
        objects[j].oc_color = objects[i].oc_color;
        objects[i].oc_color = color;

        /* shuffle material */
        if (domaterial) {
            sw = objects[j].oc_material;
            objects[j].oc_material = objects[i].oc_material;
            objects[i].oc_material = sw;
        }
    }
}

void
init_objects()
{
    register int i, first, last, sum;
    register char oclass;
#ifdef TEXTCOLOR
# define COPY_OBJ_DESCR(o_dst, o_src) \
    o_dst.oc_descr_idx = o_src.oc_descr_idx, \
    o_dst.oc_color = o_src.oc_color
#else
# define COPY_OBJ_DESCR(o_dst, o_src) o_dst.oc_descr_idx = o_src.oc_descr_idx
#endif

    /* bug fix to prevent "initialization error" abort on Intel Xenix.
     * reported by mikew@semike
     */
    for (i = 0; i < MAXOCLASSES; i++)
        bases[i] = 0;
    /* initialize object descriptions */
    for (i = 0; i < NUM_OBJECTS; i++)
        objects[i].oc_name_idx = objects[i].oc_descr_idx = i;
    /* init base; if probs given check that they add up to 1000,
       otherwise compute probs */
    first = 0;
    while( first < NUM_OBJECTS ) {
        oclass = objects[first].oc_class;
        last = first+1;
        while (last < NUM_OBJECTS && objects[last].oc_class == oclass) last++;
        bases[(int)oclass] = first;

        if (oclass == GEM_CLASS) {
            setgemprobs((d_level *)0);

            if (rn2(2)) { /* change turquoise from green to blue? */
                COPY_OBJ_DESCR(objects[TURQUOISE], objects[SAPPHIRE]);
            }
            if (rn2(2)) { /* change aquamarine from green to blue? */
                COPY_OBJ_DESCR(objects[AQUAMARINE], objects[SAPPHIRE]);
            }
            switch (rn2(4)) { /* change fluorite from violet? */
            case 0:  break;
            case 1:     /* blue */
                COPY_OBJ_DESCR(objects[FLUORITE], objects[SAPPHIRE]);
                break;
            case 2:     /* white */
                COPY_OBJ_DESCR(objects[FLUORITE], objects[DIAMOND]);
                break;
            case 3:     /* green */
                COPY_OBJ_DESCR(objects[FLUORITE], objects[EMERALD]);
                break;
            }
        }
check:
        sum = 0;
        for(i = first; i < last; i++) sum += objects[i].oc_prob;
        if(sum == 0) {
            for(i = first; i < last; i++)
                objects[i].oc_prob = (1000+i-first)/(last-first);
            goto check;
        }
        if(sum != 1000)
            error("init-prob error for class %d (%d%%)", oclass, sum);
        first = last;
    }
    /* shuffle descriptions */
    shuffle_all();
#ifdef USE_TILES
    shuffle_tiles();
#endif
}

STATIC_OVL void
shuffle_all()
{
    int first, last, oclass;

    for (oclass = 1; oclass < MAXOCLASSES; oclass++) {
        first = bases[oclass];
        last = first+1;
        while (last < NUM_OBJECTS && objects[last].oc_class == oclass)
            last++;

        if (OBJ_DESCR(objects[first]) != (char *)0 &&
            oclass != TOOL_CLASS &&
            oclass != WEAPON_CLASS &&
            oclass != ARMOR_CLASS &&
            oclass != POTION_CLASS &&
            oclass != GEM_CLASS &&
            oclass != VENOM_CLASS) {
            int j = last-1;

            if (oclass == AMULET_CLASS ||
                 oclass == SCROLL_CLASS ||
                 oclass == SPBOOK_CLASS) {
                while (!objects[j].oc_magic || objects[j].oc_unique) {
                    j--;
                }
            }

            /* non-magical amulets, scrolls, and spellbooks
             * (ex. imitation Amulets, blank, scrolls of mail)
             * and one-of-a-kind magical artifacts at the end of
             * their class in objects[] have fixed descriptions.
             */
            shuffle(first, j, TRUE);
        }
    }

    /* shuffle the helmets */
    shuffle(HELMET, HELM_OF_TELEPATHY, FALSE);

    /* shuffle the gloves */
    shuffle(LEATHER_GLOVES, GAUNTLETS_OF_DEXTERITY, FALSE);

    /* shuffle the cloaks */
    shuffle(CLOAK_OF_PROTECTION, CLOAK_OF_DISPLACEMENT, FALSE);

    /* shuffle the boots [if they change, update find_skates() below] */
    shuffle(SPEED_BOOTS, LEVITATION_BOOTS, FALSE);

    /* shuffle dragon scales / scale mails */
    int j, pos;
    int num_scales = YELLOW_DRAGON_SCALES - GRAY_DRAGON_SCALES;
    for (j = num_scales; j >= 0; j--) {
        pos = rn2(j+1);
        swap_armor(j, pos, GRAY_DRAGON_SCALES);
        swap_armor(j, pos, GRAY_DRAGON_SCALE_MAIL);
    }

    /* Shuffle the potions such that all descriptions that participate in
     * color alchemy are mapped to potions that actually exist in the game,
     * so that they can always be alchemized.
     *
     * Assumes:
     * - POT_POLYMORPH maps to the last color description.
     * - water and beyond have fixed descs
     * - POT_OIL is the last shuffled potion that isn't just a description.
     */
    /* Shuffle the non-color potion descriptions. */
    shuffle(POT_POLYMORPH + 1, POT_WATER - 1, TRUE);
    /* Shuffle potions that will exist with no description guarantees. */
    shuffle(bases[POTION_CLASS], POT_OIL, TRUE);
}

/**
 * Swaps two items of the same armor class.
 * Currently name, color and price are swapped.
 */
void
swap_armor(old_relative_position, new_relative_position, first)
int old_relative_position,  /* old position of dragon scales */
new_relative_position,      /* new position of dragon scales */
first;     /* first armor of this armor class */
{
    struct objclass tmp;

    int old_pos = old_relative_position + first;
    int new_pos = new_relative_position + first;

    tmp.oc_descr_idx = objects[old_pos].oc_descr_idx;
    tmp.oc_color    = objects[old_pos].oc_color;
    tmp.oc_cost     = objects[old_pos].oc_cost;

    objects[old_pos].oc_descr_idx = objects[new_pos].oc_descr_idx;
    objects[old_pos].oc_color    = objects[new_pos].oc_color;
    objects[old_pos].oc_cost     = objects[new_pos].oc_cost;

    objects[new_pos].oc_descr_idx = tmp.oc_descr_idx;
    objects[new_pos].oc_color    = tmp.oc_color;
    objects[new_pos].oc_cost     = tmp.oc_cost;
}

/* find the object index for snow boots; used [once] by slippery ice code */
int
find_skates()
{
    register int i;
    register const char *s;

    for (i = SPEED_BOOTS; i <= LEVITATION_BOOTS; i++)
        if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "snow boots"))
            return i;

    impossible("snow boots not found?");
    return -1;  /* not 0, or caller would try again each move */
}

/* level dependent initialization */
void
oinit()
{
    setgemprobs(&u.uz);
}

void
savenames(fd, mode)
int fd, mode;
{
    register int i;
    unsigned int len;

    if (perform_bwrite(mode)) {
        bwrite(fd, (genericptr_t)bases, sizeof bases);
        bwrite(fd, (genericptr_t)disco, sizeof disco);
        bwrite(fd, (genericptr_t)objects,
               sizeof(struct objclass) * NUM_OBJECTS);
    }
    /* as long as we use only one version of Hack we
       need not save oc_name and oc_descr, but we must save
       oc_uname for all objects */
    for (i = 0; i < NUM_OBJECTS; i++)
        if (objects[i].oc_uname) {
            if (perform_bwrite(mode)) {
                len = strlen(objects[i].oc_uname)+1;
                bwrite(fd, (genericptr_t)&len, sizeof len);
                bwrite(fd, (genericptr_t)objects[i].oc_uname, len);
            }
            if (release_data(mode)) {
                free((genericptr_t)objects[i].oc_uname);
                objects[i].oc_uname = 0;
            }
        }
}

void
restnames(fd)
register int fd;
{
    register int i;
    unsigned int len;

    mread(fd, (genericptr_t) bases, sizeof bases);
    mread(fd, (genericptr_t) disco, sizeof disco);
    mread(fd, (genericptr_t) objects, sizeof(struct objclass) * NUM_OBJECTS);
    for (i = 0; i < NUM_OBJECTS; i++)
        if (objects[i].oc_uname) {
            mread(fd, (genericptr_t) &len, sizeof len);
            objects[i].oc_uname = (char *) alloc(len);
            mread(fd, (genericptr_t)objects[i].oc_uname, len);
        }
#ifdef USE_TILES
    shuffle_tiles();
#endif
}

void
discover_object(oindx, mark_as_known, credit_hero)
register int oindx;
boolean mark_as_known;
boolean credit_hero;
{
    if (!objects[oindx].oc_name_known) {
        register int dindx, acls = objects[oindx].oc_class;

        /* Loop thru disco[] 'til we find the target (which may have been
           uname'd) or the next open slot; one or the other will be found
           before we reach the next class...
         */
        for (dindx = bases[acls]; disco[dindx] != 0; dindx++)
            if (disco[dindx] == oindx) break;
        disco[dindx] = oindx;

        if (mark_as_known) {
            objects[oindx].oc_name_known = 1;
            if (credit_hero) exercise(A_WIS, TRUE);

            if (Is_dragon_scales(oindx))
                discover_object(Dragon_scales_to_mail(oindx), mark_as_known, FALSE);
            else if (Is_dragon_mail(oindx))
                discover_object(Dragon_mail_to_scales(oindx), mark_as_known, FALSE);
        }
        if (moves > 1L) update_inventory();
    }
}

/* if a class name has been cleared, we may need to purge it from disco[] */
void
undiscover_object(oindx)
register int oindx;
{
    if (!objects[oindx].oc_name_known) {
        register int dindx, acls = objects[oindx].oc_class;
        register boolean found = FALSE;

        /* find the object; shift those behind it forward one slot */
        for (dindx = bases[acls];
             dindx < NUM_OBJECTS && disco[dindx] != 0
             && objects[dindx].oc_class == acls; dindx++)
            if (found)
                disco[dindx-1] = disco[dindx];
            else if (disco[dindx] == oindx)
                found = TRUE;

        /* clear last slot */
        if (found) disco[dindx-1] = 0;
#if 0
        /* don't freak out about already forgotten stuff */
        else warning("named object \"%s\"(index %d; base: %d; end: %d) not found in discovery list.",
                     OBJ_NAME(objects[oindx]), oindx, bases[acls], dindx);
#endif

        update_inventory();
    }
}

void
makeknown_msg(int otyp)
{
    boolean was_known, now_known;
    char oclass = objects[otyp].oc_class;
    schar osubtyp = objects[otyp].oc_subtyp;

    was_known = objects[otyp].oc_name_known;
    makeknown(otyp);
    now_known = objects[otyp].oc_name_known;

    if (flags.verbose && !was_known && now_known) {
        if (otyp == LENSES ||
            (oclass == ARMOR_CLASS &&
             (osubtyp == ARM_BOOTS || osubtyp == ARM_GLOVES))) {
            pline("They must be %s!", simple_typename(otyp));
        } else {
            pline("It must be %s!", an(simple_typename(otyp)));
        }
    }
}

static boolean
interesting_to_discover(i)
int i;
{
    /* Pre-discovered objects are now printed with a '*' */
    return((boolean)(objects[i].oc_uname != (char *)0 ||
                     (objects[i].oc_name_known && OBJ_DESCR(objects[i]) != (char *)0)));
}

/* items that should stand out once they're known */
static short uniq_objs[] = {
    AMULET_OF_YENDOR,
    SPE_BOOK_OF_THE_DEAD,
    CANDELABRUM_OF_INVOCATION,
    BELL_OF_OPENING,
};

/* the '\' command - show discovered object types */
int
dodiscovered() /* free after Robert Viduya */
{
    register int i, dis;
    int ct = 0;
    char *s, oclass, prev_class, classes[MAXOCLASSES];
    winid tmpwin;
    char buf[BUFSZ];

    tmpwin = create_nhwindow(NHW_MENU);
    putstr(tmpwin, 0, "Discoveries");
    putstr(tmpwin, 0, "");

    /* gather "unique objects" into a pseudo-class; note that they'll
       also be displayed individually within their regular class */
    for (i = dis = 0; i < SIZE(uniq_objs); i++) {
        if (objects[uniq_objs[i]].oc_name_known) {
            if (!dis++) {
                putstr(tmpwin, iflags.menu_headings, "Unique items");
            }
            Sprintf(buf, "  %s", OBJ_NAME(objects[uniq_objs[i]]));
            putstr(tmpwin, 0, buf);
            ++ct;
        }
    }
    /* display any known artifacts as another pseudo-class */
    ct += disp_artifact_discoveries(tmpwin);

    /* several classes are omitted from packorder; one is of interest here */
    Strcpy(classes, flags.inv_order);
    if (!index(classes, VENOM_CLASS)) {
        (void) strkitten(classes, VENOM_CLASS); /* append char to string */
    }

    for (s = classes; *s; s++) {
        oclass = *s;
        prev_class = oclass + 1; /* forced different from oclass */
        for (i = bases[(int)oclass]; i < NUM_OBJECTS && objects[i].oc_class == oclass; i++) {
            if ((dis = disco[i]) && interesting_to_discover(dis)) {
                ct++;
                if (oclass != prev_class) {
                    putstr(tmpwin, iflags.menu_headings, let_to_name(oclass, FALSE));
                    prev_class = oclass;
                }
                Sprintf(buf, "%s %s", (objects[dis].oc_pre_discovered ? "*" : " "),
                        obj_typename(dis));
                putstr(tmpwin, 0, buf);
            }
        }
    }
    if (ct == 0) {
        You("haven't discovered anything yet...");
    } else {
        display_nhwindow(tmpwin, TRUE);
    }
    destroy_nhwindow(tmpwin);

    return 0;
}

/**
 * Shuffles the properties of dragons according to the order of the dragons
 * scales.
 * Currently name, color are shuffled.
 */
void
dragons_init()
{
    /* Number of existing dragons. Assumes order of dragons */
    int ndragons = YELLOW_DRAGON_SCALES - GRAY_DRAGON_SCALES + 1;
    struct permonst tmp[ndragons];
    struct permonst baby_tmp[ndragons];
    int i, j;
    /* record standard order */
    for (i=0; i < ndragons; i++) {
        tmp[i].mname  = mons[i+PM_GRAY_DRAGON].mname;
        tmp[i].mcolor = mons[i+PM_GRAY_DRAGON].mcolor;
        baby_tmp[i].mname  = mons[i+PM_BABY_GRAY_DRAGON].mname;
        baby_tmp[i].mcolor = mons[i+PM_BABY_GRAY_DRAGON].mcolor;
    }
    /* copy name to new positions */
    for (i=0; i < ndragons; i++) {
        j = objects[i+GRAY_DRAGON_SCALES].oc_descr_idx-GRAY_DRAGON_SCALES;
        mons[i+PM_GRAY_DRAGON].mname  = tmp[j].mname;
        mons[i+PM_BABY_GRAY_DRAGON].mname  = baby_tmp[j].mname;
    }
    /* assign colors, brown for unidentified dragons */
    for (i=0; i < ndragons; i++) {
        if (objects[i+GRAY_DRAGON_SCALES].oc_name_known) { /* we know this dragon type */
            identify_dragon(i);
        } else {
            mons[i+PM_GRAY_DRAGON].mcolor = CLR_BROWN;
            mons[i+PM_BABY_GRAY_DRAGON].mcolor = CLR_BROWN;
            objects[i + GRAY_DRAGON_SCALES].oc_color = CLR_BROWN;
            objects[i + GRAY_DRAGON_SCALE_MAIL].oc_color = CLR_BROWN;
        }
    }
}

/* the '`' command - show discovered object types for one class */
int
doclassdisco()
{
    static NEARDATA const char
        prompt[] = "View discoveries for which sort of objects?",
        havent_discovered_any[] = "haven't discovered any %s yet.",
        unique_items[] = "unique items",
        artifact_items[] = "artifacts";
    char *s, c, oclass, menulet, allclasses[MAXOCLASSES];
    char discosyms[2 + MAXOCLASSES + 1], buf[BUFSZ];
    int i, ct, dis, xtras;
    boolean traditional;
    winid tmpwin = WIN_ERR;
    anything any;
    menu_item *pick_list = 0;

    discosyms[0] = '\0';
    traditional = (flags.menu_style == MENU_TRADITIONAL ||
                   flags.menu_style == MENU_COMBINATION);
    if (!traditional) {
        tmpwin = create_nhwindow(NHW_MENU);
        start_menu(tmpwin);
    }
    any = zeroany;
    menulet = 'a';

    /* check whether we've discovered any unique objects */
    for (i = 0; i < SIZE(uniq_objs); i++) {
        if (objects[uniq_objs[i]].oc_name_known) {
            Strcat(discosyms, "u");
            if (!traditional) {
                any.a_int = 'u';
                add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, menulet++, 0, ATR_NONE,
                         unique_items, MENU_UNSELECTED);
            }
            break;
        }
    }

    /* check whether we've discovered any artifacts */
    if (disp_artifact_discoveries(WIN_ERR) > 0) {
        Strcat(discosyms, "a");
        if (!traditional) {
            any.a_int = 'a';
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, menulet++, 0, ATR_NONE,
                     artifact_items, MENU_UNSELECTED);
        }
    }

    /* collect classes with discoveries, in packorder ordering; several
       classes are omitted from packorder and one is of interest here */
    Strcpy(allclasses, flags.inv_order);
    if (!index(allclasses, VENOM_CLASS)) {
        (void) strkitten(allclasses, VENOM_CLASS); /* append char to string */
    }
    /* construct discosyms[] */
    for (s = allclasses; *s; ++s) {
        oclass = *s;
        c = def_oc_syms[(int) oclass];
        for (i = bases[(int) oclass]; i < NUM_OBJECTS && objects[i].oc_class == oclass; ++i) {
            if ((dis = disco[i]) != 0 && interesting_to_discover(dis)) {
                if (!index(discosyms, c)) {
                    Sprintf(eos(discosyms), "%c", c);
                    if (!traditional) {
                        any.a_int = c;
                        add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, menulet++, c,
                                 ATR_NONE, let_to_name(oclass, FALSE),
                                 MENU_UNSELECTED);
                    }
                }
            }
        }
    }

    /* there might not be anything for us to do... */
    if (!discosyms[0]) {
        You(havent_discovered_any, "items");
        if (tmpwin != WIN_ERR) {
            destroy_nhwindow(tmpwin);
        }
        return 0;
    }

    /* have player choose a class */
    c = '\0'; /* class not chosen yet */
    if (traditional) {
        /* we'll prompt even if there's only one viable class; we add all
           nonviable classes as unseen acceptable choices so player can ask
           for discoveries of any class whether it has discoveries or not */
        for (s = allclasses, xtras = 0; *s; ++s) {
            c = def_oc_syms[(int) *s];
            if (!index(discosyms, c)) {
                if (!xtras++) {
                    (void) strkitten(discosyms, '\033');
                }
                (void) strkitten(discosyms, c);
            }
        }
        /* get the class (via its symbol character) */
        c = yn_function(prompt, discosyms, '\0');
        savech(c);
        if (!c) {
            clear_nhwindow(WIN_MESSAGE);
        }
    } else {
        /* menustyle:full or menustyle:partial */
        if (!discosyms[1] && flags.menu_style == MENU_PARTIAL) {
            /* only one class; menustyle:partial normally jumps past class
               filtering straight to final menu so skip class filter here */
            c = discosyms[0];
        } else {
            /* more than one choice, or menustyle:full which normally has
               an intermediate class selection menu before the final menu */
            end_menu(tmpwin, prompt);
            i = select_menu(tmpwin, PICK_ONE, &pick_list);
            if (i > 0) {
                c = pick_list[0].item.a_int;
                free(pick_list);
            } /* else c stays 0 */
        }
        destroy_nhwindow(tmpwin);
    }
    if (!c) {
        return 0; /* player declined to make a selection */
    }

    /*
     * show discoveries for object class c
     */
    tmpwin = create_nhwindow(NHW_MENU);
    ct = 0;
    switch (c) {
    case 'u':
        putstr(tmpwin, iflags.menu_headings,
               upstart(strcpy(buf, unique_items)));
        for (i = 0; i < SIZE(uniq_objs); i++) {
            if (objects[uniq_objs[i]].oc_name_known) {
                Sprintf(buf, "  %s", OBJ_NAME(objects[uniq_objs[i]]));
                putstr(tmpwin, 0, buf);
                ++ct;
            }
        }
        if (!ct) {
            You(havent_discovered_any, unique_items);
        }
        break;
    case 'a':
        /* disp_artifact_discoveries() includes a header */
        ct = disp_artifact_discoveries(tmpwin);
        if (!ct) {
            You(havent_discovered_any, artifact_items);
        }
        break;
    default:
        oclass = def_char_to_objclass(c);
        Sprintf(buf, "Discovered %s", let_to_name(oclass, FALSE));
        putstr(tmpwin, iflags.menu_headings, buf);
        for (i = bases[(int) oclass]; i < NUM_OBJECTS && objects[i].oc_class == oclass; ++i) {
            if ((dis = disco[i]) != 0 && interesting_to_discover(dis)) {
                Sprintf(buf, "%s %s",
                        objects[dis].oc_pre_discovered ? "*" : " ",
                        obj_typename(dis));
                putstr(tmpwin, 0, buf);
                ++ct;
            }
        }
        if (!ct) {
            You(havent_discovered_any, let_to_name(oclass, FALSE));
        }
        break;
    }
    if (ct) {
        display_nhwindow(tmpwin, TRUE);
    }
    destroy_nhwindow(tmpwin);
    return 0;
}

/* put up nameable subset of discoveries list as a menu */
void
rename_disco()
{
    register int i, dis;
    int ct = 0, mn = 0, sl;
    char *s, oclass, prev_class;
    winid tmpwin;
    anything any;
    menu_item *selected = 0;

    any = zeroany;
    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);

    /*
     * Skip the "unique objects" section (each will appear within its
     * regular class if it is nameable) and the artifacts section.
     * We assume that classes omitted from packorder aren't nameable
     * so we skip venom too.
     */

    /* for each class, show discoveries in that class */
    for (s = flags.inv_order; *s; s++) {
        oclass = *s;
        prev_class = oclass + 1; /* forced different from oclass */
        for (i = bases[(int) oclass]; i < NUM_OBJECTS && objects[i].oc_class == oclass; i++) {
            dis = disco[i];
            if (!dis || !interesting_to_discover(dis)) {
                continue;
            }
            ct++;
            if (!objtyp_is_callable(dis)) {
                continue;
            }
            mn++;

            if (oclass != prev_class) {
                any.a_int = 0;
                add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, iflags.menu_headings,
                         let_to_name(oclass, FALSE), MENU_UNSELECTED);
                prev_class = oclass;
            }
            any.a_int = dis;
            add_menu(tmpwin, NO_GLYPH, MENU_DEFCNT, &any, 0, 0, ATR_NONE,
                     obj_typename(dis), MENU_UNSELECTED);
        }
    }
    if (ct == 0) {
        You("haven't discovered anything yet...");
    } else if (mn == 0) {
        pline("None of your discoveries can be assigned names...");
    } else {
        end_menu(tmpwin, "Pick an object type to name");
        dis = STRANGE_OBJECT;
        sl = select_menu(tmpwin, PICK_ONE, &selected);
        if (sl > 0) {
            dis = selected[0].item.a_int;
            free(selected);
        }
        if (dis != STRANGE_OBJECT) {
            struct obj odummy;

            odummy = zeroobj;
            odummy.otyp = dis;
            odummy.oclass = objects[dis].oc_class;
            odummy.quan = 1L;
            odummy.known = !objects[dis].oc_uses_known;
            odummy.dknown = 1;
            docall(&odummy);
        }
    }
    destroy_nhwindow(tmpwin);
    return;
}

/*o_init.c*/
