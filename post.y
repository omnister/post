/* complex waveform calculator */
/* need to add back yvalue() */

%{

#include <stdlib.h>
#include "post.h"
#include "script.h"
#define YYDEBUG 1

char buf[128];
int     bflag=0;
int 	gnuplot=0;
int 	yylex();
void stringupdate(DATUM *d, char *b);
void yyerror(char *s);
int comment(void);

%}

%union {
    DATUM  *y_datum;	/* pointer to a complex number type */
    double  y_num;	/* a double precision number */
    Symbol *y_sym;	/* pointer to Symbol Table */
}

%token <y_sym>   I
%token <y_num>   NUMBER
%token <y_sym>   VAR BLTIN UNDEF STRING
%token <y_sym>   GR
%token <y_sym>   GS
%token <y_sym>   XL
%token <y_sym>   YL
%token <y_sym>   LS 
%token <y_sym>   LX 
%token <y_sym>   VS
%token <y_sym>   PR
%token <y_sym>   CI
%token <y_sym>   DI 
%token <y_sym>   QUIT 
%type  <y_datum> expr
%type  <y_datum> asgn
%type  <y_datum> pr 
%type  <y_datum> coord
%type  <y_datum> coord_list
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc UNARYMINUS UNARYPLUS
%right '^'	/* exponentiation */
%%

list:	/* empty */
	| list eos
	| list pr eos {
	    }
	| list asgn eos { 
		free_dat($2);
	    }
	| list expr eos { 
		if (!bflag) {
		    printf("\t");
		    print_dat($2);
		    printf("\n");
		}
		free_dat($2);
	    }
	| list gr eos {
	    }
	| list gs eos {
	    }
	| list LS eos {
		ls();
	    }
	| list CI STRING eos {
		if (!bflag) printf("loading %s\n", (char *) $3);
		com_ci((char *) $3);
	    }
	| list QUIT eos {
		exit(1);
	    }
	| list DI eos {
		// printf("background flag=%d\n", bflag);
		if (!bflag) symprint();
	    }
	| list error eos {
		yyclearin;
		yyerrok; 
	    }
	;

/****************************************/

gr: 	     GR {graphinit();} plotlist { 
		if (!gnuplot) {
		    graphprint_pd(0);	// default is to use pdplot
		} else {
		    graphprint_gnu(0);	// plot with gnuplot
		}
	     };
gs: 	     GS {graphinit();} plotlist { 
		if (!gnuplot) {
		    graphprint_pd(1);	// default is to use pdplot
		} else {
		    graphprint_gnu(1);	// plot with gnuplot
		}
	     };


plotlist:  plotspec 
	   | plotlist plotsep plotspec; 
	   ;

plotsep:  ';' {
		graphnext();
             };

plotspec:  expr {
	   	graphexpr($1);
		free_dat($1);
	      };
   	   | plotspec ',' expr {
	   	graphexpr($3);
		free_dat($3);
	      };
   	   | plotspec xl;
	   | plotspec yl;
	   | plotspec lx;
	   | plotspec vs;
	   ;

lx: 	   LX {
	       graphlogx();
	   };

vs:	   VS expr {
                graphversus($2);
		free_dat($2);
           };

xl:	   XL expr ',' expr {
		graphxl($2->re, $4->re);
	        free_dat($2);
	        free_dat($4);
             };
yl:	   YL expr ',' expr { 
		graphyl($2->re, $4->re);
	        free_dat($2);
	        free_dat($4);
	     };

/****************************************/

pr:	 PR printlist {printf("\n");};

printlist: expr { 
	   	print_dat($1); 
		free_dat($1);
              };
	   | STRING { 
	       printf("%s", (char *) $1); 
	      };
	   | printlist expr {
	   	print_dat($2); 
		free_dat($2);
	       };
	   | printlist STRING { 
	       printf("%s", (char *) $2); 
	      };
	   | printlist ',' expr { 
		printf(" ");
	   	print_dat($3); 
		free_dat($3);
	      };
	   | printlist ',' STRING {
		printf(" ");
	       printf("%s", (char *) $3); 
	      };

//PR STRING { 
//		printf("%s\n", (char *) $2);
//	    }
//	| PR expr {
//		print_dat($2);
//		free_dat($2);
//           }


asgn:   VAR '=' expr    { 
		free($1->u.val);	/* remove old value */
		$1->u.val = $3; 	/* install new value */
		$1->type = VAR;
		$$ = dup_dat($3);	/* put a copy on stack */
	    }
        ;


expr:	NUMBER { 
		$$ = new_dat((double) $1,0.0); 
		sprintf(buf, "%g", $1);
		stringupdate($$, strsave(buf));
	    }
	| NUMBER I {
		$$ = new_dat(0.0,(double) $1);
		sprintf(buf, "%gI", $1);
		stringupdate($$, strsave(buf));
	    } 
	| '{' eos coord_list '}' { $$ = $3; }
	| I             { $$ = new_dat(0.0, 1.0); };
	| VAR { 
		if ($1->type == UNDEF) {
		    // execerror("undefined variable", $1->name);
		    // just create a zero variable
		    $$ = new_dat(0.0, 0.0);
		} else {
		    $$ = dup_dat($1->u.val);
		}
		sprintf(buf, "%s", $1->name);
		stringupdate($$, strsave(buf));
	    }
        | VAR '(' expr ')' {
		if ($1->type == UNDEF) 
		    execerror("undefined function", $1->name);

	        $$ = interp($1->u.val, $3);
	        sprintf(buf, "%s(%g)", $1->name, $3->re);
		stringupdate($$, strsave(buf));
	        free($3);
	    }
	| BLTIN '(' expr ',' expr ')' { 
	        $$ = (*($1->u.ptr))($3,$5); 
		sprintf(buf, "%s(%s,%s)", $1->name, $3->def, $5->def);
		stringupdate($$, strsave(buf));
	        free_dat($3); 
	        free_dat($5); 
	    }
	| BLTIN '(' expr ')' {
	        $$ = (*($1->u.ptr))($3,NULL); 
		sprintf(buf, "%s(%s)", $1->name, $3->def);
		stringupdate($$, strsave(buf));
	        free_dat($3); 
	   }
	| '+' expr %prec UNARYPLUS {
	        $$ = $2;
	   }
	| '-' expr %prec UNARYMINUS {
		DATUM *pp;
		pp = new_dat(0.0, 0.0);
		$$ = binary(SUB,pp,$2); 
		sprintf(buf, "-%s", $2->def);
		stringupdate($$, strsave(buf));
		free_dat($2);
		free_dat(pp);
	    }
        | expr '+' expr { 
		$$ = binary(ADD, $1,$3);
		sprintf(buf, "%s+%s", $1->def, $3->def);
		stringupdate($$, strsave(buf));
		free_dat($1);
		free_dat($3);
	    }
	| expr '-' expr {
		$$ = binary(SUB,$1,$3); 
		sprintf(buf, "%s-%s", $1->def, $3->def);
		stringupdate($$, strsave(buf));
		free_dat($1);
		free_dat($3);
	    }
	| expr '*' expr { 
		$$ = binary(MULT,$1,$3); 
		sprintf(buf, "%s*%s", $1->def, $3->def);
		stringupdate($$, strsave(buf));
		free_dat($1);
		free_dat($3);
	    }
	| expr '/' expr { 
		if ($3->re == 0.0 && $3->im == 0.0) {
		    /* clean up the stack and abort*/
		    free_dat($1);
		    free_dat($3);
		    execerror("division by zero", "");
		}
		$$ = binary(DIV,$1,$3); 
		sprintf(buf, "%s/%s", $1->def, $3->def);
		stringupdate($$, strsave(buf));
		free_dat($1);
		free_dat($3);
	    }
	| expr '^' expr { 
		$$ = binary(POW, $1, $3);
		sprintf(buf, "%s^%s", $1->def, $3->def);
		stringupdate($$, strsave(buf));
		free_dat($1);
		free_dat($3);
	    }
	| '(' expr ')' { 
		$$ = $2; 
		sprintf(buf, "(%s)", $2->def);
		stringupdate($$, strsave(buf));
	    }
	;

coord	:  expr ',' expr {
		$3->iv = $1->re;
		$$ = $3;
	    }
	|  expr ',' expr '\n' {
		$3->iv = $1->re;
		$$ = $3;
	    }
	;

coord_list: coord ';' coord {
		$$ = link_dat($1,$3);
	    }
	| coord ';' eos coord {
		$$ = link_dat($1,$4);
	    }
	|   coord_list ';' coord {
		$$ = link_dat($1,$3);
	    }
	|   coord_list ';' eos coord {
		$$ = link_dat($1,$4);
	    }
	;

eos:    /* EMPTY */
	| '\n'
        ;
%%

#define _GNU_SOURCE
#include <string.h>	/* for strsignal() */
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "rlgetc.h"
#include <signal.h>

#define MAXSIGNAL 31	/* biggest signal under linux */
#include <setjmp.h>
jmp_buf begin;

char 	*progname;
int 	lineno = 1;
jmp_buf begin;
int	indef;
char	*infile;    /* input file name */
FILE	*fin;	    /* input file pointer */
char	**gargv;    /* global argument list */
int	gargc;
double  getdouble();

int	c;	    /* global for use by warning() */
int     yyparse();
		
void stringupdate(DATUM *d, char *b) {
   if (d!=NULL && d->def != NULL ) {
      free(d->def);
   } 
   if (d!=NULL) {
       d->def = b;
   }
}

void sighandler(x)
int x;
{
    static int last=-1;
    extern char *strsignal();

    /* printf("caught %s. Use \"quit\" to end program", strsignal(x)); */

    if (x == 3) {
       if (last==x) {
            exit(0);
       } else {
            if (!bflag) printf(": do it again and I'll die!\n");
       }
    }
    last = x;
}

char *strsave(s)   /* save string s somewhere */
char *s;
{
    char *p;

    if (s == NULL) {
        return(s);
    }

    if ((p = (char *) malloc(strlen(s)+1)) != NULL)
	strcpy(p,s);
    return(p);
}

int main(int argc, char *argv[])    /* hoc 6 */
{
    void fpecatch();
    void run();
    void sighandler();
    int moreinput();
    int err=0;
    int opt;

    extern int yydebug;
    yydebug = 0;

    rl_init();	/* Bind our completer. */

    progname = argv[0];
    
    while ((opt=getopt(argc, argv, "gr:")) != -1) {
        switch (opt) {
	case 'g':
	   gnuplot++;
	   break;
	case 'r':
	   com_ci(optarg); // open rawfile 
	   break;
	default:	/* '?' */
	   fprintf(stderr, "usage: %s [-g (use gnuplot)] [-r <rawfile>] <script>\n", argv[0]);
	   exit(1);
	}
    }

    if (optind >= argc) {	/* fake an argument list */
	static char *stdinonly[] = { "-" };
	gargv = stdinonly;
	gargc = 1;
    } else {
	gargv = &argv[optind];
	gargc = --argc;
    }

    /* set up to catch all signal */
    /* for (i=1; i<=MAXSIGNAL; i++) { */

    err+=(signal(2, &sighandler) == SIG_ERR);		/* SIGINT */
    err+=(signal(3, &sighandler) == SIG_ERR);		/* SIGQUIT */
    err+=(signal(15, &sighandler) == SIG_ERR);		/* SIGTERM */
    err+=(signal(20, &sighandler) == SIG_ERR);		/* SIGSTP */
    if (err) {
    	printf("main() had difficulty setting sighandler\n");
	return(err);
    }

    // check if we are interactive or in the background
    // if (signal(SIGINT, SIG_IGN) == SIG_IGN ) { bflag = 1; }

    init();
    while (moreinput()) {
	// if (fin==stdin) license();
	if (fin==stdin && isatty(1) && isatty(0) ) license();
	run();
    }
    return 0;
}

int moreinput()
{
    bflag=0;
    if (gargc-- <= 0)
	return 0;
    if (fin && fin != stdin) {
	fclose(fin);
    }
    infile = *gargv++;
    lineno = 1;
    if (strcmp(infile, "-") == 0) {
	bflag=0;
	fin = stdin;
	infile = 0;
    } else if (( fin=fopen(infile, "r")) == NULL) {
	fprintf(stderr, "%s: can't open %s\n", progname, infile);
	return moreinput();
    } 

    // FIXME: think this through better...
    if (!isatty(0) && !isatty(1)) bflag++;

    return 1;
}

void execerror(s,t)	/* recover from run-time error */
char *s, *t;
{
    void warning();
    warning(s,t);
    fseek(fin, 0L, 2);	/* flush rest of file */
    longjmp(begin, 0);
}

void fpecatch()  /* catch floating point exceptions */
{
    execerror("floating point exception", (char *) 0);
}

void run()	/* execute until EOF */
{
    int yyparse();

    setjmp(begin);
    signal(SIGFPE, fpecatch);
    yyparse();
    /* for (initcode(); yyparse(); initcode())
	execute(progbase);
    */
}

void warning(s,t)	/* print warning message */
char *s, *t;
{
    fprintf(stderr, "%s: %s", progname, s);
    if (t)
	fprintf(stderr, " %s", t);
    if (infile)
	fprintf(stderr, " in %s", infile);
    fprintf(stderr," near line %d\n", lineno);
    while (c != '\n' && c!= EOF)
	c = rlgetc(fin);	/* flush rest of input line */
    if(c=='\n')
	lineno++;
}


int backslash(c)	/* get next char with \'s interpreted */
int c;
{
    char *strchr();
    static char transtab[] = "b\bf\fn\nr\rt\t";
    if (c != '\\')
	return c;
    c = rlgetc(fin);
    if (islower(c) && strchr(transtab, c))
	return strchr(transtab, c)[1];
    return c;
}

int yylex()
{
    int c;
    int rval;

    while ((c=rlgetc(fin)) == ' ' || c == '\t') {
    	;
    }

    if (c == '\n')
    	lineno++;

    if (c == EOF) {
	rval=0;
    } else if (c == '.'  || isdigit(c)) {	/* number */
	double d;
	rl_ungetc(c,fin);

	d = getdouble(fin); 

	/************ Engineering Suffixes Notation <RCW> 10/1/93 **********/
	/************ added percent 11/12/06 *******************************/
	switch(c=rlgetc(fin)) {
	    case 'A':
	    case 'a': d*=1e-18; break;
	    case 'F':
	    case 'f': d*=1e-15; break;
	    case 'P':
	    case 'p': d*=1e-12; break;
	    case 'N':
	    case 'n': d*=1e-9;  break;
	    case 'U':
	    case 'u': d*=1e-6;  break;
	    case '%': d/=100.0; break;
	    case 'M':
	    case 'm': 
		if((c=rlgetc(fin)) == 'e' || c == 'E') {
		    if((c=rlgetc(fin)) == 'g' || c == 'G') {
			d*=1e6;
		    } else {
			execerror("bad engineering notation", (char *)0);
		    }		
		} else {
		    rl_ungetc(c,fin);
		    d*=1e-3; 
		}
		break;
	    case 'K':
	    case 'k': d*=1e3;  break;
	    case 'G':
	    case 'g': d*=1e9;  break;
	    case 'T':
	    case 't': d*=1e12;  break;
	    default: rl_ungetc(c,fin);
	}
	/************************************************************/
	yylval.y_num = d; 
	rval=NUMBER;
    } else if (isalpha(c) || c=='_') {
        Symbol *s;
	char sbuf[1024], *p = sbuf;
	do {
	    *p++ = c;
	} while ((c=rlgetc(fin)) != EOF && (isalnum(c) || 
		c=='_' || 
		c=='<' ||
		c=='.' ||
		c=='>' ));	/* change for Tom's cadence deck varnames */

	rl_ungetc(c,fin);
	*p = '\0';

	if ((s=lookup(sbuf)) == 0) {
	    s = install(sbuf, UNDEF, new_dat(0.0, 0.0));
	}

	yylval.y_sym=s;
	rval = s->type == UNDEF ? VAR : s->type;
    } else if (c == '/') {	// c style comment
        rval = comment(); 
    } else if (c == '#') {	// # style comment
	do {
	    c=rlgetc(fin);
	} while (c!='\n');
	// return(yylex());
	rval='\n';
    } else if ( c== '"') {	/* quoted string */
	char sbuf[1024], *p, *emalloc();
        Symbol *s;
	for (p = sbuf; (c=rlgetc(fin)) != '"'; p++) {
	    if (c == '\n' || c == EOF)
		execerror("missing quote", "");
	    if (p >= sbuf + sizeof(sbuf)-1) {
		*p = '\0';
		execerror("string too long", sbuf);
	    }
	    *p = backslash(c);
	}
	*p = 0;
	if ((s=lookup(sbuf)) == 0) {
	    yylval.y_sym = (Symbol *)emalloc(strlen(sbuf)+1);
	    strcpy((char *) yylval.y_sym, sbuf);
	    rval = STRING;
	} else {
	    yylval.y_sym=s;
	    rval = s->type == UNDEF ? VAR : s->type;
	}
    } else {
        rval = c;
    }

    return rval;
}

int comment()	/* strip out a potential comment */
{
    int c = rlgetc(fin);
    int done = 0;

    if (c != '*') { /* return if not in a comment */
	rl_ungetc(c,fin);
	return '/';
    }

    do {
	if((c=rlgetc(fin)) == '*') {
	    if((c=rlgetc(fin)) == '/') {
		done++;
	    } else {
		rl_ungetc(c,fin);
	    }
	}
    } while (done==0);
    return(yylex());
}

void yyerror(char *s)	/* report compile-time error */
{
    void warning();
    warning(s, (char *)0);
}


/* parse an input of the form [0-9]*.[0-9]*[eE][+-][0-9]* */
/* using just rlgetc(fin) and rl_ungetc(c,fin) */

double getdouble(fin) 	
FILE *fin;
{
    int c;
    int state=0;
    int done=0;
    double val=0.0;
    double frac=1.0;
    double expsign=1.0;
    double exp=0.0;
    
    state=0;

    while (!done) {
	c = rlgetc(fin);
	switch (state) {
	    case 0: 	/* integer part */
		if (isdigit(c)) {
		    val=10.0*val+(double) (c-'0');	
		} else if (c=='.') {
		    state=1;
		} else if (c=='e' || c=='E') {
		    state=2;
		} else {
		    rl_ungetc(c,fin);
		    done++;
		}
		break;
	    case 1:	/* decimal part */
		if (isdigit(c)) {
		    frac /= 10.0;
		    val=val+((double) (c-'0'))*frac;
		} else if (c=='e' || c=='E') {
		    state=2;
		} else {
		    rl_ungetc(c,fin);
		    done++;
		}
		break;
	    case 2:	/* exponent sign */
		if (c=='+') {
		    state=3;
		} else if (c=='-') {
		    expsign=-1.0;
		    state=3;
		} else if (isdigit(c)) {
		    rl_ungetc(c,fin);
		    state=3;
		} else {
		    rl_ungetc(c,fin);
		    done++;
		}
		break;
	    case 3:	/* exponent value */
		if (isdigit(c)) {
		    exp=10.0*exp+(double) (c-'0');	
		} else {
		    rl_ungetc(c,fin);
		    done++;
		}
		break;
	}
/*	printf("state=%d, c=%c, val=%g, frac=%g exp=%e expsign=%e, pow=%e\n",
	    state,c,val,frac,exp,expsign, pow(10.0,exp*expsign)); */
    }

    return(val*pow(10.0,exp*expsign));
}

