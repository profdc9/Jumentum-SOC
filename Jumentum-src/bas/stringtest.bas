PRINT "Some tests about strings"
PRINT ""
loop:
PRINT "Insert the first string A$:";
INPUT A$
PRINT "Insert the second string B$:";
INPUT B$
IF A$ >= B$ THEN loop1:
PRINT "A$<B$"
loop1:
IF A$ <> B$ THEN loop2:
PRINT "A$=B$"
loop2:
IF A$ <= B$ THEN loop3
PRINT "A$>B$"
loop3:
PRINT "A$+B$=",A$+B$

REM x=INSTR(<search string>,<substring>,<index>)
REM Searches for the first occurrence of string <substring> in
REM the string <search string> that is at or after the character
REM index <index>, with <index>=1 being the first character.
REM If the substring is not found, zero is returned.
PRINT "INSTR(A$,B$,1)=",INSTR(A$,B$,1)

REM x=LEN(<string>)
REM Returns the number of characters in <string>.
PRINT "LEN+NULL=",LEN(A$+CHR$(0)+B$)

REM x$=STRING$(<repeats>,<string>)
REM Return a string with the string <string> repeated <repeats> times.
REM It is very easy to exhaust the memory with this command.
PRINT "STRING$(5,A$)=",STRING$(5,A$)

REM x=VALLEN(<string>)
REM Returns the number of characters in the numerical representation of
REM the string <string>.
PRINT "VALLEN(A$)=",VALLEN(A$)

PRINT "STRING$(0,A$)=",STRING$(0,A$)

REM x$=LEFT$(<string>,<num characters>)
REM Returns the first <num characters> characters of string <string>.
PRINT "LEFT=",LEFT$(A$,3)
PRINT "LEFTALL=",LEFT$(A$,10000)

REM x$=RIGHT$(<string>,<num characters>)
REM Returns the last <num characters> characters of string <string>.
PRINT "RIGHT=",RIGHT$(A$,3)
PRINT "RIGHTALL=",RIGHT$(A$,10000)

REM x$=MID$(<string>,<index>,<num characters>)
REM Returns the <num characters> characters starting at index <index>
REM in string <string> as a new string.  If <num characters>=-1 or
REM is omitted then the remainder of the string to the end is returned.
PRINT "MID=",MID$(A$,2,3)
PRINT "MIDALL=",MID$(A$,2,-1)

REM x=VAL(<string>)
REM Returns a numerical representation of the string <string>.
PRINT "VAL=",VAL(A$)
GOTO loop:
