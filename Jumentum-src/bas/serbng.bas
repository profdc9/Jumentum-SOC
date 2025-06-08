REM program to test bgmicro.com serial vacuum
REM fluorescent display and/or bit bang serial output commands
REM note on 48 MHz LPC2148/2378 dly=295 is 9600 bps

t4: PRINT "test serial:"
 SETPIN 0,4
t5: mesg$="The time is "+TIMEGT$ 
 GOSUB skroll:
 IF INKEY<0 THEN t5:
 END

skroll: nskt=LEN(mesg$)+16 
 skt$=STRING$(16," ")+mesg$+STRING$(16," ")
 cskt=1
 cspd=100
 REPEAT
   te=SERBNG(0,295,CHR$(13)+MID$(skt$,cskt,16))
   WAIT cspd
   cskt=cskt+1
 UNTIL cskt>nskt
 RETURN

t1: PRINT "Test serial: "
 SETPIN 0,4
t3: PRINT "dly: ";
 INPUT dly
 IF dly=0 THEN nd:
 PRINT "text: ";
 INPUT ts$
 PRINT "output=",SERBNG(0,dly,ts$)
 GOTO t3:
