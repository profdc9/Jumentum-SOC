 server$="http://192.168.0.50:7080"
 dir$="/mp3"
 REM change this
 el$=CHR$(13)+CHR$(10)
 br$="<br>"+el$
 cn=0
REPEAT
 cn=cn+1
 PRINT "waiting for connection no ",cn
REPEAT
 wc=WEBCON
 IF INKEY=32 THEN endprg:
 UNTIL wc<>-1
 f$=WEBREQ$(wc,0)
 IF INSTR(f$,".m3u",1)<>0 THEN m3u:
 IF INSTR(f$,".mp3",1)=0 THEN dpage:
 code=WEBOUT(wc,0,"Content-type: audio/mpeg"+el$+"Connection: close"+el$+el$)
 fn$=MID$(f$,6)
 IF LEFT$(fn$,LEN(dir$))<>dir$ THEN endcon:
 fd=FOPEN(fn$,"r")
 PRINT "Sending file ",fn$
 IF fd<0 THEN endcon:
sendf: IF FEOF(fd) THEN closef:
 t$=FREAD$(fd,8192)
 code=WEBOUT(wc,0,t$)
 IF code<0 THEN closef:
 if inkey=32 then closef:
 GOTO sendf:
closef: code=FCLOSE(fd) 
 t$=""
endcon: code=WEBOUT(wc,-1,"")
 UNTIL 0
dpage: code=WEBOUT(wc,0,"<h1>MPEG 3 Player</h1>"+br$)
 code=WEBOUT(wc,0,"<h2>Directory Listing</h2>"+br$)
 od=FDIROP(dir$)
 IF od<0 THEN nodir:
 code=WEBOUT(wc,0,"<pre>"+br$)
 writdr: rd$=FDIRRD$(od)
 IF rd$="" THEN closdr:
 nm$=TRIM$(LEFT$(rd$,12))
 l=INSTR(nm$,".mp3",1)
 IF l=0 THEN nom3u:
 MID$(nm$,l)=".m3u"
 nom3u: code=WEBOUT(wc,0,"<A HREF=""/basr"+dir$+"/"+nm$+""">"+WEBESC$(rd$)+"</A>"+br$)
 GOTO writdr:
 code=FDIRCL(od)
closdr: code=WEBOUT(wc,0,"</pre>"+br$)
nodir: GOTO endcon:
m3u: code=WEBOUT(wc,0,"Content-type: audio/x-Mpegurl"+el$+"Connection: close"+el$+el$)
 l=INSTR(f$,".m3u",1)
 IF l=0 THEN nom3u2:
 MID$(f$,l)=".mp3"
 code=WEBOUT(wc,0,server$+f$)
nom3u2: GOTO endcon:
endprg: PRINT "ending"
 END
