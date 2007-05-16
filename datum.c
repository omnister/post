#include "post.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 	/* for sleep() */

static int mem_use=0;
static int debug=0;

int readnext( DATUM **list, DATUM *datumold, DATUM *datum);
int docode(double ttb2,double ttb,double tta2,double tta);
double xinterp(double tt, double x2, double y2, double x1, double y1);
void emit(BINOP op, DATUM **result, double *tmax, DATUM *datuma, DATUM *datumb);
DATUM *twovectors( BINOP op, DATUM *a, DATUM *b);

int dat_stat() {			/* return number of malloc'd datums */
    return (mem_use);
}

DATUM * interp(DATUM *var, DATUM *pt) { 		/* interpolate a PWL */
   DATUM *pd;
   double t, told;
   double x, xold;
   double i, iold;
   if (var == NULL || pt == NULL) {
   	return(NULL);
   }

   pd = var;
   if (pd->next == NULL) {
   	return(NULL);
   }

   told = pd->iv;
   xold = pd->re;
   iold = pd->im;

   for (pd=pd->next; pd!=NULL; pd=pd->next) {
       t = pd->iv;
       x = pd->re;
       i = pd->im;

       if (told <= pt->re && t >= pt->re) {
    	   return (
	   	new_dat( x+(xold-x)*(t-pt->re)/(t-told),
	   	i+(iold-i)*(t-pt->re)/(t-told))
	   );
       }
       iold = i;
       told = t;
       xold = x;
   }
   return (NULL);
}

//
// Inserting into a single linked list is a highly inefficient n^2 procedure 
// so we modify DATUM to have a prev pointer to keep tail address handy 
//
//	    -----------   -----------
//	    |          |  |         |
//          V          |  V         |
//  -->[element1]---->[elem2]----->[e3]---->|null
//        |                         ^
//        |                         |
//	  --------------------------+
//           
// So every element points to it's predecessor, except the first element
// which points at the last one.
//
//

DATUM * link_dat(DATUM *head, DATUM *tail) { 	/* link two DATUMs */

   DATUM *tmp;

   if (head == NULL) {			/* linking tail to NULL */	
      head = tail;
      tail->prev=tail;
   } else if (head->prev == NULL) {	/* linking two scalars */
       head->next = tail;
       head->prev = tail;
       tail->prev = head;
   } else {			/* linking scalar tail to vector head */
       tmp = head->prev; 	/* last element */
       tmp->next = tail;        /* append new element */
       head->prev = tail;	/* make head point to new tail */
       tmp->next->prev = tmp;   /* make new point */
   }
   return (head);
}

DATUM * dup_dat(DATUM *p) { 	/* duplicate a DATUM list */
   DATUM *pd;
   DATUM *new;
   DATUM *pn;
   DATUM *tmp;

   new = pn = new_dat(p->re, p->im);
   pn->iv = p->iv;
   pn->def = strsave(p->def);

   for (pd=p->next; pd!=NULL; pd=pd->next) {
	tmp = new_dat(pd->re, pd->im);
	tmp->iv = pd->iv;
	link_dat(new, tmp);
   }
   return(new);
}

DATUM * new_dat(double re, double im) { 	/* malloc a permanent DATUM type */
    DATUM *p;
    mem_use++;
    p = (DATUM *) malloc(sizeof(DATUM));
    p -> iv = 0.0;	/* independent variable for list of data points */
    p -> re = re;       /* real part of complex value */
    p -> im = im;	/* imag part of complex value */
    p -> def = NULL;	/* textual equivalent */
    p -> next = (DATUM *) NULL;
    p -> prev = (DATUM *) NULL;
    if (debug) printf("creating datum %g %g\n", re, im);
    return (p);
}

void free_dat(DATUM *p) {		/* free a DATUM type */
    DATUM *pd;
    DATUM *ps;

    if (debug) printf("freeing datum %g %g\n", p->re, p->im);
    for (pd=p; pd!=NULL; pd=ps) {
        ps = pd->next;
	if(pd->def != NULL) {
	   free(pd->def);
	}
   	free(pd);
	if (debug) printf("free\n");
   	mem_use--;
    }
}

void print_dat(DATUM *p) {		/* format and print a datum */
   DATUM *pd;
   if (p == NULL) {
   	printf("\t    <NULL>\n");
   } else if (p->next != NULL) {
       printf("\t{\n"); 
       for (pd=p; pd!=NULL; pd=pd->next) {
       	   printf("\t    %.12g,", pd->iv);
	   if (pd->im == 0.0) {
		printf("%.12g\n",  pd->re); 		/* 8+0i prints as "8" */
	   } else {
		if (pd->re == 0.0) {
		    if (pd->im == 1.0) {
			printf("i\n"); 		/* 1i => "i" */
		    } else {
			printf("%.12gi\n", pd->im); 	/* 2i => "2i" */
		    }
		} else {
		    if (pd->im == 1.0) {
			printf("%.12g+i\n", pd->re); 	/* 8+1i => "8+i" */
		    } else {
			printf("%.12g%+.12gi\n", pd->re, pd->im); /* 8+2i => "8+2i */
		    }
		}
	   }
       }
       printf("\t}\n"); 
   } else {
	if (p->im == 0.0) {
	    printf("\t%.12g\n",  p->re); 		/* 8+0i prints as "8" */
	} else {
	    if (p->re == 0.0) {
		if (p->im == 1.0) {
		    printf("\ti\n"); 		/* 1i => "i" */
		} else {
		    printf("\t%.12gi\n", p->im); 	/* 2i => "2i" */
		}
	    } else {
		if (p->im == 1.0) {
		    printf("\t%.12g+i\n", p->re); 	/* 8+1i => "8+i" */
		} else {
		    printf("\t%.12g%+.12gi\n", p->re, p->im); /* 8+2i => "8+2i */
		}
	    }
	}
    }
}

DATUM * scalar(BINOP operation, DATUM *a, DATUM *b) {	/* do a binary operation on scalar DATUMS */
    DATUM *tmp;
    double re1, im1;
    double re2, im2;

    /* this is a service function for binary() */
    /* we ignore [ab]->next, [ab]->iv and let binary() handle those details */

    switch (operation) {
	case ADD:
	    return(new_dat(a->re + b->re, a->im + b->im));
	    break;
	case AVG:
	    return(new_dat((a->re + b->re)/2.0, (a->im + b->im)/2.0));
	    break;
	case DIV:
	    return(
		new_dat( 
		    (a->re*b->re + a->im*b->im) / (b->im*b->im + b->re*b->re),
		    (-a->re*b->im + b->re*a->im) / (b->im*b->im + b->re*b->re)
		)
	    );
	    break;
	case MAX:
	    if ((a->re) > (b->re)) {
		return(new_dat(a->re, a->im));
	    } else {
		return(new_dat(b->re, b->im));
	    }
	    break;
	case MIN:
	    if ((a->re) > (b->re)) {
		return(new_dat(b->re, b->im));
	    } else {
		return(new_dat(a->re, a->im));
	    }
	    break;
	case MULT:
	    re1 = (a->re * b->re) - (a->im * b->im);
	    im1 = (a->re * b->im) + (a->im * b->re);
	    return(new_dat(re1, im1));
	    break;
	case POW:
	    if (a->im == 0.0 && b->im == 0.0) {
		return( new_dat( pow(a->re,b->re), 0.0) );
	    } else {
		/* ln(z) */
		re1 = log(sqrt((a->re*a->re + a->im*a->im)));
		im1 = atan2(a->im, a->re);

		/* multiply ln(z) by u */
		re2 = re1*b->re - im1*b->im;
		im2 = re1*b->im + b->re*im1;

		/* take exp(re2 + i*im2) */
		return( new_dat( exp(re2)*cos(im2), exp(re2)*sin(im2) ));
	    }
	    break;
	case SUB:
	    return(new_dat(a->re - b->re, a->im - b->im));
	    break;
	case WARP:
	    tmp = new_dat(a->re, a->im);
	    tmp->iv = a->iv + b->re;
	    return(tmp);
	    break;
	default:
	    printf("bad case in scalar/scalar binary op table\n");
	    return(new_dat(0.0, 0.0));
	    break;
    }
    printf("fell off scalar without matching any cases!\n");
    return(new_dat(0.0, 0.0));
}


DATUM * binary(BINOP op, DATUM *a, DATUM *b) {	/* do a binary operation on vector DATUMS */
    DATUM *tmp;
    DATUM *p;
    DATUM *head;

    if (b == NULL) {
       printf("function needs two arguments!\n");
       return(NULL);
    }
    int b_is_scalar = 0;

    if (a->next == NULL && b->next == NULL) {		/* both scalar */
        return(scalar(op, a, b));
    } else if (a->next != NULL && b->next != NULL) {	/* both vector */
	return(twovectors(op, a, b));	
    } else {                                            /* mixed */
        if (b->next == NULL) {       
	    b_is_scalar++;				/* flag b as scalar */
	}
	head = scalar(op, a, b);
	if (b_is_scalar) {
	    if (op != WARP) head->iv = a->iv; 		
	    for (p=a->next; p!=NULL; p=p->next) {
		tmp = scalar(op, p, b);
		if (op != WARP) tmp->iv = p->iv;
		link_dat(head,tmp);
	    }
	} else {
	    if (op != WARP) head->iv = b->iv;  	
	    for (p=b->next; p!=NULL; p=p->next) {
		tmp = scalar(op, a, p);
		if (op != WARP) tmp->iv = p->iv;
		link_dat(head,tmp);
	    }
	}
        return(head);
    }
    printf("fell off binary without matching any cases!\n");
    return(new_dat(0.0, 0.0));
}

double xinterp(double tt, double x2, double y2, double x1, double y1) {
    if (x2 > x1 || tt < x2 || tt > x1) {
	printf("xinterp args out of bounds, non-monotonic time signal?\n");
    }
    return (y1+(y2-y1)*(tt-x1)/(x2-x1));
}


/* return a DATUM interpolated between a2 and a1 at time t */

DATUM *zint(double t, DATUM *a2, DATUM *a) {

    double re, im;
    DATUM *tmp;

    if (a2->iv > a->iv || t < a2->iv || t > a->iv) {
	printf("zint args out of bounds, non-monotonic time signal?\n");
    }
    
    re = xinterp(t, a2->iv, a2->re, a->iv, a->re);  
    im = xinterp(t, a2->iv, a2->im, a->iv, a->im);  

    tmp = new_dat(re, im);
    tmp->iv = t;

    return (tmp);
}


DATUM *twovectors( BINOP op, DATUM *a, DATUM *b) {

    double tmax;
    int error = 0;
    int c;
    int reta, retb;
    
    DATUM *lista;
    DATUM *listb;
    DATUM *result=NULL;

    DATUM datuma2;
    DATUM datuma;
    DATUM datumb2;
    DATUM datumb;

    DATUM *tmp1;
    DATUM *tmp2;

    lista = a;
    listb = b;

    /* get set up */
    if ((reta = readnext(&lista, &datuma2, &datuma)) == 0) error++;
    if ((reta = readnext(&lista, &datuma2, &datuma)) == 0) error++;
    if ((retb = readnext(&listb, &datumb2, &datumb)) == 0) error++;
    if ((retb = readnext(&listb, &datumb2, &datumb)) == 0) error++;

    /* set the tmax variable (the highest time outputted) */
    /* to be the most early time point */

    tmax = datuma2.iv;
    if (datumb2.iv < tmax) tmax = datumb2.iv;
    tmax-=1.0;

    if (error) { printf("not enough points!"); exit(1); }

    c = docode(datumb2.iv, datumb.iv, datuma2.iv, datuma.iv);

    while (reta != 0 && retb != 0) {
	c = docode(datumb2.iv, datumb.iv, datuma2.iv, datuma.iv);
	if (c == 0) {
	    retb = readnext(&listb, &datumb2, &datumb);
	} else if (c == 24) {
	    reta = readnext(&lista, &datuma2, &datuma);
	} else if (c==1 || c == 2 || c == 7 || c == 6) {	/* 6 is bug fix for coincident points */
	    emit(op, &result, &tmax, &datuma2, tmp1=zint(datuma2.iv, &datumb2, &datumb));
	    emit(op, &result, &tmax, tmp2=zint(datumb.iv, &datuma2, &datuma), &datumb);
	    free_dat(tmp1); free_dat(tmp2);
	    retb = readnext(&listb, &datumb2, &datumb);

	} else if (c==12 || c == 13 ) {
	    emit(op, &result, &tmax, tmp1=zint(datumb2.iv, &datuma2, &datuma), &datumb2);
	    emit(op, &result, &tmax, tmp2=zint(datumb.iv, &datuma2, &datuma), &datumb);
	    free_dat(tmp1); free_dat(tmp2);
	    retb = readnext(&listb, &datumb2, &datumb);

	} else if (c==8 || c == 4 || c == 3) {
	    emit(op, &result, &tmax, &datuma2, tmp1=zint(datuma2.iv, &datumb2, &datumb));
	    emit(op, &result, &tmax, &datuma, tmp2=zint(datuma.iv, &datumb2, &datumb));
	    free_dat(tmp1); free_dat(tmp2);
	    reta = readnext(&lista, &datuma2, &datuma);

	} else if (c==9 || c == 14 || c == 19) {
	    emit(op, &result, &tmax, tmp1=zint(datumb2.iv, &datuma2, &datuma), &datumb2);
	    emit(op, &result, &tmax, &datuma, tmp2=zint(datuma.iv, &datumb2, &datumb));
	    free_dat(tmp1); free_dat(tmp2);
	    reta = readnext(&lista, &datuma2, &datuma);

	} else {
	   printf("bad case: %d!\n", c);
	}
    }
    return(result);
}



/* do BINOP on datuma, and datumb, append the result to **result */

void emit(BINOP op, DATUM **result, double *tmax, DATUM *datuma, DATUM *datumb) {

    static DATUM datuma_old;
    static DATUM datumb_old;
    DATUM *tmp;
    DATUM *tmp1;
    DATUM *tmp2;
    double told;
    double t;

    double x,y;
    double xold, yold;
    double tt, ttold, tint;

    t = datuma->iv;

    if (*result == NULL) { 		/* initialize old values */
       datuma_old = *datuma;
       datumb_old = *datumb;
       told = t;
    }

    if (t > *tmax) {
	switch(op) {
	    case ADD:
	    case AVG:
	    case SUB:
	    case DIV:
	    case LAM:
	    case POW:
		tmp = scalar(op, datuma, datumb); tmp->iv = t;
		*result = link_dat(*result, tmp);
	        break;
	    case WARP:
		tmp = scalar(op, datuma, datumb);
		*result = link_dat(*result, tmp);
	        break;
	    case MULT: 	/* FIXME: need to find zero crossings */
	    case MAX:
	    case MIN:

	        /*
	        # find intersection * where x == y
	        # y = yyold + (yy-yyold)*(t-ttold)/(tt-ttold)
	        # x = xxold + (xx-xxold)*(t-ttold)/(tt-ttold)
	        #
	        # yyold + (yy-yyold)*(t-told)/(tt-told) =  
	        #      xxold + (xx-xxold)*(t-told)/(tt-told)
	        #
	        # yyold-xxold  =  ((xx-xxold)-(yy-yyold))*(t-told)/(tt-told)
	        #
	        # (yyold - xxold)/((xx-xxold)-(yy-yyold)) = (t-told)/(tt-told)
	        # t = (tt-told)*(yyold - xxold)/((xx-xxold)-(yy-yyold))+told 
	        */

		if ((datuma_old.re > datumb_old.re && datuma->re < datumb->re) ||
		   (datuma_old.re < datumb_old.re && datuma->re > datumb->re) ) {
                   
		   x = datuma->re;
		   y = datumb->re;
		   xold = datuma_old.re;
		   yold = datumb_old.re;
		   tt = datuma->iv;
		   ttold = datuma_old.iv;
	           tint = (tt-ttold)*(yold - xold)/((x-xold)-(y-yold))+ttold;

		   tmp = scalar(op, 
		   	tmp1=zint(tint, &datuma_old, datuma),
		   	tmp2=zint(tint, &datumb_old, datumb) 
		   );
		   free(tmp1); free(tmp2);
		   tmp->iv = tint;
		   *result = link_dat(*result, tmp); 
	        }

		tmp = scalar(op, datuma, datumb); 
		tmp->iv = t;
		*result = link_dat(*result, tmp); 

	        break;
	    default:
		printf("bad mode: %d\n", op);
		exit (1);
		break;
	}

        datuma_old = *datuma;
        datumb_old = *datumb;
        told = t;
	*tmax = t;
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


int readnext( DATUM **list, DATUM *datumold, DATUM *datum) {

    if ( *list == NULL ) {
       return(0);
    }

    datumold->iv = datum->iv; 
    datumold->re = datum->re; 
    datumold->im = datum->im; 

    datum->iv = (*list)->iv;
    datum->re = (*list)->re;
    datum->im = (*list)->im;

    *list = (*list)->next;

    return(1);
}


/* ------------------------------------------------------------ */

DATUM * Sqrt(DATUM *a, DATUM *b) {
    DATUM tmp;

    if (b != NULL) {
       printf("function takes only one argument!\n");
       return(NULL);
    }

    tmp.re=0.5;
    tmp.im=0.0;
    tmp.next = NULL;
    tmp.iv = 0.0;
    tmp.def = NULL;
    return(binary(POW, a, &tmp));
}

DATUM * Avg(DATUM *a, DATUM *b) {

    DATUM *tmp;
    DATUM *pd;
    double miniv, maxiv;
    double re, re2, im, im2, iv, iv2;
    double reintegral=0.0;
    double imintegral=0.0;

    if (b != NULL) {
       return(binary(AVG, a, b));
    } else {
	if (a->next == NULL) {
	   return(dup_dat(a));
	}
        miniv=a->iv;
	maxiv=a->prev->iv;

	re2=a->re; 
	im2=a->im; 
	iv2=a->iv;
	for (pd=a->next; pd!=NULL; pd=pd->next) {
	    re=pd->re; 
	    im=pd->im; 
	    iv=pd->iv;
	    /* trapezoidal integration */
	    imintegral+=(iv-iv2)*(im+im2)/2.0;
	    reintegral+=(iv-iv2)*(re+re2)/2.0;
	    re2 = re;
	    im2 = im;
	    iv2 = iv;
	}
	tmp = new_dat(reintegral/(maxiv-miniv), imintegral/(maxiv-miniv));
	return(tmp);
    }
}

DATUM * Max(DATUM *a, DATUM *b) {

    DATUM *max;
    DATUM *pd;

    if (b != NULL) {
	return(binary(MAX, a, b));
    } else { 
	max = new_dat(a->re, a->im);
	for (pd=a->next; pd!=NULL; pd=pd->next) {
	   if (a->re > max->re) {
	       max->re = a->re;
	       max->im = a->im;
	   }
	}
    }
    return(max);
}

DATUM * Min(DATUM *a, DATUM *b) {

    DATUM *min;
    DATUM *pd;

    if (b != NULL) {
	return(binary(MIN, a, b));
    } else { 
	min = new_dat(a->re, a->im);
	for (pd=a->next; pd!=NULL; pd=pd->next) {
	   if (a->re < min->re) {
	       min->re = a->re;
	       min->im = a->im;
	   }
	}
    }
    return(min);
}

DATUM * Pow(DATUM *a, DATUM *b) {
    return(binary(POW, a, b));
}

DATUM * Warp(DATUM *a, DATUM *b) {
    return(binary(WARP, a, b));
}

DATUM * Integral(DATUM *a, DATUM *b) {
    DATUM *pd;
    DATUM *result;
    DATUM *new;
    double reintegral=0.0;
    double imintegral=0.0;
    double re, im, re2, im2, iv, iv2;

    if (b != NULL) {
       printf("function takes only one argument!\n");
       return(NULL);
    }

    if (a->next == NULL) {
       return(new_dat(0.0, 0.0));
    }

    re2=a->re; 
    im2=a->im; 
    iv2=a->iv;
    result=new_dat(0.0,0.0);
    result->iv = a->iv;

    for (pd=a->next; pd!=NULL; pd=pd->next) {
        re=pd->re; 
        im=pd->im; 
	iv=pd->iv;

	/* trapezoidal integration */

	imintegral+=(iv-iv2)*(im+im2)/2.0;
	reintegral+=(iv-iv2)*(re+re2)/2.0;

	new = new_dat(reintegral, imintegral);
	new->iv = iv;

	result = link_dat(result, new);
	re2 = re;
	im2 = im;
	iv2 = iv;
    }
    return(result);
}

DATUM * ui(DATUM *a, DATUM *b) {
    DATUM *pd;
    DATUM *result=NULL;
    DATUM *tmp;
    double re, re2, iv, iv2;
    double oldtcross;
    double tcross;
    int cross = 0;
    int debug=0;

    if (b != NULL) {
       printf("function takes only one argument!\n");
       return(NULL);
    }

    if (a->next == NULL) {
       return(new_dat(0.0, 0.0));
    }

    re=a->re; 
    iv=a->iv;
    for (pd=a->next; pd!=NULL; pd=pd->next) {
	re2 = re; re=pd->re; 
	iv2 = iv; iv=pd->iv; 
        
	// if ((re2 < 0 && re >= 0) || (re2 > 0 && re <= 0)) {
	if ((re2 < 0 && re >= 0)) {
	    oldtcross = tcross;
	    tcross = (iv2 + (iv-iv2)*(0.0-re2)/(re-re2));
	    if (debug) printf("%g %g %g %g %g\n", iv2, re2, iv, re, tcross);
	    cross++;
	    if (cross > 1) {
		tmp = new_dat(tcross-oldtcross, 0.0);
		tmp->iv = (double) iv;
		result = link_dat(result, tmp);
	    }
	}
    }
    return(result);
}

DATUM * xcross(DATUM *a, DATUM *b) {
    DATUM *pd;
    DATUM *result=NULL;
    DATUM *tmp;
    double re, re2, iv, iv2;
    double tcross;
    int cross = 0;
    int number;
    int debug=0;

    if (b == NULL) {
       printf("function requires two arguments!\n");
       return(NULL);
    }

    if (a->next == NULL) {
       return(new_dat(0.0, 0.0));
    }

    number = (int) b->re;

    re=a->re; 
    iv=a->iv;
    for (pd=a->next; pd!=NULL; pd=pd->next) {
	re2 = re; re=pd->re; 
	iv2 = iv; iv=pd->iv; 
        
	if ((re2 < 0 && re >= 0) || (re2 > 0 && re <= 0)) {
	    tcross = (iv2 + (iv-iv2)*(0.0-re2)/(re-re2));
	    if (debug) printf("%g %g %g %g %g\n", iv2, re2, iv, re, tcross);
	    cross++;
	    if ((number == 0)) { 
		tmp = new_dat(tcross, 0.0);
		tmp->iv = (double) cross;
	        result = link_dat(result, tmp);
	    } else if (cross == number) {
	        result = link_dat(result, new_dat(tcross, 0.0));
	    }
	}
    }
    return(result);
}

DATUM * xcrossn(DATUM *a, DATUM *b) {
    DATUM *pd;
    DATUM *result=NULL;
    DATUM *tmp;
    double re, re2, iv, iv2;
    double tcross;
    int cross = 0;
    int number;
    int debug=0;

    if (b == NULL) {
       printf("function requires two arguments!\n");
       return(NULL);
    }

    if (a->next == NULL) {
       return(new_dat(0.0, 0.0));
    }

    number = (int) b->re;

    re=a->re; 
    iv=a->iv;
    for (pd=a->next; pd!=NULL; pd=pd->next) {
	re2 = re; re=pd->re; 
	iv2 = iv; iv=pd->iv; 
        
	if ((re2 > 0 && re <= 0)) {
	    tcross = (iv2 + (iv-iv2)*(0.0-re2)/(re-re2));
	    if (debug) printf("%g %g %g %g %g\n", iv2, re2, iv, re, tcross);
	    cross++;
	    if ((number == 0)) { 
		tmp = new_dat(tcross, 0.0);
		tmp->iv = (double) cross;
	        result = link_dat(result, tmp);
	    } else if (cross == number) {
	        result = link_dat(result, new_dat(tcross, 0.0));
	    }
	}
    }
    return(result);
}

DATUM * xcrossp(DATUM *a, DATUM *b) {
    DATUM *pd;
    DATUM *result=NULL;
    DATUM *tmp;
    double re, re2, iv, iv2;
    double tcross;
    int cross = 0;
    int number;
    int debug=0;

    if (b == NULL) {
       printf("function requires two arguments!\n");
       return(NULL);
    }

    if (a->next == NULL) {
       return(new_dat(0.0, 0.0));
    }

    number = (int) b->re;

    re=a->re; 
    iv=a->iv;
    for (pd=a->next; pd!=NULL; pd=pd->next) {
	re2 = re; re=pd->re; 
	iv2 = iv; iv=pd->iv; 
        
	if ((re2 < 0 && re >= 0)) {
	    tcross = (iv2 + (iv-iv2)*(0.0-re2)/(re-re2));
	    if (debug) printf("%g %g %g %g %g\n", iv2, re2, iv, re, tcross);
	    cross++;
	    if ((number == 0)) { 
		tmp = new_dat(tcross, 0.0);
		tmp->iv = (double) cross;
	        result = link_dat(result, tmp);
	    } else if (cross == number) {
	        result = link_dat(result, new_dat(tcross, 0.0));
	    }
	}
    }
    return(result);
}

/* 
FIXME: everything from here down should be refactored into unaryscalar() and 
a unary() functions using case statements like the binary() and scalar() above.
Will avoid multiple repetitions of the iteration logic for each data type.
*/

DATUM * Re(DATUM *a, DATUM *b) {

    DATUM *p;
    DATUM *head;
    DATUM *tmp;

    if (b != NULL) {
       printf("function takes only one argument!\n");
       return(NULL);
    }

    if (a->next == NULL) {
	return( new_dat( a->re, 0.0 ) ); 
    } else {
        head = new_dat(a->re, 0.0);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    tmp = new_dat(p->re, 0.0);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}

DATUM * Im(DATUM *a, DATUM *b) {

    DATUM *p;
    DATUM *head;
    DATUM *tmp;

    if (b != NULL) {
       printf("function takes only one argument!\n");
       return(NULL);
    }

    if (a->next == NULL) {
	return( new_dat( a->im, 0.0 ) ); 
    } else {
        head = new_dat(a->im, 0.0);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    tmp = new_dat(p->re, 0.0);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}

DATUM * Pha(DATUM *a, DATUM *b) {
    DATUM *p;
    DATUM *head;
    DATUM *tmp;

    if (b != NULL) {
       printf("function takes only one argument!\n");
       return(NULL);
    }

    if (a->next == NULL) {
	return( new_dat(  atan2(a->im, a->re) , 0.0 ) ); 
    } else {
        head = new_dat( atan2(a->im, a->re), 0.0);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    tmp = new_dat(atan2(p->im, p->re), 0.0);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}

DATUM * Mag(DATUM *a, DATUM *b) {
    DATUM *p;
    DATUM *head;
    DATUM *tmp;

    if (b != NULL) {
       printf("function takes only one argument!\n");
       return(NULL);
    }

    if (a->next == NULL) {
	return( new_dat(sqrt((a->re*a->re + a->im*a->im)), 0.0 ) ); 
    } else {
        head = new_dat( sqrt((a->re*a->re + a->im*a->im))  , 0.0);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    tmp = new_dat( sqrt((p->re*p->re + p->im*p->im)), 0.0);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}

DATUM * Db(DATUM *a, DATUM *b) {
    DATUM *p;
    DATUM *head;
    DATUM *tmp;

    if (b != NULL) {
       printf("function takes only one argument!\n");
       return(NULL);
    }

    if (a->next == NULL) {
	return( new_dat(20.0*log(sqrt((a->re*a->re + a->im*a->im)))/log(10.0), 0.0 ) ); 
    } else {
        head = new_dat(20.0*log(sqrt((a->re*a->re + a->im*a->im)))/log(10.0)  , 0.0);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    tmp = new_dat(20.0*log(sqrt((p->re*p->re + p->im*p->im)))/log(10.0), 0.0);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}

void doln(double *re,double *im) {
    double radius;
    double angle;
    
    radius = sqrt(((*re)*(*re) + (*im)*(*im)));
    angle = atan2(*im, *re);

    *re = log(radius);
    *im = angle;
}

DATUM * Log10(DATUM *a, DATUM *b) {
    DATUM *p;
    DATUM *head;
    DATUM *tmp;
    double re, im;

    if (b != NULL) {
       printf("function takes only one argument!\n");
       return(NULL);
    }

    if (a->next == NULL) {
	re = a->re; im = a->im; doln(&re, &im);
	return( new_dat(re/log(10.0), im) ); 
    } else {
	re = a->re; im = a->im; doln(&re, &im);
        head = new_dat(re/log(10.0), im);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    re = p->re; im = p->im; doln(&re, &im);
	    tmp = new_dat(re/log(10.0),im);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}

DATUM * Ln(DATUM *a, DATUM *b) {
    DATUM *p;
    DATUM *head;
    DATUM *tmp;
    double re, im;

    if (b != NULL) {
       printf("function takes only one argument!\n");
       return(NULL);
    }

    if (a->next == NULL) {
	re = a->re; im = a->im; doln(&re, &im);
	return( new_dat(re, im) ); 
    } else {
	re = a->re; im = a->im; doln(&re, &im);
        head = new_dat(re, im);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    re = p->re; im = p->im; doln(&re, &im);
	    tmp = new_dat(re,im);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}

void doexp(double *re, double *im) {
    double real, imag;
    /* e^(a+bi) = (e^a)*(cos(b)+i(sin(b)) */
    real = exp(*re)*cos(*im);
    imag = exp(*re)*sin(*im);
    *re = real;
    *im = imag;
}

DATUM * Exp(DATUM *a, DATUM *b) {		
    DATUM *p;
    DATUM *head;
    DATUM *tmp;
    double re, im;

    if (b != NULL) {
       printf("function takes only one argument!\n");
       return(NULL);
    }

    if (a->next == NULL) {
	re = a->re; im = a->im; doexp(&re, &im);
	return( new_dat(re, im) ); 
    } else {
	re = a->re; im = a->im; doexp(&re, &im);
        head = new_dat(re, im);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    re = p->re; im = p->im; doexp(&re, &im);
	    tmp = new_dat(re,im);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}

/* e = (warp(a,-DT/2)-warp(a,DT/2))/DT */

DATUM * dt(DATUM *a, DATUM *b) {		
    DATUM *c, *d, *e;
    DATUM arg;
    Symbol *s;
    double dt;

    if (b != NULL) {
       printf("function takes only one argument!\n");
       return(NULL);
    }

    s=lookup("DT");

    dt = s->u.val->re;

    arg.next = NULL;
    arg.prev = &arg;
    arg.iv = 0.0;
    arg.re = -dt/2.0;
    arg.im = 0.0;

    c = Warp(a, &arg);
    arg.re = dt/2.0;
    d = Warp(a, &arg);

    e = binary(SUB, c, d);
    free_dat(c);
    free_dat(d);

    arg.re = 1.0/dt;

    c = binary(MULT, e, &arg);
    free_dat(e);

    return(c);
}

DATUM * dopause(DATUM *a, DATUM *b) {		/* ansi already has a pause() command */

    if (b != NULL) {
       printf("function takes only one argument!\n");
       return(NULL);
    }

    return(new_dat((double) sleep((int) a->re) ,0.0));
}

DATUM * lpf(DATUM *a, DATUM *b) {		
    DATUM *p, *tmp;
    DATUM *head;
    double t, tau, dt, vx, v, vold, tnew, told, slope, vflat, vout;

    if (b == NULL) {
       printf("function needs two arguments!\n");
       return(NULL);
    }

    tau = b->re; 

    if (a->next == NULL) {
	return( new_dat(a->re, a->im) ); 
    } else {
	vout = v = a->re;
        head = new_dat(vout, 0.0);
        tnew = head->iv = a->iv; 
	t = tnew+tau/16.0;
	for (p=a->next; p!=NULL; p=p->next) {
	    vold = v; v = p->re;
	    told = tnew; tnew = p->iv;
	    dt=tnew-told;

	    vflat = vold-vout;        /* flat portion */
	    slope = (v-vold)/dt;      /* ramp portion */

	    while(t <= tnew) {
	        vx = vout + (vflat)*(1.0-exp(-(t-told)/tau));
		/* printf("%g %g %g %g %g %g %g %g %g\n", v, vout, vold, vflat, t, tau, tnew, told, dt); */
	        vx  += slope*((t-told) + tau*(exp(-(t-told)/tau) - 1.0));
		tmp = new_dat(vx,0.0);
		tmp->iv = t;
		link_dat(head,tmp);
		t+=tau/16.0;
            }

	    /* printf("... %g %g %g %g\n", vflat, dt, tau, slope); */
	    vout += vflat*(1.0-exp(-dt/tau));
	    vout += slope*(dt + tau*(exp(-dt/tau) - 1.0));
       }
       return(head);
    }
}
