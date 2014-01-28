// read in an ngspice rawfile
// includes .ac/.tran/.dc analysis
// 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

int read_header(int *nvars, int *npts, int *cflag) {
   char *arg;
   char s[1024];   
   int err=1;
   printf("--------------------------------------------\n");
   while(fgets(s, 1024, stdin) != NULL) {
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
  char bytes[8];

  if (cflag) {
      total = npts*(1+(nvars-1)*2); // complex data
  } else {
      total = npts*nvars;
  }

  printf("in read_bin, cflag=%d, total=%d\n",cflag, total);

  for (p=0; p<npts; p++) {
      for (v=0; v<nvars; v++) {
	   fread(&bytes, 8, 1, stdin);
	   d = *((double*)bytes);
	   printf("%g ",d);
      }
      printf("\n",d);
  }
}

main() {
   int i,x;

   int nvars, npts;
   int complexflag;

   while(!feof(stdin)) {
       if (feof(stdin)) break;
       if (read_header(&nvars, &npts, &complexflag) == 0) {
	   read_bin(nvars, npts, complexflag);
       } else {
	  break;
       }
   }
}
