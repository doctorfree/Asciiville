/*	SCCS Id: @(#)gnbind.h	3.4	2000/07/16	*/
/* Copyright (C) 1998 by Erik Andersen <andersee@debian.org> */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef dummy_h
#define dummy_h


/* Some prototypes */
void dummy_init_nhwindows(int* argc, char** argv);
void dummy_player_selection(void);
void dummy_askname(void);
void dummy_get_nh_event(void);
void dummy_exit_nhwindows(const char *);
void dummy_suspend_nhwindows(const char *);
void dummy_resume_nhwindows(void);
winid dummy_create_nhwindow(int type);
void dummy_create_nhwindow_by_id(int type, winid i);
void dummy_clear_nhwindow(winid wid);
void dummy_display_nhwindow(winid wid, BOOLEAN_P block);
void dummy_destroy_nhwindow(winid wid);
void dummy_curs(winid wid, int x, int y);
void dummy_putstr(winid wid, int attr, const char *text);
void dummy_display_file(const char *filename,BOOLEAN_P must_exist);
void dummy_start_menu(winid wid);
void dummy_add_menu(winid wid, int glyph, int cnt, const ANY_P * identifier,
		CHAR_P accelerator, CHAR_P group_accel, int attr, 
		const char *str, BOOLEAN_P presel);
void dummy_end_menu(winid wid, const char *prompt);
int  dummy_select_menu(winid wid, int how, MENU_ITEM_P **selected);
/* No need for message_menu -- we'll use genl_message_menu instead */	
void dummy_update_inventory(void);
void dummy_mark_synch(void);
void dummy_wait_synch(void);
void dummy_cliparound(int x, int y);
/* The following function does the right thing.  The nethack
 * dummy_cliparound (which lacks the winid) simply calls this funtion.
*/
void dummy_cliparound_proper(winid wid, int x, int y);
void dummy_print_glyph(winid wid,XCHAR_P x,XCHAR_P y,int glyph);
void dummy_raw_print(const char *str);
void dummy_raw_print_bold(const char *str);
int  dummy_nhgetch(void);
int  dummy_nh_poskey(int *x, int *y, int *mod);
void dummy_nhbell(void);
int  dummy_doprev_message(void);
char dummy_yn_function(const char *question, const char *choices,
		CHAR_P def);
void dummy_getlin(const char *question, char *input);
int  dummy_get_ext_cmd(void);
void dummy_number_pad(int state);
void dummy_delay_output(void);
void dummy_start_screen(void);
void dummy_end_screen(void);
void dummy_outrip(winid wid, int how);

#endif /* dummy_h */


