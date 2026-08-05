/*
 * com_vi(), check in a spice raw file  
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> 	/* for getopt() */
#include <stdlib.h>	/* for exit() */
#include <sys/stat.h>
#include <time.h>

#include "post.h"
#include "y.tab.h"

int com_vi(char *arg)
{
    char cmd[200];
    char raw[80];	// "foo.raw"
    char name[100];	// "foo"
    char cki[120];	// "foo.cki"
    struct stat sb;
    int before;
    int after;

    if (strcmp("none", rawfile_name())==0) {
	printf("\tnot editing any file\n");
	return(0);
    }

    strcpy(raw, rawfile_name());	// "foo.raw"
    strcpy(name, strtok(raw,"."));	// "foo"
    snprintf(cki, sizeof(cki), "%s.cki", name);  // "foo.cki"

    // -------------------------------------
    if (stat(cki, &sb) == -1) {
         perror("lstat"); exit(-1);
    }
    before = sb.st_mtime; // printf("Last file modification:   %s", ctime(&sb.st_mtime));
    // -------------------------------------

    snprintf(cmd, sizeof(cmd), "vi %s.cki", name); 
    system(cmd);

    // -------------------------------------
    if (stat(cki, &sb) == -1) {
         perror("lstat"); exit(-1);
    }
    after = sb.st_mtime; // printf("Last file modification:   %s", ctime(&sb.st_mtime));
    // -------------------------------------

    if ((after-before)!=0) {		// vi modified the file
	printf("modified, rerunning ngspice\n");

	// SPICE="ngspice -b $DECK.cki -r $DECK.raw -o $DECK.txt"
	snprintf(cmd, sizeof(cmd), "ngspice -b %s.cki -r %s.raw -o %s.txt", name, name, name); 
	system(cmd);

	// POST="post -r $DECK.raw"
	char *args[]={"post", "-r", rawfile_name(), NULL};
	execvp("post", args);

    }

    return(0);
}
