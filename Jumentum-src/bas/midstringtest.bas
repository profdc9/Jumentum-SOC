A$="Hello World"
B$="Jumentum"

PRINT "First String: ",A$
PRINT "Second String: ",B$

REM x$=MID$(<string>,<index>,<num characters>)
REM Returns the <num characters> characters starting at index <index>
REM in string <string> as a new string.  If <num characters>=-1 or
REM is omitted then the remainder of the string to the end is returned.
MID$(A$,7)=B$

PRINT "Result: ",A$

