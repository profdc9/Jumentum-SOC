REM demo TO serve web pages from SD card
dir$="/htdocs"
l$=CHR$(13)+CHR$(10)
REPEAT
REPEAT
 wc=WEBCON
 IF INKEY=32 THEN endp:
UNTIL wc<>-1
 w$=WEBREQ$(wc,0)
 IF LEFT$(w$,11)="/basr/file/" THEN webpg:
 c=WEBOUT(wc,0,"</h1>Web Server</h1><P>")
 c=WEBOUT(wc,0,"<a href=""/basr/file/index.htm"">Access web pages</a>")
nextc: c=WEBOUT(wc,-1,"") 
 UNTIL 0

webpg: fl$=dir$+"/"+MID$(w$,12)
 IF INSTR(fl$,"..",1)<>0 THEN nextc:
 PRINT "filename=",fl$
 f=FOPEN(fl$,"r")
 IF f<0 THEN nextc:
 t$="text/plain"
 IF RIGHT$(fl$,4)<>".htm" THEN nohtml:
 t$="text/html"
 nohtml: IF RIGHT$(fl$,4)<>".jpg" THEN nojpg:
 t$="image/jpeg"
 nojpg: c=WEBOUT(wc,0,"Content-type: "+t$+l$+l$)
 rdfl: IF FEOF(f) THEN closf:
  t$=FREAD$(f,1000)
  c=WEBOUT(wc,0,t$)
  GOTO rdfl:
closf: c=FCLOSE(f)
 GOTO nextc:

endp: PRINT "end web server"
