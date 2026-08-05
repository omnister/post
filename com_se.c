#include "post.h"
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include "newread.h"

// stub for select command
// either print list of loaded sims w/no options
// or change to selected sim if given an option

int se(int simno) {

    if (simno != -1) {	// try to set simno
	if(set_simno(simno)<0) {
	    printf("no such simulation\n");
	}
    } 
    se_list();
    return (1);
}


