#include "rlgetc.h"
#include <stdlib.h>
#include <term.h>
#include <readline/readline.h>
#include <readline/history.h>  

extern char *getwd();
extern char *xmalloc();
char * stripwhite();
char *lineread = (char *) NULL;
int pushback = (char) NULL;

#include <signal.h>
#include <setjmp.h>
jmp_buf begin;

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

char * expdupstr(s,n)
int s, n;
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

    if (fd != stdin) {
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
    lineread = readline (prompt);
    if (lineread == NULL) {
	return(NULL);
    }
    fflush(stdout);

    /* If the line has any text in it, save it on the history. */
    if (lineread && *lineread)
	add_history (lineread);
    
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
    rl_readline_name = ".postrc";
}


static int savestat = 0;
static char savebuf[128];
static char *pbuf = savebuf;

void rl_save_start() {
   savestat++;
   pbuf=savebuf;
}

char *rl_save_end() {
   savestat=0;
   *pbuf='\0';
   printf("<%s>\n", savebuf);
   return(savebuf);
}

/* note: "rl_getc" is an internal readline function... Do NOT rename */
/* this function from "rlgetc" to "rl_getc" or all hell will break loose */

int rlgetc(fd)
FILE *fd;
{
    int c;
    static char *lp = NULL;

    if (fd != stdin) {
	c=getc(fd);
    } else {
	if (pushback != (char) NULL) {
	    c=pushback;
	    pushback=(char) NULL;
	} else if(lp != NULL && *lp != '\0') {
	    c=*(lp++);
	} else {
	    lineread=rl_gets(":");
	    if (lineread == NULL) {
		c=EOF;
	    } else {
		lp = lineread;
		c=*(lp++);
	    }
	}
    }

    if (savestat) {
        *pbuf=c;
	pbuf++;
    }
    return (c);
}
