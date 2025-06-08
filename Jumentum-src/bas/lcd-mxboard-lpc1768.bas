 REM example TO program hd44780 LCD no network
 REM ngxtechnologies.com nxLPC1768
 GOSUB intlcd:

outpt:
 PRINT "Message: ";
 INPUT mesg$
 IF mesg$="" THEN endp:
 GOSUB lcdout:
 GOTO outpt:  
endp: END

REM subroutine TO output TO hd44780
REM assume pin 205=RS(pin 4 LCD), pin 207=E(pin 6), pin 19=DB4(pin 11),
REM        pin 20=DB5(pin 12), pin 21=DB6(pin 13), pin 22=DB7(pin 14)
REM        pin 206=RW(pin 5 LCD) is held low        
intlcd: 
 nlines=1
 nchars=16
 SETPIN 205,4
 SETPIN 207,4
 SETPIN 206,4
 OUTD 206,0
 SETPIN 19,4
 SETPIN 20,4
 SETPIN 21,4
 SETPIN 22,4
 cmd=0
 byt=0x33
 GOSUB lclock:
 byt=0x32
 GOSUB lclock:
 byt=0x28+(nlines-1)*4
 GOSUB lclock:
 byt=0x0E
 GOSUB lclock:
 byt=0x06
 GOSUB lclock:
 RETURN
lcdout:
 cmd=0
 byt=0x01
 GOSUB lclock:
 byt=0x02
 GOSUB lclock:
 byt=0x80
 GOSUB lclock:
 cmd=1
 FOR ch=1 TO LEN(mesg$)
 byt=ASCII(MID$(mesg$,ch,1))
 GOSUB lclock:
 IF ch<>nchars THEN nextc:
 cmd=0
 byt=0xC0
 GOSUB lclock:
 cmd=1
 nextc: NEXT ch
 RETURN
lclock:
 OUTD 205,cmd
 OUTD 207,0
 OUTD 19,(byt & 0x10)<>0
 OUTD 20,(byt & 0x20)<>0
 OUTD 21,(byt & 0x40)<>0
 OUTD 22,(byt & 0x80)<>0
 OUTD 207,1
 OUTD 207,0
 OUTD 19,(byt & 0x01)<>0
 OUTD 20,(byt & 0x02)<>0
 OUTD 21,(byt & 0x04)<>0
 OUTD 22,(byt & 0x08)<>0
 OUTD 207,1
 OUTD 207,0
 RETURN
