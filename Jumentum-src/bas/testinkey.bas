REM Test Script
REM Tests the Interpreter
REM By Malcolm Mclean

PRINT "Test INKEY keyword: type a char please..."
PRINT ""

loop:
REM x=INKEY
REM Returns the character code of the currently entered character on
REM the terminal UART0, OR -1 IF no character is available.
A=INKEY
IF A=-1 THEN loop:
PRINT CHR$(A)," ",STR$(A)," 0x",HEX$(A)
GOTO loop:
