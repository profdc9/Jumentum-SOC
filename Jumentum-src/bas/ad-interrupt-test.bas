GOTO test1:
x=PEEK(0xe01fc0c4)
PRINT "pconp=",HEX$(x)
POKE 0xe01fc0c4,(x | 0x80000000)
PRINT "pconpafter=",HEX$(PEEK(0xe01fc0c4))
buf=0x7fd00000
FOR i=0 TO 16 STEP 4
   POKE buf+i,i
   PRINT i," ",PEEK(buf+i)
   NEXT i
END
test1:
SETPIN 28,1
ad1=INADC(1)
PRINT "adc 1=",ad1
ADTRIG 3,1,500
ADSTART 0x02,0x01,0x20,1,1
REPEAT
UNTIL ADRDBUF > 1000
a$=ADREAD$(ADRDBUF)
PRINT "read=",LEN(a$)
FOR i=1 TO 20
PRINT "byte=",HEX$(BSTR(MID$(a$,i*2-1,2),2))
NEXT i
PRINT "cur to read=",ADRDBUF
REM ADSTOP 
