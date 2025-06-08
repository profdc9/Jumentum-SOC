REM Test Web Command
br$="<br>"+chr$(13)+chr$(10)
PRINT "Test Web Commands"
REPEAT
 PRINT "waiting for connection"
 REPEAT
  REM wc=WEBCON
  REM Get the web connection number for the currently open web connection to
  REM the controller, or -1 if no connection is currently open.
  wc=WEBCON
  IF INKEY<>-1 THEN endprg:
 UNTIL wc<>-1
 
 REM x$=WEBREQ$(<webconnection>,<code #>)
 REM Gets information about the current web connection to the string x$.  Currently
 REM two <code #> are supported.  <code #>=0 retrieves the current URL, and 
 REM <code #>=1 retrieves "P" if the current connection is a POST, or "G" if it is
 REM a GET.
 w$=WEBREQ$(wc,0)
 
 IF INSTR(w$,"/doit",1)=0 THEN prform:
 REM x$=WEBPST$(<webconnection>,<numchars>)
 REM Get the POST data (e.g. for HTML forms) from webconnection <webconnection> to
 REM string x$.  <numchars> is the maximum number of POST data characters to
 REM allocate.  This is useful in conjunction with WEBFRM$ to get HTML forms.
 REM Will block until FORM data is received, or current connection is terminated.
 REM Note, the WEBPST$ command must be performed before attempting to send data with WEBOUT,
 REM or the WEBOUT will block.
 a$=WEBPST$(wc,200)
 
 gosub web1:

 REM x=WEBOUT(<webconnection>,<status>,<htmlstring>)
 REM Output the string <htmlstring> on the connection <webconnection>.  If the
 REM connection exists and the transmission is successful, x=0, otherwise
 REM x=-1.  If <status>=0 then continue the connection, otherwise if <status><0
 REM then terminate the connection after transmitting <htmlstring>.  Will
 REM block until data is transmitted, or current connection is terminated.
 code=WEBOUT(wc,0,"postdata="+WEBESC$(a$)+br$)
 code=WEBOUT(wc,0,"field1="+WEBESC$(WEBFRM$(a$,"field1"))+br$)
 code=WEBOUT(wc,0,"field2="+WEBESC$(WEBFRM$(a$,"field2"))+br$)
 code=WEBOUT(wc,0,"field3="+WEBESC$(WEBFRM$(a$,"field3"))+br$)
 code=WEBOUT(wc,0,"go="+WEBESC$(WEBFRM$(a$,"go"))+br$)
 GOTO endcon:
 prform: 
 gosub web1:
 code=WEBOUT(wc,0,"<form method=post action=""bas/doit"">")
 code=WEBOUT(wc,0,"field1: <input type=text name=""field1"">"+br$)
 code=WEBOUT(wc,0,"field2: <input type=text name=""field2"">"+br$)
 code=WEBOUT(wc,0,"<input type=submit name=""go"" value=""label""</form>"+br$)
 endcon: 
 code=WEBOUT(wc,-1,"")
UNTIL 0
endprg: 
PRINT "ending"
END

web1: 
code=WEBOUT(wc,0,"filename="+w$+br$)
code=WEBOUT(wc,0,"request_type="+WEBREQ$(wc,1)+br$)
return
 