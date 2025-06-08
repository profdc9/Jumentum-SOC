fd=FOPEN("test.bin","wa")
PRINT "file desc=",fd
width=1024
ADTRIG 0,1,500
ADSTART 0x02,0x01,0xf0,4,1
FOR i=1 TO 10000
 REPEAT
 UNTIL ADRDBUF>=width
 h$=ADREAD$(width)
 code=FWRITE(fd,h$)
 PRINT "reading packet ",i," adrdbuf=",ADRDBUF," total=",FTELL(fd)," ercode=",code
 IF INKEY<>32 THEN skip:
 i=10000
skip: NEXT i 
ADSTOP
code=FCLOSE(fd)
PRINT "program complete"
