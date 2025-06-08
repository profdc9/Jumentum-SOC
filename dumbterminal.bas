rem dumb terminal through telnet
SERINIT 1,9600,8,1,0
PRINT "welcome to terminal"
terml: ch=INKEY
IF ch=3 THEN endp:
IF ch=-1 THEN noout:
PRINT CHR$(ch);
code=SEROUT(1,CHR$(ch))
IF ch<>13 THEN noout:
PRINT CHR$(10);
noout: i$=SERINP$(1,1,-1,100)
PRINT i$;
IF i$<>CHR$(13) THEN terml:
PRINT CHR$(10);
GOTO terml:
endp: PRINT "goodbye!"
