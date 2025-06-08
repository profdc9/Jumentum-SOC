REM 
PRINT "Test Web Commands"
lf$=CHR$(10)
REPEAT
 PRINT "waiting for connection"
 REPEAT
  REM wc=WEBCON
  REM Get the web connection number for the currently open web connection to
  REM the controller, or -1 if no connection is currently open.
  wc=WEBCON
  IF INKEY<>-1 THEN fine:
 UNTIL wc<>-1
 
 PRINT "connected=",wc
 PRINT "filename=",WEBREQ$(wc,0)
 
 REM x=WEBOUT(<webconnection>,<status>,<htmlstring>)
 REM Output the string <htmlstring> on the connection <webconnection>.  If the
 REM connection exists and the transmission is successful, x=0, otherwise
 REM x=-1.  If <status>=0 then continue the connection, otherwise if <status><0
 REM then terminate the connection after transmitting <htmlstring>.  Will
 REM block until data is transmitted, or current connection is terminated.
 code=WEBOUT(wc,0,"Content-type: text/html"+lf$+lf$)
 
 REM x$=WEBREQ$(<webconnection>,<code #>)
 REM Gets information about the current web connection to the string x$.  Currently
 REM two <code #> are supported.  <code #>=0 retrieves the current URL, and 
 REM <code #>=1 retrieves "P" if the current connection is a POST, or "G" if it is
 REM a GET.
 PRINT "line=",WEBREQ$(wc,1)
 PRINT "code=",WEBOUT(wc,0,"hey there<br>")
 
 FOR i=1 TO 100
  code=WEBOUT(wc,0,STR$(i)+" bottles of beer on the wall, "+STR$(i)+" bottles of beer <br>")
 NEXT i
 PRINT "code=",WEBOUT(wc,-1,"go<br>")
UNTIL 0

fine: 
PRINT "ending"
END