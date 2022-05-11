/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     CHAR = 258,
     INTEGER = 259,
     BOOLEAN = 260,
     PERCENT = 261,
     SPERCENT = 262,
     MINUS_INTEGER = 263,
     PLUS_INTEGER = 264,
     MAZE_GRID_ID = 265,
     SOLID_FILL_ID = 266,
     MINES_ID = 267,
     MESSAGE_ID = 268,
     LEVEL_ID = 269,
     LEV_INIT_ID = 270,
     GEOMETRY_ID = 271,
     NOMAP_ID = 272,
     OBJECT_ID = 273,
     COBJECT_ID = 274,
     MONSTER_ID = 275,
     TRAP_ID = 276,
     DOOR_ID = 277,
     DRAWBRIDGE_ID = 278,
     MAZEWALK_ID = 279,
     WALLIFY_ID = 280,
     REGION_ID = 281,
     FILLING = 282,
     ALTAR_ID = 283,
     LADDER_ID = 284,
     STAIR_ID = 285,
     NON_DIGGABLE_ID = 286,
     NON_PASSWALL_ID = 287,
     ROOM_ID = 288,
     PORTAL_ID = 289,
     TELEPRT_ID = 290,
     BRANCH_ID = 291,
     LEV = 292,
     CHANCE_ID = 293,
     CORRIDOR_ID = 294,
     GOLD_ID = 295,
     ENGRAVING_ID = 296,
     FOUNTAIN_ID = 297,
     POOL_ID = 298,
     SINK_ID = 299,
     NONE = 300,
     RAND_CORRIDOR_ID = 301,
     DOOR_STATE = 302,
     LIGHT_STATE = 303,
     CURSE_TYPE = 304,
     ENGRAVING_TYPE = 305,
     DIRECTION = 306,
     RANDOM_TYPE = 307,
     A_REGISTER = 308,
     ALIGNMENT = 309,
     LEFT_OR_RIGHT = 310,
     CENTER = 311,
     TOP_OR_BOT = 312,
     ALTAR_TYPE = 313,
     UP_OR_DOWN = 314,
     SUBROOM_ID = 315,
     NAME_ID = 316,
     FLAGS_ID = 317,
     FLAG_TYPE = 318,
     MON_ATTITUDE = 319,
     MON_ALERTNESS = 320,
     MON_APPEARANCE = 321,
     ROOMDOOR_ID = 322,
     IF_ID = 323,
     ELSE_ID = 324,
     SPILL_ID = 325,
     TERRAIN_ID = 326,
     HORIZ_OR_VERT = 327,
     REPLACE_TERRAIN_ID = 328,
     EXIT_ID = 329,
     SHUFFLE_ID = 330,
     QUANTITY_ID = 331,
     BURIED_ID = 332,
     LOOP_ID = 333,
     SWITCH_ID = 334,
     CASE_ID = 335,
     BREAK_ID = 336,
     DEFAULT_ID = 337,
     ERODED_ID = 338,
     TRAPPED_ID = 339,
     RECHARGED_ID = 340,
     INVIS_ID = 341,
     GREASED_ID = 342,
     FEMALE_ID = 343,
     CANCELLED_ID = 344,
     REVIVED_ID = 345,
     AVENGE_ID = 346,
     FLEEING_ID = 347,
     BLINDED_ID = 348,
     PARALYZED_ID = 349,
     STUNNED_ID = 350,
     CONFUSED_ID = 351,
     SEENTRAPS_ID = 352,
     ALL_ID = 353,
     MON_GENERATION_ID = 354,
     MONTYPE_ID = 355,
     GRAVE_ID = 356,
     ERODEPROOF_ID = 357,
     FUNCTION_ID = 358,
     INCLUDE_ID = 359,
     SOUNDS_ID = 360,
     MSG_OUTPUT_TYPE = 361,
     WALLWALK_ID = 362,
     COMPARE_TYPE = 363,
     rect_ID = 364,
     fillrect_ID = 365,
     line_ID = 366,
     randline_ID = 367,
     grow_ID = 368,
     selection_ID = 369,
     flood_ID = 370,
     rndcoord_ID = 371,
     circle_ID = 372,
     ellipse_ID = 373,
     filter_ID = 374,
     STRING = 375,
     MAP_ID = 376,
     NQSTRING = 377,
     VARSTRING = 378,
     VARSTRING_INT = 379,
     VARSTRING_INT_ARRAY = 380,
     VARSTRING_STRING = 381,
     VARSTRING_STRING_ARRAY = 382,
     VARSTRING_VAR = 383,
     VARSTRING_VAR_ARRAY = 384,
     VARSTRING_COORD = 385,
     VARSTRING_COORD_ARRAY = 386,
     VARSTRING_REGION = 387,
     VARSTRING_REGION_ARRAY = 388,
     VARSTRING_MAPCHAR = 389,
     VARSTRING_MAPCHAR_ARRAY = 390,
     VARSTRING_MONST = 391,
     VARSTRING_MONST_ARRAY = 392,
     VARSTRING_OBJ = 393,
     VARSTRING_OBJ_ARRAY = 394,
     VARSTRING_SEL = 395,
     VARSTRING_SEL_ARRAY = 396,
     DICE = 397
   };
#endif
/* Tokens.  */
#define CHAR 258
#define INTEGER 259
#define BOOLEAN 260
#define PERCENT 261
#define SPERCENT 262
#define MINUS_INTEGER 263
#define PLUS_INTEGER 264
#define MAZE_GRID_ID 265
#define SOLID_FILL_ID 266
#define MINES_ID 267
#define MESSAGE_ID 268
#define LEVEL_ID 269
#define LEV_INIT_ID 270
#define GEOMETRY_ID 271
#define NOMAP_ID 272
#define OBJECT_ID 273
#define COBJECT_ID 274
#define MONSTER_ID 275
#define TRAP_ID 276
#define DOOR_ID 277
#define DRAWBRIDGE_ID 278
#define MAZEWALK_ID 279
#define WALLIFY_ID 280
#define REGION_ID 281
#define FILLING 282
#define ALTAR_ID 283
#define LADDER_ID 284
#define STAIR_ID 285
#define NON_DIGGABLE_ID 286
#define NON_PASSWALL_ID 287
#define ROOM_ID 288
#define PORTAL_ID 289
#define TELEPRT_ID 290
#define BRANCH_ID 291
#define LEV 292
#define CHANCE_ID 293
#define CORRIDOR_ID 294
#define GOLD_ID 295
#define ENGRAVING_ID 296
#define FOUNTAIN_ID 297
#define POOL_ID 298
#define SINK_ID 299
#define NONE 300
#define RAND_CORRIDOR_ID 301
#define DOOR_STATE 302
#define LIGHT_STATE 303
#define CURSE_TYPE 304
#define ENGRAVING_TYPE 305
#define DIRECTION 306
#define RANDOM_TYPE 307
#define A_REGISTER 308
#define ALIGNMENT 309
#define LEFT_OR_RIGHT 310
#define CENTER 311
#define TOP_OR_BOT 312
#define ALTAR_TYPE 313
#define UP_OR_DOWN 314
#define SUBROOM_ID 315
#define NAME_ID 316
#define FLAGS_ID 317
#define FLAG_TYPE 318
#define MON_ATTITUDE 319
#define MON_ALERTNESS 320
#define MON_APPEARANCE 321
#define ROOMDOOR_ID 322
#define IF_ID 323
#define ELSE_ID 324
#define SPILL_ID 325
#define TERRAIN_ID 326
#define HORIZ_OR_VERT 327
#define REPLACE_TERRAIN_ID 328
#define EXIT_ID 329
#define SHUFFLE_ID 330
#define QUANTITY_ID 331
#define BURIED_ID 332
#define LOOP_ID 333
#define SWITCH_ID 334
#define CASE_ID 335
#define BREAK_ID 336
#define DEFAULT_ID 337
#define ERODED_ID 338
#define TRAPPED_ID 339
#define RECHARGED_ID 340
#define INVIS_ID 341
#define GREASED_ID 342
#define FEMALE_ID 343
#define CANCELLED_ID 344
#define REVIVED_ID 345
#define AVENGE_ID 346
#define FLEEING_ID 347
#define BLINDED_ID 348
#define PARALYZED_ID 349
#define STUNNED_ID 350
#define CONFUSED_ID 351
#define SEENTRAPS_ID 352
#define ALL_ID 353
#define MON_GENERATION_ID 354
#define MONTYPE_ID 355
#define GRAVE_ID 356
#define ERODEPROOF_ID 357
#define FUNCTION_ID 358
#define INCLUDE_ID 359
#define SOUNDS_ID 360
#define MSG_OUTPUT_TYPE 361
#define WALLWALK_ID 362
#define COMPARE_TYPE 363
#define rect_ID 364
#define fillrect_ID 365
#define line_ID 366
#define randline_ID 367
#define grow_ID 368
#define selection_ID 369
#define flood_ID 370
#define rndcoord_ID 371
#define circle_ID 372
#define ellipse_ID 373
#define filter_ID 374
#define STRING 375
#define MAP_ID 376
#define NQSTRING 377
#define VARSTRING 378
#define VARSTRING_INT 379
#define VARSTRING_INT_ARRAY 380
#define VARSTRING_STRING 381
#define VARSTRING_STRING_ARRAY 382
#define VARSTRING_VAR 383
#define VARSTRING_VAR_ARRAY 384
#define VARSTRING_COORD 385
#define VARSTRING_COORD_ARRAY 386
#define VARSTRING_REGION 387
#define VARSTRING_REGION_ARRAY 388
#define VARSTRING_MAPCHAR 389
#define VARSTRING_MAPCHAR_ARRAY 390
#define VARSTRING_MONST 391
#define VARSTRING_MONST_ARRAY 392
#define VARSTRING_OBJ 393
#define VARSTRING_OBJ_ARRAY 394
#define VARSTRING_SEL 395
#define VARSTRING_SEL_ARRAY 396
#define DICE 397




/* Copy the first part of user declarations.  */
#line 1 "lev_comp.y"

/*	SCCS Id: @(#)lev_yacc.c	3.4	2000/01/17	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the Level Compiler code
 * It may handle special mazes & special room-levels
 */

/* In case we're using bison in AIX.  This definition must be
 * placed before any other C-language construct in the file
 * excluding comments and preprocessor directives (thanks IBM
 * for this wonderful feature...).
 *
 * Note: some cpps barf on this 'undefined control' (#pragma).
 * Addition of the leading space seems to prevent barfage for now,
 * and AIX will still see the directive.
 */
#ifdef _AIX
 #pragma alloca		/* keep leading space! */
#endif

#include "hack.h"
#include "sp_lev.h"

#define ERR		(-1)
/* many types of things are put in chars for transference to NetHack.
 * since some systems will use signed chars, limit everybody to the
 * same number for portability.
 */
#define MAX_OF_TYPE	128

#define MAX_NESTED_IFS	20
#define MAX_SWITCH_CASES 20
#define MAX_SWITCH_BREAKS 20

#define New(type)		\
	(type *) memset((genericptr_t)alloc(sizeof(type)), 0, sizeof(type))
#define NewTab(type, size)	(type **) alloc(sizeof(type *) * size)
#define Free(ptr)		free((genericptr_t)ptr)

extern void VDECL(lc_error, (const char *, ...));
extern void VDECL(lc_warning, (const char *, ...));
extern void FDECL(yyerror, (const char *));
extern void FDECL(yywarning, (const char *));
extern int NDECL(yylex);
int NDECL(yyparse);
 extern void FDECL(include_push, (const char *));
extern int NDECL(include_pop);

extern int FDECL(get_floor_type, (CHAR_P));
extern int FDECL(get_room_type, (char *));
extern int FDECL(get_trap_type, (char *));
extern int FDECL(get_monster_id, (char *,CHAR_P));
extern int FDECL(get_object_id, (char *,CHAR_P));
extern boolean FDECL(check_monster_char, (CHAR_P));
extern boolean FDECL(check_object_char, (CHAR_P));
extern char FDECL(what_map_char, (CHAR_P));
extern void FDECL(scan_map, (char *, sp_lev *));
extern void FDECL(add_opcode, (sp_lev *, int, genericptr_t));
extern genericptr_t FDECL(get_last_opcode_data1, (sp_lev *, int));
extern genericptr_t FDECL(get_last_opcode_data2, (sp_lev *, int,int));
extern boolean FDECL(check_subrooms, (sp_lev *));
extern boolean FDECL(write_level_file, (char *,sp_lev *));
extern struct opvar *FDECL(set_opvar_int, (struct opvar *, long));
extern void VDECL(add_opvars, (sp_lev *, const char *, ...));

extern struct lc_funcdefs *FDECL(funcdef_new,(long,char *));
extern void FDECL(funcdef_free_all,(struct lc_funcdefs *));
extern struct lc_funcdefs *FDECL(funcdef_defined,(struct lc_funcdefs *,char *, int));

extern struct lc_vardefs *FDECL(vardef_new,(long,char *));
extern void FDECL(vardef_free_all,(struct lc_vardefs *));
extern struct lc_vardefs *FDECL(vardef_defined,(struct lc_vardefs *,char *, int));

extern void FDECL(splev_add_from, (sp_lev *, sp_lev *));

extern void FDECL(check_vardef_type, (struct lc_vardefs *, char *, long));
extern struct lc_vardefs *FDECL(add_vardef_type, (struct lc_vardefs *, char *, long));

extern int FDECL(reverse_jmp_opcode, (int));


struct coord {
	long x;
	long y;
};

sp_lev *splev = NULL;

static struct opvar *if_list[MAX_NESTED_IFS];

static short n_if_list = 0;

unsigned int max_x_map, max_y_map;
int obj_containment = 0;

int in_container_obj = 0;

int in_switch_statement = 0;
static struct opvar *switch_check_jump = NULL;
static struct opvar *switch_default_case = NULL;
static struct opvar *switch_case_list[MAX_SWITCH_CASES];
static long switch_case_value[MAX_SWITCH_CASES];
int n_switch_case_list = 0;
static struct opvar *switch_break_list[MAX_SWITCH_BREAKS];
int n_switch_break_list = 0;


extern struct lc_vardefs *variable_definitions;


static struct lc_funcdefs *function_definitions = NULL;
int in_function_definition = 0;
sp_lev *function_splev_backup = NULL;

extern int fatal_error;
extern int got_errors;
extern int line_number;
extern const char *fname;



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 126 "lev_comp.y"
{
	long	i;
	char*	map;
	struct {
		long room;
		long wall;
		long door;
	} corpos;
    struct {
	long area;
	long x1;
	long y1;
	long x2;
	long y2;
    } lregn;
    struct {
	long x;
	long y;
    } crd;
    struct {
	long ter;
	long lit;
    } terr;
    struct {
	long height;
	long width;
    } sze;
    struct {
	long die;
	long num;
    } dice;
}
/* Line 187 of yacc.c.  */
#line 537 "y.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 550 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  10
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1063

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  160
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  151
/* YYNRULES -- Number of rules.  */
#define YYNRULES  391
/* YYNRULES -- Number of states.  */
#define YYNSTATES  870

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   397

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   155,   159,     2,
     122,   123,   153,   151,   120,   152,   158,   154,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   121,     2,
       2,   156,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   124,     2,   125,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   126,   157,   127,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,     9,    12,    14,    16,    19,
      21,    24,    28,    32,    38,    44,    61,    62,    65,    67,
      69,    70,    74,    78,    80,    81,    84,    86,    88,    90,
      92,    94,    96,    98,   100,   102,   104,   106,   108,   110,
     112,   114,   116,   118,   120,   122,   124,   126,   128,   130,
     132,   134,   136,   138,   140,   142,   144,   146,   148,   150,
     152,   154,   156,   158,   160,   162,   164,   166,   168,   170,
     172,   174,   176,   178,   180,   182,   184,   186,   188,   190,
     192,   194,   196,   198,   200,   202,   204,   206,   208,   210,
     212,   214,   218,   222,   228,   232,   236,   242,   248,   254,
     258,   262,   268,   274,   280,   288,   296,   304,   310,   312,
     316,   318,   322,   324,   328,   330,   334,   336,   340,   342,
     346,   348,   352,   353,   362,   366,   368,   369,   371,   372,
     375,   377,   383,   384,   393,   394,   397,   398,   404,   405,
     410,   411,   414,   416,   418,   419,   428,   429,   434,   438,
     439,   448,   452,   459,   468,   470,   474,   478,   484,   490,
     498,   503,   504,   517,   518,   533,   534,   537,   543,   545,
     551,   553,   559,   561,   567,   569,   579,   585,   587,   589,
     591,   593,   595,   599,   601,   603,   605,   609,   615,   621,
     623,   625,   627,   629,   635,   637,   641,   647,   653,   655,
     659,   665,   671,   676,   677,   686,   691,   692,   696,   698,
     700,   702,   704,   707,   709,   711,   713,   715,   717,   721,
     725,   729,   731,   733,   737,   739,   741,   745,   750,   751,
     760,   763,   764,   768,   770,   774,   776,   780,   784,   786,
     788,   792,   794,   796,   798,   802,   804,   806,   808,   815,
     823,   829,   838,   840,   844,   850,   856,   864,   872,   879,
     885,   886,   889,   893,   897,   901,   903,   909,   919,   926,
     936,   940,   944,   945,   956,   957,   961,   969,   975,   981,
     985,   991,   999,  1001,  1003,  1005,  1007,  1008,  1011,  1016,
    1018,  1020,  1022,  1024,  1026,  1028,  1030,  1032,  1034,  1038,
    1040,  1042,  1047,  1049,  1051,  1053,  1058,  1060,  1062,  1067,
    1069,  1074,  1080,  1082,  1084,  1086,  1091,  1101,  1103,  1105,
    1110,  1112,  1118,  1120,  1122,  1127,  1129,  1131,  1137,  1139,
    1141,  1143,  1148,  1150,  1152,  1158,  1160,  1162,  1166,  1168,
    1170,  1174,  1176,  1181,  1185,  1189,  1193,  1197,  1201,  1205,
    1207,  1209,  1213,  1217,  1221,  1225,  1229,  1233,  1237,  1239,
    1242,  1245,  1250,  1257,  1262,  1269,  1276,  1283,  1286,  1293,
    1302,  1311,  1322,  1324,  1328,  1330,  1334,  1336,  1338,  1340,
    1342,  1344,  1346,  1348,  1350,  1352,  1353,  1355,  1357,  1359,
    1361,  1372
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     161,     0,    -1,   162,    -1,   162,   165,    -1,    -1,   163,
     162,    -1,   187,    -1,   164,    -1,   104,   128,    -1,   166,
      -1,   166,   165,    -1,   167,   171,   173,    -1,    14,   121,
     306,    -1,    15,   121,    11,   120,   263,    -1,    15,   121,
      10,   120,     3,    -1,    15,   121,    12,   120,     3,   120,
       3,   120,     5,   120,     5,   120,   280,   120,   170,   169,
      -1,    -1,   120,     3,    -1,     5,    -1,    52,    -1,    -1,
      62,   121,   172,    -1,    63,   120,   172,    -1,    63,    -1,
      -1,   174,   173,    -1,   208,    -1,   168,    -1,   272,    -1,
     273,    -1,   235,    -1,   232,    -1,   258,    -1,   211,    -1,
     179,    -1,   178,    -1,   267,    -1,   223,    -1,   209,    -1,
     250,    -1,   275,    -1,   260,    -1,   274,    -1,   194,    -1,
     202,    -1,   204,    -1,   190,    -1,   187,    -1,   189,    -1,
     253,    -1,   228,    -1,   251,    -1,   238,    -1,   244,    -1,
     268,    -1,   262,    -1,   256,    -1,   210,    -1,   269,    -1,
     216,    -1,   214,    -1,   261,    -1,   265,    -1,   264,    -1,
     266,    -1,   254,    -1,   255,    -1,   257,    -1,   249,    -1,
     252,    -1,   133,    -1,   135,    -1,   137,    -1,   139,    -1,
     141,    -1,   143,    -1,   145,    -1,   147,    -1,   149,    -1,
     132,    -1,   134,    -1,   136,    -1,   138,    -1,   140,    -1,
     142,    -1,   144,    -1,   146,    -1,   148,    -1,   175,    -1,
     176,    -1,   131,    -1,    75,   121,   175,    -1,   177,   156,
     300,    -1,   177,   156,   114,   121,   302,    -1,   177,   156,
     128,    -1,   177,   156,   177,    -1,   177,   156,    71,   121,
     293,    -1,   177,   156,    20,   121,   295,    -1,   177,   156,
      18,   121,   297,    -1,   177,   156,   289,    -1,   177,   156,
     291,    -1,   177,   156,   126,   185,   127,    -1,   177,   156,
     126,   184,   127,    -1,   177,   156,   126,   183,   127,    -1,
     177,   156,    71,   121,   126,   182,   127,    -1,   177,   156,
      20,   121,   126,   181,   127,    -1,   177,   156,    18,   121,
     126,   180,   127,    -1,   177,   156,   126,   186,   127,    -1,
     297,    -1,   180,   120,   297,    -1,   295,    -1,   181,   120,
     295,    -1,   293,    -1,   182,   120,   293,    -1,   291,    -1,
     183,   120,   291,    -1,   289,    -1,   184,   120,   289,    -1,
     300,    -1,   185,   120,   300,    -1,   128,    -1,   186,   120,
     128,    -1,    -1,   103,   130,   122,   123,   188,   126,   173,
     127,    -1,   130,   122,   123,    -1,    74,    -1,    -1,     6,
      -1,    -1,   120,     7,    -1,     6,    -1,   124,   299,   108,
     299,   125,    -1,    -1,    79,   124,   287,   125,   195,   126,
     196,   127,    -1,    -1,   197,   196,    -1,    -1,    80,   304,
     121,   198,   200,    -1,    -1,    82,   121,   199,   200,    -1,
      -1,   201,   200,    -1,    81,    -1,   174,    -1,    -1,    78,
     124,   287,   125,   203,   126,   173,   127,    -1,    -1,    68,
     193,   205,   206,    -1,   126,   173,   127,    -1,    -1,   126,
     173,   127,   207,    69,   126,   173,   127,    -1,    13,   121,
     298,    -1,   107,   121,   288,   120,   292,   192,    -1,   107,
     121,   288,   120,   292,   120,   292,   192,    -1,    46,    -1,
      46,   121,   304,    -1,    46,   121,    52,    -1,    39,   121,
     212,   120,   212,    -1,    39,   121,   212,   120,   304,    -1,
     122,     4,   120,    51,   120,   227,   123,    -1,   277,   191,
     120,   280,    -1,    -1,    60,   121,   213,   120,   220,   120,
     222,   218,   215,   126,   173,   127,    -1,    -1,    33,   121,
     213,   120,   219,   120,   221,   120,   222,   218,   217,   126,
     173,   127,    -1,    -1,   120,     5,    -1,   122,     4,   120,
       4,   123,    -1,    52,    -1,   122,     4,   120,     4,   123,
      -1,    52,    -1,   122,   230,   120,   231,   123,    -1,    52,
      -1,   122,     4,   120,     4,   123,    -1,    52,    -1,    67,
     121,   224,   120,   279,   120,   225,   120,   227,    -1,    22,
     121,   279,   120,   302,    -1,     5,    -1,    52,    -1,   226,
      -1,    52,    -1,    51,    -1,    51,   157,   226,    -1,     4,
      -1,    52,    -1,    17,    -1,   229,   218,   129,    -1,    16,
     121,   288,   218,   129,    -1,    16,   121,   230,   120,   231,
      -1,    55,    -1,    56,    -1,    57,    -1,    56,    -1,   105,
     121,   287,   120,   233,    -1,   234,    -1,   234,   120,   233,
      -1,   122,   106,   120,   298,   123,    -1,    99,   121,     7,
     120,   236,    -1,   237,    -1,   237,   120,   236,    -1,   122,
     287,   120,   285,   123,    -1,   122,   287,   120,   306,   123,
      -1,    20,   307,   121,   240,    -1,    -1,    20,   307,   121,
     240,   239,   126,   173,   127,    -1,   294,   120,   288,   241,
      -1,    -1,   241,   120,   242,    -1,   298,    -1,    64,    -1,
      65,    -1,   282,    -1,    66,   298,    -1,    88,    -1,    86,
      -1,    89,    -1,    90,    -1,    91,    -1,    92,   121,   287,
      -1,    93,   121,   287,    -1,    94,   121,   287,    -1,    95,
      -1,    96,    -1,    97,   121,   243,    -1,   128,    -1,    98,
      -1,   128,   157,   243,    -1,    18,   307,   121,   246,    -1,
      -1,    19,   307,   121,   246,   245,   126,   173,   127,    -1,
     296,   247,    -1,    -1,   247,   120,   248,    -1,    49,    -1,
     100,   121,   294,    -1,   305,    -1,    61,   121,   298,    -1,
      76,   121,   287,    -1,    77,    -1,    48,    -1,    83,   121,
     287,    -1,   102,    -1,    47,    -1,    84,    -1,    85,   121,
     287,    -1,    86,    -1,    87,    -1,   288,    -1,    21,   307,
     121,   276,   120,   288,    -1,    23,   121,   288,   120,    51,
     120,   279,    -1,    24,   121,   288,   120,    51,    -1,    24,
     121,   288,   120,    51,   120,     5,   169,    -1,    25,    -1,
      25,   121,   290,    -1,    29,   121,   288,   120,    59,    -1,
      30,   121,   288,   120,    59,    -1,    30,   121,   309,   120,
     309,   120,    59,    -1,    34,   121,   309,   120,   309,   120,
     306,    -1,    35,   121,   309,   120,   309,   259,    -1,    36,
     121,   309,   120,   309,    -1,    -1,   120,    59,    -1,    42,
     121,   302,    -1,    44,   121,   302,    -1,    43,   121,   302,
      -1,     3,    -1,   122,     3,   120,   280,   123,    -1,    73,
     121,   290,   120,   292,   120,   292,   120,     7,    -1,    71,
     307,   121,   302,   120,   292,    -1,    70,   121,   288,   120,
     263,   120,    51,   120,     4,    -1,    31,   121,   290,    -1,
      32,   121,   290,    -1,    -1,    26,   121,   290,   120,   280,
     120,   277,   278,   270,   271,    -1,    -1,   126,   173,   127,
      -1,    28,   121,   288,   120,   281,   120,   283,    -1,   101,
     121,   288,   120,   298,    -1,   101,   121,   288,   120,    52,
      -1,   101,   121,   288,    -1,    40,   121,   300,   120,   288,
      -1,    41,   121,   288,   120,   308,   120,   298,    -1,   306,
      -1,    52,    -1,   306,    -1,    52,    -1,    -1,   120,    27,
      -1,   120,    27,   120,     5,    -1,    47,    -1,    52,    -1,
      48,    -1,    52,    -1,    54,    -1,   284,    -1,    52,    -1,
      54,    -1,   284,    -1,    53,   121,    52,    -1,    58,    -1,
      52,    -1,    53,   124,     4,   125,    -1,     3,    -1,   128,
      -1,   134,    -1,   135,   124,   300,   125,    -1,   299,    -1,
     289,    -1,   116,   122,   302,   123,    -1,   138,    -1,   139,
     124,   300,   125,    -1,   122,     4,   120,     4,   123,    -1,
      52,    -1,   291,    -1,   140,    -1,   141,   124,   300,   125,
      -1,   122,     4,   120,     4,   120,     4,   120,     4,   123,
      -1,   293,    -1,   142,    -1,   143,   124,   300,   125,    -1,
       3,    -1,   122,     3,   120,   280,   123,    -1,   295,    -1,
     144,    -1,   145,   124,   300,   125,    -1,   128,    -1,     3,
      -1,   122,     3,   120,   128,   123,    -1,    52,    -1,   297,
      -1,   146,    -1,   147,   124,   300,   125,    -1,   128,    -1,
       3,    -1,   122,     3,   120,   128,   123,    -1,    52,    -1,
     286,    -1,   298,   158,   286,    -1,     4,    -1,   303,    -1,
     122,     8,   123,    -1,   132,    -1,   133,   124,   300,   125,
      -1,   299,   151,   299,    -1,   299,   152,   299,    -1,   299,
     153,   299,    -1,   299,   154,   299,    -1,   299,   155,   299,
      -1,   122,   300,   123,    -1,     4,    -1,   303,    -1,   122,
       8,   123,    -1,   300,   151,   300,    -1,   300,   152,   300,
      -1,   300,   153,   300,    -1,   300,   154,   300,    -1,   300,
     155,   300,    -1,   122,   300,   123,    -1,   288,    -1,   109,
     290,    -1,   110,   290,    -1,   111,   288,   152,   288,    -1,
     112,   288,   152,   288,   120,   300,    -1,   113,   122,   302,
     123,    -1,   113,   122,   226,   120,   302,   123,    -1,   119,
     122,     7,   120,   302,   123,    -1,   119,   122,   302,   120,
     302,   123,    -1,   115,   288,    -1,   117,   122,   288,   120,
     300,   123,    -1,   117,   122,   288,   120,   300,   120,    27,
     123,    -1,   118,   122,   288,   120,   300,   120,   300,   123,
      -1,   118,   122,   288,   120,   300,   120,   300,   120,    27,
     123,    -1,   148,    -1,   122,   302,   123,    -1,   301,    -1,
     301,   159,   302,    -1,   150,    -1,     8,    -1,     9,    -1,
       4,    -1,     8,    -1,     9,    -1,     4,    -1,   303,    -1,
     128,    -1,    -1,   193,    -1,    50,    -1,    52,    -1,   310,
      -1,    37,   122,     4,   120,     4,   120,     4,   120,     4,
     123,    -1,   122,     4,   120,     4,   120,     4,   120,     4,
     123,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   230,   230,   231,   234,   235,   238,   239,   243,   250,
     251,   254,   276,   299,   308,   317,   343,   346,   353,   354,
     358,   361,   367,   371,   378,   381,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   408,   409,   410,
     411,   412,   413,   414,   415,   416,   417,   418,   419,   420,
     421,   422,   423,   424,   425,   426,   427,   428,   429,   430,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   455,   456,
     457,   460,   472,   478,   484,   491,   512,   518,   524,   530,
     536,   542,   549,   556,   563,   570,   577,   584,   593,   598,
     605,   610,   617,   622,   629,   634,   641,   646,   653,   657,
     663,   669,   678,   677,   704,   731,   738,   741,   749,   752,
     759,   765,   772,   771,   829,   830,   834,   833,   846,   845,
     860,   861,   864,   875,   881,   880,   908,   907,   932,   941,
     940,   967,   973,   977,   983,   987,   991,   997,  1004,  1013,
    1021,  1032,  1031,  1043,  1042,  1055,  1058,  1064,  1074,  1080,
    1089,  1095,  1100,  1106,  1111,  1117,  1126,  1132,  1133,  1136,
    1137,  1140,  1144,  1150,  1151,  1154,  1160,  1166,  1174,  1180,
    1181,  1184,  1185,  1188,  1195,  1199,  1205,  1211,  1223,  1227,
    1233,  1239,  1248,  1261,  1260,  1280,  1287,  1293,  1301,  1306,
    1311,  1316,  1321,  1326,  1331,  1336,  1341,  1346,  1351,  1356,
    1361,  1366,  1371,  1376,  1383,  1390,  1394,  1407,  1421,  1420,
    1443,  1451,  1457,  1465,  1470,  1475,  1480,  1485,  1490,  1495,
    1500,  1505,  1510,  1521,  1526,  1531,  1536,  1541,  1548,  1561,
    1586,  1591,  1598,  1602,  1608,  1614,  1620,  1630,  1640,  1655,
    1665,  1668,  1674,  1680,  1686,  1692,  1697,  1704,  1715,  1729,
    1745,  1751,  1758,  1757,  1777,  1780,  1786,  1792,  1796,  1801,
    1808,  1814,  1821,  1829,  1832,  1842,  1846,  1849,  1853,  1859,
    1860,  1863,  1864,  1867,  1868,  1869,  1875,  1876,  1877,  1883,
    1884,  1887,  1896,  1907,  1912,  1918,  1927,  1933,  1937,  1941,
    1947,  1955,  1961,  1967,  1971,  1977,  1985,  1994,  1998,  2004,
    2012,  2021,  2032,  2036,  2042,  2050,  2059,  2068,  2077,  2083,
    2087,  2093,  2101,  2111,  2120,  2129,  2136,  2137,  2143,  2144,
    2145,  2146,  2152,  2158,  2159,  2160,  2161,  2162,  2163,  2166,
    2167,  2168,  2169,  2170,  2171,  2172,  2173,  2174,  2177,  2181,
    2185,  2189,  2193,  2198,  2202,  2206,  2210,  2214,  2218,  2222,
    2226,  2230,  2234,  2240,  2246,  2250,  2256,  2262,  2263,  2264,
    2267,  2271,  2275,  2279,  2285,  2289,  2293,  2310,  2311,  2314,
    2318,  2336
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "CHAR", "INTEGER", "BOOLEAN", "PERCENT",
  "SPERCENT", "MINUS_INTEGER", "PLUS_INTEGER", "MAZE_GRID_ID",
  "SOLID_FILL_ID", "MINES_ID", "MESSAGE_ID", "LEVEL_ID", "LEV_INIT_ID",
  "GEOMETRY_ID", "NOMAP_ID", "OBJECT_ID", "COBJECT_ID", "MONSTER_ID",
  "TRAP_ID", "DOOR_ID", "DRAWBRIDGE_ID", "MAZEWALK_ID", "WALLIFY_ID",
  "REGION_ID", "FILLING", "ALTAR_ID", "LADDER_ID", "STAIR_ID",
  "NON_DIGGABLE_ID", "NON_PASSWALL_ID", "ROOM_ID", "PORTAL_ID",
  "TELEPRT_ID", "BRANCH_ID", "LEV", "CHANCE_ID", "CORRIDOR_ID", "GOLD_ID",
  "ENGRAVING_ID", "FOUNTAIN_ID", "POOL_ID", "SINK_ID", "NONE",
  "RAND_CORRIDOR_ID", "DOOR_STATE", "LIGHT_STATE", "CURSE_TYPE",
  "ENGRAVING_TYPE", "DIRECTION", "RANDOM_TYPE", "A_REGISTER", "ALIGNMENT",
  "LEFT_OR_RIGHT", "CENTER", "TOP_OR_BOT", "ALTAR_TYPE", "UP_OR_DOWN",
  "SUBROOM_ID", "NAME_ID", "FLAGS_ID", "FLAG_TYPE", "MON_ATTITUDE",
  "MON_ALERTNESS", "MON_APPEARANCE", "ROOMDOOR_ID", "IF_ID", "ELSE_ID",
  "SPILL_ID", "TERRAIN_ID", "HORIZ_OR_VERT", "REPLACE_TERRAIN_ID",
  "EXIT_ID", "SHUFFLE_ID", "QUANTITY_ID", "BURIED_ID", "LOOP_ID",
  "SWITCH_ID", "CASE_ID", "BREAK_ID", "DEFAULT_ID", "ERODED_ID",
  "TRAPPED_ID", "RECHARGED_ID", "INVIS_ID", "GREASED_ID", "FEMALE_ID",
  "CANCELLED_ID", "REVIVED_ID", "AVENGE_ID", "FLEEING_ID", "BLINDED_ID",
  "PARALYZED_ID", "STUNNED_ID", "CONFUSED_ID", "SEENTRAPS_ID", "ALL_ID",
  "MON_GENERATION_ID", "MONTYPE_ID", "GRAVE_ID", "ERODEPROOF_ID",
  "FUNCTION_ID", "INCLUDE_ID", "SOUNDS_ID", "MSG_OUTPUT_TYPE",
  "WALLWALK_ID", "COMPARE_TYPE", "rect_ID", "fillrect_ID", "line_ID",
  "randline_ID", "grow_ID", "selection_ID", "flood_ID", "rndcoord_ID",
  "circle_ID", "ellipse_ID", "filter_ID", "','", "':'", "'('", "')'",
  "'['", "']'", "'{'", "'}'", "STRING", "MAP_ID", "NQSTRING", "VARSTRING",
  "VARSTRING_INT", "VARSTRING_INT_ARRAY", "VARSTRING_STRING",
  "VARSTRING_STRING_ARRAY", "VARSTRING_VAR", "VARSTRING_VAR_ARRAY",
  "VARSTRING_COORD", "VARSTRING_COORD_ARRAY", "VARSTRING_REGION",
  "VARSTRING_REGION_ARRAY", "VARSTRING_MAPCHAR", "VARSTRING_MAPCHAR_ARRAY",
  "VARSTRING_MONST", "VARSTRING_MONST_ARRAY", "VARSTRING_OBJ",
  "VARSTRING_OBJ_ARRAY", "VARSTRING_SEL", "VARSTRING_SEL_ARRAY", "DICE",
  "'+'", "'-'", "'*'", "'/'", "'%'", "'='", "'|'", "'.'", "'&'", "$accept",
  "file", "header_stmts", "header_stmt", "include_def", "levels", "level",
  "level_def", "lev_init", "opt_fillchar", "walled", "flags", "flag_list",
  "levstatements", "levstatement", "any_var_array", "any_var",
  "any_var_or_arr", "shuffle_detail", "variable_define", "encodeobj_list",
  "encodemonster_list", "mapchar_list", "encoderegion_list",
  "encodecoord_list", "integer_list", "string_list", "function_define",
  "@1", "function_call", "exitstatement", "opt_percent", "opt_spercent",
  "comparestmt", "switchstatement", "@2", "switchcases", "switchcase",
  "@3", "@4", "breakstatements", "breakstatement", "loopstatement", "@5",
  "ifstatement", "@6", "if_ending", "@7", "message", "wallwalk_detail",
  "random_corridors", "corridor", "corr_spec", "room_begin", "subroom_def",
  "@8", "room_def", "@9", "roomfill", "room_pos", "subroom_pos",
  "room_align", "room_size", "door_detail", "secret", "door_wall",
  "dir_list", "door_pos", "map_definition", "map_geometry", "h_justif",
  "v_justif", "sounds_detail", "sounds_list", "lvl_sound_part",
  "mon_generation", "mon_gen_list", "mon_gen_part", "monster_detail",
  "@10", "monster_desc", "monster_infos", "monster_info", "seen_trap_mask",
  "object_detail", "@11", "object_desc", "object_infos", "object_info",
  "trap_detail", "drawbridge_detail", "mazewalk_detail", "wallify_detail",
  "ladder_detail", "stair_detail", "stair_region", "portal_region",
  "teleprt_region", "branch_region", "teleprt_detail", "fountain_detail",
  "sink_detail", "pool_detail", "terrain_type", "replace_terrain_detail",
  "terrain_detail", "spill_detail", "diggable_detail", "passwall_detail",
  "region_detail", "@12", "region_detail_end", "altar_detail",
  "grave_detail", "gold_detail", "engraving_detail", "trap_name",
  "room_type", "prefilled", "door_state", "light_state", "alignment",
  "alignment_prfx", "altar_type", "a_register", "monster", "string_or_var",
  "integer_or_var", "coord_or_var", "encodecoord", "region_or_var",
  "encoderegion", "mapchar_or_var", "mapchar", "monster_or_var",
  "encodemonster", "object_or_var", "encodeobj", "string_expr",
  "math_expr_var", "math_expr", "ter_selection_x", "ter_selection", "dice",
  "all_integers", "all_ints_push", "string", "chance", "engraving_type",
  "lev_region", "region", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
      44,    58,    40,    41,    91,    93,   123,   125,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,   389,   390,   391,   392,   393,   394,   395,   396,
     397,    43,    45,    42,    47,    37,    61,   124,    46,    38
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   160,   161,   161,   162,   162,   163,   163,   164,   165,
     165,   166,   167,   168,   168,   168,   169,   169,   170,   170,
     171,   171,   172,   172,   173,   173,   174,   174,   174,   174,
     174,   174,   174,   174,   174,   174,   174,   174,   174,   174,
     174,   174,   174,   174,   174,   174,   174,   174,   174,   174,
     174,   174,   174,   174,   174,   174,   174,   174,   174,   174,
     174,   174,   174,   174,   174,   174,   174,   174,   174,   174,
     175,   175,   175,   175,   175,   175,   175,   175,   175,   176,
     176,   176,   176,   176,   176,   176,   176,   176,   177,   177,
     177,   178,   179,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   180,   180,
     181,   181,   182,   182,   183,   183,   184,   184,   185,   185,
     186,   186,   188,   187,   189,   190,   191,   191,   192,   192,
     193,   193,   195,   194,   196,   196,   198,   197,   199,   197,
     200,   200,   201,   201,   203,   202,   205,   204,   206,   207,
     206,   208,   209,   209,   210,   210,   210,   211,   211,   212,
     213,   215,   214,   217,   216,   218,   218,   219,   219,   220,
     220,   221,   221,   222,   222,   223,   223,   224,   224,   225,
     225,   226,   226,   227,   227,   228,   228,   228,   229,   230,
     230,   231,   231,   232,   233,   233,   234,   235,   236,   236,
     237,   237,   238,   239,   238,   240,   241,   241,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   242,   243,   243,   243,   244,   245,   244,
     246,   247,   247,   248,   248,   248,   248,   248,   248,   248,
     248,   248,   248,   248,   248,   248,   248,   248,   249,   250,
     251,   251,   252,   252,   253,   254,   255,   256,   257,   258,
     259,   259,   260,   261,   262,   263,   263,   264,   265,   266,
     267,   268,   270,   269,   271,   271,   272,   273,   273,   273,
     274,   275,   276,   276,   277,   277,   278,   278,   278,   279,
     279,   280,   280,   281,   281,   281,   282,   282,   282,   283,
     283,   284,   285,   286,   286,   286,   287,   288,   288,   288,
     288,   289,   289,   290,   290,   290,   291,   292,   292,   292,
     293,   293,   294,   294,   294,   295,   295,   295,   295,   296,
     296,   296,   297,   297,   297,   297,   298,   298,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   299,   299,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   301,   301,
     301,   301,   301,   301,   301,   301,   301,   301,   301,   301,
     301,   301,   301,   301,   302,   302,   303,   304,   304,   304,
     305,   305,   305,   305,   306,   307,   307,   308,   308,   309,
     309,   310
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     0,     2,     1,     1,     2,     1,
       2,     3,     3,     5,     5,    16,     0,     2,     1,     1,
       0,     3,     3,     1,     0,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     5,     3,     3,     5,     5,     5,     3,
       3,     5,     5,     5,     7,     7,     7,     5,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     0,     8,     3,     1,     0,     1,     0,     2,
       1,     5,     0,     8,     0,     2,     0,     5,     0,     4,
       0,     2,     1,     1,     0,     8,     0,     4,     3,     0,
       8,     3,     6,     8,     1,     3,     3,     5,     5,     7,
       4,     0,    12,     0,    14,     0,     2,     5,     1,     5,
       1,     5,     1,     5,     1,     9,     5,     1,     1,     1,
       1,     1,     3,     1,     1,     1,     3,     5,     5,     1,
       1,     1,     1,     5,     1,     3,     5,     5,     1,     3,
       5,     5,     4,     0,     8,     4,     0,     3,     1,     1,
       1,     1,     2,     1,     1,     1,     1,     1,     3,     3,
       3,     1,     1,     3,     1,     1,     3,     4,     0,     8,
       2,     0,     3,     1,     3,     1,     3,     3,     1,     1,
       3,     1,     1,     1,     3,     1,     1,     1,     6,     7,
       5,     8,     1,     3,     5,     5,     7,     7,     6,     5,
       0,     2,     3,     3,     3,     1,     5,     9,     6,     9,
       3,     3,     0,    10,     0,     3,     7,     5,     5,     3,
       5,     7,     1,     1,     1,     1,     0,     2,     4,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       1,     4,     1,     1,     1,     4,     1,     1,     4,     1,
       4,     5,     1,     1,     1,     4,     9,     1,     1,     4,
       1,     5,     1,     1,     4,     1,     1,     5,     1,     1,
       1,     4,     1,     1,     5,     1,     1,     3,     1,     1,
       3,     1,     4,     3,     3,     3,     3,     3,     3,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     1,     2,
       2,     4,     6,     4,     6,     6,     6,     2,     6,     8,
       8,    10,     1,     3,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     1,     1,     1,
      10,     9
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     0,     0,     2,     4,     7,     6,     0,     8,
       1,     0,     3,     9,    20,     5,     0,     0,    10,     0,
      24,   122,   384,    12,     0,     0,     0,     0,   185,   385,
     385,   385,   385,     0,     0,     0,   252,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   154,     0,     0,     0,     0,   385,     0,
     125,     0,     0,     0,     0,     0,     0,     0,     0,    90,
      79,    70,    80,    71,    81,    72,    82,    73,    83,    74,
      84,    75,    85,    76,    86,    77,    87,    78,    27,    11,
      24,    88,    89,     0,    35,    34,    47,    48,    46,    43,
      44,    45,    26,    38,    57,    33,    60,    59,    37,    50,
     165,    31,    30,    52,    53,    68,    39,    51,    69,    49,
      65,    66,    56,    67,    32,    41,    61,    55,    63,    62,
      64,    36,    54,    58,    28,    29,    42,    40,     0,    23,
      21,     0,     0,     0,   130,     0,   386,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   146,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    25,     0,     0,     0,
      24,     0,   303,   304,     0,   336,   151,     0,     0,     0,
     312,   189,   190,     0,     0,   309,     0,     0,   165,   307,
     338,     0,   341,     0,   376,     0,   339,     0,     0,     0,
       0,   289,   290,     0,     0,     0,     0,   314,     0,   253,
     313,     0,     0,     0,     0,     0,     0,     0,   389,   270,
     271,   285,     0,   126,   284,     0,     0,     0,     0,     0,
       0,   349,     0,     0,   350,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   372,   358,   374,   262,
     264,   263,   379,   377,   378,   156,   155,     0,   177,   178,
       0,     0,     0,     0,     0,    91,     0,   306,     0,     0,
     279,     0,     0,   124,     0,     0,     0,     0,     0,     0,
      94,    95,    99,   100,    92,   166,   186,     0,    22,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   333,   335,
       0,   332,   330,     0,   227,   231,   329,   228,   326,   328,
       0,   325,   323,     0,   202,     0,   322,   283,     0,   282,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   127,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     359,   360,     0,     0,     0,   367,     0,     0,     0,     0,
       0,     0,     0,    24,   147,     0,     0,     0,   144,   132,
       0,     0,     0,     0,     0,     0,     0,     0,   349,   120,
       0,     0,     0,     0,   116,   114,   118,   123,     0,   337,
      14,   265,     0,    13,     0,     0,     0,     0,   192,   191,
     188,   187,   340,   348,     0,     0,   343,   344,   345,   346,
     347,     0,     0,   230,     0,     0,     0,     0,     0,     0,
     176,     0,   250,     0,     0,   291,   292,     0,   295,     0,
     293,     0,   294,   254,     0,     0,   255,     0,   168,     0,
       0,     0,     0,     0,   260,   259,     0,   157,   158,   351,
     357,   280,   352,   353,   354,   355,   356,   387,   388,     0,
       0,     0,   181,     0,     0,     0,     0,     0,     0,   373,
     375,   170,     0,     0,     0,     0,     0,     0,   320,     0,
     318,     0,     0,   317,     0,     0,     0,   197,   198,   278,
     277,     0,   193,   194,   128,     0,    98,     0,    97,     0,
      96,    93,     0,     0,   103,     0,   102,     0,   101,     0,
     107,   305,     0,     0,   308,     0,   310,   342,   131,     0,
       0,     0,    24,     0,     0,    24,   206,   248,     0,     0,
       0,   315,     0,     0,     0,     0,     0,     0,     0,     0,
     160,     0,     0,     0,   258,     0,     0,   361,     0,     0,
       0,   363,     0,     0,     0,     0,     0,     0,     0,   148,
       0,   268,     0,     0,     0,    24,   134,     0,     0,     0,
       0,     0,   152,     0,   108,     0,   110,     0,   112,     0,
     115,   117,   119,   121,     0,     0,   311,     0,   331,   382,
     380,   381,   242,   239,   233,     0,     0,   238,     0,   243,
       0,   245,   246,     0,   241,   232,   247,   383,   235,     0,
       0,   324,     0,   205,   249,    16,     0,   286,     0,   300,
     299,   276,     0,     0,   256,     0,   172,     0,     0,   257,
     261,     0,   281,     0,   182,     0,     0,     0,     0,     0,
       0,   174,     0,   165,   180,     0,   179,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   134,     0,   199,     0,
     195,   129,   128,     0,   106,     0,   105,     0,   104,     0,
       0,   334,     0,     0,     0,     0,     0,   229,   327,   204,
       0,     0,   251,     0,     0,   272,   301,     0,     0,     0,
       0,     0,   183,   184,     0,   362,   364,     0,   368,     0,
     365,   366,     0,     0,   161,     0,     0,     0,     0,   319,
       0,   145,     0,   138,   133,   135,   302,     0,     0,     0,
       0,   153,   109,   111,   113,   266,     0,   236,   237,   240,
     244,   234,     0,   296,   209,   210,     0,   214,   213,   215,
     216,   217,     0,     0,     0,   221,   222,     0,   207,   211,
     297,   208,    17,     0,   287,   274,     0,     0,   167,     0,
     165,   159,     0,     0,   169,     0,     0,   175,    24,   269,
     321,   267,   136,   140,   200,   201,   196,     0,     0,   212,
       0,     0,     0,     0,     0,     0,    24,   273,     0,     0,
       0,   163,   369,     0,   370,     0,    24,     0,   140,   142,
     143,   139,   140,     0,   298,   218,   219,   220,   225,   224,
     223,   316,   288,     0,     0,   391,   171,     0,     0,   173,
       0,   150,   137,   141,     0,     0,   275,   390,    24,   371,
     162,     0,   226,     0,     0,   164,    18,    19,    16,    15
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,     5,     6,    12,    13,    14,    88,   712,
     868,    20,   140,    89,    90,    91,    92,    93,    94,    95,
     603,   605,   607,   410,   411,   412,   413,    96,   138,    97,
      98,   364,   602,   146,    99,   515,   685,   686,   828,   803,
     831,   832,   100,   514,   101,   281,   394,   677,   102,   103,
     104,   105,   250,   242,   106,   796,   107,   847,   189,   470,
     503,   658,   673,   108,   280,   675,   493,   724,   109,   110,
     207,   430,   111,   522,   523,   112,   517,   518,   113,   447,
     344,   643,   778,   840,   114,   444,   334,   443,   635,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   574,
     125,   126,   127,   423,   128,   129,   130,   131,   132,   133,
     785,   817,   134,   135,   136,   137,   348,   243,   715,   223,
     457,   461,   779,   651,   462,   747,   195,   286,   267,   209,
     229,   230,   512,   513,   345,   346,   335,   336,   196,   287,
     372,   268,   269,   254,   276,   638,   244,   147,   489,   237,
     238
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -734
static const yytype_int16 yypact[] =
{
     312,   -72,   -22,    68,   123,   312,  -734,  -734,     4,  -734,
    -734,    86,  -734,   123,   147,  -734,   151,   107,  -734,   120,
     797,  -734,  -734,  -734,   213,   172,   183,   187,  -734,    61,
      61,    61,    61,   201,   205,   207,   240,   244,   246,   256,
     276,   286,   305,   323,   334,   337,   338,   340,   342,   343,
     349,   350,   352,   354,   355,   376,    61,   377,    61,   379,
    -734,   382,   234,   245,   385,   387,   389,   394,   190,  -734,
    -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,
    -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,
     797,  -734,  -734,   216,  -734,  -734,  -734,  -734,  -734,  -734,
    -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,
     347,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,
    -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,
    -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,   373,   384,
    -734,   -49,    70,   197,  -734,    13,  -734,   397,   398,   399,
     401,    75,   -29,   -29,   -44,   -44,   -29,   -29,    12,   -44,
     -44,   -14,     7,     7,     7,   390,    18,   -29,   635,   635,
     635,   275,   -14,    55,  -734,   -29,   402,   -44,   430,    13,
      13,   500,   -29,    13,   -29,   411,  -734,   346,   512,   395,
     797,   213,  -734,  -734,   431,  -734,   400,   405,   439,   444,
    -734,  -734,  -734,   448,   568,  -734,   450,   456,   347,  -734,
    -734,    27,  -734,   454,  -734,   178,  -734,    80,    80,   112,
      -7,  -734,  -734,   460,   464,   465,   583,  -734,   474,  -734,
    -734,   488,   508,   509,   482,   616,   510,   517,  -734,  -734,
    -734,  -734,   518,   619,  -734,   622,   519,   520,   521,   638,
     536,  -734,    44,   258,  -734,   550,   -44,   -44,   -29,   -29,
     552,   -29,   575,   576,   585,   153,  -734,  -734,   513,  -734,
    -734,  -734,  -734,  -734,  -734,  -734,  -734,   589,  -734,  -734,
     590,   579,   591,   635,   592,  -734,   588,   299,   593,   594,
     595,   596,   597,  -734,   598,   600,   601,   602,    51,    37,
    -734,  -734,  -734,  -734,   463,  -734,  -734,   605,  -734,    18,
     -49,   721,    40,   722,   635,   609,    18,   295,   608,   617,
     230,    18,    13,    13,    13,    13,    13,    13,  -734,  -734,
     739,  -734,  -734,   625,  -734,  -734,  -734,  -734,  -734,  -734,
     740,  -734,  -734,   631,   630,   640,  -734,  -734,   642,  -734,
     635,   707,   713,   646,    18,   203,   341,   710,   766,   651,
     716,     7,    14,  -734,   652,   656,     7,     7,     7,   657,
      53,   655,   378,   -29,    18,    18,    18,    18,    18,   119,
    -734,  -734,   627,   628,   484,  -734,   -29,   -29,   318,   658,
     635,    21,    75,   797,  -734,    40,   662,   100,  -734,  -734,
     663,   154,   664,   100,    50,   122,    66,   635,   667,  -734,
      -9,    -3,   105,   134,  -734,  -734,   463,  -734,   386,  -734,
    -734,  -734,   781,  -734,   668,   666,   807,   391,  -734,  -734,
    -734,  -734,  -734,  -734,   396,   458,   251,   251,  -734,  -734,
    -734,   704,    18,   714,   709,   724,    18,   719,   -29,   -29,
    -734,   726,   727,   838,   494,  -734,  -734,   728,  -734,   725,
    -734,   730,  -734,  -734,   731,   848,  -734,   733,  -734,   850,
     735,   203,   852,   738,   741,  -734,   808,  -734,  -734,  -734,
    -734,  -734,   288,   288,  -734,  -734,  -734,  -734,  -734,   742,
     -29,   -29,   703,   743,   746,   753,   754,   757,   758,  -734,
    -734,  -734,   862,   759,   760,   755,   761,   100,  -734,   880,
    -734,   762,   764,  -734,   763,   765,    13,  -734,   767,  -734,
     400,   779,  -734,   768,   770,    39,  -734,   101,  -734,    48,
    -734,  -734,   888,   771,  -734,    22,  -734,    18,  -734,   769,
    -734,  -734,   774,   892,  -734,   776,  -734,  -734,  -734,   773,
     499,   505,   797,   775,   506,   797,  -734,  -734,    75,   900,
     786,  -734,   -14,   903,   181,   904,   191,   851,   789,    25,
    -734,   791,   107,   853,  -734,   793,   -49,  -734,   794,   864,
     635,  -734,    18,    18,   635,   635,   796,    36,   367,   849,
     866,  -734,   799,    18,   100,   797,    10,   800,   663,   801,
     664,    69,  -734,   140,  -734,   160,  -734,   170,  -734,   214,
    -734,  -734,   463,  -734,   203,   802,  -734,   803,  -734,  -734,
    -734,  -734,  -734,  -734,  -734,   804,   826,  -734,   827,  -734,
     828,  -734,  -734,   829,  -734,  -734,  -734,  -734,  -734,   824,
     830,  -734,   825,   834,  -734,   835,   919,   836,   832,  -734,
    -734,  -734,   839,   920,  -734,   954,  -734,   383,   840,  -734,
    -734,    42,   400,    18,  -734,   841,   237,   294,   842,   843,
     957,  -734,   958,   347,  -734,   847,  -734,   894,   854,   203,
     511,   855,   844,   290,   856,   845,    10,    24,  -734,   -49,
    -734,  -734,   858,    39,  -734,   101,  -734,    48,  -734,   846,
     963,  -734,   -49,    13,    13,    13,   112,  -734,  -734,  -734,
     252,   967,  -734,   859,   946,  -734,  -734,   972,   860,   861,
     863,    36,  -734,  -734,   865,   463,  -734,   955,  -734,    18,
    -734,  -734,   867,   869,  -734,    42,   868,   977,   870,  -734,
     978,  -734,   871,  -734,  -734,  -734,  -734,   872,   873,   -60,
     979,  -734,  -734,  -734,  -734,  -734,   877,   400,  -734,  -734,
    -734,  -734,   235,  -734,  -734,  -734,   -49,  -734,  -734,  -734,
    -734,  -734,   878,   879,   881,  -734,  -734,   882,  -734,  -734,
    -734,   400,  -734,   983,   884,   875,   885,   987,  -734,   295,
     347,  -734,   883,   248,  -734,   994,   886,  -734,   797,  -734,
    -734,  -734,  -734,   660,  -734,  -734,  -734,  1002,   956,   400,
      13,    13,    13,   -27,   887,  1004,   797,  -734,  1007,   890,
     891,  -734,  -734,   988,  -734,   893,   797,   895,   660,  -734,
    -734,  -734,   660,   897,  -734,  -734,  -734,  -734,  -734,   874,
    -734,  -734,  -734,   896,   898,  -734,  -734,   899,   901,  -734,
     902,  -734,  -734,  -734,   203,   -27,  -734,  -734,   797,  -734,
    -734,   906,  -734,   905,    60,  -734,  -734,  -734,   835,  -734
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -734,  -734,  1013,  -734,  -734,  1006,  -734,  -734,  -734,   152,
    -734,  -734,   837,   -90,  -733,   857,  -734,   876,  -734,  -734,
    -734,  -734,  -734,  -734,  -734,  -734,  -734,   108,  -734,  -734,
    -734,  -734,   335,   974,  -734,  -734,   348,  -734,  -734,  -734,
    -532,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,
    -734,  -734,   669,   889,  -734,  -734,  -734,  -734,  -204,  -734,
    -734,  -734,   315,  -734,  -734,  -734,  -504,   298,  -734,  -734,
     380,   249,  -734,   440,  -734,  -734,   443,  -734,  -734,  -734,
    -734,  -734,  -734,   189,  -734,  -734,   831,  -734,  -734,  -734,
    -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,  -734,
    -734,  -734,  -734,   647,  -734,  -734,  -734,  -734,  -734,  -734,
    -734,  -734,  -734,  -734,  -734,  -734,  -734,   481,  -734,  -372,
    -458,  -734,  -734,  -734,   336,  -734,   737,  -177,  -128,  -180,
     118,  -179,  -391,  -390,   339,  -386,  -734,  -383,  -387,  -127,
    -161,  -734,  -160,  -143,  -359,  -734,   -16,   188,  -734,  -130,
    -734
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -204
static const yytype_int16 yytable[] =
{
     186,    23,   216,   288,   318,   253,   291,   302,   303,   270,
     271,   478,   524,   570,   520,   208,   530,   210,   215,   528,
     504,   526,   251,   200,   224,   225,   304,   746,   232,   233,
     236,   251,   246,   247,   248,   319,   216,   216,   241,   255,
     216,   251,   328,   421,   234,   347,   722,   282,   251,   234,
     320,   508,   371,   328,   290,   408,   292,   272,     8,   371,
     278,   273,   274,   806,   200,   866,   468,   144,    10,   508,
     830,   838,   508,   501,   200,   664,   691,   656,   226,   192,
     197,   198,   199,   328,   676,   193,   194,   203,   671,   200,
     683,   329,   684,   204,   723,   830,   227,   228,   310,   830,
     307,   839,   329,   508,   338,   389,     9,   279,     7,   205,
     206,   533,   867,     7,    22,   338,   591,   535,   534,   414,
     415,    22,   221,   396,   536,   338,    16,   222,   203,   245,
     382,   383,   329,   385,   235,   211,   469,    11,   416,   608,
     252,   606,   604,   502,   204,   212,   213,   657,   418,   252,
     205,   206,    22,   339,   425,   427,   699,   315,   672,   298,
     434,   330,   422,   214,   339,   409,   252,   331,   214,   487,
     509,   488,   330,   252,   339,   249,   525,   214,   331,   216,
     216,   216,   216,   216,   216,   145,   644,   214,   509,   662,
     450,   509,   529,   454,   214,   435,   436,   437,   438,   439,
     440,   214,   330,   681,   349,   200,   519,    17,   331,    19,
     692,   510,   511,   482,   483,   484,   485,   486,   148,   149,
     150,   738,   509,   340,   494,   537,   332,   333,   498,   341,
     500,   467,   538,   649,   340,    22,   473,   474,   475,   650,
     341,    24,   510,   511,   340,   481,   176,   531,   527,   200,
     341,   455,   201,   202,   539,   456,   342,   343,   495,   496,
     693,   540,   256,   257,   258,   259,   260,   694,   261,   203,
     262,   263,   264,   231,    21,   265,   139,   239,   240,   272,
     695,   550,   192,   273,   274,   554,   322,   696,   193,   194,
     697,   205,   206,   141,   272,   284,   852,   698,   273,   274,
     853,   266,   749,   505,   142,   762,   763,   754,   143,   753,
     752,   653,   185,   203,   616,   757,   764,   765,   766,   204,
     556,   557,   151,   781,   742,   497,   152,   275,   153,   323,
     324,   325,   326,   327,   646,   205,   206,   616,   767,   597,
     768,   769,   770,   771,   772,   773,   774,   775,   776,   777,
     251,   428,   429,   433,   610,   611,   808,   727,   179,   563,
     728,   154,   577,   578,   294,   155,   295,   156,   823,   180,
     200,   824,   187,   216,   380,   381,   612,   157,   373,   809,
     192,   374,   375,   376,   377,   378,   193,   194,   374,   375,
     376,   377,   378,   458,   459,   460,   861,   158,   200,   374,
     375,   376,   377,   378,   325,   326,   327,   159,   637,   374,
     375,   376,   377,   378,   729,     1,     2,   296,   492,   674,
     665,   666,   667,   636,   668,   669,   160,   256,   257,   258,
     259,   260,   680,   261,   203,   262,   263,   264,   201,   202,
     265,   376,   377,   378,   161,   374,   375,   376,   377,   378,
     323,   324,   325,   326,   327,   162,   205,   206,   163,   164,
     297,   165,   639,   166,   167,   642,   266,   188,   298,   734,
     168,   169,   299,   170,   300,   171,   172,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,   214,   173,   175,   190,
     177,   480,   725,   178,   191,   682,   181,   289,   182,   619,
     183,   541,   249,   620,   621,   184,   546,   305,   217,   218,
     219,   547,   220,   283,   306,   311,   758,   759,   760,   374,
     375,   376,   377,   378,   293,   492,   200,   374,   375,   376,
     377,   378,   374,   375,   376,   377,   378,   374,   375,   376,
     377,   378,   622,   623,   624,   309,   659,   200,   310,   312,
     216,   216,   216,    71,   313,    73,   625,    75,   793,    77,
     314,    79,   315,    81,   316,    83,   317,    85,   321,    87,
     350,   626,   627,   548,   351,   352,   821,   353,   628,   629,
     630,   631,   632,   256,   257,   258,   259,   260,   354,   261,
     203,   262,   263,   264,   358,   633,   265,   634,   355,   323,
     324,   325,   326,   327,   374,   375,   376,   377,   378,   561,
     359,   203,   205,   206,   618,   363,   365,   204,   356,   357,
     360,   641,   266,   835,   836,   837,   739,   361,   362,   366,
     367,   368,   369,   205,   206,   374,   375,   376,   377,   378,
     374,   375,   376,   377,   378,   214,   370,   374,   375,   376,
     377,   378,   374,   375,   376,   377,   378,   216,   216,   216,
     379,   748,   390,    25,   384,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,   200,    38,    39,
      40,    41,    42,    43,    44,    45,    46,   386,   387,    47,
      48,    49,    50,    51,    52,   393,    53,   388,   827,   391,
     392,   395,   397,   398,   400,   401,   402,   403,   399,   404,
      54,   405,   406,   407,   420,   424,   843,    55,    56,   426,
      57,    58,   417,    59,    60,    61,   850,   431,    62,    63,
     432,   829,   441,   445,   256,   257,   258,   259,   260,   442,
     261,   203,   262,   263,   264,   446,  -203,   265,   451,    64,
     448,    65,   449,     1,   452,    66,   453,    67,   863,   463,
     464,   465,   471,   205,   206,   466,   472,   476,   479,   490,
     491,   499,   507,   266,   542,   516,   521,   532,   543,   544,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      25,   545,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,   549,    38,    39,    40,    41,    42,
      43,    44,    45,    46,   551,   552,    47,    48,    49,    50,
      51,    52,   560,    53,   553,   555,   558,   559,   562,   563,
     564,   565,   566,   567,   568,   569,   571,    54,   572,   575,
     579,   573,   576,   580,    55,    56,   586,    57,    58,   581,
      59,    60,    61,   582,   583,    62,    63,   584,   585,   587,
     588,   590,   589,   592,   594,   599,   593,   598,   600,   595,
     601,   596,   609,   226,   614,   615,    64,   613,    65,   616,
       1,   617,    66,   640,    67,   645,   646,   648,   652,   655,
     654,   653,   660,   661,   663,   492,   670,   678,  -149,   679,
     687,   689,   700,   713,   718,   702,   701,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,   703,   704,   705,
     706,   707,   709,   708,   710,   711,   714,   716,   719,   717,
     721,   732,   733,   736,   726,   730,   731,   735,   756,   755,
     782,   741,   744,   784,   737,   740,   786,   743,   750,   783,
     787,   799,   792,   789,   788,   801,   691,   814,   791,   795,
     794,   819,   802,   800,   798,   804,   805,   807,   825,   810,
     811,   816,   812,   813,   815,   818,   822,   833,   834,   842,
     841,   844,   826,   845,   846,   848,   849,   854,    15,    18,
     869,   857,   851,   856,   859,   858,   864,   751,   308,   860,
     174,   855,   865,   797,   745,   285,   790,   720,   820,   477,
     690,   688,   506,   647,   862,   761,   780,   419,     0,   337,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   277,     0,   301
};

static const yytype_int16 yycheck[] =
{
      90,    17,   145,   180,   208,   166,   183,   187,   187,   169,
     170,   370,   403,   471,   401,   143,   406,     4,   145,   405,
     392,   404,     4,    52,   152,   153,   187,     3,   156,   157,
     158,     4,   162,   163,   164,     8,   179,   180,    52,   167,
     183,     4,     3,     3,    37,    52,     4,   175,     4,    37,
     211,     3,     8,     3,   182,     4,   184,     4,   130,     8,
       5,     8,     9,   123,    52,     5,    52,     6,     0,     3,
     803,    98,     3,    52,    52,   579,     7,    52,   122,   128,
      10,    11,    12,     3,   588,   134,   135,   116,    52,    52,
      80,    52,    82,   122,    52,   828,   140,   141,   158,   832,
     190,   128,    52,     3,     3,   265,   128,    52,     0,   138,
     139,   120,    52,     5,   128,     3,   507,   120,   127,   299,
     299,   128,    47,   283,   127,     3,   122,    52,   116,   122,
     258,   259,    52,   261,   122,   122,   122,    14,   299,   529,
     122,   527,   525,   122,   122,   132,   133,   122,   309,   122,
     138,   139,   128,    52,   314,   316,   614,     4,   122,   122,
     321,   122,   122,   150,    52,   128,   122,   128,   150,    50,
     122,    52,   122,   122,    52,   122,   126,   150,   128,   322,
     323,   324,   325,   326,   327,   124,   558,   150,   122,   576,
     350,   122,   126,   354,   150,   322,   323,   324,   325,   326,
     327,   150,   122,   594,   220,    52,    52,   121,   128,    62,
     601,   142,   143,   374,   375,   376,   377,   378,    30,    31,
      32,   679,   122,   122,   384,   120,   146,   147,   388,   128,
     390,   361,   127,    52,   122,   128,   366,   367,   368,    58,
     128,   121,   142,   143,   122,   373,    58,   407,   126,    52,
     128,    48,    55,    56,   120,    52,   144,   145,   386,   387,
     120,   127,   109,   110,   111,   112,   113,   127,   115,   116,
     117,   118,   119,   155,   123,   122,    63,   159,   160,     4,
     120,   442,   128,     8,     9,   446,   108,   127,   134,   135,
     120,   138,   139,   121,     4,   177,   828,   127,     8,     9,
     832,   148,   689,   393,   121,    53,    54,   697,   121,   695,
     693,   120,   122,   116,   123,   702,    64,    65,    66,   122,
     448,   449,   121,   710,   683,     7,   121,    52,   121,   151,
     152,   153,   154,   155,   120,   138,   139,   123,    86,   516,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
       4,    56,    57,   123,   533,   535,   121,   120,   124,   124,
     123,   121,   490,   491,    18,   121,    20,   121,   120,   124,
      52,   123,   156,   516,   256,   257,   537,   121,   120,   766,
     128,   151,   152,   153,   154,   155,   134,   135,   151,   152,
     153,   154,   155,    52,    53,    54,   854,   121,    52,   151,
     152,   153,   154,   155,   153,   154,   155,   121,   551,   151,
     152,   153,   154,   155,   120,   103,   104,    71,    51,    52,
     580,   582,   583,   551,   584,   585,   121,   109,   110,   111,
     112,   113,   593,   115,   116,   117,   118,   119,    55,    56,
     122,   153,   154,   155,   121,   151,   152,   153,   154,   155,
     151,   152,   153,   154,   155,   121,   138,   139,   121,   121,
     114,   121,   552,   121,   121,   555,   148,   120,   122,   673,
     121,   121,   126,   121,   128,   121,   121,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   121,   121,   126,
     121,   123,   663,   121,   120,   595,   121,     7,   121,     4,
     121,   125,   122,     8,     9,   121,   125,     5,   121,   121,
     121,   125,   121,   121,   129,   120,   703,   704,   705,   151,
     152,   153,   154,   155,   123,    51,    52,   151,   152,   153,
     154,   155,   151,   152,   153,   154,   155,   151,   152,   153,
     154,   155,    47,    48,    49,   124,   572,    52,   158,   120,
     703,   704,   705,   133,   120,   135,    61,   137,   729,   139,
     122,   141,     4,   143,   124,   145,   120,   147,   124,   149,
     120,    76,    77,   125,   120,   120,   790,     4,    83,    84,
      85,    86,    87,   109,   110,   111,   112,   113,   124,   115,
     116,   117,   118,   119,   122,   100,   122,   102,   120,   151,
     152,   153,   154,   155,   151,   152,   153,   154,   155,   125,
       4,   116,   138,   139,   125,     6,     4,   122,   120,   120,
     120,   125,   148,   810,   811,   812,   125,   120,   120,   120,
     120,   120,     4,   138,   139,   151,   152,   153,   154,   155,
     151,   152,   153,   154,   155,   150,   120,   151,   152,   153,
     154,   155,   151,   152,   153,   154,   155,   810,   811,   812,
     120,   687,   159,    13,   122,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    52,    28,    29,
      30,    31,    32,    33,    34,    35,    36,   122,   122,    39,
      40,    41,    42,    43,    44,   126,    46,   122,   798,   120,
     120,   120,   120,   125,   120,   120,   120,   120,   125,   121,
      60,   121,   121,   121,     3,     3,   816,    67,    68,   120,
      70,    71,   127,    73,    74,    75,   826,   129,    78,    79,
     123,    81,     3,     3,   109,   110,   111,   112,   113,   124,
     115,   116,   117,   118,   119,   124,   126,   122,    51,    99,
     120,   101,   120,   103,    51,   105,   120,   107,   858,    59,
       4,   120,   120,   138,   139,    59,   120,   120,   123,   152,
     152,   123,   120,   148,     3,   122,   122,   120,   120,   123,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
      13,     4,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,   120,    28,    29,    30,    31,    32,
      33,    34,    35,    36,   120,   126,    39,    40,    41,    42,
      43,    44,     4,    46,   120,   126,   120,   120,   120,   124,
     120,   120,     4,   120,     4,   120,     4,    60,   120,    51,
     157,   120,   120,   120,    67,    68,     4,    70,    71,   123,
      73,    74,    75,   120,   120,    78,    79,   120,   120,   120,
     120,   120,   127,     3,   120,   106,   124,   120,   120,   126,
     120,   126,     4,   122,   120,     3,    99,   128,   101,   123,
     103,   128,   105,   128,   107,     5,   120,     4,     4,   120,
      59,   120,    59,   120,   120,    51,   120,    51,    69,   120,
     120,   120,   120,     4,     4,   121,   123,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   121,   121,   121,
     121,   127,   127,   123,   120,   120,   120,   125,     4,   120,
     120,     4,     4,    69,   123,   123,   123,   120,     5,   123,
       3,   127,   127,    27,   120,   120,     4,   121,   120,   120,
     120,     4,    27,   120,   123,     7,     7,     4,   123,   120,
     123,     4,   121,   123,   126,   123,   123,   120,     4,   121,
     121,   126,   121,   121,   120,   120,   123,     5,    52,     5,
     123,     4,   126,   123,   123,    27,   123,   120,     5,    13,
     868,   123,   127,   127,   123,   126,   120,   692,   191,   127,
      56,   157,   127,   735,   686,   178,   721,   657,   789,   370,
     600,   598,   395,   562,   855,   706,   710,   310,    -1,   218,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   172,    -1,   187
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   103,   104,   161,   162,   163,   164,   187,   130,   128,
       0,    14,   165,   166,   167,   162,   122,   121,   165,    62,
     171,   123,   128,   306,   121,    13,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    39,    40,    41,
      42,    43,    44,    46,    60,    67,    68,    70,    71,    73,
      74,    75,    78,    79,    99,   101,   105,   107,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   168,   173,
     174,   175,   176,   177,   178,   179,   187,   189,   190,   194,
     202,   204,   208,   209,   210,   211,   214,   216,   223,   228,
     229,   232,   235,   238,   244,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   260,   261,   262,   264,   265,
     266,   267,   268,   269,   272,   273,   274,   275,   188,    63,
     172,   121,   121,   121,     6,   124,   193,   307,   307,   307,
     307,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   193,   121,   307,   121,   121,   124,
     124,   121,   121,   121,   121,   122,   173,   156,   120,   218,
     126,   120,   128,   134,   135,   286,   298,    10,    11,    12,
      52,    55,    56,   116,   122,   138,   139,   230,   288,   289,
       4,   122,   132,   133,   150,   299,   303,   121,   121,   121,
     121,    47,    52,   279,   288,   288,   122,   140,   141,   290,
     291,   290,   288,   288,    37,   122,   288,   309,   310,   290,
     290,    52,   213,   277,   306,   122,   309,   309,   309,   122,
     212,     4,   122,   300,   303,   288,   109,   110,   111,   112,
     113,   115,   117,   118,   119,   122,   148,   288,   301,   302,
     302,   302,     4,     8,     9,    52,   304,   213,     5,    52,
     224,   205,   288,   121,   290,   175,   287,   299,   287,     7,
     288,   287,   288,   123,    18,    20,    71,   114,   122,   126,
     128,   177,   289,   291,   300,     5,   129,   173,   172,   124,
     158,   120,   120,   120,   122,     4,   124,   120,   218,     8,
     300,   124,   108,   151,   152,   153,   154,   155,     3,    52,
     122,   128,   146,   147,   246,   296,   297,   246,     3,    52,
     122,   128,   144,   145,   240,   294,   295,    52,   276,   306,
     120,   120,   120,     4,   124,   120,   120,   120,   122,     4,
     120,   120,   120,     6,   191,     4,   120,   120,   120,     4,
     120,     8,   300,   120,   151,   152,   153,   154,   155,   120,
     290,   290,   288,   288,   122,   288,   122,   122,   122,   302,
     159,   120,   120,   126,   206,   120,   302,   120,   125,   125,
     120,   120,   120,   120,   121,   121,   121,   121,     4,   128,
     183,   184,   185,   186,   289,   291,   300,   127,   300,   286,
       3,     3,   122,   263,     3,   302,   120,   300,    56,    57,
     231,   129,   123,   123,   300,   299,   299,   299,   299,   299,
     299,     3,   124,   247,   245,     3,   124,   239,   120,   120,
     302,    51,    51,   120,   300,    48,    52,   280,    52,    53,
      54,   281,   284,    59,     4,   120,    59,   309,    52,   122,
     219,   120,   120,   309,   309,   309,   120,   212,   304,   123,
     123,   288,   300,   300,   300,   300,   300,    50,    52,   308,
     152,   152,    51,   226,   302,   288,   288,     7,   302,   123,
     302,    52,   122,   220,   279,   173,   263,   120,     3,   122,
     142,   143,   292,   293,   203,   195,   122,   236,   237,    52,
     298,   122,   233,   234,   292,   126,   297,   126,   295,   126,
     293,   302,   120,   120,   127,   120,   127,   120,   127,   120,
     127,   125,     3,   120,   123,     4,   125,   125,   125,   120,
     300,   120,   126,   120,   300,   126,   288,   288,   120,   120,
       4,   125,   120,   124,   120,   120,     4,   120,     4,   120,
     280,     4,   120,   120,   259,    51,   120,   288,   288,   157,
     120,   123,   120,   120,   120,   120,     4,   120,   120,   127,
     120,   292,     3,   124,   120,   126,   126,   287,   120,   106,
     120,   120,   192,   180,   297,   181,   295,   182,   293,     4,
     291,   289,   300,   128,   120,     3,   123,   128,   125,     4,
       8,     9,    47,    48,    49,    61,    76,    77,    83,    84,
      85,    86,    87,   100,   102,   248,   288,   303,   305,   173,
     128,   125,   173,   241,   279,     5,   120,   277,     4,    52,
      58,   283,     4,   120,    59,   120,    52,   122,   221,   306,
      59,   120,   298,   120,   226,   302,   300,   300,   302,   302,
     120,    52,   122,   222,    52,   225,   226,   207,    51,   120,
     300,   292,   173,    80,    82,   196,   197,   120,   236,   120,
     233,     7,   292,   120,   127,   120,   127,   120,   127,   280,
     120,   123,   121,   121,   121,   121,   121,   127,   123,   127,
     120,   120,   169,     4,   120,   278,   125,   120,     4,     4,
     230,   120,     4,    52,   227,   300,   123,   120,   123,   120,
     123,   123,     4,     4,   218,   120,    69,   120,   280,   125,
     120,   127,   304,   121,   127,   196,     3,   285,   306,   298,
     120,   192,   297,   295,   293,   123,     5,   298,   287,   287,
     287,   294,    53,    54,    64,    65,    66,    86,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,   242,   282,
     284,   298,     3,   120,    27,   270,     4,   120,   123,   120,
     222,   123,    27,   300,   123,   120,   215,   227,   126,     4,
     123,     7,   121,   199,   123,   123,   123,   120,   121,   298,
     121,   121,   121,   121,     4,   120,   126,   271,   120,     4,
     231,   218,   123,   120,   123,     4,   126,   173,   198,    81,
     174,   200,   201,     5,    52,   287,   287,   287,    98,   128,
     243,   123,     5,   173,     4,   123,   123,   217,    27,   123,
     173,   127,   200,   200,   120,   157,   127,   123,   126,   123,
     127,   280,   243,   173,   120,   127,     5,    52,   170,   169
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 8:
#line 244 "lev_comp.y"
    {
		      include_push( (yyvsp[(2) - (2)].map) );
		      Free((yyvsp[(2) - (2)].map));
		  }
    break;

  case 11:
#line 255 "lev_comp.y"
    {
			if (fatal_error > 0) {
				(void) fprintf(stderr,
				"%s: %d errors detected for level \"%s\". No output created!\n",
					       fname, fatal_error, (yyvsp[(1) - (3)].map));
				fatal_error = 0;
				got_errors++;
			} else if (!got_errors) {
				if (!write_level_file((yyvsp[(1) - (3)].map), splev)) {
				    lc_error("Can't write output file for '%s'!", (yyvsp[(1) - (3)].map));
				    exit(EXIT_FAILURE);
				}
			}
			Free((yyvsp[(1) - (3)].map));
			Free(splev);
			splev = NULL;
			vardef_free_all(variable_definitions);
			variable_definitions = NULL;
		  }
    break;

  case 12:
#line 277 "lev_comp.y"
    {
		      struct lc_funcdefs *f;
			if (index((yyvsp[(3) - (3)].map), '.'))
			    lc_error("Invalid dot ('.') in level name '%s'.", (yyvsp[(3) - (3)].map));
			if ((int) strlen((yyvsp[(3) - (3)].map)) > 8)
			    lc_error("Level names limited to 8 characters ('%s').", (yyvsp[(3) - (3)].map));
			f = function_definitions;
			while (f) {
			    f->n_called = 0;
			    f = f->next;
			}
			splev = (sp_lev *)alloc(sizeof(sp_lev));
			splev->n_opcodes = 0;
			splev->opcodes = NULL;

			vardef_free_all(variable_definitions);
			variable_definitions = NULL;

			(yyval.map) = (yyvsp[(3) - (3)].map);
		  }
    break;

  case 13:
#line 300 "lev_comp.y"
    {
		      long filling = (yyvsp[(5) - (5)].terr).ter;
		      if (filling == INVALID_TYPE || filling >= MAX_TYPE)
			  lc_error("INIT_MAP: Invalid fill char type.");
		      add_opvars(splev, "iiiiiiiio", LVLINIT_SOLIDFILL,filling,0,(long)(yyvsp[(5) - (5)].terr).lit, 0,0,0,0, SPO_INITLEVEL);
		      max_x_map = COLNO-1;
		      max_y_map = ROWNO;
		  }
    break;

  case 14:
#line 309 "lev_comp.y"
    {
		      long filling = what_map_char((char) (yyvsp[(5) - (5)].i));
		      if (filling == INVALID_TYPE || filling >= MAX_TYPE)
			  lc_error("INIT_MAP: Invalid fill char type.");
		      add_opvars(splev, "iiiiiiiio", LVLINIT_MAZEGRID,filling,0,0, 0,0,0,0, SPO_INITLEVEL);
		      max_x_map = COLNO-1;
		      max_y_map = ROWNO;
		  }
    break;

  case 15:
#line 318 "lev_comp.y"
    {
		      long fg = what_map_char((char) (yyvsp[(5) - (16)].i));
		      long bg = what_map_char((char) (yyvsp[(7) - (16)].i));
		      long smoothed = (yyvsp[(9) - (16)].i);
		      long joined = (yyvsp[(11) - (16)].i);
		      long lit = (yyvsp[(13) - (16)].i);
		      long walled = (yyvsp[(15) - (16)].i);
		      long filling = (yyvsp[(16) - (16)].i);
		      if (fg == INVALID_TYPE || fg >= MAX_TYPE)
			  lc_error("INIT_MAP: Invalid foreground type.");
		      if (bg == INVALID_TYPE || bg >= MAX_TYPE)
			  lc_error("INIT_MAP: Invalid background type.");
		      if (joined && fg != CORR && fg != ROOM)
			  lc_error("INIT_MAP: Invalid foreground type for joined map.");

		      if (filling == INVALID_TYPE)
			  lc_error("INIT_MAP: Invalid fill char type.");

		      add_opvars(splev, "iiiiiiiio", LVLINIT_MINES,filling,walled,lit, joined,smoothed,bg,fg, SPO_INITLEVEL);
			max_x_map = COLNO-1;
			max_y_map = ROWNO;
		  }
    break;

  case 16:
#line 343 "lev_comp.y"
    {
		      (yyval.i) = -1;
		  }
    break;

  case 17:
#line 347 "lev_comp.y"
    {
		      (yyval.i) = what_map_char((char) (yyvsp[(2) - (2)].i));
		  }
    break;

  case 20:
#line 358 "lev_comp.y"
    {
		      add_opvars(splev, "io", 0, SPO_LEVEL_FLAGS);
		  }
    break;

  case 21:
#line 362 "lev_comp.y"
    {
		      add_opvars(splev, "io", (yyvsp[(3) - (3)].i), SPO_LEVEL_FLAGS);
		  }
    break;

  case 22:
#line 368 "lev_comp.y"
    {
		      (yyval.i) = ((yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i));
		  }
    break;

  case 23:
#line 372 "lev_comp.y"
    {
		      (yyval.i) = (yyvsp[(1) - (1)].i);
		  }
    break;

  case 24:
#line 378 "lev_comp.y"
    {
		      (yyval.i) = 0;
		  }
    break;

  case 25:
#line 382 "lev_comp.y"
    {
		      (yyval.i) = 1 + (yyvsp[(2) - (2)].i);
		  }
    break;

  case 91:
#line 461 "lev_comp.y"
    {
		      struct lc_vardefs *vd;
		      if ((vd = vardef_defined(variable_definitions, (yyvsp[(3) - (3)].map), 1))) {
			  if (!(vd->var_type & SPOVAR_ARRAY))
			      lc_error("Trying to shuffle non-array variable '%s'", (yyvsp[(3) - (3)].map));
		      } else lc_error("Trying to shuffle undefined variable '%s'", (yyvsp[(3) - (3)].map));
		      add_opvars(splev, "so", (yyvsp[(3) - (3)].map), SPO_SHUFFLE_ARRAY);
		      Free((yyvsp[(3) - (3)].map));
		  }
    break;

  case 92:
#line 473 "lev_comp.y"
    {
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (3)].map), SPOVAR_INT);
		      add_opvars(splev, "iso", 0, (yyvsp[(1) - (3)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (3)].map));
		  }
    break;

  case 93:
#line 479 "lev_comp.y"
    {
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (5)].map), SPOVAR_SEL);
		      add_opvars(splev, "iso", 0, (yyvsp[(1) - (5)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (5)].map));
		  }
    break;

  case 94:
#line 485 "lev_comp.y"
    {
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (3)].map), SPOVAR_STRING);
		      add_opvars(splev, "siso", (yyvsp[(3) - (3)].map), 0, (yyvsp[(1) - (3)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (3)].map));
		      Free((yyvsp[(3) - (3)].map));
		  }
    break;

  case 95:
#line 492 "lev_comp.y"
    {
		      struct lc_vardefs *vd1, *vd2;
		      if (!strcmp((yyvsp[(1) - (3)].map), (yyvsp[(3) - (3)].map))) lc_error("Trying to set variable '%s' to value of itself", (yyvsp[(1) - (3)].map));
		      vd2 = vardef_defined(variable_definitions, (yyvsp[(3) - (3)].map), 1);
		      if (!vd2) {
			  lc_error("Trying to use an undefined variable '%s'", (yyvsp[(3) - (3)].map));
		      } else {
			  if ((vd1 = vardef_defined(variable_definitions, (yyvsp[(1) - (3)].map), 1))) {
			      if (vd1->var_type != vd2->var_type)
				  lc_error("Trying to redefine variable '%s' as different type", (yyvsp[(1) - (3)].map));
			  } else {
			      vd1 = vardef_new(vd2->var_type, (yyvsp[(1) - (3)].map));
			      vd1->next = variable_definitions;
			      variable_definitions = vd1;
			  }
		      }
		      add_opvars(splev, "siso", (yyvsp[(3) - (3)].map), -1, (yyvsp[(1) - (3)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (3)].map));
		      Free((yyvsp[(3) - (3)].map));
		  }
    break;

  case 96:
#line 513 "lev_comp.y"
    {
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (5)].map), SPOVAR_MAPCHAR);
		      add_opvars(splev, "miso", (long)(yyvsp[(5) - (5)].i), 0, (yyvsp[(1) - (5)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (5)].map));
		  }
    break;

  case 97:
#line 519 "lev_comp.y"
    {
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (5)].map), SPOVAR_MONST);
		      add_opvars(splev, "Miso", (long)(yyvsp[(5) - (5)].i), 0, (yyvsp[(1) - (5)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (5)].map));
		  }
    break;

  case 98:
#line 525 "lev_comp.y"
    {
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (5)].map), SPOVAR_OBJ);
		      add_opvars(splev, "Oiso", (long)(yyvsp[(5) - (5)].i), 0, (yyvsp[(1) - (5)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (5)].map));
		  }
    break;

  case 99:
#line 531 "lev_comp.y"
    {
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (3)].map), SPOVAR_COORD);
		      add_opvars(splev, "ciso", (long)(yyvsp[(3) - (3)].i), 0, (yyvsp[(1) - (3)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (3)].map));
		  }
    break;

  case 100:
#line 537 "lev_comp.y"
    {
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (3)].map), SPOVAR_REGION);
		      add_opvars(splev, "riso", (long)(yyvsp[(3) - (3)].i), 0, (yyvsp[(1) - (3)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (3)].map));
		  }
    break;

  case 101:
#line 543 "lev_comp.y"
    {
		      long n_items = (yyvsp[(4) - (5)].i);
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (5)].map), SPOVAR_INT|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, (yyvsp[(1) - (5)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (5)].map));
		  }
    break;

  case 102:
#line 550 "lev_comp.y"
    {
		      long n_items = (yyvsp[(4) - (5)].i);
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (5)].map), SPOVAR_COORD|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, (yyvsp[(1) - (5)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (5)].map));
		  }
    break;

  case 103:
#line 557 "lev_comp.y"
    {
		      long n_items = (yyvsp[(4) - (5)].i);
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (5)].map), SPOVAR_REGION|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, (yyvsp[(1) - (5)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (5)].map));
		  }
    break;

  case 104:
#line 564 "lev_comp.y"
    {
		      long n_items = (yyvsp[(6) - (7)].i);
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (7)].map), SPOVAR_MAPCHAR|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, (yyvsp[(1) - (7)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (7)].map));
		  }
    break;

  case 105:
#line 571 "lev_comp.y"
    {
		      long n_items = (yyvsp[(6) - (7)].i);
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (7)].map), SPOVAR_MONST|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, (yyvsp[(1) - (7)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (7)].map));
		  }
    break;

  case 106:
#line 578 "lev_comp.y"
    {
		      long n_items = (yyvsp[(6) - (7)].i);
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (7)].map), SPOVAR_OBJ|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, (yyvsp[(1) - (7)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (7)].map));
		  }
    break;

  case 107:
#line 585 "lev_comp.y"
    {
		      long n_items = (yyvsp[(4) - (5)].i);
		      variable_definitions = add_vardef_type(variable_definitions, (yyvsp[(1) - (5)].map), SPOVAR_STRING|SPOVAR_ARRAY);
		      add_opvars(splev, "iso", n_items, (yyvsp[(1) - (5)].map), SPO_VAR_INIT);
		      Free((yyvsp[(1) - (5)].map));
		  }
    break;

  case 108:
#line 594 "lev_comp.y"
    {
		      add_opvars(splev, "O", (yyvsp[(1) - (1)].i));
		      (yyval.i) = 1;
		  }
    break;

  case 109:
#line 599 "lev_comp.y"
    {
		      add_opvars(splev, "O", (yyvsp[(3) - (3)].i));
		      (yyval.i) = 1 + (yyvsp[(1) - (3)].i);
		  }
    break;

  case 110:
#line 606 "lev_comp.y"
    {
		      add_opvars(splev, "M", (yyvsp[(1) - (1)].i));
		      (yyval.i) = 1;
		  }
    break;

  case 111:
#line 611 "lev_comp.y"
    {
		      add_opvars(splev, "M", (yyvsp[(3) - (3)].i));
		      (yyval.i) = 1 + (yyvsp[(1) - (3)].i);
		  }
    break;

  case 112:
#line 618 "lev_comp.y"
    {
		      add_opvars(splev, "m", (yyvsp[(1) - (1)].i));
		      (yyval.i) = 1;
		  }
    break;

  case 113:
#line 623 "lev_comp.y"
    {
		      add_opvars(splev, "m", (yyvsp[(3) - (3)].i));
		      (yyval.i) = 1 + (yyvsp[(1) - (3)].i);
		  }
    break;

  case 114:
#line 630 "lev_comp.y"
    {
		      add_opvars(splev, "r", (yyvsp[(1) - (1)].i));
		      (yyval.i) = 1;
		  }
    break;

  case 115:
#line 635 "lev_comp.y"
    {
		      add_opvars(splev, "r", (yyvsp[(3) - (3)].i));
		      (yyval.i) = 1 + (yyvsp[(1) - (3)].i);
		  }
    break;

  case 116:
#line 642 "lev_comp.y"
    {
		      add_opvars(splev, "c", (yyvsp[(1) - (1)].i));
		      (yyval.i) = 1;
		  }
    break;

  case 117:
#line 647 "lev_comp.y"
    {
		      add_opvars(splev, "c", (yyvsp[(3) - (3)].i));
		      (yyval.i) = 1 + (yyvsp[(1) - (3)].i);
		  }
    break;

  case 118:
#line 654 "lev_comp.y"
    {
		      (yyval.i) = 1;
		  }
    break;

  case 119:
#line 658 "lev_comp.y"
    {
		      (yyval.i) = 1 + (yyvsp[(1) - (3)].i);
		  }
    break;

  case 120:
#line 664 "lev_comp.y"
    {
		      add_opvars(splev, "s", (yyvsp[(1) - (1)].map));
		      Free((yyvsp[(1) - (1)].map));
		      (yyval.i) = 1;
		  }
    break;

  case 121:
#line 670 "lev_comp.y"
    {
		      add_opvars(splev, "s", (yyvsp[(3) - (3)].map));
		      Free((yyvsp[(1) - (3)].i));
		      (yyval.i) = 1 + (yyvsp[(1) - (3)].i);
		  }
    break;

  case 122:
#line 678 "lev_comp.y"
    {
		      struct lc_funcdefs *funcdef;

		      if (in_function_definition)
			  lc_error("Recursively defined functions not allowed (function %s).", (yyvsp[(2) - (4)].map));

		      in_function_definition++;

		      if (funcdef_defined(function_definitions, (yyvsp[(2) - (4)].map), 1))
			  lc_error("Function '%s' already defined once.", (yyvsp[(2) - (4)].map));

		      funcdef = funcdef_new(-1, (yyvsp[(2) - (4)].map));
		      funcdef->next = function_definitions;
		      function_definitions = funcdef;
		      function_splev_backup = splev;
		      splev = &(funcdef->code);
		      Free((yyvsp[(2) - (4)].map));
		  }
    break;

  case 123:
#line 697 "lev_comp.y"
    {
		      add_opvars(splev, "io", 0, SPO_RETURN);
		      splev = function_splev_backup;
		      in_function_definition--;
		  }
    break;

  case 124:
#line 705 "lev_comp.y"
    {
		      struct lc_funcdefs *tmpfunc;
		      tmpfunc = funcdef_defined(function_definitions, (yyvsp[(1) - (3)].map), 1);
		      if (tmpfunc) {
			  long l;
			  if (!(tmpfunc->n_called)) {
			      /* we haven't called the function yet, so insert it in the code */
			      struct opvar *jmp = New(struct opvar);
			      set_opvar_int(jmp, splev->n_opcodes+1);
			      add_opcode(splev, SPO_PUSH, jmp);
			      add_opcode(splev, SPO_JMP, NULL); /* we must jump past it first, then CALL it, due to RETURN. */

			      tmpfunc->addr = splev->n_opcodes;
			      splev_add_from(splev, &(tmpfunc->code));
			      set_opvar_int(jmp, splev->n_opcodes - jmp->vardata.l);
			  }
			  l = tmpfunc->addr - splev->n_opcodes - 2;
			  add_opvars(splev, "iio", 0, l, SPO_CALL);
			  tmpfunc->n_called++;
		      } else {
			  lc_error("Function '%s' not defined.", (yyvsp[(1) - (3)].map));
		      }
		      Free((yyvsp[(1) - (3)].map));
		  }
    break;

  case 125:
#line 732 "lev_comp.y"
    {
		      add_opcode(splev, SPO_EXIT, NULL);
		  }
    break;

  case 126:
#line 738 "lev_comp.y"
    {
		      (yyval.i) = 100;
		  }
    break;

  case 127:
#line 742 "lev_comp.y"
    {
		      if ((yyvsp[(1) - (1)].i) < 0 || (yyvsp[(1) - (1)].i) > 100) lc_error("Unexpected percentile chance '%li%%'", (yyvsp[(1) - (1)].i));
		      (yyval.i) = (yyvsp[(1) - (1)].i);
		  }
    break;

  case 128:
#line 749 "lev_comp.y"
    {
		      (yyval.i) = 100;
		  }
    break;

  case 129:
#line 753 "lev_comp.y"
    {
		      if ((yyvsp[(2) - (2)].i) < 0 || (yyvsp[(2) - (2)].i) > 100) lc_error("Unexpected percentile chance '%li%%'", (yyvsp[(2) - (2)].i));
		      (yyval.i) = (yyvsp[(2) - (2)].i);
		  }
    break;

  case 130:
#line 760 "lev_comp.y"
    {
		      /* val > rn2(100) */
		      add_opvars(splev, "iio", (long)(yyvsp[(1) - (1)].i), 100, SPO_RN2);
		      (yyval.i) = SPO_JG;
                  }
    break;

  case 131:
#line 766 "lev_comp.y"
    {
		      (yyval.i) = (yyvsp[(3) - (5)].i);
                  }
    break;

  case 132:
#line 772 "lev_comp.y"
    {
		      struct opvar *chkjmp;
		      if (in_switch_statement > 0)
			  lc_error("Cannot nest switch-statements.");

		      in_switch_statement++;

		      n_switch_case_list = 0;
		      n_switch_break_list = 0;
		      switch_default_case = NULL;

		      add_opvars(splev, "o", SPO_RN2);

		      chkjmp = New(struct opvar);
		      set_opvar_int(chkjmp, splev->n_opcodes+1);
		      switch_check_jump = chkjmp;
		      add_opcode(splev, SPO_PUSH, chkjmp);
		      add_opcode(splev, SPO_JMP, NULL);
		  }
    break;

  case 133:
#line 792 "lev_comp.y"
    {
		      struct opvar *endjump = New(struct opvar);
		      int i;

		      set_opvar_int(endjump, splev->n_opcodes+1);

		      add_opcode(splev, SPO_PUSH, endjump);
		      add_opcode(splev, SPO_JMP, NULL);

		      set_opvar_int(switch_check_jump, splev->n_opcodes - switch_check_jump->vardata.l);

		      for (i = 0; i < n_switch_case_list; i++) {
			  add_opvars(splev, "oio", SPO_COPY, switch_case_value[i], SPO_CMP);
			  set_opvar_int(switch_case_list[i], switch_case_list[i]->vardata.l - splev->n_opcodes-1);
			  add_opcode(splev, SPO_PUSH, switch_case_list[i]);
			  add_opcode(splev, SPO_JE, NULL);
		      }

		      if (switch_default_case) {
			  set_opvar_int(switch_default_case, switch_default_case->vardata.l - splev->n_opcodes-1);
			  add_opcode(splev, SPO_PUSH, switch_default_case);
			  add_opcode(splev, SPO_JMP, NULL);
		      }

		      set_opvar_int(endjump, splev->n_opcodes - endjump->vardata.l);

		      for (i = 0; i < n_switch_break_list; i++) {
			  set_opvar_int(switch_break_list[i], splev->n_opcodes - switch_break_list[i]->vardata.l);
		      }

		      add_opcode(splev, SPO_POP, NULL); /* get rid of the value in stack */
		      in_switch_statement--;


		  }
    break;

  case 136:
#line 834 "lev_comp.y"
    {
		      if (n_switch_case_list < MAX_SWITCH_CASES) {
			  struct opvar *tmppush = New(struct opvar);
			  set_opvar_int(tmppush, splev->n_opcodes);
			  switch_case_value[n_switch_case_list] = (yyvsp[(2) - (3)].i);
			  switch_case_list[n_switch_case_list++] = tmppush;
		      } else lc_error("Too many cases in a switch.");
		  }
    break;

  case 137:
#line 843 "lev_comp.y"
    {
		  }
    break;

  case 138:
#line 846 "lev_comp.y"
    {
		      struct opvar *tmppush = New(struct opvar);

		      if (switch_default_case)
			  lc_error("Switch default case already used.");

		      set_opvar_int(tmppush, splev->n_opcodes);
		      switch_default_case = tmppush;
		  }
    break;

  case 139:
#line 856 "lev_comp.y"
    {
		  }
    break;

  case 142:
#line 865 "lev_comp.y"
    {
		      struct opvar *tmppush = New(struct opvar);
		      set_opvar_int(tmppush, splev->n_opcodes);
		      if (n_switch_break_list >= MAX_SWITCH_BREAKS)
			  lc_error("Too many BREAKs inside single SWITCH");
		      switch_break_list[n_switch_break_list++] = tmppush;

		      add_opcode(splev, SPO_PUSH, tmppush);
		      add_opcode(splev, SPO_JMP, NULL);
		  }
    break;

  case 143:
#line 876 "lev_comp.y"
    {
		  }
    break;

  case 144:
#line 881 "lev_comp.y"
    {
		      struct opvar *tmppush = New(struct opvar);

		      if (n_if_list >= MAX_NESTED_IFS) {
			  lc_error("LOOP: Too deeply nested conditionals.");
			  n_if_list = MAX_NESTED_IFS - 1;
		      }
		      set_opvar_int(tmppush, splev->n_opcodes);
		      if_list[n_if_list++] = tmppush;

		      add_opvars(splev, "o", SPO_DEC);
		  }
    break;

  case 145:
#line 894 "lev_comp.y"
    {
		      struct opvar *tmppush;

		      add_opvars(splev, "oio", SPO_COPY, 0, SPO_CMP);

		      tmppush = (struct opvar *) if_list[--n_if_list];
		      set_opvar_int(tmppush, tmppush->vardata.l - splev->n_opcodes-1);
		      add_opcode(splev, SPO_PUSH, tmppush);
		      add_opcode(splev, SPO_JG, NULL);
		      add_opcode(splev, SPO_POP, NULL); /* get rid of the count value in stack */
		  }
    break;

  case 146:
#line 908 "lev_comp.y"
    {
		      struct opvar *tmppush2 = New(struct opvar);

		      if (n_if_list >= MAX_NESTED_IFS) {
			  lc_error("IF: Too deeply nested conditionals.");
			  n_if_list = MAX_NESTED_IFS - 1;
		      }

		      add_opcode(splev, SPO_CMP, NULL);

		      set_opvar_int(tmppush2, splev->n_opcodes+1);

		      if_list[n_if_list++] = tmppush2;

		      add_opcode(splev, SPO_PUSH, tmppush2);

		      add_opcode(splev, reverse_jmp_opcode( (yyvsp[(2) - (2)].i) ), NULL);
		  }
    break;

  case 147:
#line 927 "lev_comp.y"
    {
		     /* do nothing */
		  }
    break;

  case 148:
#line 933 "lev_comp.y"
    {
		      if (n_if_list > 0) {
			  struct opvar *tmppush;
			  tmppush = (struct opvar *) if_list[--n_if_list];
			  set_opvar_int(tmppush, splev->n_opcodes - tmppush->vardata.l);
		      } else lc_error("IF: Huh?!  No start address?");
		  }
    break;

  case 149:
#line 941 "lev_comp.y"
    {
		      if (n_if_list > 0) {
			  struct opvar *tmppush = New(struct opvar);
			  struct opvar *tmppush2;

			  set_opvar_int(tmppush, splev->n_opcodes+1);
			  add_opcode(splev, SPO_PUSH, tmppush);

			  add_opcode(splev, SPO_JMP, NULL);

			  tmppush2 = (struct opvar *) if_list[--n_if_list];

			  set_opvar_int(tmppush2, splev->n_opcodes - tmppush2->vardata.l);
			  if_list[n_if_list++] = tmppush;
		      } else lc_error("IF: Huh?!  No else-part address?");
		  }
    break;

  case 150:
#line 958 "lev_comp.y"
    {
		      if (n_if_list > 0) {
			  struct opvar *tmppush;
			  tmppush = (struct opvar *) if_list[--n_if_list];
			  set_opvar_int(tmppush, splev->n_opcodes - tmppush->vardata.l);
		      } else lc_error("IF: Huh?! No end address?");
		  }
    break;

  case 151:
#line 968 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_MESSAGE);
		  }
    break;

  case 152:
#line 974 "lev_comp.y"
    {
		      add_opvars(splev, "mio", SP_MAPCHAR_PACK(ROOM,-2), (yyvsp[(6) - (6)].i), SPO_WALLWALK);
		  }
    break;

  case 153:
#line 978 "lev_comp.y"
    {
		      add_opvars(splev, "io", (yyvsp[(8) - (8)].i), SPO_WALLWALK);
		  }
    break;

  case 154:
#line 984 "lev_comp.y"
    {
		      add_opvars(splev, "iiiiiio", -1,  0, -1, -1, -1, -1, SPO_CORRIDOR);
		  }
    break;

  case 155:
#line 988 "lev_comp.y"
    {
		      add_opvars(splev, "iiiiiio", -1, (yyvsp[(3) - (3)].i), -1, -1, -1, -1, SPO_CORRIDOR);
		  }
    break;

  case 156:
#line 992 "lev_comp.y"
    {
		      add_opvars(splev, "iiiiiio", -1, -1, -1, -1, -1, -1, SPO_CORRIDOR);
		  }
    break;

  case 157:
#line 998 "lev_comp.y"
    {
		      add_opvars(splev, "iiiiiio",
				 (yyvsp[(3) - (5)].corpos).room, (yyvsp[(3) - (5)].corpos).door, (yyvsp[(3) - (5)].corpos).wall,
				 (yyvsp[(5) - (5)].corpos).room, (yyvsp[(5) - (5)].corpos).door, (yyvsp[(5) - (5)].corpos).wall,
				 SPO_CORRIDOR);
		  }
    break;

  case 158:
#line 1005 "lev_comp.y"
    {
		      add_opvars(splev, "iiiiiio",
				 (yyvsp[(3) - (5)].corpos).room, (yyvsp[(3) - (5)].corpos).door, (yyvsp[(3) - (5)].corpos).wall,
				 -1, -1, (long)(yyvsp[(5) - (5)].i),
				 SPO_CORRIDOR);
		  }
    break;

  case 159:
#line 1014 "lev_comp.y"
    {
			(yyval.corpos).room = (yyvsp[(2) - (7)].i);
			(yyval.corpos).wall = (yyvsp[(4) - (7)].i);
			(yyval.corpos).door = (yyvsp[(6) - (7)].i);
		  }
    break;

  case 160:
#line 1022 "lev_comp.y"
    {
		      if (((yyvsp[(2) - (4)].i) < 100) && ((yyvsp[(1) - (4)].i) == OROOM))
			  lc_error("Only typed rooms can have a chance.");
		      else {
			  add_opvars(splev, "iii", (long)(yyvsp[(1) - (4)].i), (long)(yyvsp[(2) - (4)].i), (long)(yyvsp[(4) - (4)].i));
		      }
                  }
    break;

  case 161:
#line 1032 "lev_comp.y"
    {
		      add_opvars(splev, "iiiiiiio", (long)(yyvsp[(8) - (8)].i), ERR, ERR,
				 (yyvsp[(5) - (8)].crd).x, (yyvsp[(5) - (8)].crd).y, (yyvsp[(7) - (8)].sze).width, (yyvsp[(7) - (8)].sze).height, SPO_SUBROOM);
		  }
    break;

  case 162:
#line 1037 "lev_comp.y"
    {
		      add_opcode(splev, SPO_ENDROOM, NULL);
		  }
    break;

  case 163:
#line 1043 "lev_comp.y"
    {
		      add_opvars(splev, "iiiiiiio", (long)(yyvsp[(10) - (10)].i),
				 (yyvsp[(7) - (10)].crd).x, (yyvsp[(7) - (10)].crd).y, (yyvsp[(5) - (10)].crd).x, (yyvsp[(5) - (10)].crd).y,
				 (yyvsp[(9) - (10)].sze).width, (yyvsp[(9) - (10)].sze).height, SPO_ROOM);
		  }
    break;

  case 164:
#line 1049 "lev_comp.y"
    {
		      add_opcode(splev, SPO_ENDROOM, NULL);
		  }
    break;

  case 165:
#line 1055 "lev_comp.y"
    {
			(yyval.i) = 1;
		  }
    break;

  case 166:
#line 1059 "lev_comp.y"
    {
			(yyval.i) = (yyvsp[(2) - (2)].i);
		  }
    break;

  case 167:
#line 1065 "lev_comp.y"
    {
			if ( (yyvsp[(2) - (5)].i) < 1 || (yyvsp[(2) - (5)].i) > 5 ||
			    (yyvsp[(4) - (5)].i) < 1 || (yyvsp[(4) - (5)].i) > 5 ) {
			    lc_error("Room positions should be between 1-5: (%li,%li)!", (yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i));
			} else {
			    (yyval.crd).x = (yyvsp[(2) - (5)].i);
			    (yyval.crd).y = (yyvsp[(4) - (5)].i);
			}
		  }
    break;

  case 168:
#line 1075 "lev_comp.y"
    {
			(yyval.crd).x = (yyval.crd).y = ERR;
		  }
    break;

  case 169:
#line 1081 "lev_comp.y"
    {
			if ( (yyvsp[(2) - (5)].i) < 0 || (yyvsp[(4) - (5)].i) < 0) {
			    lc_error("Invalid subroom position (%li,%li)!", (yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i));
			} else {
			    (yyval.crd).x = (yyvsp[(2) - (5)].i);
			    (yyval.crd).y = (yyvsp[(4) - (5)].i);
			}
		  }
    break;

  case 170:
#line 1090 "lev_comp.y"
    {
			(yyval.crd).x = (yyval.crd).y = ERR;
		  }
    break;

  case 171:
#line 1096 "lev_comp.y"
    {
		      (yyval.crd).x = (yyvsp[(2) - (5)].i);
		      (yyval.crd).y = (yyvsp[(4) - (5)].i);
		  }
    break;

  case 172:
#line 1101 "lev_comp.y"
    {
		      (yyval.crd).x = (yyval.crd).y = ERR;
		  }
    break;

  case 173:
#line 1107 "lev_comp.y"
    {
			(yyval.sze).width = (yyvsp[(2) - (5)].i);
			(yyval.sze).height = (yyvsp[(4) - (5)].i);
		  }
    break;

  case 174:
#line 1112 "lev_comp.y"
    {
			(yyval.sze).height = (yyval.sze).width = ERR;
		  }
    break;

  case 175:
#line 1118 "lev_comp.y"
    {
			/* ERR means random here */
			if ((yyvsp[(7) - (9)].i) == ERR && (yyvsp[(9) - (9)].i) != ERR) {
			    lc_error("If the door wall is random, so must be its pos!");
			} else {
			    add_opvars(splev, "iiiio", (long)(yyvsp[(9) - (9)].i), (long)(yyvsp[(5) - (9)].i), (long)(yyvsp[(3) - (9)].i), (long)(yyvsp[(7) - (9)].i), SPO_ROOM_DOOR);
			}
		  }
    break;

  case 176:
#line 1127 "lev_comp.y"
    {
		      add_opvars(splev, "io", (long)(yyvsp[(3) - (5)].i), SPO_DOOR);
		  }
    break;

  case 181:
#line 1141 "lev_comp.y"
    {
		      (yyval.i) = (yyvsp[(1) - (1)].i);
		  }
    break;

  case 182:
#line 1145 "lev_comp.y"
    {
		      (yyval.i) = ((yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i));
		  }
    break;

  case 185:
#line 1155 "lev_comp.y"
    {
		      add_opvars(splev, "ciisiio", 0, 0, 1, (char *)0, 0, 0, SPO_MAP);
		      max_x_map = COLNO-1;
		      max_y_map = ROWNO;
		  }
    break;

  case 186:
#line 1161 "lev_comp.y"
    {
		      add_opvars(splev, "cii", ((long)((yyvsp[(1) - (3)].i) % 10) & 0xff) + (((long)((yyvsp[(1) - (3)].i) / 10) & 0xff) << 16), 1, (long)(yyvsp[(2) - (3)].i));
		      scan_map((yyvsp[(3) - (3)].map), splev);
		      Free((yyvsp[(3) - (3)].map));
		  }
    break;

  case 187:
#line 1167 "lev_comp.y"
    {
		      add_opvars(splev, "ii", 2, (long)(yyvsp[(4) - (5)].i));
		      scan_map((yyvsp[(5) - (5)].map), splev);
		      Free((yyvsp[(5) - (5)].map));
		  }
    break;

  case 188:
#line 1175 "lev_comp.y"
    {
			(yyval.i) = (yyvsp[(3) - (5)].i) + ((yyvsp[(5) - (5)].i) * 10);
		  }
    break;

  case 193:
#line 1189 "lev_comp.y"
    {
		      long n_sounds = (yyvsp[(5) - (5)].i);
		      add_opvars(splev, "io", n_sounds, SPO_LEVEL_SOUNDS);
		  }
    break;

  case 194:
#line 1196 "lev_comp.y"
    {
		      (yyval.i) = 1;
		  }
    break;

  case 195:
#line 1200 "lev_comp.y"
    {
		      (yyval.i) = 1 + (yyvsp[(3) - (3)].i);
		  }
    break;

  case 196:
#line 1206 "lev_comp.y"
    {
		      add_opvars(splev, "i", (long)(yyvsp[(2) - (5)].i));
		  }
    break;

  case 197:
#line 1212 "lev_comp.y"
    {
		      long chance = (yyvsp[(3) - (5)].i);
		      long total_mons = (yyvsp[(5) - (5)].i);
		      if (chance < 0) chance = 0;
		      else if (chance > 100) chance = 100;

		      if (total_mons < 1) lc_error("Monster generation: zero monsters defined?");
		      add_opvars(splev, "iio", chance, total_mons, SPO_MON_GENERATION);
		  }
    break;

  case 198:
#line 1224 "lev_comp.y"
    {
		      (yyval.i) = 1;
		  }
    break;

  case 199:
#line 1228 "lev_comp.y"
    {
		      (yyval.i) = 1 + (yyvsp[(3) - (3)].i);
		  }
    break;

  case 200:
#line 1234 "lev_comp.y"
    {
		      long token = (yyvsp[(4) - (5)].i);
		      if (token == ERR) lc_error("Monster generation: Invalid monster symbol");
		      add_opvars(splev, "ii", token, 1);
		  }
    break;

  case 201:
#line 1240 "lev_comp.y"
    {
		      long token;
		      token = get_monster_id((yyvsp[(4) - (5)].map), (char)0);
		      if (token == ERR) lc_error("Monster generation: Invalid monster name");
		      add_opvars(splev, "ii", token, 0);
		  }
    break;

  case 202:
#line 1249 "lev_comp.y"
    {
		      add_opvars(splev, "io", 0, SPO_MONSTER);

		      if ( 1 == (yyvsp[(2) - (4)].i) ) {
			  if (n_if_list > 0) {
			      struct opvar *tmpjmp;
			      tmpjmp = (struct opvar *) if_list[--n_if_list];
			      set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			  } else lc_error("Conditional creation of monster, but no jump point marker.");
		      }
		  }
    break;

  case 203:
#line 1261 "lev_comp.y"
    {
		      add_opvars(splev, "io", 1, SPO_MONSTER);
		      (yyval.i) = (yyvsp[(2) - (4)].i);
		      in_container_obj++;
		  }
    break;

  case 204:
#line 1267 "lev_comp.y"
    {
		     in_container_obj--;
		     add_opvars(splev, "o", SPO_END_MONINVENT);
		     if ( 1 == (yyvsp[(5) - (8)].i) ) {
			 if (n_if_list > 0) {
			     struct opvar *tmpjmp;
			     tmpjmp = (struct opvar *) if_list[--n_if_list];
			     set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			 } else lc_error("Conditional creation of monster, but no jump point marker.");
		     }
		 }
    break;

  case 205:
#line 1281 "lev_comp.y"
    {
		      /* nothing */
		  }
    break;

  case 206:
#line 1287 "lev_comp.y"
    {
		      struct opvar *stopit = New(struct opvar);
		      set_opvar_int(stopit, SP_M_V_END);
		      add_opcode(splev, SPO_PUSH, stopit);
		      (yyval.i) = 0x0000;
		  }
    break;

  case 207:
#line 1294 "lev_comp.y"
    {
		      if (( (yyvsp[(1) - (3)].i) & (yyvsp[(3) - (3)].i) ))
			  lc_error("MONSTER extra info defined twice.");
		      (yyval.i) = ( (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i) );
		  }
    break;

  case 208:
#line 1302 "lev_comp.y"
    {
		      add_opvars(splev, "i", SP_M_V_NAME);
		      (yyval.i) = 0x0001;
		  }
    break;

  case 209:
#line 1307 "lev_comp.y"
    {
		      add_opvars(splev, "ii", (long)(yyvsp[(1) - (1)].i), SP_M_V_PEACEFUL);
		      (yyval.i) = 0x0002;
		  }
    break;

  case 210:
#line 1312 "lev_comp.y"
    {
		      add_opvars(splev, "ii", (long)(yyvsp[(1) - (1)].i), SP_M_V_ASLEEP);
		      (yyval.i) = 0x0004;
		  }
    break;

  case 211:
#line 1317 "lev_comp.y"
    {
		      add_opvars(splev, "ii", (long)(yyvsp[(1) - (1)].i), SP_M_V_ALIGN);
		      (yyval.i) = 0x0008;
		  }
    break;

  case 212:
#line 1322 "lev_comp.y"
    {
		      add_opvars(splev, "ii", (long)(yyvsp[(1) - (2)].i), SP_M_V_APPEAR);
		      (yyval.i) = 0x0010;
		  }
    break;

  case 213:
#line 1327 "lev_comp.y"
    {
		      add_opvars(splev, "ii", 1, SP_M_V_FEMALE);
		      (yyval.i) = 0x0020;
		  }
    break;

  case 214:
#line 1332 "lev_comp.y"
    {
		      add_opvars(splev, "ii", 1, SP_M_V_INVIS);
		      (yyval.i) = 0x0040;
		  }
    break;

  case 215:
#line 1337 "lev_comp.y"
    {
		      add_opvars(splev, "ii", 1, SP_M_V_CANCELLED);
		      (yyval.i) = 0x0080;
		  }
    break;

  case 216:
#line 1342 "lev_comp.y"
    {
		      add_opvars(splev, "ii", 1, SP_M_V_REVIVED);
		      (yyval.i) = 0x0100;
		  }
    break;

  case 217:
#line 1347 "lev_comp.y"
    {
		      add_opvars(splev, "ii", 1, SP_M_V_AVENGE);
		      (yyval.i) = 0x0200;
		  }
    break;

  case 218:
#line 1352 "lev_comp.y"
    {
		      add_opvars(splev, "i", SP_M_V_FLEEING);
		      (yyval.i) = 0x0400;
		  }
    break;

  case 219:
#line 1357 "lev_comp.y"
    {
		      add_opvars(splev, "i", SP_M_V_BLINDED);
		      (yyval.i) = 0x0800;
		  }
    break;

  case 220:
#line 1362 "lev_comp.y"
    {
		      add_opvars(splev, "i", SP_M_V_PARALYZED);
		      (yyval.i) = 0x1000;
		  }
    break;

  case 221:
#line 1367 "lev_comp.y"
    {
		      add_opvars(splev, "ii", 1, SP_M_V_STUNNED);
		      (yyval.i) = 0x2000;
		  }
    break;

  case 222:
#line 1372 "lev_comp.y"
    {
		      add_opvars(splev, "ii", 1, SP_M_V_CONFUSED);
		      (yyval.i) = 0x4000;
		  }
    break;

  case 223:
#line 1377 "lev_comp.y"
    {
		      add_opvars(splev, "ii", (long)(yyvsp[(3) - (3)].i), SP_M_V_SEENTRAPS);
		      (yyval.i) = 0x8000;
		  }
    break;

  case 224:
#line 1384 "lev_comp.y"
    {
		      int token = get_trap_type((yyvsp[(1) - (1)].map));
		      if (token == ERR || token == 0)
			  lc_error("Unknown trap type '%s'!", (yyvsp[(1) - (1)].map));
		      (yyval.i) = (1L << (token - 1));
		  }
    break;

  case 225:
#line 1391 "lev_comp.y"
    {
		      (yyval.i) = (long) ~0;
		  }
    break;

  case 226:
#line 1395 "lev_comp.y"
    {
		      int token = get_trap_type((yyvsp[(1) - (3)].map));
		      if (token == ERR || token == 0)
			  lc_error("Unknown trap type '%s'!", (yyvsp[(1) - (3)].map));

		      if ((1L << (token - 1)) & (yyvsp[(3) - (3)].i))
			  lc_error("Monster seen_traps, trap '%s' listed twice.", (yyvsp[(1) - (3)].map));

		      (yyval.i) = ((1L << (token - 1)) | (yyvsp[(3) - (3)].i));
		  }
    break;

  case 227:
#line 1408 "lev_comp.y"
    {
		      long cnt = 0;
		      if (in_container_obj) cnt |= SP_OBJ_CONTENT;
		      add_opvars(splev, "io", cnt, SPO_OBJECT);
		      if ( 1 == (yyvsp[(2) - (4)].i) ) {
			  if (n_if_list > 0) {
			      struct opvar *tmpjmp;
			      tmpjmp = (struct opvar *) if_list[--n_if_list];
			      set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			  } else lc_error("conditional creation of obj, but no jump point marker.");
		      }
		  }
    break;

  case 228:
#line 1421 "lev_comp.y"
    {
		      long cnt = SP_OBJ_CONTAINER;
		      if (in_container_obj) cnt |= SP_OBJ_CONTENT;
		      add_opvars(splev, "io", cnt, SPO_OBJECT);
		      (yyval.i) = (yyvsp[(2) - (4)].i);
		      in_container_obj++;
		  }
    break;

  case 229:
#line 1429 "lev_comp.y"
    {
		     in_container_obj--;
		     add_opcode(splev, SPO_POP_CONTAINER, NULL);

		     if ( 1 == (yyvsp[(5) - (8)].i) ) {
			 if (n_if_list > 0) {
			     struct opvar *tmpjmp;
			     tmpjmp = (struct opvar *) if_list[--n_if_list];
			     set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			 } else lc_error("Conditional creation of obj, but no jump point marker.");
		     }
		 }
    break;

  case 230:
#line 1444 "lev_comp.y"
    {
		      if (( (yyvsp[(2) - (2)].i) & 0x4000) && in_container_obj) lc_error("Object cannot have a coord when contained.");
		      else if (!( (yyvsp[(2) - (2)].i) & 0x4000) && !in_container_obj) lc_error("Object needs a coord when not contained.");
		  }
    break;

  case 231:
#line 1451 "lev_comp.y"
    {
		      struct opvar *stopit = New(struct opvar);
		      set_opvar_int(stopit, SP_O_V_END);
		      add_opcode(splev, SPO_PUSH, stopit);
		      (yyval.i) = 0x00;
		  }
    break;

  case 232:
#line 1458 "lev_comp.y"
    {
		      if (( (yyvsp[(1) - (3)].i) & (yyvsp[(3) - (3)].i) ))
			  lc_error("OBJECT extra info defined twice.");
		      (yyval.i) = ( (yyvsp[(1) - (3)].i) | (yyvsp[(3) - (3)].i) );
		  }
    break;

  case 233:
#line 1466 "lev_comp.y"
    {
		      add_opvars(splev, "ii", (long)(yyvsp[(1) - (1)].i), SP_O_V_CURSE);
		      (yyval.i) = 0x0001;
		  }
    break;

  case 234:
#line 1471 "lev_comp.y"
    {
		      add_opvars(splev, "i", SP_O_V_CORPSENM);
		      (yyval.i) = 0x0002;
		  }
    break;

  case 235:
#line 1476 "lev_comp.y"
    {
		      add_opvars(splev, "i", SP_O_V_SPE);
		      (yyval.i) = 0x0004;
		  }
    break;

  case 236:
#line 1481 "lev_comp.y"
    {
		      add_opvars(splev, "i", SP_O_V_NAME);
		      (yyval.i) = 0x0008;
		  }
    break;

  case 237:
#line 1486 "lev_comp.y"
    {
		      add_opvars(splev, "i", SP_O_V_QUAN);
		      (yyval.i) = 0x0010;
		  }
    break;

  case 238:
#line 1491 "lev_comp.y"
    {
		      add_opvars(splev, "ii", 1, SP_O_V_BURIED);
		      (yyval.i) = 0x0020;
		  }
    break;

  case 239:
#line 1496 "lev_comp.y"
    {
		      add_opvars(splev, "ii", (long)(yyvsp[(1) - (1)].i), SP_O_V_LIT);
		      (yyval.i) = 0x0040;
		  }
    break;

  case 240:
#line 1501 "lev_comp.y"
    {
		      add_opvars(splev, "i", SP_O_V_ERODED);
		      (yyval.i) = 0x0080;
		  }
    break;

  case 241:
#line 1506 "lev_comp.y"
    {
		      add_opvars(splev, "ii", -1, SP_O_V_ERODED);
		      (yyval.i) = 0x0080;
		  }
    break;

  case 242:
#line 1511 "lev_comp.y"
    {
		      if ((yyvsp[(1) - (1)].i) == D_LOCKED) {
			  add_opvars(splev, "ii", 1, SP_O_V_LOCKED);
			  (yyval.i) = 0x0100;
		      } else if ((yyvsp[(1) - (1)].i) == D_BROKEN) {
			  add_opvars(splev, "ii", 1, SP_O_V_BROKEN);
			  (yyval.i) = 0x0200;
		      } else
			  lc_error("OBJECT state can only be locked or broken.");
		  }
    break;

  case 243:
#line 1522 "lev_comp.y"
    {
		      add_opvars(splev, "ii", 1, SP_O_V_TRAPPED);
		      (yyval.i) = 0x0400;
		  }
    break;

  case 244:
#line 1527 "lev_comp.y"
    {
		      add_opvars(splev, "i", SP_O_V_RECHARGED);
		      (yyval.i) = 0x0800;
		  }
    break;

  case 245:
#line 1532 "lev_comp.y"
    {
		      add_opvars(splev, "ii", 1, SP_O_V_INVIS);
		      (yyval.i) = 0x1000;
		  }
    break;

  case 246:
#line 1537 "lev_comp.y"
    {
		      add_opvars(splev, "ii", 1, SP_O_V_GREASED);
		      (yyval.i) = 0x2000;
		  }
    break;

  case 247:
#line 1542 "lev_comp.y"
    {
		      add_opvars(splev, "i", SP_O_V_COORD);
		      (yyval.i) = 0x4000;
		  }
    break;

  case 248:
#line 1549 "lev_comp.y"
    {
		      add_opvars(splev, "io", (long)(yyvsp[(4) - (6)].i), SPO_TRAP);
		      if ( 1 == (yyvsp[(2) - (6)].i) ) {
			  if (n_if_list > 0) {
			      struct opvar *tmpjmp;
			      tmpjmp = (struct opvar *) if_list[--n_if_list];
			      set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			  } else lc_error("Conditional creation of trap, but no jump point marker.");
		      }
		  }
    break;

  case 249:
#line 1562 "lev_comp.y"
    {
		       long d, state = 0;
		       /* convert dir from a DIRECTION to a DB_DIR */
		       d = (yyvsp[(5) - (7)].i);
		       switch(d) {
		       case W_NORTH: d = DB_NORTH; break;
		       case W_SOUTH: d = DB_SOUTH; break;
		       case W_EAST:  d = DB_EAST;  break;
		       case W_WEST:  d = DB_WEST;  break;
		       default:
			   lc_error("Invalid drawbridge direction.");
			   break;
		       }

		       if ( (yyvsp[(7) - (7)].i) == D_ISOPEN )
			   state = 1;
		       else if ( (yyvsp[(7) - (7)].i) == D_CLOSED )
			   state = 0;
		       else
			   lc_error("A drawbridge can only be open or closed!");
		       add_opvars(splev, "iio", state, d, SPO_DRAWBRIDGE);
		   }
    break;

  case 250:
#line 1587 "lev_comp.y"
    {
		      add_opvars(splev, "iiio",
				 (long)(yyvsp[(5) - (5)].i), 1, 0, SPO_MAZEWALK);
		  }
    break;

  case 251:
#line 1592 "lev_comp.y"
    {
		      add_opvars(splev, "iiio",
				 (long)(yyvsp[(5) - (8)].i), (long)(yyvsp[(7) - (8)].i), (long)(yyvsp[(8) - (8)].i), SPO_MAZEWALK);
		  }
    break;

  case 252:
#line 1599 "lev_comp.y"
    {
		      add_opvars(splev, "ro", SP_REGION_PACK(-1,-1,-1,-1), SPO_WALLIFY);
		  }
    break;

  case 253:
#line 1603 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_WALLIFY);
		  }
    break;

  case 254:
#line 1609 "lev_comp.y"
    {
		      add_opvars(splev, "io", (long)(yyvsp[(5) - (5)].i), SPO_LADDER);
		  }
    break;

  case 255:
#line 1615 "lev_comp.y"
    {
		      add_opvars(splev, "io", (long)(yyvsp[(5) - (5)].i), SPO_STAIR);
		  }
    break;

  case 256:
#line 1621 "lev_comp.y"
    {
		      add_opvars(splev, "iiiii iiiii iiso",
				 (yyvsp[(3) - (7)].lregn).x1, (yyvsp[(3) - (7)].lregn).y1, (yyvsp[(3) - (7)].lregn).x2, (yyvsp[(3) - (7)].lregn).y2, (yyvsp[(3) - (7)].lregn).area,
				 (yyvsp[(5) - (7)].lregn).x1, (yyvsp[(5) - (7)].lregn).y1, (yyvsp[(5) - (7)].lregn).x2, (yyvsp[(5) - (7)].lregn).y2, (yyvsp[(5) - (7)].lregn).area,
				 (long)(((yyvsp[(7) - (7)].i)) ? LR_UPSTAIR : LR_DOWNSTAIR),
				 0, (char *)0, SPO_LEVREGION);
		  }
    break;

  case 257:
#line 1631 "lev_comp.y"
    {
		      add_opvars(splev, "iiiii iiiii iiso",
				 (yyvsp[(3) - (7)].lregn).x1, (yyvsp[(3) - (7)].lregn).y1, (yyvsp[(3) - (7)].lregn).x2, (yyvsp[(3) - (7)].lregn).y2, (yyvsp[(3) - (7)].lregn).area,
				 (yyvsp[(5) - (7)].lregn).x1, (yyvsp[(5) - (7)].lregn).y1, (yyvsp[(5) - (7)].lregn).x2, (yyvsp[(5) - (7)].lregn).y2, (yyvsp[(5) - (7)].lregn).area,
				 LR_PORTAL, 0, (yyvsp[(7) - (7)].map), SPO_LEVREGION);
		      Free((yyvsp[(7) - (7)].map));
		  }
    break;

  case 258:
#line 1641 "lev_comp.y"
    {
		      long rtype;
		      switch((yyvsp[(6) - (6)].i)) {
		      case -1: rtype = LR_TELE; break;
		      case  0: rtype = LR_DOWNTELE; break;
		      case  1: rtype = LR_UPTELE; break;
		      }
		      add_opvars(splev, "iiiii iiiii iiso",
				 (yyvsp[(3) - (6)].lregn).x1, (yyvsp[(3) - (6)].lregn).y1, (yyvsp[(3) - (6)].lregn).x2, (yyvsp[(3) - (6)].lregn).y2, (yyvsp[(3) - (6)].lregn).area,
				 (yyvsp[(5) - (6)].lregn).x1, (yyvsp[(5) - (6)].lregn).y1, (yyvsp[(5) - (6)].lregn).x2, (yyvsp[(5) - (6)].lregn).y2, (yyvsp[(5) - (6)].lregn).area,
				 rtype, 0, (char *)0, SPO_LEVREGION);
		  }
    break;

  case 259:
#line 1656 "lev_comp.y"
    {
		      add_opvars(splev, "iiiii iiiii iiso",
				 (yyvsp[(3) - (5)].lregn).x1, (yyvsp[(3) - (5)].lregn).y1, (yyvsp[(3) - (5)].lregn).x2, (yyvsp[(3) - (5)].lregn).y2, (yyvsp[(3) - (5)].lregn).area,
				 (yyvsp[(5) - (5)].lregn).x1, (yyvsp[(5) - (5)].lregn).y1, (yyvsp[(5) - (5)].lregn).x2, (yyvsp[(5) - (5)].lregn).y2, (yyvsp[(5) - (5)].lregn).area,
				 (long)LR_BRANCH, 0, (char *)0, SPO_LEVREGION);
		  }
    break;

  case 260:
#line 1665 "lev_comp.y"
    {
			(yyval.i) = -1;
		  }
    break;

  case 261:
#line 1669 "lev_comp.y"
    {
			(yyval.i) = (yyvsp[(2) - (2)].i);
		  }
    break;

  case 262:
#line 1675 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_FOUNTAIN);
		  }
    break;

  case 263:
#line 1681 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_SINK);
		  }
    break;

  case 264:
#line 1687 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_POOL);
		  }
    break;

  case 265:
#line 1693 "lev_comp.y"
    {
		      (yyval.terr).lit = -2;
		      (yyval.terr).ter = what_map_char((char) (yyvsp[(1) - (1)].i));
		  }
    break;

  case 266:
#line 1698 "lev_comp.y"
    {
		      (yyval.terr).lit = (yyvsp[(4) - (5)].i);
		      (yyval.terr).ter = what_map_char((char) (yyvsp[(2) - (5)].i));
		  }
    break;

  case 267:
#line 1705 "lev_comp.y"
    {
		      long chance;

		      chance = (yyvsp[(9) - (9)].i);
		      if (chance < 0) chance = 0;
		      else if (chance > 100) chance = 100;
		      add_opvars(splev, "io", chance, SPO_REPLACETERRAIN);
		  }
    break;

  case 268:
#line 1716 "lev_comp.y"
    {
		     add_opvars(splev, "o", SPO_TERRAIN);

		     if ( 1 == (yyvsp[(2) - (6)].i) ) {
			 if (n_if_list > 0) {
			     struct opvar *tmpjmp;
			     tmpjmp = (struct opvar *) if_list[--n_if_list];
			     set_opvar_int(tmpjmp, splev->n_opcodes - tmpjmp->vardata.l);
			 } else lc_error("Conditional terrain modification, but no jump point marker.");
		     }
		 }
    break;

  case 269:
#line 1730 "lev_comp.y"
    {
		    long c, typ;

		    typ = (yyvsp[(5) - (9)].terr).ter;
		    if (typ == INVALID_TYPE || typ >= MAX_TYPE) {
			lc_error("SPILL: Invalid map character!");
		    }

		    c = (yyvsp[(9) - (9)].i);
		    if (c < 1) lc_error("SPILL: Invalid count '%li'!", c);

		    add_opvars(splev, "iiiio", typ, (long)(yyvsp[(7) - (9)].i), c, (long)(yyvsp[(5) - (9)].terr).lit, SPO_SPILL);
		}
    break;

  case 270:
#line 1746 "lev_comp.y"
    {
		     add_opvars(splev, "o", SPO_NON_DIGGABLE);
		  }
    break;

  case 271:
#line 1752 "lev_comp.y"
    {
		     add_opvars(splev, "o", SPO_NON_PASSWALL);
		  }
    break;

  case 272:
#line 1758 "lev_comp.y"
    {
		      long rt, irr;
		      rt = (yyvsp[(7) - (8)].i);
		      if (( (yyvsp[(8) - (8)].i) ) & 1) rt += MAXRTYPE+1;
		      irr = ((( (yyvsp[(8) - (8)].i) ) & 2) != 0);
		      add_opvars(splev, "iiio",
				 (long)(yyvsp[(5) - (8)].i), rt, irr, SPO_REGION);
		      (yyval.i) = (irr || ((yyvsp[(8) - (8)].i) & 1) || rt != OROOM);
		  }
    break;

  case 273:
#line 1768 "lev_comp.y"
    {
		      if ( (yyvsp[(9) - (10)].i) ) {
			  add_opcode(splev, SPO_ENDROOM, NULL);
		      } else if ( (yyvsp[(10) - (10)].i) )
			  lc_error("Cannot use lev statements in non-permanent REGION");
		  }
    break;

  case 274:
#line 1777 "lev_comp.y"
    {
		      (yyval.i) = 0;
		  }
    break;

  case 275:
#line 1781 "lev_comp.y"
    {
		      (yyval.i) = (yyvsp[(2) - (3)].i);
		  }
    break;

  case 276:
#line 1787 "lev_comp.y"
    {
		      add_opvars(splev, "iio", (long)(yyvsp[(7) - (7)].i), (long)(yyvsp[(5) - (7)].i), SPO_ALTAR);
		  }
    break;

  case 277:
#line 1793 "lev_comp.y"
    {
		      add_opvars(splev, "io", 2, SPO_GRAVE);
		  }
    break;

  case 278:
#line 1797 "lev_comp.y"
    {
		      add_opvars(splev, "sio",
				 (char *)0, 1, SPO_GRAVE);
		  }
    break;

  case 279:
#line 1802 "lev_comp.y"
    {
		      add_opvars(splev, "sio",
				 (char *)0, 0, SPO_GRAVE);
		  }
    break;

  case 280:
#line 1809 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_GOLD);
		  }
    break;

  case 281:
#line 1815 "lev_comp.y"
    {
		      add_opvars(splev, "io",
				 (long)(yyvsp[(5) - (7)].i), SPO_ENGRAVING);
		  }
    break;

  case 282:
#line 1822 "lev_comp.y"
    {
			int token = get_trap_type((yyvsp[(1) - (1)].map));
			if (token == ERR)
			    lc_error("Unknown trap type '%s'!", (yyvsp[(1) - (1)].map));
			(yyval.i) = token;
			Free((yyvsp[(1) - (1)].map));
		  }
    break;

  case 284:
#line 1833 "lev_comp.y"
    {
			int token = get_room_type((yyvsp[(1) - (1)].map));
			if (token == ERR) {
			    lc_warning("Unknown room type \"%s\"!  Making ordinary room...", (yyvsp[(1) - (1)].map));
				(yyval.i) = OROOM;
			} else
				(yyval.i) = token;
			Free((yyvsp[(1) - (1)].map));
		  }
    break;

  case 286:
#line 1846 "lev_comp.y"
    {
			(yyval.i) = 0;
		  }
    break;

  case 287:
#line 1850 "lev_comp.y"
    {
			(yyval.i) = (yyvsp[(2) - (2)].i);
		  }
    break;

  case 288:
#line 1854 "lev_comp.y"
    {
			(yyval.i) = (yyvsp[(2) - (4)].i) + ((yyvsp[(4) - (4)].i) << 1);
		  }
    break;

  case 295:
#line 1870 "lev_comp.y"
    {
			(yyval.i) = - MAX_REGISTERS - 1;
		  }
    break;

  case 298:
#line 1878 "lev_comp.y"
    {
			(yyval.i) = - MAX_REGISTERS - 1;
		  }
    break;

  case 301:
#line 1888 "lev_comp.y"
    {
			if ( (yyvsp[(3) - (4)].i) >= 3 )
				lc_error("Register Index overflow!");
			else
				(yyval.i) = - (yyvsp[(3) - (4)].i) - 1;
		  }
    break;

  case 302:
#line 1897 "lev_comp.y"
    {
			if (check_monster_char((char) (yyvsp[(1) - (1)].i)))
				(yyval.i) = (yyvsp[(1) - (1)].i) ;
			else {
			    lc_error("Unknown monster class '%c'!", (yyvsp[(1) - (1)].i));
			    (yyval.i) = ERR;
			}
		  }
    break;

  case 303:
#line 1908 "lev_comp.y"
    {
		      add_opvars(splev, "s", (yyvsp[(1) - (1)].map));
		      Free((yyvsp[(1) - (1)].map));
		  }
    break;

  case 304:
#line 1913 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (1)].map), SPOVAR_STRING);
		      add_opvars(splev, "v", (yyvsp[(1) - (1)].map));
		      Free((yyvsp[(1) - (1)].map));
		  }
    break;

  case 305:
#line 1919 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (4)].map), SPOVAR_STRING|SPOVAR_ARRAY);
		      add_opvars(splev, "v", (yyvsp[(1) - (4)].map));
		      Free((yyvsp[(1) - (4)].map));
		  }
    break;

  case 306:
#line 1928 "lev_comp.y"
    {
		      /* nothing */
		  }
    break;

  case 307:
#line 1934 "lev_comp.y"
    {
		      add_opvars(splev, "c", (yyvsp[(1) - (1)].i));
		  }
    break;

  case 308:
#line 1938 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_SEL_RNDCOORD);
		  }
    break;

  case 309:
#line 1942 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (1)].map), SPOVAR_COORD);
		      add_opvars(splev, "v", (yyvsp[(1) - (1)].map));
		      Free((yyvsp[(1) - (1)].map));
		  }
    break;

  case 310:
#line 1948 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (4)].map), SPOVAR_COORD|SPOVAR_ARRAY);
		      add_opvars(splev, "v", (yyvsp[(1) - (4)].map));
		      Free((yyvsp[(1) - (4)].map));
		  }
    break;

  case 311:
#line 1956 "lev_comp.y"
    {
		      if ((yyvsp[(2) - (5)].i) < 0 || (yyvsp[(4) - (5)].i) < 0 || (yyvsp[(2) - (5)].i) >= COLNO || (yyvsp[(4) - (5)].i) >= ROWNO)
			  lc_error("Coordinates (%li,%li) out of map range!", (yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i));
		      (yyval.i) = SP_COORD_PACK((yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].i));
		  }
    break;

  case 312:
#line 1962 "lev_comp.y"
    {
		      (yyval.i) = SP_COORD_PACK(-1,-1);
		  }
    break;

  case 313:
#line 1968 "lev_comp.y"
    {
		      add_opvars(splev, "r", (yyvsp[(1) - (1)].i));
		  }
    break;

  case 314:
#line 1972 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (1)].map), SPOVAR_REGION);
		      add_opvars(splev, "v", (yyvsp[(1) - (1)].map));
		      Free((yyvsp[(1) - (1)].map));
		  }
    break;

  case 315:
#line 1978 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (4)].map), SPOVAR_REGION|SPOVAR_ARRAY);
		      add_opvars(splev, "v", (yyvsp[(1) - (4)].map));
		      Free((yyvsp[(1) - (4)].map));
		  }
    break;

  case 316:
#line 1986 "lev_comp.y"
    {
		      if ( (yyvsp[(2) - (9)].i) > (yyvsp[(6) - (9)].i) || (yyvsp[(4) - (9)].i) > (yyvsp[(8) - (9)].i) )
			  lc_error("Region start > end: (%li,%li,%li,%li)!", (yyvsp[(2) - (9)].i), (yyvsp[(4) - (9)].i), (yyvsp[(6) - (9)].i), (yyvsp[(8) - (9)].i));

		      (yyval.i) = SP_REGION_PACK((yyvsp[(2) - (9)].i), (yyvsp[(4) - (9)].i), (yyvsp[(6) - (9)].i), (yyvsp[(8) - (9)].i));
		  }
    break;

  case 317:
#line 1995 "lev_comp.y"
    {
		      add_opvars(splev, "m", (yyvsp[(1) - (1)].i));
		  }
    break;

  case 318:
#line 1999 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (1)].map), SPOVAR_MAPCHAR);
		      add_opvars(splev, "v", (yyvsp[(1) - (1)].map));
		      Free((yyvsp[(1) - (1)].map));
		  }
    break;

  case 319:
#line 2005 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (4)].map), SPOVAR_MAPCHAR|SPOVAR_ARRAY);
		      add_opvars(splev, "v", (yyvsp[(1) - (4)].map));
		      Free((yyvsp[(1) - (4)].map));
		  }
    break;

  case 320:
#line 2013 "lev_comp.y"
    {
		      if (what_map_char((char) (yyvsp[(1) - (1)].i)) != INVALID_TYPE)
			  (yyval.i) = SP_MAPCHAR_PACK(what_map_char((char) (yyvsp[(1) - (1)].i)), -2);
		      else {
			  lc_error("Unknown map char type '%c'!", (yyvsp[(1) - (1)].i));
			  (yyval.i) = SP_MAPCHAR_PACK(STONE, -2);
		      }
		  }
    break;

  case 321:
#line 2022 "lev_comp.y"
    {
		      if (what_map_char((char) (yyvsp[(2) - (5)].i)) != INVALID_TYPE)
			  (yyval.i) = SP_MAPCHAR_PACK(what_map_char((char) (yyvsp[(2) - (5)].i)), (yyvsp[(4) - (5)].i));
		      else {
			  lc_error("Unknown map char type '%c'!", (yyvsp[(2) - (5)].i));
			  (yyval.i) = SP_MAPCHAR_PACK(STONE, (yyvsp[(4) - (5)].i));
		      }
		  }
    break;

  case 322:
#line 2033 "lev_comp.y"
    {
		      add_opvars(splev, "M", (yyvsp[(1) - (1)].i));
		  }
    break;

  case 323:
#line 2037 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (1)].map), SPOVAR_MONST);
		      add_opvars(splev, "v", (yyvsp[(1) - (1)].map));
		      Free((yyvsp[(1) - (1)].map));
		  }
    break;

  case 324:
#line 2043 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (4)].map), SPOVAR_MONST|SPOVAR_ARRAY);
		      add_opvars(splev, "v", (yyvsp[(1) - (4)].map));
		      Free((yyvsp[(1) - (4)].map));
		  }
    break;

  case 325:
#line 2051 "lev_comp.y"
    {
		      long m = get_monster_id((yyvsp[(1) - (1)].map), (char)0);
		      if (m == ERR) {
			  lc_error("Unknown monster \"%s\"!", (yyvsp[(1) - (1)].map));
			  (yyval.i) == -1;
		      } else
			  (yyval.i) = SP_MONST_PACK(m, def_monsyms[(int)mons[m].mlet]);
		  }
    break;

  case 326:
#line 2060 "lev_comp.y"
    {
			if (check_monster_char((char) (yyvsp[(1) - (1)].i)))
			    (yyval.i) = SP_MONST_PACK(-1, (yyvsp[(1) - (1)].i));
			else {
			    lc_error("Unknown monster class '%c'!", (yyvsp[(1) - (1)].i));
			    (yyval.i) = -1;
			}
		  }
    break;

  case 327:
#line 2069 "lev_comp.y"
    {
		      long m = get_monster_id((yyvsp[(4) - (5)].map), (char) (yyvsp[(2) - (5)].i));
		      if (m == ERR) {
			  lc_error("Unknown monster ('%c', \"%s\")!", (yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].map));
			  (yyval.i) == -1;
		      } else
			  (yyval.i) = SP_MONST_PACK(m, (yyvsp[(2) - (5)].i));
		  }
    break;

  case 328:
#line 2078 "lev_comp.y"
    {
		      (yyval.i) = -1;
		  }
    break;

  case 329:
#line 2084 "lev_comp.y"
    {
		      add_opvars(splev, "O", (yyvsp[(1) - (1)].i));
		  }
    break;

  case 330:
#line 2088 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (1)].map), SPOVAR_OBJ);
		      add_opvars(splev, "v", (yyvsp[(1) - (1)].map));
		      Free((yyvsp[(1) - (1)].map));
		  }
    break;

  case 331:
#line 2094 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (4)].map), SPOVAR_OBJ|SPOVAR_ARRAY);
		      add_opvars(splev, "v", (yyvsp[(1) - (4)].map));
		      Free((yyvsp[(1) - (4)].map));
		  }
    break;

  case 332:
#line 2102 "lev_comp.y"
    {
		      long m = get_object_id((yyvsp[(1) - (1)].map), (char)0);
		      if (m == ERR) {
			  lc_error("Unknown object \"%s\"!", (yyvsp[(1) - (1)].map));
			  (yyval.i) == -1;
		      } else
			  (yyval.i) = SP_OBJ_PACK(m, 1); /* obj class != 0 to force generation of a specific item */

		  }
    break;

  case 333:
#line 2112 "lev_comp.y"
    {
			if (check_object_char((char) (yyvsp[(1) - (1)].i)))
			    (yyval.i) = SP_OBJ_PACK(-1, (yyvsp[(1) - (1)].i));
			else {
			    lc_error("Unknown object class '%c'!", (yyvsp[(1) - (1)].i));
			    (yyval.i) = -1;
			}
		  }
    break;

  case 334:
#line 2121 "lev_comp.y"
    {
		      long m = get_object_id((yyvsp[(4) - (5)].map), (char) (yyvsp[(2) - (5)].i));
		      if (m == ERR) {
			  lc_error("Unknown object ('%c', \"%s\")!", (yyvsp[(2) - (5)].i), (yyvsp[(4) - (5)].map));
			  (yyval.i) == -1;
		      } else
			  (yyval.i) = SP_OBJ_PACK(m, (yyvsp[(2) - (5)].i));
		  }
    break;

  case 335:
#line 2130 "lev_comp.y"
    {
		      (yyval.i) = -1;
		  }
    break;

  case 336:
#line 2136 "lev_comp.y"
    { }
    break;

  case 337:
#line 2138 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_MATH_ADD);
		  }
    break;

  case 338:
#line 2143 "lev_comp.y"
    { add_opvars(splev, "i", (yyvsp[(1) - (1)].i) ); }
    break;

  case 339:
#line 2144 "lev_comp.y"
    { }
    break;

  case 340:
#line 2145 "lev_comp.y"
    { add_opvars(splev, "i", (yyvsp[(2) - (3)].i) ); }
    break;

  case 341:
#line 2147 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (1)].map), SPOVAR_INT);
		      add_opvars(splev, "v", (yyvsp[(1) - (1)].map));
		      Free((yyvsp[(1) - (1)].map));
		  }
    break;

  case 342:
#line 2153 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (4)].map), SPOVAR_INT|SPOVAR_ARRAY);
		      add_opvars(splev, "v", (yyvsp[(1) - (4)].map));
		      Free((yyvsp[(1) - (4)].map));
		  }
    break;

  case 343:
#line 2158 "lev_comp.y"
    { add_opvars(splev, "o", SPO_MATH_ADD); }
    break;

  case 344:
#line 2159 "lev_comp.y"
    { add_opvars(splev, "o", SPO_MATH_SUB); }
    break;

  case 345:
#line 2160 "lev_comp.y"
    { add_opvars(splev, "o", SPO_MATH_MUL); }
    break;

  case 346:
#line 2161 "lev_comp.y"
    { add_opvars(splev, "o", SPO_MATH_DIV); }
    break;

  case 347:
#line 2162 "lev_comp.y"
    { add_opvars(splev, "o", SPO_MATH_MOD); }
    break;

  case 348:
#line 2163 "lev_comp.y"
    { }
    break;

  case 349:
#line 2166 "lev_comp.y"
    { add_opvars(splev, "i", (yyvsp[(1) - (1)].i) ); }
    break;

  case 350:
#line 2167 "lev_comp.y"
    { }
    break;

  case 351:
#line 2168 "lev_comp.y"
    { add_opvars(splev, "i", (yyvsp[(2) - (3)].i) ); }
    break;

  case 352:
#line 2169 "lev_comp.y"
    { add_opvars(splev, "o", SPO_MATH_ADD); }
    break;

  case 353:
#line 2170 "lev_comp.y"
    { add_opvars(splev, "o", SPO_MATH_SUB); }
    break;

  case 354:
#line 2171 "lev_comp.y"
    { add_opvars(splev, "o", SPO_MATH_MUL); }
    break;

  case 355:
#line 2172 "lev_comp.y"
    { add_opvars(splev, "o", SPO_MATH_DIV); }
    break;

  case 356:
#line 2173 "lev_comp.y"
    { add_opvars(splev, "o", SPO_MATH_MOD); }
    break;

  case 357:
#line 2174 "lev_comp.y"
    { }
    break;

  case 358:
#line 2178 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_SEL_POINT);
		  }
    break;

  case 359:
#line 2182 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_SEL_RECT);
		  }
    break;

  case 360:
#line 2186 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_SEL_FILLRECT);
		  }
    break;

  case 361:
#line 2190 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_SEL_LINE);
		  }
    break;

  case 362:
#line 2194 "lev_comp.y"
    {
		      /* randline (x1,y1),(x2,y2), roughness */
		      add_opvars(splev, "o", SPO_SEL_RNDLINE);
		  }
    break;

  case 363:
#line 2199 "lev_comp.y"
    {
		      add_opvars(splev, "io", W_ANY, SPO_SEL_GROW);
		  }
    break;

  case 364:
#line 2203 "lev_comp.y"
    {
		      add_opvars(splev, "io", (yyvsp[(3) - (6)].i), SPO_SEL_GROW);
		  }
    break;

  case 365:
#line 2207 "lev_comp.y"
    {
		      add_opvars(splev, "iio", (yyvsp[(3) - (6)].i), 0, SPO_SEL_FILTER);
		  }
    break;

  case 366:
#line 2211 "lev_comp.y"
    {
		      add_opvars(splev, "io", 1, SPO_SEL_FILTER);
		  }
    break;

  case 367:
#line 2215 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_SEL_FLOOD);
		  }
    break;

  case 368:
#line 2219 "lev_comp.y"
    {
		      add_opvars(splev, "oio", SPO_COPY, 1, SPO_SEL_ELLIPSE);
		  }
    break;

  case 369:
#line 2223 "lev_comp.y"
    {
		      add_opvars(splev, "oio", SPO_COPY, (yyvsp[(7) - (8)].i), SPO_SEL_ELLIPSE);
		  }
    break;

  case 370:
#line 2227 "lev_comp.y"
    {
		      add_opvars(splev, "io", 1, SPO_SEL_ELLIPSE);
		  }
    break;

  case 371:
#line 2231 "lev_comp.y"
    {
		      add_opvars(splev, "io", (yyvsp[(9) - (10)].i), SPO_SEL_ELLIPSE);
		  }
    break;

  case 372:
#line 2235 "lev_comp.y"
    {
		      check_vardef_type(variable_definitions, (yyvsp[(1) - (1)].map), SPOVAR_SEL);
		      add_opvars(splev, "v", (yyvsp[(1) - (1)].map));
		      Free((yyvsp[(1) - (1)].map));
		  }
    break;

  case 373:
#line 2241 "lev_comp.y"
    {
		      /* nothing */
		  }
    break;

  case 374:
#line 2247 "lev_comp.y"
    {
		      /* nothing */
		  }
    break;

  case 375:
#line 2251 "lev_comp.y"
    {
		      add_opvars(splev, "o", SPO_SEL_ADD);
		  }
    break;

  case 376:
#line 2257 "lev_comp.y"
    {
		      add_opvars(splev, "iio", (yyvsp[(1) - (1)].dice).num, (yyvsp[(1) - (1)].dice).die, SPO_DICE);
		  }
    break;

  case 380:
#line 2268 "lev_comp.y"
    {
		      add_opvars(splev, "i", (yyvsp[(1) - (1)].i) );
		  }
    break;

  case 381:
#line 2272 "lev_comp.y"
    {
		      add_opvars(splev, "i", (yyvsp[(1) - (1)].i) );
		  }
    break;

  case 382:
#line 2276 "lev_comp.y"
    {
		      add_opvars(splev, "i", (yyvsp[(1) - (1)].i) );
		  }
    break;

  case 383:
#line 2280 "lev_comp.y"
    {
		      /* nothing */
		  }
    break;

  case 385:
#line 2289 "lev_comp.y"
    {
		      /* by default we just do it, unconditionally. */
		      (yyval.i) = 0;
		  }
    break;

  case 386:
#line 2294 "lev_comp.y"
    {
		      /* otherwise we generate an IF-statement */
		      struct opvar *tmppush2 = New(struct opvar);
		      if (n_if_list >= MAX_NESTED_IFS) {
			  lc_error("Comparison: Too deeply nested IFs.");
			  n_if_list = MAX_NESTED_IFS - 1;
		      }
		      add_opcode(splev, SPO_CMP, NULL);
		      set_opvar_int(tmppush2, splev->n_opcodes+1);
		      if_list[n_if_list++] = tmppush2;
		      add_opcode(splev, SPO_PUSH, tmppush2);
		      add_opcode(splev, reverse_jmp_opcode( (yyvsp[(1) - (1)].i) ), NULL);
		      (yyval.i) = 1;
		  }
    break;

  case 389:
#line 2315 "lev_comp.y"
    {
			(yyval.lregn) = (yyvsp[(1) - (1)].lregn);
		  }
    break;

  case 390:
#line 2319 "lev_comp.y"
    {
			if ((yyvsp[(3) - (10)].i) <= 0 || (yyvsp[(3) - (10)].i) >= COLNO)
			    lc_error("Region (%li,%li,%li,%li) out of level range (x1)!", (yyvsp[(3) - (10)].i), (yyvsp[(5) - (10)].i), (yyvsp[(7) - (10)].i), (yyvsp[(9) - (10)].i));
			else if ((yyvsp[(5) - (10)].i) < 0 || (yyvsp[(5) - (10)].i) >= ROWNO)
			    lc_error("Region (%li,%li,%li,%li) out of level range (y1)!", (yyvsp[(3) - (10)].i), (yyvsp[(5) - (10)].i), (yyvsp[(7) - (10)].i), (yyvsp[(9) - (10)].i));
			else if ((yyvsp[(7) - (10)].i) <= 0 || (yyvsp[(7) - (10)].i) >= COLNO)
			    lc_error("Region (%li,%li,%li,%li) out of level range (x2)!", (yyvsp[(3) - (10)].i), (yyvsp[(5) - (10)].i), (yyvsp[(7) - (10)].i), (yyvsp[(9) - (10)].i));
			else if ((yyvsp[(9) - (10)].i) < 0 || (yyvsp[(9) - (10)].i) >= ROWNO)
			    lc_error("Region (%li,%li,%li,%li) out of level range (y2)!", (yyvsp[(3) - (10)].i), (yyvsp[(5) - (10)].i), (yyvsp[(7) - (10)].i), (yyvsp[(9) - (10)].i));
			(yyval.lregn).x1 = (yyvsp[(3) - (10)].i);
			(yyval.lregn).y1 = (yyvsp[(5) - (10)].i);
			(yyval.lregn).x2 = (yyvsp[(7) - (10)].i);
			(yyval.lregn).y2 = (yyvsp[(9) - (10)].i);
			(yyval.lregn).area = 1;
		  }
    break;

  case 391:
#line 2337 "lev_comp.y"
    {
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if ((yyvsp[(2) - (9)].i) < 0 || (yyvsp[(2) - (9)].i) > (int)max_x_map)
			    lc_error("Region (%li,%li,%li,%li) out of map range (x1)!", (yyvsp[(2) - (9)].i), (yyvsp[(4) - (9)].i), (yyvsp[(6) - (9)].i), (yyvsp[(8) - (9)].i));
			else if ((yyvsp[(4) - (9)].i) < 0 || (yyvsp[(4) - (9)].i) > (int)max_y_map)
			    lc_error("Region (%li,%li,%li,%li) out of map range (y1)!", (yyvsp[(2) - (9)].i), (yyvsp[(4) - (9)].i), (yyvsp[(6) - (9)].i), (yyvsp[(8) - (9)].i));
			else if ((yyvsp[(6) - (9)].i) < 0 || (yyvsp[(6) - (9)].i) > (int)max_x_map)
			    lc_error("Region (%li,%li,%li,%li) out of map range (x2)!", (yyvsp[(2) - (9)].i), (yyvsp[(4) - (9)].i), (yyvsp[(6) - (9)].i), (yyvsp[(8) - (9)].i));
			else if ((yyvsp[(8) - (9)].i) < 0 || (yyvsp[(8) - (9)].i) > (int)max_y_map)
			    lc_error("Region (%li,%li,%li,%li) out of map range (y2)!", (yyvsp[(2) - (9)].i), (yyvsp[(4) - (9)].i), (yyvsp[(6) - (9)].i), (yyvsp[(8) - (9)].i));
			(yyval.lregn).area = 0;
			(yyval.lregn).x1 = (yyvsp[(2) - (9)].i);
			(yyval.lregn).y1 = (yyvsp[(4) - (9)].i);
			(yyval.lregn).x2 = (yyvsp[(6) - (9)].i);
			(yyval.lregn).y2 = (yyvsp[(8) - (9)].i);
		  }
    break;


/* Line 1267 of yacc.c.  */
#line 5223 "y.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 2357 "lev_comp.y"


/*lev_comp.y*/

