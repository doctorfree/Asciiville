/* This file contains the functions that create Sheol filler levels. */

#include "hack.h"
#include "sp_lev.h"

#define HEIGHT	(ROWNO - 1)
#define WIDTH	(COLNO - 2)

/* Minimum distance between points when using voronoi diagram to place
 * water, ice and solid ground.
 *
 * This is euclidean distance to second power. 
 * (i.e. 2 == 4) */
#define MINIMUM_VORONOI_DISTANCE 4

/* See sheol.txt for Sheol description.
 * This file deals with generating the levels. */

/* init_level_base_voronoi() uses this. */
typedef struct spatchcoord
{
	coord c;
	schar typ;
} patchcoord;

/* probability of making a floor:
 * prob / out_of or if guaranteed is 1 then it's 100% chance */
typedef struct sfloorprob
{
	int prob;
	int out_of;

	int guaranteed;
} floorprob;

/* These are chances which type to use for each tile in a voronoi diagram. */
static schar typs[10] = { ICEWALL,
			  CRYSTALICEWALL, CRYSTALICEWALL, CRYSTALICEWALL,
			  POOL, POOL,
			  STONE, STONE, STONE, STONE };

static schar opentyps[10] = { ICEWALL,
			  CRYSTALICEWALL, CRYSTALICEWALL, STONE,
			  POOL, POOL,
			  ROOM, ICE, ICE, ROOM };

static void init_level_base_voronoi(schar* vtyps, int numtyps, int numpoints);
static int check_voronoi_winner(patchcoord* coords, int num_coords,
				int x, int y);
static void carve_path(floorprob* probs);
static void fuzzy_circle(int x, int y,
			 int guaranteed_passage_radius, int fallout,
			 floorprob* floorprobs);
static void wallify_map(void);

static void finalize_map(void);
static int under_middle(void);
static int at_middle(void);

/* Return values from plug_unreachable_places and verify_stairs_place. */
/*  Any value >0 means "ok" */
#define STAT_REJECT 0       /* "please reject the map, it's too bad" */
#define STAT_SEMIPLUGGED  1 /* Some places are only reachable behind ice */
#define STAT_ALLREACHABLE 2 /* Every place is reachable by walking or 
			       levitating */
#define STAT_STAIRSOK     3 /*  Stairs have adequate space. */

static int plug_unreachable_places(void);
static int verify_stairs_place(void);

static void mkopensheol(void);
static void place_clouds(void);
static void shake_position(int* x, int* y);

void
mksheol(init_lev_par)
	void *init_lev_par;
{
	lev_init* init_lev = (lev_init*) init_lev_par;

	/* init_lev isn't actually used... */
	((void) init_lev);

	int i1, i2;
	int testval;
	floorprob* probs;

	/* Sometimes make an almost open level instead */
	if (!under_middle() &&
	    !at_middle() && !rn2(3)) {
		mkopensheol();
		return;
	}

	probs = (floorprob*) alloc(sizeof(floorprob) * COLNO * ROWNO);

	again:
	memset(probs, 0, sizeof(floorprob) * COLNO * ROWNO);

	init_level_base_voronoi(typs, 10, 300);

	/* Then, carve a "path" from somewhere left of the level to the right
	 * of the level. */
	carve_path(probs);

	for (i1 = 1; i1 < COLNO; ++i1)
		for (i2 = 0; i2 < ROWNO; ++i2) {
			testval = 1;
			if (probs[i1 + i2 * COLNO].out_of != 0) {
				testval = rn2(probs[i1+i2*COLNO].out_of);	
			}
			if (testval < probs[i1+i2*COLNO].prob ||
			    probs[i1+i2*COLNO].guaranteed)
			{
				if (levl[i1][i2].typ == ICEWALL ||
				    levl[i1][i2].typ == CRYSTALICEWALL)
					levl[i1][i2].typ = ICE;
				else if (levl[i1][i2].typ != POOL)
					levl[i1][i2].typ = ROOM;

			}
			else if (levl[i1][i2].typ == POOL)
				levl[i1][i2].typ = ICEWALL;
		}
	
	/* Sometimes, put a lot of clouds somewhere on the level. */
	if (!rn2(5))
		place_clouds();

	if (verify_stairs_place() == STAT_REJECT)
		goto again;

	finalize_map();

	if (plug_unreachable_places() == STAT_REJECT)
		goto again;

	free(probs);
}

static void 
init_level_base_voronoi(schar* vtyps, int numtyps, int numpoints)
{
	int patches, i1, i2;
	int winner_patch;
	patchcoord* points;

	/* We use a voronoi diagram to put ice and solid ground on the level.
	 * Maybe it looks interesting? I hope so. That's kind of the point. */
	points = (patchcoord*) alloc(sizeof(patchcoord)*numpoints);
	patches = numpoints;

	for (i1 = 0; i1 < patches; ++i1) {
		points[i1].c.x = rn1(COLNO-1, 1);
		points[i1].c.y = rn2(ROWNO);
		points[i1].typ = vtyps[rn2(numtyps)];

		/* Don't want them to be too close to other points... */
		for (i2 = 0; i2 < i1; ++i2) {
			if (dist2(points[i1].c.x, points[i1].c.y,
				  points[i2].c.x, points[i2].c.y) < 
				  MINIMUM_VORONOI_DISTANCE) {
				--i1;
				break;
			}
		}
	}

	for (i1 = 1; i1 < COLNO; ++i1)
		for (i2 = 0; i2 < ROWNO; ++i2) {
			winner_patch = check_voronoi_winner(points, patches, i1, i2);
			levl[i1][i2].typ = points[winner_patch].typ;
		}
	
	free(points);
}

static int 
check_voronoi_winner(coords, num_coords, x, y)
	patchcoord* coords;
	int num_coords;
	int x, y;
{
	int i1;
	int winner = 0, winner_distance;
	int d;

	winner_distance = 10000;

	for (i1 = 0; i1 < num_coords; ++i1)
	{
		d = dist2(coords[i1].c.x, coords[i1].c.y, x, y);
		if (d < winner_distance) {
			winner_distance = d;
			winner = i1;
		}
	}

	return winner;
}

typedef struct sbezcoord
{
	int x, y;         /* coordinate */
} bezcoord;

static void 
carve_path(floorprobs)
	floorprob* floorprobs;
{
	/*
	 * We'll draw a bezier curve from the left of the level to the right.
	 * We first pick a rough path from left to right by picking points.
	 * Then we'll make a smooth path using a bezier curve by going through
	 * those points.
	 *
	 * We use every second coordinate as a control point.
	 */

	bezcoord points[100];
	int i1, i2, num_points, c1_x, c1_y, c2_x, c2_y;
	int r_i2;
	int x, y;

	int sample_points;
	
	int path_x, path_y;
	int tries;

	/* Attempt to make ~random point paths to the end. */
	while(1) {
		path_x = -2;
		path_y = ROWNO / 2;

		for (i1 = 0; i1 < 100; ++i1) {
			tries = 0;
			while(tries < 100) {
				tries++;
				points[i1].x = path_x;
				points[i1].y = path_y;

				if (!rn2(2))
					points[i1].x += rn2(11)-5;
				else
					points[i1].x += rn2(6)-2;
				points[i1].y += rn2(11)-5;

				if (points[i1].y < 0 ||
				    points[i1].y >= ROWNO ||
				    points[i1].x < 1 ||
				    points[i1].x >= COLNO)
					continue;
				for (i2 = 0; i2 < i1; ++i2)
					if (dist2(points[i2].x, points[i2].y,
						  points[i1].x, points[i1].y) <=
						16)
						break;
				if (i2 < i1)
					continue;
				break;
			}
			if (tries >= 100)
			{
				path_x = -2;
				path_y = ROWNO / 2;
				i1 = -1;
				continue;
			}
			if (i1 > 0 && points[i1-1].x >= COLNO-5)
				break;
			path_x = points[i1].x;
			path_y = points[i1].y;
		}
		if (i1 >= 100)
			continue;
		break;
	}

	num_points = i1;

	for (i1 = 0; i1 < num_points - 2; i1 += 2) {
		sample_points = isqrt(dist2(points[i1].x, points[i1].y,
					    points[i1+1].x, points[i1+1].y)+
				    dist2(points[i1+1].x, points[i1+1].y,
				    	  points[i1+2].x, points[i1+2].y))*20;
		for (i2 = 0; i2 <= sample_points; ++i2)
		{
			r_i2 = sample_points - i2;
			#define BEZNUMERICAL(targ, m, idx1, idx2) \
				targ = points[idx1].m * i2 / sample_points; \
				targ += points[idx2].m * r_i2 / sample_points;

			BEZNUMERICAL(c1_x, x, i1, i1+1);
			BEZNUMERICAL(c1_y, y, i1, i1+1);
			BEZNUMERICAL(c2_x, x, i1+1, i1+2);
			BEZNUMERICAL(c2_y, y, i1+1, i1+2);

			x = c1_x * i2 / sample_points;
			x += c2_x * r_i2 / sample_points;
			y = c1_y * i2 / sample_points;
			y += c2_y * r_i2 / sample_points;

			#undef BEZNUMERICAL

			if (under_middle())
				fuzzy_circle(x, y, 1, 0, floorprobs);
			else
				fuzzy_circle(x, y, 1, 2, floorprobs);
		}
	}
}


/* Makes a passable circle centered at x, y.
 * To radius guaranteed_passage, guarantees passability.
 *
 * After that, linearly makes it less likely that a passable square is
 * made. This makes a sort of "rough" edge. */
static void 
fuzzy_circle(x, y, guaranteed_passage_radius, fallout, floorprobs)
	int x, y;
	int guaranteed_passage_radius, fallout;
	floorprob* floorprobs;
{
	int i1, i2;
	int fallout_2 = fallout * fallout;
	int d;

	if (fallout < guaranteed_passage_radius)
		fallout = guaranteed_passage_radius;

	for (i1 = x - fallout; i1 <= x + fallout; ++i1)
		for (i2 = y - fallout; i2 <= y + fallout; ++i2) {
			if (i1 < 1 || i1 >= COLNO ||
			    i2 < 0 || i2 >= ROWNO)
				continue;
			d = dist2(i1, i2, x, y);
			if (d > fallout_2)
				continue;
			if (d <= guaranteed_passage_radius)
				floorprobs[i1 + i2 * COLNO].guaranteed = 1;
			else {
				d -= guaranteed_passage_radius;
				d = (d * 100) / 
					((fallout_2 - 
					  guaranteed_passage_radius));
				d = 100 - d;
				floorprobs[i1+i2*COLNO].prob += d;
				floorprobs[i1+i2*COLNO].out_of += 100;
			}
		}
}

static void
wallify_map(void)
{

    int x, y, xx, yy;

    for(x = 1; x < COLNO; x++)
	for(y = 0; y < ROWNO; y++)
	    if(levl[x][y].typ == STONE) {
		for(yy = y - 1; yy <= y+1; yy++)
		    for(xx = x - 1; xx <= x+1; xx++)
			if(isok(xx,yy) && levl[xx][yy].typ == ROOM) {
			    if(yy != y)	levl[x][y].typ = HWALL;
			    else	levl[x][y].typ = VWALL;
			}
	    }
}

#define VALID_PASSABLE(x, y) (levl[x][y].typ == ICE || \
			      levl[x][y].typ == POOL || \
			      levl[x][y].typ == ROOM)
#define VALID_PASSABLE2(x, y) (VALID_PASSABLE(x, y) || \
			       IS_ANY_ICEWALL(levl[x][y].typ))

static int 
plug_unreachable_places(void)
{
	char fillmap[COLNO][ROWNO];
	int not_passable;
	int x, y;
	int flood_x, flood_y;
	int tries;
	int done;

	not_passable = 0;

	memset(fillmap, 0, sizeof(fillmap));

	tries = 100;
	while(tries > 0) {
		tries--;
		x = rn1(COLNO-2, 1);
		y = rn2(ROWNO);
		if (levl[x][y].typ != ROOM &&
		    levl[x][y].typ != ICE)
			continue;
		break;
	}
	if (tries <= 0)
		return STAT_REJECT;
	
	/* flood fill */
	done = 0;
	flood_x = x;
	flood_y = y;
	fillmap[flood_x][flood_y] = 1;

	while(!done) {
		done = 1;
		for (x = 2; x < COLNO-1; ++x)
			for (y = 1; y < ROWNO-1; ++y)
			{
				if (!fillmap[x][y])
					continue;
				if (fillmap[x+1][y] == 0 &&
				    VALID_PASSABLE(x+1, y)) {
				    fillmap[x+1][y] = 1;
				    done = 0;
				}
				if (fillmap[x-1][y] == 0 &&
				    VALID_PASSABLE(x-1, y)) {
				    fillmap[x-1][y] = 1;
				    done = 0;
				}
				if (fillmap[x][y-1] == 0 &&
				    VALID_PASSABLE(x, y-1)) {
				    fillmap[x][y-1] = 1;
				    done = 0;
				}
				if (fillmap[x][y+1] == 0 &&
				    VALID_PASSABLE(x, y+1)) {
				    fillmap[x][y+1] = 1;
				    done = 0;
				}
			}
	}

	for (x = 1; x < COLNO; ++x)
		for (y = 0; y < ROWNO; ++y)
			if (VALID_PASSABLE(x, y) &&
			    fillmap[x][y] != 1)
			{
			    not_passable = 1;
			    break;
			}

	if (!not_passable)
		return STAT_ALLREACHABLE;

	/* flood fill again, but go through ice this time */
	done = 0;
	not_passable = 0;
	memset(fillmap, 0, sizeof(fillmap));
	fillmap[flood_x][flood_y] = 1;

	while(!done) {
		done = 1;
		for (x = 2; x < COLNO-1; ++x)
			for (y = 1; y < ROWNO-1; ++y)
			{
				if (!fillmap[x][y])
					continue;
				if (fillmap[x+1][y] == 0 &&
				    VALID_PASSABLE2(x+1, y)) {
				    fillmap[x+1][y] = 1;
				    done = 0;
				}
				if (fillmap[x-1][y] == 0 &&
				    VALID_PASSABLE2(x-1, y)) {
				    fillmap[x-1][y] = 1;
				    done = 0;
				}
				if (fillmap[x][y-1] == 0 &&
				    VALID_PASSABLE2(x, y-1)) {
				    fillmap[x][y-1] = 1;
				    done = 0;
				}
				if (fillmap[x][y+1] == 0 &&
				    VALID_PASSABLE2(x, y+1)) {
				    fillmap[x][y+1] = 1;
				    done = 0;
				}
			}
	}

	for (x = 1; x < COLNO; ++x)
		for (y = 0; y < ROWNO; ++y)
			if (VALID_PASSABLE(x, y) &&
			    fillmap[x][y] != 1)
			{
			    not_passable = 1;
			    break;
			}
	
	if (not_passable)
		return STAT_REJECT;
	return STAT_SEMIPLUGGED;
}

/* Return 1 if below the middle Sheol level. */
static int 
under_middle(void) {
	return u.uz.dlevel > 2;
}

/* Return 0 if right at the middle level. */
static int
at_middle(void) {
	return u.uz.dlevel == 2;
}

static void 
mkopensheol(void) {
	/*  This one's simple. */
again:
	do {
		init_level_base_voronoi(opentyps, 10, 300);
	} while(verify_stairs_place() != STAT_STAIRSOK);

	finalize_map();

	if (plug_unreachable_places() == STAT_REJECT)
		goto again;
}

static void
finalize_map(void) {
	int i1, i2;

	for (i1 = 1; i1 < COLNO; ++i1)
		for (i2 = 0; i2 < ROWNO; ++i2) {
			levl[i1][i2].lit = TRUE; /* lit things up */
			if (IS_ROCK(levl[i1][i2].typ) &&
			    !IS_ANY_ICEWALL(levl[i1][i2].typ))
				levl[i1][i2].wall_info |= W_NONDIGGABLE;
		}
	
	wallify_map();

	level.flags.is_maze_lev = FALSE;
	level.flags.is_cavernous_lev = TRUE;
}

static int
verify_stairs_place(void) {
	int i1, i2;

	/* Make sure there is ROOM somewhere in both sides of the level */
	for (i1 = 1; i1 <= 15; ++i1) {
		for (i2 = 0; i2 < ROWNO; ++i2)
			if (levl[i1][i2].typ == ROOM)
				break;
		if (i2 < ROWNO)
			break;
	}
	if (i1 > 15)
		return STAT_REJECT;

	for (i1 = 65; i1 < COLNO; ++i1) {
		for (i2 = 0; i2 < ROWNO; ++i2)
			if (levl[i1][i2].typ == ROOM)
				break;
		if (i2 < ROWNO)
			break;
	}
	if (i1 >= COLNO)
		return STAT_REJECT;

	return STAT_STAIRSOK;
}

static void
place_clouds(void) {
	int num_clouds;
	int x, y;
	int tries;

	num_clouds = rn2(25) + rn2(40) + 5;

	while(1) {
		x = rn1(COLNO-2,1);
		y = rn2(ROWNO);
		if (levl[x][y].typ == ICE ||
		    levl[x][y].typ == ROOM)
			break;
	}

	tries = 200;
	while(num_clouds > 0 && tries > 0) {
		if (levl[x][y].typ == ICE ||
		    levl[x][y].typ == ROOM) {
			levl[x][y].typ = CLOUD;
			num_clouds--;
			tries = 200;
		}
		else {
			--tries;
			shake_position(&x, &y);
		}
	}
}

static void
shake_position(x, y)
	int* x;
	int* y;
{
	int i1;
	int old_x, old_y;
	int tries;

	int dx, dy;

	old_x = (*x);
	old_y = (*y);

	tries = 10;

again:
	if (!tries)
		return;
	--tries;

	(*x) = old_x;
	(*y) = old_y;

	if (!rn2(30)) {
		(*x) += rn2(7)-3;
		(*y) += rn2(7)-3;
	}
	else {
		(*x) += rn2(3)-1;
		(*y) += rn2(3)-1;
	}

	if ((*x) >= COLNO)
		(*x) = COLNO-1;
	if ((*x) < 1)
		(*x) = 1;
	if ((*y) < 0)
		(*y) = 0;
	if ((*y) >= ROWNO)
		(*y) = ROWNO-1;

	dx = (*x) > old_x ? 1 : -1;
	dy = (*y) > old_y ? 1 : -1;

	for (i1 = old_x; i1 != (*x); i1 += dx)
	{
		if (levl[i1][old_y].typ != ICE ||
		    levl[i1][old_y].typ != ROOM ||
		    levl[i1][old_y].typ != CLOUD)
			goto again;
	}
	if (levl[*x][old_y].typ != ICE ||
	    levl[*x][old_y].typ != ROOM ||
	    levl[*x][old_y].typ != CLOUD)
		goto again;
	for (i1 = old_y; i1 != (*y); i1 += dy)
	{
		if (levl[*x][i1].typ != ICE ||
		    levl[*x][i1].typ != ROOM ||
		    levl[*x][i1].typ != CLOUD)
			goto again;
	}
	if (levl[*x][*y].typ != ICE ||
	    levl[*x][*y].typ != ROOM ||
	    levl[*x][*y].typ != CLOUD)
		goto again;
}

