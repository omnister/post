#include <stdlib.h>	/* for malloc() */
#include <stdio.h>

#include "post.h"
#include "y.tab.h"

static Symbol *symlist = 0; /* symbol table: linked list */

Symbol *lookup(s)           /* find s in symbol table */
char *s;
{
    Symbol *sp;

    for (sp=symlist; sp!=(Symbol *)0; sp=sp->next)
        if (strcmp(sp->name, s)==0)
            return sp;
    return 0;               /* 0 ==> not found */
}

Symbol *install(s,t,d)      /* install s in symbol table */
char *s;
int t;
DATUM *d;
{
    Symbol *sp;
    char *emalloc();

    sp = (Symbol *) emalloc(sizeof(Symbol));
    sp->name = emalloc(strlen(s)+1); /* +1 for '\0' */
    strcpy(sp->name, s);
    sp->type = t;
    sp->u.val = d;
    sp->next = symlist; /* put at front of list */
    symlist = sp;
    return sp;
}

void symprint()
{
    Symbol *sp;
    for (sp=symlist; sp!=(Symbol *)0; sp=sp->next)
        if (sp->type == VAR) 
            printf("    %s\n", sp->name);
}

char *emalloc(n)    /* check return from malloc */
unsigned n;
{
    char *p;

    p = (char *) malloc(n);
    if (p == 0)
        execerror("out of memory", (char *) 0);
    return p;
}
