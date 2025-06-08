REM program TO generate output on display, nist AND weather

 GOSUB intlcd:
 REM set up pins TO go TO displays
 cl$=CHR$(13)+CHR$(10)
 it=99
 nrots=5
 DIM mrot$(nrots)
 mcur=0

 REM every 100 iterations get time from NIST
 REM every 30 iterations display weather from NOAA
 REM every iteration show a fortune
 REM every iteration show the time in UTC AND local
t5: it=(it+1) MOD 100
 PRINT "iteration ",it
 IF it>0 THEN t6:
 GOSUB nistm:
 it=0
t6: IF (it MOD 20)<>5 THEN t7:
 GOSUB wthr:
t7: IF (it MOD 5)<>0 THEN t72:
 mesg$=" EDT -04 EST -05"
 GOSUB wrln:
 mesg$=TIMEGT$+" UTC"
 GOSUB skroll:
t72: GOSUB fortun:
 IF (it MOD 10)<>0 THEN t8:
 GOSUB subtm:
 mesg$="The Snicker Ticker by Dan Marks, running off a Jumentum system-on-chip, http://jumentum.sourceforge.net/"
 GOSUB skroll:
t8: mcur=(mcur+1) MOD nrots
 mesg$=mrot$(mcur+1)
 IF mesg$="" AND mcur>0 THEN t8:
 GOSUB skroll:
 IF INKEY<0 THEN t5:
 END

REM show the local time adjusting UTC TO local
REM e.g. tz=-4 is EDT, tz=-5 is EST
subtm:
   tz=-4
   hr=(VAL(MID$(TIMEGT$,12,2))+24+tz) MOD 24
   tim$=RIGHT$("0"+STR$(((hr+11) MOD 12)+1),2)+MID$(TIMEGT$,14,6)+II$(hr<12," AM"," PM")
   mesg$="  "+tim$
   iln=1
   GOSUB lcdout:
   RETURN

REM write a line TO the second display using
REM bit-banging.  295=9600 bps on my board
wrln:
   mesg$=LEFT$(mesg$+STRING$(20," "),20)
   iln=1
   GOSUB lcdout:
   RETURN

REM scroll a message across the first display
REM use bit-banging TO send TO the display
skroll: nskt=LEN(mesg$)+16
 skt$=STRING$(20," ")+mesg$+STRING$(20," ")
 cskt=1
 cspd=200
 REPEAT
   te$=MID$(skt$,cskt,20)
   PRINT te$,CHR$(13);
   iln=0
   mesg$=te$
   GOSUB lcdout:
   FOR wt=1 TO 500
   wc=WEBCON
   IF wc<>-1 THEN getweb:
webct: NEXT wt
   cskt=cskt+1
 UNTIL cskt>nskt
 RETURN

getweb: w$=WEBREQ$(wc,0)
   IF INSTR(w$,"/msgs",1)=0 THEN prform:
   code=WEBOUT(wc,0,"<H2>SNICKER TICKER Programmed rotating messages</H2><BR><BR>")
   mesg$=WEBPST$(wc,200)
   FOR i=1 TO nrots
   mrot$(i)=WEBFRM$(mesg$,"msg"+STR$(i))
   code=WEBOUT(wc,0,"Message "+STR$(i)+": """+WEBESC$(mrot$(i))+"""<br>")
   NEXT i
   GOTO webc:
prform: code=WEBOUT(wc,0,"<H1>SNICKER TICKER rotating messages</H1><BR><BR><form method=post action=""bas/msgs""><pre>")
 FOR i=1 TO nrots
 code=WEBOUT(wc,0,"Message "+STR$(i)+": <input type=text size=40 name=""msg"+STR$(i)+""" value="""+WEBESC$(mrot$(i))+"""><br>")
 NEXT i
 code=WEBOUT(wc,0,"<input type=submit name=""SET"" value=""SET""></form><br>")
webc: code=WEBOUT(wc,-1,"")
 GOTO webct:

REM get the time from the NIST time service
nistm:
   mesg$="Retrieving time"
   GOSUB wrln:
   sv$="64.236.96.53"
   ou=SCCON(sv$,13,2000)
   wt=5000
nistm2: wt=wt-1
   IF SCISCON(ou)=0 AND wt>0 THEN nistm2:
   IF wt=0 THEN nisten:
rdln: t$=SCREAD$(ou,200,13)
   IF t$="" THEN rdln:
   t$="20"+MID$(t$,8,17)
   PRINT "Setting time to ",t$
   wt=TIMESET(t$)
   wt=SCCLOSE(ou)
   pseu=(-(VAL(MID$(t$,15,2)))*256-VAL(MID$(t$,18,2)))
   wt=RND(pseu)
nisten: RETURN

REM get the weather from NOAA
wthr:
   wln$=""
   mesg$="Retr. weather"
   wcity$="...RALEIGH"
   GOSUB wrln:
   sv$="140.90.113.200"
   ssv$="www.nws.noaa.gov"
   surl$="/view/prodsByState.php?state=NC&prodtype=zone"
   ou=SCCON(sv$,80,2000)
   wt=5000
wthr1: wt=wt-1
   IF SCISCON(ou)=0 AND wt>0 THEN wthr1:
   IF wt=0 THEN wthr4:
   surl$="GET "+surl$+" HTTP/1.1"+cl$+"Host: "+ssv$+cl$+cl$
   wt=SCWRITE(ou,surl$)
   wt=0
wthr2: t$=SCREAD$(ou,200,10)
   wt=wt+IIF(wt=0,INSTR(t$,wcity$,1)>0,0)+IIF(wt=1,INSTR(t$,"TODAY",1) OR INSTR(t$,"TONIGHT",1)>0,0)
   IF wt=2 THEN wthr21:
   IF SCISCON(ou)=0 AND t$="" THEN wthr3:
   GOTO wthr2:
wthr21: mesg$="Reading weather"
   GOSUB wrln:
   wln$=""
   GOTO wthr23:
wthr22: t$=SCREAD$(ou,200,10)
   IF SCISCON(ou)=0 AND t$="" THEN wthr3:
wthr23: wln$=wln$+TRIM$(t$)+" "
   IF LEN(wln$)<500 THEN wthr22: 
wthr3: wt=SCCLOSE(ou)
wthr4:
   IF wln$="" THEN wthr5:
   mesg$="Wthr"+wcity$
   GOSUB wrln:
   mesg$=wln$
   GOSUB skroll:
   mesg$=""
   wln$=""
wthr5: RETURN

REM read a random fortune from fortune file
REM on flash card.
fortun: GOSUB subtm:
   fn=FOPEN("fortune.txt","R")
   IF fn<0 THEN forten:
   wt=FSEEK(fn,0,2)
   fln=FTELL(wt)
frnd: fgo=(RND(256) LSL 16)+(RND(256) LSL 8)+RND(256)
   IF fgo>fln THEN frnd:
   wt=FSEEK(fn,fgo,0)
frt: c$=FREAD$(fn,1)
   IF c$<>"%" AND c$<>"" THEN frt:
   IF c$="" THEN forte2:
   mesg$=""
frt2: c$=FREAD$(fn,1)
   IF c$="%" OR c$="" THEN frt3:
   IF ASCII(c$)<32 THEN frt22:
   mesg$=mesg$+c$
   GOTO frt2:
frt22: mesg$=TRIM$(mesg$)+" "
   GOTO frt2:
frt3: GOSUB skroll:
forte2: wt=FCLOSE(fn)
forten: RETURN

REM subroutine TO output TO hd44780
REM assume pin 10=RS(pin 4 LCD), pin 11=E(pin 6), pin 6=DB4(pin 11),
REM        pin 7=DB5(pin 12), pin 8=DB6(pin 13), pin 9=DB7(pin 14)
REM        pin 5 is held low on LCD        
intlcd: 
 nlines=1
 nchars=20
 SETPIN 6,4
 SETPIN 7,4
 SETPIN 120,4
 SETPIN 121,4
 SETPIN 122,4
 SETPIN 123,4
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
 byt=0x01
 GOSUB lclock:
 RETURN
lcdout:
 cmd=0
 byt=0x02
 GOSUB lclock:
 FOR dly=1 TO 10
 NEXT dly
 byt=0x80+iln*0x40
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
 OUTD 6,cmd
 OUTD 7,0
 OUTD 120,(byt & 0x10)<>0
 OUTD 121,(byt & 0x20)<>0
 OUTD 122,(byt & 0x40)<>0
 OUTD 123,(byt & 0x80)<>0
 OUTD 7,1
 OUTD 7,0
 OUTD 120,(byt & 0x01)<>0
 OUTD 121,(byt & 0x02)<>0
 OUTD 122,(byt & 0x04)<>0
 OUTD 123,(byt & 0x08)<>0
 OUTD 7,1
 OUTD 7,0
 RETURN
