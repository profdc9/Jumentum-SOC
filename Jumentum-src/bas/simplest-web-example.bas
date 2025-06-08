REM  Set pin 23 as ADC input
SETPIN 23,1

start: 
REM wc=WEBCON
REM Get the web connection number for the currently open web connection to
REM the controller, or -1 if no connection is currently open.
wc=WEBCON
IF wc=-1 THEN start:

REM x=WEBOUT(<webconnection>,<status>,<htmlstring>)
REM Output the string <htmlstring> on the connection <webconnection>.  If the
REM connection exists and the transmission is successful, x=0, otherwise
REM x=-1.  If <status>=0 then continue the connection, otherwise if <status><0
REM then terminate the connection after transmitting <htmlstring>.  Will
REM block until data is transmitted, or current connection is terminated.
code=WEBOUT(wc,-1,"Value on ADC 1="+STR$(INADC(0)))

GOTO start:

REM See the ADC value at http://<IP_ADDRESS>/bas 