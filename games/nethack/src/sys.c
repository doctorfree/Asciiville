/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

struct sysopt sysopt;

void
sys_early_init()
{
#ifdef LIVELOGFILE
    sysopt.livelog = LIVELOG_DETAIL;
    sysopt.ll_conduct_turns = 0;
#endif
}

void
sysopt_release()
{
    if (sysopt.support) {
        free(sysopt.support), sysopt.support = (char *) 0;
    }
}

/*sys.c*/
