REM 
PRINT "LPC2378-STK board Tests:"
GOSUB clears:

REM x=TIMESET(<setstring>)
REM Sets the time, with a string in the format "YYYY-MM-DD HH:MM:SS".
tt$="2008-08-23 17-52-00"
x=TIMESET(tt$)

REM Make the pins as ADC 
SETPIN 23, 1
SETPIN 24, 1
SETPIN 12, 3
REM Make the pin as output
SETPIN 21,4

loop1:

Xadc=0
FOR I=1 TO 4
 REM x=INADC(<ad #>)
 REM Gets the analog value at analog TO digital converter numbered AD0.<ad #>.  
 REM Must do a SETPIN first TO the appropriate pin TO set up the ADC
 Xadc=Xadc+INADC(1)
NEXT I
Xadc=Xadc/4

Yadc=0
FOR I=1 TO 4
 REM x=INADC(<ad #>)
 REM Gets the analog value at analog TO digital converter numbered AD0.<ad #>.  
 REM Must do a SETPIN first TO the appropriate pin TO set up the ADC
 Yadc=Yadc+INADC(0)
NEXT I
Yadc=Yadc/4

Zadc=0
FOR I=1 TO 4
 REM x=INADC(<ad #>)
 REM Gets the analog value at analog TO digital converter numbered AD0.<ad #>.  
 REM Must do a SETPIN first TO the appropriate pin TO set up the ADC
 Zadc=Zadc+INADC(6)
NEXT I
Zadc=Zadc/4

LOCATE 3,0
CLREOL
PRINT "X Value: ",Xadc
LOCATE 4,0
CLREOL
PRINT "Y Value: ",Yadc
LOCATE 5,0
CLREOL
PRINT "Z Value: ",Zadc

REM x$=TIMEGT$
REM Gets the current time TO a string in the format "YYYY-MM-DD HH:MM:SS"
x$=TIMEGT$
LOCATE 1,0
CLREOL
PRINT "Time : ",x$

OUTD 21, IIF( IND(21), 0, 1)

FOR I=0 TO 10000
NEXT I

GOTO loop1:

clears:
FOR S=0 TO 24
 LOCATE S,0
 CLREOL
NEXT S
RETURN
END

