/* Alexander Giang, cssc0030
 * p3helper.c
 * Program 3 assignment
 * CS570 Spring 2018, Dr. Carroll
 * SDSU
 * Due Dates - EC: 4/6/2018, Normal: 4/11/2018
 *
 * This is the ONLY file you are allowed to change. (In fact, the other
 * files should be symbolic links to
 *   ~cs570/Three/p3main.c
 *   ~cs570/Three/p3robot.c
 *   ~cs570/Three/p3.h
 *   ~cs570/Three/makefile
 *   ~cs570/Three/CHK.h    )
 * The file ~cs570/Three/createlinks is a script that will create these for you.
 */
#include "p3.h"

/* You may put declarations/definitions here.
 * In particular, you will probably want access to information
 * about the job (for details see the assignment and the documentation
 * in p3robot.c):
 */
extern int nrRobots;
extern int seed;
extern int width;
extern int quota;

sem_t *mutex; 
char semMutx[SEMNAMESIZE]; // for sem_open

/* General documentation for the following functions is in p3.h
 * Here you supply the code, and internal documentation:
 */
void initStudentStuff(void) {
    int fd;    //for opening counter file
    int count = 0; //counter for printing "N" and "F"

    sprintf(semMutx,"%s%ldmutx",COURSEID, (long)getuid());
    // initialize mutex, but if it already exists, open existing (w/o O_EXCL) 
    if( SEM_FAILED == (mutex = sem_open(semMutx,O_CREAT|O_RDWR|O_EXCL,S_IRUSR|S_IWUSR,1)) )
        CHK(SEM_FAILED != (mutex = sem_open(semMutx,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR,1)));

    sem_wait(mutex); // request enter critical region (decrement)
    /******* critical region for file manipulation*******/
    //opening counter file
    CHK( fd = open("countfile",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR));
    if(lseek(fd, 0, SEEK_END) == 0) { // if file is empty, write 0
        CHK(lseek(fd,0,SEEK_SET)); // go to start of file
        //initialize file with a 0, otherwise count won't increment properly
        assert(sizeof(count) == write(fd,&count,sizeof(count)));
    }
    sem_post(mutex); // exit crit region (increment)
    /******* end of critical region *******/
    CHK(close(fd));
} // end of initStudentStuff()

/* In this braindamaged version of placeWidget, the widget builders don't
 * coordinate at all, and merely print a random pattern. You should replace
 * this code with something that fully follows the p3 specification.
 */
void placeWidget(int n) {
    int fd;    //for opening counter file
    int count = 0; //counter for printing "N" and "F"

    sem_wait(mutex); // request enter critical region (decrement)
    /******* critical region for file manipulation and printing *******/
    //opening counter file
    CHK( fd = open("countfile",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR));
    CHK(lseek(fd,0,SEEK_SET)); //go to start of file
    assert(sizeof(count) == read(fd,&count,sizeof(count))); //read count
    count++; //increment count
    CHK(lseek(fd,0,SEEK_SET)); //go back to start of file
    assert(sizeof(count) == write(fd,&count,sizeof(count)));
    CHK(close(fd));  //close countfile

    printeger(n);
    if ( count == nrRobots*quota ) { // after last robot/pid printed
        printf("F\n");               
        fflush(stdout);
    }
    else if ( count % width == 0 ) { // if width is 2, then when count is 2,4,
        printf("N\n");          // 6,etc. % 2 = 0, which is the appropriate
        fflush(stdout);         // condition to print "N" 
    }
    sem_post(mutex); // exit crit region (increment)
    /****** end of critical region ******/

    if ( count == nrRobots*quota ) { //at the very end
        //clean-up semaphore after all robots have finished
        CHK(unlink("countfile")); 
        CHK(sem_close(mutex));
        CHK(sem_unlink(semMutx));
    }

} //end of placeWidget()
