 PRINT "select feature: ";
 PRINT "0=flash, 1=tstadc, 2=tstdac, 3=tstpwm, 4=alt, 5=end"
 INPUT f
 IF f=1 THEN tstadc:
 IF f=2 THEN tstdac:
 IF f=3 THEN tstpwm:
 IF f=4 THEN alt:
 IF f=5 THEN endprg:
 SETPIN 10,4
 SETPIN 11,4
 SETPIN 15,0
 SETPIN 16,0
flash: OUTD 10,1
 OUTD 11,0
 PRINT "15=",IND(15),"16=",IND(16)
 FOR i=1 TO 10000
 NEXT i
 OUTD 10,0
 OUTD 11,1
 PRINT "15=",IND(15),"16=",IND(16)
 FOR i=1 TO 10000
 NEXT i
 IF INKEY=-1 THEN flash:
 END
tstadc: SETPIN 30,1
 a=INADC(3)
 PRINT "a=",a
 OUTDAC 0,a
 IF INKEY=-1 THEN tstadc:
 END
tstdac: SETPIN 25,2
 INPUT a
 OUTDAC 0,a
 GOTO tstdac:
tstpwm: SETPIN 21,1
 PRINT "pwm5=";
 INPUT a
 PWM 5,100,a
 GOTO tstpwm:
alt: SETPIN 21,4
 OUTD 21,1
 PRINT "1";
 FOR i=1 TO 30000
 NEXT i
 OUTD 21,0
 PRINT "0";
 FOR i=1 TO 30000
 NEXT i
 IF INKEY=-1 THEN alt:
 END
endprg: END
