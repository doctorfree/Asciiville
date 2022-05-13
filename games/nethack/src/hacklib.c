/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include <stdarg.h>

/* Assorted 'small' utility routines.  They're virtually independent of
   NetHack, except that rounddiv may call panic(). */

#ifdef LINT
# define Static     /* pacify lint */
#else
# define Static static
#endif

static boolean FDECL(pmatch_internal, (const char *, const char *,
                                       BOOLEAN_P, const char *));

/* is 'c' a digit? */
boolean
digit(c)
char c;
{
    return((boolean)('0' <= c && c <= '9'));
}

/* is 'c' a letter?  note: '@' classed as letter */
boolean
letter(c)
char c;
{
    return((boolean)(('@' <= c && c <= 'Z') || ('a' <= c && c <= 'z')));
}

/* force 'c' into uppercase */
char
highc(c)
char c;
{
    return((char)(('a' <= c && c <= 'z') ? (c & ~040) : c));
}

/* force 'c' into lowercase */
char
lowc(c)
char c;
{
    return((char)(('A' <= c && c <= 'Z') ? (c | 040) : c));
}

/* convert a string into all lowercase */
char *
lcase(s)
char *s;
{
    register char *p;

    for (p = s; *p; p++)
        if ('A' <= *p && *p <= 'Z') *p |= 040;
    return s;
}

/* convert a string into all uppercase */
char *
ucase(s)
char *s;
{
    register char *p;

    for (p = s; *p; p++)
        if ('a' <= *p && *p <= 'z')
            *p &= ~040;
    return s;
}

/* convert first character of a string to uppercase */
char *
upstart(s)
char *s;
{
    if (s) *s = highc(*s);
    return s;
}

/* remove excess whitespace from a string buffer (in place) */
char *
mungspaces(bp)
char *bp;
{
    register char c, *p, *p2;
    boolean was_space = TRUE;

    for (p = p2 = bp; (c = *p) != '\0'; p++) {
        if (c == '\n') {
            break; /* treat newline the same as end-of-string */
        }
        if (c == '\t') {
            c = ' ';
        }
        if (c != ' ' || !was_space) {
            *p2++ = c;
        }
        was_space = (c == ' ');
    }
    if (was_space && p2 > bp) {
        p2--;
    }
    *p2 = '\0';
    return bp;
}

/* skip leading whitespace; remove trailing whitespace, in place */
char *
trimspaces(txt)
char *txt;
{
    char *end;

    /* leading whitespace will remain in the buffer */
    while (*txt == ' ' || *txt == '\t')
        txt++;
    end = eos(txt);
    while (--end >= txt && (*end == ' ' || *end == '\t'))
        *end = '\0';

    return txt;
}

/* remove \n from end of line; remove \r too if one is there */
char *
strip_newline(str)
char *str;
{
    char *p = rindex(str, '\n');

    if (p) {
        if (p > str && *(p - 1) == '\r')
            --p;
        *p = '\0';
    }
    return str;
}

/* return the end of a string (pointing at '\0') */
char *
eos(s)
register char *s;
{
    while (*s) s++; /* s += strlen(s); */
    return s;
}

/* determine whether 'str' ends in 'chkstr' */
boolean
str_end_is(str, chkstr)
const char *str, *chkstr;
{
    int clen = (int) strlen(chkstr);

    if ((int) strlen(str) >= clen)
        return (boolean) (!strncmp(eos((char *) str) - clen, chkstr, clen));
    return FALSE;
}

/* return the max line length from buffer comprising of newline-separated strings */
int
str_lines_maxlen(str)
const char *str;
{
    const char *s1, *s2;
    int len, max_len = 0;

    s1 = str;
    while (s1 && *s1) {
        s2 = index(s1, '\n');
        if (s2) {
            len = (int) (s2 - s1);
            s1 = s2 + 1;
        } else {
            len = (int) strlen(s1);
            s1 = (char *) 0;
        }
        if (len > max_len)
            max_len = len;
    }

    return max_len;
}

/* append a character to a string (in place): strcat(s, {c,'\0'}); */
char *
strkitten(s, c)
char *s;
char c;
{
    char *p = eos(s);

    *p++ = c;
    *p = '\0';
    return s;
}

/* truncating string copy */
void
copynchars(dst, src, n)
char *dst;
const char *src;
int n;
{
    /* copies at most n characters, stopping sooner if terminator reached;
       treats newline as input terminator; unlike strncpy, always supplies
       '\0' terminator so dst must be able to hold at least n+1 characters */
    while (n > 0 && *src != '\0' && *src != '\n') {
        *dst++ = *src++;
        --n;
    }
    *dst = '\0';
}

/* convert char nc into oc's case; mostly used by strcasecpy */
char
chrcasecpy(oc, nc)
int oc, nc;
{
#if 0 /* this will be necessary if we switch to <ctype.h> */
    oc = (int) (unsigned char) oc;
    nc = (int) (unsigned char) nc;
#endif
    if ('a' <= oc && oc <= 'z') {
        /* old char is lower case; if new char is upper case, downcase it */
        if ('A' <= nc && nc <= 'Z')
            nc += 'a' - 'A'; /* lowc(nc) */
    } else if ('A' <= oc && oc <= 'Z') {
        /* old char is upper case; if new char is lower case, upcase it */
        if ('a' <= nc && nc <= 'z')
            nc += 'A' - 'a'; /* highc(nc) */
    }
    return (char) nc;
}

/* overwrite string, preserving old chars' case;
   for case-insensitive editions of makeplural() and makesingular();
   src might be shorter, same length, or longer than dst */
char *
strcasecpy(dst, src)
char *dst;
const char *src;
{
    char *result = dst;
    int ic, oc, dst_exhausted = 0;

    /* while dst has characters, replace each one with corresponding
       character from src, converting case in the process if they differ;
       once dst runs out, propagate the case of its last character to any
       remaining src; if dst starts empty, it must be a pointer to the
       tail of some other string because we examine the char at dst[-1] */
    while ((ic = (int) *src++) != '\0') {
        if (!dst_exhausted && !*dst)
            dst_exhausted = 1;
        oc = (int) *(dst - dst_exhausted);
        *dst++ = chrcasecpy(oc, ic);
    }
    *dst = '\0';
    return result;
}

/* replace (in place) characters below space in the string */
void
sanitizestr(s)
char *s;
{
    if (!s) return;
    while (*s) {
        if (*s <= ' ') *s = ' ';
        s++;
    }
}


/* return a name converted to possessive */
char *
s_suffix(s)
const char *s;
{
    Static char buf[BUFSZ];

    Strcpy(buf, s);
    if (!strcmpi(buf, "it")) /* it -> its */
        Strcat(buf, "s");
    else if (!strcmpi(buf, "you")) /* you -> your */
        Strcat(buf, "r");
    else if (*(eos(buf) - 1) == 's') /* Xs -> Xs' */
        Strcat(buf, "'");
    else /* X -> X's */
        Strcat(buf, "'s");
    return buf;
}

/* construct a gerund (a verb formed by appending "ing" to a noun) */
char *
ing_suffix(s)
const char *s;
{
    static const char vowel[] = "aeiouwy";
    static char buf[BUFSZ];
    char onoff[10];
    char *p;

    Strcpy(buf, s);
    p = eos(buf);
    onoff[0] = *p = *(p + 1) = '\0';
    if ((p >= &buf[3] && !strcmpi(p - 3, " on"))
        || (p >= &buf[4] && !strcmpi(p - 4, " off"))
        || (p >= &buf[5] && !strcmpi(p - 5, " with"))) {
        p = rindex(buf, ' ');
        Strcpy(onoff, p);
        *p = '\0';
    }
    if (p >= &buf[3] && !index(vowel, *(p - 1))
        && index(vowel, *(p - 2)) && !index(vowel, *(p - 3))) {
        /* tip -> tipp + ing */
        *p = *(p - 1);
        *(p + 1) = '\0';
    } else if (p >= &buf[2] && !strcmpi(p - 2, "ie")) { /* vie -> vy + ing */
        *(p - 2) = 'y';
        *(p - 1) = '\0';
    } else if (p >= &buf[1] && *(p - 1) == 'e') /* grease -> greas + ing */
        *(p - 1) = '\0';
    Strcat(buf, "ing");
    if (onoff[0])
        Strcat(buf, onoff);
    return buf;
}
/* trivial text encryption routine (see makedefs) */
char *
xcrypt(str, buf)
const char *str;
char *buf;
{
    register const char *p;
    register char *q;
    register int bitmask;

    for (bitmask = 1, p = str, q = buf; *p; q++) {
        *q = *p++;
        if (*q & (32|64)) *q ^= bitmask;
        if ((bitmask <<= 1) >= 32) bitmask = 1;
    }
    *q = '\0';
    return buf;
}

/* is a string entirely whitespace? */
boolean
onlyspace(s)
const char *s;
{
    for (; *s; s++)
        if (*s != ' ' && *s != '\t') return FALSE;
    return TRUE;
}

/* expand tabs into proper number of spaces */
char *
tabexpand(sbuf)
char *sbuf;
{
    char buf[BUFSZ];
    register char *bp, *s = sbuf;
    register int idx;

    if (!*s) return sbuf;

    /* warning: no bounds checking performed */
    for (bp = buf, idx = 0; *s; s++)
        if (*s == '\t') {
            do *bp++ = ' '; while (++idx % 8);
        } else {
            *bp++ = *s;
            idx++;
        }
    *bp = 0;
    return strcpy(sbuf, buf);
}

#define VISCTRL_NBUF 5
/* make a displayable string from a character */
char *
visctrl(c)
char c;
{
    Static char visctrl_bufs[VISCTRL_NBUF][5];
    static int nbuf = 0;
    register int i = 0;
    char *ccc = visctrl_bufs[nbuf];
    nbuf = (nbuf + 1) % VISCTRL_NBUF;

    if ((uchar) c & 0200) {
        ccc[i++] = 'M';
        ccc[i++] = '-';
    }
    c &= 0177;
    if (c < 040) {
        ccc[i++] = '^';
        ccc[i++] = c | 0100; /* letter */
    } else if (c == 0177) {
        ccc[i++] = '^';
        ccc[i++] = c & ~0100; /* '?' */
    } else {
        ccc[i++] = c; /* printable character */
    }
    ccc[i] = '\0';
    return ccc;
}

/* strip all the chars in stuff_to_strip from orig */
/* caller is responsible for ensuring that bp is a
   valid pointer to a BUFSZ buffer */
char *
stripchars(bp, stuff_to_strip, orig)
char *bp;
const char *stuff_to_strip, *orig;
{
    int i = 0;
    char *s = bp;

    if (s) {
        while (*orig && i < (BUFSZ - 1)) {
            if (!index(stuff_to_strip, *orig)) {
                *s++ = *orig;
                i++;
            }
            orig++;
        }
        *s = '\0';
    } else
        impossible("no output buf in stripchars");
    return bp;
}

/* remove digits from string */
char *
stripdigits(s)
char *s;
{
    char *s1, *s2;

    for (s1 = s2 = s; *s1; s1++)
        if (*s1 < '0' || *s1 > '9')
            *s2++ = *s1;
    *s2 = '\0';

    return s;
}

/* substitute a word or phrase in a string (in place) */
/* caller is responsible for ensuring that bp points to big enough buffer */
char *
strsubst(bp, orig, replacement)
char *bp;
const char *orig, *replacement;
{
    char *found, buf[BUFSZ];

    if (bp) {
        /* [this could be replaced by strNsubst(bp, orig, replacement, 1)] */
        found = strstr(bp, orig);
        if (found) {
            Strcpy(buf, found + strlen(orig));
            Strcpy(found, replacement);
            Strcat(bp, buf);
        }
    }
    return bp;
}

/* substitute the Nth occurrence of a substring within a string (in place);
   if N is 0, substitute all occurrences; returns the number of subsitutions;
   maximum output length is BUFSZ (BUFSZ-1 chars + terminating '\0') */
int
strNsubst(inoutbuf, orig, replacement, n)
char *inoutbuf; /* current string, and result buffer */
const char *orig, /* old substring; if "" then insert in front of Nth char */
           *replacement; /* new substring; if "" then delete old substring */
int n; /* which occurrence to replace; 0 => all */
{
    char *bp, *op, workbuf[BUFSZ];
    const char *rp;
    unsigned len = (unsigned) strlen(orig);
    int ocount = 0, /* number of times 'orig' has been matched */
        rcount = 0; /* number of subsitutions made */

    for (bp = inoutbuf, op = workbuf; *bp && op < &workbuf[BUFSZ - 1]; ) {
        if ((!len || !strncmp(bp, orig, len)) && (++ocount == n || n == 0)) {
            /* Nth match found */
            for (rp = replacement; *rp && op < &workbuf[BUFSZ - 1]; )
                *op++ = *rp++;
            ++rcount;
            if (len) {
                bp += len; /* skip 'orig' */
                continue;
            }
        }
        /* no match (or len==0) so retain current character */
        *op++ = *bp++;
    }
    if (!len && n == ocount + 1) {
        /* special case: orig=="" (!len) and n==strlen(inoutbuf)+1,
           insert in front of terminator (in other words, append);
           [when orig=="", ocount will have been incremented once for
           each input char] */
        for (rp = replacement; *rp && op < &workbuf[BUFSZ - 1]; )
            *op++ = *rp++;
        ++rcount;
    }
    if (rcount) {
        *op = '\0';
        Strcpy(inoutbuf, workbuf);
    }
    return rcount;
}

/* return the ordinal suffix of a number */
const char *
ordin(n)
int n;              /* note: should be non-negative */
{
    register int dd = n % 10;

    return (dd == 0 || dd > 3 || (n % 100) / 10 == 1) ? "th" :
           (dd == 1) ? "st" : (dd == 2) ? "nd" : "rd";
}

/* make a signed digit string from a number */
char *
sitoa(n)
int n;
{
    Static char buf[13];

    Sprintf(buf, (n < 0) ? "%d" : "+%d", n);
    return buf;
}

/* return the sign of a number: -1, 0, or 1 */
int
sgn(n)
int n;
{
    return (n < 0) ? -1 : (n != 0);
}

/* calculate x/y, rounding as appropriate */
int
rounddiv(x, y)
long x;
int y;
{
    int r, m;
    int divsgn = 1;

    if (y == 0)
        panic("division by zero in rounddiv");
    else if (y < 0) {
        divsgn = -divsgn;  y = -y;
    }
    if (x < 0) {
        divsgn = -divsgn;  x = -x;
    }
    r = x / y;
    m = x % y;
    if (2*m >= y) r++;

    return divsgn * r;
}

/* distance between two points, in moves */
int
distmin(x0, y0, x1, y1) /* distance between two points, in moves */
int x0, y0, x1, y1;
{
    register int dx = x0 - x1, dy = y0 - y1;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    /*  The minimum number of moves to get from (x0,y0) to (x1,y1) is the
       :  larger of the [absolute value of the] two deltas.
     */
    return (dx < dy) ? dy : dx;
}

/* square of euclidean distance between pair of pts */
int
dist2(x0, y0, x1, y1)
int x0, y0, x1, y1;
{
    register int dx = x0 - x1, dy = y0 - y1;
    return dx * dx + dy * dy;
}

/* integer square root function without using floating point */
int
isqrt(val)
int val;
{
    int rt = 0;
    int odd = 1;
    /*
     * This could be replaced by a faster algorithm, but has not been because:
     * + the simple algorithm is easy to read;
     * + this algorithm does not require 64-bit support;
     * + in current usage, the values passed to isqrt() are not really that
     *   large, so the performance difference is negligible;
     * + isqrt() is used in only few places, which are not bottle-necks.
     */
    while(val >= odd) {
        val = val-odd;
        odd = odd+2;
        rt = rt + 1;
    }
    return rt;
}

/** integer log2 function without using floating point.
 *
 * @returns rounded down log2 value, or -1 if value <= 0
 */
int
ilog2(value)
int value;
{
    if (value <= 0) {
        return -1;
    }

    int ret = 0;
    while (value >>= 1) {
        ret++;
    }

    return ret;
}

/* are two points lined up (on a straight line)? */
boolean
online2(x0, y0, x1, y1)
int x0, y0, x1, y1;
{
    int dx = x0 - x1, dy = y0 - y1;
    /*  If either delta is zero then they're on an orthogonal line,
     *  else if the deltas are equal (signs ignored) they're on a diagonal.
     */
    return((boolean)(!dy || !dx || (dy == dx) || (dy + dx == 0)));  /* (dy == -dx) */
}

/* guts of pmatch(), pmatchi(), and pmatchz();
   match a string against a pattern */
static boolean
pmatch_internal(patrn, strng, ci, sk)
const char *patrn, *strng;
boolean ci;     /* True => case-insensitive, False => case-sensitive */
const char *sk; /* set of characters to skip */
{
    char s, p;
    /*
     *  Simple pattern matcher:  '*' matches 0 or more characters, '?' matches
     *  any single character.  Returns TRUE if 'strng' matches 'patrn'.
     */
pmatch_top:
    if (!sk) {
        s = *strng++;
        p = *patrn++; /* get next chars and pre-advance */
    } else {
        /* fuzzy match variant of pmatch; particular characters are ignored */
        do {
            s = *strng++;
        } while (index(sk, s));
        do {
            p = *patrn++;
        } while (index(sk, p));
    }
    if (!p)                           /* end of pattern */
        return (boolean) (s == '\0'); /* matches iff end of string too */
    else if (p == '*')                /* wildcard reached */
        return (boolean) ((!*patrn
                           || pmatch_internal(patrn, strng - 1, ci, sk))
                          ? TRUE
                          : s ? pmatch_internal(patrn - 1, strng, ci, sk)
                              : FALSE);
    else if ((ci ? lowc(p) != lowc(s) : p != s) /* check single character */
             && (p != '?' || !s))               /* & single-char wildcard */
        return FALSE;                           /* doesn't match */
    else                 /* return pmatch_internal(patrn, strng, ci, sk); */
        goto pmatch_top; /* optimize tail recursion */
}

/* case-sensitive wildcard match */
boolean
pmatch(patrn, strng)
const char *patrn, *strng;
{
    return pmatch_internal(patrn, strng, FALSE, (const char *) 0);
}

/* case-insensitive wildcard match */
boolean
pmatchi(patrn, strng)
const char *patrn, *strng;
{
    return pmatch_internal(patrn, strng, TRUE, (const char *) 0);
}

/* case-insensitive wildcard fuzzymatch */
boolean
pmatchz(patrn, strng)
const char *patrn, *strng;
{
    /* ignore spaces, tabs (just in case), dashes, and underscores */
    static const char fuzzychars[] = " \t-_";

    return pmatch_internal(patrn, strng, TRUE, fuzzychars);
}

#ifndef STRNCMPI
/* case insensitive counted string comparison */
int
strncmpi(s1, s2, n) /*{ aka strncasecmp }*/
register const char *s1, *s2;
register int n;     /*(should probably be size_t, which is usually unsigned)*/
{
    register char t1, t2;

    while (n--) {
        if (!*s2) return (*s1 != 0); /* s1 >= s2 */
        else if (!*s1) return -1; /* s1  < s2 */
        t1 = lowc(*s1++);
        t2 = lowc(*s2++);
        if (t1 != t2) return (t1 > t2) ? 1 : -1;
    }
    return 0;               /* s1 == s2 */
}
#endif  /* STRNCMPI */

#ifndef STRSTRI

/* case insensitive substring search */
char *
strstri(str, sub)   /* case insensitive substring search */
const char *str;
const char *sub;
{
    register const char *s1, *s2;
    register int i, k;
# define TABSIZ 0x20                 /* 0x40 would be case-sensitive */
    char tstr[TABSIZ], tsub[TABSIZ]; /* nibble count tables */
# if 0
    assert( (TABSIZ & ~(TABSIZ-1)) == TABSIZ ); /* must be exact power of 2 */
    assert( &lowc != 0 );                       /* can't be unsafe macro */
# endif

    /* special case: empty substring */
    if (!*sub) return (char *) str;

    /* do some useful work while determining relative lengths */
    for (i = 0; i < TABSIZ; i++) tstr[i] = tsub[i] = 0;     /* init */
    for (k = 0, s1 = str; *s1; k++) tstr[*s1++ & (TABSIZ-1)]++;
    for (   s2 = sub; *s2; --k) tsub[*s2++ & (TABSIZ-1)]++;

    /* evaluate the info we've collected */
    if (k < 0) return (char *) 0;   /* sub longer than str, so can't match */
    for (i = 0; i < TABSIZ; i++)    /* does sub have more 'x's than str? */
        if (tsub[i] > tstr[i]) return (char *) 0; /* match not possible */

    /* now actually compare the substring repeatedly to parts of the string */
    for (i = 0; i <= k; i++) {
        s1 = &str[i];
        s2 = sub;
        while (lowc(*s1++) == lowc(*s2++))
            if (!*s2) return (char *) &str[i];  /* full match */
    }
    return (char *) 0;  /* not found */
}
#endif  /* STRSTRI */

/* compare two strings for equality, ignoring the presence of specified
   characters (typically whitespace) and possibly ignoring case */
boolean
fuzzymatch(s1, s2, ignore_chars, caseblind)
const char *s1, *s2;
const char *ignore_chars;
boolean caseblind;
{
    register char c1, c2;

    do {
        while ((c1 = *s1++) != '\0' && index(ignore_chars, c1) != 0) continue;
        while ((c2 = *s2++) != '\0' && index(ignore_chars, c2) != 0) continue;
        if (!c1 || !c2) break; /* stop when end of either string is reached */

        if (caseblind) {
            c1 = lowc(c1);
            c2 = lowc(c2);
        }
    } while (c1 == c2);

    /* match occurs only when the end of both strings has been reached */
    return (boolean)(!c1 && !c2);
}

/*
 * Time routines
 *
 * The time is used for:
 *  - year on tombstone and yyyymmdd in record file
 *  - phase of the moon (various monsters react to NEW_MOON or FULL_MOON)
 *  - night and midnight (the undead are dangerous at midnight)
 *  - determination of what files are "very old"
 */

#if defined(AMIGA) && !defined(AZTEC_C) && !defined(__SASC_60) && !defined(_DCC) && !defined(__GNUC__)
extern struct tm *FDECL(localtime, (time_t *));
#endif
static struct tm *NDECL(getlt);

static int
get_int_from_dev_random()
{
    unsigned int random_seed=0;
#ifdef DEV_RANDOM
    FILE *fptr = NULL;

    fptr = fopen(DEV_RANDOM, "r");
    if (fptr) {
        fread(&random_seed, sizeof(int), 1, fptr);
        fclose(fptr);
    }
#endif
    return random_seed;
}

/** Initialize random number generator. */
void
init_random(unsigned int seed)
{
    unsigned int random_seed=get_int_from_dev_random();

    if (seed == 0) {
        seed = (unsigned int) (time((time_t *)0)) + random_seed;
    } else {
        /* user has set seed in config */
        level_info[0].flags |= PRE_SEEDED;
    }

    /* save seed in the dummy level 0 */
    level_info[0].seed = seed;

    set_random_state(seed);
}

void
reseed_random()
{
    unsigned int random_seed=0;
    random_seed = (unsigned int)(time((time_t *)0)) + get_int_from_dev_random();
    set_random_state(random_seed);
}

time_t
getnow()
{
    time_t datetime = 0;

    (void) time(&datetime);
    return datetime;
}

static struct tm *
getlt()
{
    time_t date = current_epoch();

#if (defined(ULTRIX) && !(defined(ULTRIX_PROTO) || defined(NHSTDC))) || (defined(BSD) && !defined(POSIX_TYPES))
    return(localtime((long *)(&date)));
#else
    return(localtime(&date));
#endif
}

int
getyear()
{
    return(1900 + getlt()->tm_year);
}

/** Returns current month (1-12, 1 = January) */
int
getmonth()
{
    return(1 + getlt()->tm_mon);
}

/** Returns current day of month (1-31) */
int
getmday()
{
    return(getlt()->tm_mday);
}


long
yyyymmdd(date)
time_t date;
{
    long datenum;
    struct tm *lt;

    if (date == 0) {
        lt = getlt();
    } else {
        lt = localtime(&date);
    }

    /* just in case somebody's localtime supplies (year % 100)
       rather than the expected (year - 1900) */
    if (lt->tm_year < 70) {
        datenum = (long)lt->tm_year + 2000L;
    } else {
        datenum = (long)lt->tm_year + 1900L;
    }
    /* yyyy --> yyyymm */
    datenum = datenum * 100L + (long)(lt->tm_mon + 1);
    /* yyyymm --> yyyymmdd */
    datenum = datenum * 100L + (long)lt->tm_mday;
    return datenum;
}

long
hhmmss(date)
time_t date;
{
    long timenum;
    struct tm *lt;

    if (date == 0) {
        lt = getlt();
    } else {
        lt = localtime(&date);
    }

    timenum = lt->tm_hour * 10000L + lt->tm_min * 100L + lt->tm_sec;
    return timenum;
}

char *
yyyymmddhhmmss(date)
time_t date;
{
    long datenum;
    static char datestr[BUFSZ];
    struct tm *lt;

    if (date == 0) {
        lt = getlt();
    } else {
        lt = localtime(&date);
    }

    /* just in case somebody's localtime supplies (year % 100)
       rather than the expected (year - 1900) */
    if (lt->tm_year < 70) {
        datenum = (long) lt->tm_year + 2000L;
    } else {
        datenum = (long) lt->tm_year + 1900L;
    }
    Sprintf(datestr, "%04ld%02d%02d%02d%02d%02d", datenum, lt->tm_mon + 1,
            lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
    debug_pline("yyyymmddhhmmss() produced date string %s", datestr);
    return datestr;
}

time_t
time_from_yyyymmddhhmmss(buf)
char *buf;
{
    int k;
    time_t timeresult = (time_t) 0;
    struct tm t, *lt;
    char *d, *p, y[5], mo[3], md[3], h[3], mi[3], s[3];

    if (buf && strlen(buf) == 14) {
        d = buf;
        p = y; /* year */
        for (k = 0; k < 4; ++k)
            *p++ = *d++;
        *p = '\0';
        p = mo; /* month */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = md; /* day */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = h; /* hour */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = mi; /* minutes */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = s; /* seconds */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        lt = getlt();
        if (lt) {
            t = *lt;
            t.tm_year = atoi(y) - 1900;
            t.tm_mon = atoi(mo) - 1;
            t.tm_mday = atoi(md);
            t.tm_hour = atoi(h);
            t.tm_min = atoi(mi);
            t.tm_sec = atoi(s);
            timeresult = mktime(&t);
        }
        if ((int) timeresult == -1) {
            debug_pline("time_from_yyyymmddhhmmss(%s) would have returned -1",
                        buf ? buf : "");
        } else {
            return timeresult;
        }
    }
    return (time_t) 0;
}

/*
 * moon period = 29.53058 days ~= 30, year = 365.2422 days
 * days moon phase advances on first day of year compared to preceding year
 *  = 365.2422 - 12*29.53058 ~= 11
 * years in Metonic cycle (time until same phases fall on the same days of
 *  the month) = 18.6 ~= 19
 * moon phase on first day of year (epact) ~= (11*(year%19) + 29) % 30
 *  (29 as initial condition)
 * current phase in days = first day phase + days elapsed in year
 * 6 moons ~= 177 days
 * 177 ~= 8 reported phases * 22
 * + 11/22 for rounding
 */
int
phase_of_the_moon()     /* 0-7, with 0: new, 4: full */
{
    register struct tm *lt = getlt();
    register int epact, diy, goldn;

    diy = lt->tm_yday;
    goldn = (lt->tm_year % 19) + 1;
    epact = (11 * goldn + 18) % 30;
    if ((epact == 25 && goldn > 11) || epact == 24)
        epact++;

    return( (((((diy + epact) * 6) + 11) % 177) / 22) & 7 );
}

boolean
friday_13th()
{
    register struct tm *lt = getlt();

    return((boolean)(lt->tm_wday == 5 /* friday */ && lt->tm_mday == 13));
}

int
night()
{
    register int hour = getlt()->tm_hour;

    return(hour < 6 || hour > 21);
}

int
midnight()
{
    return(getlt()->tm_hour == 0);
}

boolean
towelday()
{
    return (boolean)((getmday()==25) && (getmonth()==5));
}

boolean
piday()
{
    return (boolean)(
        ((getmonth()==3) && (getmday()==14)) || /* Pi Day */
        ((getmday()==22) && (getmonth()==7))); /* Pi Approximation Day */

}

boolean
aprilfoolsday()
{
    return (boolean)((getmonth()==4) && (getmday()==1));
}

boolean
pirateday()
{
    return (boolean)((getmday()==19) && (getmonth()==9));
}

boolean
discordian_holiday()
{
    return (boolean)((getmday()==23) && (getmonth()==5));
}


static char buf_fmt_time[BUFSZ];
/** Returns a date formatted by strftime.
 * Returns current time if time is 0. */
char *
get_formatted_time(time, fmt)
time_t time;
const char *fmt;
{
    strftime(buf_fmt_time, BUFSZ, fmt,
             (time == 0) ? getlt() : localtime(&time));
    return buf_fmt_time;
}

/** Returns a iso-8601 formatted date (e.g. 2010-03-19T08:46:23+0100). */
char *
iso8601(date)
time_t date;
{
    return get_formatted_time(date, "%Y-%m-%dT%H:%M:%S%z");
}

static char buf_fmt_duration[BUFSZ];
/** Returns a iso-8601 formatted duration (e.g. PThh:mm:ss). */
char *
iso8601_duration(seconds)
long seconds;
{
    /* currently no days, months and years, as the conversion
     * is non-trivial */
    long minutes = seconds / 60;
    long hours = minutes / 60;
    long days = hours / 24;

    if (days > 0) {
        /* PddDThh:mm:ss */
        sprintf(buf_fmt_duration, "P%02ldDT%02ld:%02ld:%02ld",
                days, hours % 24, minutes % 60, seconds % 60);
    } else {
        /* PThh:mm:ss */
        sprintf(buf_fmt_duration, "PT%02ld:%02ld:%02ld",
                hours, minutes % 60, seconds % 60);
    }
    return buf_fmt_duration;
}

/** Returns a human readable formatted duration (e.g. 2h:03m:ss). */
char *
format_duration(seconds)
long seconds;
{
    long minutes = seconds / 60;
    long hours = minutes / 60;
    long days = hours / 24;

    seconds = seconds % 60;
    minutes = minutes % 60;
    hours = hours % 24;

    if (days > 0) {
        sprintf(buf_fmt_duration, "%ldd:%2.2ldh:%2.2ldm:%2.2lds", days, hours, minutes, seconds);
    } else if (hours > 0) {
        sprintf(buf_fmt_duration, "%ldh:%2.2ldm:%2.2lds", hours, minutes, seconds);
    } else {
        sprintf(buf_fmt_duration, "%ldm:%2.2lds", minutes, seconds);
    }
    return buf_fmt_duration;
}

/** Returns epoch time. */
time_t
current_epoch()
{
    if (iflags.debug_fuzzer) {
        /* 100_000 moves makes the game run from 2020-01-01 to 2020-04-25 */
        return (time_t)(moves*10 + 1577836800);
    }

    time_t date;

#if defined(BSD) && !defined(POSIX_TYPES)
    (void) time((long *)(&date));
#else
    (void) time(&date);
#endif
    return date;
}

/* strbuf_init() initializes strbuf state for use */
void
strbuf_init(strbuf)
strbuf_t *strbuf;
{
    strbuf->str = NULL;
    strbuf->len = 0;
}

/* strbuf_append() appends given str to strbuf->str */
void
strbuf_append(strbuf, str)
strbuf_t *strbuf;
const char *str;
{
    int len = (int) strlen(str) + 1;

    strbuf_reserve(strbuf,
                   len + (strbuf->str ? (int) strlen(strbuf->str) : 0));
    Strcat(strbuf->str, str);
}

/* strbuf_reserve() ensure strbuf->str has storage for len characters */
void
strbuf_reserve(strbuf, len)
strbuf_t *strbuf;
int len;
{
    if (strbuf->str == NULL) {
        strbuf->str = strbuf->buf;
        strbuf->str[0] = '\0';
        strbuf->len = (int) sizeof strbuf->buf;
    }

    if (len > strbuf->len) {
        char *oldbuf = strbuf->str;

        strbuf->len = len + (int) sizeof strbuf->buf;
        strbuf->str = (char *) alloc(strbuf->len);
        Strcpy(strbuf->str, oldbuf);
        if (oldbuf != strbuf->buf)
            free((genericptr_t) oldbuf);
    }
}

/* strbuf_empty() frees allocated memory and set strbuf to initial state */
void
strbuf_empty(strbuf)
strbuf_t *strbuf;
{
    if (strbuf->str != NULL && strbuf->str != strbuf->buf)
        free((genericptr_t) strbuf->str);
    strbuf_init(strbuf);
}

/* strbuf_nl_to_crlf() converts all occurences of \n to \r\n */
void
strbuf_nl_to_crlf(strbuf)
strbuf_t *strbuf;
{
    if (strbuf->str) {
        int len = (int) strlen(strbuf->str);
        int count = 0;
        char *cp = strbuf->str;

        while (*cp)
            if (*cp++ == '\n')
                count++;
        if (count) {
            strbuf_reserve(strbuf, len + count + 1);
            for (cp = strbuf->str + len + count; count; --cp)
                if ((*cp = cp[-count]) == '\n') {
                    *--cp = '\r';
                    --count;
                }
        }
    }
}

/** swapbits(val, bita, bitb) swaps bit a with bit b in val */
int
swapbits(int val, int bita, int bitb)
{
    int tmp = ((val >> bita) & 1) ^ ((val >> bitb) & 1);

    return (val ^ ((tmp << bita) | (tmp << bitb)));
}

/** Strip [] from strings like "a [cursed] rusty iron wand" so
 * menucolors regexp can apply.
 * Completely hacky. */
void
strip_brackets(char *str)
{
    /* from http://stackoverflow.com/questions/4161822 */
    char *src, *dest;

    src = dest = str; /* both pointers point to the first char of input */
    while (*src != '\0') {
        /* exit loop when null terminator reached */
        if (*src != '[' && *src != ']') {
            /* if source is not a [] char */
            *dest = *src; /* copy the char at source to destination */
            dest++;       /* increment destination pointer */
        }
        src++; /* increment source pointer */
    }
    *dest = '\0'; /* terminate string with null terminator */
}

/*
 * Wrap snprintf for use in the main code.
 *
 * Wrap reasons:
 *   1. If there are any platform issues, we have one spot to fix them -
 *      snprintf is a routine with a troubling history of bad implementations.
 *   2. Add combersome error checking in one spot.  Problems with text wrangling
 *      do not have to be fatal.
 *   3. Gcc 9+ will issue a warning unless the return value is used.
 *      Annoyingly, explicitly casting to void does not remove the error.
 *      So, use the result - see reason #2.
 */
void
nh_snprintf(const char *func, int line, char *str, size_t size,
            const char *fmt, ...)
{
    va_list ap;
    int n;

    va_start(ap, fmt);
#ifdef NO_VSNPRINTF
    n = vsprintf(str, fmt, ap);
#else
    n = vsnprintf(str, size, fmt, ap);
#endif
    va_end(ap);
    if (n < 0 || (size_t)n >= size) {
        /* is there a problem? */
        impossible("snprintf %s: func %s, file line %d",
                   n < 0 ? "format error"
                         : "overflow",
                   func, line);
        str[size-1] = 0; /* make sure it is nul terminated */
    }
}

/*hacklib.c*/
