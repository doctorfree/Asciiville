/*Written by Timothy B. Terriberry (tterribe@xiph.org) 1999-2009.
   CC0 (Public domain) - see http://creativecommons.org/publicdomain/zero/1.0/ for details
   Based on the public domain implementation by Robert J. Jenkins Jr.*/
#include <math.h>
#include <string.h>
#include "isaac.h"


#define ISAAC_MASK        (0xFFFFFFFFU)

/* Extract ISAAC_SZ_LOG bits (starting at bit 2). */
static inline uint32_t lower_bits(uint32_t x)
{
    return (x & ((ISAAC_SZ-1) << 2)) >> 2;
}

/* Extract next ISAAC_SZ_LOG bits (starting at bit ISAAC_SZ_LOG+2). */
static inline uint32_t upper_bits(uint32_t y)
{
    return (y >> (ISAAC_SZ_LOG+2)) & (ISAAC_SZ-1);
}

static void isaac_update(isaac_ctx *_ctx){
    uint32_t *m;
    uint32_t *r;
    uint32_t a;
    uint32_t b;
    uint32_t x;
    uint32_t y;
    int i;
    m=_ctx->m;
    r=_ctx->r;
    a=_ctx->a;
    b=_ctx->b+(++_ctx->c);
    for(i=0; i<ISAAC_SZ/2; i++) {
        x=m[i];
        a=(a^a<<13)+m[i+ISAAC_SZ/2];
        m[i]=y=m[lower_bits(x)]+a+b;
        r[i]=b=m[upper_bits(y)]+x;
        x=m[++i];
        a=(a^a>>6)+m[i+ISAAC_SZ/2];
        m[i]=y=m[lower_bits(x)]+a+b;
        r[i]=b=m[upper_bits(y)]+x;
        x=m[++i];
        a=(a^a<<2)+m[i+ISAAC_SZ/2];
        m[i]=y=m[lower_bits(x)]+a+b;
        r[i]=b=m[upper_bits(y)]+x;
        x=m[++i];
        a=(a^a>>16)+m[i+ISAAC_SZ/2];
        m[i]=y=m[lower_bits(x)]+a+b;
        r[i]=b=m[upper_bits(y)]+x;
    }
    for(i=ISAAC_SZ/2; i<ISAAC_SZ; i++) {
        x=m[i];
        a=(a^a<<13)+m[i-ISAAC_SZ/2];
        m[i]=y=m[lower_bits(x)]+a+b;
        r[i]=b=m[upper_bits(y)]+x;
        x=m[++i];
        a=(a^a>>6)+m[i-ISAAC_SZ/2];
        m[i]=y=m[lower_bits(x)]+a+b;
        r[i]=b=m[upper_bits(y)]+x;
        x=m[++i];
        a=(a^a<<2)+m[i-ISAAC_SZ/2];
        m[i]=y=m[lower_bits(x)]+a+b;
        r[i]=b=m[upper_bits(y)]+x;
        x=m[++i];
        a=(a^a>>16)+m[i-ISAAC_SZ/2];
        m[i]=y=m[lower_bits(x)]+a+b;
        r[i]=b=m[upper_bits(y)]+x;
    }
    _ctx->b=b;
    _ctx->a=a;
    _ctx->n=ISAAC_SZ;
}

static void isaac_mix(uint32_t _x[8]){
    static const unsigned char SHIFT[8]={11, 2, 8, 16, 10, 4, 8, 9};
    int i;
    for(i=0; i<8; i++) {
        _x[i]^=_x[(i+1)&7]<<SHIFT[i];
        _x[(i+3)&7]+=_x[i];
        _x[(i+1)&7]+=_x[(i+2)&7];
        i++;
        _x[i]^=_x[(i+1)&7]>>SHIFT[i];
        _x[(i+3)&7]+=_x[i];
        _x[(i+1)&7]+=_x[(i+2)&7];
    }
}


void isaac_init(isaac_ctx *_ctx, const unsigned char *_seed, int _nseed){
    _ctx->a=_ctx->b=_ctx->c=0;
    memset(_ctx->r, 0, sizeof(_ctx->r));
    isaac_reseed(_ctx, _seed, _nseed);
}

void isaac_reseed(isaac_ctx *_ctx, const unsigned char *_seed, int _nseed){
    uint32_t *m;
    uint32_t *r;
    uint32_t x[8];
    int i;
    int j;
    m=_ctx->m;
    r=_ctx->r;
    if(_nseed>ISAAC_SEED_SZ_MAX) _nseed=ISAAC_SEED_SZ_MAX;
    for(i=0; i<_nseed>>2; i++) {
        r[i]^=(uint32_t)_seed[i<<2|3]<<24|(uint32_t)_seed[i<<2|2]<<16|
               (uint32_t)_seed[i<<2|1]<<8|_seed[i<<2];
    }
    _nseed-=i<<2;
    if(_nseed>0) {
        uint32_t ri;
        ri=_seed[i<<2];
        for(j=1; j<_nseed; j++) ri|=(uint32_t)_seed[i<<2|j]<<(j<<3);
        r[i++]^=ri;
    }
    x[0]=x[1]=x[2]=x[3]=x[4]=x[5]=x[6]=x[7]=0x9E3779B9U;
    for(i=0; i<4; i++) isaac_mix(x);
    for(i=0; i<ISAAC_SZ; i+=8) {
        for(j=0; j<8; j++) x[j]+=r[i+j];
        isaac_mix(x);
        memcpy(m+i, x, sizeof(x));
    }
    for(i=0; i<ISAAC_SZ; i+=8) {
        for(j=0; j<8; j++) x[j]+=m[i+j];
        isaac_mix(x);
        memcpy(m+i, x, sizeof(x));
    }
    isaac_update(_ctx);
}

uint32_t isaac_next_uint32(isaac_ctx *_ctx){
    if(!_ctx->n) isaac_update(_ctx);
    return _ctx->r[--_ctx->n];
}

uint32_t isaac_next_uint(isaac_ctx *_ctx, uint32_t _n){
    uint32_t r;
    uint32_t v;
    uint32_t d;
    do {
        r=isaac_next_uint32(_ctx);
        v=r%_n;
        d=r-v;
    }
    while(((d+_n-1)&ISAAC_MASK)<d);
    return v;
}
