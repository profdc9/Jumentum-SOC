 br$="<br>"+chr$(13)+chr$(10)
 PRINT "test web commands"
REPEAT
 PRINT "waiting for connection"
REPEAT
 wc=WEBCON
 IF INKEY<>-1 THEN endprg:
UNTIL wc<>-1
 w$=WEBREQ$(wc,0)
 IF INSTR(w$,"/doit",1)=0 THEN prform:
 a$=WEBPST$(wc,200)
 gosub web1:
 code=WEBOUT(wc,0,"postdata="+WEBESC$(a$)+br$)
 code=WEBOUT(wc,0,"field1="+WEBESC$(WEBFRM$(a$,"field1"))+br$)
 code=WEBOUT(wc,0,"field2="+WEBESC$(WEBFRM$(a$,"field2"))+br$)
 code=WEBOUT(wc,0,"field3="+WEBESC$(WEBFRM$(a$,"field3"))+br$)
 code=WEBOUT(wc,0,"go="+WEBESC$(WEBFRM$(a$,"go"))+br$)
GOTO endcon:
prform: gosub web1:
 code=WEBOUT(wc,0,"<form method=post action=""bas/doit"">")
 code=WEBOUT(wc,0,"field1: <input type=text name=""field1"">"+br$)
 code=WEBOUT(wc,0,"field2: <input type=text name=""field2"">"+br$)
 code=WEBOUT(wc,0,"<input type=submit name=""go"" value=""label""</form>"+br$)
endcon: code=WEBOUT(wc,-1,"")
UNTIL 0
endprg: PRINT "ending"
 END
web1: code=WEBOUT(wc,0,"filename="+w$+br$)
 code=WEBOUT(wc,0,"request_type="+WEBREQ$(wc,1)+br$)
 return
 