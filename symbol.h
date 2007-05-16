#include <string.h>

typedef struct Symbol {     /* symbol table entry */
    char    *name;
    short   type;
    union {
        DATUM   *val;       /* VAR */
        DATUM   *(*ptr)();   /* BLTIN */
        int     (*defn)();  /* FUNCTION, PROCEDURE */
        char    *str;       /* STRING */
    } u;
    struct Symbol   *next;  /* to link to another */
} Symbol;

Symbol  *install(), *lookup();
void symprint();
