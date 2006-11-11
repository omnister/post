#include <term.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>  

extern void  rl_setprompt(char* prompt);
extern void  rl_saveprompt();
extern void  rl_restoreprompt();
extern void  rl_init();
extern int   rlgetc();
extern int   rl_ungetc();
extern char *rl_gets();
