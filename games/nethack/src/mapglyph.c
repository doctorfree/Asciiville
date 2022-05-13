/*  SCCS Id: @(#)mapglyph.c 3.4 2003/01/08  */
/* Copyright (c) David Cohrs, 1991                */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#if defined(TTY_GRAPHICS)
#include "wintty.h" /* for prototype of has_color() only */
#endif
#include "color.h"
#define HI_DOMESTIC CLR_WHITE   /* monst.c */

int explcolors[] = {
    CLR_BLACK,  /* dark    */
    CLR_GREEN,  /* noxious */
    CLR_BROWN,  /* muddy   */
    CLR_BLUE,   /* wet     */
    CLR_MAGENTA,    /* magical */
    CLR_ORANGE, /* fiery   */
    CLR_WHITE,  /* frosty  */
};

#if !defined(TTY_GRAPHICS)
#define has_color(n)  TRUE
#endif

#ifdef TEXTCOLOR
#define zap_color(n)  color = iflags.use_color ? zapcolors[n] : NO_COLOR
#define cmap_color(n) color = iflags.use_color ? defsyms[n].color : NO_COLOR
#define obj_color(n)  color = iflags.use_color ? objects[n].oc_color : NO_COLOR
#define mon_color(n)  color = iflags.use_color ? mons[n].mcolor : NO_COLOR
#define invis_color(n) color = NO_COLOR
#define pet_color(n)  color = iflags.use_color ? mons[n].mcolor : NO_COLOR
#define warn_color(n) color = iflags.use_color ? def_warnsyms[n].color : NO_COLOR
#define explode_color(n) color = iflags.use_color ? explcolors[n] : NO_COLOR
#define sokoban_prize_color()  color = iflags.use_color ? CLR_BRIGHT_MAGENTA : NO_COLOR
# if defined(REINCARNATION) && defined(ASCIIGRAPH)
#  define ROGUE_COLOR
# endif

#else   /* no text color */

#define zap_color(n)
#define cmap_color(n)
#define obj_color(n)
#define mon_color(n)
#define invis_color(n)
#define pet_color(c)
#define warn_color(n)
#define explode_color(n)
#define sokoban_prize_color()
#endif

#ifdef ROGUE_COLOR
# if defined(USE_TILES) && defined(MSDOS)
#define HAS_ROGUE_IBM_GRAPHICS (iflags.IBMgraphics && !iflags.grmode && \
                                Is_rogue_level(&u.uz))
# else
#define HAS_ROGUE_IBM_GRAPHICS (iflags.IBMgraphics && Is_rogue_level(&u.uz))
# endif
#endif

/** Returns the correct monster glyph.
 *  Returns a Unicode codepoint in UTF8graphics and an ASCII character otherwise. */
glyph_t
get_monsym(glyph)
int glyph;
{
    if (iflags.UTF8graphics &&
        mons[glyph].unicode_codepoint) {
        /* only return a Unicode codepoint when there is one configured */
        return mons[glyph].unicode_codepoint;
    } else {
        return monsyms[(int)mons[glyph].mlet];
    }
}

/** Returns the correct object glyph.
 *  Returns a Unicode codepoint in UTF8graphics and an ASCII character otherwise. */
static glyph_t
get_objsym(glyph)
int glyph;
{
    if (iflags.UTF8graphics &&
        objects[glyph].unicode_codepoint) {
        /* only return a Unicode codepoint when there is one configured */
        return objects[glyph].unicode_codepoint;
    } else {
        return oc_syms[(int)objects[glyph].oc_class];
    }
}

/*ARGSUSED*/
void
mapglyph(glyph, ochar, ocolor, ospecial, x, y, mgflags)
int glyph, *ocolor, x, y;
glyph_t *ochar;
unsigned *ospecial;
unsigned mgflags UNUSED;
{
    register int offset;
#if defined(TEXTCOLOR) || defined(ROGUE_COLOR)
    int color = NO_COLOR;
#endif
    glyph_t ch;
    unsigned special = 0;

    /* ugly workaround, needs proper structure in add_menu */
    if (glyph < 0) {
        x = abs(glyph) % COLNO;
        y = abs(glyph) / COLNO;
        glyph = glyph_at(x, y);
    }

    /*
     *  Map the glyph back to a character and color.
     *
     *  Warning:  For speed, this makes an assumption on the order of
     *        offsets.  The order is set in display.h.
     */
    if ((offset = (glyph - GLYPH_STATUE_OFF)) >= 0) { /* a statue */
        ch = get_monsym(offset);
        obj_color(STATUE);
        special |= MG_STATUE;
        if (level.objects[x][y] && level.objects[x][y]->nexthere) {
            special |= MG_INVERSE;
        }
    } else if ((offset = (glyph - GLYPH_WARNING_OFF)) >= 0) {  /* a warning flash */
        ch = warnsyms[offset];
# ifdef ROGUE_COLOR
        if (HAS_ROGUE_IBM_GRAPHICS)
            color = NO_COLOR;
        else
# endif
        warn_color(offset);
    } else if ((offset = (glyph - GLYPH_SWALLOW_OFF)) >= 0) {   /* swallow */
        /* see swallow_to_glyph() in display.c */
        ch = showsyms[S_sw_tl + (offset & 0x7)];
#ifdef ROGUE_COLOR
        if (HAS_ROGUE_IBM_GRAPHICS && iflags.use_color)
            color = NO_COLOR;
        else
#endif
        mon_color(offset >> 3);
    } else if ((offset = (glyph - GLYPH_ZAP_OFF)) >= 0) {   /* zap beam */
        /* see zapdir_to_glyph() in display.c */
        ch = showsyms[S_vbeam + (offset & 0x3)];
#ifdef ROGUE_COLOR
        if (HAS_ROGUE_IBM_GRAPHICS && iflags.use_color)
            color = NO_COLOR;
        else
#endif
        zap_color((offset >> 2));
    } else if ((offset = (glyph - GLYPH_EXPLODE_OFF)) >= 0) {   /* explosion */
        ch = showsyms[(offset % MAXEXPCHARS) + S_explode1];
        explode_color(offset / MAXEXPCHARS);
    } else if ((offset = (glyph - GLYPH_CMAP_OFF)) >= 0) {  /* cmap */
        ch = showsyms[offset];
#ifdef ROGUE_COLOR
        if (HAS_ROGUE_IBM_GRAPHICS && iflags.use_color) {
            if (offset >= S_vwall && offset <= S_hcdoor)
                color = CLR_BROWN;
            else if (offset >= S_arrow_trap && offset <= S_polymorph_trap)
                color = CLR_MAGENTA;
            else if (offset == S_corr || offset == S_litcorr)
                color = CLR_GRAY;
            else if (offset >= S_room && offset <= S_water && offset != S_darkroom)
                color = CLR_GREEN;
            else
                color = NO_COLOR;
        } else
#endif
#ifdef TEXTCOLOR
        /* provide a visible difference if normal and lit corridor
         * use the same symbol */
        if (iflags.use_color &&
            offset == S_litcorr && ch == showsyms[S_corr])
            color = CLR_WHITE;
        else if (iflags.use_color &&
                 (offset == S_upstair || offset == S_dnstair) &&
                 (x == sstairs.sx && y == sstairs.sy)) {
            color = CLR_YELLOW;
        } else if (offset == S_altar && iflags.use_color) {
            int amsk = altarmask_at(x, y); /* might be a mimic */

            if (((Is_astralevel(&u.uz) || Is_sanctum(&u.uz))) && (amsk & AM_SHRINE)) {
                /* high altar */
                color = CLR_BRIGHT_MAGENTA;
            } else {
                switch (amsk & AM_MASK) {
                case AM_NONE:
                    color = CLR_RED;
                    break;

                default:
                    if (Amask2align(amsk & AM_MASK) == u.ualign.type) {
                        color = CLR_WHITE;
                    } else {
                        cmap_color(S_altar); /* gray */
                    }
                }
            }
        } else
#endif
        cmap_color(offset);
    } else if ((offset = (glyph - GLYPH_OBJ_OFF)) >= 0) {   /* object */
        if (offset == BOULDER && iflags.bouldersym) ch = iflags.bouldersym;
        else ch = get_objsym(offset);
#ifdef ROGUE_COLOR
        if (HAS_ROGUE_IBM_GRAPHICS && iflags.use_color) {
            switch(objects[offset].oc_class) {
            case COIN_CLASS: color = CLR_YELLOW; break;
            case FOOD_CLASS: color = CLR_RED; break;
            default: color = CLR_BRIGHT_BLUE; break;
            }
        } else
#endif
        obj_color(offset);
        /* use inverse video for multiple items */
        if (offset != BOULDER &&
            level.objects[x][y] &&
            level.objects[x][y]->nexthere) {
            special |= MG_INVERSE;
        }
        if (level.objects[x][y] && Is_sokoprize(level.objects[x][y])) {
            sokoban_prize_color();
        }

    } else if ((offset = (glyph - GLYPH_RIDDEN_OFF)) >= 0) {    /* mon ridden */
        ch = get_monsym(offset);
#ifdef ROGUE_COLOR
        if (HAS_ROGUE_IBM_GRAPHICS)
            /* This currently implies that the hero is here -- monsters */
            /* don't ride (yet...).  Should we set it to yellow like in */
            /* the monster case below?  There is no equivalent in rogue. */
            color = NO_COLOR; /* no need to check iflags.use_color */
        else
#endif
        mon_color(offset);
        special |= MG_RIDDEN;
    } else if ((offset = (glyph - GLYPH_BODY_OFF)) >= 0) {  /* a corpse */
        ch = get_objsym(CORPSE);
#ifdef ROGUE_COLOR
        if (HAS_ROGUE_IBM_GRAPHICS && iflags.use_color)
            color = CLR_RED;
        else
#endif
        mon_color(offset);
        special |= MG_CORPSE;
        /* use inverse video for multiple items */
        if (level.objects[x][y] &&
            level.objects[x][y]->nexthere) {
            special |= MG_INVERSE;
        }
    } else if ((offset = (glyph - GLYPH_DETECT_OFF)) >= 0) {    /* mon detect */
        ch = get_monsym(offset);
#ifdef ROGUE_COLOR
        if (HAS_ROGUE_IBM_GRAPHICS)
            color = NO_COLOR; /* no need to check iflags.use_color */
        else
#endif
        mon_color(offset);
        /* Disabled for now; anyone want to get reverse video to work? */
        /* is_reverse = TRUE; */
        special |= MG_DETECT;
    } else if ((offset = (glyph - GLYPH_INVIS_OFF)) >= 0) { /* invisible */
        ch = DEF_INVISIBLE;
#ifdef ROGUE_COLOR
        if (HAS_ROGUE_IBM_GRAPHICS)
            color = NO_COLOR; /* no need to check iflags.use_color */
        else
#endif
        invis_color(offset);
        special |= MG_INVIS;
    } else if ((offset = (glyph - GLYPH_PET_OFF)) >= 0) {   /* a pet */
        ch = get_monsym(offset);
#ifdef ROGUE_COLOR
        if (HAS_ROGUE_IBM_GRAPHICS)
            color = NO_COLOR; /* no need to check iflags.use_color */
        else
#endif
        pet_color(offset);
        special |= MG_PET;
    } else {                            /* a monster */
        ch = get_monsym(glyph);
#ifdef ROGUE_COLOR
        if (HAS_ROGUE_IBM_GRAPHICS && iflags.use_color) {
            if (x == u.ux && y == u.uy)
                /* actually player should be yellow-on-gray if in a corridor */
                color = CLR_YELLOW;
            else
                color = NO_COLOR;
        } else
#endif
        {
            mon_color(glyph);
            /* special case the hero for `showrace' option */
#ifdef TEXTCOLOR
            if (iflags.use_color && x == u.ux && y == u.uy &&
                iflags.showrace && !Upolyd)
                color = HI_DOMESTIC;
#endif
        }
    }

#ifdef TEXTCOLOR
    /* Turn off color if no color defined, or rogue level w/o PC graphics. */
# ifdef REINCARNATION
#  ifdef ASCIIGRAPH
    if (!has_color(color) || (Is_rogue_level(&u.uz) && !HAS_ROGUE_IBM_GRAPHICS))
#  else
    if (!has_color(color) || Is_rogue_level(&u.uz))
#  endif
# else
    if (!has_color(color))
# endif
        color = NO_COLOR;
#endif

    *ochar = ch;
    *ospecial = special;
#ifdef TEXTCOLOR
    *ocolor = color;
#endif
    return;
}

char *
encglyph(glyph)
int glyph;
{
    static char encbuf[20]; /* 10+1 would suffice */

    Sprintf(encbuf, "\\G%04X%04X", flags.rndencode, glyph);
    return encbuf;
}

char *
decode_mixed(buf, str)
char *buf;
const char *str;
{
    static const char hex[] = "00112233445566778899aAbBcCdDeEfF";
    char *put = buf;

    if (!str) {
        return strcpy(buf, "");
    }

    while (*str) {
        if (*str == '\\') {
            glyph_t ch = 0;
            int rndchk, dcount, so, gv, oc = 0;
            unsigned os = 0;
            const char *dp, *save_str;

            save_str = str++;
            switch (*str) {
            case 'G': /* glyph value \GXXXXNNNN*/
                rndchk = dcount = 0;
                for (++str; *str && ++dcount <= 4; ++str) {
                    if ((dp = index(hex, *str)) != 0) {
                        rndchk = (rndchk * 16) + ((int) (dp - hex) / 2);
                    } else {
                        break;
                    }
                }
                if (rndchk == flags.rndencode) {
                    gv = dcount = 0;
                    for (; *str && ++dcount <= 4; ++str) {
                        if ((dp = index(hex, *str)) != 0) {
                            gv = (gv * 16) + ((int) (dp - hex) / 2);
                        } else {
                            break;
                        }
                    }
                    mapglyph(gv, &ch, &oc, &os, 0, 0, 0);
                    *put++ = ch;
                    /* 'str' is ready for the next loop iteration and '*str'
                       should not be copied at the end of this iteration */
                    continue;
                } else {
                    /* possible forgery - leave it the way it is */
                    str = save_str;
                }
                break;
#if 0
            case 'S': /* symbol offset */
                so = rndchk = dcount = 0;
                for (++str; *str && ++dcount <= 4; ++str) {
                    if ((dp = index(hex, *str)) != 0) {
                        rndchk = (rndchk * 16) + ((int) (dp - hex) / 2);
                    } else {
                        break;
                    }
                }
                if (rndchk == context.rndencode) {
                    dcount = 0;
                    for (; *str && ++dcount <= 2; ++str) {
                        if ((dp = index(hex, *str)) != 0) {
                            so = (so * 16) + ((int) (dp - hex) / 2);
                        } else {
                            break;
                        }
                    }
                }
                *put++ = showsyms[so];
                break;
#endif
            case '\\':
                break;
            case '\0':
                /* String ended with '\\'.  This can happen when someone
                   names an object with a name ending with '\\', drops the
                   named object on the floor nearby and does a look at all
                   nearby objects. */
                /* brh - should we perhaps not allow things to have names
                   that contain '\\' */
                str = save_str;
                break;
            }
        }
        *put++ = *str++;
    }
    *put = '\0';
    return buf;
}

/*
 * This differs from putstr() because the str parameter can
 * contain a sequence of characters representing:
 *        \GXXXXNNNN    a glyph value, encoded by encglyph().
 *
 * For window ports that haven't yet written their own
 * XXX_putmixed() routine, this general one can be used.
 * It replaces the encoded glyph sequence with a single
 * showsyms[] char, then just passes that string onto
 * putstr().
 */

void
genl_putmixed(window, attr, str)
winid window;
int attr;
const char *str;
{
    char buf[BUFSZ];

    /* now send it to the normal putstr */
    putstr(window, attr, decode_mixed(buf, str));
}

/*mapglyph.c*/
