/*  SCCS Id: @(#)func_tab.h 3.4 1992/04/03  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef FUNC_TAB_H
#define FUNC_TAB_H

/* extended command flags */
#define IFBURIED          0x01 /* can do command when buried */
#define AUTOCOMPLETE      0x02 /* command autocompletes */
#define WIZMODECMD        0x04 /* wizard-mode command */
#define GENERALCMD        0x08 /* general command, does not take game time */
#define CMD_NOT_AVAILABLE 0x10 /* recognized but non-functional (!SHELL,&c) */

struct ext_func_tab {
    uchar key;
    const char *ef_txt, *ef_desc;
    int NDECL((*ef_funct));
    int flags;
    const char *f_text;
};

extern struct ext_func_tab extcmdlist[];

#endif /* FUNC_TAB_H */
