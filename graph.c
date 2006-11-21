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
    int count=0;
    double max,min,range, scale;
    char name[128];
    char *pn;

    plottime = time(NULL);
    strftime(buf, MAXBUF, "%m/%d/%y-%H:%M:%S", localtime(&plottime));
    if ((pn=rawfile_name()) == NULL) {
	sprintf(title, "title <stdin> - %s\n", buf);
    } else {
	sprintf(title, "title %s - %s\n", pn, buf);
    }

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

    count=0;
    for (i=0; i<num; i++) {
	p = &(plottab[i]);
	if (p->datum != NULL) {
	    if (count == 0) {
		max=min=p->datum->iv;
	    }
	    count++;
	    for (pd=p->datum; pd!=NULL; pd=pd->next) {
		if (max<pd->iv) max=pd->iv;
		if (min>pd->iv) min=pd->iv;
	        sprintf(buf, "%g %g", pd->iv, pd->re);
	        scriptfeed(buf);
	    }
	    sprintf(buf, "label -12%% %d%% %s", (int) (100.0-count*10.0), p->datum->def);
	    scriptfeed(buf);
	}
    }
    if (max-min != 0.0) {
	range = log(max-min)/log(10.0);
    	if (range < -12) {
	   scale = 1e-15;
	   strcpy(name, "femto");
	} else if (range < -9.0) {
	   scale = 1e-12;
	   strcpy(name, "pico");
	} else if (range < -6.0) {
	   scale = 1e-9;
	   strcpy(name, "nano");
	} else if (range < -3.0) {
	   scale = 1e-6;
	   strcpy(name, "micro");
	} else if (range < 0) {
	   scale = 1e-3;
	   strcpy(name, "milli");
	} else if (range < 3.0) {
	   scale = 1.0;
	   strcpy(name, "");
	} else if (range < 6.0) {
	   scale = 1e3;
	   strcpy(name, "kilo");
	} else if (range < 9.0) {
	   scale = 1e6;
	   strcpy(name, "mega");
	} else if (range < 12.0) {
	   scale = 1e9;
	   strcpy(name, "giga");
	} else if (range < 15.0) {
	   scale = 1e12;
	   strcpy(name, "tera");
	} else {
	   scale = 1.0;
	   strcpy(name, "");
	} 

	sprintf(buf, "xscale %g %sseconds", scale, name);
	/* printf("%s: range:%g, max/min=%g/%g\n", buf, range, max,min); */
	scriptfeed(buf);
    }

    count=0;
    for (i=0; i<num; i++) {
	p = &(plottab[i]);
	if (p->xlmin != p->xlmax) {
	  sprintf(buf, "xset %g %g", p->xlmin, p->xlmax);
	  scriptfeed(buf);
	}
	if (p->ylmin != p->ylmax) {
	  sprintf(buf, "yset %g %g", p->ylmin, p->ylmax);
	  scriptfeed(buf);
	}
	if (p->newgraph) {
	  count=0;
	  sprintf(buf, "nextygraph");
	  scriptfeed(buf);
	}
	if (p->datum != NULL) {
	    if (count == 0) {
		max=min=p->datum->iv;
	    }
	    count++;
	    for (pd=p->datum; pd!=NULL; pd=pd->next) {
		if (max<pd->iv) max=pd->iv;
		if (min>pd->iv) min=pd->iv;
	        sprintf(buf, "%g %g\n", pd->iv, pd->re);
	        scriptfeed(buf);
	    }
	    sprintf(buf, "label -12%% %d%% %s", (int) (100.0-count*10.0), p->datum->def);
	    scriptfeed(buf);
	}
    }
    scriptclose();
}


