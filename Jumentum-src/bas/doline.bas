REM example TO use DOLINE
 br$="<br>"+CHR$(13)+CHR$(10)
 e$=""
 m$=""
REPEAT
REPEAT
 wc=WEBCON
 IF INKEY<>-1 THEN endprg:
UNTIL wc<>-1
 w$=WEBREQ$(wc,0)
 IF INSTR(w$,"/doit",1)=0 THEN prform:
 a$=WEBPST$(wc,1000)
 e$=WEBFRM$(a$,"function")
 DOLINE e$,er
 code=WEBOUT(wc,0,"<h1>Evaluating expression</h1>"+br$)
 code=WEBOUT(wc,0,"Command: "+WEBESC$(e$)+br$+br$)
 code=WEBOUT(wc,0,"Error code: "+ERC$(er))
prform: code=WEBOUT(wc,0,"<h1>Enter expression to be evaluated</h1>"+br$+"<form method=post action=""/bas/doit"">")
 code=WEBOUT(wc,0,"Expression: <input type=text name=""function"" value="""+WEBESC$(e$)+""">"+br$+br$)
 code=WEBOUT(wc,0,"<input type=submit name=""go"" value=""Evaluate""</form>"+br$)
endcon: code=WEBOUT(wc,-1,"")
UNTIL 0
endprg: PRINT "ending"
 END
 
