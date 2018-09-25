/* Filename  - p2.h
 * Author: Alexander Giang cssc0030
 * CS570 - Dr. John Carroll
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include "getword.h"
#define MAXITEM 100 // max number of words per line
#define MAXPIPES 10 // max number of pipes

/***
 * used for signal(int, func) handler
 * ref: sighandler.c example
 */
void myhandler(int sig);

/***
 * Runs the simple shell, slightly more complex than the original p2
 */
int main();

/***
 * parse specs:
 * All syntactic analysis should be done within parse(). This means, among other
 * things, that parse() should set appropriate flags when getword() encounters
 * words that are metacharacters. You may be graded off if your routines are
 * ill-designed or do not adhere to the requirements. Examples of poor design:
 * calling parse() more than once per command [that is, parse() should continue
 * to call getword() repeatedly, not returning until it has encountered one of
 * 3 command terminators], or having getword() unnecessarily set flags that are
 * more appropriately done within parse().
 *
 * parse() must use a getword() function that follows the specs from Program 1,
 * [copied into the directory ~/Two].  Examining the file ~cs570/Two/makefile
 * may be helpful in determining how it is linked into your program.
 */
int parse();

/***
 * Handles piping for the first and last children
 */
void piping();

/***
 * Handles piping for the middle children
 */
void midpiping();
