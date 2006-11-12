#include <stdlib.h>	/* malloc() */
#include <stdio.h>
#include <string.h>

void g_free(void *ptr) {
    free(ptr);
}

/* estrdup: duplicate a string, report if error */
char *g_strdup(char *s)
{
    char *t;

    t = (char *) malloc(strlen(s)+1);
    if (t == NULL) 
	printf("g_strdup(\"%.20s\") failed:", s);
    strcpy(t,s);
    return(t);
}

/* g_new0: calloc and report if error */
void *g_new0(size_t n, int count)
{
    void *p;

    p = (void *) calloc(n, count);
    if (p == NULL) 
	printf("calloc of %u bytes failed:", n);
    return(p); 
}

/* g_new: malloc and report if error */
void *g_new(size_t n, int count)
{
    void *p;

    p = (void *) malloc(n*count);
    if (p == NULL) 
	printf("malloc of %u bytes failed:", n);
    return(p); 
}

/* g_realloc: malloc and report if error */
void *g_realloc(void *p, size_t n)
{
    void *newp;
    newp = (void *) realloc(p, n);
    if (newp == NULL) 
	printf("eralloc of %u bytes failed:", n);
    return(newp); 
}

void g_assert(int i) {
    ;
}
