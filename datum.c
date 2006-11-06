#include "datum.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"

static int mem_use=0;
static int debug=0;

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
    	   return (new_dat_perm( x+(xold-x)*(t-pt->re)/(t-told) , i+(iold-i)*(t-pt->re)/(t-told)));
       }
       iold = i;
       told = t;
       xold = x;
   }
   return (NULL);
}

/* FIXME: link_dat is a highly inefficient n^2 procedure */
/* modify DATUM to have a prev pointer to keep tail address handy */

DATUM * link_dat(DATUM *head, DATUM *tail) { 	/* link two DATUMs */
   DATUM *pd;
   for (pd=head; pd->next!=NULL; pd=pd->next) {
   	;
   }
   pd->next = tail;
   return (head);
}

DATUM * dup_dat(DATUM *p) { 	/* duplicate a DATUM */
   DATUM *pd;
   DATUM *new;
   DATUM *pn;

   new = pn = new_dat(p->re, p->im);
   pn->iv = p->iv;

   for (pd=p->next; pd!=NULL; pd=pd->next) {
	pn->next = new_dat(pd->re, pd->im);
	pn = pn->next;
	pn->iv = pd->iv;
   }
   return(new);
}

DATUM * new_dat(double re, double im) { 	/* malloc a temporary DATUM type */
    mem_use++;
    return (new_dat_perm(re,im));
}

DATUM * new_dat_perm(double re, double im) { 	/* malloc a permanent DATUM type */
    DATUM *p;
    p = (DATUM *) malloc(sizeof(DATUM));
    p -> iv = 0.0;	/* independent variable for list of data points */
    p -> re = re;       /* real part of complex value */
    p -> im = im;	/* imag part of complex value */
    p -> next = (DATUM *) NULL;
    if (debug) printf("creating datum %g %g\n", re, im);
    return (p);
}

void free_dat(DATUM *p) {		/* free a DATUM type */
    DATUM *pd;
    DATUM *ps;

    if (debug) printf("freeing datum %g %g\n", p->re, p->im);
    for (pd=p; pd!=NULL; pd=ps) {
        ps = pd->next;
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
       	   printf("\t    %g,", pd->iv);
	   if (pd->im == 0.0) {
		printf("%g\n",  pd->re); 		/* 8+0i prints as "8" */
	   } else {
		if (pd->re == 0.0) {
		    if (pd->im == 1.0) {
			printf("i\n"); 		/* 1i => "i" */
		    } else {
			printf("%gi\n", pd->im); 	/* 2i => "2i" */
		    }
		} else {
		    if (pd->im == 1.0) {
			printf("%g+i\n", pd->re); 	/* 8+1i => "8+i" */
		    } else {
			printf("%g%+gi\n", pd->re, pd->im); /* 8+2i => "8+2i */
		    }
		}
	   }
       }
       printf("\t}\n"); 
   } else {
	if (p->im == 0.0) {
	    printf("\t%g\n",  p->re); 		/* 8+0i prints as "8" */
	} else {
	    if (p->re == 0.0) {
		if (p->im == 1.0) {
		    printf("\ti\n"); 		/* 1i => "i" */
		} else {
		    printf("\t%gi\n", p->im); 	/* 2i => "2i" */
		}
	    } else {
		if (p->im == 1.0) {
		    printf("\t%g+i\n", p->re); 	/* 8+1i => "8+i" */
		} else {
		    printf("\t%g%+gi\n", p->re, p->im); /* 8+2i => "8+2i */
		}
	    }
	}
    }
}



DATUM * m_plus(DATUM *a, DATUM *b) {	/* add two DATUMs */
    DATUM *tmp;
    DATUM *p;
    DATUM *head;


    if (a->next == NULL && b->next == NULL) {		/* both scalar */
	return(new_dat(a->re + b->re, a->im + b->im));
    } else if (a->next != NULL && b->next != NULL) {	/* both vector */
	return(new_dat(a->re + b->re, a->im + b->im));	/* FIXME: need to do interp here */
    } else {                                            /* mixed */
        if (a->next == NULL) {                          /* make b the scalar */
	    tmp = b;
	    b = a;
	    a = tmp;
	}

        head = new_dat(b->re + a->re, b->im + a->im);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    tmp = new_dat_perm(b->re + p->re, b->im + p->im);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
        }
        return(head);
    }
}

DATUM * m_minus(DATUM *a, DATUM *b) {	/* subtract two DATUMs */
    return(new_dat(a->re - b->re, a->im - b->im));
}

DATUM * m_mult(DATUM *a, DATUM *b) {	/* multiply two DATUMs */
    double real, imag;
    real = (a->re * b->re) - (a->im * b->im);
    imag = (a->re * b->im) + (a->im * b->re);
    return(new_dat(real, imag));
}


DATUM * m_div(DATUM *a, DATUM *b) {	/* divide two DATUMs */
    return(
    	new_dat( 
	    (a->re*b->re + a->im*b->im) / (b->im*b->im + b->re*b->re),
	    (-a->re*b->im + b->re*a->im) / (b->im*b->im + b->re*b->re)
	)
    );
}


DATUM * Pow(DATUM *a, DATUM *b) { 	
    double re1, im1;
    double re2, im2;

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
}



DATUM * Re(DATUM *a) {

    DATUM *p;
    DATUM *head;
    DATUM *tmp;

    if (a->next == NULL) {
	return( new_dat( a->re, 0.0 ) ); 
    } else {
        head = new_dat(a->re, 0.0);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    tmp = new_dat_perm(p->re, 0.0);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}

DATUM * Im(DATUM *a) {

    DATUM *p;
    DATUM *head;
    DATUM *tmp;

    if (a->next == NULL) {
	return( new_dat( a->im, 0.0 ) ); 
    } else {
        head = new_dat(a->im, 0.0);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    tmp = new_dat_perm(p->re, 0.0);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}

DATUM * Pha(DATUM *a) {
    DATUM *p;
    DATUM *head;
    DATUM *tmp;

    if (a->next == NULL) {
	return( new_dat(  atan2(a->im, a->re) , 0.0 ) ); 
    } else {
        head = new_dat( atan2(a->im, a->re), 0.0);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    tmp = new_dat_perm(atan2(p->im, p->re), 0.0);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}

DATUM * Mag(DATUM *a) {
    DATUM *p;
    DATUM *head;
    DATUM *tmp;

    if (a->next == NULL) {
	return( new_dat(sqrt((a->re*a->re + a->im*a->im)), 0.0 ) ); 
    } else {
        head = new_dat( sqrt((a->re*a->re + a->im*a->im))  , 0.0);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    tmp = new_dat_perm( sqrt((p->re*p->re + p->im*p->im)), 0.0);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}

DATUM * Db(DATUM *a) {
    DATUM *p;
    DATUM *head;
    DATUM *tmp;

    if (a->next == NULL) {
	return( new_dat(20.0*log(sqrt((a->re*a->re + a->im*a->im)))/log(10.0), 0.0 ) ); 
    } else {
        head = new_dat(20.0*log(sqrt((a->re*a->re + a->im*a->im)))/log(10.0)  , 0.0);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    tmp = new_dat_perm(20.0*log(sqrt((p->re*p->re + p->im*p->im)))/log(10.0), 0.0);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}

/* RCW (edit mark) */

void dolog(double *re,double *im) {
    double radius;
    double angle;
    
    radius = sqrt(((*re)*(*re) + (*im)*(*im)));
    angle = atan2(*im, *re);

    *re = log(radius);
    *im = angle;
}

DATUM * Log(DATUM *a) {
    DATUM *p;
    DATUM *head;
    DATUM *tmp;
    double re, im;

    if (a->next == NULL) {
	re = a->re; im = a->im; dolog(&re, &im);
	return( new_dat(re, im) ); 
    } else {
	re = a->re; im = a->im; dolog(&re, &im);
        head = new_dat(re, im);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    re = p->re; im = p->im; dolog(&re, &im);
	    tmp = new_dat_perm(re,im);
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

DATUM * Exp(DATUM *a) {		
    DATUM *p;
    DATUM *head;
    DATUM *tmp;
    double re, im;

    if (a->next == NULL) {
	re = a->re; im = a->im; doexp(&re, &im);
	return( new_dat(re, im) ); 
    } else {
	re = a->re; im = a->im; doexp(&re, &im);
        head = new_dat(re, im);
        head->iv = a->iv; 
	for (p=a->next; p!=NULL; p=p->next) {
	    re = p->re; im = p->im; doexp(&re, &im);
	    tmp = new_dat_perm(re,im);
	    tmp->iv = p->iv;
            link_dat(head,tmp);
       }
       return(head);
    }
}
