
#define NEARDATA

#include "objects.c"
#include "monst.c"

struct window_procs windowprocs;

struct cmd Cmd;

#include "../../src/hacklib.c"
#include "../../src/decl.c"
#include "../../src/drawing.c"
#include "../../src/options.c"
#include "../../src/alloc.c"
#include "base32.c"


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
void nh_terminate(int status) { exit(0); }
void docrt() {}

int str2role(const char *str) { return 0; }
int str2race(const char *str) { return 0; }
int str2gend(const char *str) { return 0; }
int str2align(const char *str) { return 0; }

int name_to_mon(const char *str) { return 0; }
boolean read_config_file(const char *str, int src) { return FALSE; }
const char* configfile = "";

const struct Align aligns[] = {};
const struct Gender genders[] = {};
const struct Role roles[] = {};
const struct Race races[] = {};

char* makeplural(const char *str) { return (char*)str; }

int doredraw() { return 0; }
boolean on_level(d_level *a, d_level *b) { return FALSE; }

void vision_recalc(int a) {}
void choose_windows(const char *str) {}
void reassign() {}

void set_random_state(unsigned int a) { }
int rnd(int a) { return 0; }

void reset_commands(BOOLEAN_P b) {}

char txt2key(char *txt) { return ' '; }
boolean bind_key(UCHAR_P key, const char *command) { return FALSE; }
boolean bind_specialkey(UCHAR_P key, const char *command) { return FALSE; }
