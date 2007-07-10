/* 
 * label.c: demonstrate nice graph labeling
 * Paul Heckbert 2 Dec 88
 * --- typed in from "Graphic Gems in C"
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX(a,b) (((a)>(b))?(a):(b))

static double nicenum();


#define NTICK 5	/* desired number of tick marks */

main(int ac, char ** av) {
    double min, max;
    if (ac!=3) {
    	fprintf(stderr,"usage: label <min> <max>\n");
	exit(1);
    }
    min = atof(av[1]);
    max = atof(av[2]);
    loose_label(min,max,NTICK);
}

loose_label(double min, double max, int nticks) {
    char str[6], temp[20];
    int nfrac;
    double d;	/* tick mark spacing */
    double graphmin, graphmax;
    double range, x;

    /* we expect min!=max */
    range = nicenum(max-min, 0);
    d = nicenum(range/(nticks-1),1);
    graphmin = floor(min/d)*d;
    graphmax = ceil(max/d)*d;
    nfrac = MAX(-floor(log10(d)),0);	/* # frac digits to show */
    sprintf(str,"%%.%df", nfrac); 	/* simplest axis labels */
    printf("graphmin=%g graphmax=%g increment=%g\n",
        graphmin, graphmax, d);
    for (x=graphmin; x<graphmax+0.5*d; x+=d) {
        sprintf(temp, str, x);
        printf("(%s)\n", temp);
    }
}

/*
 * nicenum: find a "nice" number approximately equal to x.
 * Round the number if round=1, take ceiling if round=0
 */

static double nicenum(x,round)
double x;
int round;
{
    int exp;	/* exponent of x */
    double f;	/* fractional part of x */
    double nf;	/* nice, rounded fraction */


    exp = floor(log10(x));
    f = x/pow(10.0, (double) exp);	/* between 1 and 10 */

    if (round) {
        if (f<1.5) nf = 1.0;
	else if (f<3.0) nf = 2.0;
	else if (f<7.0) nf = 5.0;
	else nf = 10.0;
    } else {
        if (f<1.0) nf = 1.0;
	else if (f<2.0) nf = 2.0;
	else if (f<5.0) nf = 5.0;
	else nf = 10.0;
    }

    return nf*pow(10.0, (double) exp);
}
