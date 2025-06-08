Jumentum BASIC for ARM 

**

## Jumentum-SOC BASIC language

**

**

# Brief description of the Basic language:

**

The BASIC interpreter has one statement per line. Each line may be preceded by a label with a format "label:" (where label is the name of the label). Labels may have up to six characters not including the colon. A label must be specified to jump to a particular line. Labels in statements must also have a colon, e.g.

    myline: print "hello"
    goto myline:
	

A statement is either an assigment of _&lt;variable&gt; = &lt;expression&gt;_ (as in the LET statement, but the actual word LET may be omitted), or one of the commands below.

An expression for numbers includes the operators

*   \-a negative a (e.g. 0-a)
*   a+b sum of a and b
*   a-b difference between a and b
*   a\*b multiplication of a and b
*   a/b division of a and b. If integer division is used this is rounded down to the largest integer equal to or less than a/b.
*   a **MOD** b calculates a modulo b. If floating point arithmetic is
*   compiled in, this is fmod(a,b), otherwise this is (a % b).
*   a&b bitwise AND of a and b
*   a|b bitwise OR of a and b
*   a^b bitwise XOR of a and b
*   ~a bitwise NOT (complement) of a
*   a **LSL** b logical shift left of a by b
*   a **LSR** b logical shift right of a by b (shifts zeros in high bits)

Sub expressions can be evaluated first by placing them in parentheses. Numbers can be expressed either in decimal as decimal digits, or as hexadecimal preceded by "0x" as in C.

Booleans and numbers are one and the same. A **TRUE** value (for the IF statement is nonzero), or zero if **FALSE**. The operations yielding TRUE/FALSE values are:

*   a&lt;b conditional evaluating whether a is less than b
*   a&gt;b conditional evaluating whether a is greater than b
*   a=b conditional evaluating whether a is equal to b
*   a&lt;=b conditional evaluating whether a is less than or equal to b
*   a&gt;=b conditional evaluating whether a is greater than or equal to b

Both strings and numbers can be compared, with strings being compared by evaluating the code number (ASCII code) precedence of each character of each string starting with the first character.

In addition the conditionals can be joined by:  

x **AND** y  
The logical AND of x and y (used in a condition in an IF statement). (x AND y) is true if both of x or y evaluate to true.

x **OR** y  
The logical OR of x and y (used in a condition in an IF statement). (x OR y) is true if either of x or y evaluates to true.

**NOT** x  
Logical NOT of expression.

One quirk in how the order-of-operations occurs: the typing of strings and integers has precedence over all other operations. That means if you try

	A$="HELLO"
	PRINT A$&gt;"GOODBYE"
	

it will fail, because the parse things that A$ is a string first, before it evaluates the whole expression A$&gt;"GOODBYE". However, all expressions in parenthesis are assumed to evaluate to numbers, so you can do

    PRINT (A$&gt;"GOODBYE")
	

Strings are 8-bit clean (they can contain zero-valued bytes), so they can be used as hardware buffers.

**

# The basic commands are described below:

**

**ABORT** &lt;label:&gt;  
Jumps to line denoted by &lt;label:&gt;. Unwinds all GOSUB, REPEAT/UNTIL, and FOR/NEXT loops. Good for error recovery to ensure the program will continue to run.

x=**ABS**(&lt;number&gt;)  
Takes the absolute value of number.

x=**ASCII**(&lt;string&gt;)  
Returns an integer 0 to 255 giving the character code value of the first character of &lt;string&gt;. An alternative x=ASCII(&lt;string&gt;,n) gives the code of the nth character (or -1 if the string is too short to have an nth character).

x$=**BIN$**(x,&lt;digs&gt;)  
Compute the string equivalent of the raw data value of x. &lt;digs&gt; is the number of bytes to use in x$. Negative &lt;digs&gt; is big-endian, positive &lt;digs&gt; is little endian. This is the complement of **BSTR**.

**BOLD** &lt;value&gt;  
A non-zero value outputs a VT100/ANSI code to turn on bold type, a zero value turns off bold.

x=**BSTR**(x$,&lt;digs&gt;)  
Compute the numerical equivalent of the raw data value of x$. &lt;digs&gt; is the number of bytes to use in x$. Negative &lt;digs&gt; is big-endian, positive &lt;digs&gt; is little endian. This is the complement of **BIN$**.

**CHR$**(&lt;value&gt;)  
Returns a string with a single character with character code &lt;value&gt; (from 0 to 255).

**CLREOL**  
Outputs a VT100/ANSI code to clear the current line from the cursor position to the end of the line.

**CLRSCR**  
Outputs a VT100/ANSI code to clear the screen and place the cursor at the upper left corner.

x$=**CONF$**(&lt;confstring&gt;)  
Reads the configuration variable out of the configuration file and places it into x$. E.g. if there was a line in the configuration file such as MYNAME=Joe Brown then x$=**CONF$**("MYNAME") places "Joe Brown" into x$

**DIM A**(&lt;index 1&gt;)  
**DIM A**(&lt;index 1&gt;,&lt;index 2&gt;)  
**DIM A$**(&lt;index 1&gt;&gt;)  
**DIM A$**(&lt;index 1&gt;,&lt;index 2&gt;)  
These commands create arrays of strings or numbers, either one or two dimensional. These can be initialized to particular values by using the =, e.g.

	DIM A(10)=10,9,8,7,6,5,4,3,2,1
	DIM NOTES$(7)="Do","Re","Mi","Fa","So","La","Si"
	

**DOLINE** &lt;CMDSTRING&gt;,&lt;ERRORCODE&gt;  
This command allows one to dynamically execute a limited set of BASIC commands. The command to be executed is in the string &lt;CMDSTRING&gt;. The command is interpreted as a BASIC command and executed and the error code result is placed in the numeric variable &lt;ERRORCODE&gt;. The commands IF/THEN, FOR/NEXT, GOSUB, RETURN, GOTO, REPEAT, UNTIL, ONERROR, ABORT, END, and DOLINE can not be used in this command. Examples:  

	DOLINE "z=5",er
	DOLINE "PRINT ""hello""",er
	

The first example assigns the value 5 to the variable "z," and zero to er to indicate no error. The second example prints hello. Note: this command is highly susceptible to phishing attacks from scripts, you have been warned.

**END**  
Terminates the program.

x$=**ERC$**(&lt;errorcodenumber&gt;)  
Return a string describing the error associated with errorcode number. If number is negative, then reports the last error.

x=**ERR**(0)  
This retrieves either the error code of the last error for **ONERROR**. Reading this clears the error code so subsequent accesses return 0 until another error is thrown. x=**ERR**(1) This retrieves the line number of the last error for **ONERROR**. Reading this clears the line number of the error so subsequent access return 0 until another error is thrown. In the editor, the line number of a particular statement can be jumped to by using the CTRL-G command.

**FOR** &lt;var&gt;=&lt;begin&gt; **TO** &lt;end&gt; **STEP** &lt;step&gt;  
Iterates &lt;var&gt; starting at &lt;begin&gt; until it reaches &lt;end&gt;, each time through the loop incrementing the step size the value &lt;step&gt;. &lt;step&gt; may be negative if &lt;end&gt; is smaller than &lt;begin&gt;. The end of the loop is **NEXT** &lt;var&gt;.

e.g.

	FOR I=99 TO 1 STEP -1
	PRINT I," bottles of beer on the wall"
	NEXT I
	

**GOSUB** &lt;label:&gt;  
Calls a subroutine at the line labeled with &lt;label:&gt;. Control returns to the statement after the **GOSUB** command when a **RETURN** is encountered.

**GOTO** &lt;label:&gt;  
Jumps program execution to the line labeled by &lt;label:&gt;.

x$=**HEX$**(&lt;number&gt;)  
Returns a string representation of the unsigned hexadecimal equivalent of &lt;number&gt;. Decimal can be returned with **STR$**.

**IF** &lt;condition&gt; **THEN** &lt;labelthen:&gt;  
**IF** &lt;condition&gt; **THEN** &lt;labelthen:&gt; **ELSE** &lt;labelelse:&gt; If the condition is true, then jump program execution to the line labeled by &lt;labelthen:&gt;. If **ELSE** is included, then jump to the line labeled by &lt;labelelse:&gt; if the condition is false.

x=**IIF**(&lt;condition&gt;,&lt;truevalue&gt;,&lt;falsevalue&gt;)  
If the condition is true, then return &lt;truevalue&gt; otherwise return &lt;falsevalue&gt;. Note that both &lt;truevalue&gt; and &lt;falsevalue&gt; are evaluated even though only one is used.

x$=**II$**(&lt;condition&gt;,&lt;truestring&gt;,&lt;falsestring&gt;)  
If the condition is true, then return &lt;truestring&gt;, otherwise return &lt;falsestring&gt;. Note that both &lt;truestring&gt; and &lt;falsestring&gt; are evaluated even though only one is used.

x=**INADC**(&lt;ad #&gt;)  
Gets the analog value at analog to digital converter numbered from 0-15. The &lt;ad #&gt; from 0-7 corresponds to AD0.&lt;ad #&gt; and the &lt;ad #&gt; from 8-15 corresponds to AD1.&lt;ad #&gt;. Must do a **SETPIN** first to the appropriate pin to set up the ADC (see **SETPIN** examples).

x=**IND**(&lt;pin #&gt;)  
Get the digital input at the pin number denoted by . The actual pin for the &lt;pin #&gt; is given in the following table:

0-31 pins P0.0 to P0.31
100-131 pins P1.0 to P1.31
200-231 pins P2.0 to P2.31
300-331 pins P3.0 to P3.31
400-431 pins P4.0 to P4.31

Use the **SETPIN** &lt;pin #&gt;,0 command first to set up the pin as a GPIO input.

x=**INKEY**  
Returns the character code of the currently entered character on the terminal UART0, or -1 if no character is available.

x=**INPLEN**(&lt;length&gt;)  
Sets the maximum number of characters to accept for subsequent **INPUT** statements. Returns the old **INPLEN** value. The input length maximum defaults to 80 characters.

**INPUT** &lt;numerical variable&gt;  
**INPUT** &lt;string variable&gt;  
Takes input from the terminal UART0 and places a numerical representation in &lt;numerical variable&gt; of the number entered at the terminal, or into &lt;string variable$&gt; the string entered at the terminal. The entered strings have a maximum size set by the INPLEN commands, which defaults to 80 characters.

x=**INSTR**(&lt;search string&gt;,&lt;substring&gt;,&lt;index&gt;)  
Searches for the first occurrence of string &lt;substring&gt; in the string &lt;search string&gt; that is at or after the character index &lt;index&gt;, with &lt;index&gt;=1 being the first character. If the substring is not found, zero is returned.

**INT**(&lt;number&gt;)  
Returns the integer representation of the number &lt;number&gt; (does nothing unless floating point is compiled in).

x$=**LEFT$**(&lt;string&gt;,&lt;num characters&gt;)  
Returns the first &lt;num characters&gt; characters of string &lt;string&gt;.

x=**LEN**(&lt;string&gt;)  
Returns the number of characters in &lt;string&gt;.

**LET** &lt;variable&gt;=&lt;expression&gt;  
Assigns the variable &lt;variable&gt; to the expression &lt;expression&gt;. If the expression is a string, then the variable must be a string variable. or if the expression is a number then the variable must be a numeric variable. **LET** is optional, e.g.  
X = 5  
is a valid statement (the **LET** may be omitted).

**LOCATE** &lt;y cursor position&gt;,&lt;x cursor position&gt;  
Sends the VT100/ANSI code to the terminal to position the cursor at y position &lt;y cursor position&gt; and x position &lt;x cursor position&gt;.

x$=**MID$**(&lt;string&gt;,&lt;index&gt;,&lt;num characters&gt;)  
Returns the &lt;num characters&gt; characters starting at index &lt;index&gt; in string &lt;string&gt; as a new string. If &lt;num characters&gt;=-1 or is omitted then the remainder of the string to the end is returned.

**MID$**(&lt;string&gt;,&lt;index&gt;,&lt;num characters&gt;)=&lt;substring&gt;  
Substitutes characters from &lt;substring&gt; into &lt;string&gt; starting at index &lt;index&gt;. If &lt;num characters&gt; is specified, then it substitutes only &lt;num characters&gt; into &lt;string&gt; (if that many are available), or if not specified, substitutes all the characters from &lt;substring&gt;. The string &lt;string&gt; is extended if the &lt;substring&gt; overwrites the end of &lt;string&gt;.

**NEXT** &lt;variable&gt;  
Ends a **FOR**/**NEXT** loop (see **FOR**).

**ONERROR** &lt;label:&gt;  
On an error, jump to the line labeled by &lt;label:&gt;. Note that after an error is trapped, **ONERROR** must be reissued to trap another error (to prevent a potential infinite loop). Omitting the label restores built-in error handling (which terminates program execution). This is useful with **ERR**() to get the error code, and **ABORT** to unwind the **GOSUB**/**REPEAT-UNTIL**/**FOR-NEXT** stacks.

**OUTD** &lt;pin #&gt;,&lt;state&gt;  
Output a low for state=0, or a high for state=1 to the pin number denoted by &lt;pin #&gt;. The actual pin corresponding to &lt;pin #&gt; is given in the following table:

0-31 pins P0.0 to P0.31
100-131 pins P1.0 to P1.31
200-231 pins P2.0 to P2.31
300-331 pins P3.0 to P3.31
400-431 pins P4.0 to P4.31

Use the command **SETPIN** &lt;pin #&gt;,4 first to set up the pin as a GPIO output.

**OUTDAC** &lt;dac #&gt;,&lt;value&gt;  
Outputs an analog signal on DAC &lt;dac #&gt; proportional to &lt;value&gt;. Currently only DAC 0 is supported (there is only one DAC on the LPC2148). Must do **SETPIN** 25,2 on LPC2148 to set up the DAC as an output pin first. For an alternative "analog" output signal, see PWM. This does nothing on LPC2106/LPC2119.

x=**PEEK**(&lt;address&gt;)  
Reads the memory location &lt;address&gt; and returns the 32-bit value. If you want only a byte you can do bitwise-AND (e.g.)

	x=PEEK(&lt;address&gt;) & 0xFF
	

**POKE** &lt;address&gt;,&lt;value&gt;  
Writes the memory location at location &lt;address&gt; with the 32-bit value &lt;value&gt;.

**PRINT** &lt;number&gt;  
**PRINT** &lt;string&gt;  
**PRINT** &lt;v1&gt;,&lt;v2&gt;,...  
**PRINT** &lt;v1&gt;,&lt;v2&gt;,... ;  
Writes characters to the terminal. The ASCII representation of a number in decimal is output for a number, or the characters of a string for a string. Multiple values can be output to the terminal in the same **PRINT** statement if separated by commas. By placing a semicolon at the end of the line, no linefeed is output at the end of the **PRINT** statement. The "?" character can be used as an alias for **PRINT**.

**PWM** &lt;pwm #&gt;,&lt;total count&gt;,&lt;fractional count&gt;  
Outputs a pulse width modulated output signal on PWM &lt;pwm #&gt;. &lt;pwm #&gt; is from 1 to 6, though on the LPC2148 **PWM** 1 and 3 are unavailable because they are used for UART0. Must prepare the pin first using the appropriate **SETPIN** command. The duty cycle set up on the pin is given by &lt;fractional count&gt; divided by &lt;total count&gt;. By using a low pass filter on the pin one can obtain an analog signal proportional to the duty cycle. NOTE: all of the **PWM** channels have the same &lt;total count&gt;, so changing the &lt;total count&gt; for one channels changes &lt;total count&gt; for the rest of them. Therefore one should usually pass the same total count value always when using the PWM command.

**REM** &lt;remark&gt;  
Allows a comment to be placed in the code. If the comment is not in quotes, it will be tokenized e.g.: **REM** I want to print the value will turn into **REM** I want to **PRINT** the value Alternatively, **REM** "I want to print the value" is not changed.

**REPEAT**  
Repeats a section of code until a corresponding **UNTIL** condition is satisfied. Do not **GOTO** or **IF**/**THEN** out of a **REPEAT**/**UNTIL** loop, and therefore do not skip over the **UNTIL**, otherwise the loop with not be properly unwound. Always use the **UNTIL** to exit the **REPEAT**/**UNTIL** loop (this BASIC has no notion of program blocks).

x$=**RIGHT$**(&lt;string&gt;,&lt;num characters&gt;)  
Returns the last &lt;num characters&gt; characters of string &lt;string&gt;.

x=**RND**(&lt;number&gt;)  
Returns a pseudorandom number between 0 and &lt;number&gt;-1 if &lt;number&gt; is positive. If &lt;number&gt; is negative, the value -&lt;number&gt; is used to seed the random number generator.

**RETURN**  
Return to the statement after the calling **GOSUB** command.

**SERBNG** &lt;ercode&gt;=**SERBNG**(&lt;pin #&gt;,&lt;speed&gt;,&lt;string&gt;)  
Outputs a RS232 compliant serial stream over the &lt;pin #&gt; pin with 8 bits, 1 stop bit, no parity. This is bit-banged so any pin can be used to output data. The &lt;speed&gt; is how long each bit should be delayed. For a 48 MHz LPC2368, speed=295 corresponds to 9600 bps. For example, speed=74 would correspond roughly to 38400 bps. The &lt;string&gt; is the string data to output to the pin as serial data.

**SERI2C** &lt;input bytes string&gt;=**SERI2C**(&lt;SDA pin out #&gt;,&lt;SDA pin in #&gt;,&lt;SCL pin #&gt;,&lt;addr&gt;,&lt;end transmission&gt;,&lt;input bytes to transfer&gt;,&lt;output string&gt;)  
Clocks data in or out (or both) using the I2C master protocol (currently assumes single master). &lt;SDA pin out #&gt; is used as the data output pin on the I2C bus. This must be set up as an output pin. Data input and output pins are separate because in general outputs are not open collector, so an open collector circuit must be present on the input/output SDA pins. &lt;SDA pin in #&gt; is used as the data input pin on the I2C bus. This must be set up as an input pin. &lt;SCL pin #&gt; is the clock pin (an output pin). If data is to be output, &lt;output string&gt; is a non-zero length string containing the output data. &lt;input bytes to transfer&gt; is the number of bytes to input in the transfer, which is returned as the &lt;input bytes&gt; string. If input and output are both desired, a combined I2C transfer is used (use separate commands if this is not desired). Finally, &lt;end transmission&gt; is non-zero if the transmission should be ended after the data output phase.

**SERINIT** &lt;port #&gt;,&lt;baud rate&gt;,&lt;databits&gt;,&lt;stopbits&gt;,&lt;parity&gt;  
Initializes the serial port denoted by &lt;port #&gt;. Currently two serial ports are supported denoted by &lt;port #&gt;=0 and &lt;port #&gt;=1. &lt;baud rate&gt; is in bps. &lt;databits&gt;=7 or 8 for the number of data bits, &lt;stopbits&gt;=1 or 2 for the number of stop bits, and &lt;parity&gt;=0 for no parity, &lt;parity&gt;=1 for odd parity, and &lt;parity&gt;=2 for even parity.

&lt;instring&gt;=**SERINP$**(&lt;port #&gt;,&lt;numchars&gt;,&lt;endchar&gt;,&lt;timeout&gt;)  
Reads up to &lt;numchars&gt; from serial port &lt;port #&gt;. If &lt;numchars&gt; is negative, then -&lt;numchars&gt; characters will be read, but discarded, which is useful for clearing the receive queue. It will stop receiving characters if the character denoted by &lt;endchar&gt; is received and &lt;endchar&gt; is not a negative number. The number &lt;timeout&gt; is proportional to the amount of time to wait for the characters to arrive before the **SERINP$** function terminates. Currently its about &lt;timeout&gt;=50000 per second, but this could change. Set &lt;timeout&gt;=1 to return with only the queued characters. A string is returned with the characters received. See **SERINIT** to set the serial port parameters.

x=**SEROUT**(&lt;port #&gt;,&lt;string&gt;)  
Send the characters in &lt;string&gt; out serial port number &lt;port #&gt;. The return code currently is zero, but would be negative for an error code. See SERINIT to set the serial port parameters.

**SERSPI** &lt;spibytein&gt;=**SERSPI**(&lt;MOSI pin #&gt;,&lt;MISO pin #&gt;,&lt;CLK pin #&gt;,&lt;output data&gt;,&lt;delay&gt;)  
Clocks a 8-bit byte using the SPI master protocol with CPOL=0 and CPHA=0. &lt;MOSI pin #&gt; is used as a MOSI pin (must be set up as an output pin), &lt;MISO pin #&gt; is used as a MISO pin (must be set up as an input pin), &lt;CLK pin #&gt; is the clock pin (an output pin), and &lt;output data&gt; is a byte to output for the data cycle. The &lt;delay&gt; controls the speed at which the clock runs, larger values are slower. The received byte is output by the function. Any select lines must be handled separately.

&lt;oldstate&gt;=**SETIDLE**(&lt;idlestate&gt;)  
Sets whether or not the idle task should be run during program execution. &lt;idlestate&gt; is zero for normal idle task running, and nonzero to suspend idle tasks. The idle task includes network services, so network services will not resume until "x=**SETIDLE**(0)" is used. The old idle state is returned in &lt;oldstate&gt;. This function is useful to make sure the maximum performance is available for critical tasks. However, if the idle state is accidentally left disabled, network access will not be available, and the controller may be unreachable by the network.

**SETPIN** &lt;pin #&gt;,&lt;function&gt;  
**SETPIN** sets up the pin &lt;pin #&gt; to have function denoted by number &lt;function&gt;. The actual pin corresponding to &lt;pin #&gt; is given in the following table:

0-31 pins P0.0 to P0.31
100-131 pins P1.0 to P1.31
200-231 pins P2.0 to P2.31
300-331 pins P3.0 to P3.31
400-431 pins P4.0 to P4.31. 

The meaning of the function number depends on the pin. Here are the examples of how to use **SETPIN**, for the LPC2148:

	SETPIN ,0     - set up any pin P0.2-P0.31 as GPIO input
	SETPIN ,4     - set up any pin P0.2-P0.31 as GPIO output

	SETPIN 7,2           - set up P0.7 as PWM 2
	SETPIN 8,2           - set up P0.8 as PWM 4
	SETPIN 21,1          - set up P0.21 as PWM 5
	SETPIN 9,2           - set up P0.9 as PWM 6

	SETPIN 28,1          - set up P0.28 as AD0.1
	SETPIN 29,1          - set up P0.29 as AD0.2
	SETPIN 30,1          - set up P0.30 as AD0.3
	SETPIN 25,1          - set up P0.25 as AD0.4
	SETPIN 4,3           - set up P0.4 as AD0.6
	SETPIN 5,3           - set up P0.5 as AD0.7

	SETPIN 25,2          - set up P0.25 as DAC
	

x$=**STRING$**(&lt;repeats&gt;,&lt;string&gt;)  
Return a string with the string &lt;string&gt; repeated &lt;repeats&gt; times. It is very easy to exhaust the memory with this command.

x$=**TIMEGT$**  
Gets the current time to a string in the format "YYYY-MM-DD HH:MM:SS" where YYYY is the year, MM is the month, DD is the day, HH is the hour (in 24 hour format), MM is the minute, and SS is the second. Currently the RTC only works when a 32.768 kHz crystal is attached to the LPC2148 (what's the point otherwise?).

x=**TIMESET**(&lt;setstring&gt;)  
Sets the time, with a string in the format

"YYYY-MM-DD HH:MM:SS"

. Returns zero for no error, or a negative number for error (improperly formatted string).

**TONE** &lt;pin #&gt;,&lt;duration&gt;,&lt;frequency&gt;  
Toggle the pin &lt;pin #&gt; high and low for a duration of &lt;duration&gt; milliseconds at a frequency of &lt;frequency&gt; cycles per second (Hz). The pin must be set as a GPIO output first, e.g. **SETPIN** &lt;pin #&gt;,4. The frequency/duration is not very accurate.

x$=**TRIM$**(&lt;string&gt;,&lt;mode&gt;)  
Returns a new string with the spaces, tabs, newlines, and carriage returns removed from the end of the string if mode=0, or the beginning of the string if mode=1. If mode is omitted, mode=0 is assumed.

**UNTIL** &lt;condition&gt;  
If the &lt;condition&gt; is false, return to the last **REPEAT**, otherwise continue with the next statement. Do not skip over an **UNTIL** with a **GOTO**/b statement, the **UNTIL** must always evaluate to true to remove **UNTIL** from **REPEAT**/**UNTIL** stack.

x$=**UPPER$**(&lt;string&gt;,&lt;mode&gt;)  
Returns a new string with the letters A-Z converted to uppercase if mode=0, or lowercase if mode=1. If mode is omitted, uppercase conversion is assumed.

x=**VAL**(&lt;string&gt;)  
Returns a numerical representation of the string &lt;string&gt;.

x=**VALLEN**(&lt;string&gt;)  
Returns the number of characters in the numerical representation of the string &lt;string&gt;. e.g.

x=VALLEN("100")

assigns 3 to x.

x=VALLEN("100blah")

assigns 3 to x

**WAIT** &lt;milliseconds&gt;  
Waits a certain number of milliseconds.

**

# Interrupt-Driven I/O Commands. If the interrupt mode is on, **INADC**/**OUTDAC** can not be used.

**

**ADSTOP**  
Terminates the interrupt driven input and output. Any data remaining in ADC buffer can still be read out. The interrupt-based I/O can be started with **ADSTART**.

**ADSTART** &lt;inadcchans&gt;,&lt;outddacchans&gt;,&lt;clkdiv&gt;,&lt;indiv&gt;,&lt;outdiv&gt;  
Turns on the interrupt based I/O. If trigger mode is zero, then input and output begins immediately, otherwise the I/O begins when the trigger condition is met (see **ADTRIG**).

&lt;inadcchans&gt; = a bit mask selecting which analog input channels should be sampled. The ADC alternates sampling between all of the chosen active sampled channels, so that, for example, if two channels are selected they will both be sampled at half the rate of one channel. The value for the &lt;inadcchans&gt; bit mask can be calculated as follows by adding together the following values:

	ADC Input   Add to mask
	AD0.0       1
	AD0.1       2
	AD0.2       4
	AD0.3       8
	AD0.4       16
	AD0.5       32
	AD0.6       64
	AD0.7       128
	

&lt;outdacchans&gt; = The value for the &lt;outdacchans&gt; bit mask can be calculated as follows by adding together (there is currently only one DAC on the LPC series)

	DAC Output  Add to mask
	DAC0        1
	

*   &lt;clkdiv&gt; = A clock divider that determines how often the I/O interrupt should be called. This is divided from the peripheral clock. For example, if the peripheral clock is 60 MHz, and clkdiv=60, then the interrupt is called at (60 Mhz)/(10\*60) = 100 kHz (the 10 comes from 10-bit successive approximation ADC). Currently the interrupt overhead is large and therefore dividers less than 60 are not recommended.
*   &lt;indiv&gt; = Determines the downsampling rate for which data should be recorded. For example, if &lt;indiv&gt;=4 then the result of each ADC will only be recorded every fourth sample. For example, if two ADC channels are sampled, and &lt;clkdiv&gt;=60 and &lt;indiv&gt;=4, then the interrupt rate is 60 Mhz/(10\*60) = 100 kHz, and since only every fourth sample is recorded from two channels, then the actual sampling rate is 100 kHz/(2\*4) = 12.5 kHz
*   &lt;outdiv&gt; = Determines the output sample rate for the DAC. The calculation of the rate is the same as &lt;indiv&gt;, but currently there is only one DAC channel available.

**ADTRIG** &lt;mode&gt;,&lt;chan&gt;,&lt;level&gt;  
Selects the trigger mode to decide when after the **ADSTART** command is issued the ADC/DAC starts sampling data.  

*   &lt;mode&gt;=0 Turn off triggering so that data acquisition starts immediately upon issuing **ADSTART** command, or the issue of this command if **ADSTART** has already been issued.
*   &lt;mode&gt;=1 Pause acquisition so that data acquisition only restarts when the triggering is set to a different mode.
*   &lt;mode&gt;=2 Trigger acquisition so that it begins when the channel number indicated by &lt;chan&gt; rises above the level set by &lt;level&gt;.
*   &lt;mode&gt;=3 Trigger acquisition so that it begins when the channel number indicated by &lt;chan&gt; drops below the level set by &lt;level&gt;.
*   &lt;chan&gt; - set which channel is to be monitored for trigger in modes 2 and 3.
*   &lt;level&gt; - set the level at which triggering should occur for modes 2 and 3.

code=**ADWRITE**(&lt;datastring&gt;)  
Feeds data into the output DAC buffer. The &lt;datastring&gt; is a string consisting of concatenated byte pairs, each pair a sample to output to the DAC. The byte pairs are little-endian, with the least significant 8-bits in the first byte, and the most significant 4-bits in the least significant bits of the second byte. Currently the range of samples that can be output to the DAC are from 0 to 1023. The high four bits in the second byte are reserved for a channel number (currently ignored). The length of &lt;datastring&gt; should be two times the number of samples to output. The following table summarizes the output sample format:

		  
	first byte   second byte
	xxxxxxxx     yyyyzzzz 
	
	xxxxxxxx=sample(7:0)
	zzzz=sample(11:8)
	yyyy=DAC channel(3:0)   (0 to 15)
	

The returned code gives the number of samples that were actually inserted into the output buffer.

code=**ADWRLFT**  
Returns the number of samples that can be inserted into the write buffer before it is full.

&lt;datastring&gt;&gt;=**ADREAD$**(&lt;numsamples&gt;)  
Reads &lt;numsamples&gt; samples out of the ADC input buffer, and returns the samples as a string &lt;datastring&gt;. The string &lt;datastring&gt; consists of concatenated byte pairs, each pair a sample read from the ADC buffer. The length of &lt;datastring&gt; is the number of samples read times two. The byte pairs are little-endian, with the least significant 8-bits of the read sample in the first byte, and the most signifcant 4-bits of the sample in the least significant bits of the second byte. The high four bits of the second byte are the ADC channel from which the sample was recorded. The following table summarizes the input sample format:

	first byte   second byte
	xxxxxxxx     yyyyzzzz 
	
	xxxxxxxx=sample(7:0)
	zzzz=sample(11:8)
	yyyy=ADC channel(3:0)   (0 to 15)
	

code=**ADRDBUF**  
Returns the number of samples available to be read from the input sample buffer.

**

# Web commands. These are only present if network support is compiled in.

**

wc=**WEBCON**  
Get the web connection number for the currently open web connection to the controller, or -1 if no connection is currently open.

x$=**WEBESC$**(&lt;string&gt;)  
Returns the HTML equivalent of &lt;string&gt; with & as &amp; " as &quot; &lt; as &lt; and &gt; as &gt; so that these characters can be included in a document or HTML form data without inadvertently initiating or ending HTML tags.

x$=**WEBFRM$**(&lt;poststring&gt;,&lt;field;&gt;)  
Parse the form data &lt;field&gt; out of the string &lt;poststring&gt; and return it in x$, or a string of zero length if &lt;field&gt; does not exist.

vx=**WEBOUT**(&lt;webconnection&gt;,&lt;status&gt;,&lt;htmlstring&gt;)  
Output the string &lt;htmlstring&gt; on the connection &lt;webconnection&gt;. If the connection exists and the transmission is successful, x=0, otherwise x=-1. If &lt;status&gt;=0 then continue the connection, otherwise if &lt;status&gt;&lt;0 then terminate the connection after transmitting &lt;htmlstring&gt;. Will block until data is transmitted, or current connection is terminated.

x$=**WEBPST$**(&lt;webconnection&gt;,&lt;numchars&gt;)  
Get the POST data (e.g. for HTML forms) from webconnection &lt;webconnection&gt; to string x$. &lt;numchars&gt; is the maximum number of POST data characters to allocate. This is useful in conjunction with **WEBFRM$** to get HTML forms. Will block until FORM data is received, or current connection is terminated. Note, the **WEBPST$** command must be performed before attempting to send data with **WEBOUT**, or the WEBOUT will block. The following is a useful way to get a web post:

	  waitfr: wc=WEBCON
	  if wc=-1 then waitfr:
	  if webreq$(wc,1)&lt;&gt;"P" then nopost:
	  a$=webpst$(wc,1000)
	  nopost: print "continuing"
	  

x$=**WEBREQ$**(&lt;webconnection&gt;,&lt;code #&gt;)  
Gets information about the current web connection to the string x$. Currently two &lt;code #&gt; are supported. &lt;code #&gt;=0 retrieves the current URL, and &lt;code #&gt;=1 retrieves "P" if the current connection is a POST, or "G" if it is a GET.

x=**WEBMAIL**(&lt;tostring&gt;,&lt;ccstring&gt;,&lt;fromstring&gt;,&lt;subjectstring&gt;,&lt;messagestring&gt;)  
Sends an E-mail to the address &lt;tostring&gt;, from the address &lt;fromstring&gt;, with a carbon copy line &lt;ccstring&gt;, subject &lt;subjectstring&gt;, and a message &lt;messagestring&gt;. Note: the configuration settings HOSTNAME must be set to the hostname, and SMTP must be set to the IP address of the SMTP server, so that the mail can be relayed.

**

# Socket commands (experimental). These are only present if network support is compiled in.

**

&lt;socket number&gt;=**SCACEPT**(&lt;port number&gt;,&lt;buffer size&gt;)  
Accepts a connection on port number &lt;port number&gt;. Uses an incoming buffer size of &lt;buffer size&gt; bytes for the connection. The buffer size should be at least as big as one packet (800 bytes). The return is either the socket number or -1 for no connection available.

&lt;errorcode&gt;=**SCCLOSE**(&lt;socket number&gt;)  
Closes the socket associated with &lt;socket number&gt;. Returns zero for no error, or a negative number for an error.

&lt;socket number&gt;=**SCCON**(&lt;IP address string&gt;,&lt;port number&gt;,&lt;buffer size&gt;)  
Connects to a socket on a remote port &lt;port number&gt; on a machine with IP address &lt;IP address string&gt; (e.g. "10.0.0.4"). The buffer size for incoming data is given by &lt;buffer size&gt; bytes. This size should be at least as big as one packet (800 bytes). Returns a socket number for the connection. The connection will not be made immediately, one should use SCISCON() to see if the connection has been made before attempting to send/receive data over the connection..

&lt;result&gt;=**SCISCON**(&lt;socket number&gt;)  
Returns whether or not the connection given by &lt;socket number&gt; is currently connected or not. Returns zero for not connected, or one for connected.

&lt;bytes&gt;=**SCNEWDT**(&lt;socket number&gt;)  
Returns the number of bytes available to be read on the socket denoted by &lt;socket number&gt;. Returns zero for no available data, or a number of bytes for available data.

&lt;read string&gt;=**SCREAD$**(&lt;socket number&gt;,&lt;number of bytes&gt;,&lt;end character number&gt;)  
Reads up to &lt;number of bytes&gt; data from the socket &lt;socket number&gt;. If the &lt;end character number&gt; is a number between 0 and 255, the reading will stop at the byte with the value given by &lt;end character number&gt;. If &lt;end character number&gt; is -1, then it waits for &lt;number of bytes&gt; characters (unless the socket is closed). If &lt;end character number&gt; is -2, then it does not block and only returns with the characters immediately available up to &lt;number of bytes&gt; bytes.

&lt;error code&gt;=**SCWRITE**(&lt;socket number&gt;,&lt;write string&gt;)  
Output the string data &lt;write string&gt; over the socket given by &lt;socket number&gt;. Returns zero for no error sending data, or a negative number for an error.

**

# File commands (to SD/MMC card). These are only present if FATFS support is compiled in.

**

Error codes for SD/MMC command

		 0	FR\_OK (no error)
		 1	FR\_BASIC,	    	
		 2	FR\_NOT\_READY,		
		 3	FR\_NO\_FILE,			
		 4	FR\_NO\_PATH,			
		 5	FR\_INVALID\_NAME,	
		 6	FR\_INVALID\_DRIVE,	
		 7	FR\_DENIED,			
		 8	FR\_EXIST,			
		 9	FR\_RW\_ERROR,		
		 10	FR\_WRITE\_PROTECTED,	
		 11	FR\_NOT\_ENABLED,		
		 12	FR\_NO\_FILESYSTEM,	
		 13	FR\_INVALID\_OBJECT,	
		 14	FR\_MKFS\_ABORTED
		 

&lt;ercode&gt;=**FCLOSE**(&lt;fileno&gt;)  
Closes the file given by &lt;fileno&gt;. The file associated with &lt;fileno&gt; must have been opened with the **FOPEN**() function. Returns an error code if there is a problem, but it always deallocates the file structure if &lt;fileno&gt; points to a valid structure.

&lt;ercode&gt;=**FDEL**(&lt;filenamestring&gt;)  
Deletes the file given by the name &lt;filenamestring&gt;. Returns an error code if there is a problem.

&lt;ercode&gt;=**FDIRCL**(&lt;fileno&gt;)  
Closes the directory given by &lt;fileno&gt;. The directory associated with &lt;fileno&gt; must have been opened with the **FDIROP**() function. Returns an error code if there is a problem, but always deallocates the directory structure if &lt;fileno&gt; points to a valid structure.

&lt;fileno&gt;=**FDIROP**(&lt;directorystring&gt;)  
Opens a directory with the name &lt;directorystring&gt;. Returns an error code if there is a problem (negative), or a positive number with the fileno.

vx$=**FDIRRD$**(&lt;fileno&gt;)  
Gets the next entry from the directory listing given by the open directory indicated by &lt;fileno&gt;. The directory associated with &lt;fileno&gt; must have been opened with the **FDIROP**() function. Returns a blank string if the directory is ended, or if there is a problem, otherwise it returns a string in the following format:

	temp.fil      DRHSA 1980/01/01 01:01 20000
	Filename      flags create time/date length
	

The flags have the format "DRHSA" with the meanings:

	D: is a directory.
	R: is a read-only file.
	H: is a hidden file.
	S: is a system file.
	A: is an archive file.
	

Either the corresponding letter or a "-" is in a particular position.

The contents of this string are always in the same locations (e.g. the filename is characters 1-12). The **TRIM$** function is useful for removing spaces from data parsed from this string using **MID$**().

&lt;feofflag&gt;=**FEOF**(&lt;fileno&gt;)  
Determines whether the end of file has been reached. The file associated with &lt;fileno&gt; must have been opened with the FOPEN() function. Returns an error code if there is a problem, or zero if the end of the file is not reached, or one if it is reached.

&lt;ercode&gt;=**FMKDIR**(&lt;directorystring&gt;)  
Creates a new directory with the name given by &lt;directorystring&gt;. Returns an error code if there is a problem.

&lt;fileno&gt;=**FOPEN**(&lt;filenamestring&gt;,&lt;flagstring&gt;)  
Opens a file given by the file &lt;filenamestring&gt;. Returns an error code if an error is encountered (negative number), or the fileno of the opened file. All files are automatically closed when a program is ended. The string &lt;flagstring&gt; can contain the following characters indicating how to open the file: "**R**"=_read_, "**W**"=_write_, "**C**"=_create new file_, "**A**"=_always create file_, "**O**"=_always open file._

x$=**FREAD$**(&lt;fileno&gt;,&lt;chars&gt;)  
Read &lt;chars&gt; number of characters out of the file given by &lt;fileno&gt;. The file associated with &lt;fileno&gt; must have been opened with the **FOPEN**() function. Returns a string of the length of the actual number of readable characters from the file, or a blank string if there was an error or no characters could be read. Is handy in conjunction with FEOF() to determine if the end of the file has been reached.

x$=**FREADL$**(&lt;fileno&gt;,&lt;chars&gt;)  
Read &lt;chars&gt; number of characters out of the file given by &lt;fileno&gt;, but stops at the first newline (linefeed '\\n'). The linefeed is included in the string. The **TRIM$** function is handy to remove the linefeed. The file associated with &lt;fileno&gt; must have been opened with the **FOPEN**() function. Returns a string of the length of the actual number of readable characters from the file, or a blank string if there was an error or no characters could be read. Is handy in conjunction with **FEOF**() to determine if the end of the file has been reached.

&lt;ercode&gt;=**FSEEK**(&lt;fileno&gt;,&lt;offset&gt;,&lt;mode&gt;)  
Moves the file pointer to a new location in the file. The file associated with &lt;fileno&gt; must have been opened with the FOPEN() function. Returns an error code if there is a problem, or zero if there is no problem. The mode determines the behavior. &lt;offset&gt;=0 means seek relative to the beginning of the file. &lt;offset&gt;=1 means seek relative to current position. &lt;offset&gt;=2 means seek relative to end of the file, so that FSEEK(&lt;fileno&gt;,0,2) seeks to the end of the file.

x$=**FSTAT$**(&lt;filenamestring&gt;)  
Gets the information about the file given by &lt;filenamestring&gt;. Returns a blank string if the file does not exist, or if there is a problem, otherwise it returns a string in the following format:

	temp          DRHSA 1980/0 /0  0 :0  20000
	Filename      flags create time/date length
	

See **FDIRRD$** for details on the format.

&lt;ercode&gt;=**FSYNC**(&lt;fileno&gt;)  
Writes any unwritten data to the file given by &lt;fileno&gt;, but does not close the file. The file associated with &lt;fileno&gt; must have been opened with the **FOPEN**() function. Returns an error code if there is a problem.

&lt;offset&gt;=**FTELL**(&lt;fileno&gt;)  
Returns the current file pointer corresponding to &lt;fileno&gt;, or a negative error code otherwise. The file associated with &lt;fileno&gt; must have been opened with the **FOPEN**() function.

&lt;ercode&gt;=**FWRITE**(&lt;fileno&gt;,&lt;string&gt;)  
Writes the data in &lt;string&gt; to the file denoted by &lt;fileno&gt;. The file associated with &lt;fileno&gt; must have been opened with the **FOPEN**() function. Returns an error code if there is a problem, or zero if there is no error. Note a newline is NOT automatically added to the &lt;string&gt;, you must add it manually (e.g. &lt;string&gt;+**CHR$**(10)).
