REM dumb terminal through telnet
REM Character are echoed from telnet to serial and viceversa.
REM

REM SERINIT <port #>,<baud rate>,<databits>,<stopbits>,<parity>
REM Initializes the serial port denoted by <port #>.  Currently two serial ports
REM are supported denoted by <port #>=0 and <port #>=1.
SERINIT 1,38400,8,1,0
PRINT "welcome to terminal"
terml: 
ch=INKEY
IF ch=ASCII("Q") THEN endp:
IF ch=-1 THEN noout:
PRINT CHR$(ch);

REM x=SEROUT(<port #>,<string>)
REM Send the characters in <string> out serial port number <port #>.  The return code
REM currently is zero, but would be negative for an error code.
code=SEROUT(1,CHR$(ch))
IF ch<>13 THEN noout:
PRINT CHR$(10);

noout: 
REM <instring>=SERINP$(<port #>,<numchars>,<endchar>,<timeout>)
REM Reads up to <numchars> from serial port <port #>.  If <numchars> is
REM negative, then -<numchars> characters will be read, but discarded,
REM which is useful for clearing the receive queue.  It will stop receiving
REM characters if the character denoted by <endchar> is received and <endchar>
REM is not a negative number. 
i$=SERINP$(1,1,-1,100)
PRINT i$;
IF i$<>CHR$(13) THEN terml:
PRINT CHR$(10);
GOTO terml:
endp: 
PRINT "goodbye!"
