REM 
REM x=TIMESET(<setstring>)
REM Sets the time, with a string in the format "YYYY-MM-DD HH:MM:SS".
tt$="2008-08-23 17-52-00"
PRINT "Setting Time: ",tt$
x=TIMESET(tt$)

FOR I=0 TO 40000
NEXT I

REM x$=TIMEGT$
REM Gets the current time to a string in the format "YYYY-MM-DD HH:MM:SS"
x$=TIMEGT$
PRINT "Time is now: ",x$