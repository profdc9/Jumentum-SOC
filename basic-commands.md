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
	

A statement is either an assigment of _<variable> = <expression>_ (as in the LET statement, but the actual word LET may be omitted), or one of the commands below.

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

*   a<b conditional evaluating whether a is less than b
*   a>b conditional evaluating whether a is greater than b
*   a=b conditional evaluating whether a is equal to b
*   a<=b conditional evaluating whether a is less than or equal to b
*   a>=b conditional evaluating whether a is greater than or equal to b

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
	PRINT A$>"GOODBYE"
	

it will fail, because the parse things that A$ is a string first, before it evaluates the whole expression A$>"GOODBYE". However, all expressions in parenthesis are assumed to evaluate to numbers, so you can do

    PRINT (A$>"GOODBYE")
	

Strings are 8-bit clean (they can contain zero-valued bytes), so they can be used as hardware buffers.

**

# The basic commands are described below:

**

**ABORT** <label:>  
Jumps to line denoted by <label:>. Unwinds all GOSUB, REPEAT/UNTIL, and FOR/NEXT loops. Good for error recovery to ensure the program will continue to run.

x=**ABS**(<number>)  
Takes the absolute value of number.

x=**ASCII**(<string>)  
Returns an integer 0 to 255 giving the character code value of the first character of <string>. An alternative x=ASCII(<string>,n) gives the code of the nth character (or -1 if the string is too short to have an nth character).

x$=**BIN$**(x,<digs>)  
Compute the string equivalent of the raw data value of x. <digs> is the number of bytes to use in x$. Negative <digs> is big-endian, positive <digs> is little endian. This is the complement of **BSTR**.

**BOLD** <value>  
A non-zero value outputs a VT100/ANSI code to turn on bold type, a zero value turns off bold.

x=**BSTR**(x$,<digs>)  
Compute the numerical equivalent of the raw data value of x$. <digs> is the number of bytes to use in x$. Negative <digs> is big-endian, positive <digs> is little endian. This is the complement of **BIN$**.

**CHR$**(<value>)  
Returns a string with a single character with character code <value> (from 0 to 255).

**CLREOL**  
Outputs a VT100/ANSI code to clear the current line from the cursor position to the end of the line.

**CLRSCR**  
Outputs a VT100/ANSI code to clear the screen and place the cursor at the upper left corner.

x$=**CONF$**(<confstring>)  
Reads the configuration variable out of the configuration file and places it into x$. E.g. if there was a line in the configuration file such as MYNAME=Joe Brown then x$=**CONF$**("MYNAME") places "Joe Brown" into x$

**DIM A**(<index 1>)  
**DIM A**(<index 1>,<index 2>)  
**DIM A$**(<index 1>>)  
**DIM A$**(<index 1>,<index 2>)  
These commands create arrays of strings or numbers, either one or two dimensional. These can be initialized to particular values by using the =, e.g.

	DIM A(10)=10,9,8,7,6,5,4,3,2,1
	DIM NOTES$(7)="Do","Re","Mi","Fa","So","La","Si"
	

**DOLINE** <CMDSTRING>,<ERRORCODE>  
This command allows one to dynamically execute a limited set of BASIC commands. The command to be executed is in the string <CMDSTRING>. The command is interpreted as a BASIC command and executed and the error code result is placed in the numeric variable <ERRORCODE>. The commands IF/THEN, FOR/NEXT, GOSUB, RETURN, GOTO, REPEAT, UNTIL, ONERROR, ABORT, END, and DOLINE can not be used in this command. Examples:  

	DOLINE "z=5",er
	DOLINE "PRINT ""hello""",er
	

The first example assigns the value 5 to the variable "z," and zero to er to indicate no error. The second example prints hello. Note: this command is highly susceptible to phishing attacks from scripts, you have been warned.

**END**  
Terminates the program.

x$=**ERC$**(<errorcodenumber>)  
Return a string describing the error associated with errorcode number. If number is negative, then reports the last error.

x=**ERR**(0)  
This retrieves either the error code of the last error for **ONERROR**. Reading this clears the error code so subsequent accesses return 0 until another error is thrown. x=**ERR**(1) This retrieves the line number of the last error for **ONERROR**. Reading this clears the line number of the error so subsequent access return 0 until another error is thrown. In the editor, the line number of a particular statement can be jumped to by using the CTRL-G command.

**FOR** <var>=<begin> **TO** <end> **STEP** <step>  
Iterates <var> starting at <begin> until it reaches <end>, each time through the loop incrementing the step size the value <step>. <step> may be negative if <end> is smaller than <begin>. The end of the loop is **NEXT** <var>.

e.g.

	FOR I=99 TO 1 STEP -1
	PRINT I," bottles of beer on the wall"
	NEXT I
	

**GOSUB** <label:>  
Calls a subroutine at the line labeled with <label:>. Control returns to the statement after the **GOSUB** command when a **RETURN** is encountered.

**GOTO** <label:>  
Jumps program execution to the line labeled by <label:>.

x$=**HEX$**(<number>)  
Returns a string representation of the unsigned hexadecimal equivalent of <number>. Decimal can be returned with **STR$**.

**IF** <condition> **THEN** <labelthen:>  
**IF** <condition> **THEN** <labelthen:> **ELSE** <labelelse:> If the condition is true, then jump program execution to the line labeled by <labelthen:>. If **ELSE** is included, then jump to the line labeled by <labelelse:> if the condition is false.

x=**IIF**(<condition>,<truevalue>,<falsevalue>)  
If the condition is true, then return <truevalue> otherwise return <falsevalue>. Note that both <truevalue> and <falsevalue> are evaluated even though only one is used.

x$=**II$**(<condition>,<truestring>,<falsestring>)  
If the condition is true, then return <truestring>, otherwise return <falsestring>. Note that both <truestring> and <falsestring> are evaluated even though only one is used.

x=**INADC**(<ad #>)  
Gets the analog value at analog to digital converter numbered from 0-15. The <ad #> from 0-7 corresponds to AD0.<ad #> and the <ad #> from 8-15 corresponds to AD1.<ad #>. Must do a **SETPIN** first to the appropriate pin to set up the ADC (see **SETPIN** examples).

x=**IND**(<pin #>)  
Get the digital input at the pin number denoted by . The actual pin for the <pin #> is given in the following table:

0-31 pins P0.0 to P0.31
100-131 pins P1.0 to P1.31
200-231 pins P2.0 to P2.31
300-331 pins P3.0 to P3.31
400-431 pins P4.0 to P4.31

Use the **SETPIN** <pin #>,0 command first to set up the pin as a GPIO input.

x=**INKEY**  
Returns the character code of the currently entered character on the terminal UART0, or -1 if no character is available.

x=**INPLEN**(<length>)  
Sets the maximum number of characters to accept for subsequent **INPUT** statements. Returns the old **INPLEN** value. The input length maximum defaults to 80 characters.

**INPUT** <numerical variable>  
**INPUT** <string variable>  
Takes input from the terminal UART0 and places a numerical representation in <numerical variable> of the number entered at the terminal, or into <string variable$> the string entered at the terminal. The entered strings have a maximum size set by the INPLEN commands, which defaults to 80 characters.

x=**INSTR**(<search string>,<substring>,<index>)  
Searches for the first occurrence of string <substring> in the string <search string> that is at or after the character index <index>, with <index>=1 being the first character. If the substring is not found, zero is returned.

**INT**(<number>)  
Returns the integer representation of the number <number> (does nothing unless floating point is compiled in).

x$=**LEFT$**(<string>,<num characters>)  
Returns the first <num characters> characters of string <string>.

x=**LEN**(<string>)  
Returns the number of characters in <string>.

**LET** <variable>=<expression>  
Assigns the variable <variable> to the expression <expression>. If the expression is a string, then the variable must be a string variable. or if the expression is a number then the variable must be a numeric variable. **LET** is optional, e.g.  
X = 5  
is a valid statement (the **LET** may be omitted).

**LOCATE** <y cursor position>,<x cursor position>  
Sends the VT100/ANSI code to the terminal to position the cursor at y position <y cursor position> and x position <x cursor position>.

x$=**MID$**(<string>,<index>,<num characters>)  
Returns the <num characters> characters starting at index <index> in string <string> as a new string. If <num characters>=-1 or is omitted then the remainder of the string to the end is returned.

**MID$**(<string>,<index>,<num characters>)=<substring>  
Substitutes characters from <substring> into <string> starting at index <index>. If <num characters> is specified, then it substitutes only <num characters> into <string> (if that many are available), or if not specified, substitutes all the characters from <substring>. The string <string> is extended if the <substring> overwrites the end of <string>.

**NEXT** <variable>  
Ends a **FOR**/**NEXT** loop (see **FOR**).

**ONERROR** <label:>  
On an error, jump to the line labeled by <label:>. Note that after an error is trapped, **ONERROR** must be reissued to trap another error (to prevent a potential infinite loop). Omitting the label restores built-in error handling (which terminates program execution). This is useful with **ERR**() to get the error code, and **ABORT** to unwind the **GOSUB**/**REPEAT-UNTIL**/**FOR-NEXT** stacks.

**OUTD** <pin #>,<state>  
Output a low for state=0, or a high for state=1 to the pin number denoted by <pin #>. The actual pin corresponding to <pin #> is given in the following table:

0-31 pins P0.0 to P0.31
100-131 pins P1.0 to P1.31
200-231 pins P2.0 to P2.31
300-331 pins P3.0 to P3.31
400-431 pins P4.0 to P4.31

Use the command **SETPIN** <pin #>,4 first to set up the pin as a GPIO output.

**OUTDAC** <dac #>,<value>  
Outputs an analog signal on DAC <dac #> proportional to <value>. Currently only DAC 0 is supported (there is only one DAC on the LPC2148). Must do **SETPIN** 25,2 on LPC2148 to set up the DAC as an output pin first. For an alternative "analog" output signal, see PWM. This does nothing on LPC2106/LPC2119.

x=**PEEK**(<address>)  
Reads the memory location <address> and returns the 32-bit value. If you want only a byte you can do bitwise-AND (e.g.)

	x=PEEK(<address>) & 0xFF
	

**POKE** <address>,<value>  
Writes the memory location at location <address> with the 32-bit value <value>.

**PRINT** <number>  
**PRINT** <string>  
**PRINT** <v1>,<v2>,...  
**PRINT** <v1>,<v2>,... ;  
Writes characters to the terminal. The ASCII representation of a number in decimal is output for a number, or the characters of a string for a string. Multiple values can be output to the terminal in the same **PRINT** statement if separated by commas. By placing a semicolon at the end of the line, no linefeed is output at the end of the **PRINT** statement. The "?" character can be used as an alias for **PRINT**.

**PWM** <pwm #>,<total count>,<fractional count>  
Outputs a pulse width modulated output signal on PWM <pwm #>. <pwm #> is from 1 to 6, though on the LPC2148 **PWM** 1 and 3 are unavailable because they are used for UART0. Must prepare the pin first using the appropriate **SETPIN** command. The duty cycle set up on the pin is given by <fractional count> divided by <total count>. By using a low pass filter on the pin one can obtain an analog signal proportional to the duty cycle. NOTE: all of the **PWM** channels have the same <total count>, so changing the <total count> for one channels changes <total count> for the rest of them. Therefore one should usually pass the same total count value always when using the PWM command.

**REM** <remark>  
Allows a comment to be placed in the code. If the comment is not in quotes, it will be tokenized e.g.: **REM** I want to print the value will turn into **REM** I want to **PRINT** the value Alternatively, **REM** "I want to print the value" is not changed.

**REPEAT**  
Repeats a section of code until a corresponding **UNTIL** condition is satisfied. Do not **GOTO** or **IF**/**THEN** out of a **REPEAT**/**UNTIL** loop, and therefore do not skip over the **UNTIL**, otherwise the loop with not be properly unwound. Always use the **UNTIL** to exit the **REPEAT**/**UNTIL** loop (this BASIC has no notion of program blocks).

x$=**RIGHT$**(<string>,<num characters>)  
Returns the last <num characters> characters of string <string>.

x=**RND**(<number>)  
Returns a pseudorandom number between 0 and <number>-1 if <number> is positive. If <number> is negative, the value -<number> is used to seed the random number generator.

**RETURN**  
Return to the statement after the calling **GOSUB** command.

**SERBNG** <ercode>=**SERBNG**(<pin #>,<speed>,<string>)  
Outputs a RS232 compliant serial stream over the <pin #> pin with 8 bits, 1 stop bit, no parity. This is bit-banged so any pin can be used to output data. The <speed> is how long each bit should be delayed. For a 48 MHz LPC2368, speed=295 corresponds to 9600 bps. For example, speed=74 would correspond roughly to 38400 bps. The <string> is the string data to output to the pin as serial data.

**SERI2C** <input bytes string>=**SERI2C**(<SDA pin out #>,<SDA pin in #>,<SCL pin #>,<addr>,<end transmission>,<input bytes to transfer>,<output string>)  
Clocks data in or out (or both) using the I2C master protocol (currently assumes single master). <SDA pin out #> is used as the data output pin on the I2C bus. This must be set up as an output pin. Data input and output pins are separate because in general outputs are not open collector, so an open collector circuit must be present on the input/output SDA pins. <SDA pin in #> is used as the data input pin on the I2C bus. This must be set up as an input pin. <SCL pin #> is the clock pin (an output pin). If data is to be output, <output string> is a non-zero length string containing the output data. <input bytes to transfer> is the number of bytes to input in the transfer, which is returned as the <input bytes> string. If input and output are both desired, a combined I2C transfer is used (use separate commands if this is not desired). Finally, <end transmission> is non-zero if the transmission should be ended after the data output phase.

**SERINIT** <port #>,<baud rate>,<databits>,<stopbits>,<parity>  
Initializes the serial port denoted by <port #>. Currently two serial ports are supported denoted by <port #>=0 and <port #>=1. <baud rate> is in bps. <databits>=7 or 8 for the number of data bits, <stopbits>=1 or 2 for the number of stop bits, and <parity>=0 for no parity, <parity>=1 for odd parity, and <parity>=2 for even parity.

<instring>=**SERINP$**(<port #>,<numchars>,<endchar>,<timeout>)  
Reads up to <numchars> from serial port <port #>. If <numchars> is negative, then -<numchars> characters will be read, but discarded, which is useful for clearing the receive queue. It will stop receiving characters if the character denoted by <endchar> is received and <endchar> is not a negative number. The number <timeout> is proportional to the amount of time to wait for the characters to arrive before the **SERINP$** function terminates. Currently its about <timeout>=50000 per second, but this could change. Set <timeout>=1 to return with only the queued characters. A string is returned with the characters received. See **SERINIT** to set the serial port parameters.

x=**SEROUT**(<port #>,<string>)  
Send the characters in <string> out serial port number <port #>. The return code currently is zero, but would be negative for an error code. See SERINIT to set the serial port parameters.

**SERSPI** <spibytein>=**SERSPI**(<MOSI pin #>,<MISO pin #>,<CLK pin #>,<output data>,<delay>)  
Clocks a 8-bit byte using the SPI master protocol with CPOL=0 and CPHA=0. <MOSI pin #> is used as a MOSI pin (must be set up as an output pin), <MISO pin #> is used as a MISO pin (must be set up as an input pin), <CLK pin #> is the clock pin (an output pin), and <output data> is a byte to output for the data cycle. The <delay> controls the speed at which the clock runs, larger values are slower. The received byte is output by the function. Any select lines must be handled separately.

<oldstate>=**SETIDLE**(<idlestate>)  
Sets whether or not the idle task should be run during program execution. <idlestate> is zero for normal idle task running, and nonzero to suspend idle tasks. The idle task includes network services, so network services will not resume until "x=**SETIDLE**(0)" is used. The old idle state is returned in <oldstate>. This function is useful to make sure the maximum performance is available for critical tasks. However, if the idle state is accidentally left disabled, network access will not be available, and the controller may be unreachable by the network.

**SETPIN** <pin #>,<function>  
**SETPIN** sets up the pin <pin #> to have function denoted by number <function>. The actual pin corresponding to <pin #> is given in the following table:

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
	

x$=**STRING$**(<repeats>,<string>)  
Return a string with the string <string> repeated <repeats> times. It is very easy to exhaust the memory with this command.

x$=**TIMEGT$**  
Gets the current time to a string in the format "YYYY-MM-DD HH:MM:SS" where YYYY is the year, MM is the month, DD is the day, HH is the hour (in 24 hour format), MM is the minute, and SS is the second. Currently the RTC only works when a 32.768 kHz crystal is attached to the LPC2148 (what's the point otherwise?).

x=**TIMESET**(<setstring>)  
Sets the time, with a string in the format

"YYYY-MM-DD HH:MM:SS"

. Returns zero for no error, or a negative number for error (improperly formatted string).

**TONE** <pin #>,<duration>,<frequency>  
Toggle the pin <pin #> high and low for a duration of <duration> milliseconds at a frequency of <frequency> cycles per second (Hz). The pin must be set as a GPIO output first, e.g. **SETPIN** <pin #>,4. The frequency/duration is not very accurate.

x$=**TRIM$**(<string>,<mode>)  
Returns a new string with the spaces, tabs, newlines, and carriage returns removed from the end of the string if mode=0, or the beginning of the string if mode=1. If mode is omitted, mode=0 is assumed.

**UNTIL** <condition>  
If the <condition> is false, return to the last **REPEAT**, otherwise continue with the next statement. Do not skip over an **UNTIL** with a **GOTO**/b statement, the **UNTIL** must always evaluate to true to remove **UNTIL** from **REPEAT**/**UNTIL** stack.

x$=**UPPER$**(<string>,<mode>)  
Returns a new string with the letters A-Z converted to uppercase if mode=0, or lowercase if mode=1. If mode is omitted, uppercase conversion is assumed.

x=**VAL**(<string>)  
Returns a numerical representation of the string <string>.

x=**VALLEN**(<string>)  
Returns the number of characters in the numerical representation of the string <string>. e.g.

x=VALLEN("100")

assigns 3 to x.

x=VALLEN("100blah")

assigns 3 to x

**WAIT** <milliseconds>  
Waits a certain number of milliseconds.

**

# Interrupt-Driven I/O Commands. If the interrupt mode is on, **INADC**/**OUTDAC** can not be used.

**

**ADSTOP**  
Terminates the interrupt driven input and output. Any data remaining in ADC buffer can still be read out. The interrupt-based I/O can be started with **ADSTART**.

**ADSTART** <inadcchans>,<outddacchans>,<clkdiv>,<indiv>,<outdiv>  
Turns on the interrupt based I/O. If trigger mode is zero, then input and output begins immediately, otherwise the I/O begins when the trigger condition is met (see **ADTRIG**).

<inadcchans> = a bit mask selecting which analog input channels should be sampled. The ADC alternates sampling between all of the chosen active sampled channels, so that, for example, if two channels are selected they will both be sampled at half the rate of one channel. The value for the <inadcchans> bit mask can be calculated as follows by adding together the following values:

	ADC Input   Add to mask
	AD0.0       1
	AD0.1       2
	AD0.2       4
	AD0.3       8
	AD0.4       16
	AD0.5       32
	AD0.6       64
	AD0.7       128
	

<outdacchans> = The value for the <outdacchans> bit mask can be calculated as follows by adding together (there is currently only one DAC on the LPC series)

	DAC Output  Add to mask
	DAC0        1
	

*   <clkdiv> = A clock divider that determines how often the I/O interrupt should be called. This is divided from the peripheral clock. For example, if the peripheral clock is 60 MHz, and clkdiv=60, then the interrupt is called at (60 Mhz)/(10\*60) = 100 kHz (the 10 comes from 10-bit successive approximation ADC). Currently the interrupt overhead is large and therefore dividers less than 60 are not recommended.
*   <indiv> = Determines the downsampling rate for which data should be recorded. For example, if <indiv>=4 then the result of each ADC will only be recorded every fourth sample. For example, if two ADC channels are sampled, and <clkdiv>=60 and <indiv>=4, then the interrupt rate is 60 Mhz/(10\*60) = 100 kHz, and since only every fourth sample is recorded from two channels, then the actual sampling rate is 100 kHz/(2\*4) = 12.5 kHz
*   <outdiv> = Determines the output sample rate for the DAC. The calculation of the rate is the same as <indiv>, but currently there is only one DAC channel available.

**ADTRIG** <mode>,<chan>,<level>  
Selects the trigger mode to decide when after the **ADSTART** command is issued the ADC/DAC starts sampling data.  

*   <mode>=0 Turn off triggering so that data acquisition starts immediately upon issuing **ADSTART** command, or the issue of this command if **ADSTART** has already been issued.
*   <mode>=1 Pause acquisition so that data acquisition only restarts when the triggering is set to a different mode.
*   <mode>=2 Trigger acquisition so that it begins when the channel number indicated by <chan> rises above the level set by <level>.
*   <mode>=3 Trigger acquisition so that it begins when the channel number indicated by <chan> drops below the level set by <level>.
*   <chan> - set which channel is to be monitored for trigger in modes 2 and 3.
*   <level> - set the level at which triggering should occur for modes 2 and 3.

code=**ADWRITE**(<datastring>)  
Feeds data into the output DAC buffer. The <datastring> is a string consisting of concatenated byte pairs, each pair a sample to output to the DAC. The byte pairs are little-endian, with the least significant 8-bits in the first byte, and the most significant 4-bits in the least significant bits of the second byte. Currently the range of samples that can be output to the DAC are from 0 to 1023. The high four bits in the second byte are reserved for a channel number (currently ignored). The length of <datastring> should be two times the number of samples to output. The following table summarizes the output sample format:

		  
	first byte   second byte
	xxxxxxxx     yyyyzzzz 
	
	xxxxxxxx=sample(7:0)
	zzzz=sample(11:8)
	yyyy=DAC channel(3:0)   (0 to 15)
	

The returned code gives the number of samples that were actually inserted into the output buffer.

code=**ADWRLFT**  
Returns the number of samples that can be inserted into the write buffer before it is full.

<datastring>>=**ADREAD$**(<numsamples>)  
Reads <numsamples> samples out of the ADC input buffer, and returns the samples as a string <datastring>. The string <datastring> consists of concatenated byte pairs, each pair a sample read from the ADC buffer. The length of <datastring> is the number of samples read times two. The byte pairs are little-endian, with the least significant 8-bits of the read sample in the first byte, and the most signifcant 4-bits of the sample in the least significant bits of the second byte. The high four bits of the second byte are the ADC channel from which the sample was recorded. The following table summarizes the input sample format:

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

x$=**WEBESC$**(<string>)  
Returns the HTML equivalent of <string> with & as &amp; " as &quot; < as &lt; and > as &gt; so that these characters can be included in a document or HTML form data without inadvertently initiating or ending HTML tags.

x$=**WEBFRM$**(<poststring>,<field;>)  
Parse the form data <field> out of the string <poststring> and return it in x$, or a string of zero length if <field> does not exist.

vx=**WEBOUT**(<webconnection>,<status>,<htmlstring>)  
Output the string <htmlstring> on the connection <webconnection>. If the connection exists and the transmission is successful, x=0, otherwise x=-1. If <status>=0 then continue the connection, otherwise if <status><0 then terminate the connection after transmitting <htmlstring>. Will block until data is transmitted, or current connection is terminated.

x$=**WEBPST$**(<webconnection>,<numchars>)  
Get the POST data (e.g. for HTML forms) from webconnection <webconnection> to string x$. <numchars> is the maximum number of POST data characters to allocate. This is useful in conjunction with **WEBFRM$** to get HTML forms. Will block until FORM data is received, or current connection is terminated. Note, the **WEBPST$** command must be performed before attempting to send data with **WEBOUT**, or the WEBOUT will block. The following is a useful way to get a web post:

	  waitfr: wc=WEBCON
	  if wc=-1 then waitfr:
	  if webreq$(wc,1)<>"P" then nopost:
	  a$=webpst$(wc,1000)
	  nopost: print "continuing"
	  

x$=**WEBREQ$**(<webconnection>,<code #>)  
Gets information about the current web connection to the string x$. Currently two <code #> are supported. <code #>=0 retrieves the current URL, and <code #>=1 retrieves "P" if the current connection is a POST, or "G" if it is a GET.

x=**WEBMAIL**(<tostring>,<ccstring>,<fromstring>,<subjectstring>,<messagestring>)  
Sends an E-mail to the address <tostring>, from the address <fromstring>, with a carbon copy line <ccstring>, subject <subjectstring>, and a message <messagestring>. Note: the configuration settings HOSTNAME must be set to the hostname, and SMTP must be set to the IP address of the SMTP server, so that the mail can be relayed.

**

# Socket commands (experimental). These are only present if network support is compiled in.

**

<socket number>=**SCACEPT**(<port number>,<buffer size>)  
Accepts a connection on port number <port number>. Uses an incoming buffer size of <buffer size> bytes for the connection. The buffer size should be at least as big as one packet (800 bytes). The return is either the socket number or -1 for no connection available.

<errorcode>=**SCCLOSE**(<socket number>)  
Closes the socket associated with <socket number>. Returns zero for no error, or a negative number for an error.

<socket number>=**SCCON**(<IP address string>,<port number>,<buffer size>)  
Connects to a socket on a remote port <port number> on a machine with IP address <IP address string> (e.g. "10.0.0.4"). The buffer size for incoming data is given by <buffer size> bytes. This size should be at least as big as one packet (800 bytes). Returns a socket number for the connection. The connection will not be made immediately, one should use SCISCON() to see if the connection has been made before attempting to send/receive data over the connection..

<result>=**SCISCON**(<socket number>)  
Returns whether or not the connection given by <socket number> is currently connected or not. Returns zero for not connected, or one for connected.

<bytes>=**SCNEWDT**(<socket number>)  
Returns the number of bytes available to be read on the socket denoted by <socket number>. Returns zero for no available data, or a number of bytes for available data.

<read string>=**SCREAD$**(<socket number>,<number of bytes>,<end character number>)  
Reads up to <number of bytes> data from the socket <socket number>. If the <end character number> is a number between 0 and 255, the reading will stop at the byte with the value given by <end character number>. If <end character number> is -1, then it waits for <number of bytes> characters (unless the socket is closed). If <end character number> is -2, then it does not block and only returns with the characters immediately available up to <number of bytes> bytes.

<error code>=**SCWRITE**(<socket number>,<write string>)  
Output the string data <write string> over the socket given by <socket number>. Returns zero for no error sending data, or a negative number for an error.

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
		 

<ercode>=**FCLOSE**(<fileno>)  
Closes the file given by <fileno>. The file associated with <fileno> must have been opened with the **FOPEN**() function. Returns an error code if there is a problem, but it always deallocates the file structure if <fileno> points to a valid structure.

<ercode>=**FDEL**(<filenamestring>)  
Deletes the file given by the name <filenamestring>. Returns an error code if there is a problem.

<ercode>=**FDIRCL**(<fileno>)  
Closes the directory given by <fileno>. The directory associated with <fileno> must have been opened with the **FDIROP**() function. Returns an error code if there is a problem, but always deallocates the directory structure if <fileno> points to a valid structure.

<fileno>=**FDIROP**(<directorystring>)  
Opens a directory with the name <directorystring>. Returns an error code if there is a problem (negative), or a positive number with the fileno.

vx$=**FDIRRD$**(<fileno>)  
Gets the next entry from the directory listing given by the open directory indicated by <fileno>. The directory associated with <fileno> must have been opened with the **FDIROP**() function. Returns a blank string if the directory is ended, or if there is a problem, otherwise it returns a string in the following format:

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

<feofflag>=**FEOF**(<fileno>)  
Determines whether the end of file has been reached. The file associated with <fileno> must have been opened with the FOPEN() function. Returns an error code if there is a problem, or zero if the end of the file is not reached, or one if it is reached.

<ercode>=**FMKDIR**(<directorystring>)  
Creates a new directory with the name given by <directorystring>. Returns an error code if there is a problem.

<fileno>=**FOPEN**(<filenamestring>,<flagstring>)  
Opens a file given by the file <filenamestring>. Returns an error code if an error is encountered (negative number), or the fileno of the opened file. All files are automatically closed when a program is ended. The string <flagstring> can contain the following characters indicating how to open the file: "**R**"=_read_, "**W**"=_write_, "**C**"=_create new file_, "**A**"=_always create file_, "**O**"=_always open file._

x$=**FREAD$**(<fileno>,<chars>)  
Read <chars> number of characters out of the file given by <fileno>. The file associated with <fileno> must have been opened with the **FOPEN**() function. Returns a string of the length of the actual number of readable characters from the file, or a blank string if there was an error or no characters could be read. Is handy in conjunction with FEOF() to determine if the end of the file has been reached.

x$=**FREADL$**(<fileno>,<chars>)  
Read <chars> number of characters out of the file given by <fileno>, but stops at the first newline (linefeed '\\n'). The linefeed is included in the string. The **TRIM$** function is handy to remove the linefeed. The file associated with <fileno> must have been opened with the **FOPEN**() function. Returns a string of the length of the actual number of readable characters from the file, or a blank string if there was an error or no characters could be read. Is handy in conjunction with **FEOF**() to determine if the end of the file has been reached.

<ercode>=**FSEEK**(<fileno>,<offset>,<mode>)  
Moves the file pointer to a new location in the file. The file associated with <fileno> must have been opened with the FOPEN() function. Returns an error code if there is a problem, or zero if there is no problem. The mode determines the behavior. <offset>=0 means seek relative to the beginning of the file. <offset>=1 means seek relative to current position. <offset>=2 means seek relative to end of the file, so that FSEEK(<fileno>,0,2) seeks to the end of the file.

x$=**FSTAT$**(<filenamestring>)  
Gets the information about the file given by <filenamestring>. Returns a blank string if the file does not exist, or if there is a problem, otherwise it returns a string in the following format:

	temp          DRHSA 1980/0 /0  0 :0  20000
	Filename      flags create time/date length
	

See **FDIRRD$** for details on the format.

<ercode>=**FSYNC**(<fileno>)  
Writes any unwritten data to the file given by <fileno>, but does not close the file. The file associated with <fileno> must have been opened with the **FOPEN**() function. Returns an error code if there is a problem.

<offset>=**FTELL**(<fileno>)  
Returns the current file pointer corresponding to <fileno>, or a negative error code otherwise. The file associated with <fileno> must have been opened with the **FOPEN**() function.

<ercode>=**FWRITE**(<fileno>,<string>)  
Writes the data in <string> to the file denoted by <fileno>. The file associated with <fileno> must have been opened with the **FOPEN**() function. Returns an error code if there is a problem, or zero if there is no error. Note a newline is NOT automatically added to the <string>, you must add it manually (e.g. <string>+**CHR$**(10)).
