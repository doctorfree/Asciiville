/*  SCCS Id: @(#)rnd.c  3.4 1996/02/07  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#include "isaac.h"

isaac_ctx default_rng;
isaac_ctx mon_rng;
int use_mon_rng=0;
int
RND(int x)
{

    if (use_mon_rng) {
        return (isaac_next_uint32(&mon_rng) % x);
    } else {
        return (isaac_next_uint32(&default_rng) % x);
    }
}

void
set_random_state(unsigned int x)
{
    unsigned char seed[8];
    int i;
    for (i=0; i<8; i++) {
        seed[i]= (unsigned char)(x & 0xFF);
        x >>= 8;
    }

    isaac_init(&mon_rng, seed, 8);
    isaac_init(&default_rng, seed, 8);
}

int
rn2(x)      /**< 0 <= rn2(x) < x */
register int x;
{
    if (x <= 0) {
        warning("rn2(%d) attempted", x);
        return(0);
    }
    x = RND(x);
    return(x);
}

int
rnl(x)      /**< 0 <= rnl(x) < x; sometimes subtracting Luck */
register int x; /**< good luck approaches 0, bad luck approaches (x-1) */
{
    register int i;

    if (x <= 0) {
        warning("rnl(%d) attempted", x);
        return(0);
    }

    i = RND(x);

    if (Luck && rn2(50 - Luck)) {
        i -= (x <= 15 && Luck >= -5 ? Luck/3 : Luck);
        if (i < 0) i = 0;
        else if (i >= x) i = x-1;
    }

    return i;
}

int
rnd(x)      /**< 1 <= rnd(x) <= x */
register int x;
{
    if (x <= 0) {
        warning("rnd(%d) attempted", x);
        return(1);
    }
    x = RND(x)+1;
    return(x);
}

int
d(n, x)      /**< n <= d(n,x) <= (n*x) */
register int n, x;
{
    register int tmp = n;

    if (x < 0 || n < 0 || (x == 0 && n != 0)) {
        warning("d(%d,%d) attempted", n, x);
        return(1);
    }
    while(n--) tmp += RND(x);
    return(tmp); /* Alea iacta est. -- J.C. */
}

int
rne(x)
register int x;
{
    int i;

    int utmp = (u.ulevel < 15) ? 5 : u.ulevel/3;
    int r[MAXULEV/3];
    for (i=0; i < MAXULEV/3; i++) {
        r[i] = rnf(2, x+2);
    }
    /* Slightly higher probabilities for higher n than in NetHack 3.4.3
     * p(n) = \left(\frac{2}{x+2}\right)^{n-1} \frac{x}{x+2} */
    int n = 1;
    while (n < utmp && r[n-1]) {
        n++;
    }
    return n;

    /* was:
     *  tmp = 1;
     *  while(!rn2(x)) tmp++;
     *  return(min(tmp,(u.ulevel < 15) ? 5 : u.ulevel/3));
     * which is clearer but less efficient and stands a vanishingly
     * small chance of overflowing tmp
     */
}

int
rnz(i)
int i;
{
#ifdef LINT
    int x = i;
    int tmp = 1000;
#else
    register long x = i;
    register long tmp = 1000;
#endif
    tmp += rn2(1000);
    tmp *= rne(4);
    if (rn2(2)) { x *= tmp; x /= 1000; }
    else { x *= 1000; x /= tmp; }
    return((int)x);
}

int
rnf(numerator, denominator) /**< @returns (rnf(n,d) < n/d) */
int numerator, denominator;
{
    return rn2(denominator) < numerator;
}

/*rnd.c*/
