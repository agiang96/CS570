/* Filename  - p2.c
 * Author: Alexander Giang cssc0030
 * CS570 - Dr. John Carroll
 * Note: when typing input on the prompt,
 *       use CTRL+D for EOF
 */
#include "p2.h"

char s[STORAGE*MAXITEM];//current line contents
char *newargv[MAXITEM]; //current command and args to be processed
char *INFILE, *OUTFILE; //stdin and stdout files
int argCount; //newargv count

int pipeArgs[MAXPIPES]; //pipe argument
int pandArgs[MAXPIPES]; //pipe-and argument
int fd[2*MAXPIPES]; //file descriptors: Read&Write for each pipe
int midptr; //reference pointer for middle pipe

// keeps track of '<','>','|','&','#','|&' characters
int inFlag = 0, outFlag = 0, pipeFlag = 0, andFlag = 0, hashFlag = 0;
int scriptFlag = 0; // keeps track if there's a script
                                                                                
int inFileOpen, outFileOpen,scriptFileOpen; // keeping track of files

// only using necessary read/write flags for files
int infile_flags = O_RDONLY;
int outfile_flags = O_RDWR | O_CREAT | O_EXCL;

// only using necessary user permissions
int usr_flags = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

void myhandler(int signum){
    //printf("Received SIGTERM and handler is running...\n");
}

int main(int argc, char* argv[]) {
    //declarations of locals;
    int nargc, child, mvlink; //for parse, piping, and MV respectively
    int devnull; //to redirect to /dev/null
    pid_t parent, pid; // for piping 

    DIR *dirp; //to check if it's a directory
    struct dirent *dp; // same as above
    int nflag, fflag; //for '-n' and '-f' in MV
    int mvargc; //counts how many arguments for mvargv
    char *mvargv[2]; //keeps track of the files for MV
    char *firstFile,*secondFile; //points to filename
    struct stat sbFirst, sbSecond; //to check if file exists
    int currpath; // path pointer to get 1st filename
    int slash; //keeps track of how many slashes to get 1st filename
    int fileExists; //check if file is in directory

    //any necessary set-up, including signal catcher and setpgid();
    int currpgrp = setpgrp();
    signal(SIGTERM, myhandler);

    // -----------------------scripting -----------------------
    if (argv[1] != NULL) { //if there is a script
        if( (scriptFileOpen = open(argv[1], infile_flags)) < 0) {
            fprintf(stderr, "Failed to open: %s\n", argv[1]);
        }
        scriptFlag++;
        if(dup2(scriptFileOpen, STDIN_FILENO) != 0) {
            fprintf(stderr, "Failed scriptfile open\n" );
            exit(1);
        }
       if(close(scriptFileOpen) != 0) {
            fprintf(stderr, "Failed scriptfile close\n" );
            exit(1);
       }
    }
    else if (argc > 2) { // too many scripts
        fprintf(stderr, "Too many scripts; can only accept one\n");
        exit(1);
    }
    
    for (;;) {
        //issue PROMPT if it's not a script
        if(scriptFlag == 0) {
            printf("p2: ");
        }
        //call your parse() function, setting [global] flags as needed;
        nargc = parse();
        //if (first word is EOF) break;
        if(nargc == -10)
            break;
        //if (line is empty) continue;
        if(nargc == 0)
            continue;
        //# commenting behavior  
        if(argCount != 0 && hashFlag == 1)
            continue;
        // setting up redirection for dup2()
        if(outFlag != 0) { //for output file
            if(argCount == 0) { //if no words in newargv
                fprintf(stderr,"Missing name for out redirection \n");
                continue;
            }
            else if(OUTFILE == '\0' ){ //no OUTFILE specified
                fprintf(stderr, "Invalid outfile for redirection \n");
                continue;
            }
            else {
                if(outFlag > 2) {
                    fprintf(stderr, "Failed to open writing file \n");
                    continue;
                }
                if((outFileOpen = open(OUTFILE, outfile_flags, usr_flags)) < 0){
                    fprintf(stderr,"%s: File already exists!\n", OUTFILE);
                    continue;
                }
            }
        } 
        if(inFlag != 0) { //set-up for input file
            if(argCount == 0) { //if no words in newargv
                fprintf(stderr,"Missing name for redirection \n");
                continue;
            }
            else if(INFILE == '\0'){ //no INFILE specified
                fprintf(stderr, "Missing infile for redirection \n");
                continue;
            }
            else {
                if(inFlag > 2) {
                    fprintf(stderr, "Failed to open reading file\n");
                    continue;
                }
                if((inFileOpen = open(INFILE, infile_flags)) < 0)  {
                    fprintf(stderr,"Unable to open: %s\n", INFILE);
                    continue;
                }
            }
        } // end of redirection set-up

    //********************** cd ***********************
        if(strcmp(newargv[0],"cd") == 0) {
            if(nargc > 2)
                fprintf(stderr, "Too many arguments. \n");
            if(nargc == 1) {
                char *homeDir = getenv("HOME");
                if(chdir(homeDir) != 0)
                    fprintf(stderr, "No home directory can be found. \n");
            }
            if(nargc == 2) {
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
            mvargc = 0;
            int x;
            for(x = 1; x < nargc ; x++) {
                if (strcmp(newargv[x], "-n") == 0) {
                    nflag = 1;
                    fflag = 0;
                }
                else if (strcmp(newargv[x], "-f") == 0) {
                    fflag = 1;
                    nflag = 0;
                }
                else {
                    //get the file names
                    if(mvargc < 2) {
                        mvargv[mvargc] = newargv[x];
                    }
                    mvargc++;
                }
            }

            // if the 1st path exists, get filename
            if (stat(mvargv[0], &sbFirst) == 0) {
                slash = 0;
                currpath = 0;
                while( *(mvargv[0] + currpath) != '\0' ) {
                    if(*(mvargv[0] + currpath) == '/'){
                        firstFile = mvargv[0] + currpath;
                        slash++;
                        currpath++;
                    }
                    else
                        currpath++;
                }
                if(slash == 0) { // no '/' so path = filename
                    firstFile = mvargv[0];
                }
            } 
            else {
                fprintf(stderr, "%s: File does not exist\n", mvargv[0]);
                continue;
            }
            // STAT/DIR Checks
            if (stat(mvargv[1], &sbSecond) == 0){ //if 2nd path exists
                if(S_ISDIR(sbSecond.st_mode)) { //if it's a directory
                    dirp = opendir(mvargv[1]);
                    while(dirp) {
                        //look at each file in directory
                        if((dp = readdir(dirp)) != NULL){
                            //check if file has same name as firstFile
                            if(strcmp(dp->d_name, firstFile) == 0) {
                                fileExists = 1;
                                break;
                            }
                        }
                        else{
                            closedir(dirp);
                            break;
                        }
                    }
                    if (fileExists == 1) { //file exists
                        //overwrite according to flags
                        if(nflag == 1) {
                            fprintf(stderr, "%s: File exists \n", mvargv[1]);
                        }
                        else if(fflag == 1) {
                            if((unlink(mvargv[1])) != 0) {
                                fprintf(stderr, "Cannot unlink old file '%s'\n", 
                                        mvargv[1]);
                            }

                        }
                    }
                strcat(mvargv[1],"/"); //concat / at the end of dir
                strcat(mvargv[1],firstFile); //concat filename at end of dir
                }// end of if-directory
                else{ //if it's a file
                    if(nflag == 1) {
                        fprintf(stderr, "%s: File exists \n", mvargv[1]);
                    }
                    else if(fflag == 1) {
                        if((unlink(mvargv[1])) != 0) {
                            fprintf(stderr, "Cannot unlink old file '%s' \n", 
                                    mvargv[1]);
                        }
                    }
                }
            }// end of stat/dir checks

            if(mvargc == 0)
                fprintf(stderr, "%s: missing file operand \n", newargv[0]);
            else if (mvargc == 1)
                fprintf(stderr,"MV: missing destination file after '%s'\n",
                                mvargv[0]);
            else if(mvargc > 2)
                fprintf(stderr, "Too many arguments \n");
            else {
                if((mvlink = link(mvargv[0],mvargv[1])) != 0)
                    fprintf(stderr,"MV: cannot move '%s': No such file or directory\n",
                            mvargv[1]);
                if(mvlink == 0 && (unlink(mvargv[0]) != 0))
                    fprintf(stderr, "Cannot unlink the old path '%s' \n", mvargv[0]);
            }
            continue;
        }
    //******************** end of MV *****************

    //************************ Piping ***********************
        if(pipeFlag > 0) {
            if(pipeFlag > MAXPIPES) {
                fprintf(stderr, "'|': p2 only supports 10 pipe. \n");
                continue;
            }
            else if(newargv[pipeArgs[pipeFlag-1]] == '\0' ) {
                fprintf(stderr, "Missing name for pipe \n");
                continue;
            }
            else
                piping();
            continue;
        }
    //***************** End of Piping *********************

        if(inFlag == 2 || outFlag == 2 || pipeFlag > 1 || hashFlag == 1 ||
            scriptFlag > 0) {
            fflush(stdout); //flushing stdout
        }
        fflush(stderr); //flushing stderr
        child = fork();
        if(child == 0) { //if (fork() == 0)
            //redirect I/O as requested;
            if(inFlag == 2 && outFlag != 2) {
                if( dup2(inFileOpen, 0) == -1) {
                    fprintf(stderr, "dup2 inFileOpen failed\n");
                    exit(1);
                }
                if (close(inFileOpen) != 0) {
                    fprintf(stderr, "close inFileOpen failed\n");
                    exit(1);
                }

            }
            //write to file
            if(inFlag != 2 && outFlag == 2 ) {
                if( dup2(outFileOpen, 1) == -1) {
                    fprintf(stderr, "dup2 outFileOpen failed\n");
                    exit(1);
                }
                if (close(outFileOpen) != 0 ) {
                    fprintf(stderr, "close outFileOpen failed\n");
                    exit(1);
                }

            }
            if(inFlag == 2 && outFlag == 2) {
                if( dup2(inFileOpen, 0) == -1) {
                    fprintf(stderr, "dup2 inFileOpen failed\n");
                    exit(1);
                }
                if( dup2(outFileOpen, 1) == -1) {
                    fprintf(stderr, "dup2 outFileOpen failed\n");
                    exit(1);
                }
                if (close(inFileOpen) != 0) {
                    fprintf(stderr, "close inFileOpen failed\n");
                    exit(1);
                }
                if (close(outFileOpen) != 0 ) {
                    fprintf(stderr, "close outFileOpen failed\n");
                    exit(1);
                }
            }

            //use execvp() to start requested process;
            //if the execvp() failed
            if(execvp(newargv[0], newargv) < 0) {
                //print an error message;
                fprintf(stderr,"%s: Command not found \n", newargv[0]);
                exit(9);
            }
        } //end of child/fork

        /* if appropriate, wait for child to complete;
         * else print the child's pid (and in this case, the child should
         * usually have redirected its stdin to /dev/null [if it does not
         * already have a '<' redirection to obey]);
         */
        else if(child > 0 ) { //parent
            if(andFlag != 0) {
                printf("%s [%d] \n", newargv[0], child);
                continue;

                //if there's a &' and no '<' send to /dev/null/
                if ( andFlag == 1 && inFlag == 0 ) {
                    if (devnull = open("/dev/null",infile_flags) < 0 ) {
                        fprintf(stderr, "failed to open /dev/null/ \n");
                        exit(1);
                    }
                    if (dup2(devnull,STDIN_FILENO) == -1){
                        fprintf(stderr, "dup2 devnull failed \n");
                        exit(1);
                    }
                    if (close(devnull) != 0 ) {
                        fprintf(stderr, "close devnull failed \n");
                        exit(1);
                    }
                }// end of elseif inFlag & andFlag
            }
            else { //waiting for child to finish
                for(;;) {
                    pid = wait(NULL);
                    if(child == pid)
                        break;
                }
            }
        }//end of parent-elseif
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
    int pointCurrArg = 0; //pointer to current arg
    int strlength;
    int ismeta = 0;
    //initializing all parameters to 0, since each parse() = new input
    int argvWord = 0;
    argCount = 0; // each new line initialize to 0
    inFlag = 0, outFlag = 0, pipeFlag = 0, andFlag = 0, hashFlag = 0;
    INFILE = '\0';
    OUTFILE = '\0';

    //while there are no line terminators
    while((strlength = getword( s + pointCurrArg)) != 0 && strlength != -10) {
        // if it's a metacharacter
        if (strlength <= -1 || inFlag == 1 || outFlag == 1) {
            //track the number of '&'
            if(*(s + pointCurrArg) == '&') {
                andFlag++;
                break;
            }
            //tracks '<' and passes in infile
            else if(*(s + pointCurrArg) == '<' || inFlag == 1) {
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
                newargv[argCount++] = '\0';
                pipeArgs[pipeFlag] = argCount;
                if(strlength == -2) { //checks if '|&'
                    pandArgs[pipeFlag] = 1;  //|& flag
                }
                pipeFlag++;
            }
   
            //'#' for commenting
            else if(*(s + pointCurrArg) == '#') {
                if(scriptFlag == 0 && pointCurrArg != 0) {
                    newargv[argCount++] = s + pointCurrArg;
                    s[pointCurrArg + 1] = '\0';
                    argvWord++; 
                }
                else if(pointCurrArg == 0) {
                    hashFlag = 1;
                    //get all words until EOF or \n
                    while((strlength = getword(s + pointCurrArg)) != 0 &&
                                strlength != -10) {
                        s[pointCurrArg + strlength] = '\0';
                        pointCurrArg += strlength + 1;
                        argvWord++;
                    }
                    return -1;
                }
                else if (scriptFlag > 0) {
                    while((strlength = getword(s + pointCurrArg)) != 0 &&
                                strlength != -10) {
                        s[pointCurrArg + strlength] = '\0';
                        pointCurrArg += strlength + 1;
                        argvWord++;
                    }
                    break;
                }
            } // end of '#' elseif

            if (inFlag == 2 || outFlag == 2)
                pointCurrArg += strlength + 1;
            else
                pointCurrArg += 2; //pointer + sizeof(hash) + 1
        } // end of if metachar
        else{ //put the word in newargv
            newargv[argCount++] = s + pointCurrArg;
            s[pointCurrArg + strlength] = '\0';
            pointCurrArg += strlength + 1;
        }
        argvWord++;
    }//end of while

    newargv[argCount] = '\0';
    if(argvWord == 0 && strlength != 0 ) return 0; //empty
    else if(argvWord == 0 && strlength == 0  ) return -10; //EOF
    else return argvWord;

} //end of parse()

void piping(){
    pid_t child, gchild;

    fflush(stdout);
    fflush(stderr);
    child = fork();
    if(child == 0) {
        if(pipe(fd) < 0) {
            fprintf(stderr,"Couldn't pipe \n");
            exit(9);
        }
        if(pipeFlag > 1) { //lets last child know of end fd's
        	if(pipe((fd + 2*(pipeFlag-1))) < 0) {
        		fprintf(stderr, "Couldn't pipe \n");
        		exit(9);
        	}
        }
        gchild = fork();
        if(gchild == 0) { //grandchild works with child
            if((dup2(fd[2*pipeFlag-1], STDOUT_FILENO)) < 0) {
                fprintf(stderr, "Bad file descriptor \n");
                exit(9);
            } 
            if(pandArgs[0] == 1) { //'|&' dup2() to STDERR
                if(dup2(fd[2*pipeFlag-1], STDERR_FILENO) < 0) {
                fprintf(stderr, "Bad file descriptor \n");
                exit(9);
                }
            }
            //closing inherited fd's from parent
            close(fd[0]);
            close(fd[1]);
            if(pipeFlag > 1) {
                //close end fd from parent
                close(fd[2*(pipeFlag-1)]);
                close(fd[2*(pipeFlag-1)+1]);
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
        else { //1st child
            
            if(pipeFlag > 1) {
            	//sets mid pipe pointer to point at next mid pipe
            	midptr = 1; 
            	midpiping(); 
            }
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

void midpiping() {
	pid_t child;
	int currptr; //current pointer
	int i; //for-loop for inherited fd's
	int j; //for-loop for other fd's

	//open all pipes but the last one since parent already opened it
	if( (midptr+1) < pipeFlag) {
		if(pipe(fd+2*midptr) < 0) {
			fprintf(stderr, "Couldn't midpipe\n");
			exit(9);
		}
	}
	child = fork();
	if(child == 0) { //grandest child
		currptr = midptr;
		if(currptr < pipeFlag - 1) {
			midptr++;
			midpiping();
		}
		if( dup2(fd[2*currptr],STDIN_FILENO) < 0){ //redir stdin
			fprintf(stderr, "Bad infile descriptor \n");
			exit(9);
		}  
			
		if( dup2(fd[2*currptr - 1],STDOUT_FILENO) < 0) { //redir stdout
			fprintf(stderr, "Bad outfile descriptor \n");
			exit(9);
		}
        if(pandArgs[currptr] == 1) { //'|&' dup2() to STDERR
            if(dup2(fd[2*currptr - 1], STDERR_FILENO) < 0) {
                fprintf(stderr, "Bad file descriptor \n");
                exit(9);
            }
        }
		for(i = 0; i < 2*currptr+2; i++) { //close all inherited fd
			close(fd[i]);  
		}
        if((execvp(newargv[pipeArgs[pipeFlag - 1 - currptr]], 
            newargv+pipeArgs[pipeFlag - 1 - currptr])) < 0) {
            fprintf(stderr, "%s: command not found \n", 
                newargv[pipeArgs[pipeFlag - currptr - 1]]);
            exit(9);
        }
	}// end of if-child
	else if (child > 0) { //child
		if(midptr == 1) { //first pipe behaves differently
            // without this check, the next lines re-close fd[2] and fd[3]
            // without close(fd[2 and 3]), there's a deadlock. 
            if ( pipeFlag != 2) { 
                close(fd[2]); //close first read in midpipes
                close(fd[3]); //close first write in midpipes
            }
            close(fd[2*pipeFlag-1]);//close last write
            close(fd[2*pipeFlag-2]);//close last read
        }
		else { //close the rest of fd
			for(j = 2*midptr; j < 2*pipeFlag; j++) {
				close(fd[j]);
		    }
	    } 
    }// end of elseif-child 
    else {
        fprintf(stderr,"failed middle piping behavior\n");
        exit(9);
    }
} // end of midpiping
    