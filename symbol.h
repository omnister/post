#pragma once

#include <string.h>
#include "datum.h"

typedef struct Symbol {     /* symbol table entry */
    char    *name;
    short   type;
    int     simno;		    // simulation number
    union {
        DATUM   *val;       /* VAR */
        DATUM   *(*ptr)();   /* BLTIN */
        int     (*defn)();  /* FUNCTION, PROCEDURE */
        char    *str;       /* STRING */
    } u;
    struct Symbol   *next;  /* to link to another */
} Symbol;

void symprint(void);
Symbol *lookup(char *s, int simno);
Symbol *install(char *s, int t, DATUM *d, int simno);

