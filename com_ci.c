/*
 * com_ci(), check in a spice raw file  
 *
 * $Log: com_ci.c,v $
 * Revision 1.1  2007/05/17 06:05:54  walker
 * Initial revision
 *
 * Revision 1.1  2005/08/03 15:56:17  walker
 * Initial revision
 *
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> 	/* for getopt() */
#include <stdlib.h>	/* for exit() */

// #include "ss_wavefile.h"
#include "post.h"
#include "y.tab.h"
#include "newread.h"

int com_ci(char *rawfile)
{
    sp_read(rawfile);
    return(0);
}
