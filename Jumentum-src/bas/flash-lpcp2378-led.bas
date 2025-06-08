PRINT "hello there #4"
SETPIN 21,4
FOR i=1 TO 10
PRINT "adc=",INADC(0)
PRINT "time=",TIMEGT$
OUTD 21,0
FOR j=101 TO 200
PRINT j," ";
NEXT j
OUTD 21,1
FOR j=201 TO 300
PRINT j," ";
NEXT j
NEXT i
