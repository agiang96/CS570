/* Filename  - getword.c
 * Author: Alexander Giang cssc0030
 * CS570 - Dr. John Carroll
 * Note: when typing input on the prompt, 
 *       use CTRL+D for EOF 
 */
#include "getword.h"

/*********** getword(char *w) *************
 * A simple lexical analyzer that will 
 * return the size of the word and store it
 * at the address given in parameter "*w".
 * There will be special conditions as  
 * stated as an "EXCEPTION" in "getword.h" 
 * that will redefine what a "word" is 
 * (i.e. '|&') and the outputs should change 
 * accordingly. 
 *****************************************/
int getword(char *w)
{
    int iochar;
    int size = 0;

    //while not at EOF (modified Foster's code)
    while ( size < STORAGE -1 && ( iochar = getchar() ) != EOF) {

        //while no leading spaces
        while ( iochar == ' ' && size == 0 )
            iochar = getchar();

        // prevents endless-loop with ungetc for newline
        if ( iochar == '\n' && size == 0 ) {
            w[size] = '\0';
            return -10;
        }
        // end of word (newline)
        if ( iochar == '\n') {
            w[size] = '\0';
            ungetc(iochar, stdin); 
            return size;
        }
        // end of word (space)
        if ( iochar == ' ') {
            w[size] = '\0';
            return size;
        }

        // prevents endless-loop with ungetc for <,>,&,#
        if ( (iochar == '<' || iochar == '>' || 
              iochar == '&' || iochar == '#') && size == 0 ) { 
            w[size] = iochar;
            w[++size] = '\0';
            return -1;
        }
        // metacharacters special condition (<,>,&)
        if ( iochar == '<' || iochar == '>' || 
             iochar == '&' ) { 
            w[size] = '\0';
            ungetc(iochar,stdin); //retrieve char to print
            return size;
        }

        //metacharacter condition (|,|&)
        if ( iochar == '|' ) {
            w[size] = iochar;
            if ( (iochar = getchar()) == '&' ) {
                w[++size] = iochar;
                w[++size] = '\0';            
                return -2; 
            }
            ungetc(iochar,stdin);
            w[++size] = '\0';
            return -1;
        }

        //metacharacter condition (\)
        if ( iochar == '\\' ) {
            iochar = getchar();
            if ( iochar != '\n' && iochar != EOF ) {
                w[size] = iochar;
                size++;        
            }
            else 
                ungetc(iochar,stdin);
        }

        //stores char until end of word
        else {
            w[size] = iochar;
            size++;
        }
   
    } // end of while

    if (size > 0) {
        w[size] = '\0';
        return size;
    }

// at EOF; null terminates and outputs 0
w[0] = '\0';
return 0;
}// end of getword()

