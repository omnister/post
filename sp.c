/*
 * test routine for WaveFile data file readers
 *
 * $Log: sp.c,v $
 * Revision 1.1  2007/05/17 06:05:54  walker
 * Initial revision
 *
 * Revision 1.1  2005/08/03 15:56:17  walker
 * Initial revision
 *
 * Revision 1.5  2003/07/30 06:18:49  sgt
 * better handling of the last point in a wavevar,
 * in particular when wv_interp_val asks for a point beyond 
 * the end of the iv range enhance test_read.c to
 *
 * Revision 1.4  2000/08/09 23:37:39  sgt
 * ss_hspice.c - wrote sf_guessrows_hsbin routine.
 * others - instrumented to count reallocs and print out the number.
 *
 * Revision 1.3  2000/01/07 05:04:48  tell
 * updating with old changes as we construct the CVS
 *
 * Revision 1.2  1998/09/17 18:25:09  tell
 * prints out variable type
 *
 * Revision 1.1  1998/08/31 21:00:28  tell
 * Initial revision
 *
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> 	/* for getopt() */
#include <stdlib.h>	/* for exit() */

#include "ss_wavefile.h"

void test_interp(WaveFile *wf, double mytm);

int
main(int argc, char **argv)
{
	WaveFile *wf;
	int i, j;
	extern int optind;
	extern char *optarg;
	int i_flag = 0;
	int v_flag = 0;
	int l_flag = 0;
	int r_flag = 0;
	int errflg = 0;
	char *filetype = NULL;
	int c;

	while ((c = getopt (argc, argv, "ilrt:v")) != EOF) {
		switch(c) {
		case 'i':
			i_flag = 1;
			break;
		case 'v':
			v_flag = 1;
			break;
		case 'l':
			l_flag = 1;
			break;
		case 'r':
			r_flag = 1;
			break;
		case 't':
			filetype = optarg;
			break;
		default:
			errflg = 1;
			break;
		}
	}

	if(errflg || optind >= argc)  {
	    fprintf(stderr, "usage: %s [-ltvx] file\n", argv[0]);
	    fprintf(stderr, "     [-i] ; information listing \n");
	    fprintf(stderr, "     [-t <filetype>] ; override filetype autosense\n");
	    fprintf(stderr, "     [-l] ; list all waveform values \n");
	    fprintf(stderr, "     [-v] ; verbose debugging info\n");
	    exit(1);
	}

	if (r_flag) {
	   printf("got r_flag\n");
	}
	
	spicestream_msg_level = ERR;
	wf = wf_read(argv[optind], filetype);
	if(!wf) {
	    if(errno) {
		perror(argv[1]);
	    }
	    fprintf(stderr, "test_read: unable to read file\n");
	    exit(1);
	}

	if (i_flag) {
	    printf("filename: \"%s\"\n", wf->wf_filename);
	    printf("independent variable:\n");
	    printf("  name: \"%s\"\n", wf->iv->wv_name);
	    printf("  type: %s\n", vartype_name_str(wf->iv->wv_type));
	    printf("  npts: %d\n", wf->nvalues);
	    printf("  min: %g\n", wf->iv->wds->min);
	    printf("  max: %g\n", wf->iv->wds->max);
	    printf("  blocks: %d/%d\n", wf->iv->wds->bpused, wf->iv->wds->bpsize);
	    printf("  reallocs: %d\n", wf->iv->wds->nreallocs);

	    printf("columns: %d\n", wf->wf_ncols);
	    printf("dependent variables: %d\n", wf->wf_ndv);
	    for(i = 0; i < wf->wf_ndv; i++) {
		    printf(" dv[%d] \"%s\" ", i, wf->dv[i].wv_name);
		    printf(" (type=%s)", vartype_name_str(wf->dv[i].wv_type));
		    if(wf->dv[i].wv_ncols > 1)
			    printf(" (%d columns)\n", wf->dv[i].wv_ncols);
		    for(j = 0; j < wf->dv[i].wv_ncols; j++) {
			    if(wf->dv[i].wv_ncols > 1)
				    printf("    col[%d] ", j);
			    printf("blocks=%d/%d ",
				   wf->dv[i].wds[j].bpused, wf->dv[i].wds[j].bpsize);
			    printf("min=%g ", wf->dv[i].wds[j].min);
			    printf("max=%g ", wf->dv[i].wds[j].max);
			    printf("first=%g ", wds_get_point(&wf->dv[i].wds[j], 0));
			    printf("last=%g\n", wds_get_point(&wf->dv[i].wds[j],
						     wf->nvalues-1));
		    }
	    }
	}

	if(l_flag) {

		/* print header showing all signal names */
		printf("#[xxx]    %10s", wf->iv->wv_name);		/* independent variable */
		for(i = 0; i < wf->wf_ndv; i++) {
			printf(" %10s", wf->dv[i].wv_name);		/* dependent variables */
		}
		putchar('\n');
		for(j = 0; j < wf->nvalues; j++) {
			printf("[%03d] %10g", j, wds_get_point(wf->iv->wds, j));
			for(i = 0; i < wf->wf_ndv; i++) {
			    switch (wf->dv[i].wv_ncols) {
				case 1:
				    printf(" %10g", 
					   wds_get_point(&wf->dv[i].wds[0], j));
				    break;
				case 2:
				    printf(" (%g,%g) ", 
					   wds_get_point(&wf->dv[i].wds[0], j),
					   wds_get_point(&wf->dv[i].wds[1], j));
				    break;
				default:
				    fprintf(stderr,"bad number of columns\n");
				    exit(1);
				    break;
			    }
			}
			putchar('\n');
		}
	}

	if(v_flag) {
		double mytm;
		double delta;

		mytm = wds_get_point(wf->iv->wds, 0);
		delta = (wds_get_point(wf->iv->wds, wf->nvalues-1) - mytm) / 40.0;
		printf("40 divisions, delta=%g\n", delta);
		for(i = 0; i <= 41; i++, mytm += delta) {
			test_interp(wf, mytm);
		}
		mytm = wds_get_point(wf->iv->wds, wf->nvalues-2);
		test_interp(wf, mytm);
   	}
	exit(0);
}

void
test_interp(WaveFile *wf, double mytm)
{
    int idx;
    idx = wf_find_point(wf->iv, mytm);
    printf("last %8s < %14.8g is %14.8g at [%4d];",
	   wf->iv->wv_name,
	   mytm,
	   wds_get_point(wf->iv->wds, idx),
	   idx);
    fflush(stdout);
    printf("%8s at %8s=%14.8g is %14.8g\n",
	   wf->dv[0].wv_name,
	   wf->iv->wv_name,
	   mytm,
	   wv_interp_value(&wf->dv[0], mytm));
}
