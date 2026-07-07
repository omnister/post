#include <term.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>  

extern void  rl_setprompt(char* prompt);
// extern void  rl_saveprompt();
// extern void  rl_restoreprompt();
extern void  rl_init(void);
//extern void  rl_save_start();
//extern char *rl_save_end();
extern int   rlgetc(FILE *fd);
extern int   rl_ungetc(int c, FILE *fd);
extern char *rl_gets(char *prompt);
