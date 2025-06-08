fid = FOPEN("Plog2.txt", "aw")
Begin:
i=10000
REPEAT 
pout$ = "Test 1 " +  STR$(i) + CHR$(13)
e = FWRITE(fid, pout$)
i = i - 1
UNTIL i = 0
PRINT "loc=",FTELL(fid)," e=",e
GOTO Begin:
