REM 
PRINT "number: ";
INPUT a
PRINT "rep:";
INPUT rp

REM x$=BIN$(x,<digs>)
REM Compute the string equivalent of the raw data value of x.  <digs>
REM is the number of bytes to use in x$.
b$=BIN$(a,rp)
PRINT "bin=",b$

REM x=BSTR(x$,<digs>)
REM Compute the numerical equivalent of the raw data value of x$.  <digs>
REM is the number of bytes to use in x$.
c=BSTR(b$,rp)
PRINT "bstr=",c
PRINT "binc=",BIN$(c,rp)