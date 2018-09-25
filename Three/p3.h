/* p3.h
   Program 3
   CS570
   SDSU

   See the assignment file ~cs570/program3 for more information.

 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <semaphore.h>
#include "CHK.h"

#define MAXROBOTS 10
#define NRROBOTS 3
#define WIDTH 2
#define QUOTA 5
#define SEED 25
#define LIFESPAN 100
#define WIDGETBUILDER "p3robot"

/* MAXINTSTRING is intended as the size of a char array large enough
   to hold the longest string representation (in decimal) of an int
   in this system, including the trailing \0. 
   I don't know of any system place where this is defined, so this
   is something that might have to be redefined if we port this code. */
#define MAXINTSTRING 12

/* SEMNAMESIZE is intended as the size of a char array large enough
   to hold the longest semaphore name constructed as the concatenation
     "/class-id" "uid" "user-chosen-id" 
   including the trailing \0.
   Note that this is very fragile! For instance, I'm assuming class id's
   are never more than 3 characters, and uid's are never more than
   5 characters and user-chosen-id's are never more than 20 characters. (Note
   that man sem_open warns that:
     "For maximum portability, [the entire semaphore name string] should 
     include no more than 14 characters, but this limit is not
     enforced."
   So in fact, if this convention is used, it would be good for the
   user-chosen-id to be much shorter than 20 characters!
   For an example of how SEMNAMESIZE might be used -- and the
   rationale for using it -- see p3main.c
*/
#define SEMNAMESIZE 1+3+5+20+1
#define COURSEID "570"

#define FALSE 0
#define TRUE 1


/*-------------ALREADY WRITTEN. SEE p3robot.c----------------------------
 * Name: printeger
 * Purpose: does a formated (width 6) print of an integer (no \n) to
   stdout.
 * Input parameters: integer to be printed.
 * Output parameters: none.
 * Other side effects: Even though no \n is printed, output is flushed.
 * Routines called: printf fflush sleep random
 * References: SDSU CS570 p3 assignment. See also
   the general discussion at the beginning of this file.
 */
void printeger(int n);

/*------------YOU WRITE THIS IN p3helper.c:------------------------------
 * Name: initStudentStuff
 * Purpose: initialize any data structures (including semaphores) that
   the cooperating robots may need.
   (Such a generic function lump is not usually good style, but in
   this case is makes it easy for us to keep student and problem
   issues separate.)
 * Input parameters: None
 * Output parameters: None
 * Other side effects: ...
 * Routines called: ... No I/O is done, and there is no call to
   sleep, usleep, nanosleep.
 * References: The p3 assignment
 */
void initStudentStuff(void);

/*------------YOU WRITE THIS IN p3helper.c:------------------------------
 * Name: placeWidget
 * Purpose: Place a widget "on the warehouse floor". Uses
   printeger() to print an integer, with new lines (and 'F')
   coordinated with others who are printing. The result of the
   coordination is that all the widgets (integers) get put out in the
   formatted way described in the p3 assignment.
 * Input parameters: an integer (the widget) to be printed
 * Output parameters: none.
 * Other side effects: none.
 * Routines called: ... printeger(), printf("N\n"), and printf("F\n")
   are the only output tools that placeWidget uses.
   fflush(stdout) is called after each call to printf.
   No calls are made to sleep, usleep, nanosleep.
 * References: p3 assignment.
 */
void placeWidget(int n);

