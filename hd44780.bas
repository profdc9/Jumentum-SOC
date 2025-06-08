 REM example TO program hd44780 LCD
 br$="<br>"+CHR$(13)+CHR$(10)
 mesg$=""
 GOSUB intlcd: 
REPEAT
REPEAT
 wc=WEBCON
 IF INKEY<>-1 THEN endprg:
UNTIL wc<>-1
 w$=WEBREQ$(wc,0)
 IF INSTR(w$,"/doit",1)=0 THEN prform:
 a$=WEBPST$(wc,200)
 mesg$=WEBFRM$(a$,"message")
 GOSUB lcdout:
 code=WEBOUT(wc,0,"<h1>Sending message to LCD display</h1>"+br$)
 code=WEBOUT(wc,0,"Message: "+WEBESC$(mesg$)+br$+br$)
prform: code=WEBOUT(wc,0,"<h1>Enter data to be displayed by LCD</h1>"+br$+"<form method=post action=""/bas/doit"">")
 code=WEBOUT(wc,0,"Message: <input type=text name=""message"" value="""+WEBESC$(mesg$)+""">"+br$+br$)
 code=WEBOUT(wc,0,"<input type=submit name=""go"" value=""Display Message""</form>"+br$)
endcon: code=WEBOUT(wc,-1,"")
UNTIL 0
endprg: PRINT "ending"
 END

REM subroutine TO output TO hd44780
REM assume pin 11=RS(pin 4 LCD), pin 12=E(pin 6), pin 13=DB4(pin 11),
REM        pin 14=DB5(pin 12), pin 15=DB6(pin 13), pin 16=DB7(pin 14)
REM        pin 5 is held low on LCD        
intlcd: 
 nlines=1
 nchars=40
 SETPIN 11,4
 SETPIN 12,4
 SETPIN 13,4
 SETPIN 14,4
 SETPIN 15,4
 SETPIN 16,4
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
 FOR ch=1 TO LEN(msg$)
 byt=asc(MID$(msg$,ch,1))
 GOSUB lclock:
 IF ch<>nchars THEN nextc:
 cmd=0
 byt=0xC0
 GOSUB lclock:
 cmd=1
 nextc: NEXT ch
 RETURN
lclock:
 OUTD 11,cmd
 OUTD 12,0
 OUTD 13,(byt & 0x10)<>0
 OUTD 14,(byt & 0x20)<>0
 OUTD 15,(byt & 0x40)<>0
 OUTD 16,(byt & 0x80)<>0
 OUTD 12,1
 OUTD 12,0
 OUTD 13,(byt & 0x01)<>0
 OUTD 14,(byt & 0x02)<>0
 OUTD 15,(byt & 0x04)<>0
 OUTD 16,(byt & 0x08)<>0
 OUTD 12,1
 OUTD 12,0
 FOR dly=1 TO 100
 NEXT dly
 RETURN
