#ifndef WINLISP_H
#define WINLISP_H

#ifndef E
#define E extern
#endif

#if defined(BOS) || defined(NHSTDC)
#define DIMENSION_P int
#else
# ifdef WIDENED_PROTOTYPES
#define DIMENSION_P unsigned int
# else
#define DIMENSION_P Dimension
# endif
#endif

extern struct window_procs tty_procs;

/* ### winlisp.c ### */
E void NDECL(win_lisp_init);
E void FDECL(lisp_init_nhwindows, (int *, char **));
E void NDECL(lisp_player_selection);
E void NDECL(lisp_askname);
E void NDECL(lisp_get_nh_event) ;
E void FDECL(lisp_exit_nhwindows, (const char *));
E void FDECL(lisp_suspend_nhwindows, (const char *));
E void NDECL(lisp_resume_nhwindows);
E winid FDECL(lisp_create_nhwindow, (int));
E void FDECL(lisp_clear_nhwindow, (winid));
E void FDECL(lisp_display_nhwindow, (winid, BOOLEAN_P));
E void FDECL(lisp_destroy_nhwindow, (winid));
E void FDECL(lisp_curs, (winid,int,int));
E void FDECL(lisp_putstr, (winid, int, const char *));
#ifdef FILE_AREAS
E void FDECL(lisp_display_file, (const char *, const char *, BOOLEAN_P));
#else
E void FDECL(lisp_display_file, (const char *, BOOLEAN_P));
#endif
E void FDECL(lisp_start_menu, (winid));
E void FDECL(lisp_add_menu, (winid, int, int, const ANY_P *,
			     CHAR_P, CHAR_P, int, const char *, BOOLEAN_P));
E void FDECL(lisp_end_menu, (winid, const char *));
E int FDECL(lisp_select_menu, (winid, int, MENU_ITEM_P **));
E char FDECL(lisp_message_menu, (char, int, const char *mesg));
E void NDECL(lisp_update_inventory);
E void NDECL(lisp_mark_synch);
E void NDECL(lisp_wait_synch);
#ifdef CLIPPING
E void FDECL(lisp_cliparound, (int, int));
#endif
#ifdef POSITIONBAR
E void FDECL(lisp_update_positionbar, (char *));
#endif
E void FDECL(lisp_print_glyph, (winid,XCHAR_P,XCHAR_P,int));
E void FDECL(lisp_raw_print, (const char *));
E void FDECL(lisp_raw_print_bold, (const char *));
E int NDECL(lisp_nhgetch);
E int FDECL(lisp_nh_poskey, (int *, int *, int *));
E void NDECL(lisp_nhbell);
E int NDECL(lisp_doprev_message);
E char FDECL(lisp_yn_function, (const char *, const char *, CHAR_P));
E void FDECL(lisp_getlin, (const char *,char *));
E int NDECL(lisp_get_ext_cmd);
E void FDECL(lisp_number_pad, (int));
E void NDECL(lisp_delay_output);

/* other defs that really should go away (they're tty specific) */
E void NDECL(lisp_start_screen);
E void NDECL(lisp_end_screen);

E void FDECL(lisp_outrip, (winid,int));

#endif /* WINLISP_H */
