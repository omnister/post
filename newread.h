
typedef struct spicedat {
   char *title;         // title of deck
   char *date;          // date of run
   char *plottype;      // type of plot
   char *filename;      // raw file name
   int  cflag;          // 0=real, 1=complex data
   int  binary;         // 0=ASCII, 1=binary (rawfile)
   int  nvars;          // number of variables (including iv, dv);
   int  npts;           // total number of simulated time/freq points
   char **varname;      // table of variable names
   double *data;        // pointer to data
} SPICEDAT;


int set_simno(int sim);
int cur_simno(void);
void se_list(void);
void sp_read(char *filename);
