#include <math.h>
#include "post.h"
#include "y.tab.h"

extern DATUM * Mag(), *Pha(), *Re(), *Im();
extern void  symprint();

/* keywords */

static struct {	  
    char    *name;
    int	    kval;
} keywords[] = {
    {"i",	I},	/* sqrt(-1) */
    {"I",	I},	/* sqrt(-1) */
    {"gr",	GR},	/* graphnew */
    {"ci",      CI},	/* load in a rawfile */
    {"di",      DI},	/* display loaded sig names */
    {"ls",      LS},	/* list loadable rawfiles in cwd */
    {"pr",      PR},	/* print variable */
    {"xl",      XL},	/* print variable */
    {"yl",      YL},	/* print variable */
    {"quit",    QUIT},	/* quit */
    {"exit",    QUIT},	/* quit */
    {"bye" ,    QUIT},	/* quit */
    {0,		0}
};

/*
    {"func",	FUNC},
    {"return",	RETURN},
    {"if",	IF},
    {"else",	ELSE},
    {"while",	WHILE},
    {"print",	PRINT},
    {"read",	READ},
*/

static struct {	    /* constants */
    char    *name;
    double re;
    double im;
} consts[] = {
    {"PI",	3.14159265358979323846,0.0},
    {"E",	2.71828182845904523536,0.0},
    {"GAMMA",	0.57721566490153286060,0.0}, /* Euler */
    {"DEG",     57.29577951308232087680,0.0},/* deg/radian */
    {"PHI",	1.61803398874989484820,0.0}, /* golden ratio */
    {"DT",	1e-6,0.0}, 		     /* default integration delta */
    {0,		0.0, 0.0}
};

// need sin() cos() atan() int() abs() conj()

static struct {	    /* built-ins */
    char    *name;
    DATUM   *(*func)();
} builtins[] = {
    {"avg",	   Avg},	/* binop */
    {"db",	   Db},
    {"dt", 	   dt},
    {"exp",	   Exp},
    {"im",	   Im},
    {"integral",  Integral},
    {"pha",	   Pha},
    {"ln",	   Ln},
    {"log10",	   Log10},
    {"log",	   Log10},
    {"re",	   Re},
    {"mag",	   Mag},	/* binop */
    {"min", 	   Min},	/* binop */
    {"max", 	   Max},	/* binop */
    {"pause", 	   dopause},	
    {"pow", 	   Pow},	/* binop */
    {"sqrt", 	   Sqrt},	/* binop */
    {"warp", 	   Warp},	/* binop */
    {"delay", 	   Warp},	/* binop */
    {"xcross", 	   xcross},	/* binop */
    {"xcrossn",    xcrossn},	/* binop */
    {"xcrossp",    xcrossp},	/* binop */
    {0,		   0}
};

void init()	/* install constants and built-ins in table */
{
    int i;

    Symbol *s;
    
    for (i=0; keywords[i].name; i++) {
	install(keywords[i].name, keywords[i].kval, 0.0);
    }

    for (i=0; builtins[i].name; i++) {
	s = install(builtins[i].name, BLTIN, NULL);
	s->u.ptr = builtins[i].func;
    }

    for (i=0; consts[i].name; i++) {
	install(consts[i].name, VAR, new_dat(consts[i].re, consts[i].im));
    }
}
