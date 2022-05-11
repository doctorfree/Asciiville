/* Copyright (c) Shawn Betts, Ryan Yeske, 2001                    */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * "Main" file for the lisp/emacs window-port.  This contains most of
 * the interface routines.  Please see doc/window.doc for an
 * description of the window interface.  
 */

#ifdef MSDOS			/* from compiler */
#define SHORT_FILENAMES
#endif

#include "hack.h"
#include "winlisp.h"
#include "func_tab.h"

#include "dlb.h"
#ifdef SHORT_FILENAMES
#include "patchlev.h"
#else
#include "patchlevel.h"
#endif

#define CMD_KEY 0
#define CMD_EXT 1
#define CMD_LISP 2 /* These are commands specific to the lisp port */

/* from tile.c */
extern short glyph2tile[];
extern int total_tiles_used;

typedef struct
{
  anything identifier;
  char accelerator;
} lisp_menu_item_t;

/* An iterator for assigning accelerator keys. */
static char lisp_current_accelerator;

/* Helper structures to map menu id's to nethack anything's */
static lisp_menu_item_t lisp_menu_item_list[1000];
static int lisp_menu_list_size = 1000;
static int lisp_menu_list_num;

extern char *enc_stat[];
const char *hunger_stat[] = {
	"Satiated",
	"",
	"Hungry",
	"Weak",
	"Fainting",
	"Fainted",
	"Starved"
};

typedef struct 
{
  char *name;
  int type;
  int cmd;			/* The command (a keystroke) */
}cmd_index_t;

#ifndef C
#define C(c)		(0x1f & (c))
#endif

/* Taken from cmd.c */
cmd_index_t cmd_index[] = { {"gowest", CMD_KEY, 'h'},
			    {"gowestontop", CMD_KEY, 'H'},
			    {"gowestnear", CMD_KEY, C('h')},

			    {"gosouth", CMD_KEY, 'j'},
			    {"gosouthontop", CMD_KEY, 'J'},
			    {"gosouthnear", CMD_KEY,  C('j')},

			    {"gonorth", CMD_KEY, 'k'},
			    {"gonorthontop", CMD_KEY, 'K'},
			    {"gonorthnear", CMD_KEY, C('k')},

			    {"goeast", CMD_KEY, 'l'},
			    {"goeastontop", CMD_KEY, 'L'},
			    {"goeastnear", CMD_KEY, C('l')},

			    {"gonorthwest", CMD_KEY, 'y'},
			    {"gonorthwestontop", CMD_KEY, 'Y'},
			    {"gonorthwestnear", CMD_KEY, C('y')},

			    {"gonortheast", CMD_KEY, 'u'},
			    {"gonortheastontop", CMD_KEY, 'U'},
			    {"gonortheastnear", CMD_KEY, C('u')},

			    {"gosouthwest", CMD_KEY, 'b'},
			    {"gosouthwestontop", CMD_KEY, 'B'},
			    {"gosouthwestnear", CMD_KEY, C('b')},

			    {"gosoutheast", CMD_KEY, 'n'},
			    {"gosoutheastontop", CMD_KEY, 'N'},
			    {"gosoutheastnear", CMD_KEY, C('n')},

			    {"travel", CMD_KEY, '_'},

			    {"idtrap", CMD_KEY, '^'},
			    {"apply", CMD_KEY, 'a'},
			    {"remarm", CMD_KEY, 'A'},
			    {"close", CMD_KEY, 'c'},
			    {"drop", CMD_KEY, 'd'},

			    {"ddrop", CMD_KEY, 'D'},
			    {"eat", CMD_KEY, 'e'},
			    {"engrave", CMD_KEY, 'E'},
			    {"fire", CMD_KEY, 'f'},
			    {"inv", CMD_KEY, 'i'},

			    {"typeinv", CMD_KEY, 'I'},
			    {"open", CMD_KEY, 'o'},
			    {"set", CMD_KEY, 'O'},
			    {"pay", CMD_KEY, 'p'},
			    {"puton", CMD_KEY, 'P'},

			    {"drink", CMD_KEY, 'q'},
			    {"wieldquiver", CMD_KEY, 'Q'},
			    {"read", CMD_KEY, 'r'},
			    {"remring", CMD_KEY, 'R'},
			    {"search", CMD_KEY, 's'},

			    {"save", CMD_KEY, 'S'},
			    {"throw", CMD_KEY, 't'},
			    {"takeoff", CMD_KEY, 'T'},
			    {"simpleversion", CMD_KEY, 'v'},
			    {"history", CMD_KEY, 'V'},

			    {"wield", CMD_KEY, 'w'},
			    {"wear", CMD_KEY, 'W'},
			    {"swapweapon", CMD_KEY, 'x'},
			    {"enter_explore_mode", CMD_KEY, 'X'},
			    {"zap", CMD_KEY, 'z'},

			    {"cast", CMD_KEY, 'Z'},
			    {"up", CMD_KEY, '<'},
			    {"down", CMD_KEY, '>'},
			    {"whatis", CMD_KEY, '/'},
			    {"help", CMD_KEY, '?'},

			    {"whatdoes", CMD_KEY, '&'},
			    {"sh", CMD_KEY, '!'},
			    {"discovered", CMD_KEY, '\\'},
			    {"null", CMD_KEY, '.'},
			    {"look", CMD_KEY, ':'},

			    {"quickwhatis", CMD_KEY, ';'},
			    {"pickup", CMD_KEY, ','},
			    {"togglepickup", CMD_KEY, '@'},
			    {"prinuse", CMD_KEY, '*'},
			    {"countgold", CMD_KEY, '$'},

			    {"kick", CMD_KEY, C('d')},
			    {"listspells", CMD_KEY, '+'},
			    {"redraw", CMD_KEY, C('r')},
			    {"teleport", CMD_KEY, C('t')},
			    {"callmon", CMD_KEY, 'C'},
			    {"fight", CMD_KEY, 'F'},
			    {"movenear", CMD_KEY, 'g'},
			    {"move", CMD_KEY, 'G'},
			    {"movenopickuporfight", CMD_KEY, 'm'},
			    {"movenopickup", CMD_KEY, 'M'},
			    {"showweapon", CMD_KEY, ')'},
			    {"showarmor", CMD_KEY, '['},
			    {"showrings", CMD_KEY, '='},
			    {"showamulet", CMD_KEY, '"'},
			    {"showtool", CMD_KEY, '('},
			    {"attributes", CMD_KEY, C('x')},
#ifdef REDO
			    {"again", CMD_KEY, DOAGAIN},
#endif /* REDO */

			    /* wizard commands */
			    {"wiz_detect", CMD_KEY, C('e')},
			    {"wiz_map", CMD_KEY, C('f')},
			    {"wiz_genesis", CMD_KEY, C('g')},
			    {"wiz_identify", CMD_KEY, C('i')},
			    {"wiz_where", CMD_KEY, C('o')},
			    {"wiz_level_tele", CMD_KEY, C('v')},
			    {"wiz_wish", CMD_KEY, C('w')},

			    /* wizard extended commands */
#ifdef WIZARD
			    {"light sources", CMD_EXT, 0},
			    {"seenv", CMD_EXT, 0},
			    {"stats", CMD_EXT, 0},
			    {"timeout", CMD_EXT, 0},
			    {"vision", CMD_EXT, 0},
#ifdef DEBUG
			    {"wizdebug", CMD_EXT, 0},
#endif /* DEBUG */
			    {"wmode", CMD_EXT, 0},
#endif /* WIZARD */
			    {"pray", CMD_EXT, 0},
			    {"adjust", CMD_EXT, 0},
			    {"chat", CMD_EXT, 0},
			    {"conduct", CMD_EXT, 0},
			    {"dip", CMD_EXT, 0},

			    {"enhance", CMD_EXT, 0},
			    {"force", CMD_EXT, 0},
			    {"invoke", CMD_EXT, 0},
			    {"jump", CMD_EXT, 0},
			    {"loot", CMD_EXT, 0},

			    {"monster", CMD_EXT, 0},
			    {"name", CMD_EXT, 0},
			    {"offer", CMD_EXT, 0},
			    {"quit", CMD_EXT, 0},
			    {"ride", CMD_EXT, 0},

			    {"rub", CMD_EXT, 0},
			    {"sit", CMD_EXT, 0},
			    {"turn", CMD_EXT, 0},
			    {"twoweapon", CMD_EXT, 0},
			    {"untrap", CMD_EXT, 0},

			    {"version", CMD_EXT, 0},
			    {"wipe", CMD_EXT, 0},

			    /* Lisp port specific commands  */
			    {"options", CMD_LISP, 0},

			    {0, CMD_KEY, '\0'} };

/* This variable is set when the user has selected an extended command. */
static int extended_cmd_id;
  
/* Interface definition, for windows.c */
struct window_procs lisp_procs = {
  "lisp",
  WC_COLOR|WC_HILITE_PET,
  0L,
  lisp_init_nhwindows,
  lisp_player_selection,
  lisp_askname,
  lisp_get_nh_event,
  lisp_exit_nhwindows,
  lisp_suspend_nhwindows,
  lisp_resume_nhwindows,
  lisp_create_nhwindow,
  lisp_clear_nhwindow,
  lisp_display_nhwindow,
  lisp_destroy_nhwindow,
  lisp_curs,
  lisp_putstr,
  lisp_display_file,
  lisp_start_menu,
  lisp_add_menu,
  lisp_end_menu,
  lisp_select_menu,
  genl_message_menu,
  lisp_update_inventory,
  lisp_mark_synch,
  lisp_wait_synch,
#ifdef CLIPPING
  lisp_cliparound,
#endif
#ifdef POSITIONBAR
  lisp_update_positionbar,
#endif
  lisp_print_glyph,
  lisp_raw_print,
  lisp_raw_print_bold,
  lisp_nhgetch,
  lisp_nh_poskey,
  lisp_nhbell,
  lisp_doprev_message,
  lisp_yn_function,
  lisp_getlin,
  lisp_get_ext_cmd,
  lisp_number_pad,
  lisp_delay_output,
#ifdef CHANGE_COLOR	/* the Mac uses a palette device */
  donull,
  donull,
#endif
  /* other defs that really should go away (they're tty specific) */
  lisp_start_screen,
  lisp_end_screen,
  lisp_outrip,
  genl_preference_update,
};

/* macros for printing lisp output */
#define lisp_cmd(s,body)			\
  do						\
    {						\
      printf("(nhapi-%s ",s);		\
      body;					\
      printf(")\n");				\
    }						\
  while (0)
/* #define lisp_cmd0(s) printf ("(nhapi-%s)\n", s) */
#define lisp_list(body)				\
  do						\
    {						\
      printf("(");				\
      body; 					\
      printf(") ");				\
    }						\
  while (0)

#define lisp_open printf("(")
#define lisp_close printf(") ")
#define lisp_quote printf("'")
#define lisp_dot printf(". ")
#define lisp_t printf("t ")
#define lisp_nil printf("nil ")
#define lisp_literal(x)				\
  do						\
    {						\
      lisp_quote;				\
      printf ("%s ", x);			\
    }						\
  while (0)
#define lisp_cons(x,y)				\
  do						\
    {						\
      lisp_open;				\
      x;					\
      lisp_dot;					\
      y;					\
      lisp_close;				\
    }						\
  while (0)
#define lisp_int(i) printf("%d ",i)
#define lisp_long(i) printf("%ld", i)
#define lisp_coord(c) printf("'(%d,%d) ",c.x,c.y)
#define lisp_boolean(i) printf("%s ",i?"t":"nil")
#define lisp_string(s)					\
  do							\
     {							\
       int nhi;						\
       printf ("\"");					\
       if (s)						\
	 for (nhi=0;nhi<strlen(s);nhi++)		\
	   {						\
	     if (s[nhi] == 34 				\
		 || s[nhi] == 92) putchar('\\');	\
	     putchar(s[nhi]);				\
	   }						\
       printf("\" ");					\
     }							\
  while (0)

static const char*
attr_to_string(attr)
     int attr;
{
  switch (attr)
    {
    case ATR_NONE:
      return "atr-none";
    case ATR_ULINE:
      return "atr-uline";
    case ATR_BOLD:
      return "atr-bold";
    case ATR_BLINK:
      return "atr-blink";
    case ATR_INVERSE:
      return "atr-inverse";
    default:
      /* Should never happen. */
      impossible ("Invalid attribute code.");
      exit (EXIT_FAILURE);
      break;
    }
}

static const char*
special_glyph_to_string(special)
     unsigned special;
{
  switch (special)
    {
    case MG_CORPSE:
      return "corpse";
    case MG_INVIS:
      return "invis";
    case MG_DETECT:
      return "detect";
    case MG_PET:
      return "pet";
    case MG_RIDDEN:
      return "ridden";
    }
  
  /* If it's a combination, just return special. */
  if (special)
    return "special";
  else
    return "none";
}

static const char*
wintype_to_string(type)
     int type;
{
  switch (type)
    {
    case NHW_MAP:
      return "nhw-map";
    case NHW_MESSAGE:
      return "nhw-message";
    case NHW_STATUS:
      return "nhw-status";
    case NHW_MENU:
      return "nhw-menu";
    case NHW_TEXT:
      return "nhw-text";
    default:
      fprintf (stderr, "Invalid window code\n");
      exit (EXIT_FAILURE);
      break;
    }
}

static const char*
how_to_string (how)
     int how;
{
  switch (how)
    {
    case PICK_NONE:
      return "pick-none";
    case PICK_ONE:
      return "pick-one";
    case PICK_ANY:
      return "pick-any";
    default:
      impossible ("Invalid how value %d", how);
    }
  return NULL;
}

static int
read_int (prompt, i)
     const char* prompt;
     int *i;
{
  char line[BUFSZ];
  int rv;
  printf ("%s> ", prompt);
  fflush(stdout);
  fgets (line, BUFSZ, stdin);
  rv = sscanf (line, "%d", i);
  if (rv != 1) *i = -1;
  return rv;
}

static int
read_string (prompt, str)
     const char* prompt;
     char **str;
{
  char* rv;
  int len;
  int size;
  char tmp[BUFSZ];

  len = 0;
  size = BUFSZ * 2;
  *str = malloc (size);
  (*str)[0] = '\0';

  printf ("%s> ", prompt);
  fflush(stdout);
  do
    {
      /* Read the string */
      rv = fgets (tmp, BUFSZ, stdin);
      if (rv == NULL)
	break;

      len += strlen (tmp);
      if (len >= size - 1)
	{
	  size *= 2;
	  *str = realloc (*str, size);
	  if (*str == NULL)
	    panic ("Memory allocation failure; cannot get %u bytes", size);
	}
      strcat (*str, tmp);
    } while (tmp[strlen (tmp) - 1] != '\n');

  /* Did we read a string or error out? */
  if (rv == NULL)
    {
      free (*str);
      return -1;
    }
  else
    {
      /* chop the newline */
      (*str) [strlen (*str) - 1] = '\0';
      return 0;
    }
}


static int
read_command (prompt, cmd, count)
     const char *prompt;
     char *cmd;
     char *count;
{
  char *buf;
  int rv;
  cmd[0] = '\0';
  *count = 0;
  if (read_string (prompt, &buf) == -1)
    return -1;
  rv = sscanf (buf, "%s %s", cmd, count);
  free (buf);
  if (rv != 2) *count = 0;
  return rv;
}

void
bail(mesg)
     const char *mesg;
{
  clearlocks ();
  lisp_exit_nhwindows (mesg);
  terminate (EXIT_SUCCESS);
  /*NOTREACHED*/
}

void
win_lisp_init ()
{
  /* Code to be executed on startup. */
}

void
lisp_player_selection ()
{
	int i, k, n;
	char pick4u = 'n', thisch, lastch = 0;
	char pbuf[QBUFSZ], plbuf[QBUFSZ];
	winid win;
	anything any;
	menu_item *selected = 0;

	/* prevent an unnecessary prompt */
	rigid_role_checks();

	/* Should we randomly pick for the player? */
	if (!flags.randomall &&
	    (flags.initrole == ROLE_NONE || flags.initrace == ROLE_NONE ||
	     flags.initgend == ROLE_NONE || flags.initalign == ROLE_NONE)) {

	  pick4u = lisp_yn_function ("Shall I pick a character for you? [ynq] ", "ynq", 'y');

	  if (pick4u != 'y' && pick4u != 'n') 
	    {
	    give_up:	/* Quit */
	      if (selected) free((genericptr_t) selected);
	      bail((char *)0);
	      /*NOTREACHED*/
	      return;
	    }

	}

	(void)  root_plselection_prompt(plbuf, QBUFSZ - 1,
			flags.initrole, flags.initrace, flags.initgend, flags.initalign);

	/* Select a role, if necessary */
	/* we'll try to be compatible with pre-selected race/gender/alignment,
	 * but may not succeed */
	if (flags.initrole < 0) {
	    char rolenamebuf[QBUFSZ];
	    /* Process the choice */
	    if (pick4u == 'y' || flags.initrole == ROLE_RANDOM || flags.randomall) {
		/* Pick a random role */
		flags.initrole = pick_role(flags.initrace, flags.initgend,
						flags.initalign, PICK_RANDOM);
		if (flags.initrole < 0) {
/* 		    lisp_putstr(BASE_WINDOW, 0, "Incompatible role!"); */
		    flags.initrole = randrole();
		}
 	    } else {
		/* Prompt for a role */
		win = create_nhwindow(NHW_MENU);
		start_menu(win);
		any.a_void = 0;         /* zero out all bits */
		for (i = 0; roles[i].name.m; i++) {
		    if (ok_role(i, flags.initrace, flags.initgend,
							flags.initalign)) {
			any.a_int = i+1;	/* must be non-zero */
			thisch = lowc(roles[i].name.m[0]);
			if (thisch == lastch) thisch = highc(thisch);
			if (flags.initgend != ROLE_NONE && flags.initgend != ROLE_RANDOM) {
				if (flags.initgend == 1  && roles[i].name.f)
					Strcpy(rolenamebuf, roles[i].name.f);
				else
					Strcpy(rolenamebuf, roles[i].name.m);
			} else {
				if (roles[i].name.f) {
					Strcpy(rolenamebuf, roles[i].name.m);
					Strcat(rolenamebuf, "/");
					Strcat(rolenamebuf, roles[i].name.f);
				} else 
					Strcpy(rolenamebuf, roles[i].name.m);
			}	
			add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, thisch,
			    0, ATR_NONE, an(rolenamebuf), MENU_UNSELECTED);
			lastch = thisch;
		    }
		}
		any.a_int = pick_role(flags.initrace, flags.initgend,
				    flags.initalign, PICK_RANDOM)+1;
		if (any.a_int == 0)	/* must be non-zero */
		    any.a_int = randrole()+1;
		add_menu(win, NO_GLYPH, MENU_DEFCNT, &any , '*', 0, ATR_NONE,
				"Random", MENU_UNSELECTED);
		any.a_int = i+1;	/* must be non-zero */
		add_menu(win, NO_GLYPH, MENU_DEFCNT, &any , 'q', 0, ATR_NONE,
				"Quit", MENU_UNSELECTED);
		Sprintf(pbuf, "Pick a role for your %s", plbuf);
		end_menu(win, pbuf);
		n = select_menu(win, PICK_ONE, &selected);
		destroy_nhwindow(win);

		/* Process the choice */
		if (n != 1 || selected[0].item.a_int == any.a_int)
		    goto give_up;		/* Selected quit */

		flags.initrole = selected[0].item.a_int - 1;
		free((genericptr_t) selected),	selected = 0;
	    }
	    (void)  root_plselection_prompt(plbuf, QBUFSZ - 1,
			flags.initrole, flags.initrace, flags.initgend, flags.initalign);
	}
	
	/* Select a race, if necessary */
	/* force compatibility with role, try for compatibility with
	 * pre-selected gender/alignment */
	if (flags.initrace < 0 || !validrace(flags.initrole, flags.initrace)) {
	    /* pre-selected race not valid */
	    if (pick4u == 'y' || flags.initrace == ROLE_RANDOM || flags.randomall) {
		flags.initrace = pick_race(flags.initrole, flags.initgend,
							flags.initalign, PICK_RANDOM);
		if (flags.initrace < 0) {
/* 		    lisp_putstr(BASE_WINDOW, 0, "Incompatible race!"); */
		    flags.initrace = randrace(flags.initrole);
		}
	    } else {	/* pick4u == 'n' */
		/* Count the number of valid races */
		n = 0;	/* number valid */
		k = 0;	/* valid race */
		for (i = 0; races[i].noun; i++) {
		    if (ok_race(flags.initrole, i, flags.initgend,
							flags.initalign)) {
			n++;
			k = i;
		    }
		}
		if (n == 0) {
		    for (i = 0; races[i].noun; i++) {
			if (validrace(flags.initrole, i)) {
			    n++;
			    k = i;
			}
		    }
		}

		/* Permit the user to pick, if there is more than one */
		if (n > 1) {
		    win = create_nhwindow(NHW_MENU);
		    start_menu(win);
		    any.a_void = 0;         /* zero out all bits */
		    for (i = 0; races[i].noun; i++)
			if (ok_race(flags.initrole, i, flags.initgend,
							flags.initalign)) {
			    any.a_int = i+1;	/* must be non-zero */
			    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, races[i].noun[0],
				0, ATR_NONE, races[i].noun, MENU_UNSELECTED);
			}
		    any.a_int = pick_race(flags.initrole, flags.initgend,
					flags.initalign, PICK_RANDOM)+1;
		    if (any.a_int == 0)	/* must be non-zero */
			any.a_int = randrace(flags.initrole)+1;
		    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any , '*', 0, ATR_NONE,
				    "Random", MENU_UNSELECTED);
		    any.a_int = i+1;	/* must be non-zero */
		    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any , 'q', 0, ATR_NONE,
				    "Quit", MENU_UNSELECTED);
		    Sprintf(pbuf, "Pick the race of your %s", plbuf);
		    end_menu(win, pbuf);
		    n = select_menu(win, PICK_ONE, &selected);
		    destroy_nhwindow(win);
		    if (n != 1 || selected[0].item.a_int == any.a_int)
			goto give_up;		/* Selected quit */

		    k = selected[0].item.a_int - 1;
		    free((genericptr_t) selected),	selected = 0;
		}
		flags.initrace = k;
	    }
	    (void)  root_plselection_prompt(plbuf, QBUFSZ - 1,
			flags.initrole, flags.initrace, flags.initgend, flags.initalign);
	}

	/* Select a gender, if necessary */
	/* force compatibility with role/race, try for compatibility with
	 * pre-selected alignment */
	if (flags.initgend < 0 || !validgend(flags.initrole, flags.initrace,
						flags.initgend)) {
	    /* pre-selected gender not valid */
	    if (pick4u == 'y' || flags.initgend == ROLE_RANDOM || flags.randomall) {
		flags.initgend = pick_gend(flags.initrole, flags.initrace,
						flags.initalign, PICK_RANDOM);
		if (flags.initgend < 0) {
/* 		    lisp_putstr(BASE_WINDOW, 0, "Incompatible gender!"); */
		    flags.initgend = randgend(flags.initrole, flags.initrace);
		}
	    } else {	/* pick4u == 'n' */
		/* Count the number of valid genders */
		n = 0;	/* number valid */
		k = 0;	/* valid gender */
		for (i = 0; i < ROLE_GENDERS; i++) {
		    if (ok_gend(flags.initrole, flags.initrace, i,
							flags.initalign)) {
			n++;
			k = i;
		    }
		}
		if (n == 0) {
		    for (i = 0; i < ROLE_GENDERS; i++) {
			if (validgend(flags.initrole, flags.initrace, i)) {
			    n++;
			    k = i;
			}
		    }
		}

		/* Permit the user to pick, if there is more than one */
		if (n > 1) {
		    win = create_nhwindow(NHW_MENU);
		    start_menu(win);
		    any.a_void = 0;         /* zero out all bits */
		    for (i = 0; i < ROLE_GENDERS; i++)
			if (ok_gend(flags.initrole, flags.initrace, i,
							    flags.initalign)) {
			    any.a_int = i+1;
			    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, genders[i].adj[0],
				0, ATR_NONE, genders[i].adj, MENU_UNSELECTED);
			}
		    any.a_int = pick_gend(flags.initrole, flags.initrace,
					    flags.initalign, PICK_RANDOM)+1;
		    if (any.a_int == 0)	/* must be non-zero */
			any.a_int = randgend(flags.initrole, flags.initrace)+1;
		    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any , '*', 0, ATR_NONE,
				    "Random", MENU_UNSELECTED);
		    any.a_int = i+1;	/* must be non-zero */
		    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any , 'q', 0, ATR_NONE,
				    "Quit", MENU_UNSELECTED);
		    Sprintf(pbuf, "Pick the gender of your %s", plbuf);
		    end_menu(win, pbuf);
		    n = select_menu(win, PICK_ONE, &selected);
		    destroy_nhwindow(win);
		    if (n != 1 || selected[0].item.a_int == any.a_int)
			goto give_up;		/* Selected quit */

		    k = selected[0].item.a_int - 1;
		    free((genericptr_t) selected),	selected = 0;
		}
		flags.initgend = k;
	    }
	    (void)  root_plselection_prompt(plbuf, QBUFSZ - 1,
			flags.initrole, flags.initrace, flags.initgend, flags.initalign);
	}

	/* Select an alignment, if necessary */
	/* force compatibility with role/race/gender */
	if (flags.initalign < 0 || !validalign(flags.initrole, flags.initrace,
							flags.initalign)) {
	    /* pre-selected alignment not valid */
	    if (pick4u == 'y' || flags.initalign == ROLE_RANDOM || flags.randomall) {
		flags.initalign = pick_align(flags.initrole, flags.initrace,
							flags.initgend, PICK_RANDOM);
		if (flags.initalign < 0) {
/* 		    lisp_putstr(BASE_WINDOW, 0, "Incompatible alignment!"); */
		    flags.initalign = randalign(flags.initrole, flags.initrace);
		}
	    } else {	/* pick4u == 'n' */
		/* Count the number of valid alignments */
		n = 0;	/* number valid */
		k = 0;	/* valid alignment */
		for (i = 0; i < ROLE_ALIGNS; i++) {
		    if (ok_align(flags.initrole, flags.initrace, flags.initgend,
							i)) {
			n++;
			k = i;
		    }
		}
		if (n == 0) {
		    for (i = 0; i < ROLE_ALIGNS; i++) {
			if (validalign(flags.initrole, flags.initrace, i)) {
			    n++;
			    k = i;
			}
		    }
		}

		/* Permit the user to pick, if there is more than one */
		if (n > 1) {
		    win = create_nhwindow(NHW_MENU);
		    start_menu(win);
		    any.a_void = 0;         /* zero out all bits */
		    for (i = 0; i < ROLE_ALIGNS; i++)
			if (ok_align(flags.initrole, flags.initrace,
							flags.initgend, i)) {
			    any.a_int = i+1;
			    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any, aligns[i].adj[0],
				 0, ATR_NONE, aligns[i].adj, MENU_UNSELECTED);
			}
		    any.a_int = pick_align(flags.initrole, flags.initrace,
					    flags.initgend, PICK_RANDOM)+1;
		    if (any.a_int == 0)	/* must be non-zero */
			any.a_int = randalign(flags.initrole, flags.initrace)+1;
		    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any , '*', 0, ATR_NONE,
				    "Random", MENU_UNSELECTED);
		    any.a_int = i+1;	/* must be non-zero */
		    add_menu(win, NO_GLYPH, MENU_DEFCNT, &any , 'q', 0, ATR_NONE,
				    "Quit", MENU_UNSELECTED);
		    Sprintf(pbuf, "Pick the alignment of your %s", plbuf);
		    end_menu(win, pbuf);
		    n = select_menu(win, PICK_ONE, &selected);
		    destroy_nhwindow(win);
		    if (n != 1 || selected[0].item.a_int == any.a_int)
			goto give_up;		/* Selected quit */

		    k = selected[0].item.a_int - 1;
		    free((genericptr_t) selected),	selected = 0;
		}
		flags.initalign = k;
	    }
	}
	/* Success! */
/* 	lisp_display_nhwindow(BASE_WINDOW, FALSE); */
}

/* Reads from standard in, the player's name. */
void
lisp_askname ()
{
  char *line;
  lisp_cmd ("askname",);
  read_string ("string", &line);
  strncpy (plname, line, PL_NSIZ);
  plname[PL_NSIZ-1] = '\0';
  free (line);
}

/* This is a noop for tty and X, so should it be a noop for us too? */
void
lisp_get_nh_event ()
{
/*   lisp_cmd ("get-event",); */
}

/* Global Functions */
void
lisp_raw_print(str)
     const char *str;
{
  lisp_cmd ("raw-print", lisp_string (str));
}

void
lisp_raw_print_bold(str)
     const char *str;
{
  lisp_cmd ("raw-print-bold", lisp_string (str));
}

void
lisp_curs(window, x, y)
     winid window;
     int x, y;
{
  if (window == WIN_MAP)
    lisp_cmd ("curs",
	      lisp_int (x);
	      lisp_int (y));
  else if (window == WIN_STATUS)
    {
      /* do nothing */
    }
  else
    lisp_cmd ("error", lisp_string("lisp_curs bad window"); lisp_int (window));
}

/* Send the options to the lisp process */
static void
get_options()
{
  lisp_cmd ("options",
	    lisp_boolean(iflags.cbreak);	/* in cbreak mode, rogue format */
	    lisp_boolean(iflags.DECgraphics);	/* use DEC VT-xxx extended character set */
	    lisp_boolean(iflags.echo);		/* 1 to echo characters */
	    lisp_boolean(iflags.IBMgraphics);	/* use IBM extended character set */
	    lisp_int(iflags.msg_history);	/* hint: # of top lines to save */
	    lisp_boolean(iflags.num_pad);	/* use numbers for movement commands */
	    lisp_boolean(iflags.news);		/* print news */
	    lisp_boolean(iflags.window_inited); /* true if init_nhwindows() completed */
	    lisp_boolean(iflags.vision_inited); /* true if vision is ready */
	    lisp_boolean(iflags.menu_tab_sep);	/* Use tabs to separate option menu fields */
	    lisp_boolean(iflags.menu_requested); /* Flag for overloaded use of 'm' prefix
						  * on some non-move commands */
	    lisp_int(iflags.num_pad_mode);
	    lisp_int(iflags.purge_monsters);	/* # of dead monsters still on fmon list */
/* 	    lisp_int(*iflags.opt_booldup);	/\* for duplication of boolean opts in config file *\/ */
/* 	    lisp_int(*iflags.opt_compdup);	/\* for duplication of compound opts in config file *\/ */
	    lisp_int(iflags.bouldersym);	/* symbol for boulder display */
	    lisp_coord(iflags.travelcc);	/* coordinates for travel_cache */
#ifdef WIZARD
	    lisp_boolean(iflags.sanity_check); /* run sanity checks */
	    lisp_boolean(iflags.mon_polycontrol); /* debug: control monster polymorphs */
#endif
	    );
}

static void
generate_status_line ()
{
  /* Ripped from botl.c */
  int hp, hpmax;

  hp = Upolyd ? u.mh : u.uhp;
  hpmax = Upolyd ? u.mhmax : u.uhpmax;
  if(hp < 0) hp = 0;

  printf ("(nhapi-update-status ");
  lisp_quote;
  printf ("(");
  lisp_list (lisp_string ("name");
	     lisp_string (plname););
  if (Upolyd) 
    {
      lisp_list (lisp_string ("rank");
		 lisp_nil);
      lisp_list (lisp_string ("monster");
		 lisp_string (mons[u.umonnum].mname));
    }
  else
    {
      lisp_list (lisp_string ("rank");
		 lisp_string (rank_of(u.ulevel, Role_switch, flags.female)););;
      lisp_list (lisp_string ("monster");
		 lisp_nil);
    }

  lisp_list (lisp_string ("St");
	     lisp_int (ACURR(A_STR)););
  lisp_list (lisp_string ("Dx");
	     lisp_int (ACURR(A_DEX)););
  lisp_list (lisp_string ("Co");
	     lisp_int (ACURR(A_CON)););
  lisp_list (lisp_string ("In");
	     lisp_int (ACURR(A_INT)););
  lisp_list (lisp_string ("Wi");
	     lisp_int (ACURR(A_WIS)););
  lisp_list (lisp_string ("Ch");
	     lisp_int (ACURR(A_CHA)););
  lisp_list (lisp_string ("Align");
	     if (u.ualign.type == A_CHAOTIC)
	     lisp_string ("Chaotic");
	     else if (u.ualign.type == A_NEUTRAL)
	     lisp_string ("Neutral");
	     else
	     lisp_string ("Lawful"););

#ifdef SCORE_ON_BOTL
  lisp_list (lisp_string ("Score");
	     lisp_int (botl_score()););
#endif

  if (In_endgame(&u.uz))
    {
      lisp_list (lisp_string ("Dungeon");
		 if (Is_astralevel(&u.uz))
		 lisp_string ("Astral Plane");
		 else
		 lisp_string ("End Game"););
    }
  else
    {
      lisp_list (lisp_string ("Dungeon");
		 lisp_string (dungeons[u.uz.dnum].dname););
    }

      lisp_list (lisp_string ("Dlvl");
		 lisp_int (depth(&u.uz)););

  lisp_list (lisp_string ("$");
	     lisp_long (u.ugold););
  lisp_list (lisp_string ("HP");
	     lisp_int (hp););
  lisp_list (lisp_string ("HPmax");
	     lisp_int (hpmax););
  lisp_list (lisp_string ("PW");
	     lisp_int (u.uen););
  lisp_list (lisp_string ("PWmax");
	     lisp_int (u.uenmax););
  lisp_list (lisp_string ("AC");
	     lisp_int (u.uac););

  if (Upolyd)
    {
      lisp_list (lisp_string ("HD");
		 lisp_int (mons[u.umonnum].mlevel););
    }
  else
    {
      lisp_list (lisp_string ("HD");
		 lisp_nil);
    }

  lisp_list (lisp_string ("Level");
	     lisp_int (u.ulevel););
#ifdef EXP_ON_BOTL
  lisp_list (lisp_string ("XP");
	     lisp_long (u.uexp););
#endif
  lisp_list (lisp_string ("T");
	     lisp_long (moves););

  if (Confusion)
    lisp_list (lisp_string ("confusion"); lisp_string ("Conf"));
  else
    lisp_list (lisp_string ("confusion"); lisp_nil);

  if (u.uhs != 1)
    lisp_list (lisp_string ("hunger"); lisp_string (hunger_stat[u.uhs]));
  else
    lisp_list (lisp_string ("hunger"); lisp_nil);

  if (Sick) 
    {
      if (u.usick_type & SICK_VOMITABLE)
	lisp_list (lisp_string ("sick"); lisp_string ("FoodPois"));
      if (u.usick_type & SICK_NONVOMITABLE)
	lisp_list (lisp_string ("sick"); lisp_string ("Ill"));
    }
  else
    lisp_list (lisp_string ("sick"); lisp_nil);

  if (Blind)
    lisp_list (lisp_string ("blind"); lisp_string ("Blind"));
  else
    lisp_list (lisp_string ("blind"); lisp_nil);

  if (Stunned)
    lisp_list (lisp_string ("stunned"); lisp_string ("Stun"));
  else
    lisp_list (lisp_string ("stunned"); lisp_nil);

  if (Hallucination)
    lisp_list (lisp_string ("hallucination"); lisp_string ("Hallu"));
  else
    lisp_list (lisp_string ("hallucination"); lisp_nil);

  if (Slimed)
    lisp_list (lisp_string ("slimed"); lisp_string ("Slime"));
  else
    lisp_list (lisp_string ("slimed"); lisp_nil);

  if (near_capacity() > UNENCUMBERED)
    lisp_list (lisp_string ("encumbrance"); 
	       lisp_string (enc_stat[near_capacity()]));
  else
    lisp_list (lisp_string ("encumbrance"); lisp_nil);

  printf (" ))\n");
}

void
lisp_putstr(window, attr, str)
     winid window;
     int attr;
     const char *str;
{
  static char statline1[BUFSZ] = "";
  if (window == WIN_STATUS)
    {
      if (statline1[0]=='\0')
	Strcpy (statline1, str);
      else
	{
	  generate_status_line ();
	  statline1[0]='\0';
	}
    }
  else if (window == WIN_MESSAGE)
    lisp_cmd ("message", 
	      lisp_literal (attr_to_string (attr));
	      lisp_string (str));
  else
    lisp_cmd ("menu-putstr",
	      lisp_int (window);
	      lisp_literal (attr_to_string (attr));
	      lisp_string (str));
}

void
lisp_start_menu(window)
     winid window;
{
  lisp_menu_list_num = 0;
  lisp_current_accelerator = 'a';
  lisp_cmd ("start-menu", lisp_int (window));
}

void
lisp_add_menu(window, glyph, cnt, identifier, ch, gch, attr, str, preselected)
    winid window;		/* window to use, must be of type NHW_MENU */
    int glyph;			/* glyph to display with item (unused) */
    int cnt;                    /* max number of times this item can be selected */
    const anything *identifier;	/* what to return if selected */
    char ch;			/* keyboard accelerator (0 = pick our own) */
    char gch;			/* group accelerator (0 = no group) */
    int attr;			/* attribute for string (like tty_putstr()) */
    const char *str;		/* menu string */
    boolean preselected;	/* item is marked as selected */
{
  if (identifier->a_void)
    {
      lisp_menu_item_list[lisp_menu_list_num].identifier = *identifier;
      if (ch == 0)
	{
	  ch = lisp_menu_item_list[lisp_menu_list_num].accelerator = lisp_current_accelerator;
	  if (lisp_current_accelerator == 'z')
	    lisp_current_accelerator = 'A';
	  else
	    lisp_current_accelerator++;
	}      
      else
	lisp_menu_item_list[lisp_menu_list_num].accelerator = ch;

      lisp_menu_list_num++;
    }
  else
    ch = -1;

  lisp_cmd ("add-menu",
	    lisp_int (window);
	    lisp_int (glyph);
	    lisp_int (glyph2tile[glyph]);
	    lisp_int (ch);
	    lisp_int (gch);
	    lisp_literal (attr_to_string (attr));
	    lisp_string (str);
	    preselected ? lisp_t : lisp_nil;
	    /* The max count feature is already implemented in
	       nethack-el, so we can avoid emitting `cnt'.  */
	    /* lisp_int (cnt); */
	    );
}

void
lisp_end_menu(window, prompt)
    winid window;	/* menu to use */
    const char *prompt;	/* prompt to for menu */
{
  lisp_cmd ("end-menu",
	    lisp_int (window);
	    lisp_string (prompt));
}

static int
lisp_get_menu_identifier(ch, identifier)
     char ch;
     anything *identifier;
{
  int i;

  for(i=0; i < lisp_menu_list_num; i++)
    {
      if( lisp_menu_item_list[i].accelerator == ch )
	{
	  *identifier = lisp_menu_item_list[i].identifier;
	  return 1;
	}
    }

  return 0;
}

int
lisp_select_menu(window, how, menu_list)
    winid window;
    int how;
    menu_item **menu_list;
{
  const char *delim = "() \n";
  char *list;
  char *token;
  int size = 0;

  lisp_cmd ("select-menu",
	    lisp_int (window);
	    lisp_literal (how_to_string (how)));

  read_string ("menu", &list);

/*   lisp_prompt ("menu"); */
/*   fgets (list, LINESIZ, stdin); */

  /* The client should submit a structure like this:

   ((ch count) (ch count) (ch count) ...) 

   where ch is the accelerator for the menu item and count is the
   number of them to select.
   
   We strtok it so we just get id count id count id count. */

  token = strtok (list, delim);

  /* Start with some memory so realloc doesn't fail. */
  *menu_list = malloc (sizeof (menu_item));
  if (*menu_list == NULL)
    {
      panic ("Memory allocation failure; cannot get %u bytes", 
	     sizeof (menu_item));
    }
  size = 0;

  while (token != NULL)
    {
      /* Make more room in the array for the new item */
      size++;
      if ((*menu_list = realloc (*menu_list, size * sizeof (menu_item))) == NULL)
	{
	  panic ("Memory allocation failure; cannot get %u bytes", 
		 size * sizeof (menu_item));
	}

      /* assign the item ID */
      lisp_get_menu_identifier (atoi (token), &(*menu_list)[size-1].item );

      /* Read the item count */
      token = strtok (NULL, delim);
      (*menu_list)[size-1].count = atoi (token);

      /* read the next item ID */
      token = strtok (NULL, delim);
    }

  free (list);

  return size;
}

/* This is a tty-specific hack. Do we need it? */
char
lisp_message_menu(let, how, mesg)
     char let;
     int how;
     const char *mesg;
{
  lisp_cmd ("message-menu",
	    lisp_int (let);
	    lisp_literal (how_to_string (how));
	    lisp_string (mesg));
  return '\0';
}

static int
lisp_get_cmd(str)
     const char *str;
{
  int i;

  for (i=0; cmd_index[i].name != (char *)0; i++)
    {
      if (!strcmp (str, cmd_index[i].name)) 
	return i;
    }	  

  return -1;
}

static int
lisp_get_ext_cmd_id (str)
     const char *str;
{
  int i;

  for (i=0; extcmdlist[i].ef_txt != (char *)0; i++) {
    if (!strcmp (str, extcmdlist[i].ef_txt)) return i;
  }
  
  return -1;
}

/* static int */
/* num_digits(n) */
/*      int n; */
/* { */
/*   int i; */
/*   int ret = 1; */
  
/*   for (i=10;n / i; i *= 10) */
/*     { */
/*       ret++; */
/*     } */

/*   return ret; */
/* } */

/* static */
/* int */
/* power_of_ten (n) */
/*      int n; */
/* { */
/*   int i; */
/*   int power = 1; */

/*   for (i=0; i<n; i++) */
/*     { */
/*       power *= 10; */
/*     } */

/*   return power; */
/* } */

int
lisp_nhgetch()
{
  /* multi is not 0 if this  */
  static char count_buf[BUFSIZ] = "";
  static char *count_pos = count_buf;
  static int count_cmd = -1;
  int cmd;

  if (*count_pos)
    {
      char *tmp = count_pos;
      count_pos++;
      return *tmp;
    }

  if (count_cmd >= 0)
    {
      cmd = count_cmd;
      count_cmd = -1;
    }
  else
    {
      char cmdstr[BUFSZ];
      int nh_cmd = 0;

      while (!nh_cmd)
	{
	  read_command ("command", cmdstr, count_buf);

	  count_pos = count_buf;
	  cmd = lisp_get_cmd (cmdstr);
	  if (cmd == -1)
	    {
	      printf ("(nhapi-message 'atr-none \"undefined-command %s\")\n", cmdstr);
	    }
	  else if (cmd_index[cmd].type == CMD_LISP)
	    {
	      /* We have to handle Lisp commands in this inner loop, because
		 they don't interact with the nethack layer. */
	      /* FIXME: Maybe this should go in an array? */
	      if (!strcmp(cmd_index[cmd].name, "options"))
		{
		  get_options();
		}
	    } else {
	      /* We have a nh command. */
	      nh_cmd = 1;
	    }
	}

      if (atoi (count_pos) > 1)
	{
	  char* tmp = count_pos;
	  count_pos++;
	  count_cmd = cmd;
	  return *tmp;
	}
      else
	{
	  /* Since the count is 1, zero out the string. */
	  *count_pos = 0;
	}
    }

  if (cmd_index[cmd].type == CMD_KEY)
    {
      return cmd_index[cmd].cmd;
    }
  else if (cmd_index[cmd].type == CMD_EXT)
    {
      if ((extended_cmd_id = lisp_get_ext_cmd_id (cmd_index[cmd].name)) == -1)
	{
	  /* Can never happen. */
	  printf ("%s:%d: Bad extended command name\n", __FILE__,  __LINE__);
	}
      return '#';
    }
  else
    {
      impossible ("Impossible command type: %d", cmd_index[cmd].type);
    }
  return NULL;
}

int
lisp_nh_poskey(x, y, mod)
     int *x, *y, *mod;
{
/*    char scratch[256]; */

/*    printf ("(nethack-api-poskey)\n"); */

/*    scanf ("( %d %d '%255s )", x, y, scratch); */
/*    if (!strcmp (scratch, "click-1")) *mod = CLICK_1; */
/*    else *mod = CLICK_2; */

/*    return 0; */

  return lisp_nhgetch();
}

static boolean inven_win_created = FALSE;

/* These globals are used to keep track of window IDs. */
static winid *winid_list = NULL;
static int winid_list_len = 0;
static int winid_list_max = 0;

/* returns index into winid_list that can be used. */
static int
find_empty_cell ()
{
  int i;

  /* Check for a vacant spot in the list. */
  for (i=0; i<winid_list_len; i++)
    {
      if (winid_list[i] == -1) return i;
    }

  /* no vacant ones, so grow the array. */
  if (winid_list_len >= winid_list_max)
    {
      winid_list_max *= 2;
      winid_list = realloc (winid_list, sizeof (int) * winid_list_max);
      if (winid_list == NULL)
	bail ("Out of memory\n");
    }
  winid_list_len++;

  return winid_list_len-1;
}

static int
winid_is_taken (winid n)
{
  int i;

  for (i=0; i<winid_list_len; i++)
    if (winid_list[i] == n) return 1;
  
  return 0;
}

static int
add_winid (winid n)
{
  if (winid_is_taken (n)) return 0; /* failed. */
  
  winid_list[find_empty_cell()] = n;
  return 1; /* success! */
}

static winid
get_unique_winid ()
{
  winid i;
  
  /* look for a unique number, and add it to the list of taken
     numbers. */
  i = 0;
  while (!add_winid (i)) i++;

  return i;	  
}

/* When a window is destroyed, it gives back its window number with
   this function. */
static void
return_winid (winid n)
{
  int i;

  for (i=0; i<winid_list_len; i++)
    {
      if (winid_list[i] == n) 
	{
	  winid_list[i] = -1;
	  return;
	}
    }
}

static void
init_winid_list ()
{
  winid_list_max = 10;
  winid_list_len = 0;

  winid_list = malloc (winid_list_max * sizeof (int));
}

/* Prints a create_nhwindow function and expects from stdin the id of
   this new window as a number. */
winid
lisp_create_nhwindow(type)
     int type;
{
  winid id = get_unique_winid();

  switch (type)
    {
    case NHW_MESSAGE:
      lisp_cmd ("create-message-window",);
      break;
    case NHW_MAP:
      lisp_cmd ("create-map-window",);
      break;
    case NHW_STATUS:
      lisp_cmd ("create-status-window",);
      break;
    case NHW_TEXT:
      lisp_cmd ("create-text-window", lisp_int (id));
      break;
    case NHW_MENU:
      if (!inven_win_created)
	{
	  lisp_cmd ("create-inventory-window", lisp_int (id));
	  inven_win_created = TRUE;
	}
      else
	lisp_cmd ("create-menu-window", lisp_int (id));
      break;
    default:
      impossible ("Unknown window type: %d", type);
    };

  return id;
}

void
lisp_clear_nhwindow(window)
     winid window;
{
  if (window == WIN_MESSAGE)
    lisp_cmd ("clear-message",);
  else if (window == WIN_MAP)
    lisp_cmd ("clear-map",);
  else
    /* are other window types ever cleared? */
    lisp_cmd ("error", lisp_string("clearing unknown winid"));
}

void
lisp_display_nhwindow(window, blocking)
     winid window;
     boolean blocking;
{
  /* don't send display messages for anything but menus */
  char *dummy;
  if (window != WIN_MESSAGE && window != WIN_STATUS && window != WIN_MAP)
    {
      lisp_cmd ("display-menu", lisp_int (window));
      read_string ("dummy", &dummy);
      free (dummy);
    }
  else if (blocking)
    {
      if (window == WIN_MESSAGE)
	{
	  /* blocking on the message window happens only at the end of
	     the game */
	  lisp_cmd ("end",);
	}
      else
	{
	  lisp_cmd ("block",);
	  read_string ("dummy", &dummy);
	  free (dummy);
	}
    }
  else if (window == WIN_STATUS)
    {
      /* initial window setup hack here :) */
      lisp_cmd ("restore-window-configuration",);
    }
}

void
lisp_destroy_nhwindow(window)
     winid window;
{
  if ((window != WIN_STATUS)
      && (window != WIN_MESSAGE)
      && (window != WIN_MAP))
    {
      lisp_cmd ("destroy-menu", lisp_int (window));
      return_winid (window);
    }
}

void
lisp_update_inventory()
{
  lisp_cmd ("update-inventory",);
}

int
lisp_doprev_message()
{
  lisp_cmd ("doprev-message",);
  return 0;
}

void
lisp_nhbell()
{
  lisp_cmd ("nhbell",);
}

/* Can be an empty call says window.doc. */
void
lisp_mark_synch()
{
  /* lisp_cmd ("mark-sync",); */
}

void
lisp_wait_synch()
{
  lisp_cmd ("wait-synch",);
}

/* Since nethack will never be suspended, we need not worry about this
   function. */
void
lisp_resume_nhwindows()
{
  return;
}

/* Since nethack will never be suspended, we need not worry about this
   function. */
void
lisp_suspend_nhwindows(str)
     const char *str;
{
  return;
}

/* All keys are defined in emacs, so number_pad makes no sense. */
void
lisp_number_pad(state)
     int state;
{
  return;
}

void
lisp_init_nhwindows(argcp,argv)
     int* argcp;
     char** argv;
{
  int i;

  /* Print each command-line option, constructing a list of strings */
  lisp_cmd ("init-nhwindows",
	    for (i=0; i<*argcp; i++) 
	      lisp_string (argv[i]));

  /* FIXME: doesn't remove the arguments parsed, as specified in the
     api doc. */

  /* Setup certain flags lisp clients need */
  iflags.num_pad = FALSE;
#ifdef EXP_ON_BOTL		/* we are going to lose if Nethack is
				   compiled without this option -rcy */
  flags.showexp = TRUE;
#endif
  flags.time = TRUE;

  /* inform nethack that the windows have been initialized. */
  iflags.window_inited = TRUE;

  init_winid_list();
}

void
lisp_exit_nhwindows (str)
     const char *str;
{
  lisp_cmd ("exit-nhwindows ", lisp_string (str));
}

void
lisp_delay_output()
{
  char *dummy;
  lisp_cmd ("delay-output",);
  read_string ("dummy", &dummy);
  free (dummy);
}

void
lisp_getlin(question, input)
     const char *question;
     char *input;
{
  char *tmp;
  lisp_cmd ("getlin", lisp_string (question));
  read_string ("string", &tmp);
  /* FIXME: potential buffer overflow. */
  strcpy (input, tmp);
}

int
lisp_get_ext_cmd()
{
/*    int cmd; */
/*    int i; */

/*    printf ("(nethack-api-get-ext-cmd '("); */

/*    for (i=0; extcmdlist[i].ef_txt != (char *)0; i++) { */
/*      printf ("(\"%s\" . %d)", extcmdlist[i].ef_txt, i); */
/*    } */
/*    printf ("))\n"); */

/*    scanf ("%d", &cmd); */

  /* This is set when the user chooses an extended command. */
  return extended_cmd_id;
}

void
#ifdef FILE_AREAS
lisp_display_file(farea, fname, complain)
     const char *farea;
#else
lisp_display_file(fname, complain)
#endif
     const char *fname;
     boolean complain;
{
  lisp_cmd ("display-file",
	    lisp_string (fname);
	    complain ? lisp_t : lisp_nil);;
}

char
lisp_yn_function(ques, choices, def)
     const char *ques;
     const char *choices;
     char def;
{
  int answer;

  /* Some questions have special functions. */
  if (!strncmp (ques, "In what direction", 17)
      || !strncmp (ques, "Talk to whom? (in what direction)", 33))
    {
      char *dir;
      lisp_cmd ("ask-direction",
		lisp_string (ques));
      read_string ("direction", &dir);
      if (!strcmp (dir, "n"))
	answer = 'k';
      else if (!strcmp (dir, "s"))
	answer = 'j';
      else if (!strcmp (dir, "e"))
	answer = 'l';
      else if (!strcmp (dir, "w"))
	answer = 'h';
      else if (!strcmp (dir, "ne"))
	answer = 'u';
      else if (!strcmp (dir, "nw"))
	answer = 'y';
      else if (!strcmp (dir, "se"))
	answer = 'n';
      else if (!strcmp (dir, "sw"))
	answer = 'b';
      else if (!strcmp (dir, "up"))
	answer = '<';
      else if (!strcmp (dir, "down"))
	answer = '>';
      else if (!strcmp (dir, "self"))
	answer = '.';
      else
	{
	  if (def == '\0')
	    answer = 0x20;		/* space */
	  else
	    answer = def;
	}

      free (dir);
    }
  else
    {
      lisp_cmd ("yn-function",
		lisp_string (ques);
		lisp_string (choices);
		lisp_int (def));
      read_int ("number", &answer);
    }

  return (char)answer;
}

#ifdef POSITIONBAR
void
lisp_update_positionbar(features)
     char *features;
{
  lisp_cmd ("update-positionbar", lisp_string (features));
}
#endif

#define zap_color(n)  zapcolors[n]
#define cmap_color(n) defsyms[n].color
#define obj_color(n)  objects[n].oc_color
#define mon_color(n)  mons[n].mcolor
#define invis_color(n) NO_COLOR
#define pet_color(n)  mons[n].mcolor
#define warn_color(n) def_warnsyms[n].color

void
lisp_print_glyph(window, x, y, glyph)
    winid window;
    xchar x, y;
    int glyph;
{
    glyph_t ch;
    int	    color;
    unsigned special;

    /* map glyph to character and color */
    mapglyph(glyph, &ch, &color, &special, x, y);

    /* If the user doesn't want to highlight the pet, then we erase
       the PET bit from special. In the lisp code the special argument
       will be 'pet if the glyph is a pet and will be printed in the
       color of the pet highlight face. But we don't want this if the
       user hasn't turned on hilite_pet. */
    if (!iflags.hilite_pet)
      {
	special &= ~MG_PET;
      }

    if (window == WIN_MAP)
      {
	/* The last parameter, special, is optional. It is only
	   present when the tile is special in some way. FIXME: This
	   duplicate code is a bit gross. */
	if (special)
	  {
	    lisp_cmd ("print-glyph",
		      lisp_int (x);
		      lisp_int (y);
		      lisp_int (color);
		      lisp_int (glyph);
		      lisp_int (glyph2tile[glyph]);
		      lisp_int (ch);
		      lisp_literal (special_glyph_to_string (special)););
	  }
	else
	  {
	    lisp_cmd ("print-glyph",
		      lisp_int (x);
		      lisp_int (y);
		      lisp_int (color);
		      lisp_int (glyph);
		      lisp_int (glyph2tile[glyph]);
		      lisp_int (ch););
	  }
      }
    else
      lisp_cmd ("error",
		lisp_string ("lisp_print_glyph bad window");
		lisp_int (window));
}

#ifdef CLIPPING
void
lisp_cliparound(x, y)
     int x;
     int y;
{
  /* as far as I can tell, the x and y values here are exactly the
     ones given by the next lisp_curs call, so its redundant
     information -rcy */

  /*   lisp_cmd ("cliparound", lisp_int (x); lisp_int (y)); */
}
#endif

void lisp_start_screen() { return; } /* called from setftty() in unixtty.c */
void lisp_end_screen() {return; }    /* called from settty() in unixtty.c */

static void
get_death_text (buf)
     char buf[BUFSZ];
{
  
}

void
lisp_outrip(window, how)
     winid window;
     int how;
{
  lisp_cmd ("outrip",
	    lisp_int (window);	    
	    lisp_string (plname);
	    lisp_long (u.ugold);
	    lisp_string ("Died while trying to finish nethack-el."));
}
