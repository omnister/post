#include "symbol.h"
#include <math.h>

extern  void push();
extern  void eval(), add(), sub(), mul(), xdiv(), negate(), power(), modulo();
extern  char *strsave();
extern  void license();
extern  int  collinear();

typedef int (*Inst)();
#define STOP    (Inst) 0

extern  Inst *progp, *progbase, prog[], *code();
extern  void assign(), bltin(), varpush(), constpush(), print(), varread();
extern  void prexpr(), prstr();
extern  void gt(void), lt(void), eq(void), ge(void), levoid(), ne(void), and(void), or(void), not(void);
extern  void ifcode(), whilecode(), call(), arg(), argassign();
extern  void funcret(void), procret(void);
extern  void define(Symbol* sp);
extern  void execerror(char *s, char *t);
extern  int ls(void);
extern  int com_ci(char *file);
extern  int se(int simno);
extern  const char *rawfile_name();
extern  const char *independent_varname();

extern  void initcode(void);
extern  void init(void);
extern  void execute(Inst* p);

extern void graphinit(void);
extern void graphnext(void);
extern void graphexpr(DATUM *d);
extern void graphxl(double xlmin, double xlmax);
extern void graphyl(double ylmin, double ylmax);
extern void graphlogx(void);
extern void graphversus(DATUM *d);
extern void graphprint(int mode);
extern void graphprint_gnu(int mode);
extern void graphprint_pd(int mode);

char    *rl_gets();
int     rlgetc();
int     rl_ungetc();
int     moreinput();

int com_ci(char *rawfile);
