#include "hacklib.c"
#include "decl.c"
#include "rnd.c"
#include "alloc.c"

#include <stdarg.h>

void raw_printf(const char *str,...) {}
void impossible(const char *str,...) {}
void warning(const char *str,...) {}

void pline(const char *str,...) {}
void pline_The(const char *str,...) {}
void You_cant(const char *str,...) {}
void panic(const char *str,...) {}
void nh_terminate(int status) { exit(0); }
void docrt() {}

uint32_t isaac_next_uint32(isaac_ctx *_ctx) { return 0; }
void isaac_init(isaac_ctx *_ctx,const unsigned char *_seed,int _nseed) {}
