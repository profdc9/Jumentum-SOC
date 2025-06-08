REM Test Script
REM Tests the Interpreter
REM By Malcolm Mclean

PRINT "HERE" 
PRINT INSTR("FRED", "ED", 4)
PRINT VALLEN("12a"), VALLEN("xyz")
DIM A(10)
LET A(4)=40
INPUT A(5)
FOR I=1 TO 10
PRINT "A(",I,")=",A(I)
NEXT I
LET X = INT(A(5) + 1)
PRINT "X=",X
PRINT MID$("1234567890", X, -1)
