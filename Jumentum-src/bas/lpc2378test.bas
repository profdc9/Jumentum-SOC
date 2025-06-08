PRINT "LPC2378 Tests:"

REM x=TIMESET(<setstring>)
REM Sets the time, with a string in the format "YYYY-MM-DD HH:MM:SS".
tt$="2008-08-23 17-52-00"
PRINT "Setting Time: ",tt$
x=TIMESET(tt$)

loop1:
REM SETPIN <pin #>,<function>
REM SETPIN sets up the pin P0.<pin #> to have function denoted by number <function>.
REM Pin # is between 2 and 31 (pins 0 and 1 are reserved for UART0). What the
REM function number does depends on the pin.
REM
SETPIN 21,4
PRINT "Led Flashing..."

REM Loop for flashing...
FOR T=0 TO 3
 REM OUTD <pin #>,<state>
 REM Output a low for state=0, or a high for state=1 to the pin number
 REM P0.<pin #>.  Pin # is from 2 to 31. Must do SETPIN <pin #>,4
 REM first to set up the pin as a GPIO output.
 OUTD 21,0
 
 FOR I=0 TO 10000
 NEXT I
 REM 
 OUTD 21,1
 
 FOR I=0 TO 10000
 NEXT I
NEXT T

REM
OUTD 21,0

REM Make the pin23 as ADC 
SETPIN 23, 1

vadc=0
FOR I=1 TO 4
 REM x=INADC(<ad #>)
 REM Gets the analog value at analog to digital converter numbered AD0.<ad #>.  
 REM Must do a SETPIN first to the appropriate pin to set up the ADC
 vadc=vadc+INADC(0)
NEXT I
vadc=vadc/4

PRINT "ADC.0 Value: ",vadc

REM x$=TIMEGT$
REM Gets the current time TO a string in the format "YYYY-MM-DD HH:MM:SS"
x$=TIMEGT$
PRINT "Time is now: ",x$

GOTO loop1:
END

