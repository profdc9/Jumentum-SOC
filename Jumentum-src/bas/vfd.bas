 GOTO pr:
 st: PRINT "pin: ";
  INPUT pin
  IF pin=-1 THEN endprg:
  PRINT "state: ";
  INPUT stt
  OUTD pin,1
  SETPIN pin,stt
  GOTO st:
  

pr: REM example TO program hd44780 LCD
 br$="<br>"+CHR$(13)+CHR$(10)
 mesg$=""
init: GOSUB intlcd: 
enttxt:
 PRINT "text: ";
 INPUT a$
 IF a$="!" THEN init:
 IF a$="#" THEN drnd:
 IF a$="$" THEN pins:
 IF a$<>"." THEN dout:
 END
pins: SETPIN 0,0
 SETPIN 1,0
 SETPIN 6,0
 SETPIN 7,0
 SETPIN 8,0
 SETPIN 9,0
 GOTO enttxt:
dout: mesg$=a$ 
 GOSUB lcdout:
 GOTO enttxt:
drnd:
 FOR i=1 TO 10000
 cmd=1
 byt=RND(0x100)
 GOSUB lclock:
 NEXT i
 GOTO init:

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
REM assume pin 0=RS(pin 4 LCD), pin 6=E(pin 1), 
REM        pin 7=DB4(pin 6),
REM        pin 8=DB5(pin 7), pin 9=DB6(pin 8), pin 23=DB7(pin 9)
intlcd:
 OUTD 1,0 
 SETPIN 0,4
 SETPIN 1,4
 SETPIN 6,4
 SETPIN 7,4
 SETPIN 8,4
 SETPIN 9,4
 nlines=2
 nchars=20
 cmd=0
 byt=0x33
 GOSUB lclock:
 byt=0x32
 GOSUB lclock:
 byt=0x28
 GOSUB lclock:
 byt=0x00
 GOSUB lclock:
 cmd=1
 byt=0x08
 GOSUB lclock:
 cmd=0
 byt=0x08
 GOSUB lclock:
 byt=0x01
 GOSUB lclock:
 byt=0x0F
 GOSUB lclock:
 byt=0x06
 GOSUB lclock:
lcde: RETURN
lcdout:
 cmd=0
 REM byt=0x01
 REM GOSUB lclock:
 REM byt=0x02
 REM GOSUB lclock:
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
 OUTD 1,0
 OUTD 0,cmd
 PRINT "cmd=",cmd," byt=",HEX$(byt)
 OUTD 6,(byt & 0x10)<>0
 OUTD 7,(byt & 0x20)<>0
 OUTD 8,(byt & 0x40)<>0
 OUTD 9,(byt & 0x80)<>0
 WAIT 10
 OUTD 1,1
 OUTD 1,0
 WAIT 10
 OUTD 6,(byt & 0x01)<>0
 OUTD 7,(byt & 0x02)<>0
 OUTD 8,(byt & 0x04)<>0
 OUTD 9,(byt & 0x08)<>0
 WAIT 10
 OUTD 1,1
 OUTD 1,0
 WAIT 10
 RETURN
           