/* Filename  - getword.c
 * Author: Alexander Giang cssc0030
 * CS570 - Dr. John Carroll
 * Note: when typing input on the prompt, 
 *       use CTRL+D for EOF 
 */
#include "getword.h"

int getword(char *w)
{
    int iochar;
    int size = 0;

    //while not at EOF (modified Foster's code)
    while ( ( iochar = getchar() ) != EOF ) {

        //while no leading spaces
        while ( iochar == ' ' && size == 0 )
            iochar = getchar();

        //deals with '\n' as a result of ungetc
        if ( iochar == '\n' && size == 0 ) {
            w[0] = '\0';
            return -10;
        }

        // if end of word: newline
        if ( iochar == '\n') {
            w[size] = '\0';
            ungetc(iochar, stdin);
            return size;
        }

        // if end of word: space
        if ( iochar == ' ') {
            w[size] = '\0';
            return size;
        }

        // if '#' return -1
        if ( iochar == '#' && size == 0) { 
            w[size] = iochar;
            w[++size] = '\0';
            return -1;
        }

        //adds char until \n,space, or EOF & increase size
        else {
            w[size++] = iochar;
        }
    } // end of while

if (size > 0) {
    w[size] = '\0';
    return size;
}

// at EOF, so null terminates and outputs 0
w[0] = '\0';
return 0;
}// EOF

