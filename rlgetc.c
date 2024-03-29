#include "rlgetc.h"
#include <stdlib.h>
#include <unistd.h>
#include <term.h>
#include <readline/readline.h>
#include <readline/history.h>  

#define MAXHIST 1024		/* max history lines to save */
#define HISTORY ".posthist"
#define POSTRC ".postrc"

extern char *getwd();
extern char *xmalloc();
char * stripwhite();
char *lineread = (char *) NULL;
int pushback = '\0';


#include <signal.h>
#include <setjmp.h>
// jmp_buf begin;

char    *rl_gets();

/* *************************************************** */
/* some routines to implement command line history ... */
/* *************************************************** */


int xrlgetc(fd) 
FILE *fd;
{
    int c;
    c=getc(fd);
    /* printf("->%2.2x %c\n",c,c); */
    return (c);
}

int xrl_ungetc(c,fd)
int c;
FILE *fd;
{
    /* printf("ungetting %2.2x %c\n",c,c); */
    return ungetc(c,fd);
}


/* expand and duplicate a string with malloc */

char * expdupstr(char * s,int n)
{
    char *r;

    r = xmalloc(strlen( (char *) s) + n);
    strcpy(r, (char *) s);
    return (r);
}

int rl_ungetc(c,fd) 
int c;
FILE *fd;
{
    /* printf("ungetting %2.2x %c\n",c,c); */

    if (fd != stdin || !isatty(1) || !isatty(0)) {
	return(ungetc(c,fd));
    } else {
	pushback=c;
	return(1);
    }
}


/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
char * rl_gets (prompt)
char *prompt;
{

    char *s;
    int debug=0;

    /* If the buffer has already been allocated, return the memory
       to the free pool. */

    if (lineread) {
        free (lineread);
        lineread = (char *) NULL;
    }

    /* Get a line from the user. */
    if (debug) printf("entering readline\n");
    lineread = readline(prompt);
    if (lineread == NULL) {
	return(NULL);
    }
    fflush(stdout);

    /* If the line has any text in it, save it on the history. */
    if (lineread && *lineread) {
	add_history (lineread);
	write_history(HISTORY);
    }
    
    /* add a newline to return string */

    s = expdupstr(lineread,2);
    strcat(s,"\n");
    free (lineread);
    lineread = s;

    return (lineread);
}

void rl_init()
{
    /* Allow conditional parsing of the ~/.inputrc file. */
    rl_readline_name = POSTRC;
    // stifle_history(MAXHIST);	/* maximum number of history lines */
    read_history(HISTORY);
}


/* note: "rl_getc" is an internal readline function... Do NOT rename */
/* this function from "rlgetc" to "rl_getc" or all hell will break loose */

int rlgetc(fd)
FILE *fd;
{
    int c;
    static char *lp = NULL;

    if (fd != stdin || !isatty(1) || !isatty(0)) {
	c=getc(fd);
    } else {
	if (pushback != '\0') {
	    c=pushback;
	    pushback= '\0';
	} else if(lp != NULL && *lp != '\0') {
	    c=*(lp++);
	} else {
	    if (isatty(1) && isatty(0)) {
		lineread=rl_gets(":");
	    } else {
		lineread=rl_gets("");
	    }
	    if (lineread == NULL) {
		c=EOF;
	    } else {
		lp = lineread;
		c=*(lp++);
	    }
	}
    }
    return (c);
}
