#include "post.h"
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <regex.h>

#define SP ".*[.]raw$|.*[.]braw$|.*[.]acs$|.*[.]tr0$|.*[.].ac0$|.*[.]N$|.*[.]sw0$|.*[.]W$" 

regex_t preg;

int filter(const struct dirent *de) {
   return !(regexec(&preg, de->d_name, (size_t) 0 , NULL, (int) 0)); 
}

int ls() {

    struct dirent **namelist;
    int n;	/* number of files found */
    int i;
    
    if ((n=regcomp(&preg, SP, REG_EXTENDED)) != 0) {
	printf("regcomp error: %d\n",n);
	exit(0);
    };

    n = scandir(".", &namelist, filter, alphasort);
    if (n<0) {
    	perror("scandir");
    } else {
        for(i=0; i<n; i++) {
	    printf("%s\n", namelist[i]->d_name);
	    free(namelist[i]);
	}
	free(namelist);
    }
    return (1);
}
