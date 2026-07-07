// read in an ngspice rawfile from first principles...
// includes .ac/.tran/.dc analysis
// RCW: 07/20/2014 general parsing structure
// RCW: 06/27/2016 start on command line options
// RCW: 06/27/2016 parse variable names into array
// RCW: 11/29/2016 parse multiple analysis raw files
// RCW: 07/06/2026 split out into separate reader process to support multiple simulations

/*
    select (abbreviated to "se")
    se: print a table of contents
	0 tdn.raw tran
	1 foo.raw tran
	2 xyx.raw ac
    se <n>: select nth simulation for graphing

*/

// some planning...
// cat <rawfile> | sp2 <varname1> <varname2> ... ; extract data for varnames
// cat <rawfile> | sp2 -l  			; list varnames in rawfile
// cat <rawfile> | sp2 -l -v  			; list everything
// sp2 -? 					; usage message

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include "datum.h"
#include "post.h"

#define MAXTAB 64	// max number of analyses in deck

typedef struct spicedat {
   char *title;		// title of deck
   char *date;		// date of run
   char *plottype; 	// type of plot
   char *filename; 	// raw file name
   int  cflag;		// 0=real, 1=complex data
   int  binary;		// 0=ASCII, 1=binary (rawfile)
   int  nvars;		// number of variables (including iv, dv);
   int  npts;		// total number of simulated time/freq points
   char **varname;	// table of variable names
   double *data; 	// pointer to data
} SPICEDAT;

SPICEDAT *sptab[MAXTAB];	// table of analyses
int ntab=0;			// number of analyses

// forward declarations:

char *skipblanks(char *s);
char *prefix(char *s, char *p);

// read a header, calls newspicedat to create SPICEDAT struct
// then uses header data to malloc data[] array
SPICEDAT *sp_read(char *filename);
SPICEDAT *read_header(FILE *fp);		
SPICEDAT *newspicedat();
void read_bin(SPICEDAT *sp, FILE *fp); 		// reads the binary part of the file and fills in the data[] array

void dumpspiceheaders(SPICEDAT *sp);	// dump all the headers in a SPICEDAT struct
void dumpspicedat(SPICEDAT *sp);	// dump all the data in a SPICEDAT struct

int splookup(SPICEDAT *sp, char *varname);	// return index to PWL with varname
void dumpvar(SPICEDAT *sp, int nv);		// dump a variable with index nv

void freespicedat(SPICEDAT *sp);	// destroy SPICEDAT struct
const char *rawfile_name(void);
const char *independent_varname(void);

char *progname;

int usage() {
    fprintf(stderr, "usage: %s [options] varnames ... < rawfile\n",progname);
    fprintf(stderr, "  [-h] print this help usage\n");
    fprintf(stderr, "  [-l] list only index to varnames\n");
    fprintf(stderr, "  [-p <n>] operate on given analysis\n");
    fprintf(stderr, "  [-v] verbose\n");
    exit(3);
}

int dolist=0;	// do a listing
int verbose=0;	// add in more verbosity
int plotnum=0;	// default to first analysis

int main(int argc, char **argv) {
   int i;

   SPICEDAT *sp;
   int n;

   int c;
   extern char *optarg;
   extern int optind, optopt;

   progname = argv[0];
   while ((c = getopt(argc, argv, "hlp:v?")) != -1) {
      switch(c) {
	 default:
         case '?':
         case 'h':
	    usage();
	    break;
         case 'p':
	    plotnum=atoi(optarg);
	    break;
         case 'l':
	    dolist++;
	    break;
         case 'v':
	    verbose++;
	    break;
      }
   }

   // if (dolist) printf("dolist\n");
   // if (verbose) printf("verbose\n");

   // this reads a spice file and then rereads so that concatenate
   // raw files can be read in as multiple data sets 

   sp = sp_read("a0.raw");
   printf("ntab: %d\n", ntab);
   sp = sp_read("a1.raw");
   printf("ntab: %d\n", ntab);
   sp = sp_read("a2.raw");
   printf("ntab: %d\n", ntab);

   if (plotnum > (ntab-1)) {
       fprintf(stderr, "no such simulation: %d: %d\n" , plotnum, ntab);
       exit(3);
   } else {
       sp = sptab[plotnum];	// default to first simulation in multiple read
   }

   // pre scan for variables 
   // bail out if we can't find them all

   int error=0;
   for (i=0; i<(argc-optind); i++) {
      if (splookup(sp, argv[optind+i]) == -1) {
	 fprintf(stderr,"can't find variable %s in rawfile\n",argv[optind+i]);
	 error++;
      }
      // printf("found variable %s at index %d\n", argv[optind+i], n);
   }
   if (error) {
      exit(4);
   }


   if (argc==optind) {	// no vars listed, operate on all data
       if (dolist) {
	   printf("# %s", sp->title);
	   printf("# got %d analyses numbered 0:%d\n", ntab, ntab-1);
	   for (i=0; i<ntab; i++) {
	       sp = sptab[i];
	       printf("# ---------------------\n");
	       // printf("# analysis %d: %s\n", i, sp->filename);
	       printf("# analysis %d:\n", i);
	       dumpspiceheaders(sp);	// print headers
	   }
       } else {
	   // dumpspiceheaders(sp);	// print headers
	   dumpspicedat(sp);	// print out data tables
       }
   } else {		// dump specific variables
       for (i=0; i<(argc-optind); i++) {
	  // prechecked these so should be good...
	  n=splookup(sp, argv[optind+i]);
	  dumpvar(sp,n);
	}
   }


   freespicedat(sp);
   exit(1);
}

SPICEDAT *newspicedat() {
    SPICEDAT *sp;
    sp = (SPICEDAT*) malloc(sizeof(SPICEDAT));
    sp->title = '\0';
    sp->date= '\0';
    sp->plottype= '\0';
    sp->cflag = 0;
    sp->binary = 0;
    sp->nvars = 0;
    sp->npts = 0;
    char **varname=NULL;
    double *data = NULL;
    return sp;
}

void freespicedat(SPICEDAT *sp) {
    int i;
    sp = (SPICEDAT*) malloc(sizeof(SPICEDAT));
    free(sp->title);
    free(sp->date);
    free(sp->plottype);
    free(sp->filename);
    for (i=0; i<sp->nvars; i++) {
       free(&(sp->varname[i]));
    }
    free(sp->varname);
    free (sp->data);
}

// lookup a variable name in SPICEDAT struct 
// and return an index number or -1 if not found
int splookup(SPICEDAT *sp, char *varname) {
   int nv;

   if (sp == NULL) return(-1);

   for (nv=0; nv<sp->nvars; nv++) {
      if (strcmp(varname, sp->varname[nv])==0) {
         return (nv);
      }
   }
   return(-1);
}

void dumpspiceheaders(SPICEDAT *sp) {
   int nv;
   
   if (sp == NULL) {
	fprintf(stderr,"NULL sp struct in dumpspiceheaders()\n");
	// exit(8);
   }

   printf("# filename: %s\n", sp->filename);
   printf("# title: %s", sp->title);
   printf("# date: %s", sp->date);
   printf("# plottype: %s", sp->plottype);
   if (verbose) {
       printf("# cflag: %d\n", sp->cflag);
       printf("# binary: %d\n", sp->binary);
       printf("# nvars: %d\n", sp->nvars);
       printf("# npts: %d\n", sp->npts);
   }

   for (nv=0; nv<sp->nvars; nv++) {
      printf("# variable %d %s\n", nv, sp->varname[nv]);
   }
}

void load_symbol(SPICEDAT *sp)
{
    int nv, np;
    double re, im,  iv;
    double re1,im1, iv1;
    double re2,im2, iv2;
    char *s, *t;
    char buf[128];
    double *data = sp->data;
    int c;

    DATUM *tmp;
    DATUM *result;

    for (nv = 1; nv < sp->nvars; nv++) {
	result=NULL;
	for (np = 0; np < sp->npts; np++) {	// print out data array
	    if (sp->cflag) {
		iv=data[2 * (np * sp->nvars + 0)];
		re=data[2 * (np * sp->nvars + nv)];
		im=data[2 * (np * sp->nvars + nv) + 1];
	    } else {
		iv=data[np * sp->nvars + 0];
		re=data[np * sp->nvars + nv];
	    }
	    if (np<2) {
		tmp=new_dat(re,im);
		tmp->iv = iv;
		result = link_dat(result,tmp);
	    } else if (np>2 && (!collinear(iv2,re2, iv1,re1, iv,re) || !collinear(iv2,im2, iv1,im1, iv,im))) {
		tmp=new_dat(re1,im1);
                tmp->iv = iv1;
                result = link_dat(result, tmp);
	    }
	    iv2 = iv1; iv1 = iv;
            re2 = re1; re1 = re;
            im2 = im1; im1 = im;
	}
	tmp=new_dat(re1,im1);
        tmp->iv = iv1;
        result = link_dat(result, tmp);

        s = sp->varname[nv];
	t = buf;
	while (*s!='\0') {
            switch (c=*s++){
                case  ')':
                case  '(':
                case  '#':
                    break;
                case  '%':
                   *t++ = '_';
                    break;
                default:
                   *t++ = c;
                   break;
            }
        }
        *t = '\0';

	install(buf, VAR, result, ntab);
    }
}

void dumpvar(SPICEDAT *sp, int nv)
{
    int np;
    double *data = sp->data;

    for (np = 0; np < sp->npts; np++) {	// print out data array
	// for (nv = 0; nv<sp->nvars; nv++) {
	if (sp->cflag) {
	    printf("%0.12g %0.12g %0.12g ",
		   data[2 * (np * sp->nvars + 0)],
		   data[2 * (np * sp->nvars + nv)],
		   data[2 * (np * sp->nvars + nv) + 1]);
	} else {
	    printf("%0.12g %0.12g ",
		   data[np * sp->nvars + 0], data[np * sp->nvars + nv]);
	}
	// }
	printf("\n");
    }
}

void dumpspicedat(SPICEDAT *sp)
{
    int nv, np;
    double *data = sp->data;

    for (np = 0; np < sp->npts; np++) {	// print out data array
	for (nv = 0; nv < sp->nvars; nv++) {
	    if (sp->cflag) {
		printf("%0.12g %0.12g ",
		       data[2 * (np * sp->nvars + nv)],
		       data[2 * (np * sp->nvars + nv) + 1]);
	    } else {
		printf("%0.12g ", data[np * sp->nvars + nv]);
	    }
	}
	printf("\n");
    }
}

char *skipblanks(char *s)
{
    for (; *s == ' ' && *s != '\0'; s++) {
	;
    }
    return s;
}

// check if string s is prefixed by string p
// return s with p stripped, or NULL if not matched

char *prefix(char *s, char *p)
{
    if (!strncmp(s, p, strlen(p))) {
	return (skipblanks(s + strlen(p)));
    } else {
	return NULL;
    }
}

char *strsave(char *s)
{				/* save string s somewhere */
    char *p;

    if (s == NULL) {
	return (s);
    }

    if ((p = (char *) malloc(strlen(s) + 1)) != NULL)
	strcpy(p, s);
    return (p);
}

// if the vars are left like "v(4)" you can't
// specify them on the command line without quoting
// the parenthesis, so we take 'em out

void editname(char *s)
{				/* edit variable name */
    char *p;

    for (p = s; *s != '\0'; s++) {
	*p = *s;
	if (*s != '(' && *s != ')') {
	    p++;
	}
    }
    *p = '\0';
}

SPICEDAT *read_header(FILE *fp)
{
    char *arg;
    char s[1024];
    int err = 1;
    SPICEDAT *sp;
    int debug = 0;
    int nvars, npts;

    sp = newspicedat();

    if (debug)
	printf("--------------------------------------------\n");

    while (fgets(s, sizeof(s), fp) != NULL) {
	if ((arg = prefix(s, "Title:"))) {
	    if (debug)
		printf("Title: %s", arg);
	    sp->title = strsave(arg);
	    err = 0;
	} else if ((arg = prefix(s, "Date:"))) {
	    sp->date = strsave(arg);
	    if (debug)
		printf("Date: %s", arg);
	} else if ((arg = prefix(s, "Plotname:"))) {
	    sp->plottype = strsave(arg);
	    if (debug)
		printf("Plottype: %s", arg);
	} else if ((arg = prefix(s, "Flags:"))) {
	    if (prefix(arg, "complex")) {
		sp->cflag = 1;
	    } else if (prefix(arg, "real")) {
		sp->cflag = 0;
	    } else {
		err++;
		fprintf(stderr, "bad parse of Flags: header line\n");
	    }
	    if (debug)
		printf("cflag: %d: Flags: %s", sp->cflag, arg);
	} else if ((arg = prefix(s, "No. Variables:"))) {
	    if (sscanf(arg, "%d", &nvars) != 1) {
		fprintf(stderr,
			"bad parse of No. Variables header line\n");
		err++;
	    }
	    sp->nvars = nvars;
	    if (debug)
		printf("NVAR: %d\n", sp->nvars);
	    // FIXME: create name string array here
	    sp->varname = (char **) malloc(nvars * sizeof(char *));
	} else if ((arg = prefix(s, "No. Points:"))) {
	    if (sscanf(arg, "%d", &npts) != 1) {
		fprintf(stderr, "bad parse of No. Points header line\n");
		err++;
	    }
	    sp->npts = npts;
	    if (debug)
		printf("NPTS: %d\n", sp->npts);
	} else if ((arg = prefix(s, "Command:"))) {
	    if (debug)
		printf("Command: %s", arg);
	} else if ((arg = prefix(s, "Variables:"))) {
	    if (debug)
		printf("Variables: %s", arg);
	} else if ((arg = prefix(s, "Binary:"))) {
	    if (debug)
		printf("Binary: %s", arg);
	    sp->binary = 1;
	    break;		// get out of input loop
	} else if (s[0] == '\t') {
	    int varnum;
	    char strbuf[128];
	    char strtype[128];
	    if (debug)
		printf("header line    : %s", s);
	    if (sscanf(s, "%d %s %s", &varnum, strbuf, strtype) == 3) {
		if (debug)
		    printf("%d %s %s\n", varnum, strbuf, strtype);
		editname(strbuf);
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
	return (NULL);
    }
    return (sp);
}

void read_bin(SPICEDAT *sp, FILE *fp)
{
    int v, p;
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
		    fread(&bytes, sizeof(bytes), 1, fp);
		    d = *((double *) bytes);
		    *dp = d;
		    dp++;
		} else {
		    fread(&bytes, sizeof(bytes), 1, fp);
		    d = *((double *) bytes);	// real
		    fread(&bytes, sizeof(bytes), 1, fp);
		    e = *((double *) bytes);	// imag
		    *dp = d;
		    dp++;
		    *dp = e;
		    dp++;
		}
	    }
	}
    }
}

// read in a header and a data segment, loop to handle multiple
// concatenated data files, update sptab[ntab], ntab++, with sp
// structure

SPICEDAT *sp_read(char *filename)
{

    FILE *fp;
    SPICEDAT *sp;
    int total;

    if ((fp = fopen(filename, "r")) == NULL) {
	fprintf(stderr, "cannot open %s\n", filename);
	exit(7);
    }

    while (!feof(fp)) {
	if (feof(fp))
	    break;
	while ((sp = sptab[ntab] = read_header(fp)) != NULL) {

	    sp->filename = strsave(filename);
	    total = sp->npts * sp->nvars;
	    if (sp->cflag) {
		total *= 2;
	    }

	    // malloc data array 
	    sp->data = (double *) malloc(total * sizeof(double));

	    read_bin(sp, fp);
	    load_symbol(sp);	// put in symbol table 

	    if (ntab < MAXTAB - 2) {
		ntab++;
	    } else {
		fprintf(stderr, "exceeded max number of analyses: %d!\n",
			MAXTAB);
	    }
	    break;
	}
    }


    return (sp);
}

const char *rawfile_name(void)
{
    return (sptab[plotnum]->filename);
}

const char *independent_varname(void)
{
    return (sptab[plotnum]->varname[0]);
}
