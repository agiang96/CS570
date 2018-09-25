/*  p3robot.c
      cs570 Program 3
      SDSU

 *-----------------------------------------------------------------------
 * Name: p3robot
 * Purpose: This is the program for a single widget-making
   robot. It is intended to be started from the code of p3main.c.
   p3robot.c constitutes an important part of the specification for p3.
   Your code, however, will all be in p3helper.c
   (The p3helper.c stub that I provide in the assignment directory
   should compile and link with this -- and run (though with clearly
   inadequate performance).
 * Input parameters: Note that this program is only intended to be
   exec'd from the code shown in p3main.c
   p3main does the argument validation.
     o argv[0] is a pointer to the name of the program
       (automatically supplied by the runtime system).
     o argv[1] is a string representation of the number 
       of robots needed for this run. This should
       be a positive number <= MAXROBOTS.
     o argv[2] is a string representation of the integer 
       seed to be used for random number generation.
     o argv[3] is a string representation of the positive
       integer specifying the number of widgets to be
       placed on each full row of output.
     o argv[4] is a string representation of the nonnegative
       integer specifying widget quota of each robot.
 * Output parameters: None.
 * Other side effects: p3robot prints its widget output to stdout.
      This is done in coordination with the other robots, per the
      p3 assignment (see ~cs570/program3)
 * Routines called: printf, sprintf, atoi, srandom, sem_open, sem_post,
      fflush, sleep, initStudentStuff, placeWidget, (macro)CHK.
 * References: The p3 assignment file.
 */
#include "p3.h"

/* The next four integers are global, for access from the
   students' code in p3helper.c
   (The (-1) initializations are mainly for error catching
   purposes. The code here in p3robot main should put the correct
   values into these variables at runtime.)
   */
int nrRobots = -1; /* number of robots */
int width = -1; /* number of widgets on a full line */
int quota = -1; /* number of widgets each widget builder is to make */
int seed = -1;  /* random number initializer */

int main(int argc, char *argv[]) {
  int i; /* loop counter */
  sem_t *done; /* semaphore used by main to wait till everyone done */
  char semaphoreName[SEMNAMESIZE];

  assert( 5 == argc ); /* p3robot assumes validation is done before
                          our robot process is started, so this
                          assert is just a sanity check. */
  nrRobots = atoi(argv[1]);
  seed = atoi(argv[2]);
  width = atoi(argv[3]);
  quota = atoi(argv[4]);
  srandom(seed);

  initStudentStuff();  /* provided by you, in p3helper.c */

  /* robot will terminate after LIFESPAN seconds. 
     Note that this is to protect the system from bugs in your solution,
     since your robot is supposed to exit in any case after it has
     done its quota of work.  Thus LIFESPAN is intended to be longer
     than any reasonable robot would need.  */
  alarm(LIFESPAN);

  /* robot builds/places a total of quota widgets */
  for (i=0; i<quota; i++) {
    placeWidget(getpid());
    sleep(1+random()%2);
  }

  /* Try to reconstruct the "name" for the semaphore that p3main used.
     See man sem_open, and the commentary under "side effects" in file
     p3main.c */
  sprintf(semaphoreName,"/%s%lddone",COURSEID,(long)getuid());
  /* Open the done semaphore. Note that this semaphore should already
     exist. */
  done = sem_open(semaphoreName,0);
  CHK(SEM_FAILED != done);
  CHK(sem_post(done));  

  exit(0);
}

/* doc for printeger is p3.h */
void printeger(int n)
{
  printf("%6d",n);
  fflush(stdout);
  sleep(random()%2);
}
