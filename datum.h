
typedef struct complex {
    double iv; 		/* independent variable for list of data points */
    double re; 		/* real part of complex value */
    double im; 		/* imag part of complex value */
    char *def;		/* textual equivalent */
    struct complex *next;
    struct complex *prev;
} DATUM;

typedef enum {ADD, AVG, DIV, MAX, MIN, MULT, POW, SUB, WARP, LAM} BINOP;

void free_dat(DATUM *p); 
void print_dat(DATUM *p);
void plot_dat(DATUM *p, int mode);	/* mode, 0=graph same, 1=graph new */
int  dat_stat();
DATUM * new_dat();		/* alloc a temporary variable */
DATUM * new_dat_perm();		/* alloc a permanent variable */
DATUM * dup_dat(DATUM *p);	/* copy a DATUM */
DATUM * link_dat();
DATUM * interp();

DATUM * dopause();

/* binaries */

DATUM *  binary(BINOP operation, DATUM *a, DATUM *b);
DATUM *  Pow(DATUM *a, DATUM *b);
DATUM *  Max(DATUM *a, DATUM *b);
DATUM *  Min(DATUM *a, DATUM *b);
DATUM *  Avg(DATUM *a, DATUM *b);
DATUM *  Warp(DATUM *a, DATUM *b);
DATUM *  xcross(DATUM *a, DATUM *b);
DATUM *  xcrossp(DATUM *a, DATUM *b);
DATUM *  xcrossn(DATUM *a, DATUM *b);

/* unaries -- second arg is for error checking */
/* should be passed as NULL */

DATUM *       dt(DATUM *a, DATUM *b);
DATUM *       Db(DATUM *a, DATUM *b);
DATUM *      Exp(DATUM *a, DATUM *b);
DATUM *       Im(DATUM *a, DATUM *b);
DATUM *      Mag(DATUM *a, DATUM *b);
DATUM *      Pha(DATUM *a, DATUM *b);
DATUM *       Re(DATUM *a, DATUM *b);
DATUM *       Ln(DATUM *a, DATUM *b);
DATUM *    Log10(DATUM *a, DATUM *b);
DATUM *     Sqrt(DATUM *a, DATUM *b);
DATUM * Integral(DATUM *a, DATUM *b);

