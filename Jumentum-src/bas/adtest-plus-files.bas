fd=FOPEN("test.bin","wa")
width=384
ADTRIG 0,1,500
ADSTART 0x02,0x01,0xf0,4,1
sa=0
FOR i=1 TO 100
 PRINT "reading packet ",i," samp #",sa
 h$=STRING$(width*2," ")
 ct=1
 REPEAT
 REPEAT
 UNTIL ADRDBUF>=64
 MID$(h$,ct*2-1)=ADREAD$(64)
 ct=ct+64
 UNTIL ct>width
 FOR j=1 TO width
    sa=sa+1
    code=FWRITE(fd,"Sa #"+STR$(sa)+"="+STR$(INT((BSTR(MID$(h$,j*2-1,2),2) & 0x3FF)))+CHR$(13)+CHR$(10))
 NEXT j
 NEXT i 
 ADSTOP
code=FCLOSE(fd)
PRINT "program complete"

