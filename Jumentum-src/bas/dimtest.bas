REM DIM A(<index 1>)
REM DIM A(<index 1>,<index 2>)
REM DIM A$(<index 1>)
REM DIM A$(<index 1>,<index 2>)
REM These commands create arrays of strings or numbers, either one or
REM two dimensional.  These can be initialized to particular values 
REM by using the =, e.g.
REM DIM A(10)=10,9,8,7,6,5,4,3,2,1
REM DIM NOTES$(7)="Do","Re","Mi","Fa","So","La","Ti"
REM 
DIM a(6,4)
FOR i=1 TO 6
FOR j=1 TO 4
a(i,j)=i*10+j
NEXT j
NEXT i
FOR i=1 TO 6
FOR j=1 TO 4
PRINT i,j,a(i,j)
NEXT j
NEXT i
