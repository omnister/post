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
#define MAXBUF 1024

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

int collinear(x1,y1, x2,y2, x3,y3) 
double x1,y1,x2,y2,x3,y3;
{
    double val;
    val = (x3-x1)*(y2-y1) - (y3-y1)*(x2-x1);

    if (val == 0.0) {
       return(1);
    } else {
       return(0);
    }
}

void graphprint(int mode) {		/* autoplot */
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
	    if (scriptopen("ap", NULL, NULL) == 0) { 
	       printf("can't open autoplot!\n");
	       return;
	    } 
	    break;
	case 1:
	    if (scriptopen("ap", "-n", NULL) == 0) { 
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
	        sprintf(buf, "%g %g\n", pd->iv, pd->re);
	        scriptfeed(buf);
	    }
	    sprintf(buf, "label -12%% %d%% %s\n", (int) (100.0-count*10.0), p->datum->def);
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

	sprintf(buf, "xscale %g %sseconds\n", scale, name);
	/* printf("%s: range:%g, max/min=%g/%g\n", buf, range, max,min); */
	scriptfeed(buf);
    }

    count=0;
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
	  count=0;
	  sprintf(buf, "nextygraph\n");
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
	    sprintf(buf, "label -12%% %d%% %s\n", (int) (100.0-count*10.0), p->datum->def);
	    scriptfeed(buf);
	}
    }
    scriptclose();
}


void graphprint_gnu(int mode) {		/* gnuplot */
    DATUM *pd;
    int i;
    char buf[128];
    time_t plottime;
    char *pn;
    double ngraphs;

    system("killall gnuplot_x11");

    plottime = time(NULL);
    strftime(buf, MAXBUF, "%m/%d/%y-%H:%M:%S", localtime(&plottime));
    if ((pn=rawfile_name()) == NULL) {
	sprintf(title, "title <stdin> - %s\n", buf);
    } else {
	sprintf(title, "title %s - %s\n", pn, buf);
    }

    if (scriptopen("gnuplot", NULL, NULL) == 0) { 
       printf("can't open gnuplot!\n");
       return;
    } 

    /*******  Print gnuplot headers *********/
    /* scriptfeed("reset\n"); */
    scriptfeed("set term x11 persist\n");
    scriptfeed("set multiplot\n");
    scriptfeed("set xtics nomirror\n");
    scriptfeed("set ytics nomirror\n");
    scriptfeed("set grid back\n");
    scriptfeed("set style data lines\n");
    /****************************************/

    double ylmin=0.0; 
    double ylmax=0.0;

    ngraphs=1.0;	/* scan through to find how many graphs, get ylmin/max */
    for (i=0; i<num; i++) {
	if (plottab[i].newgraph) {
	    ngraphs++;
	}
	if (plottab[i].ylmin != plottab[i].ylmax) {
	    ylmin = plottab[i].ylmin;
	    ylmax = plottab[i].ylmax;
	}
    }

    double xlmin=0.0; 
    double xlmax=0.0;
    double start, stop;
    double plot=0.0;

    
    start=0.0;
    stop=0.0;
    while (stop!=num) {
        plot++;

	for (i=start; (i<num && !plottab[i].newgraph); i++) {
	    if (plottab[i].xlmin != plottab[i].xlmax) {
		xlmin = plottab[i].xlmin;
		xlmax = plottab[i].xlmax;
	    }
	}
	stop=i;
	
	/* set frame size */

	sprintf(buf, "#ngraphs = %g, plot=%g\n", ngraphs, plot);
	scriptfeed(buf);

	sprintf(buf, "set size 1,%g\n", 0.9/ngraphs);
	scriptfeed(buf);
	sprintf(buf, "set origin 0.0,%g\n", 0.1+(plot-1.0)*(0.9/ngraphs));
	scriptfeed(buf);

	/* create plot header, ranges, names */

	scriptfeed("plot ");
	if (xlmin != xlmax) {
	    sprintf(buf, "[%g:%g] ", xlmin, xlmax);
	    scriptfeed(buf);
	} else {
	    scriptfeed("[:] ");
	}
	if (ylmin != ylmax) {
	    sprintf(buf, "[%g:%g] ", ylmin, ylmax);
	    scriptfeed(buf);
	} else {
	    scriptfeed("[:] ");
	}

	int c=0;
	for (i=start; i<stop; i++) {
	    if (plottab[i].datum != NULL) {
		if (c!=0) {
		    scriptfeed(",");
		}
		sprintf(buf, "\"-\" title \"%s\"", plottab[i].datum->def);
		scriptfeed(buf);
		c++;
	    }
	}
        scriptfeed("\n");

	for (i=start; i<stop; i++) {
	    if (plottab[i].datum != NULL) {
		for (pd=plottab[i].datum; pd!=NULL; pd=pd->next) {
		    sprintf(buf, "%g %g\n", pd->iv, pd->re);
		    scriptfeed(buf);
		}
		scriptfeed("e\n");
	    }
	}

	start=stop+1;
    }

    scriptfeed("unset multiplot\n");
    scriptclose();
}

/*
    scriptfeed("plot [:][-1:1] sin(x) title \"f(x)\", cos(x), cos(x+1)");

    scriptfeed("set size 1,0.45");
    scriptfeed("set origin 0.0,0.1");

    scriptfeed("set format x \"%g\"");

    scriptfeed("set lmargin 10");
    scriptfeed("set rmargin 10");
    scriptfeed("set bmargin 0");

    scriptfeed("plot [:][-2:2] cos(x)");

    scriptfeed("unset multiplot");

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
	  count++
    
	  sprintf(buf, "set size 1,%g", 0.9/ngraphs);
	  scriptfeed(buf);
	  sprintf(buf, "set origin 0.0,%g", 0.1+(count-1.0)*(0.9/ngraphs));
	  scriptfeed(buf);
	  scriptfeed("set lmargin 10");
	  scriptfeed("set rmargin 10");
	  scriptfeed("set bmargin 0");

	  if (count == ngraphs) {
	      scriptfeed("set format x \"\"");
	  }
	}

	if (p->datum != NULL) {
	    for (pd=p->datum; pd!=NULL; pd=pd->next) {
	        sprintf(buf, "%g %g\n", pd->iv, pd->re);
	        scriptfeed(buf);
	    }
	    scriptfeed(buf);
	}
    }
    scriptclose();
}

xyminmax
newgraph
graphs
...
newgraph
graphs
...

preamble

for(k=0; k<=plotframe; k++) {
    header(k)
    title(ka) title(kb)... title(numplots_k)
    data(ka)
    data(kb)
    ...
    data(kn_1)
}
*/
