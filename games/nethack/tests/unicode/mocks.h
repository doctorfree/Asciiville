
#define NEARDATA

#include "objects.c"
#include "monst.c"

struct window_procs windowprocs;

#include "../../src/hacklib.c"
#include "../../src/decl.c"
#include "../../src/drawing.c"
#include "../../src/options.c"
#include "../../src/rnd.c"
#include "../../src/alloc.c"


#include <stdarg.h>

void raw_printf(const char *str,...) {}
void impossible(const char *str,...) {}
void warning(const char *str,...) {}

unsigned long get_feature_notice_ver(char *str) { return 0; }
unsigned long get_current_feature_ver() { return 0; }

void pline(const char *str,...) {}
void pline_The(const char *str,...) {}
void You_cant(const char *str,...) {}
void panic(const char *str,...) {}
void terminate(int status) {}

int str2role(char *str) { return 0; }
int str2race(char *str) { return 0; }
int str2gend(char *str) { return 0; }
int str2align(char *str) { return 0; }

int name_to_mon(const char *str) { return 0; }
void read_config_file(const char *str) {}
const char* configfile = "";

const struct Align aligns[] = {};
const struct Gender genders[] = {};
const struct Role roles[] = {};
const struct Race races[] = {};


int doredraw() { return 0; }
boolean on_level(d_level *a, d_level *b) { return FALSE; }

void vision_recalc(int a) {}
void choose_windows(const char *str) {}
void reassign() {}

