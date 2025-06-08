 REM example TO program hd44780 LCD no network
 REM ngxtechnologies.com nxLPC2148
 GOSUB intlcd:

outpt:
 PRINT "Message: ";
 INPUT mesg$
 IF mesg$="" THEN endp:
 GOSUB lcdout:
 GOTO outpt:  
endp: END

REM subroutine TO output TO hd44780
REM assume pin 124=RS(pin 4 LCD), pin 122=E(pin 6), pin 10=DB4(pin 11),
REM        pin 11=DB5(pin 12), pin 12=DB6(pin 13), pin 13=DB7(pin 14)
REM        pin 123=RW(pin 5 LCD) is held low        
intlcd: 
 nlines=1
 nchars=16
 SETPIN 124,4
 SETPIN 122,4
 SETPIN 123,4
 OUTD 123,0
 SETPIN 10,4
 SETPIN 11,4
 SETPIN 12,4
 SETPIN 13,4
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
 OUTD 124,cmd
 OUTD 122,0
 OUTD 10,(byt & 0x10)<>0
 OUTD 11,(byt & 0x20)<>0
 OUTD 12,(byt & 0x40)<>0
 OUTD 13,(byt & 0x80)<>0
 OUTD 122,1
 OUTD 122,0
 OUTD 10,(byt & 0x01)<>0
 OUTD 11,(byt & 0x02)<>0
 OUTD 12,(byt & 0x04)<>0
 OUTD 13,(byt & 0x08)<>0
 OUTD 122,1
 OUTD 122,0
 RETURN
