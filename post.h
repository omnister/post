#include "datum.h"
#include "symbol.h"
#include <math.h>

extern  void push();
extern  void eval(), add(), sub(), mul(), xdiv(), negate(), power(), modulo();
extern  char *strsave();
extern  void license();

typedef int (*Inst)();
#define STOP    (Inst) 0

extern  Inst *progp, *progbase, prog[], *code();
extern  void assign(), bltin(), varpush(), constpush(), print(), varread();
extern  void prexpr(), prstr();
extern  void gt(), lt(), eq(), ge(), le(), ne(), and(), or(), not();
extern  void ifcode(), whilecode(), call(), arg(), argassign();
extern  void funcret(), procret();
extern  void define();
extern  void execerror();
extern  int ls(void);
extern  int com_ci(char *file);
extern  char *rawfile_name();

extern  void initcode();
extern  void init();
extern  void execute();

extern void graphinit();
extern void graphnext();
extern void graphexpr(DATUM *d);
extern void graphxl(double xlmin, double xlmax);
extern void graphyl(double ylmin, double ylmax);
extern void graphprint();
extern void graphprint_gnu();

char    *rl_gets();
int     rlgetc();
int     rl_ungetc();
int     moreinput();

