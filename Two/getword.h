/* getword.h - header file for the getword() function used in
   CS570 Spring 2018
   San Diego State University
*/

#include <stdio.h>
#include <string.h>
#include <strings.h>

#define STORAGE 255
      /* This is one more than the max wordsize that getword() can handle */

int getword(char *w);
/* (Note: the preceding line is an ANSI C prototype statement for getword().
    It will work fine with edoras' C compilers.)

* The getword() function gets one word from the input stream.
* It returns 0 when there is no word, and only end-of-file is encountered;
* It returns -10 when there is no word, and only newline is encountered
* Otherwise, it returns the number of characters in the word
* EXCEPTION: return the *negative* of the length for metacharacters, e.g.,
* "|" returns -1, "|&" returns -2, etc.
*
* INPUT: a pointer to the beginning of a character string
* OUTPUT: -1 or the number of characters in the word
* SIDE EFFECTS: bytes beginning at address w will be overwritten.
*   Anyone using this routine should have w pointing to an
*   available area at least STORAGE bytes long before calling getword().

Upon return, the string pointed to by w contains the next word in the line
from stdin. A "word" is a string containing a single metacharacter or a string
consisting of non-metacharacters delimited by blanks or metacharacters
(newline and EOF also terminate strings).

The metacharacters are "<", ">", "|", "&", "#", and the combination "|&" .
The last word on a line may be terminated by the newline character OR by
end-of-file.  Word collection is "greedy": getword() always tries each time
to read the largest word that does not violate the rules.  For example, |&foo
is parsed as "|&" and then "foo", NOT as "|", "&", and "foo" .

getword() skips leading blanks, so if getword() is called and there are
no more words on the line, then w points to an empty string. All strings,
including the empty string, will be delimited by a zero-byte (eight 0-bits),
as per the normal C convention (this delimiter is not 'counted' when determining
the length of the string that getword() will report as a return value).

The backslash character "\" is special, and may change the behavior of
the character that directly follows it on the input line.  When "\" precedes
a metacharacter, that metacharacter is treated like most other characters.
(That is, the symbol will be part of a word rather than a word delimiter.)

Thus, three calls applied to the input
Null&void
will return 4,-1,4 and produce the strings "Null", "&", "void", respectively.

However, one call to getword() applied to the input
Null\&void
returns 9 and produces the string "Null&void".
Note that the '\' is NOT part of the resulting string!

Similarly, "\<" is treated as the [non-meta]character "<", "\>" is ">",
"\&" is "&", "\|" is "|", and "\\" represents the [non-special] character "\".
The combination "\ " should be treated as " ", and therefore
allow a space to be embedded in a word:
Null\ void
returns 9 and produces the string "Null void".
(A backslash preceding any other character should simply be ignored; in
particular, a backslash before a newline will not affect the meaning of that
newline.)  By these rules, "|&" returns -2, "\|\&" returns +2, whereas "\|&"
requires TWO calls to getword(), returning 1 with "|" and then -1 with "&".
That is, when '\' causes you to treat a metacharacter as a 'regular' character,
return a positive number, not a negative number.

Generally, the integer that getword() returns is the length of the resultant
string to which w points. There are exceptions to this: If the rest of
the line consists of zero or more blanks followed by end-of-file, then w
still points to an empty string, but the returned integer is 0; blanks followed
by a newline returns -10, and metacharacters generally return negative numbers.

Example: Suppose the input line were
Hi there&  
(Assume there are two trailing spaces, followed by a newline character.)
Four calls to getword(w) would return 2,5,-1,-10 and fill each of the
areas pointed to by w with the strings "Hi", "there", "&", and "",
respectively.  (If EOF followed the newline, then a fifth call would
produce "" and return 0.)

Note that we would obtain exactly the same series of results if the input
line had been
    Hi   there     &
(This example has leading blanks and a newline right after the ampersand.)

'#' follows somewhat different rules than the other metacharacters; it is NEVER
treated as special unless it is the first character in a word (and even then,
a '\' would make it non-special.  For example:
#hi requires *2* calls to getword(), forming "#" with -1, and then "hi" with +2.
\#hi creates "#hi" and returns 3
h#i creates "h#i" and also returns 3

If the word scanned is longer than STORAGE-1, then getword() constructs the
string consisting of the first STORAGE-1 bytes only. (As usual, a zero-byte
is appended. The next getword() call will begin with the rest of that word.)

Useful manpages to consider are those for ungetc() and getchar().

*/
