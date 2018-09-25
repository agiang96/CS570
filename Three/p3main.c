/*  p3main.c
      cs570 Program 3
      SDSU
      (Adapted from an assignment created by Professor Vernor Vinge)

 *-----------------------------------------------------------------------
 * Name: p3main
 * Purpose: This is the driver program starting all single widget-making
   robots (see p3robot.c). 
   This constitutes an important part of the specification for p3.
   Your code, however, will all be in p3helper.c (The p3helper.c stub
   that provided in the assignment directory should compile and link
   with this code -- and run (though with clearly inadequate performance).
   NOTE: It is assumed that all the processes are run with the
         same uid.
   NOTE: It is assumed the WIDGETBUILDER is accessible for execution
         via the current PATH.
 * Input parameters:  
   The program with the tail of the argument list omitted, and defaults
   will be used for the omitted items.
   Note that p3main validates the command line parameters:
     o argv[0] is a pointer to the name of the program
       (automatically supplied by the runtime system).
     o argv[1] is a string representation of the number 
       of robots needed for this run. This should
       be a positive number <= MAXROBOTS.
       (The default is NRROBOTS.)
     o argv[2] is a string representation of the integer 
       seed to be used for random number generation.
       (The default is SEED.) If this arg is specified, then
       the preceding ones must all be specified too.
       It is recommended that this be an unsigned int between
       0 and 10000.
     o argv[3] is a string representation of the positive
       integer specifying the number of widgets to be
       placed on each full row of output.
       (The default is WIDTH.) If this arg is specified, then
       the preceding ones must all be specified too.
     o argv[4] is a string representation of the nonnegative
       integer specifying widget quota of each robot.
       (The default is QUOTA.) If this arg is specified, then
       the preceding ones must all be specified too.
 * Output parameters: exits with 0 if no error detected. Exits
   with 1 if error detected.
 * Other side effects: 
   Of course the main side effect is that each robot process
   is forked and exec'd:
   each robot is given 
     o argv[0] = WIDGETBUILDER (string constant in p3.h)
     o argv[1] = string representation of the number that p3main
                 got as its argv[1] parameter (or the default).
     o argv[2] = the string representation of a random number derived 
                 from successively applying the random number
                 generator to the seed that p3main received as its
                 argv[2] (or the default).
     o argv[3] = string representation of the number that p3main
                 got as its argv[3] parameter (or the default).
     o argv[4] = string representation of the number that p3main
                 got as its argv[4] parameter (or the default).
     o argv[5] contains NULL. 
   p3main prints routine information about its operation to stdout. Error
   reporting on that operation is sent to stderr.
   NOTE: p3main uses a POSIX named semaphore to learn when
   all the robots have completed. In Solaris 2.6, each named semaphore
   appears to have two entries in /tmp. For instance the semaphore
   named "foo" below will probably have entries "/tmp/.SEMDfoo" and
   "/tmp/.SEMLfoo".  (See man sem_open for restrictions on semaphore
   names.)  Thus, under Solaris 2.6 there is the possibility of name
   clashes!  I have attempted to customize the name per uid and course
   number in the code below. If everyone on the host machine follows
   this convention there should not be name collisions. 

   If this program terminates normally, it will remove the named 
   semaphore  "files" that it created.
   If this program terminates abnormally (for example, via control-C),
   these files may still exist and must be removed "by hand" before
   you can successfully rerun this example.
 * Routines called: printf, fprintf, sprintf, atoi, srandom, random, alarm,
     sem_open sem_post sem_wait sem_unlink, fork, execvp,
     fflush, sleep, robot, (macro)CHK.
 * References: The p3 assignment file.
 */
#include "p3.h"

int main(int argc, char *argv[]) {
  int nrRobots;
  int width; /* number of widgets on a full line */
  int quota; /* number of widgets each widget builder is to make */
  int seed; /* random number initializer */
  int i; /* loop counter */
  sem_t *done; /* semaphore used by main to wait till everyone done */
  char semaphoreName[SEMNAMESIZE];

printf("Here is the output of /bin/ls -o /dev/shm/ | grep $USER\n");
system("/bin/ls -o /dev/shm/ | grep $USER");
printf("If you see any files listed above that you own, you may need to rm them before\n");
printf("testing p3.  If you just want this report without testing p3, try 'p3 0'\n");
fflush(stdout);

  nrRobots = (argc>1) ? atoi(argv[1]) : NRROBOTS;
  seed = (argc>2) ? atoi(argv[2]) : SEED;
  width = (argc>3) ? atoi(argv[3]) : WIDTH;
  quota = (argc>4) ? atoi(argv[4]) : QUOTA;

  if ( (nrRobots < 1) || (nrRobots > MAXROBOTS) ) {
    fprintf(stderr,"nrRobots = %d is illegal. Goodbye.\n",nrRobots);
    exit(1);
  }
  if ( width < 1 ) {
    fprintf(stderr,"width = %d is illegal. Goodbye.\n",width);
    exit(1);
  }
  if ( quota < 0 ) {
    fprintf(stderr,"quota = %d is illegal. Goodbye.\n",quota);
    exit(1);
  }

  /* set up the random number generator for this process */
  srandom(seed);

  /* LIFESPAN is an emergency termination limit. It should be longer
     than a run should ever take.  Note that this is to protect the
     system from bugs in your solution, Thus LIFESPAN is intended to
     be longer than any reasonable robots would need.  */
  alarm(LIFESPAN);

  /* Try to make a "name" for the semaphore that will not clash.
     See man sem_open, and the commentary under "side effects" at the
     beginning of this file. */
  sprintf(semaphoreName,"/%s%lddone",COURSEID,(long)getuid());
 /* unlink any 'old' version of the semaphore (if the previous run of p3
    ended badly, the semaphore name might still exist in the filesystem,
    and could prevent p3 from running successfully in the future) */
    (void) sem_unlink(semaphoreName);
  /* Create and Initialize the Done semaphore. See man sem_open for 
     a discussion of these parameters. This follows Bill Gallmeister's
     PROGRAMMING FOR THE REAL WORLD: POSIX.4 advice on setting the modes. */
     done = sem_open(semaphoreName,O_RDWR|O_CREAT|O_EXCL,S_IRWXU,0);
  CHK(SEM_FAILED != done);

  /* start the widget builders */
  for (i=0; i<nrRobots; i++) { 
    pid_t pid;
    char nameString[] = WIDGETBUILDER;
    char nrRobotsString[MAXINTSTRING];
    char seedString[MAXINTSTRING];
    char widthString[MAXINTSTRING];
    char quotaString[MAXINTSTRING];
    char *robotArgs[6];

    sprintf(nrRobotsString,"%d",nrRobots);
    seed++;
    sprintf(seedString,"%d",seed);
    sprintf(widthString,"%d",width);
    sprintf(quotaString,"%d",quota);

    robotArgs[0] = nameString;
    robotArgs[1] = nrRobotsString;
    robotArgs[2] = seedString;
    robotArgs[3] = widthString;
    robotArgs[4] = quotaString;
    robotArgs[5] = NULL;

    CHK(pid = fork());
    if ( 0 == pid ) { /* exec robot in this child: */
      CHK(execvp(nameString,robotArgs));
      /* should never reach next line, even if execvp fails! */
      exit(1); 
    }               
  }

  /* mother process waits for all robots to complete and then
     prints the result: */
  for (i=0; i<nrRobots; i++) {
    CHK(sem_wait(done));
  }

  /* remove the semaphore name from the name space! */
  CHK(sem_unlink(semaphoreName));
  exit(0);
}

