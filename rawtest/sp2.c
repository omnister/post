// read in an ngspice rawfile from first principles...
// includes .ac/.tran/.dc analysis
// RCW: 07/20/2014
// RCW: 06/27/2016
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct spicedat {
   char *title;		// title of deck
   char *date;		// date of run
   char *plotname; 	// type of plot
   int  cflag;		// 0=real, 1=complex data
   int  binary;		// 0=ASCII, 1=binary (rawfile)
   int  nvars;		// number of variables (including iv, dv);
   int  npts;		// total number of simulated time/freq points
   char **varname;	// table of variable names
   double *data; 	// pointer to data
} SPICEDAT;

// forward declarations:

char *skipblanks(char *s);
char *prefix(char *s, char *p);
SPICEDAT *read_header();
void read_bin(SPICEDAT *sp); 
SPICEDAT *newspicedat();
void freespicedat(SPICEDAT *sp);
void dumpspicedat(SPICEDAT *sp);
void dumpspiceheaders(SPICEDAT *sp);

main() {
   int i,x;

   int nvars, npts;
   int nv, np;
   int complexflag;
   int total;
   SPICEDAT *sp;

   while(!feof(stdin)) {
       if (feof(stdin)) break;
       if ((sp=read_header()) != NULL) {

	   total = sp->npts*sp->nvars;
	   if (sp->cflag) {
	      total *= 2;
	   }

	   // malloc data array 
	   sp->data = (double *) malloc(total*sizeof(double));

	   read_bin(sp);
	   break;

       } else {
	   exit(0);
       }
   }

   dumpspiceheaders(sp);	// print headers
   dumpspicedat(sp);		// print out data tables
   freespicedat(sp);

   exit(1);
}

SPICEDAT *newspicedat() {
    SPICEDAT *sp;
    sp = (SPICEDAT*) malloc(sizeof(SPICEDAT));
    sp->title = '\0';
    sp->date= '\0';
    sp->plotname= '\0';
    sp->cflag = 0;
    sp->binary = 0;
    sp->nvars = 0;
    sp->npts = 0;
    char **varname=NULL;
    double *data = NULL;
}

void freespicedat(SPICEDAT *sp) {
    int i;
    sp = (SPICEDAT*) malloc(sizeof(SPICEDAT));
    free(sp->title);
    free(sp->date);
    free(sp->plotname);
    for (i=0; i<sp->nvars; i++) {
       free(&(sp->varname[i]));
    }
    free(sp->varname);
    free (sp->data);
}

void dumpspiceheaders(SPICEDAT *sp) {
   int nv, np;
   
   printf("#title: %s", sp->title);
   printf("#date: %s", sp->date);
   printf("#plotname: %s", sp->plotname);
   printf("#cflag: %d\n", sp->cflag);
   printf("#binary: %d\n", sp->binary);
   printf("#nvars: %d\n", sp->nvars);
   printf("#npts: %d\n", sp->npts);

   for (nv=0; nv<sp->nvars; nv++) {
      printf("#variable %d %s\n", nv, sp->varname[nv]);
   }
}

void dumpspicedat(SPICEDAT *sp) {
   int nv, np;
   double *data = sp->data;

   for (np = 0; np<sp->npts; np++) { 	// print out data array
       for (nv = 0; nv<sp->nvars; nv++) {
	  if (sp->cflag) {
	      printf("%0.12g %0.12g ", 
		    data[2*(np*sp->nvars + nv)], 
		    data[2*(np*sp->nvars + nv)+1]);
	  } else {
	      printf("%0.12g ", 
		    data[np*sp->nvars + nv]);
	  }
       }
       printf("\n");
   }
}

char *skipblanks(char *s) {
   for (; *s==' ' && *s !='\0'; s++) {
     ;
   }
   return s;
}

// check if string s is prefixed by string p
// return s with p stripped, or NULL if not matched

char *prefix(char *s, char *p) {
   if (!strncmp(s, p, strlen(p))) {
      return(skipblanks(s+strlen(p)));
   } else {
      return NULL;
   }
}

char *strsave(char *s)   /* save string s somewhere */
{
    char *p;

    if (s == NULL) {
        return(s);
    }

    if ((p = (char *) malloc(strlen(s)+1)) != NULL)
	strcpy(p,s);
    return(p);
}

SPICEDAT *read_header() {
   char *arg;
   char s[1024];   
   int err=1;
   SPICEDAT *sp;
   int debug=0;
   int nvars, npts;

   sp=newspicedat();

   if (debug) printf("--------------------------------------------\n");

   while(fgets(s, sizeof(s), stdin) != NULL) {
	if (arg=prefix(s, "Title:")) {
	    if (debug) printf("Title: %s", arg);
	    sp->title = strsave(arg);
	    err=0;
	} else if (arg=prefix(s,"Date:")) {
	    sp->date = strsave(arg);
	    if (debug) printf("Date: %s", arg);
	} else if (arg=prefix(s,"Plotname:")) {
	    sp->plotname = strsave(arg);
	    if (debug) printf("Plotname: %s", arg);
	} else if (arg=prefix(s,"Flags:")) {
	    if (prefix(arg,"complex")) {
	        sp->cflag=1;
	    } else if (prefix(arg,"real")) {
	        sp->cflag=0;
	    } else {
	       err++;
	       fprintf(stderr,"bad parse of Flags: header line\n");
	    }
	    if (debug) printf("cflag: %d: Flags: %s", sp->cflag, arg);
	} else if (arg=prefix(s,"No. Variables:")) {
	    if (sscanf(arg, "%d", &nvars) != 1) {
	       fprintf(stderr,"bad parse of No. Variables header line\n");
	       err++;
	    }
	    sp->nvars=nvars;
	    if (debug) printf("NVAR: %d\n", sp->nvars);	
	    // FIXME: create name string array here
	    sp->varname = (char **) malloc(nvars*sizeof(char *)); 
	} else if (arg=prefix(s,"No. Points:")) {
	    if (sscanf(arg, "%d", &npts) != 1) {
	       fprintf(stderr,"bad parse of No. Points header line\n");
	       err++;
	    }
	    sp->npts=npts;
	    if (debug) printf("NPTS: %d\n", sp->npts);
	} else if (arg=prefix(s,"Command:")) {
	    if (debug) printf("Command: %s", arg);
	} else if (arg=prefix(s,"Variables:")) {
	    if (debug) printf("Variables: %s", arg);
   	} else if (arg=prefix(s, "Binary:")) {
	   if (debug) printf("Binary: %s", arg);
	   sp->binary=1;
	   break;		// get out of input loop
	} else if (s[0]=='\t') {
	   int varnum;
	   char strbuf[128];
	   char strtype[128];
	   if (debug) printf("header line    : %s", s);
	   if (sscanf(s, "%d %s %s", &varnum, strbuf, strtype) == 3) {
	      if (debug) printf("%d %s %s\n", varnum, strbuf, strtype);
	      sp->varname[varnum] = strsave(strbuf);
	   } else {
	       printf("unparsed header line    : %s", arg);
	   }
	} else {
	   printf("unparsed header line    : %s", arg);
	}
   }
   if (err) {
      freespicedat(sp);
      return(NULL);
   }
   return(sp);
}

void read_bin(SPICEDAT * sp)
{
    int i, v, p;
    double d;
    double e;
    char bytes[8];
    double *dp = sp->data;

    if (sp->binary != 1) {
	fprintf(stderr, "can't parse non-binary rawfile!\n");
	exit(1);
    } else {
	for (p = 0; p < sp->npts; p++) {
	    for (v = 0; v < sp->nvars; v++) {	// read simulation vars
		if (!sp->cflag) {
		    fread(&bytes, sizeof(bytes), 1, stdin);
		    d = *((double *) bytes);
		    *dp = d; dp++;
		} else {
		    fread(&bytes, sizeof(bytes), 1, stdin);
		    d = *((double *) bytes);	// real
		    fread(&bytes, sizeof(bytes), 1, stdin);
		    e = *((double *) bytes);	// imag
		    *dp = d; dp++;
		    *dp = e; dp++;
		}
	    }
	}
    }
}
