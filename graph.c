#include "post.h"
#include "script.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

typedef struct plotspec {
    int newgraph;	/* set to 1 for a new graph */
    double xlmin;	
    double xlmax;
    double ylmin;	/* cause xset command to be emitted */
    double ylmax;	/* ylmin != ylmax */
    DATUM *datum;	/* expression to be plotted */
    char   *name;	/* string for label if not NULL */ 
} PLOTSPEC;

int debug = 0;

#define MAXPLOTELEMENTS 128
#define MAXBUF 128

PLOTSPEC plottab[MAXPLOTELEMENTS];

char title[MAXBUF];

static int num;

void graphinit() {
   int i;
   PLOTSPEC *p;

   if (debug) printf("in graphinit()\n");

   for (i=0; i<MAXPLOTELEMENTS; i++) {
       p = &(plottab[i]);
       p->newgraph = 0;
       p->xlmin = 0.0;
       p->xlmax = 0.0;
       p->ylmin = 0.0;
       p->ylmax = 0.0;
       if (p->datum != NULL) {
          free_dat(p->datum);
       }
       p->datum = NULL;
       if (p->name != NULL) {
          free(p->name);
       }
       p->name = NULL;
   }

   num = 0;	
}

void graphnext() {
   if (debug) printf("in graphnext()\n");
   plottab[num++].newgraph = 1;
}

void graphexpr(DATUM *d) {
   if (debug) printf("in graphexpr()\n");
   plottab[num++].datum = dup_dat(d);
}

void graphxl(double xlmin, double xlmax) {
   PLOTSPEC *p;
   if (debug) printf("in graphxl()\n");
   p = &(plottab[num++]);
   p->xlmin = xlmin;
   p->xlmax = xlmax;
}

void graphyl(double ylmin, double ylmax) {
   PLOTSPEC *p;
   if (debug) printf("in graphyl()\n");
   p = &(plottab[num++]);
   p->ylmin = ylmin;
   p->ylmax = ylmax;
}

void graphprint(int mode) {
    PLOTSPEC *p;
    DATUM *pd;
    int i;
    char buf[128];
    time_t plottime;

    plottime = time(NULL);
    strftime(title, MAXBUF, "title %m/%d/%y-%H:%M:%S", localtime(&plottime));

    switch(mode) {
	case 0:
	    if (scriptopen("ap", NULL) == 0) { 
	       printf("can't open autoplot!\n");
	       return;
	    } 
	    break;
	case 1:
	    if (scriptopen("ap", "-n") == 0) { 
	       printf("can't open autoplot!\n");
	       return;
	    }
	    break;
	default:
	    printf("bad graph_print parameter: %d\n", mode);
	    break;
    }

    scriptfeed(title);		/* emit title */

    for (i=0; i<num; i++) {
	p = &(plottab[i]);
	if (p->xlmin != p->xlmax) {
	  sprintf(buf, "xset %g %g\n", p->xlmin, p->xlmax);
	  scriptfeed(buf);
	}
	if (p->ylmin != p->ylmax) {
	  sprintf(buf, "yset %g %g\n", p->ylmin, p->ylmax);
	  scriptfeed(buf);
	}
	if (p->newgraph) {
	  sprintf(buf, "nextygraph\n");
	  scriptfeed(buf);
	}
	if (p->datum != NULL) {
	    for (pd=p->datum; pd!=NULL; pd=pd->next) {
	       sprintf(buf, "%g %g\n", pd->iv, pd->re);
	       scriptfeed(buf);
	    }
	}
    }
    scriptclose();
}

