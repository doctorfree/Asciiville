/*	SCCS Id: @(#)gnbind.c	3.4	2000/07/16	*/
/* Copyright (C) 1998 by Erik Andersen <andersee@debian.org> */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "dlb.h"
#include "func_tab.h"

#include <termios.h>
#include <unistd.h>

#include "dummy_graphics.h"

/*
 * This file implements the interface between the window port specific
 * code in the Gnome port and the rest of the nethack game engine. 
*/

/* Interface definition, for windows.c */
struct window_procs dummy_procs = {
    "dummy",
    0L,
    0L,
    dummy_init_nhwindows,
    dummy_player_selection,
    dummy_askname,
    dummy_get_nh_event,
    dummy_exit_nhwindows,
    dummy_suspend_nhwindows,
    dummy_resume_nhwindows,
    dummy_create_nhwindow,
    dummy_clear_nhwindow,
    dummy_display_nhwindow,
    dummy_destroy_nhwindow,
    dummy_curs,
    dummy_putstr,
    dummy_display_file,
    dummy_start_menu,
    dummy_add_menu,
    dummy_end_menu,
    dummy_select_menu,
    genl_message_menu,		/* no need for X-specific handling */
    dummy_update_inventory,
    dummy_mark_synch,
    dummy_wait_synch,
#ifdef CLIPPING
    dummy_cliparound,
#endif
#ifdef POSITIONBAR
    donull,
#endif
    dummy_print_glyph,
    dummy_raw_print,
    dummy_raw_print_bold,
    dummy_nhgetch,
    dummy_nh_poskey,
    dummy_nhbell,
    dummy_doprev_message,
    dummy_yn_function,
    dummy_getlin,
    dummy_get_ext_cmd,
    dummy_number_pad,
    dummy_delay_output,
#ifdef CHANGE_COLOR	/* only a Mac option currently */
    donull,
    donull,
#endif
    /* other defs that really should go away (they're tty specific) */
    dummy_start_screen,
    dummy_end_screen,
    dummy_outrip,
    genl_preference_update,
};

/*
init_nhwindows(int* argcp, char** argv)
                -- Initialize the windows used by NetHack.  This can also
                   create the standard windows listed at the top, but does
                   not display them.
                -- Any commandline arguments relevant to the windowport
                   should be interpreted, and *argcp and *argv should
                   be changed to remove those arguments.
                -- When the message window is created, the variable
                   iflags.window_inited needs to be set to TRUE.  Otherwise
                   all plines() will be done via raw_print().
                ** Why not have init_nhwindows() create all of the "standard"
                ** windows?  Or at least all but WIN_INFO?      -dean
*/
void dummy_init_nhwindows(int* argc, char** argv)
{
	printf("dummy_init_nhwindows\n");
}


/* Do a window-port specific player type selection. If player_selection()
   offers a Quit option, it is its responsibility to clean up and terminate
   the process. You need to fill in pl_character[0].
*/
void
dummy_player_selection()
{
#if 0
    int n, i, sel;
    const char** choices;
    int* pickmap;

    /* prevent an unnecessary prompt */
    rigid_role_checks();

    if (!flags.randomall && flags.initrole < 0) {

	/* select a role */
	for (n = 0; roles[n].name.m; n++) continue;
	choices = (const char **)alloc(sizeof(char *) * (n+1));
	pickmap = (int*)alloc(sizeof(int) * (n+1));
	for (;;) {
	    for (n = 0, i = 0; roles[i].name.m; i++) {
		if (ok_role(i, flags.initrace,
			    flags.initgend, flags.initalign)) {
		    if (flags.initgend >= 0 && flags.female && roles[i].name.f)
			choices[n] = roles[i].name.f;
		    else
			choices[n] = roles[i].name.m;
		    pickmap[n++] = i;
		}
	    }
	    if (n > 0) break;
	    else if (flags.initalign >= 0) flags.initalign = -1;    /* reset */
	    else if (flags.initgend >= 0) flags.initgend = -1;
	    else if (flags.initrace >= 0) flags.initrace = -1;
	    else panic("no available ROLE+race+gender+alignment combinations");
	}
	choices[n] = (const char *) 0;
	if (n > 1)
	    sel = ghack_player_sel_dialog(choices,
		_("Player selection"), _("Choose one of the following roles:"));
	else sel = 0;
	if (sel >= 0) sel = pickmap[sel];
	else if (sel == ROLE_NONE) {		/* Quit */
	    clearlocks();
	    dummy_exit_nhwindows(0);
	}
	free(choices);
	free(pickmap);
    } else if (flags.initrole < 0) sel = ROLE_RANDOM;
    else sel = flags.initrole;
  
    if (sel == ROLE_RANDOM) {	/* Random role */
	sel = pick_role(flags.initrace, flags.initgend,
			  flags.initalign, PICK_RANDOM);
	if (sel < 0) sel = randrole();
    }

    flags.initrole = sel;

    /* Select a race, if necessary */
    /* force compatibility with role, try for compatibility with
     * pre-selected gender/alignment */
    if (flags.initrace < 0 || !validrace(flags.initrole, flags.initrace)) {
	if (flags.initrace == ROLE_RANDOM || flags.randomall) {
	    flags.initrace = pick_race(flags.initrole, flags.initgend,
				       flags.initalign, PICK_RANDOM);
	    if (flags.initrace < 0) flags.initrace = randrace(flags.initrole);
	} else {
	    /* Count the number of valid races */
	    n = 0;	/* number valid */
	    for (i = 0; races[i].noun; i++) {
		if (ok_race(flags.initrole, i, flags.initgend, flags.initalign))
		    n++;
	    }
	    if (n == 0) {
		for (i = 0; races[i].noun; i++) {
		    if (validrace(flags.initrole, i)) n++;
		}
	    }

	    choices = (const char **)alloc(sizeof(char *) * (n+1));
	    pickmap = (int*)alloc(sizeof(int) * (n + 1));
	    for (n = 0, i = 0; races[i].noun; i++) {
		if (ok_race(flags.initrole, i, flags.initgend,
			    flags.initalign)) {
		    choices[n] = races[i].noun;
		    pickmap[n++] = i;
		}
	    }
	    choices[n] = (const char *) 0;
	    /* Permit the user to pick, if there is more than one */
	    if (n > 1)
		sel = ghack_player_sel_dialog(choices, _("Race selection"),
			_("Choose one of the following races:"));
	    else sel = 0;
	    if (sel >= 0) sel = pickmap[sel];
	    else if (sel == ROLE_NONE) { /* Quit */
		clearlocks();
		dummy_exit_nhwindows(0);
	    }
	    flags.initrace = sel;
	    free(choices);
	    free(pickmap);
	}
	if (flags.initrace == ROLE_RANDOM) {	/* Random role */
	    sel = pick_race(flags.initrole, flags.initgend,
			    flags.initalign, PICK_RANDOM);
	    if (sel < 0) sel = randrace(flags.initrole);
	    flags.initrace = sel;
	}
    }

    /* Select a gender, if necessary */
    /* force compatibility with role/race, try for compatibility with
     * pre-selected alignment */
    if (flags.initgend < 0 ||
	!validgend(flags.initrole, flags.initrace, flags.initgend)) {
	if (flags.initgend == ROLE_RANDOM || flags.randomall) {
	    flags.initgend = pick_gend(flags.initrole, flags.initrace,
				       flags.initalign, PICK_RANDOM);
	    if (flags.initgend < 0)
		flags.initgend = randgend(flags.initrole, flags.initrace);
	} else {
	    /* Count the number of valid genders */
	    n = 0;	/* number valid */
	    for (i = 0; i < ROLE_GENDERS; i++) {
		if (ok_gend(flags.initrole, flags.initrace, i, flags.initalign))
		    n++;
	    }
	    if (n == 0) {
		for (i = 0; i < ROLE_GENDERS; i++) {
		    if (validgend(flags.initrole, flags.initrace, i)) n++;
		}
	    }

	    choices = (const char **)alloc(sizeof(char *) * (n+1));
	    pickmap = (int*)alloc(sizeof(int) * (n + 1));
	    for (n = 0, i = 0; i < ROLE_GENDERS; i++) {
		if (ok_gend(flags.initrole, flags.initrace, i,
				flags.initalign)) {
		    choices[n] = genders[i].adj;
		    pickmap[n++] = i;
		}
	    }
	    choices[n] = (const char *) 0;
	    /* Permit the user to pick, if there is more than one */
	    if (n > 1)
		sel = ghack_player_sel_dialog(choices, _("Gender selection"),
			_("Choose one of the following genders:"));
	    else sel = 0;
	    if (sel >= 0) sel = pickmap[sel];
	    else if (sel == ROLE_NONE) { /* Quit */
		clearlocks();
		dummy_exit_nhwindows(0);
	    }
	    flags.initgend = sel;
	    free(choices);
	    free(pickmap);
	}
	if (flags.initgend == ROLE_RANDOM) {	/* Random gender */
	    sel = pick_gend(flags.initrole, flags.initrace,
			    flags.initalign, PICK_RANDOM);
	    if (sel < 0) sel = randgend(flags.initrole, flags.initrace);
	    flags.initgend = sel;
	}
    }

    /* Select an alignment, if necessary */
    /* force compatibility with role/race/gender */
    if (flags.initalign < 0 ||
	!validalign(flags.initrole, flags.initrace, flags.initalign)) {
	if (flags.initalign == ROLE_RANDOM || flags.randomall) {
	    flags.initalign = pick_align(flags.initrole, flags.initrace,
					 flags.initgend, PICK_RANDOM);
	    if (flags.initalign < 0)
		flags.initalign = randalign(flags.initrole, flags.initrace);
	} else {
	    /* Count the number of valid alignments */
	    n = 0;	/* number valid */
	    for (i = 0; i < ROLE_ALIGNS; i++) {
		if (ok_align(flags.initrole, flags.initrace, flags.initgend, i))
		    n++;
	    }
	    if (n == 0) {
		for (i = 0; i < ROLE_ALIGNS; i++)
		    if (validalign(flags.initrole, flags.initrace, i)) n++;
	    }

	    choices = (const char **)alloc(sizeof(char *) * (n+1));
	    pickmap = (int*)alloc(sizeof(int) * (n + 1));
	    for (n = 0, i = 0; i < ROLE_ALIGNS; i++) {
		if (ok_align(flags.initrole,
			     flags.initrace, flags.initgend, i)) {
		    choices[n] = aligns[i].adj;
		    pickmap[n++] = i;
		}
	    }
	    choices[n] = (const char *) 0;
	    /* Permit the user to pick, if there is more than one */
	    if (n > 1)
		sel = ghack_player_sel_dialog(choices, _("Alignment selection"),
			_("Choose one of the following alignments:"));
	    else sel = 0;
	    if (sel >= 0) sel = pickmap[sel];
	    else if (sel == ROLE_NONE) { /* Quit */
		clearlocks();
		dummy_exit_nhwindows(0);
	    }
	    flags.initalign = sel;
	    free(choices);
	    free(pickmap);
	}
	if (flags.initalign == ROLE_RANDOM) {
	    sel = pick_align(flags.initrole, flags.initrace,
			     flags.initgend, PICK_RANDOM);
	    if (sel < 0) sel = randalign(flags.initrole, flags.initrace);
	    flags.initalign = sel;
	}
    }
#endif
}


/* Ask the user for a player name. */
void dummy_askname()
{
	fprintf(stdout, "What is your name? ");
	char *ret = fgets(plname, sizeof(plname), stdin);
	/* Quit if they want to quit... */
	if (ret==NULL) {
		dummy_exit_nhwindows(0);
	}
}


/* Does window event processing (e.g. exposure events).
   A noop for the tty and X window-ports.
*/
void dummy_get_nh_event()
{
	printf("dummy_get_nh_event\n");
}

/* Exits the window system.  This should dismiss all windows,
   except the "window" used for raw_print().  str is printed if possible.
*/
void dummy_exit_nhwindows(const char *str)
{
	printf("dummy_exit_nhwindows(%s)\n", str);
}

/* Prepare the window to be suspended. */
void dummy_suspend_nhwindows(const char *str)
{
	printf("dummy_suspend_nhwindows(%s)\n", str);
	/* I don't think we need to do anything here... */
	return;
}


/* Restore the windows after being suspended. */
void dummy_resume_nhwindows()
{
	/* Do Nothing. */
	printf("dummy_resume_nhwindows\n");
}

static const char*
winid2str(int type)
{
	switch(type) {
		case NHW_MESSAGE: return "NHW_MESSAGE";
		case NHW_STATUS: return "NHW_STATUS";
		case NHW_MAP: return "NHW_MAP";
		case NHW_MENU: return "NHW_MENU";
		case NHW_TEXT: return "NHW_TEXT";
		default: return "unknown win type";
	}
}


/*  Create a window of type "type" which can be 
        NHW_MESSAGE     (top line)
        NHW_STATUS      (bottom lines)
        NHW_MAP         (main dungeon)
        NHW_MENU        (inventory or other "corner" windows)
        NHW_TEXT        (help/text, full screen paged window)
*/
winid 
dummy_create_nhwindow(int type)
{
	printf("dummy_create_nhwindow(%s)\n", winid2str(type));
	return type;
}

/* Clear the given window, when asked to. */
void dummy_clear_nhwindow(winid wid)
{
	printf("dummy_clear_nhwindow(%s)\n", winid2str(wid));
}

/* -- Display the window on the screen.  If there is data
                   pending for output in that window, it should be sent.
                   If blocking is TRUE, display_nhwindow() will not
                   return until the data has been displayed on the screen,
                   and acknowledged by the user where appropriate.
                -- All calls are blocking in the tty window-port.
                -- Calling display_nhwindow(WIN_MESSAGE,???) will do a
                   --more--, if necessary, in the tty window-port.
*/
void dummy_display_nhwindow(winid wid, BOOLEAN_P block)
{
	printf("dummy_display_nhwindow(%s, %d)\n", winid2str(wid), block);
}


/* Destroy will dismiss the window if the window has not 
 * already been dismissed.
*/
void dummy_destroy_nhwindow(winid wid)
{
	printf("dummy_destroy_nhwindow(%s)\n", winid2str(wid));
}

/* Next output to window will start at (x,y), also moves
 displayable cursor to (x,y).  For backward compatibility,
 1 <= x < cols, 0 <= y < rows, where cols and rows are
 the size of window.
*/
void dummy_curs(winid wid, int x, int y)
{
	printf("winid %s, x %d, y %d\n", winid2str(wid), x, y);
}

/*
putstr(window, attr, str)
                -- Print str on the window with the given attribute.  Only
                   printable ASCII characters (040-0126) must be supported.
                   Multiple putstr()s are output on separate lines.
Attributes
                   can be one of
                        ATR_NONE (or 0)
                        ATR_ULINE
                        ATR_BOLD
                        ATR_BLINK
                        ATR_INVERSE
                   If a window-port does not support all of these, it may map
                   unsupported attributes to a supported one (e.g. map them
                   all to ATR_INVERSE).  putstr() may compress spaces out of
                   str, break str, or truncate str, if necessary for the
                   display.  Where putstr() breaks a line, it has to clear
                   to end-of-line.
                -- putstr should be implemented such that if two putstr()s
                   are done consecutively the user will see the first and
                   then the second.  In the tty port, pline() achieves this
                   by calling more() or displaying both on the same line.
*/
void dummy_putstr(winid wid, int attr, const char *text)
{
	printf("winid %s, %s, attr %d\n", winid2str(wid), text, attr);
}

/* Display the file named str.  Complain about missing files
                   iff complain is TRUE.
*/
void dummy_display_file(const char *filename,BOOLEAN_P must_exist)
{
	printf("dummy_display_file(%s, %d)\n", filename, must_exist);
	dlb *f;

	f = dlb_fopen(filename, "r");
	if (!f) {
		if (must_exist) {
			printf("Warning! Could not find file: %s\n",filename);
		}
	} else {
#define LLEN 128
		char line[LLEN];
		int num_lines, charcount;

		num_lines = 0;
		charcount = 1;
		while (dlb_fgets(line, LLEN, f)) {
			printf("%s", line);
		}
		(void) dlb_fclose(f);
	}
}

/* Start using window as a menu.  You must call start_menu()
   before add_menu().  After calling start_menu() you may not
   putstr() to the window.  Only windows of type NHW_MENU may
   be used for menus.
*/
void dummy_start_menu(winid wid)
{
	/* Do Nothing */
	printf("dummy_start_menu(%s)\n", winid2str(wid));
}

/*
add_menu(windid window, int glyph, const anything identifier,
                                char accelerator, char groupacc,
                                int attr, char *str, boolean preselected)
                -- Add a text line str to the given menu window.  If identifier
                   is 0, then the line cannot be selected (e.g. a title).
                   Otherwise, identifier is the value returned if the line is
                   selected.  Accelerator is a keyboard key that can be used
                   to select the line.  If the accelerator of a selectable
                   item is 0, the window system is free to select its own
                   accelerator.  It is up to the window-port to make the
                   accelerator visible to the user (e.g. put "a - " in front
                   of str).  The value attr is the same as in putstr().
                   Glyph is an optional glyph to accompany the line.  If
                   window port cannot or does not want to display it, this
                   is OK.  If there is no glyph applicable, then this
                   value will be NO_GLYPH.
                -- All accelerators should be in the range [A-Za-z].
                -- It is expected that callers do not mix accelerator
                   choices.  Either all selectable items have an accelerator
                   or let the window system pick them.  Don't do both.
                -- Groupacc is a group accelerator.  It may be any character
                   outside of the standard accelerator (see above) or a
                   number.  If 0, the item is unaffected by any group
                   accelerator.  If this accelerator conflicts with
                   the menu command (or their user defined alises), it loses.
                   The menu commands and aliases take care not to interfere
                   with the default object class symbols.
                -- If you want this choice to be preselected when the
                   menu is displayed, set preselected to TRUE.
*/
void dummy_add_menu(winid wid, int glyph, int cnt, const ANY_P * identifier,
		CHAR_P accelerator, CHAR_P group_accel, int attr, 
		const char *str, BOOLEAN_P presel)
{
	/* Do Nothing */
	printf("dummy_add_menu(%s, %d, %d, %d, %c, %c, %d, %s, %d\n", winid2str(wid),
	       glyph, cnt, (int)identifier, accelerator, group_accel, attr, str, presel);
}

/*
end_menu(window, prompt)
                -- Stop adding entries to the menu and flushes the window
                   to the screen (brings to front?).  Prompt is a prompt
                   to give the user.  If prompt is NULL, no prompt will
                   be printed.
                ** This probably shouldn't flush the window any more (if
                ** it ever did).  That should be select_menu's job.  -dean
*/
void dummy_end_menu(winid wid, const char *prompt)
{
	/* Do Nothing */
	printf("dummy_end_menu(%s, %s)\n", winid2str(wid), prompt);
}

/*
int select_menu(windid window, int how, menu_item **selected)
                -- Return the number of items selected; 0 if none were chosen,
                   -1 when explicitly cancelled.  If items were selected, then
                   selected is filled in with an allocated array of menu_item
                   structures, one for each selected line.  The caller must
                   free this array when done with it.  The "count" field
                   of selected is a user supplied count.  If the user did
                   not supply a count, then the count field is filled with
                   -1 (meaning all).  A count of zero is equivalent to not
                   being selected and should not be in the list.  If no items
                   were selected, then selected is NULL'ed out.  How is the
                   mode of the menu.  Three valid values are PICK_NONE,
                   PICK_ONE, and PICK_N, meaning: nothing is selectable,
                   only one thing is selectable, and any number valid items
                   may selected.  If how is PICK_NONE, this function should
                   never return anything but 0 or -1.
                -- You may call select_menu() on a window multiple times --
                   the menu is saved until start_menu() or destroy_nhwindow()
                   is called on the window.
                -- Note that NHW_MENU windows need not have select_menu()
                   called for them. There is no way of knowing whether
                   select_menu() will be called for the window at
                   create_nhwindow() time.
*/
int dummy_select_menu(winid wid, int how, MENU_ITEM_P **selected)
{
	int nReturned = -1;
	printf("dummy_select_menu(%s, %d, ...)\n", winid2str(wid), how);

	/*if (wid != -1 && dummy_windowlist[wid].win != NULL &&
			dummy_windowlist[wid].type == NHW_MENU)
	{
		nReturned=ghack_menu_window_select_menu (dummy_windowlist[wid].win,
				selected, how);
	}*/

	return nReturned;
}

/*
    -- Indicate to the window port that the inventory has been changed.
    -- Merely calls display_inventory() for window-ports that leave the 
	window up, otherwise empty.
*/
void dummy_update_inventory()
{
	/* Do Nothing */
	printf("dummy_update_inventory\n");
}

/*
mark_synch()    -- Don't go beyond this point in I/O on any channel until
                   all channels are caught up to here.  Can be an empty call
                   for the moment
*/
void dummy_mark_synch()
{
	/* Do nothing */
	printf("dummy_mark_synch\n");
}

/*
wait_synch()    -- Wait until all pending output is complete (*flush*() for
                   streams goes here).
                -- May also deal with exposure events etc. so that the
                   display is OK when return from wait_synch().
*/
void dummy_wait_synch()
{
	/* Do nothing */
	printf("dummy_wait_synch\n");
}

/*
cliparound(x, y)-- Make sure that the user is more-or-less centered on the
                   screen if the playing area is larger than the screen.
                -- This function is only defined if CLIPPING is defined.
*/
void dummy_cliparound(int x, int y)
{
	/* Do Nothing */
	printf("dummy_cliparound\n");
}

/*
print_glyph(window, x, y, glyph)
                -- Print the glyph at (x,y) on the given window.  Glyphs are
                   integers at the interface, mapped to whatever the window-
                   port wants (symbol, font, color, attributes, ...there's
                   a 1-1 map between glyphs and distinct things on the map).
*/
void dummy_print_glyph(winid wid,XCHAR_P x,XCHAR_P y,int glyph)
{
	if (wid != -1) {
		printf("winid %d; x %d, y %d, glyph: %d\n",
		       wid, x, y, glyph);
	}
}

/*
raw_print(str)  -- Print directly to a screen, or otherwise guarantee that
                   the user sees str.  raw_print() appends a newline to str.
                   It need not recognize ASCII control characters.  This is
                   used during startup (before windowing system initialization
                   -- maybe this means only error startup messages are raw),
                   for error messages, and maybe other "msg" uses.  E.g.
                   updating status for micros (i.e, "saving").
*/
void dummy_raw_print(const char *str)
{
	printf("dummy_raw_print\n%s\n", str);
}

/*
raw_print_bold(str)
                -- Like raw_print(), but prints in bold/standout (if
possible).
*/
void dummy_raw_print_bold(const char *str)
{
	printf("dummy_raw_print_bold\n*%s*\n", str);
}

static int dummy_getchar()
{
	char input[256];
	char *rets = NULL;
	do {
		rets = fgets(input, sizeof(input), stdin);
	} while (rets == NULL);

	return input[0];
}

/*
int nhgetch()   -- Returns a single character input from the user.
                -- In the tty window-port, nhgetch() assumes that tgetch()
                   will be the routine the OS provides to read a character.
                   Returned character _must_ be non-zero.
*/
int dummy_nhgetch()
{
	printf("dummy_nhgetch\n");
	int ret = dummy_getchar();
	printf("dummy_nhgetch %d %c\n",ret,ret);
	return ret;
}

/*
int nh_poskey(int *x, int *y, int *mod)
                -- Returns a single character input from the user or a
                   a positioning event (perhaps from a mouse).  If the
                   return value is non-zero, a character was typed, else,
                   a position in the MAP window is returned in x, y and mod.
                   mod may be one of

                        CLICK_1         -- mouse click type 1 
                        CLICK_2         -- mouse click type 2 

                   The different click types can map to whatever the
                   hardware supports.  If no mouse is supported, this
                   routine always returns a non-zero character.
*/
int dummy_nh_poskey(int *x, int *y, int *mod)
{
#if 0
    gtk_signal_emit (GTK_OBJECT (dummy_windowlist[WIN_STATUS].win),
		       ghack_signals[GHSIG_FADE_HIGHLIGHT]);
    
    g_askingQuestion = 0;
    /* Process events until a key or map-click arrives. */
    while ( g_numKeys == 0 && g_numClicks == 0 )
	gtk_main_iteration();
    
    if (g_numKeys > 0) {
	int key;
	GList *theFirst;
	
	theFirst = g_list_first( g_keyBuffer);
	g_keyBuffer = g_list_remove_link(g_keyBuffer, theFirst);
	key = GPOINTER_TO_INT( theFirst->data);
	g_list_free_1( theFirst);
	g_numKeys--;
	return ( key);
    }
    else {
	GHClick *click;
	GList *theFirst;
	
	theFirst = g_list_first( g_clickBuffer);
	g_clickBuffer = g_list_remove_link(g_clickBuffer, theFirst);
	click = (GHClick*) theFirst->data;
	*x=click->x;
        *y=click->y;
        *mod=click->mod;
	g_free( click);
	g_list_free_1( theFirst);
	g_numClicks--;
	return ( 0);
    }
#endif
	printf("dummy_nh_poskey\n");
	return dummy_getchar();
}

/*
nhbell()        -- Beep at user.  [This will exist at least until sounds are
                   redone, since sounds aren't attributable to windows anyway.]
*/
void dummy_nhbell()
{
	printf("BEEP!\n");
}

/*
doprev_message()
                -- Display previous messages.  Used by the ^P command.
                -- On the tty-port this scrolls WIN_MESSAGE back one line.
*/
int dummy_doprev_message()
{
	/* Do Nothing.  They can read old messages using the scrollbar. */
	return 0;
}

/*
char yn_function(const char *ques, const char *choices, char default)
                -- Print a prompt made up of ques, choices and default.
                   Read a single character response that is contained in
                   choices or default.  If choices is NULL, all possible
                   inputs are accepted and returned.  This overrides
                   everything else.  The choices are expected to be in
                   lower case.  Entering ESC always maps to 'q', or 'n',
                   in that order, if present in choices, otherwise it maps
                   to default.  Entering any other quit character (SPACE,
                   RETURN, NEWLINE) maps to default.
                -- If the choices string contains ESC, then anything after
                   it is an acceptable response, but the ESC and whatever
                   follows is not included in the prompt.
                -- If the choices string contains a '#' then accept a count.
                   Place this value in the global "yn_number" and return '#'.
                -- This uses the top line in the tty window-port, other
                   ports might use a popup.
*/
char dummy_yn_function(const char *question, const char *choices,
		CHAR_P def)
{
	// TODO
	printf("dummy_yn_function %s\n", question);
	int ret = dummy_getchar();
	printf("dummy_yn_function %d %c\n", ret, ret);
	return ret;
}

/** Strips newline from end of string. */
void strip_newline(char *str) {
	/* Strip newline from end */
	if (str[strlen(str)-1] == '\n') {
		str[strlen(str)-1] = '\0';
	}
}

/*
getlin(const char *ques, char *input)
	    -- Prints ques as a prompt and reads a single line of text,
	       up to a newline.  The string entered is returned without the
	       newline.  ESC is used to cancel, in which case the string
	       "\033\000" is returned.
	    -- getlin() must call flush_screen(1) before doing anything.
	    -- This uses the top line in the tty window-port, other
	       ports might use a popup.
*/
void dummy_getlin(const char *question, char *input)
{
	printf("dummy_getlin\n");
	fprintf(stdout, "%s:\n", question);
	fflush(stdout);
	char *ret = fgets(input, 256, stdin);

	if (ret == NULL) {
		input[0] = '\033';
		input[1] = '\0';
	} else {
		strip_newline(input);
	}

}

/*
int get_ext_cmd(void)
	    -- Get an extended command in a window-port specific way.
	       An index into extcmdlist[] is returned on a successful
	       selection, -1 otherwise.
*/
int dummy_get_ext_cmd()
{
	char cmd[255];
	int i;
	char *ret;

	printf("dummy_get_ext_cmd\n");
	ret = fgets(cmd, sizeof(cmd), stdin);

	for (i = 0; extcmdlist[i].ef_txt != (char *)0; i++) {
		strip_newline(ret);
		if (!strcmpi(ret, extcmdlist[i].ef_txt)) {
			return i;
		}
	}
	return -1;
}


/*
number_pad(state)
	    -- Initialize the number pad to the given state.
*/
void dummy_number_pad(int state)
{
	/* Do Nothing */
	printf("dummy_number_pad\n");
}

/*
delay_output()  -- Causes a visible delay of 50ms in the output.
	       Conceptually, this is similar to wait_synch() followed
	       by a nap(50ms), but allows asynchronous operation.
*/
void dummy_delay_output()
{
	// TODO
	/* Do Nothing */
	printf("dummy_delay_output\n");
}

/*
start_screen()  -- Only used on Unix tty ports, but must be declared for
	       completeness.  Sets up the tty to work in full-screen
	       graphics mode.  Look at win/tty/termcap.c for an
	       example.  If your window-port does not need this function
	       just declare an empty function.
*/
void dummy_start_screen()
{
	/* Do Nothing */
	printf("dummy_start_screen\n");
}

/*
end_screen()    -- Only used on Unix tty ports, but must be declared for
	       completeness.  The complement of start_screen().
*/
void dummy_end_screen()
{
	/* Do Nothing */
	printf("dummy_end_screen\n");
}

/*
outrip(winid, int)
	    -- The tombstone code.  If you want the traditional code use
	       genl_outrip for the value and check the #if in rip.c.
*/
void dummy_outrip(winid wid, int how)
{
	printf("dummy_outrip\n");
}
