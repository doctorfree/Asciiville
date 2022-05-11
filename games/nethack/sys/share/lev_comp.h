/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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
/* Line 1489 of yacc.c.  */
#line 366 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

