
#define NEARDATA

#include "objects.c"
#include "monst.c"

struct window_procs windowprocs;

struct cmd Cmd;

#include "hacklib.c"
#include "decl.c"
#include "drawing.c"
#include "options.c"
#include "rnd.c"
#include "alloc.c"
#include "mkobj.c"
#include "mondata.c"
#include "base32.c"

#include "objnam.c"

#include <stdarg.h>

void raw_printf(const char *str,...) {}
void impossible(const char *str,...) {}
void warning(const char *str,...) {}

unsigned long get_feature_notice_ver(char *str) { return 0; }
unsigned long get_current_feature_ver() { return 0; }

void pline(const char *str,...) {}
void pline_The(const char *str,...) {}
void You_cant(const char *str,...) {}
void Your(const char *str, ...) {}
void You_hear(const char *str, ...) {}
void You_see(const char *str, ...) {}
void verbalize(const char *str, ...) {}
void panic(const char *str,...) {}
void nh_terminate(int status) { exit(0); }
void docrt() {}
int isok(int x, int y) { return 1; }

int str2role(const char *str) { return 0; }
int str2race(const char *str) { return 0; }
int str2gend(const char *str) { return 0; }
int str2align(const char *str) { return 0; }

boolean read_config_file(const char *str, int src) { return FALSE; }
const char* configfile = "";

const struct Align aligns[] = {};
const struct Gender genders[] = {};
const struct Role roles[] = {};
const struct Race races[] = {};
struct Role urole;
struct Race urace;

int doredraw() { return 0; }
boolean on_level(d_level *a, d_level *b) { return FALSE; }

void vision_recalc(int a) {}
void choose_windows(const char *str) {}
void reassign() {}

boolean Can_fall_thru(d_level *l) { return FALSE; }
const char *align_str(ALIGNTYP_P alignment) { return "unknown"; }

const char* artifact_name(const char *name, short *typ) { return NULL; }
void attach_egg_hatch_timeout(struct obj *obj, long i) {}
void block_point(int x, int y) {}
void unblock_point(int x, int y) {}
const char* body_part(int typ) { return "body_part"; }
int can_be_hatched(int typ) { return typ; }
void consume_oeaten(struct obj *obj, int amount) {}
boolean dead_species(int typ, BOOLEAN_P bool) { return FALSE; }
void del_engr_at(int x, int y) {}
void delete_contents(struct obj *obj) {}
int genus(int typ, int number) { return 0; }
boolean is_quest_artifact(struct obj *obj) { return FALSE; }
void kill_egg(struct obj *obj) {}
void make_grave(int x, int y, const char *engraving) {}
struct trap* maketrap(int x, int y, int typ) { return NULL; }
void newsym(int x, int y) {}
int title_to_mon(const char *str, int* number1, int* number2) { return NON_PM; }

void obfree(struct obj *obj1, struct obj *obj2) {}
struct obj* oname(struct obj *obj, const char *str) { return NULL; }
int  water_damage(struct obj *o, const char *str, BOOLEAN_P b) { return FALSE; }
void water_damage_chain(struct obj *o, BOOLEAN_P b) {}
void reset_remarm() {}
struct permonst* rndmonst() { return NULL; }

boolean worm_known(struct monst *monst) { return FALSE; }
struct obj* which_armor(struct monst *monst, long number) { return NULL; }
int mon_has_amulet(struct monst *monst) { return 0; }

uint32_t isaac_next_uint32(isaac_ctx *_ctx) { return 0; }
void isaac_init(isaac_ctx *_ctx,const unsigned char *_seed,int _nseed) {}

const char *artiname(int i) { return 0; }
struct obj* mk_artifact(struct obj *obj, ALIGNTYP_P typ) { return NULL; }
boolean exist_artifact(int typ, const char *str) { return FALSE; }
void artifact_exists(struct obj *obj, const char *str, BOOLEAN_P bool) {}
int nartifact_exist() { return 0; }
boolean confers_luck(struct obj *obj) { return FALSE; }
boolean defends(int number1, struct obj *obj) { return FALSE; }
boolean defends_when_carried(int i, struct obj *o) { return FALSE; }
boolean protects(struct obj *o, BOOLEAN_P b) { return FALSE; }
boolean undiscovered_artifact(XCHAR_P typ) { return FALSE; }
boolean MATCH_WARN_OF_MON(struct monst *monst) { return FALSE; }

void set_moreluck() {}

boolean is_ice(int x, int y) { return FALSE; }

void obj_no_longer_held(struct obj *obj) {}

char* mon_nam(struct monst *monst) { return "mon_nam"; }
struct obj* realloc_obj(struct obj *obj, int number1, genericptr_t generic, int number2, const char *str) { return NULL; }

boolean In_quest(d_level *level) { return FALSE; }
boolean In_hell(d_level *level) { return FALSE; }
xchar level_difficulty() { return 1; }

int eaten_stat(int number1, struct obj *obj) { return 0; }

char* in_rooms(XCHAR_P x, XCHAR_P y, int number) { return NULL; }

int merged(struct obj **obj1, struct obj **obj2) { return 0; }
const char* currency(long number) { return "currency"; }
struct obj* g_at(int x, int y) { return NULL; }
void freeinv(struct obj *obj) {}

void del_light_source(int number, ANY_P *generic) {}
boolean obj_sheds_light(struct obj *obj) { return FALSE; }
void obj_split_light_source(struct obj *obj1, struct obj *obj2) {}

boolean prohibited_by_generation_flags(struct permonst *permonst) { return FALSE; }

int undead_to_corpse(int number) { return NON_PM; }

void discover_object(int number, BOOLEAN_P bool1, BOOLEAN_P bool2) {}

struct monst* shop_keeper(CHAR_P c) { return NULL; }
long contained_cost(struct obj *obj, struct monst *monst, long number, BOOLEAN_P bool1, BOOLEAN_P bool2) { return 0; }
long unpaid_cost(struct obj *obj, BOOLEAN_P b) { return 0; }
void addtobill(struct obj *obj, BOOLEAN_P bool1, BOOLEAN_P bool2, BOOLEAN_P bool3) {}
void splitbill(struct obj *obj1, struct obj *obj2) {}
void subfrombill(struct obj *obj, struct monst *monst) {}
boolean is_fshk(struct monst *monst) { return FALSE; }
boolean costly_spot(XCHAR_P x, XCHAR_P y) { return FALSE; }
long get_cost_of_shop_item(struct obj *obj) { return 0; }
char* shk_your(char *buf, struct obj *obj) { return "shk_your"; }
char* Shk_Your(char *buf, struct obj *obj) { return "Shk_Your"; }

boolean rloco(struct obj *obj) { return FALSE; }

void attach_fig_transform_timeout(struct obj *obj) {}
void begin_burn(struct obj *obj, BOOLEAN_P bool) {}
boolean start_timer(long timeleft, SHORT_P typ, SHORT_P action, ANY_P *generic) { return TRUE; }
long stop_timer(SHORT_P typ, ANY_P *generic) { return 0; }
void obj_split_timers(struct obj *obj1, struct obj *obj2) {}
void obj_stop_timers(struct obj *obj) {}

struct obj* tt_oname(struct obj *obj) { return NULL; }

boolean lava_effects() { return FALSE; }
boolean swamp_effects() { return FALSE; }

void drop_uswapwep() {}
boolean get_obj_location(struct obj *obj, xchar *x, xchar *y, int number) { return FALSE; }

void copy_mextra(struct monst *m1, struct monst *m2) {}
void dealloc_mextra(struct monst *m) {}

struct obj *hold_another_object(struct obj *o, const char *s1, const char *s2, const char *s3) { return NULL; }
boolean can_reach_floor(BOOLEAN_P b) { return FALSE; }
void hitfloor(struct obj *o, BOOLEAN_P b) {}
void doaltarobj(struct obj *o) {}
const char *surface(int i1, int i2) { return NULL; }
void dropy(struct obj *o) {}

anything *obj_to_any(struct obj *o) { return NULL; }

int arti_light_radius(struct obj *o) { return 0; }


void alter_cost(struct obj *o, long l) {}
boolean billable(struct monst **m, struct obj *o, CHAR_P c, BOOLEAN_P b) { return FALSE; }
void book_cursed(struct obj *o) {}
void consume_obj_charge(struct obj *o, BOOLEAN_P b) {}
void forget_temple_entry(struct monst *m) {}
void maybe_reset_pick(struct obj *o) {}
boolean mergable(struct obj *o1, struct obj *o2) { return FALSE; }
void mwepgone(struct monst *m) {}
struct obj *nxtobj(struct obj *o, int i, BOOLEAN_P b) { return NULL; }
void obj_adjust_light_radius(struct obj *o, int i) {}
int oid_price_adjustment(struct obj *o, unsigned i) { return 0; }
void set_tin_variety(struct obj *o, int i) {}
struct obj *sobj_at(int i1, int i2, int i3) { return NULL; }
long stolen_value(struct obj *o, XCHAR_P c1, XCHAR_P c2, BOOLEAN_P b1, BOOLEAN_P b2) { return 0; }
void update_mon_intrinsics(struct monst *m, struct obj *o, BOOLEAN_P b1, BOOLEAN_P b2) {}
char *x_monnam(struct monst *m, int i1, const char *s, int i2, BOOLEAN_P b) { return NULL; }
const char *safe_oname(struct obj *o) { return NULL; }

void update_inventory() {}
void reset_commands(BOOLEAN_P b) {}

char *tshirt_text(struct obj *o, char *s) { return NULL; }
char *hawaiian_motif(struct obj *o, char *s) { return NULL; }
char *apron_text(struct obj *o, char *s) { return NULL; }
void tin_details(struct obj *o, int i, char *s) {}

long count_contents(struct obj *o, BOOLEAN_P b1, BOOLEAN_P b2, BOOLEAN_P b3, BOOLEAN_P b4) { return 0; }
boolean donning(struct obj *o) { return FALSE; }
boolean doffing(struct obj *o) { return FALSE; }
struct monst *find_mid(unsigned i1, unsigned i2) { return NULL; }
char *noit_mon_nam(struct monst *m) { return NULL; }

const char *glow_color(int i) { return NULL; }
const char *glow_verb(int i, BOOLEAN_P b) { return NULL; }
boolean restoring = FALSE;

boolean is_pool(int i1, int i2) { return FALSE; }
boolean is_lava(int i1, int i3) { return FALSE; }
void feel_newsym(XCHAR_P x, XCHAR_P y) {}
void reset_utrap(BOOLEAN_P b) {}

boolean is_fainted() { return FALSE; }
boolean unconscious() { return FALSE; }

char txt2key(char *txt) { return ' '; }
boolean bind_key(UCHAR_P key, const char *command) { return FALSE; }
boolean bind_specialkey(UCHAR_P key, const char *command) { return FALSE; }

int zombie_form(struct permonst *pm) { return 0; }
