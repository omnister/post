#include "script.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAXSIGNAL 31	/* biggest signal under linux */

static FILE *rcvfp;
static FILE *sendfp = NULL;
static int sendfd;
static int pid;

void syserr(char *msg) {
    fprintf(stderr, "ERROR: %s (%s)\n", msg, strerror(errno));
    exit(1);
}

void childsig(x)
int x;
{
    extern char *strsignal();
    fprintf(stderr,"child caught (%d) %s\n", x, strsignal(x)); 
}

int scriptopen(char* prog, char *arg1, char *arg2)
{
    int pfdout[2], pfdin[2];
    char *msg;
    int err=0;

    /*
    if (scriptfeed("reset \n")) {
       printf("running\n");
       return (1);
    } else {
       printf("starting\n");
    }
    */

    do {				/* context for error checking */
	if(pipe(pfdout) == -1) { err++; msg="pipe 1"; break; };
	if(pipe(pfdin) == -1)  { err++; msg="pipe 2"; break; };
    } while(0); if (err) return(0);

    switch(pid=fork()) {			/* split off into parent-child */
    case -1: /* error */
        syserr("fork");
        break;
    case 0:  /* in child */				
	do {				/* context for error checking */
	    if (close(0) == -1)          {err++; msg="close 3" ; break;};
	    if (dup(pfdout[0]) != 0)     {err++; msg="dup 4"   ; break;};
	    if (close(1) == -1)          {err++; msg="close 5" ; break;};
	    if (dup(pfdin[1]) != 1)      {err++; msg="dup 6"   ; break;};
	    if (close(pfdout[0]) == -1)  {err++; msg="close 7" ; break;};
	    if (close(pfdout[1]) == -1)  {err++; msg="close 8" ; break;};
	    if (close(pfdin[0]) == -1)   {err++; msg="close 9" ; break;};
	    if (close(pfdin[1]) == -1)   {err++; msg="close 10"; break;};

	    execlp(prog, prog, arg1, arg2, NULL); /* thus passes control to child */
	    err++; msg="execlp"; break;	/* should only get here if execlp fails */

	} while (0); if (err) return(0);
	break;
    }

    /* from here on down we are in parent */
    /* printf("parent: child pid = %d\n", pid); */

    do {				/* context for error checking */
     	if (close(pfdout[0]) == -1)  {err++; msg="close 11" ; break;};
	if (close(pfdin[1]) == -1)   {err++; msg="close 12" ; break;};
	sendfd=pfdout[1];
    } while (0); 
    

    if ((rcvfp=fdopen(pfdin[0],"r")) == NULL) {err++; msg="fdopen rcvfp";};
    if ((sendfp=fdopen(pfdout[1], "w")) == NULL) {err++; msg="fdopen sendfp";};

    if (err) return(0);

    return(1);	/* success */
}

void onalarm() 	/* kill child process on alarm */
{
    fprintf(stderr, "child killed\n");
    kill(pid, SIGQUIT); 
    kill(pid, SIGKILL); 
}

int scriptclose() {
    /* return(0); */
    close(sendfd);	        /* send EOF */
    sendfp = NULL;
    return(1);
}

int scriptread(char *buf, int n, int timeout) {
    int status;

    close(sendfd);	/* send EOF */

    if (timeout != 0) {
	signal(SIGALRM, onalarm); 
	alarm(timeout);
    }

    if (wait(&status) == -1 || (status & 0177) != 0) {
       fprintf(stderr,"killed subprocess\n");
       return(0);
    } else {
       fgets(buf, n, rcvfp);
       kill(pid, SIGKILL); 	/* not interested in more output */
       return(1);
    }
}

int scriptfeed(char *msg) {

    if (sendfp == NULL) {
        return(0);
    }

    if (fprintf(sendfp, "%s",msg) < 0) {
        return(0);
    } else {
	fflush(sendfp);
	return (1);
    }
}
