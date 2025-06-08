REM
PRINT "Test Web I/O commands"
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

 REM 
 SETPIN 21,4
 OUTD 21,0
 REM x=WEBOUT(<webconnection>,<status>,<htmlstring>)
 REM Output the string <htmlstring> on the connection <webconnection>.  If the
 REM connection exists and the transmission is successful, x=0, otherwise
 REM x=-1.  If <status>=0 then continue the connection, otherwise if <status><0
 REM then terminate the connection after transmitting <htmlstring>.  Will
 REM block until data is transmitted, or current connection is terminated.
 code=WEBOUT(wc,0,"Call this page with ?ON or ?OFF to flash the LED<br>")
 
 REM x$=WEBREQ$(<webconnection>,<code #>)
 REM Gets information about the current web connection to the string x$.  Currently
 REM two <code #> are supported.  <code #>=0 retrieves the current URL, and 
 REM <code #>=1 retrieves "P" if the current connection is a POST, or "G" if it is
 REM a GET.
 w$=WEBREQ$(wc,0)

 OUTD 21,INSTR(w$,"ON",1)>0
 code=WEBOUT(wc,-1,"pin 21 is "+STR$(IND(21))+"<br>")
UNTIL 0

fine:
PRINT "ending"
END
