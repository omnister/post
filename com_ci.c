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

#include "ss_wavefile.h"
#include "post.h"
#include "y.tab.h"

char ivname[128]="";

static char *rawfilename = NULL;

char *rawfile_name() {
    return(rawfilename);
}

char *independent_varname() {
    return(ivname);
}

int com_ci(char *rawfile)
{
    WaveFile *wf;
    int i, j;
    char *filetype = NULL;
    char buf[128];
    char *t,*s;
    int c;
    double re, im,  iv;
    double re1,im1, iv1;
    double re2,im2, iv2;

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

    if (strncasecmp(wf->iv->wv_name,"frequency", 9)==0) {
       strcpy(ivname,"Hz");
    } else if (strncasecmp(wf->iv->wv_name,"time", 4)==0) {
       strcpy(ivname,"seconds");
    } else {
       strcpy(ivname,wf->iv->wv_name);
       printf("unrecognized wv_name in com_ci: %s\n", wf->iv->wv_name);
    }
    
    for (i = 0; i < wf->wf_ndv; i++) {
	result = NULL;
        for (j = 0; j < wf->nvalues; j++) {
	    switch (wf->dv[i].wv_ncols) {
	    case 1:
		re=wds_get_point(&wf->dv[i].wds[0], j);
		im=0.0;
		break;
	    case 2:
		re=wds_get_point(&wf->dv[i].wds[0], j);
		im=wds_get_point(&wf->dv[i].wds[1], j);
		break;
	    default:
		fprintf(stderr, "bad number of columns\n");
		exit(1);
		break;
	    }
	    iv = wds_get_point(wf->iv->wds, j);

	    if (j<2) {
		tmp=new_dat(re,im);
		tmp->iv = iv;
		result = link_dat(result, tmp);

	    } else if (j>2 && (!collinear(iv2,re2, iv1,re1, iv,re) || !collinear(iv2,im2, iv1,im1, iv,im))) { 

		tmp=new_dat(re1,im1);
		tmp->iv = iv1;
		result = link_dat(result, tmp);

	    }
	    iv2 = iv1; iv1 = iv;
	    re2 = re1; re1 = re;
	    im2 = im1; im1 = im;
	}
	tmp=new_dat(re1,im1);
	tmp->iv = iv1;
	result = link_dat(result, tmp);

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
