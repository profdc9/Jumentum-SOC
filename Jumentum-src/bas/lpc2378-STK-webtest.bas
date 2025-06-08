REM
PRINT "Test Web I/O commands"
lf$=CHR$(10)

REM x=TIMESET(<setstring>)
REM Sets the time, with a string in the format "YYYY-MM-DD HH:MM:SS".
tt$="2008-08-23 17-52-00"
x=TIMESET(tt$)

REM Make the pins as ADC 
SETPIN 23, 1
SETPIN 24, 1
SETPIN 12, 3
REM Make LED pin as output
SETPIN 21,4

REM 
OUTD 21,0

REPEAT
 PRINT "waiting for connection"
 REPEAT
  REM wc=WEBCON
  REM Get the web connection number for the currently open web connection to
  REM the controller, or -1 if no connection is currently open.
  wc=WEBCON
  IF INKEY<>-1 THEN fine:
 UNTIL wc<>-1

 REM x=WEBOUT(<webconnection>,<status>,<htmlstring>)
 REM Output the string <htmlstring> on the connection <webconnection>.  If the
 REM connection exists and the transmission is successful, x=0, otherwise
 REM x=-1.  If <status>=0 then continue the connection, otherwise if <status><0
 REM then terminate the connection after transmitting <htmlstring>.  Will
 REM block until data is transmitted, or current connection is terminated.
 code=WEBOUT(wc,0,"<META HTTP-EQUIV=""Refresh"" CONTENT=1><html><body><title>LPC2378-STK Test</title>")
 code=WEBOUT(wc,0,"<h2>LPC2378-STK Test</h2><br>")
 
 OUTD 21, IIF( IND(21), 0, 1)
 code=WEBOUT(wc,0,"Current LED Status: "+II$(IND(21),"<font color=""red"">ON</font>","OFF")+"<br>")
 
 Xval=0
 ADCnum=1
 GOSUB sampl:
 Xval=ss
 
 Yval=0
 ADCnum=0
 GOSUB sampl:
 Yval=ss
 
 Zval=0
 ADCnum=6
 GOSUB sampl:
 Zval=ss
 
 code=WEBOUT(wc,0,"Accelerometer Value:<br><ul>")
 code=WEBOUT(wc,0,"<li>X: "+STR$(Xval)+"<li>Y: "+STR$(Yval)+"<li>Z: "+STR$(Zval)+"</ul>")
 
 REM Gets the current time TO a string in the format "YYYY-MM-DD HH:MM:SS"
 x$=TIMEGT$
 code=WEBOUT(wc,-1,"Current Time is: "+x$+"<br></body></html>")
 
 
UNTIL 0

fine:
PRINT "ending"

REM Subroutine
sampl:
ss=0
FOR II=1 TO 4
 REM x=INADC(<ad #>)
 REM Gets the analog value at analog TO digital converter numbered AD0.<ad #>.  
 REM Must do a SETPIN first TO the appropriate pin TO set up the ADC
 ss=ss+INADC(ADCnum)
NEXT II
ss=ss/4
RETURN

END
