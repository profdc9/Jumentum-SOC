Jumentum-SOC 

# Introduction to the Jumentum-SOC programming environment

  
by Daniel L. Marks (profdc9 at gmail.com)

- - -

Jumentum-SOC is a programming environment for LPC1700/LPC2000-based microcontrollers. It turns your LPC1700/LPC2000 into an autonomous controller that is remotely programmable through Ethernet using a Basic-derived language. Jumentum-SOC can be accessed and programmed through a web browser, a TELNET text-based session, and the serial port. Jumentum-SOC Basic programs can be downloaded into the microcontroller and programmed into the flash so that the programs are automatically run on power up of the microcontroller. The programs can be started, stopped, paused, and the program variables inspected at any time through the web page.

Jumentum-SOC environment is written in C and a little ARM assembly language, and is provided as a project for the [CodeSourcery G++](http://www.codesourcery.com/sgpp/lite/arm) GNU toolchain development environment. An LPC1700/LPC2000-based processor is currently required for the Jumentum-SOC environment. The LPC1768 and LPC2378 is supported with its built-in ethernet, or the LPC2106, LPC2119, and LPC2148 processors are supported connected to Microchip's ENC28J60 SPI-based ethernet. For details on connecting an ENC28J60 to the LPC21xx microcontroller, see the setups detailed in the "ENC28J60-wiring.txt" file included in this distribution.

Jumentum-SOC is provided under a zlib-derived open source license that allows commercial or non-commercial use. Jumentum-SOC is intended to lower both the cost and complexity of microcontroller embedding by using the capabilities of the new generation of 32-bit self-contained microcontrollers from NXP, Atmel, and Microchip. Jumentum-SOC is excellent to help teach how to integrate microcontrollers into projects as well as a means of automating simple projects.

Features of Jumentum-SOC include

*   Jumentum is open source hardware and software, under a liberal zlib-like commercial and non-commercial use license. The source code uses the free GNU GCC tools, which also lowers the barriers to modifying the code base and sharing improvements to Jumentum-SOC.
*   LPC1700/LPC2000 (NXP ARM microcontroller) environment. Requires a minimum of 128k FLASH part and 16k of chip RAM. LPC2119, LPC2106, LPC2148, LPC2368, LPC2378. and LPC1768 for example can be used. This environment will use the built-in ethernet of the LPC17xx and LPC23xx series microcontrollers. The controller uses very low cost parts (LPC2000 series 6 to 15 USD, ENC28J60 3 to 5 USD).
*   For the LPC1768 and LPC2378, the environment uses the built-in ethernet MAC. Otherwise Jumentum can use the SPI-based Microchip ENC28J60 controller for Ethernet access. Jumentum-SOC can also be compiled to use SLIP over either of the microcontroller built-in serial ports.
*   Programmable in the Jumentum-SOC program language. Unlike most microcontroller BASIC languages, Jumentum Basic includes 8-bit clean strings and multidimensional arrays, optional floating point arithmetic, as well as bit manipulation and PEEK/POKE for the daring. Special instructions are included for hardware access.
*   No software is required on the programming computer except a web browser, TELNET client, and/or a serial port terminal program. Jumentum-SOC is self-contained on the microcontroller, and no proprietary software is required to use it, which makes it suitable for Linux and Mac as well as Microsoft Windows. The on-board BASIC program can be easily reprogrammed and accessed in the field through a web page.
*   BASIC programs can output HTML and other Mime-types through the web server and accept form data so that complex interactions are possible. For example, source code for a web-based oscilloscope and an MP3 streaming server are provided as well as many other examples.
*   BASIC programs can interact with the user over the serial port and/or a TELNET connection. The TELNET connection and serial port mirror each other for maximum convenience. VT100 screen control commands and keyboard interactive input are available, and these do not interfere with the web operation. PUTTY and Tera Term Pro are recommended as Windows applications for serial port and TELNET access.
*   A built-in VT100 full-screen editor is available by Telnet or the serial port to modify programs on the microcontroller. The full screen editor can be used to configure Jumentum-SOC or edit the resident program in Flash.
*   A web server is built in that enables remotely programming Jumentum-SOC through the web page, as well as remote diagnostics. Both the TELNET server and Web server can be password protected. Programs can be started, paused, or stopped at any time through the web browser. The variables of a running program can be examined from the web browser.
*   The hardware watchdog can be enabled to automatically reset the unit if there is a crash. Jumentum-SOC automatically feeds the watchdog.
*   A configuration page is available that enables the IP address, netmask, and gateway to be programmed without recompiling the software. Jumentum's password can be specified in the configuration page. DHCP and the watchdog can be enabled in the configuration page.
*   Jumentum-SOC supports an attached FAT-formatted SD/MMC card and enables files and directories to be accessed from Basic. This is perfect for data logging purposes, and for ensuring data can remain saved through a power cycle. A streaming MP3 server example application demonstrates the FAT FS capability.
*   The hardware capabilities of Jumentum-SOC include digital general purpose input/output, analog-to-digital input, digital-to-analog output, pulse width modulation control, and interrupt-driver ADC/DAC synchronized analog input/output. The analog input can be level triggered like an oscilloscope.
*   Jumentum-SOC is designed for high availability in remote monitoring and control situations.

- - -

A brief introduction to Jumentum-SOC:

To program your microcontroller with the Jumentum environment, you must obtain a "hex" file with the Jumentum firmware for your microcontroller. The firmware can be compiled with many options, e.g. for a LPC1768, LPC2106, LPC2119, LPC2148, or LPC2378 with or without Ethernet, with or without SLIP, floating point or integer arithmetic, etc.

To program the firmware, Flashmagic, the Phillips flash loader, or a program such as lpc21isp may be used. A windows executable for lpc21isp is included with the package, and it is also available for Linux or Macintosh. The most recent version can be retrieved from the newsgroup at [lpc21isp](https://sourceforge.net/projects/lpc21isp/). Some examples of using lpc21isp to program your micrcontroller:

#  For LPC2148 with 12 MHz crystal, attached to com1
	lpc21isp -control basic.hex com1 38400 12000
#  For LPC2119 with 19.6 MHz crystal, attached to com9
	lpc21isp -control basic.hex com9 38400 19600
#  For LPC2106 with 14.7 MHz crystal, attached to com1
	lpc21isp -control basic.hex com1 38400 14700
#  For LPC2378 with 12 MHz crystal, attached to com6
	lpc21isp -control basic.hex com6 19200 12000
#  For LPC1768 with 12 MHz crystal, attached to com1
	lpc21isp -control basic.hex com6 38400 12000

The frequency of the crystal on the microcontroller must be known and specified on the lpc21isp command line in kHz.

Once you have loaded the firmware, reset the microcontroller. Without any network attached, the default setting of the is to automatically assume the IP address 10.0.0.4 on the 10.0.0.0 subnet. If DHCP is enabled, it will attempt for a minute to acquire its address from the local network, and if that fails then assumes the default address. The default network port for the web is 80 and the default telnet port is 23.

Once the microcontroller is reset, any resident program will be started ten seconds after reset.

The easiest way to access the Jumentum-SOC system is through the web browser. Upon accessing the web page, a menu of options is presented in the browser.

"Front Page" - displays the front page you are seeing.  
"Net Connections" - displays the active connections.  
"Program" - Allows the currently installed program to be edited in the browser.  
"Variables" - Allows the variables of a currently running program to be shown.  
"Run" - Runs the program currently flashed into the microcontroller.  
"Stop" - Stops the running program  
"Pause" - Pauses the currently running program. It may be resumed with "Run."  
"Access Basic" - Access the web page produced by the currently running Basic program.  

In addition to these options, by clicking on the "Configure" link from the Front Page, the options such as IP address, etc. can be changed.

To enter a new program, use the "Program" option. Type the program into the text window and click "Upload Program." If "Program Already Running" is displayed when the upload is attempted, a program currently is running and the program can not be changed. Open another browser window and use the "Stop" option to stop the currently running program, and then attempt the upload again.

To actually see the web page produced by the running program, use the "Access Basic" option. If the program accepts web connections, it will display its web page. The URL of Web pages intended to be processed by the Basic language contains "/bas" after the web site address in the URL. This is analogous to the "/cgi-bin" of most web servers. If the URL contains "/basr" instead of "/bas", it will also be processed by the script, but the script must produce the HTTP headers also.

Instead of accessing the controller through the web, it can be accessed by a TELNET interactive terminal. The same session can also be acceessed from the first serial port with settings of 38400 bits per second, 8 data bits, 1 stop bit, no parity. Once connected using either method, press space bar to reprint the menu of options, which are:

"P" - program the controller  
"S" - reset the controller  
"E" - edit the program in the flash  
"L" - list the program in the flash  
"C" - edit the built-in configuration  
"R" - run the currently flashed program  

A file can be uploaded over the serial port rather than programmed through the web interface. To do this, press "P" and then send the program text using XON/XOFF software flow control. Tera Term Pro for Windows supports this option well.

The current program text in the flash can be edited using the "E" option. A built-in VT100 text editor will be invoked. The CTRL-W key will allow changes to be saved. Unfortunately, only rather small programs may be edited this way as the program text must fit into RAM, so this method is not recommended for large programs. If the editor will not invoke because there is too little unfragmented memory available, then the reset option can be used to clear the memory.

The uploaded program can be run with the "R" option. The programs interactive input and output (e.g. PRINT, INPUT, INKEY, LOCATE commands), will be presented through the terminal. At this time, there is no key to abort the program; this must be achieved using the "Stop" option through the web interface.

Currently, the software does not assume there is any hardware to enable it to know when a MMC/SD card is inserted or removed. All files should be closed if possible before removing a MMC/SD card, and if a new card is inserted, the unit will need to be reset.

- - -

Jumentum-SOC and any other included software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

- - -

Dan Marks, last updated _December 22, 2010_
