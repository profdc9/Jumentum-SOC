30 PRINT "num x: ";
INPUT x
PRINT "num y: ";
INPUT y
PRINT "x/y=",x/y
GOTO 30
a=1
a=a/10
FOR i=1 TO 10
   a=a/100
   PRINT i," ",a
   NEXT i
a=-10
FOR i=1 TO 10
   a=a*100
   PRINT i," ",a
   NEXT i
