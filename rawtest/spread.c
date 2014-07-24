// read in an ngspice rawfile
// includes .ac/.tran/.dc analysis
// 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *skipblanks(char *s);
char *prefix(char *s, char *p);


int read_header(int *nvars, int *npts, int *cflag) {
   char *arg;
   char s[1024];   
   int err=1;
   printf("--------------------------------------------\n");
   while(fgets(s, sizeof(s), stdin) != NULL) {
	if (arg=prefix(s, "Title:")) {
	    printf("Title: %s", arg);
	    err=0;
	} else if (arg=prefix(s,"Date:")) {
	    printf("Date: %s", arg);
	} else if (arg=prefix(s,"Plotname:")) {
	    printf("Plotname: %s", arg);
	} else if (arg=prefix(s,"Flags:")) {
	    if (prefix(arg,"complex")) {
	       *cflag=1;
	    } else if (prefix(arg,"real")) {
	       *cflag=0;
	    } else {
	       err++;
	       printf("bad parse of Flags: header line\n");
	    }
	    printf("cflag: %d: Flags: %s", *cflag, arg);
	} else if (arg=prefix(s,"No. Variables:")) {
	    if (sscanf(arg, "%d", nvars) != 1) {
	       err++;
	       printf("bad parse of No. Variables header line\n");
	    }
	    printf("NVAR: %d\n", *nvars);
	} else if (arg=prefix(s,"No. Points:")) {
	    if (sscanf(arg, "%d", npts) != 1) {
	       printf("bad parse of No. Points header line\n");
	       err++;
	    }
	    printf("NPTS: %d\n", *npts);
	} else if (arg=prefix(s,"Command:")) {
	    printf("Command: %s", arg);
	} else if (arg=prefix(s,"Variables:")) {
	    printf("Variables: %s", arg);
   	} else if (arg=prefix(s, "Binary:")) {
	   printf("Binary: %s", arg);
	   break;
	} else {
	   printf("    : %s", s);
	}
   }
   return(err);
}

read_bin(int nvars, int npts, int cflag) {
  int i,v,p;
  int total;
  double d;
  double e;
  char bytes[8];

  if (cflag) {
      total = npts*(1+(nvars-1)*2); // complex data
  } else {
      total = npts*nvars;
  }

  printf("in read_bin, cflag=%d, total=%d\n",cflag, total);

  for (p=0; p<npts; p++) {

      // get independent variable
      fread(&bytes, sizeof(bytes), 1, stdin);
      d = *((double*)bytes);
      fread(&bytes, sizeof(bytes), 1, stdin);	// throw away complex part
      printf("%g ",d);

      // read simulation vars
      for (v=1; v<nvars; v++) {
	  if (!cflag) {
	        fread(&bytes, sizeof(bytes), 1, stdin);
	        d = *((double*)bytes);
      		printf("%g ",d);
	  } else {
	        fread(&bytes, sizeof(bytes), 1, stdin);
	        d = *((double*)bytes);	// real
	        fread(&bytes, sizeof(bytes), 1, stdin);
	        e = *((double*)bytes);	// imag
      		printf("(%g,%g) ",d,e);

	  }
      }
      printf("\n");
  }
}

// table[pts][var][0:1]

main() {
   int i,x;

   int nvars, npts;
   int complexflag;
   double ** table;

   while(!feof(stdin)) {
       if (feof(stdin)) break;
       if (read_header(&nvars, &npts, &complexflag) == 0) {
	   // malloc big array of arrays here
	   read_bin(nvars, npts, complexflag);
	   break;
       } else {
	   exit(0);
       }
   }
   // print out big array here
   exit(1);
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
