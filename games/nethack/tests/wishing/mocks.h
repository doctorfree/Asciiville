
#define NEARDATA

#include "objects.c"
#include "monst.c"

struct window_procs windowprocs;

#include "hacklib.c"
#include "decl.c"
#include "drawing.c"
#include "options.c"
#include "rnd.c"
#include "alloc.c"
#include "mkobj.c"
#include "mondata.c"

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
void panic(const char *str,...) {}
void terminate(int status) {}

int str2role(char *str) { return 0; }
int str2race(char *str) { return 0; }
int str2gend(char *str) { return 0; }
int str2align(char *str) { return 0; }

void read_config_file(const char *str) {}
const char* configfile = "";

const struct Align aligns[] = {};
const struct Gender genders[] = {};
const struct Role roles[] = {};
const struct Race races[] = {};
struct Role urole;
struct Race urace;

struct obj *thrownobj = 0;

int doredraw() { return 0; }
boolean on_level(d_level *a, d_level *b) { return FALSE; }

void vision_recalc(int a) {}
void choose_windows(const char *str) {}
void reassign() {}

boolean Can_fall_thru(d_level *l) { return FALSE; }
const char *align_str(ALIGNTYP_P alignment) { return "unknown"; }

const char* artifact_name(const char *name, short *typ) { return NULL; }
void attach_egg_hatch_timeout(struct obj *obj) {}
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
int title_to_mon(const char *str, int* number1, int* number2) { return 0; }

void obfree(struct obj *obj1, struct obj *obj2) {}
struct obj* oname(struct obj *obj, const char *str) { return NULL; }
boolean water_damage(struct obj * obj, BOOLEAN_P bool1, BOOLEAN_P bool2) { return FALSE; }
void reset_remarm() {}
struct permonst* rndmonst() { return NULL; }

boolean worm_known(struct monst *monst) { return FALSE; }
struct obj* which_armor(struct monst *monst, long number) { return NULL; }
int mon_has_amulet(struct monst *monst) { return 0; }



/* ### artifact.c ### */
//E void NDECL(init_artifacts);
//E void FDECL(save_artifacts, (int));
//E void FDECL(restore_artifacts, (int));
//E const char *FDECL(artiname, (int));
struct obj* mk_artifact(struct obj *obj, ALIGNTYP_P typ) { return NULL; }
//E const char *FDECL(artifact_name, (const char *,short *));
boolean exist_artifact(int typ, const char *str) { return FALSE; }
void artifact_exists(struct obj *obj, const char *str, BOOLEAN_P bool) {}
int nartifact_exist() { return 0; }
//E boolean FDECL(spec_ability, (struct obj *,unsigned long));
boolean confers_luck(struct obj *obj) { return FALSE; }
//E boolean FDECL(arti_reflects, (struct obj *));
//E boolean FDECL(restrict_name, (struct obj *,const char *,BOOLEAN_P));
boolean defends(int number1, struct obj *obj) { return FALSE; }
boolean protects(int number2, struct obj *obj) { return FALSE; }
//E void FDECL(set_artifact_intrinsic, (struct obj *,BOOLEAN_P,long));
//E int FDECL(touch_artifact, (struct obj *,struct monst *));
//E int FDECL(spec_abon, (struct obj *,struct monst *));
//E int FDECL(spec_dbon, (struct obj *,struct monst *,int));
//E void FDECL(discover_artifact, (XCHAR_P));
boolean undiscovered_artifact(XCHAR_P typ) { return FALSE; }
//E int FDECL(disp_artifact_discoveries, (winid));
//E boolean FDECL(artifact_hit, (struct monst *,struct monst *,
//                                struct obj *,int *,int));
//E int NDECL(doinvoke);
//E void FDECL(arti_speak, (struct obj *));
//E boolean FDECL(artifact_light, (struct obj *));
//E long FDECL(spec_m2, (struct obj *));
//E boolean FDECL(artifact_has_invprop, (struct obj *,UCHAR_P));
//E long FDECL(arti_cost, (struct obj *));
boolean MATCH_WARN_OF_MON(struct monst *monst) { return FALSE; }
//E const char *FDECL(get_warned_of_monster, (struct obj *));


/* ### attrib.c ### */
//E boolean FDECL(adjattrib, (int,int,int));
//E void FDECL(change_luck, (SCHAR_P));
//E int FDECL(stone_luck, (BOOLEAN_P));
void set_moreluck() {}
//E void FDECL(gainstr, (struct obj *,int));
//E void FDECL(losestr, (int));
//E void NDECL(restore_attrib);
//E void FDECL(exercise, (int,BOOLEAN_P));
//E void NDECL(exerchk);
//E void NDECL(reset_attribute_clock);
//E void FDECL(init_attr, (int));
//E void NDECL(redist_attr);
//E void FDECL(adjabil, (int,int));
//E int NDECL(newhp);
//E schar FDECL(acurr, (int));
//E schar NDECL(acurrstr);
//E void FDECL(adjalign, (int));
//E const char * NDECL(beautiful);
//E int NDECL(uhp);
//E int NDECL(uhpmax);


/* ### dbridge.c ### */
//E boolean FDECL(is_pool, (int,int));
//E boolean FDECL(is_lava, (int,int));
boolean is_ice(int x, int y) { return FALSE; }
//E boolean FDECL(is_swamp, (int,int));
//E boolean FDECL(is_icewall, (int,int));
//E boolean FDECL(is_any_icewall, (int,int));
//E int FDECL(is_drawbridge_wall, (int,int));
//E boolean FDECL(is_db_wall, (int,int));
//E boolean FDECL(find_drawbridge, (int *,int*));
//E boolean FDECL(create_drawbridge, (int,int,int,int));
//E void FDECL(open_drawbridge, (int,int));
//E boolean FDECL(close_drawbridge, (int,int));
//E void FDECL(destroy_drawbridge, (int,int));

/* ### do.c ### */
//#ifdef USE_TRAMPOLI
//E int FDECL(drop, (struct obj *));
//E int NDECL(wipeoff);
//#endif
//E int NDECL(dodrop);
//E boolean FDECL(boulder_hits_pool, (struct obj *,int,int,BOOLEAN_P));
//E boolean FDECL(flooreffects, (struct obj *,int,int,const char *));
//E void FDECL(doaltarobj, (struct obj *));
//E boolean FDECL(canletgo, (struct obj *,const char *));
//E void FDECL(dropx, (struct obj *));
//E void FDECL(dropy, (struct obj *));
void obj_no_longer_held(struct obj *obj) {}
//E int NDECL(doddrop);
//E int NDECL(dodown);
//E int NDECL(doup);
//#ifdef INSURANCE
//E void NDECL(save_currentstate);
//#endif
//E void FDECL(goto_level, (d_level *,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P));
//E void FDECL(schedule_goto, (d_level *,BOOLEAN_P,BOOLEAN_P,int,
//                             const char *,const char *));
//E void NDECL(deferred_goto);
//E boolean FDECL(revive_corpse, (struct obj *));
//E void FDECL(revive_mon, (genericptr_t, long));
//E int NDECL(donull);
//E int NDECL(dowipe);
//E void FDECL(set_wounded_legs, (long,int));
//E void NDECL(heal_legs);

/* ### do_name.c ### */
//E void FDECL(do_oname, (struct obj *));
//E int FDECL(getpos, (coord *,BOOLEAN_P,const char *));
//E struct monst *FDECL(christen_monst, (struct monst *,const char *));
//E int NDECL(do_mname);
//E struct obj *FDECL(oname, (struct obj *,const char *));
//E int NDECL(ddocall);
//E void FDECL(docall, (struct obj *));
//E const char *NDECL(rndghostname);
//E char *FDECL(x_monnam, (struct monst *,int,const char *,int,BOOLEAN_P));
//E char *FDECL(l_monnam, (struct monst *));
char* mon_nam(struct monst *monst) { return "mon_nam"; }
//E char *FDECL(noit_mon_nam, (struct monst *));
//E char *FDECL(Monnam, (struct monst *));
//E char *FDECL(noit_Monnam, (struct monst *));
//E char *FDECL(m_monnam, (struct monst *));
//E char *FDECL(y_monnam, (struct monst *));
//E char *FDECL(Adjmonnam, (struct monst *,const char *));
//E char *FDECL(Amonnam, (struct monst *));
//E char *FDECL(a_monnam, (struct monst *));
//E char *FDECL(distant_monnam, (struct monst *,int,char *));
//E const char *NDECL(rndmonnam);
//E const char *FDECL(hcolor, (const char *));
//E const char *NDECL(rndcolor);
//#ifdef REINCARNATION
//E const char *NDECL(roguename);
//#endif
struct obj* realloc_obj(struct obj *obj, int number1, genericptr_t generic, int number2, const char *str) { return NULL; }
//E char *FDECL(coyotename, (struct monst *,char *));

/* ### dungeon.c ### */
//E void FDECL(save_dungeon, (int,BOOLEAN_P,BOOLEAN_P));
//E void FDECL(restore_dungeon, (int));
//E void FDECL(insert_branch, (branch *,BOOLEAN_P));
//E void NDECL(init_dungeons);
//E s_level *FDECL(find_level, (const char *));
//E s_level *FDECL(Is_special, (d_level *));
//#ifdef RANDOMIZED_PLANES
//E s_level *FDECL(get_next_elemental_plane, (d_level *));
//E d_level *NDECL(get_first_elemental_plane);
//#endif
//E branch *FDECL(Is_branchlev, (d_level *));
//E xchar FDECL(ledger_no, (d_level *));
//E xchar NDECL(maxledgerno);
//E schar FDECL(depth, (d_level *));
//E xchar FDECL(dunlev, (d_level *));
//E xchar FDECL(dunlevs_in_dungeon, (d_level *));
//E xchar FDECL(ledger_to_dnum, (XCHAR_P));
//E xchar FDECL(ledger_to_dlev, (XCHAR_P));
//E xchar FDECL(deepest_lev_reached, (BOOLEAN_P));
//E boolean FDECL(on_level, (d_level *,d_level *));
//E void FDECL(next_level, (BOOLEAN_P));
//E void FDECL(prev_level, (BOOLEAN_P));
//E void FDECL(u_on_newpos, (int,int));
//E void NDECL(u_on_sstairs);
//E void NDECL(u_on_upstairs);
//E void NDECL(u_on_dnstairs);
//E boolean FDECL(On_stairs, (XCHAR_P,XCHAR_P));
//E void FDECL(get_level, (d_level *,int));
//E boolean FDECL(Is_botlevel, (d_level *));
//E boolean FDECL(Can_fall_thru, (d_level *));
//E boolean FDECL(Can_dig_down, (d_level *));
//E boolean FDECL(Can_rise_up, (int,int,d_level *));
boolean In_quest(d_level *level) { return FALSE; }
//E boolean FDECL(In_mines, (d_level *));
//E boolean FDECL(In_sheol, (d_level *));
//E branch *FDECL(dungeon_branch, (const char *));
//E boolean FDECL(at_dgn_entrance, (const char *));
boolean In_hell(d_level *level) { return FALSE; }
//E boolean FDECL(In_V_tower, (d_level *));
//E boolean FDECL(On_W_tower_level, (d_level *));
//E boolean FDECL(In_W_tower, (int,int,d_level *));
//E void FDECL(find_hell, (d_level *));
//E void FDECL(goto_hell, (BOOLEAN_P,BOOLEAN_P));
//E void FDECL(assign_level, (d_level *,d_level *));
//E void FDECL(assign_rnd_level, (d_level *,d_level *,int));
//E int FDECL(induced_align, (int));
//E boolean FDECL(Invocation_lev, (d_level *));
xchar level_difficulty() { return 1; }
//E schar FDECL(lev_by_name, (const char *));
//#ifdef WIZARD
//E schar FDECL(print_dungeon, (BOOLEAN_P,schar *,xchar *));
//#endif
//E int NDECL(donamelevel);
//E int NDECL(dooverview);
//E void FDECL(forget_mapseen, (int));
//E void FDECL(init_mapseen, (d_level *));
//E void NDECL(recalc_mapseen);
//E void FDECL(recbranch_mapseen, (d_level *, d_level *));
//E void FDECL(remdun_mapseen, (int));
//E char *FDECL(get_annotation, (d_level *));
//E const char *FDECL(get_generic_level_description, (d_level *));

/* ### eat.c ### */
//#ifdef USE_TRAMPOLI
//E int NDECL(eatmdone);
//E int NDECL(eatfood);
//E int NDECL(opentin);
//E int NDECL(unfaint);
//#endif
//E boolean FDECL(is_edible, (struct obj *));
//E void NDECL(init_uhunger);
//E int NDECL(Hear_again);
//E void NDECL(reset_eat);
//E int NDECL(doeat);
//E void NDECL(gethungry);
//E void FDECL(morehungry, (int));
//E void FDECL(lesshungry, (int));
//E boolean NDECL(is_fainted);
//E void NDECL(reset_faint);
//E void NDECL(violated_vegetarian);
//#if 0
//E void NDECL(sync_hunger);
//#endif
//E void FDECL(newuhs, (BOOLEAN_P));
//E struct obj *FDECL(floorfood, (const char *,int));
//E void NDECL(vomit);
int eaten_stat(int number1, struct obj *obj) { return 0; }
//E void FDECL(food_disappears, (struct obj *));
//E void FDECL(food_substitution, (struct obj *,struct obj *));
//E boolean FDECL(bite_monster, (struct monst *mon));
//E void NDECL(fix_petrification);
//E void FDECL(consume_oeaten, (struct obj *,int));
//E boolean FDECL(maybe_finished_meal, (BOOLEAN_P));

/* ### hack.c ### */
//#ifdef DUNGEON_GROWTH
//E void FDECL(catchup_dgn_growths, (int));
//E void FDECL(dgn_growths, (BOOLEAN_P,BOOLEAN_P));
//#endif
//E boolean FDECL(revive_nasty, (int,int,const char*));
//E void FDECL(movobj, (struct obj *,XCHAR_P,XCHAR_P));
//E boolean FDECL(may_dig, (XCHAR_P,XCHAR_P));
//E boolean FDECL(may_passwall, (XCHAR_P,XCHAR_P));
//E boolean FDECL(bad_rock, (struct permonst *,XCHAR_P,XCHAR_P));
//E boolean FDECL(invocation_pos, (XCHAR_P,XCHAR_P));
//E boolean FDECL(test_move, (int, int, int, int, int));
//E void NDECL(domove);
//E void NDECL(invocation_message);
//E void FDECL(wounds_message,(struct monst *));
//E char *FDECL(mon_wounds,(struct monst *));
//E void FDECL(spoteffects, (BOOLEAN_P));
char* in_rooms(XCHAR_P x, XCHAR_P y, int number) { return NULL; }
//E boolean FDECL(in_town, (int,int));
//E void FDECL(check_special_room, (BOOLEAN_P));
//E int NDECL(dopickup);
//E void NDECL(lookaround);
//E int NDECL(monster_nearby);
//E void FDECL(nomul, (int, const char *));
//E void FDECL(unmul, (const char *));
//E void FDECL(showdmg, (int, BOOLEAN_P));
//E void FDECL(losehp, (int,const char *,BOOLEAN_P));
//E void FDECL(losehp_how, (int,const char *,BOOLEAN_P,int));
//E void FDECL(set_uhpmax, (int,BOOLEAN_P));
//E void NDECL(check_uhpmax);
//E int NDECL(weight_cap);
//E int NDECL(inv_weight);
//E int NDECL(near_capacity);
//E int FDECL(calc_capacity, (int));
//E int NDECL(max_capacity);
//E boolean FDECL(check_capacity, (const char *));
//E int NDECL(inv_cnt);
//#ifdef GOLDOBJ
//E long FDECL(money_cnt, (struct obj *));
//#endif

/* ### invent.c ### */
int merged(struct obj **obj1, struct obj **obj2) { return 0; }
const char* currency(long number) { return "currency"; }
struct obj* g_at(int x, int y) { return NULL; }
void freeinv(struct obj *obj) {}

/* ### light.c ### */
//E void FDECL(new_light_source, (XCHAR_P, XCHAR_P, int, int, genericptr_t));
void del_light_source(int number, genericptr_t generic) {}
//E void FDECL(do_light_sources, (char **));
//E struct monst *FDECL(find_mid, (unsigned, unsigned));
//E void FDECL(save_light_sources, (int, int, int));
//E void FDECL(restore_light_sources, (int));
//E void FDECL(relink_light_sources, (BOOLEAN_P));
//E void FDECL(obj_move_light_source, (struct obj *, struct obj *));
//E boolean NDECL(any_light_source);
//E void FDECL(snuff_light_source, (int, int));
boolean obj_sheds_light(struct obj *obj) { return FALSE; }
//E boolean FDECL(obj_is_burning, (struct obj *));
void obj_split_light_source(struct obj *obj1, struct obj *obj2) {}
//E void FDECL(obj_merge_light_sources, (struct obj *,struct obj *));
//E int FDECL(candle_light_range, (struct obj *));
//#ifdef WIZARD
//E int NDECL(wiz_light_sources);
//#endif
//
/* ### makemon.c ### */
//E boolean FDECL(is_home_elemental, (struct permonst *));
//E struct monst *FDECL(clone_mon, (struct monst *,XCHAR_P,XCHAR_P));
//E struct monst *FDECL(makemon, (struct permonst *,int,int,int));
//E boolean FDECL(create_critters, (int,struct permonst *));
//E struct permonst *NDECL(rndmonst);
//E void FDECL(reset_rndmonst, (int));
//E struct permonst *FDECL(mkclass, (CHAR_P,int));
//E int FDECL(adj_lev, (struct permonst *));
//E struct permonst *FDECL(grow_up, (struct monst *,struct monst *));
//E int FDECL(mongets, (struct monst *,int));
//E int FDECL(golemhp, (int));
//E boolean FDECL(peace_minded, (struct permonst *));
//E void FDECL(set_malign, (struct monst *));
//E void FDECL(set_mimic_sym, (struct monst *));
//E int FDECL(mbirth_limit, (int));
//E void FDECL(mimic_hit_msg, (struct monst *, SHORT_P));
//#ifdef GOLDOBJ
//E void FDECL(mkmonmoney, (struct monst *, long));
//#endif
//E int FDECL(bagotricks, (struct obj *));
//E boolean FDECL(propagate, (int, BOOLEAN_P,BOOLEAN_P));
//E void FDECL(create_camera_demon, (struct obj *,int,int));
//E int NDECL(min_monster_difficulty);
//E int NDECL(max_monster_difficulty);
boolean prohibited_by_generation_flags(struct permonst *permonst) { return FALSE; }


/* ### mon.c ### */
//E int FDECL(select_newcham_form, (struct monst *));
//E void FDECL(remove_monster, (int, int));
int undead_to_corpse(int number) { return NON_PM; }


/* ### o_init.c ### */
//E void NDECL(init_objects);
//E int NDECL(find_skates);
//E void NDECL(oinit);
//E void FDECL(savenames, (int,int));
//E void FDECL(restnames, (int));
void discover_object(int number, BOOLEAN_P bool1, BOOLEAN_P bool2) {}
//E void FDECL(undiscover_object, (int));
//E int NDECL(dodiscovered);
//E void NDECL(dragons_init);

/* ### shk.c ### */
//#ifdef GOLDOBJ
//E long FDECL(money2mon, (struct monst *, long));
//E void FDECL(money2u, (struct monst *, long));
//#endif
//E char *FDECL(shkname, (struct monst *));
//E void FDECL(shkgone, (struct monst *));
//E void FDECL(set_residency, (struct monst *,BOOLEAN_P));
//E void FDECL(replshk, (struct monst *,struct monst *));
//E void FDECL(restshk, (struct monst *,BOOLEAN_P));
//E char FDECL(inside_shop, (XCHAR_P,XCHAR_P));
//E void FDECL(u_left_shop, (char *,BOOLEAN_P));
//E void FDECL(remote_burglary, (XCHAR_P,XCHAR_P));
//E void FDECL(u_entered_shop, (char *));
//E boolean FDECL(same_price, (struct obj *,struct obj *));
//E void NDECL(shopper_financial_report);
//E int FDECL(inhishop, (struct monst *));
struct monst* shop_keeper(CHAR_P c) { return NULL; }
//E boolean FDECL(tended_shop, (struct mkroom *));
//E void FDECL(delete_contents, (struct obj *));
//E void FDECL(obfree, (struct obj *,struct obj *));
//E void FDECL(home_shk, (struct monst *,BOOLEAN_P));
//E void FDECL(make_happy_shk, (struct monst *,BOOLEAN_P));
//E void FDECL(hot_pursuit, (struct monst *));
//E void FDECL(make_angry_shk, (struct monst *,XCHAR_P,XCHAR_P));
//E int NDECL(dopay);
//E boolean FDECL(paybill, (int));
//E void NDECL(finish_paybill);
//E struct obj *FDECL(find_oid, (unsigned));
long contained_cost(struct obj *obj, struct monst *monst, long number, BOOLEAN_P bool1, BOOLEAN_P bool2) { return 0; }
//E long FDECL(contained_gold, (struct obj *));
//E void FDECL(picked_container, (struct obj *));
long unpaid_cost(struct obj *obj) { return 0; }
void addtobill(struct obj *obj, BOOLEAN_P bool1, BOOLEAN_P bool2, BOOLEAN_P bool3) {}
void splitbill(struct obj *obj1, struct obj *obj2) {}
void subfrombill(struct obj *obj, struct monst *monst) {}
//E long FDECL(stolen_value, (struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P,BOOLEAN_P));
//E void FDECL(sellobj_state, (int));
//E void FDECL(sellobj, (struct obj *,XCHAR_P,XCHAR_P));
//E int FDECL(doinvbill, (int));
//E struct monst *FDECL(shkcatch, (struct obj *,XCHAR_P,XCHAR_P));
//E void FDECL(add_damage, (XCHAR_P,XCHAR_P,long));
//E int FDECL(repair_damage, (struct monst *,struct damage *,BOOLEAN_P));
//E int FDECL(shk_move, (struct monst *));
//E void FDECL(after_shk_move, (struct monst *));
boolean is_fshk(struct monst *monst) { return FALSE; }
//E void FDECL(shopdig, (int));
//E void FDECL(pay_for_damage, (const char *,BOOLEAN_P));
boolean costly_spot(XCHAR_P x, XCHAR_P y) { return FALSE; }
//E struct obj *FDECL(shop_object, (XCHAR_P,XCHAR_P));
//E void FDECL(price_quote, (struct obj *));
//E void FDECL(shk_chat, (struct monst *));
//E void FDECL(check_unpaid_usage, (struct obj *,BOOLEAN_P));
//E void FDECL(check_unpaid, (struct obj *));
//E void FDECL(costly_gold, (XCHAR_P,XCHAR_P,long));
long get_cost_of_shop_item(struct obj *obj) { return 0; }
//E boolean FDECL(block_door, (XCHAR_P,XCHAR_P));
//E boolean FDECL(block_entry, (XCHAR_P,XCHAR_P));
//#ifdef BLACKMARKET
//E void FDECL(blkmar_guards, (struct monst *));
//E void NDECL(set_black_marketeer_angry);
//E void FDECL(bars_around_portal, (BOOLEAN_P));
//#endif /* BLACKMARKET */
char* shk_your(char *buf, struct obj *obj) { return "shk_your"; }
char* Shk_Your(char *buf, struct obj *obj) { return "Shk_Your"; }

/* ### teleport.c ### */
//E boolean FDECL(goodpos, (int,int,struct monst *,unsigned));
//E boolean FDECL(enexto, (coord *,XCHAR_P,XCHAR_P,struct permonst *));
//E boolean FDECL(enexto_core, (coord *,XCHAR_P,XCHAR_P,struct permonst *,unsigned));
//E boolean FDECL(enexto_core_range, (coord *,XCHAR_P,XCHAR_P,struct permonst *,unsigned,int));
//E int FDECL(epathto, (coord *,int,XCHAR_P,XCHAR_P,struct permonst *));
//E void FDECL(teleds, (int,int,BOOLEAN_P));
//E boolean FDECL(safe_teleds, (BOOLEAN_P));
//E boolean FDECL(teleport_pet, (struct monst *,BOOLEAN_P));
//E void NDECL(tele);
//E int NDECL(dotele);
//E void NDECL(level_tele);
//E void FDECL(domagicportal, (struct trap *));
//E void FDECL(tele_trap, (struct trap *));
//E void FDECL(level_tele_trap, (struct trap *));
//E void FDECL(rloc_to, (struct monst *,int,int));
//E boolean FDECL(rloc, (struct monst *, BOOLEAN_P));
//E boolean FDECL(tele_restrict, (struct monst *));
//E void FDECL(mtele_trap, (struct monst *, struct trap *,int));
//E int FDECL(mlevel_tele_trap, (struct monst *, struct trap *,BOOLEAN_P,int));
void rloco(struct obj *obj) {}
//E int NDECL(random_teleport_level);
//E boolean FDECL(u_teleport_mon, (struct monst *,BOOLEAN_P));

/* ### timeout.c ### */
//E void NDECL(burn_away_slime);
//E void NDECL(nh_timeout);
//E void FDECL(fall_asleep, (int, BOOLEAN_P));
//E void FDECL(attach_egg_hatch_timeout, (struct obj *));
void attach_fig_transform_timeout(struct obj *obj) {}
//E void FDECL(kill_egg, (struct obj *));
//E void FDECL(hatch_egg, (genericptr_t, long));
//E void FDECL(learn_egg_type, (int));
//E void FDECL(burn_object, (genericptr_t, long));
void begin_burn(struct obj *obj, BOOLEAN_P bool) {}
//E void FDECL(end_burn, (struct obj *, BOOLEAN_P));
//E void NDECL(do_storms);
boolean start_timer(long timeleft, SHORT_P typ, SHORT_P action, genericptr_t generic) { return TRUE; }
long stop_timer(SHORT_P typ, genericptr_t generic) { return 0; }
//E void NDECL(run_timers);
//E void FDECL(obj_move_timers, (struct obj *, struct obj *));
void obj_split_timers(struct obj *obj1, struct obj *obj2) {}
void obj_stop_timers(struct obj *obj) {}
//E boolean FDECL(obj_is_local, (struct obj *));
//E void FDECL(save_timers, (int,int,int));
//E void FDECL(restore_timers, (int,int,BOOLEAN_P,long));
//E void FDECL(relink_timers, (BOOLEAN_P));

/* ### topten.c ### */
//#ifdef RECORD_CONDUCT
//E long FDECL(encodeconduct, (void));
//#endif
//E void FDECL(topten, (int));
//E void FDECL(prscore, (int,char **));
struct obj* tt_oname(struct obj *obj) { return NULL; }

/* ### trap.c ### */
//E boolean FDECL(burnarmor,(struct monst *));
//E boolean FDECL(rust_dmg, (struct obj *,const char *,int,BOOLEAN_P,struct monst *));
//E void FDECL(grease_protect, (struct obj *,const char *,struct monst *));
//E struct trap *FDECL(maketrap, (int,int,int));
//E void FDECL(fall_through, (BOOLEAN_P));
//E struct monst *FDECL(animate_statue, (struct obj *,XCHAR_P,XCHAR_P,int,int *));
//E struct monst *FDECL(activate_statue_trap,
//                        (struct trap *,XCHAR_P,XCHAR_P,BOOLEAN_P));
//E void FDECL(dotrap, (struct trap *, unsigned));
//E void FDECL(seetrap, (struct trap *));
//E int FDECL(mintrap, (struct monst *));
//E void FDECL(instapetrify, (const char *));
//E void FDECL(minstapetrify, (struct monst *,BOOLEAN_P));
//#ifdef WEBB_DISINT
//E int FDECL(instadisintegrate, (const char *));
//E int FDECL(minstadisintegrate, (struct monst *));
//#endif
//E void FDECL(selftouch, (const char *));
//E void FDECL(mselftouch, (struct monst *,const char *,BOOLEAN_P));
//E void NDECL(float_up);
//E void FDECL(fill_pit, (int,int));
//E int FDECL(float_down, (long, long));
//E int FDECL(fire_damage, (struct obj *,BOOLEAN_P,BOOLEAN_P,XCHAR_P,XCHAR_P));
//E boolean FDECL(water_damage, (struct obj *,BOOLEAN_P,BOOLEAN_P));
//E boolean NDECL(drown);
//E void FDECL(drain_en, (int));
//E int NDECL(dountrap);
//E int FDECL(untrap, (BOOLEAN_P));
//E boolean FDECL(chest_trap, (struct obj *,int,BOOLEAN_P));
//E void FDECL(deltrap, (struct trap *));
//E boolean FDECL(delfloortrap, (struct trap *));
//E struct trap *FDECL(t_at, (int,int));
//E void FDECL(b_trapped, (const char *,int));
//E boolean NDECL(unconscious);
boolean lava_effects() { return FALSE; }
boolean swamp_effects() { return FALSE; }
//E void FDECL(blow_up_landmine, (struct trap *));
//E int FDECL(launch_obj,(SHORT_P,int,int,int,int,int));

/* ### wield.c ### */
//E void FDECL(setuwep, (struct obj *));
//E void FDECL(setuqwep, (struct obj *));
//E void FDECL(setuswapwep, (struct obj *));
//E int NDECL(dowield);
//E int NDECL(doswapweapon);
//E int NDECL(dowieldquiver);
//E boolean FDECL(wield_tool, (struct obj *,const char *));
//E int NDECL(can_twoweapon);
void drop_uswapwep() {}
//E int NDECL(dotwoweapon);
//E void NDECL(uwepgone);
//E void NDECL(uswapwepgone);
//E void NDECL(uqwepgone);
//E void NDECL(untwoweapon);
//E void FDECL(erode_obj, (struct obj *,BOOLEAN_P,BOOLEAN_P));
//E int FDECL(chwepon, (struct obj *,int));
//E int FDECL(welded, (struct obj *));
//E void FDECL(weldmsg, (struct obj *));
//E void FDECL(setmnotwielded, (struct monst *,struct obj *));
//E void NDECL(unwield_weapons_silently);

/* ### zap.c ### */
//E int FDECL(bhitm, (struct monst *,struct obj *));
//E void FDECL(probe_monster, (struct monst *));
boolean get_obj_location(struct obj *obj, xchar *x, xchar *y, int number) { return FALSE; }
//E boolean FDECL(get_mon_location, (struct monst *,xchar *,xchar *,int));
//E struct monst *FDECL(get_container_location, (struct obj *obj, int *, int *));
//E struct monst *FDECL(montraits, (struct obj *,coord *));
//E struct monst *FDECL(revive, (struct obj *));
//E int FDECL(unturn_dead, (struct monst *));
//E void FDECL(cancel_item, (struct obj *));
//E boolean FDECL(drain_item, (struct obj *));
//E struct obj *FDECL(poly_obj, (struct obj *, int));
//E boolean FDECL(obj_resists, (struct obj *,int,int));
//E boolean FDECL(obj_shudders, (struct obj *));
//E void FDECL(do_osshock, (struct obj *));
//E int FDECL(bhito, (struct obj *,struct obj *));
//E int FDECL(bhitpile, (struct obj *,int (*)(OBJ_P,OBJ_P),int,int));
//E int FDECL(zappable, (struct obj *));
//E void FDECL(zapnodir, (struct obj *));
//E int NDECL(dozap);
//E int FDECL(zapyourself, (struct obj *,BOOLEAN_P));
//E boolean FDECL(cancel_monst, (struct monst *,struct obj *,
//                               BOOLEAN_P,BOOLEAN_P,BOOLEAN_P));
//E void FDECL(weffects, (struct obj *));
//E int NDECL(spell_damage_bonus);
//E const char *FDECL(exclam, (int force));
//E void FDECL(hit, (const char *,struct monst *,const char *));
//E void FDECL(miss, (const char *,struct monst *));
//E struct monst *FDECL(bhit, (int,int,int,int,int (*)(MONST_P,OBJ_P),
//                             int (*)(OBJ_P,OBJ_P),struct obj *, boolean *));
//E struct monst *FDECL(boomhit, (int,int));
//E int FDECL(burn_floor_paper, (int,int,BOOLEAN_P,BOOLEAN_P));
//E void FDECL(buzz, (int,int,XCHAR_P,XCHAR_P,int,int));
//E void FDECL(melt_ice, (XCHAR_P,XCHAR_P));
//E int FDECL(zap_over_floor, (XCHAR_P,XCHAR_P,int,boolean *));
//E void FDECL(fracture_rock, (struct obj *));
//E boolean FDECL(break_statue, (struct obj *));
//E void FDECL(destroy_item, (int,int));
//E int FDECL(destroy_mitem, (struct monst *,int,int));
//E int FDECL(resist, (struct monst *,CHAR_P,int,int));
//E void FDECL(makewish, (BOOLEAN_P));

