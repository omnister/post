/*
 * com_ci(), check in a spice raw file  
 *
 * $Log: com_ci_old.c,v $
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

#include "ss_wavefile.h"
#include "post.h"
#include "y.tab.h"

static char *rawfilename = NULL;

char *rawfile_name() {
    return(rawfilename);
}

int com_ci(char *rawfile)
{
    WaveFile *wf;
    int i, j;
    char *filetype = NULL;
    char buf[128];
    char *t,*s;
    int c;

    DATUM *tmp;
    DATUM *result;

    spicestream_msg_level = ERR;
    wf = wf_read(rawfile, filetype);
    if (!wf) {
	fprintf(stderr, "com_ci: unable to read file\n");
	return(0);
    }

    if (rawfilename != NULL) { free(rawfilename); } 
    rawfilename = strsave(rawfile);
    
    for (i = 0; i < wf->wf_ndv; i++) {
	result = NULL;
        for (j = 0; j < wf->nvalues; j++) {
	    switch (wf->dv[i].wv_ncols) {
	    case 1:
		tmp=new_dat(wds_get_point(&wf->dv[i].wds[0], j),0.0);
		break;
	    case 2:
		tmp=new_dat(
			wds_get_point(&wf->dv[i].wds[0], j),
			wds_get_point(&wf->dv[i].wds[1], j));
		break;
	    default:
		fprintf(stderr, "bad number of columns\n");
		exit(1);
		break;
	    }
	    tmp->iv = wds_get_point(wf->iv->wds, j);
	    result = link_dat(result, tmp);
	}

	s=wf->dv[i].wv_name;
	t=buf;
	while (*s!='\0') {
	    switch (c=*s++){
		case  ')':
		case  '(':
		case  '#':
		    break;
		case  '%':
		   *t++ = '_';
		    break;
		default:
		   *t++ = c;
		   break;
	    }
	}
	*t = '\0';

	install(buf, VAR, result);
    }
    wf_free(wf);
    return(0);
}
