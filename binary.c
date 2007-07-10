#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

typedef enum {ADD, SUB, AVG, MAX, MIN, MULT, WARP, DIV, LAM} MODE;

int readnext(FILE *fp, double *t2, double *v2, double *t, double *v);
int docode(double ttb2,double ttb,double tta2,double tta);
double interp(double tt, double x2, double y2, double x1, double y1);
void emit(MODE mm, double *tmax, double tt, double xx, double yy);

int main() {

    double ta2, va2, ta, va;
    double tb2, vb2, tb, vb;
    double tmax;
    int error = 0;
    int c;
    int reta, retb;
    

    FILE *file1;
    FILE *file2;

    MODE mode = ADD;

    file1 = fopen("sig.a", "r+");
    file2 = fopen("sig.b", "r+");

    /* get set up */
    if ((reta = readnext(file1, &ta2, &va2, &ta, &va)) == 0) error++;
    if ((reta = readnext(file1, &ta2, &va2, &ta, &va)) == 0) error++;
    if ((retb = readnext(file2, &tb2, &vb2, &tb, &vb)) == 0) error++;
    if ((retb = readnext(file2, &tb2, &vb2, &tb, &vb)) == 0) error++;

    tmax = ta2;
    if (tb2 < tmax) tmax = tb2;

    if (error) { printf("not enough points!"); exit(1); }

    c = docode(tb2,tb,ta2,ta);

    while (reta != 0 && retb != 0) {
	c = docode(tb2,tb,ta2,ta);
	if (c == 0) {
	    retb=readnext(file2, &tb2, &vb2, &tb, &vb);
	} else if (c == 24) {
	    reta=readnext(file1, &ta2, &va2, &ta, &va);
	} else if (c==1 || c == 2 || c == 7) {

	    /*
	    # 1  tb2    tb       |          out ta2/tb    read b
	    # 2  tb2    |   tb   |          out ta2,tb,   read b
	    # 7         tb2 tb   |          out tab2,tb,  read b
	    */

	    emit( mode, &tmax, ta2, va2, interp(ta2, tb2, vb2, tb, vb));
	    emit( mode, &tmax, tb,  interp(tb, ta2, va2, ta, va),  vb);
	    retb=readnext(file2, &tb2, &vb2, &tb, &vb);

	} else if (c==12 || c == 13 ) {

	    /*
	    #12         | tb2 tb |          out tb2,tb,   read b
	    #13         | tb2    tb         out tb2,tab   read ab *
	    */

	    emit( mode, &tmax, tb2, interp(tb2, ta2, va2, ta, va), vb2);
	    emit( mode, &tmax, tb,  interp(tb, ta2, va2, ta, va), vb);
	    retb=readnext(file2, &tb2, &vb2, &tb, &vb);

	} else if (c==8 || c == 4 || c == 3) {

	    /*
	    # 8         tb2      tb         out tab2,tab, read ab *
	    # 4  tb2    |        |  tb      out ta2,ta,   read a 
	    # 3  tb2    |        tb         out ta2,ta,   read ab *
	    */

	    emit( mode, &tmax, ta2, va2, interp(ta2, tb2, vb2, tb, vb));
	    emit( mode, &tmax, ta,  va,  interp(ta, tb2, vb2, tb, vb));
	    reta=readnext(file1, &ta2, &va2, &ta, &va);

	} else if (c==9 || c == 14 || c == 19) {

	    /*
	    # 9         tb2      |   tb     out tab2,ta,  read a
	    #14         | tb2    |   tb     out tb2,ta    read a
	    #19         |        tb2 tb     out tb2/ta    read a
	    */

	    emit( mode, &tmax, tb2, interp(tb2, ta2, va2, ta, va), vb2);
	    emit( mode, &tmax, ta,  va, interp(ta, tb2, vb2, tb, vb));
	    reta=readnext(file1, &ta2, &va2, &ta, &va);

	} else {
	   printf("bad case!\n");
	}
    }
    return(1);
}

double interp(double tt, double x2, double y2, double x1, double y1) {
    if (x2 > x1 || tt < x2 || tt > x1) {
	printf("interp args out of bounds, non-monotonic time signal?\n");
    }
    return (y1+(y2-y1)*(tt-x1)/(x2-x1));
}


void emit(MODE mm, double *tmax, double tt, double xx, double yy) {

    double xxold, yyold, ttold;
    double t;

    static int initialized = 0;

    if (!initialized) {
       xxold = xx;
       yyold = yy;
       ttold = tt;
       initialized++;
    }

    if (tt > *tmax) {
	if (mm == LAM) {		/* laminate two PWLs */
	   printf("%.16g %.16g %.16g\n", tt, xx, yy);;	
	} else if (mm == ADD) {	/* add two PWLs */
	    printf("%g %g\n", tt, xx+yy);
	} else if (mm == AVG) {	/* average two PWLs */
	    printf("%g %g\n",  tt, (xx+yy)/2.0);
	} else if (mm == SUB) {	/* subtract PWL2 from PWL1 */
	    printf("%g %g\n",  tt, (xx-yy));
	} else if (mm == MAX) {       /*  output maximum value */
	    if ((xxold > yyold &&  xx < yy) || (xxold < yyold && xx > yy)) {

	       /*
	       # find intersection * where x == y
	       # xxold      yy
	       #        *
	       # yyold      xx
	       # ttold  tc  tt
	       #
	       # y = yyold + (yy-yyold)*(t-ttold)/(tt-ttold)
	       # x = xxold + (xx-xxold)*(t-ttold)/(tt-ttold)
	       #
	       # yyold + (yy-yyold)*(t-ttold)/(tt-ttold) =  xxold + (xx-xxold)*(t-ttold)/(tt-ttold)
	       # yyold - xxold  =  ((xx-xxold)-(yy-yyold))*(t-ttold)/(tt-ttold)
	       #
	       # (yyold - xxold)/((xx-xxold)-(yy-yyold)) = (t-ttold)/(tt-ttold)
	       # (tt-ttold)*(yyold - xxold)/((xx-xxold)-(yy-yyold))+ttold = t
	       */

	       t = (tt-ttold)*(yyold - xxold)/((xx-xxold)-(yy-yyold))+ttold;
	       printf("%g %g\n", t, yyold + (yy-yyold)*(t-ttold)/(tt-ttold));
	    }
	    if (xx > yy) {
	       printf("%g %g\n", tt, xx);	
	    } else {
	       printf("%g %g\n", tt, yy);
	    }
	} else if (mm == MIN) {   /* output minimum value */
	    if ((xxold > yyold &&  xx < yy) || (xxold < yyold && xx > yy)) {
	       t = (tt-ttold)*(yyold - xxold)/((xx-xxold)-(yy-yyold))+ttold;
	       printf("%g %g\n", t, yyold + (yy-yyold)*(t-ttold)/(tt-ttold));
	    }
	    if (xx < yy) {
	       printf("%g %g\n", tt, xx);	
	    } else {
	       printf("%g %g\n", tt, yy);
	    }
	} else if (mm == MULT) {  /* multiply PWL 1&2 */
	    printf("%g %g\n", tt, (xx*yy));
	} else if (mm == DIV) {   /*  divide PWL1 by PWL2 */
	    if (yy*yy < 1e-12) {
	       if (yy < 0) {
		   yy = -1e-12;
	       } else {
		   yy = 1e-12;
	       }
	    }
	    printf("%g %g\n", tt, (xx/yy));
	} else if (mm == WARP) {      /* time warp x with y */
	    printf("%g %g\n", tt+yy, xx);
	} else {
	    printf("bad mode: %d\n", mm);
	    exit (1);
	}
	*tmax = tt;
	ttold = tt;
	xxold = xx;
	yyold = yy;
    }
}


int docode(double ttb2,double ttb,double tta2,double tta) {

    int code = 0;

    /*
    # classify relationship between segments:
    # each tb,tb2 point can lie in region 0,2,4 or
    # at times 1,3.  Multiply value of tb2*5 and
    # add to value of tb:
    #
    #          0     1   2    3   4
    #
    #               ta2      ta
    #                |        |       
    # 0    tb2  tb   |        |	     	----------   read b   
    # 1    tb2       tb       |             out ta2,     read b
    # 2    tb2       |   tb   |             out ta2/tb,  read b
    # 3    tb2       |        tb            out ta2/ta,  read b *
    # 4    tb2       |        |   tb        out ta2/ta,  read a 
    # 7              tb2 tb   |          	out tb2/tb,  read b
    # 8              tb2      tb            out ta2/ta,  read b *
    # 9              tb2      |   tb        out ta2/ta,  read a
    #12              | tb2 tb |             out tb2/tb,  read b
    #13              | tb2    tb          	out tb2/tb   read a *
    #14              | tb2    |   tb      	out tb2/ta   read a
    #19              |        tb2 tb      	out tb2      read a
    #24              |        |   tb2 tb   	----------   read a
    #                |        |
    #

    # now sort patterns into consistent blocks sharing same 
    # operation, making use of cases where there is more than
    # one correct operation:

    # -------------------------------------------------------
    # 0  tb2 tb |        |	     	----------    read b   
    # -------------------------------------------------------
    #24         |        |  tb2 tb  ----------    read a
    # -------------------------------------------------------

    # 1  tb2    tb       |          out ta2/tb    read b
    # 2  tb2    |   tb   |          out ta2,tb,   read b
    # 7         tb2 tb   |          out tab2,tb,  read b
    # -------------------------------------------------------

    #12         | tb2 tb |          out tb2,tb,   read b
    #13         | tb2    tb         out tb2,tab   read ab *
    # -------------------------------------------------------

    # 8         tb2      tb         out tab2,tab, read ab *
    # 4  tb2    |        |  tb      out ta2,ta,   read a 
    # 3  tb2    |        tb         out ta2,ta,   read ab *
    # -------------------------------------------------------

    # 9         tb2      |   tb     out tab2,ta,  read a
    #14         | tb2    |   tb     out tb2,ta    read a
    #19         |        tb2 tb     out tb2/ta    read a
    # -------------------------------------------------------

    # we end up generating redundant points, but
    # can suppress them by making sure output time grows
    # monotonically.

    */

    if (ttb <= tta2) {
	if (ttb == tta2) {
	   code += 1;
	} 
    } else if (ttb <= tta) {
	if (ttb == tta) {
	    code += 3;
	} else {
	    code += 2;
	}
    } else if (ttb > tta) {
	code += 4;
    } 

    if (ttb2 <= tta2) {
	if (ttb2 == tta2) {
	   code += 5;
	} 
    } else if (ttb2 <= tta) {
	if (ttb2 == tta) {
	    code += 15;
	} else {
	    code += 10;
	}
    } else if (ttb2 > tta) {
	code += 20;
    } 
    /* print "docode", tb2, tb, ta2, ta, code */
    return(code);
}

#define MAXBUF 128

int readnext(FILE *fp, double *t2, double *v2, double *t, double *v) {
    double npts_read;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    double x;
    double y;

    /* scan a line of file *fp for two doubles */

    if (fp==NULL) 
        exit(2);

    read=getline(&line, &len, fp);
    if (read != -1) {
	if ((npts_read = sscanf(line, "%lg %lg", &x, &y)) != 2) {
	    return(0);
	}
    } else {
        return(0);
    }
 
    *t2=*t; *t=x; 
    *v2=*v; *v=y;

    return(npts_read);
}
