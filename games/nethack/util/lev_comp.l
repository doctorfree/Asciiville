%{
/*	SCCS Id: @(#)lev_lex.c	3.4	2002/03/27	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

#define LEV_LEX_C

#include "hack.h"
#include "lev_comp.h"
#include "sp_lev.h"

/* Most of these don't exist in flex, yywrap is macro and
 * yyunput is properly declared in flex.skel.
 */
#if !defined(FLEX_SCANNER) && !defined(FLEXHACK_SCANNER)
int FDECL(yyback, (int *,int));
int NDECL(yylook);
int NDECL(yyinput);
int NDECL(yywrap);
int NDECL(yylex);
	/* Traditional lexes let yyunput() and yyoutput() default to int;
	 * newer ones may declare them as void since they don't return
	 * values.  For even more fun, the lex supplied as part of the
	 * newer unbundled compiler for SunOS 4.x adds the void declarations
	 * (under __STDC__ or _cplusplus ifdefs -- otherwise they remain
	 * int) while the bundled lex and the one with the older unbundled
	 * compiler do not.  To detect this, we need help from outside --
	 * sys/unix/Makefile.utl.
	 *
	 * Digital UNIX is difficult and still has int in spite of all
	 * other signs.
	 */
# if defined(NeXT) || defined(SVR4) || defined(_AIX32)
#  define VOIDYYPUT
# endif
# if !defined(VOIDYYPUT) && defined(POSIX_TYPES)
#  if !defined(BOS) && !defined(HISX) && !defined(_M_UNIX) && !defined(VMS)
#   define VOIDYYPUT
#  endif
# endif
# if !defined(VOIDYYPUT) && defined(WEIRD_LEX)
#  if defined(SUNOS4) && defined(__STDC__) && (WEIRD_LEX > 1)
#   define VOIDYYPUT
#  endif
# endif
# if defined(VOIDYYPUT) && defined(__osf__)
#  undef VOIDYYPUT
# endif
# ifdef VOIDYYPUT
void FDECL(yyunput, (int));
void FDECL(yyoutput, (int));
# else
int FDECL(yyunput, (int));
int FDECL(yyoutput, (int));
# endif
#endif	/* !FLEX_SCANNER && !FLEXHACK_SCANNER */

#ifdef FLEX_SCANNER
#define YY_MALLOC_DECL \
	      genericptr_t FDECL(malloc, (size_t)); \
	      genericptr_t FDECL(realloc, (genericptr_t,size_t));
#endif

void FDECL(init_yyin, (FILE *));
void FDECL(init_yyout, (FILE *));

void FDECL(include_push, (const char *));
int NDECL(include_pop);

long NDECL(handle_varstring_method);
long FDECL(corefunc_str_check, (char *, long));

extern struct lc_vardefs *FDECL(vardef_defined,(struct lc_vardefs *,char *, int));

extern struct lc_vardefs *variable_definitions;

extern long FDECL(method_defined, (char *, long, long *));

void FDECL(savetoken, (char *));
void NDECL(newline);
void FDECL(advancepos, (char *));

extern void VDECL(lc_error, (const char *, ...));

extern char FDECL(core_func_retval, (int));
extern int FDECL(is_core_func, (char *));
extern int FDECL(core_func_idx, (char *));

extern int NDECL(yyparse);
extern void FDECL(yyerror, (const char *));

/*
 * This doesn't always get put in lev_comp.h
 * (esp. when using older versions of bison).
 */
extern YYSTYPE yylval;

int line_number = 1;
int token_start_pos = 0;
char curr_token[512];
static char map[4096];
static int map_cnt = 0;

int doing_included_file = 0;
YY_BUFFER_STATE include_stack[5];
int include_line[5];
char *include_file[5];

FILE *orig_yyin = NULL;

%}
%e 1500
%p 5000
%n 700
%s MAPC
%%
<MAPC>ENDMAP	{
		  savetoken(yytext);
		  BEGIN(INITIAL);
		  yylval.map = (char *) alloc(map_cnt + 1);
		  (void) strncpy(yylval.map, map, map_cnt);
		  yylval.map[map_cnt] = 0;
		  map_cnt = 0;
		  return MAP_ID;
		}
<MAPC>[-|}{+xABCISHKMPLWTtFYU\\#. 0123456789]*\r?\n	{
		  int len = yyleng;
		  savetoken(yytext);
		  /* convert \r\n to \n */
		  if (len >= 2 && yytext[len - 2] == '\r') len -= 1;
		  (void) strncpy(map + map_cnt, yytext, len);
		  map_cnt += len;
		  map[map_cnt - 1] = '\n';
		  map[map_cnt] = '\0';
		  newline();
		}
^[ \t]*#.*\n		{ savetoken(yytext); newline(); }
MESSAGE		{ savetoken(yytext); return MESSAGE_ID; }
NOMAP		{ savetoken(yytext); return NOMAP_ID; }
LEVEL		{ savetoken(yytext); return LEVEL_ID; }
INIT_MAP	{ savetoken(yytext); return LEV_INIT_ID; }
mazegrid	{ savetoken(yytext); return MAZE_GRID_ID; }
solidfill	{ savetoken(yytext); return SOLID_FILL_ID; }
mines		{ savetoken(yytext); return MINES_ID; }
sheollev	{ savetoken(yytext); return SHEOL_ID; }
rogue		{ savetoken(yytext); return ROGUELEV_ID; }
FLAGS		{ savetoken(yytext); return FLAGS_ID; }
GEOMETRY	{ savetoken(yytext); return GEOMETRY_ID; }
^MAP\r?\n		{ savetoken(yytext); BEGIN(MAPC); newline(); }
obj(ect)?	{ savetoken(yytext); return object_ID; }
OBJECT		{ savetoken(yytext); return OBJECT_ID; }
CONTAINER	{ savetoken(yytext); return COBJECT_ID; }
MONSTER		{ savetoken(yytext); return MONSTER_ID; }
monster		{ savetoken(yytext); return monster_ID; }
TRAP		{ savetoken(yytext); return TRAP_ID; }
DOOR		{ savetoken(yytext); return DOOR_ID; }
ROOMDOOR	{ savetoken(yytext); return ROOMDOOR_ID; }
DRAWBRIDGE	{ savetoken(yytext); return DRAWBRIDGE_ID; }
MAZEWALK	{ savetoken(yytext); return MAZEWALK_ID; }
WALLIFY		{ savetoken(yytext); return WALLIFY_ID; }
REGION		{ savetoken(yytext); return REGION_ID; }
ALTAR		{ savetoken(yytext); return ALTAR_ID; }
LADDER		{ savetoken(yytext); return LADDER_ID; }
STAIR		{ savetoken(yytext); return STAIR_ID; }
PORTAL		{ savetoken(yytext); return PORTAL_ID; }
TELEPORT_REGION	{ savetoken(yytext); return TELEPRT_ID; }
BRANCH		{ savetoken(yytext); return BRANCH_ID; }
FOUNTAIN	{ savetoken(yytext); return FOUNTAIN_ID; }
SINK		{ savetoken(yytext); return SINK_ID; }
POOL		{ savetoken(yytext); return POOL_ID; }
WALLWALK	{ savetoken(yytext); return WALLWALK_ID; }
NON_DIGGABLE	{ savetoken(yytext); return NON_DIGGABLE_ID; }
NON_PASSWALL	{ savetoken(yytext); return NON_PASSWALL_ID; }
IF		{ savetoken(yytext); return IF_ID; }
ELSE		{ savetoken(yytext); return ELSE_ID; }
EXIT		{ savetoken(yytext); return EXIT_ID; }
ROOM		{ savetoken(yytext); return ROOM_ID; }
SUBROOM		{ savetoken(yytext); return SUBROOM_ID; }
RANDOM_CORRIDORS	{ savetoken(yytext); return RAND_CORRIDOR_ID; }
CORRIDOR	{ savetoken(yytext); return CORRIDOR_ID; }
SPILL	  	{ savetoken(yytext); return SPILL_ID; }
TERRAIN		{ savetoken(yytext); return TERRAIN_ID; }
terrain		{ savetoken(yytext); return terrain_ID; }
REPLACE_TERRAIN	{ savetoken(yytext); return REPLACE_TERRAIN_ID; }
MON_GENERATION	{ savetoken(yytext); return MON_GENERATION_ID; }
GOLD		{ savetoken(yytext); return GOLD_ID; }
GRAVE		{ savetoken(yytext); return GRAVE_ID; }
ENGRAVING	{ savetoken(yytext); return ENGRAVING_ID; }
MINERALIZE	{ savetoken(yytext); return MINERALIZE_ID; }
(NAME|name)	{ savetoken(yytext); return NAME_ID; }
CHANCE		{ savetoken(yytext); return CHANCE_ID; }
FOR		{ savetoken(yytext); return FOR_ID; }
TO		{ savetoken(yytext); return TO_ID; }
LOOP		{ savetoken(yytext); return LOOP_ID; }
SWITCH		{ savetoken(yytext); return SWITCH_ID; }
CASE		{ savetoken(yytext); return CASE_ID; }
BREAK		{ savetoken(yytext); return BREAK_ID; }
DEFAULT		{ savetoken(yytext); return DEFAULT_ID; }
FUNCTION	{ savetoken(yytext); return FUNCTION_ID; }
INCLUDE		{ savetoken(yytext); return INCLUDE_ID; }
SOUNDS		{ savetoken(yytext); return SOUNDS_ID; }
SHUFFLE		{ savetoken(yytext); return SHUFFLE_ID; }
VAULTGEN	{ savetoken(yytext); return VAULTGEN_ID; }
montype		{ savetoken(yytext); return MONTYPE_ID; }
selection	{ savetoken(yytext); return selection_ID; }
rect		{ savetoken(yytext); return rect_ID; }
fillrect	{ savetoken(yytext); return fillrect_ID; }
line		{ savetoken(yytext); return line_ID; }
randline	{ savetoken(yytext); return randline_ID; }
grow		{ savetoken(yytext); return grow_ID; }
floodfill	{ savetoken(yytext); return flood_ID; }
rndcoord	{ savetoken(yytext); return rndcoord_ID; }
circle		{ savetoken(yytext); return circle_ID; }
ellipse		{ savetoken(yytext); return ellipse_ID; }
filter		{ savetoken(yytext); return filter_ID; }
gradient	{ savetoken(yytext); return gradient_ID; }
complement	{ savetoken(yytext); return complement_ID; }
radial		{ savetoken(yytext); yylval.i=SEL_GRADIENT_RADIAL;  return GRADIENT_TYPE; }
square		{ savetoken(yytext); yylval.i=SEL_GRADIENT_SQUARE;  return GRADIENT_TYPE; }
dry		{ savetoken(yytext); yylval.i=DRY;  return HUMIDITY_TYPE; }
wet		{ savetoken(yytext); yylval.i=WET;  return HUMIDITY_TYPE; }
hot		{ savetoken(yytext); yylval.i=HOT;  return HUMIDITY_TYPE; }
solid		{ savetoken(yytext); yylval.i=SOLID;  return HUMIDITY_TYPE; }
any		{ savetoken(yytext); yylval.i=ANY_LOC;  return HUMIDITY_TYPE; }
hear		{ savetoken(yytext); yylval.i=LVLSND_HEARD;  return MSG_OUTPUT_TYPE; }
pline		{ savetoken(yytext); yylval.i=LVLSND_PLINED; return MSG_OUTPUT_TYPE; }
verbal		{ savetoken(yytext); yylval.i=LVLSND_VERBAL; return MSG_OUTPUT_TYPE; }
feel		{ savetoken(yytext); yylval.i=LVLSND_FELT;   return MSG_OUTPUT_TYPE; }
levregion	{ savetoken(yytext); return LEV; }
quantity	{ savetoken(yytext); return QUANTITY_ID; }
buried		{ savetoken(yytext); return BURIED_ID; }
eroded		{ savetoken(yytext); return ERODED_ID; }
erodeproof	{ savetoken(yytext); return ERODEPROOF_ID; }
trapped		{ savetoken(yytext); yylval.i=1; return TRAPPED_STATE; }
not_trapped	{ savetoken(yytext); yylval.i=0; return TRAPPED_STATE; }
recharged	{ savetoken(yytext); return RECHARGED_ID; }
invisible	{ savetoken(yytext); return INVIS_ID; }
greased		{ savetoken(yytext); return GREASED_ID; }
female		{ savetoken(yytext); return FEMALE_ID; }
cancelled	{ savetoken(yytext); return CANCELLED_ID; }
revived		{ savetoken(yytext); return REVIVED_ID; }
avenge		{ savetoken(yytext); return AVENGE_ID; }
fleeing		{ savetoken(yytext); return FLEEING_ID; }
blinded		{ savetoken(yytext); return BLINDED_ID; }
paralyzed	{ savetoken(yytext); return PARALYZED_ID; }
stunned		{ savetoken(yytext); return STUNNED_ID; }
confused	{ savetoken(yytext); return CONFUSED_ID; }
seen_traps	{ savetoken(yytext); return SEENTRAPS_ID; }
all		{ savetoken(yytext); return ALL_ID; }
horizontal	{ savetoken(yytext); yylval.i=1; return HORIZ_OR_VERT; }
vertical	{ savetoken(yytext); yylval.i=2; return HORIZ_OR_VERT; }
open		{ savetoken(yytext); yylval.i=D_ISOPEN; return DOOR_STATE; }
closed		{ savetoken(yytext); yylval.i=D_CLOSED; return DOOR_STATE; }
locked		{ savetoken(yytext); yylval.i=D_LOCKED; return DOOR_STATE; }
nodoor		{ savetoken(yytext); yylval.i=D_NODOOR; return DOOR_STATE; }
broken		{ savetoken(yytext); yylval.i=D_BROKEN; return DOOR_STATE; }
secret		{ savetoken(yytext); yylval.i=D_SECRET; return DOOR_STATE; }
north		{ savetoken(yytext); yylval.i=W_NORTH; return DIRECTION; }
east		{ savetoken(yytext); yylval.i=W_EAST; return DIRECTION; }
south		{ savetoken(yytext); yylval.i=W_SOUTH; return DIRECTION; }
west		{ savetoken(yytext); yylval.i=W_WEST; return DIRECTION; }
random		{ savetoken(yytext); yylval.i = -1; return RANDOM_TYPE; }
random\[	{ savetoken(yytext); yylval.i = -1; return RANDOM_TYPE_BRACKET; }
none		{ savetoken(yytext); yylval.i = -2; return NONE; }
align		{ savetoken(yytext); return A_REGISTER; }
left		{ savetoken(yytext); yylval.i=1; return LEFT_OR_RIGHT; }
half-left	{ savetoken(yytext); yylval.i=2; return LEFT_OR_RIGHT; }
center		{ savetoken(yytext); yylval.i=3; return CENTER; }
half-right	{ savetoken(yytext); yylval.i=4; return LEFT_OR_RIGHT; }
right		{ savetoken(yytext); yylval.i=5; return LEFT_OR_RIGHT; }
top		{ savetoken(yytext); yylval.i=1; return TOP_OR_BOT; }
bottom		{ savetoken(yytext); yylval.i=5; return TOP_OR_BOT; }
lit		{ savetoken(yytext); yylval.i=1; return LIGHT_STATE; }
unlit		{ savetoken(yytext); yylval.i=0; return LIGHT_STATE; }
filled		{ savetoken(yytext); yylval.i=1; return FILLING; }
unfilled	{ savetoken(yytext); yylval.i=0; return FILLING; }
regular		{ savetoken(yytext); yylval.i=0; return IRREGULAR; }
irregular	{ savetoken(yytext); yylval.i=1; return IRREGULAR; }
unjoined	{ savetoken(yytext); yylval.i=1; return JOINED; }
joined		{ savetoken(yytext); yylval.i=0; return JOINED; }
limited		{ savetoken(yytext); yylval.i=1; return LIMITED; }
unlimited	{ savetoken(yytext); yylval.i=0; return LIMITED; }
noalign		{ savetoken(yytext); yylval.i= AM_NONE; return ALIGNMENT; }
law		{ savetoken(yytext); yylval.i= AM_LAWFUL; return ALIGNMENT; }
neutral		{ savetoken(yytext); yylval.i= AM_NEUTRAL; return ALIGNMENT; }
chaos		{ savetoken(yytext); yylval.i= AM_CHAOTIC; return ALIGNMENT; }
coaligned	{ savetoken(yytext); yylval.i= AM_SPLEV_CO; return ALIGNMENT; }
noncoaligned	{ savetoken(yytext); yylval.i= AM_SPLEV_NONCO; return ALIGNMENT; }
peaceful	{ savetoken(yytext); yylval.i=1; return MON_ATTITUDE; }
hostile		{ savetoken(yytext); yylval.i=0; return MON_ATTITUDE; }
asleep		{ savetoken(yytext); yylval.i=1; return MON_ALERTNESS; }
awake		{ savetoken(yytext); yylval.i=0; return MON_ALERTNESS; }
m_feature	{ savetoken(yytext); yylval.i= M_AP_FURNITURE; return MON_APPEARANCE; }
m_monster	{ savetoken(yytext); yylval.i= M_AP_MONSTER;   return MON_APPEARANCE; }
m_object	{ savetoken(yytext); yylval.i= M_AP_OBJECT;    return MON_APPEARANCE; }
sanctum		{ savetoken(yytext); yylval.i=2; return ALTAR_TYPE; }
shrine		{ savetoken(yytext); yylval.i=1; return ALTAR_TYPE; }
altar		{ savetoken(yytext); yylval.i=0; return ALTAR_TYPE; }
up		{ savetoken(yytext); yylval.i=1; return UP_OR_DOWN; }
down		{ savetoken(yytext); yylval.i=0; return UP_OR_DOWN; }
false		{ savetoken(yytext); yylval.i=0; return BOOLEAN; }
true		{ savetoken(yytext); yylval.i=1; return BOOLEAN; }
dust		{ savetoken(yytext); yylval.i=DUST; return ENGRAVING_TYPE; }
engrave		{ savetoken(yytext); yylval.i=ENGRAVE; return ENGRAVING_TYPE; }
burn		{ savetoken(yytext); yylval.i=BURN; return ENGRAVING_TYPE; }
mark		{ savetoken(yytext); yylval.i=MARK; return ENGRAVING_TYPE; }
blood		{ savetoken(yytext); yylval.i=ENGR_BLOOD; return ENGRAVING_TYPE; }
blessed		{ savetoken(yytext); yylval.i=1; return CURSE_TYPE; }
uncursed	{ savetoken(yytext); yylval.i=2; return CURSE_TYPE; }
cursed		{ savetoken(yytext); yylval.i=3; return CURSE_TYPE; }
rndvault	{ savetoken(yytext); yylval.i=FLAG_RNDVAULT; return FLAG_TYPE; }
noteleport	{ savetoken(yytext); yylval.i=NOTELEPORT; return FLAG_TYPE; }
sheol		{ savetoken(yytext); yylval.i=SHEOL_LEV; return FLAG_TYPE; }
hardfloor	{ savetoken(yytext); yylval.i=HARDFLOOR; return FLAG_TYPE; }
nommap		{ savetoken(yytext); yylval.i=NOMMAP; return FLAG_TYPE; }
arboreal	{ savetoken(yytext); yylval.i=ARBOREAL; return FLAG_TYPE; }	/* KMH */
shortsighted	{ savetoken(yytext); yylval.i=SHORTSIGHTED; return FLAG_TYPE; }
noflipx		{ savetoken(yytext); yylval.i=NOFLIPX; return FLAG_TYPE; }
noflipy		{ savetoken(yytext); yylval.i=NOFLIPY; return FLAG_TYPE; }
noflip		{ savetoken(yytext); yylval.i=(NOFLIPX + NOFLIPY); return FLAG_TYPE; }
mazelevel	{ savetoken(yytext); yylval.i=MAZELEVEL; return FLAG_TYPE; }
premapped	{ savetoken(yytext); yylval.i=PREMAPPED; return FLAG_TYPE; }
shroud		{ savetoken(yytext); yylval.i=SHROUD; return FLAG_TYPE; }
stormy		{ savetoken(yytext); yylval.i=STORMY; return FLAG_TYPE; }
graveyard	{ savetoken(yytext); yylval.i=GRAVEYARD; return FLAG_TYPE; }
sky		{ savetoken(yytext); yylval.i=SKYMAP; return FLAG_TYPE; }
icedpools     { savetoken(yytext); yylval.i=ICEDPOOLS; return FLAG_TYPE; }
solidify      { savetoken(yytext); yylval.i=SOLIDIFY; return FLAG_TYPE; }
corrmaze      { savetoken(yytext); yylval.i=CORRMAZE; return FLAG_TYPE; }
inaccessibles { savetoken(yytext); yylval.i=CHECK_INACCESSIBLES; return FLAG_TYPE; }
[0-9]+d[0-9]+	{ char *p = strchr(yytext, 'd');
		  savetoken(yytext);
		  if (p) {
			*p = '\0';
			p++;
			yylval.dice.num=atoi(yytext);
			yylval.dice.die=atoi(p);
		  } else { yylval.dice.num = yylval.dice.die = 1; }
		  return DICE;
		}
\[\ *[0-9]+\%\ *\] { savetoken(yytext); yylval.i = atoi(yytext + 1);
		     if (yylval.i < 0 || yylval.i > 100)
			lc_error("Unexpected percentile '%li%%'", yylval.i);
		     return PERCENT; }
-[0-9]+		{ savetoken(yytext); yylval.i=atoi(yytext); return MINUS_INTEGER; }
\+[0-9]+	{ savetoken(yytext); yylval.i=atoi(yytext); return PLUS_INTEGER; }
[0-9]+\%	{ savetoken(yytext); yylval.i = atoi(yytext);
		  if (yylval.i < 0 || yylval.i > 100)
		     lc_error("Unexpected percentile '%li%%'", yylval.i);
		  return SPERCENT; }
[0-9]+		{ savetoken(yytext); yylval.i=atoi(yytext); return INTEGER; }
\"[^"]*\"	{ savetoken(yytext);
		  yytext[yyleng-1] = 0; /* Discard the trailing \" */
		  yylval.map = (char *) alloc(strlen(yytext+1)+1);
		  Strcpy(yylval.map, yytext+1); /* Discard the first \" */
		  return STRING; }
\$[a-zA-Z_](\.?[a-zA-Z_0-9]+)*	{ savetoken(yytext); return handle_varstring_method(); }
[a-zA-Z_](\.?[a-zA-Z_0-9]+)*	{
		  savetoken(yytext);
		  yylval.map = (char *) alloc(strlen(yytext)+1);
		  Strcpy(yylval.map, yytext);
		  return corefunc_str_check(yylval.map, NQSTRING);
		  }
"=="		{ savetoken(yytext); yylval.i = SPO_JE; return COMPARE_TYPE; }
"!="		{ savetoken(yytext); yylval.i = SPO_JNE; return COMPARE_TYPE; }
"<>"		{ savetoken(yytext); yylval.i = SPO_JNE; return COMPARE_TYPE; }
"<="		{ savetoken(yytext); yylval.i = SPO_JLE; return COMPARE_TYPE; }
">="		{ savetoken(yytext); yylval.i = SPO_JGE; return COMPARE_TYPE; }
"<"		{ savetoken(yytext); yylval.i = SPO_JL; return COMPARE_TYPE; }
">"		{ savetoken(yytext); yylval.i = SPO_JG; return COMPARE_TYPE; }
\r?\n		{ newline(); }
[ \t]+		{ advancepos(yytext); }
'\\.'		{ savetoken(yytext); yylval.i = yytext[2]; return CHAR; }
'.'		{ savetoken(yytext); yylval.i = yytext[1]; return CHAR; }
.		{ savetoken(yytext); return yytext[0]; }
%%
#ifdef	AMIGA
long *alloc(n)
	unsigned n;
{
	return ((long *)malloc (n));
}
#endif

/* routine to switch to another input file; needed for flex */
void init_yyin( input_f )
FILE *input_f;
{
#if defined(FLEX_SCANNER) || defined(FLEXHACK_SCANNER)
	if (yyin)
	    yyrestart(input_f);
	else
#endif
    {
	    yyin = input_f;
	    if (!orig_yyin) orig_yyin = yyin;
    }
}
/* analogous routine (for completeness) */
void init_yyout( output_f )
FILE *output_f;
{
	yyout = output_f;
}

void
include_push(fname)
const char *fname;
{
  if (doing_included_file >= 5)
    yyerror("Too deep includes");
  else {
    FILE *fin;
    fin = fopen(fname, "r");
    if (!fin) {
       yyerror("Cannot open file for including.");
    } else {
       include_stack[doing_included_file] = YY_CURRENT_BUFFER;
       include_line[doing_included_file] = line_number;
       include_file[doing_included_file] = strdup(fname);
       doing_included_file++;
       yyin = fin;
       yy_switch_to_buffer(yy_create_buffer(fin, YY_BUF_SIZE));
       BEGIN(INITIAL);
       yyparse();
    }
  }
}

int
include_pop()
{
  if (--doing_included_file < 0) {
    doing_included_file = 0;
    init_yyin(orig_yyin);
    yy_switch_to_buffer(yy_create_buffer(orig_yyin, YY_BUF_SIZE));
  } else {
    if (include_file[doing_included_file])
       free(include_file[doing_included_file]);
    yy_switch_to_buffer(include_stack[doing_included_file]);
    line_number = include_line[doing_included_file];
  }
  return 0;
}

/*
 * Stub needed for lex interface.
 */
int
yywrap()
{
    if (doing_included_file > 0) {
        include_pop();
    }
    return 1;
}

long
handle_varstring_check()
{
  struct lc_vardefs *vd;
  yylval.map = (char *) alloc(strlen(yytext)+1);
  Strcpy(yylval.map, yytext);
  if ((vd = vardef_defined(variable_definitions, yytext, 1))) {
     long l = vd->var_type;
     long a = ((l & SPOVAR_ARRAY) == SPOVAR_ARRAY);
     l = (l & ~SPOVAR_ARRAY);
     if (l == SPOVAR_INT) return (a ? VARSTRING_INT_ARRAY : VARSTRING_INT);
     if (l == SPOVAR_STRING) return (a ? VARSTRING_STRING_ARRAY : VARSTRING_STRING);
     if (l == SPOVAR_VARIABLE) return (a ? VARSTRING_VAR_ARRAY : VARSTRING_VAR);
     if (l == SPOVAR_COORD) return (a ? VARSTRING_COORD_ARRAY : VARSTRING_COORD);
     if (l == SPOVAR_REGION) return (a ? VARSTRING_REGION_ARRAY : VARSTRING_REGION);
     if (l == SPOVAR_MAPCHAR) return (a ? VARSTRING_MAPCHAR_ARRAY : VARSTRING_MAPCHAR);
     if (l == SPOVAR_MONST) return (a ? VARSTRING_MONST_ARRAY : VARSTRING_MONST);
     if (l == SPOVAR_OBJ) return (a ? VARSTRING_OBJ_ARRAY : VARSTRING_OBJ);
     if (l == SPOVAR_SEL) return (a ? VARSTRING_SEL_ARRAY : VARSTRING_SEL);
  }
  return VARSTRING;
}

long
handle_varstring_method()
{
  struct lc_vardefs *vd;
  char *idx = index(yytext, '.');
  long r;

  if (!idx) return handle_varstring_check();
  *idx = '\0';
  idx++;

  if ((vd = vardef_defined(variable_definitions, yytext, 1))) {
     long mth;
     long l = vd->var_type;
     long a = ((l & SPOVAR_ARRAY) == SPOVAR_ARRAY);
     long rval;
     if ((rval = method_defined(idx, l, &mth))) {
          yylval.meth.varstr = strdup(yytext);
          yylval.meth.cfunc = mth;
          long rvala = ((rval & SPOVAR_ARRAY) == SPOVAR_ARRAY);
          rval = (rval & ~SPOVAR_ARRAY);
          if (rval == SPOVAR_INT) return (rvala ? METHOD_INT_ARRAY : METHOD_INT);
          if (rval == SPOVAR_STRING) return (rvala ? METHOD_STRING_ARRAY : METHOD_STRING);
          if (rval == SPOVAR_VARIABLE) return (rvala ? METHOD_VAR_ARRAY : METHOD_VAR);
          if (rval == SPOVAR_COORD) return (rvala ? METHOD_COORD_ARRAY : METHOD_COORD);
          if (rval == SPOVAR_REGION) return (rvala ? METHOD_REGION_ARRAY : METHOD_REGION);
          if (rval == SPOVAR_MAPCHAR) return (rvala ? METHOD_MAPCHAR_ARRAY : METHOD_MAPCHAR);
          if (rval == SPOVAR_MONST) return (rvala ? METHOD_MONST_ARRAY : METHOD_MONST);
          if (rval == SPOVAR_OBJ) return (rvala ? METHOD_OBJ_ARRAY : METHOD_OBJ);
          if (rval == SPOVAR_SEL) return (rvala ? METHOD_SEL_ARRAY : METHOD_SEL);
     }

  } else {
    lc_error("Trying to use a method of unknown variable '%s'.", yylval);
    return VARSTRING;
  }
  return VARSTRING;
}


long
corefunc_str_check(s, retval)
char *s;
long retval;
{
  int f = is_core_func(s);
  if (f) {
    int i = core_func_idx(s);
    switch (core_func_retval(i)) {
      default:
	lc_error("Core function '%s' has unknown return type '%c'.", yylval.map, core_func_retval(i));
	break;
      case 'i': return CFUNC_INT;
      case 's': return CFUNC_STR;
      case 'c': return CFUNC_COORD;
      case 'r': return CFUNC_REGION;
      case 0:
      case ' ': return CFUNC;
    }
  }
  return retval;
}

void
newline()
{
  line_number++;
  token_start_pos = 0;
  memset(curr_token, 0, 512);
}

void
savetoken(s)
char *s;
{
   sprintf(curr_token, "%s", s);
   advancepos(s);
}

void
advancepos(s)
char *s;
{
   token_start_pos += strlen(s);
}

/*lev_comp.l*/
