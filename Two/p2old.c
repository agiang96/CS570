/* Filename  - p2.c
 * Author: Alexander Giang cssc0030
 * CS570 - Dr. John Carroll
 * Note: when typing input on the prompt,
 *       use CTRL+D for EOF
 */
#include "p2.h"

char s[STORAGE*MAXITEM];
char *newargv[MAXITEM];
char *INFILE, *OUTFILE;
int argCount = 0;
int pipeArgs[1]; //pipe argument

// keeps track of '<','>','|','&','#' characters
int inFlag = 0, outFlag = 0, pipeFlag = 0, andFlag = 0, hashFlag = 0;
int inFileOpen, outFileOpen; // keeping track of files

// only using necessary read/write flags
// ref: http://pubs.opengroup.org/onlinepubs/7908799/xsh/open.html
int infile_flags = O_RDONLY;
int outfile_flags = O_RDWR | O_CREAT | O_EXCL;

// only using necessary user permissions
// ref: http://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html
int usr_flags = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

void myhandler(int signum){
    //printf("Received SIGTERM and handler is running...\n");
}

int main() {

    //declarations of locals;
    int argc, child, mvlink;
    pid_t parent, pid;
    //any necessary set-up, including signal catcher and setpgid();
    int currpgrp = setpgrp();
    signal(SIGTERM, myhandler);

    for (;;) {
        //issue PROMPT
        printf("p2: ");
        //call your parse() function, setting [global] flags as needed;
        argc = parse();

        //if (first word is EOF) break;
        if(argc == -10)
            break;
        //if (line is empty) continue;
        if(argc == 0)
            continue;

    //********************** cd ***********************
        if(strcmp(newargv[0],"cd") == 0) {
            if(argc > 2)
                fprintf(stderr, "Too many arguments. \n");
            if(argc == 1) {
                char *homeDir = getenv("HOME");
                if(chdir(homeDir) != 0)
                    fprintf(stderr, "No home directory can be found. \n");
            }
            if(argc == 2) {
                if(chdir(newargv[1]) != 0) {
                    fprintf(stderr, "%s: No such file/directory. \n",
                                newargv[1]);
                }
            }
            continue;
        }
    //******************** end of cd *******************

    //*********************** MV ***********************
        if(strcmp(newargv[0], "MV") == 0) {

            if(argc == 1)
                fprintf(stderr, "%s: missing file operand \n", newargv[0]);
            else if (argc == 2)
                fprintf(stderr,
                                "%s: missing destination file operand after '%s'
 \n",
                                newargv[0], newargv[1], newargv[1]);
            if(argc > 3)
                fprintf(stderr, "%s: target '%s' is not a directory \n",
                                newargv[0], newargv[argCount-1]);
            else {
                if((mvlink = link(newargv[1],newargv[2])) != 0)
                        fprintf(stderr,
                                "%s: cannot move '%s': No such file or directory
 \n",
                                        newargv[0], newargv[1], newargv[2]);
                                if(mvlink == 0 && (unlink(newargv[1]) != 0))
                                        fprintf(stderr, "Cannot unlink the old p
ath '%s' \n",
                                                        newargv[2]);
            }
            continue;
        }
    //******************** end of MV *****************

        // setting up redirection for dup2()
        if(outFlag != 0) { //for output file
            if(outFlag > 2) {
                fprintf(stderr,
                                "Ambiguous behavior. Failed to open writing file
 \n");
                continue;
            }
            if((outFileOpen = open(OUTFILE, outfile_flags, usr_flags)) < 0) {
                fprintf(stderr,"%s: File already exists!\n", OUTFILE);
                continue;
            }
        }
        if(inFlag != 0) { //input file
            if(inFlag > 2) {
                fprintf(stderr,
                                "Ambiguous behavior. Failed to open reading file
 \n");
                continue;
            }
            if((inFileOpen = open(INFILE, infile_flags)) < 0)  {
                fprintf(stderr,"Unable to open: %s\n", INFILE);
                continue;
            }
        }

    //************************ Piping ***********************
        if(pipeFlag > 0) {
            if(pipeFlag > 2) {
                fprintf(stderr, "'|': p2 only supports 1 pipe. \n");
                continue;
            }
            else
                piping();
            continue;
        }
    //***************** End of Piping *********************

        if(inFlag == 2 || outFlag == 2 || pipeFlag > 1 || hashFlag == 1) {
            fflush(stdout); //flushing stdout
        }
        fflush(stderr); //flushing stderr
        child = fork();

        //if (fork() == 0)
        if(child == 0) {
            //redirect I/O as requested;
            if(inFlag == 2 && outFlag != 2) {
                dup2(inFileOpen, 0);
                close(inFileOpen);
            }
            if(inFlag != 2 && outFlag == 2 ) { //write to file
                dup2(outFileOpen, 1);
                close(outFileOpen);
            }
            if(inFlag == 2 && outFlag == 2) {
                dup2(inFileOpen, 0);
                dup2(outFileOpen, 1);
                close(inFileOpen);
                close(outFileOpen);
            }

            //use execvp() to start requested process;
            //if the execvp() failed
            if(execvp(newargv[0], newargv) < 0) {
                //print an error message;
                fprintf(stderr,"%s: Command not found \n", newargv[0]);
                exit(9);
            }
        }
        /* if appropriate, wait for child to complete;
         * else print the child's pid (and in this case, the child should
         * usually have redirected its stdin to /dev/null [if it does not
         * already have a '<' redirection to obey]);
         */
        else if(child > 0 ) { //parent
            if(andFlag != 0) {
                printf("%s [%d] \n", newargv[0], child);
                continue;
            }
            else { //waiting for child to finish
                for(;;) {
                    pid = wait(NULL);
                    if(child == pid)
                        break;
                }
            }
        }
        else {
            fprintf(stderr, "Fork failed \n");
            return;
        }
    } // end of for(;;)

 //   CAUTION: You must use this next killpg() line verbatim, but
 //   if you haven't correctly used setpgid() earlier, this killpg()
 //   will likely kill the autograder and perhaps your login shell as well!
     killpg(getpgrp(), SIGTERM);// Terminate any children that are
                                // still running. WARNING: giving bad args
                                // to killpg() can kill the autograder!
     printf("p2 terminated.\n");// MAKE SURE this printf comes AFTER killpg

     exit(0);

}// end of main()

int parse(){
    int pointCurrArg; //pointer to current arg
    int strlength;
    //initializing all parameters to 0, since each parse() = new input
    int argvWord = pointCurrArg = argCount = 0;
    inFlag = 0, outFlag = 0, pipeFlag = 0, andFlag = 0, hashFlag = 0;
    INFILE = '\0';
    OUTFILE = '\0';

    //while there are no line terminators
    while((strlength = getword( s + pointCurrArg)) != 0 && strlength != -10) {
        //tracks '<' and passes in infile
        if(*(s + pointCurrArg) == '<' || inFlag == 1) {
            inFlag++;
            if(inFlag == 2) {
                INFILE = s + pointCurrArg;
            }
        }
        //tracks '>' and passes in outfile
        else if(*(s + pointCurrArg) == '>' || outFlag == 1) {
            outFlag++;
            if(outFlag == 2) {
                OUTFILE = s + pointCurrArg;
            }
        }
        //track the number of '|' and following argument
        else if(*(s + pointCurrArg) == '|') {
            newargv[argvWord++] = '\0';
            pipeArgs[pipeFlag] = argvWord;
            pipeFlag++;
        }
        //track the number of '&'
        else if(*(s + pointCurrArg) == '&') {
            andFlag++;
            break;
        }
        //'#' for commenting
        else if(*(s + pointCurrArg) == '#') {
            newargv[argvWord++] = s + pointCurrArg;
            s[pointCurrArg + 1] = '\0';
            argCount++;
            if(pointCurrArg == 0) {
                pointCurrArg = pointCurrArg + 2; //pointer + sizeof(hash) + 1
                hashFlag = 1;
                //get all words until EOF or \n
                while((strlength = getword(s + pointCurrArg)) != 0 &&
                                strlength != -10) {
                    s[pointCurrArg + strlength] = '\0';
                    pointCurrArg += strlength + 1;
                    argCount++;
                }
                return -1;
            }
        } // end of '#' elseif
        else    //put the word in newargv
            newargv[argvWord++] = s + pointCurrArg;

        s[pointCurrArg + strlength] = '\0';
        pointCurrArg += strlength + 1;
        argCount++;
    }//end of while

    newargv[argvWord] = '\0';
    if(strlength != 0 && argCount == 0) return 0; //empty
    else if(strlength == 0 && argCount == 0) return -10; //EOF
    else return argCount;

} //end of parse()

void piping(){
    int fd[2]; //file descriptors fd[0]:Read fd[1]:Write
    pid_t child, gchild;

    fflush(stdout);
    fflush(stderr);
    child = fork();
    if(child == 0) {
        if(pipe(fd) < 0) {
            fprintf(stderr,"Couldn't pipe \n");
            exit(9);
        }
        gchild = fork();
        if(gchild == 0) { //grandchild works with child
            if((dup2(fd[1], STDOUT_FILENO)) < 0) {
                fprintf(stderr, "Bad file descriptor \n");
                exit(9);
            }
            //closing inherited fd's from parent
            if(close(fd[0]) != 0) {
                fprintf(stderr, "fd[0] close failed \n");
                exit(9);
            }
            if(close(fd[1]) != 0) {
                fprintf(stderr, "fd[1] close failed \n");
                exit(9);
            }
            //redirect stdin from file if designated
            if(inFlag == 2) {
                if((dup2(inFileOpen, STDIN_FILENO))<0) {
                    fprintf(stderr, "Can't open read file \n");
                    exit(9);
                }
                if(close(inFileOpen) != 0) {
                        fprintf(stderr, "inFile close failed \n");
                        exit(9);
                }
            }
            if(execvp(newargv[0], newargv) < 0) { //run the command
                fprintf(stderr, "%s: command not found, \n", newargv[0]);
                exit(9);
            }
        }// end of gchild if()
        else {
            if(dup2(fd[0], STDIN_FILENO) < 0) { //redirect stdin from fd[0]
                fprintf(stderr, "Bad file descriptor \n");
                exit(9);
            }
            if(close(fd[0]) != 0) {
                fprintf(stderr, "fd[0] close failed \n");
                exit(9);
            }
            if(close(fd[1]) != 0) {
                fprintf(stderr, "fd[1] close failed \n");
                exit(9);
            }

            if(outFlag == 2) { //redirects stdout to file (if given)
                if(dup2(outFileOpen, STDOUT_FILENO) < 0) {
                    fprintf(stderr, "Couldn't redirect output to file \n");
                    exit(9);
                }
                if(close(outFileOpen) != 0) {
                        fprintf(stderr, "outFile close failed \n");
                        exit(9);
                }
            }
            if((execvp(newargv[pipeArgs[pipeFlag-1]], //run the command
                        newargv+pipeArgs[pipeFlag-1])) < 0) {
                fprintf(stderr,"%s: command not found \n",
                                newargv[pipeArgs[pipeFlag-1]]);
                exit(9);
            }
        }//end of gchild else
    } // end of child if()
    for(;;) { //wait for child processes to finish
        pid_t pid;
        pid = wait(NULL);
        if( pid == child)
            break;

    }
} // end of piping()
