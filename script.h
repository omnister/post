
/* scripting support functions */

int scriptopen(char* prog, char *arg1, char *arg2);
int scriptclose();
int scriptfeed(char *msg);
int scriptread(char *buf, int n, int timeout);

/* 
 *   scriptopen():  called with full path to executable 
 *   returns 1 on success, 0 on failure
 *
 *   scriptfeed(): may be called as many times as needed
 *   with lines in msg to be sent to stdin of script process
 *   returns 1 on success, 0 on failure.
 *
 *   scriptread(): called with a buffer to hold the stdout
 *   from measurement script.  Will only return the first
 *   line of stdout.  If timeout argument is nonzero, then
 *   a time limit of <timeout> seconds is set after which
 *   subprocess is killed and 0 returned.  Returns 1 on 
 *   success.
 */
