
/* complex waveform calculator */

%{

#include "post.h"
#include "script.h"
#define YYDEBUG 1

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
%token <y_sym>   GN
%token <y_sym>   XL
%token <y_sym>   YL
%token <y_sym>   LS 
%token <y_sym>   PR
%token <y_sym>   CI
%token <y_sym>   DI 
%token <y_sym>   QUIT 
%type  <y_datum> expr
%type  <y_datum> asgn
%type  <y_datum> fxn
%type  <y_datum> gs
%type  <y_datum> gn
%type  <y_datum> pr 
%type  <y_datum> coord
%type  <y_datum> coord_list
%right '='
%left '+' '-'
%left '*' '/'
%left UNARYMINUS UNARYPLUS
%right '^'	/* exponentiation */
%%

list:	/* empty */
	| list eos
	| list pr eos {
		print_dat($2);
		free_dat($2);
	    }
	| list asgn eos { 
		free_dat($2);
	    }
	| list expr eos { 
		print_dat($2);
		free_dat($2);
	    }
	| list gr eos {
	    }
	| list LS eos {
		ls();
	    }
	| list CI STRING eos {
		printf("loading %s\n", (char *) $3);
		com_ci((char *) $3);
	    }
	| list QUIT eos {
		exit(1);
	    }
	| list DI eos {
		symprint();
	    }
	| list error eos {
		yyclearin;
		yyerrok; 
	    }
	;

/****************************************/

gr: 	   XX plotlist { 
		graphprint(0);
	     };

XX:        GR {
                graphinit();
             };

plotlist:  plotspec {};
	   | plotlist plotsep plotspec {};
	   ;

plotsep:  ':' {
		graphnext();
             };

plotspec:  /* empty */ { };
   	   | plotspec expr {
	   	graphexpr($2);
		free_dat($2);
	      };
   	   | plotspec xl;
	   | plotspec yl;
	   ;

xl:	   XL expr expr {
		graphxl($2->re, $3->re);
	        free_dat($2);
	        free_dat($3);
             };
yl:	   YL expr expr { 
		graphyl($2->re, $3->re);
	        free_dat($2);
	        free_dat($3);
	     };

/****************************************/

pr:	PR expr	{ $$ = $2; }

gs:	GS expr	{ $$ = $2; }

gn:	GN expr	{ $$ = $2; }

fxn:    VAR '(' expr ')' {
	      $$ = interp($1->u.val, $3);
	      free($3);
	    }

asgn:   VAR '=' expr    { 
		free($1->u.val);	/* remove old value */
		$1->u.val = $3; 	/* install new value */
		$1->type = VAR;
		$$ = dup_dat($3);	/* put a copy on stack */
	    }
        ;

expr:	NUMBER 		{ $$ = new_dat((double) $1,0.0); }
	| NUMBER I      { $$ = new_dat(0.0,(double) $1); };
	| '{' coord_list '}' { $$ = $2; }
	| I             { $$ = new_dat(0.0, 1.0); };
	| VAR 		{ if ($1->type == UNDEF) 
	                     execerror("undefined variable", $1->name);
			  $$ = dup_dat($1->u.val);
			}
	| fxn
	| BLTIN '(' expr ',' expr ')' { $$ = (*($1->u.ptr))($3,$5); 
	                       free_dat($3); }
	| BLTIN '(' expr ')'          { $$ = (*($1->u.ptr))($3,NULL); 
	                       free_dat($3); }
	| '+' expr %prec UNARYPLUS {
	       $$ = $2;
	   }
	| '-' expr %prec UNARYMINUS {
		DATUM *pp;
		pp = new_dat(0.0, 0.0);
		$$ = binary(SUB,pp,$2); 
		free_dat($2);
		free_dat(pp);
	    }
        | expr '+' expr { 
		    $$ = binary(ADD, $1,$3);
		    free_dat($1);
		    free_dat($3);
		}
	| expr '-' expr {
		    $$ = binary(SUB,$1,$3); 
		    free_dat($1);
		    free_dat($3);
		}
	| expr '*' expr { 
		    $$ = binary(MULT,$1,$3); 
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
		    free_dat($1);
		    free_dat($3);
		}
	| expr '^' expr { 
		    $$ = binary(POW, $1, $3);
		    free_dat($1);
		    free_dat($3);
	            }
	| '(' expr ')' { $$ = $2; }
	;


coord	:  expr ',' expr {
		$3->iv = $1->re;
		$$ = $3;
		}
	;

coord_list: coord  coord {
		$$ = link_dat($1,$2);
		}
	|	coord_list coord {
		$$ = link_dat($1,$2);
	}
		;
eos:    '\n'
        | ';'
        ;
%%

#define _GNU_SOURCE
#include <string.h>	/* for strsignal() */
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include "rlgetc.h"
#include <signal.h>

#define MAXSIGNAL 31	/* biggest signal under linux */
#include <setjmp.h>
jmp_buf begin;

char 	*progname;
int 	lineno = 1;
int yylex();
jmp_buf begin;
int	indef;
char	*infile;    /* input file name */
FILE	*fin;	    /* input file pointer */
char	**gargv;    /* global argument list */
int	gargc;
double  getdouble();

int	c;	    /* global for use by warning() */
int     yyparse();

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
            printf(": do it again and I'll die!\n");
       }
    }
    last = x;
}

int main(argc, argv)    /* hoc 6 */
char *argv[];
{
    void fpecatch();
    void run();
    void sighandler();
    int moreinput();
    int err=0;

    extern int yydebug;
    yydebug = 0;

    rl_init();	/* Bind our completer. */

    progname = argv[0];
    if (argc == 1) {	/* fake an argument list */
	static char *stdinonly[] = { "-" };
	gargv = stdinonly;
	gargc = 1;
    } else {
	gargv = argv+1;
	gargc = argc-1;
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

    init();
    while (moreinput()) {
	run();
    }
    return 0;

}

int moreinput()
{
    if (gargc-- <= 0)
	return 0;
    if (fin && fin != stdin)
	fclose(fin);
    infile = *gargv++;
    lineno = 1;
    if (strcmp(infile, "-") == 0) {
	fin = stdin;
	infile = 0;
    } else if (( fin=fopen(infile, "r")) == NULL) {
	fprintf(stderr, "%s: can't open %s\n", progname, infile);
	return moreinput();
    }
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


int comment()	/* strip out a potential comment */
{
    int yylex();
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

int yylex()
{
    int c;

    while ((c=rlgetc(fin)) == ' ' || c == '\t') {
    	;
    }

    if (c == EOF)
    	return 0;

    if (c == '.'  || isdigit(c)) {	/* number */
	double d;
	rl_ungetc(c,fin);

	/* printf("calling getdouble()..."); */
	d = getdouble(fin); 
	/* printf("returned with %e\n",d); */

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
	return NUMBER;
    }

    if (isalpha(c)) {
        Symbol *s;
	char sbuf[100], *p = sbuf;
	do {
	    *p++ = c;
	} while ((c=rlgetc(fin)) != EOF && isalnum(c));

	rl_ungetc(c,fin);
	*p = '\0';

	if ((s=lookup(sbuf)) == 0) {
	    s = install(sbuf, UNDEF, new_dat(0.0, 0.0));
	}

	yylval.y_sym=s;
	return s->type == UNDEF ? VAR : s->type;
    }

    if (c == '\n')
    	lineno++;

    if (c == '/') return(comment());

    if ( c== '"') {	/* quoted string */
	char sbuf[100], *p, *emalloc();
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
	yylval.y_sym = (Symbol *)emalloc(strlen(sbuf)+1);
	strcpy((char *) yylval.y_sym, sbuf);
	return STRING;
    }

    return c;
}


void yyerror(s)	/* report compile-time error */
char *s;
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

