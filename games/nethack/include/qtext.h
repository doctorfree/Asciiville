/*	SCCS Id: @(#)qtext.h	3.4	1997/02/02	*/
/* Copyright (c) Mike Stephenson 1991.				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef QTEXT_H
#define QTEXT_H

#define N_HDR	16		/* Maximum number of categories */
				/* (i.e., num roles + 1) */
#define LEN_HDR 3		/* Maximum length of a category name */

struct qtmsg {
	int	msgnum;
	char	delivery;
	long	offset,
		size;
};

#ifdef MAKEDEFS_C	/***** MAKEDEFS *****/

#define N_MSG	500		/* arbitrary */

struct msghdr {
	int	n_msg;
	struct	qtmsg	qt_msg[N_MSG];
};

struct	qthdr {
	int	n_hdr;
	char	id[N_HDR][LEN_HDR];
	long	offset[N_HDR];
};

/* Error message macros */
#define CREC_IN_MSG	"Control record encountered during message - line %d\n"
#define DUP_MSG		"Duplicate message number at line %d\n"
#define END_NOT_IN_MSG	"End record encountered before message - line %d\n"
#define TEXT_NOT_IN_MSG	"Text encountered outside message - line %d\n"
#define UNREC_CREC	"Unrecognized Control record at line %d\n"
#define OUT_OF_HEADERS	"Too many message types (line %d)\nAdjust N_HDR in qtext.h and recompile.\n"
#define OUT_OF_MESSAGES "Too many messages in class (line %d)\nAdjust N_MSG in qtext.h and recompile.\n"


#else	/***** !MAKEDEFS *****/

struct	qtlists {
	struct	qtmsg	*common,
#if 0	/* UNUSED but available */
			*chrace,
#endif
			*chrole;
};


/*
 *	Quest message defines.	Used in quest.c to trigger off "realistic"
 *	dialogue to the player.
 */
#define QT_FIRSTTIME	 1
#define QT_NEXTTIME	 2
#define QT_OTHERTIME	 3

#define QT_GUARDTALK	 5	/* 5 random things guards say before quest */
#define QT_GUARDTALK2	10	/* 5 random things guards say after quest */

#define QT_FIRSTLEADER	15
#define QT_NEXTLEADER	16
#define QT_OTHERLEADER	17
#define QT_LASTLEADER	18
#define QT_BADLEVEL	19
#define QT_BADALIGN	20
#define QT_ASSIGNQUEST	21

#define QT_ENCOURAGE	25	/* 1-10 random encouragement messages */

#define QT_FIRSTLOCATE	35
#define QT_NEXTLOCATE	36

#define QT_FIRSTGOAL	40
#define QT_NEXTGOAL	41

#define QT_FIRSTNEMESIS 50
#define QT_NEXTNEMESIS	51
#define QT_OTHERNEMESIS 52
#define QT_NEMWANTSIT	53	/* you somehow got the artifact */

#define QT_DISCOURAGE	60	/* 1-10 random maledictive messages */

#define QT_GOTIT	70

#define QT_KILLEDNEM	80
#define QT_OFFEREDIT	81
#define QT_OFFEREDIT2	82

#define QT_POSTHANKS	90
#define QT_HASAMULET	91

/*
 *	Message defines for common text used in maledictions.
 */
#define COMMON_ID	"-"	/* Common message id value */

#define QT_ANGELIC	10
#define QTN_ANGELIC	10

#define QT_DEMONIC	30
#define QTN_DEMONIC	20

#define QT_BANISHED	60

/*
 *  Tutorial text common to all classes (displayed with check_tutorial_message).
 */

#define QT_T_FIRST          201

/* Introductory messages */
#define QT_T_WELCOME        201 /* at start of game */
#define QT_T_CURSOR_NUMPAD  202 /* upon pressing ; with numpad */
#define QT_T_CURSOR_VIKEYS  203 /* upon pressing ; with vikeys */
/* Looking at monsters */
#define QT_T_LOOK_TAME      210 /* farlooking a pet */
#define QT_T_LOOK_HOSTILE   211 /* farlooking a hostile */
#define QT_T_LOOK_PEACEFUL  212 /* farlooking a peaceful */
#define QT_T_LOOK_INVISIBLE 213 /* adjacent to or farlooked I */
/* Terrain messages; display when seen/adjacent, or farlooked */
#define QT_T_DOORS          230 /* adjacent to or farlooked a door */
#define QT_T_CORRIDOR       231 /* adjacent to or farlooked a corridor */
#define QT_T_SECRETDOOR     232 /* adjacent to a secret door (freebie) */
#define QT_T_POOLORMOAT     233 /* adjacent to or farlooked water */
#define QT_T_LAVA           234 /* adjacent to or farlooked lava */
#define QT_T_STAIRS         235 /* adjacent to or farlooked /downstairs/ */
#define QT_T_FOUNTAIN       236 /* adjacent to or farlooked fountain */
#define QT_T_THRONE         237 /* adjacent to or farlooked throne */
#define QT_T_SINK           238 /* adjacent to or farlooked sink */
#define QT_T_GRAVE          239 /* adjacent to or farlooked grave */
#define QT_T_ALTAR          240 /* adjacent to or farlooked altar */
#define QT_T_DRAWBRIDGE     241 /* adjacent to or farlooked drawbridge */
#define QT_T_TRAP           242 /* adjacent to or farlooked trap */
#define QT_T_L1UPSTAIRS     259 /* farlooked level 1 upstairs */
/* Interface hints and reminders */
#define QT_T_FARMOVE_VIKEYS 260 /* direction key five times in a row */
#define QT_T_FARMOVE_NUMPAD 261 /* ditto */
#define QT_T_TRAVEL         262 /* thirty direction keys in a row */
#define QT_T_DIAGONALS_VI   263 /* twenty orthogonals in a row */
#define QT_T_DIAGONALS_NUM  264 /* twenty orthogonals in a row */
#define QT_T_REPEAT_VIKEYS  265 /* s 5 times in a row */
#define QT_T_REPEAT_NUMPAD  266 /* s 5 times in a row */
#define QT_T_CHOOSEITEM     267 /* explaining [a-zA-Z?*] menus */
#define QT_T_MASSUNEQUIP    268 /* R or T twice in a row */
#define QT_T_CALLMONSTER    269 /* adjacent to two identical monsters */
#define QT_T_MULTIDROP      270 /* d twice in a row */
#define QT_T_MASSINVENTORY  271 /* I twice in a row */
#define QT_T_SECONDWIELD    272 /* w more often than twice per 50 turns */
#define QT_T_LOOK_REMINDER  273 /* no ; within the last 100 turns */
#define QT_T_FIRE           274 /* t three times in a row */
#define QT_T_DGN_OVERVIEW   275 /* dungeon #overview and #annotate */
/* Ambient advice, outside combat */
#define QT_T_VIEWTUTORIAL   280 /* peaceful, turn >= 10 */
#define QT_T_CHECK_ITEMS    281 /* peaceful, turn >= 30 */
#define QT_T_OBJECTIVE      282 /* peaceful, turn >= 60 */
#define QT_T_SAVELOAD       283 /* peaceful, turn >= 100 */
#define QT_T_MESSAGERECALL  284 /* peaceful, turn >= 150 */
/* Ambient advice, during combat; only one advice given per combat */
#define QT_T_SPELLS         290 /* knows 'force bolt' */
#define QT_T_THROWNWEAPONS  291 /* thrown weapon in invent */
#define QT_T_PROJECTILES    292 /* ammo in invent */
#define QT_T_ELBERETH       293 /* combat on dlevel 3+ */
#define QT_T_MELEE          294 /* a general combat ambient */
/* Item types; these appear when the item's picked up, farlooked, or
   the relevant inventory section is viewed */
#define QT_T_ITEM_GOLD      300
#define QT_T_ITEM_WEAPON    301
#define QT_T_ITEM_ARMOR     302
#define QT_T_ITEM_FOOD      303
#define QT_T_ITEM_SCROLL    304
#define QT_T_ITEM_WAND      305
#define QT_T_ITEM_RING      306
#define QT_T_ITEM_POTION    307
#define QT_T_ITEM_TOOL      308
#define QT_T_ITEM_CONTAINER 309 /* on pickup, or standover for chests */
#define QT_T_ITEM_AMULET    310
#define QT_T_ITEM_GEM       311
#define QT_T_ITEM_STATUE    312
#define QT_T_ITEM_BOOK      313
#define QT_T_CALLITEM       317 /* call-item prompt */
#define QT_T_ARTIFACT       318 /* obtained an artifact */
#define QT_T_RANDAPPEARANCE 319 /* obtained a rand-appearance item */
/* Events */
#define QT_T_LEVELUP        320 /* became xlevel 2 */
#define QT_T_RANKUP         321 /* became xlevel 3 */
#define QT_T_ABILUP         322 /* str, con, dex, int, wis, or cha increased */
#define QT_T_ABILDOWN       323 /* str, con, dex, int, wis, or cha decreased */
#define QT_T_DLEVELCHANGE   324 /* entered dlvl 2+ */
#define QT_T_DAMAGED        325 /* not at full hp */
#define QT_T_PWUSED         326 /* not at full pw */
#define QT_T_PWEMPTY        327 /* pw too low to cast a spell */
#define QT_T_ACIMPROVED     328 /* AC better than starting value */
#define QT_T_GAINEDEXP      329 /* not at 0 exp */
#define QT_T_HUNGER         330 /* hungry or worse */
#define QT_T_SATIATION      331 /* satiated */
#define QT_T_STATUS         332 /* other status */
#define QT_T_ENHANCE        333 /* can train a skill */
#define QT_T_POLYSELF       334 /* turned into monster form */
#define QT_T_ENGRAVING      335 /* stepped on an engraving */
#define QT_T_MAJORTROUBLE   336 /* in major trouble and #pray will work */
#define QT_T_BURDEN         337 /* became burdened */
#define QT_T_EQUIPCURSE     338 /* BCU-known item is cursed */
#define QT_T_MAILSCROLL     339 /* mail was delivered */
#define QT_T_CASTER_ARMOR   340 /* spellcaster wearing inappropriate armour */
#define QT_T_WEAPON_SKILL   341 /* wielding an unskilled weapon */
#define QT_T_ENGULFED       342 /* engulfed by a monster */
#define QT_T_DEATH          359 /* died; contains x-mode hint */
/* Dungeon areas */
#define QT_T_SHOPENTRY      360 /* entered a shop */
#define QT_T_SHOPBUY        361 /* picked up an item in a shop */
#define QT_T_MINES          362 /* entered the Mines */
#define QT_T_SOKOBAN        363 /* entered Sokoban */
#define QT_T_ORACLE         364 /* upon "welcome to Delphi" message */
/* Leave some room before this to avoid breaking savefile compatibility
   if the list is ever expanded. No tutorial message can have a number
   higher than this. */
#define QT_T_MAX            400
/* Guidebook sections not mentioned in the tutorial: conducts,
   options, riding, autopickup-exceptions, regexp sounds, play by the
   blind, scoring, credits, flavour introduction (although 'legacy'
   handles that in tutorial style), class choice (so far) */

#endif	/***** !MAKEDEFS *****/

#endif /* QTEXT_H */
