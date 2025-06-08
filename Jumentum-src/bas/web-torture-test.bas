PRINT "test web commands"
REPEAT
PRINT "waiting for connection"
REPEAT
wc=WEBCON
IF INKEY<>-1 THEN endprg:
UNTIL wc<>-1
PRINT "connection=",wc
code=WEBOUT(wc,0,"<META HTTP-EQUIV=""Refresh"" CONTENT=1>")
code=WEBOUT(wc,0,"hey there<br>")
code=WEBOUT(wc,0,"connected="+STR$(wc)+"<br>")
code=WEBOUT(wc,0,"filename="+WEBREQ$(wc,0)+"<br>")
FOR i=1 TO 100 STEP 20
a$=""
FOR j=0 TO 19
a$=a$+STR$(i+j)+" bottles of beer on the wall, "+STR$(i+j)+" bottles of beer <br>"
PRINT (i+j)," ";
NEXT j
code=WEBOUT(wc,0,a$)
NEXT i
PRINT ""
PRINT "code=",WEBOUT(wc,-1,"go<br>")
UNTIL 0
endprg: PRINT "ending"
END
