/*	SCCS Id: @(#)mkroom.h	3.4	1992/11/14	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MKROOM_H
#define MKROOM_H

/* mkroom.h - types and structures for room and shop initialization */

struct mkroom {
	schar lx,hx,ly,hy;	/* usually xchar, but hx may be -1 */
	schar rtype;		/* type of room (zoo, throne, etc...) */
	schar rlit;		/* is the room lit ? */
	schar needfill;		/* does the room need filling? */
	schar needjoining;
	schar doorct;		/* door count */
	schar fdoor;		/* index for the first door of the room */
	schar nsubrooms;	/* number of subrooms */
	boolean irregular;	/* true if room is non-rectangular */
	struct mkroom *sbrooms[MAX_SUBROOMS];  /* Subrooms pointers */
	struct monst *resident; /* priest/shopkeeper/guard for this room */
};

struct shclass {
	const char *name;	/* name of the shop type */
	char	symb;		/* this identifies the shop type */
	int	prob;		/* the shop type probability in % */
	schar	shdist;		/* object placement type */
#define D_SCATTER	0	/* normal placement */
#define D_SHOP		1	/* shop-like placement */
#define D_TEMPLE	2	/* temple-like placement */
	struct itp {
	    int iprob;		/* probability of an item type */
	    int itype;	/* item type: if >=0 a class, if < 0 a specific item */
	} iprobs[20];
	const char * const *shknms;	/* list of shopkeeper names for this type */
};

extern NEARDATA struct mkroom rooms[(MAXNROFROOMS+1)*2];
extern NEARDATA struct mkroom* subrooms;
/* the normal rooms on the current level are described in rooms[0..n] for
 * some n<MAXNROFROOMS
 * the vault, if any, is described by rooms[n+1]
 * the next rooms entry has hx -1 as a flag
 * there is at most one non-vault special room on a level
 */

extern struct mkroom *dnstairs_room, *upstairs_room, *sstairs_room;

extern NEARDATA coord doors[DOORMAX];

/* values for rtype in the room definition structure */
#define OROOM		 0	/* ordinary room */
#define COURT		 2	/* contains a throne */
#define SWAMP		 3	/* contains pools */
#define VAULT		 4	/* contains piles of gold */
#define BEEHIVE		 5	/* contains killer bees and royal jelly */
#define MORGUE		 6	/* contains corpses, undead and ghosts */
#define BARRACKS	 7	/* contains soldiers and their gear */
#define ZOO		 8	/* floor covered with treasure and monsters */
#define DELPHI		 9	/* contains Oracle and peripherals */
#define TEMPLE		10	/* contains a shrine */
#define LEPREHALL	11	/* leprechaun hall (Tom Proudfoot) */
#define COCKNEST	12	/* cockatrice nest (Tom Proudfoot) */
#define ANTHOLE		13	/* ants (Tom Proudfoot) */
#define GARDEN		14	/* nymphs, trees and fountains */
#define ARMORY		15	/* weapons, armor and rust monsters (L) */
#define LEMUREPIT	16	/* contains lemures and horned devils */
#define POOLROOM	17	/*  */
#define RNDVAULT	18
#ifdef BLACKMARKET
#define BLACKFOYER       19	/* Foyer to the black market */
#endif /* BLACKMARKET */
#define SHOPBASE	20	/* everything above this is a shop */
#define ARMORSHOP	(SHOPBASE+ 1)	/* specific shop defines for level compiler */
#define SCROLLSHOP	(SHOPBASE+ 2)
#define POTIONSHOP	(SHOPBASE+ 3)
#define WEAPONSHOP	(SHOPBASE+ 4)
#define FOODSHOP	(SHOPBASE+ 5)
#define RINGSHOP	(SHOPBASE+ 6)
#define WANDSHOP	(SHOPBASE+ 7)
#define TOOLSHOP	(SHOPBASE+ 8)
#define BOOKSHOP	(SHOPBASE+ 9)
#define TINSHOP		(SHOPBASE+10)
#define INSTRUMENTSHOP	(SHOPBASE+11)
#define PETSHOP		(SHOPBASE+12)	/* Stephen White */
#define UNIQUESHOP	(SHOPBASE+13)	/* shops here & above not randomly gen'd. */
#define CANDLESHOP	(UNIQUESHOP)
#ifdef BLACKMARKET
#define BLACKSHOP       (UNIQUESHOP+1)
#define MAXRTYPE        (UNIQUESHOP+1)      /* maximum valid room type */
#else /* BLACKMARKET */
#define MAXRTYPE        (UNIQUESHOP)      /* maximum valid room type */
#endif /* BLACKMARKET */

/* Special type for search_special() */
#define ANY_TYPE	(-1)
#define ANY_SHOP	(-2)

#define NO_ROOM		0	/* indicates lack of room-occupancy */
#define SHARED		1	/* indicates normal shared boundary */
#define SHARED_PLUS	2	/* indicates shared boundary - extra adjacent-
				 * square searching required */

#define ROOMOFFSET	3	/*
				 * (levl[x][y].roomno - ROOMOFFSET) gives
				 * rooms[] index, for inside-squares and
				 * non-shared boundaries.
				 */

#define IS_ROOM_PTR(x)		((x) >= rooms && (x) < rooms + MAXNROFROOMS)
#define IS_ROOM_INDEX(x)	((x) >= 0 && (x) < MAXNROFROOMS)
#define IS_SUBROOM_PTR(x)	((x) >= subrooms && \
				 (x) < subrooms + MAXNROFROOMS)
#define IS_SUBROOM_INDEX(x)	((x) > MAXNROFROOMS && (x) < (MAXNROFROOMS*2))
#define ROOM_INDEX(x)		((x) - rooms)
#define SUBROOM_INDEX(x)	((x) - subrooms)
#define IS_LAST_ROOM_PTR(x)	(ROOM_INDEX(x) == nroom)
#define IS_LAST_SUBROOM_PTR(x)	(!nsubroom || SUBROOM_INDEX(x) == nsubroom)

#endif /* MKROOM_H */
