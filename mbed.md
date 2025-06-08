mbed guide for Jumentum-SOC 

# mbed guide for Jumentum-SOC

This is a guide on how to use the Jumentum system-on-chip (SOC) system on your mbed. It is very simple to load onto your mbed and use to write software to inferface your projects to the web and internet. The Jumentum-SOC project is hosted on Sourceforge and can be accessed at [jumentum.sourceforge.net](http://jumentum.sourceforge.net/).

To get started, go to the Sourceforge [download page](https://sourceforge.net/projects/jumentum/files/Jumentum-SOC/) and download the most recent version (0.99 as of December 26, 2010). It will be a file name similarly to "Jumentum-SOC-0.99.zip". Download this file and unzip it.

This file is a project for Codesourcery G++ and the LPC CMSIS libraries. It uses an off-line compiler and does not compile with the mbed libraries or the mbed compiler. However, it is an example if you wish to try off-line compiling. In the "roms" subdirectory locate the file "basic-mbed.bin", which is the file to flash your mbed with. There are two versions of the program, "basic-mbed.bin" and "basic-mbed-float.bin". The difference is that the "float" version uses double-precision floating point numbers as numerical variables rather than 32 bit integers.

Copy this file into the flash area of your mbed (e.g. drag it into your mbed folder) and reset your mbed. **NOTICE: the flash memory is erased each time you drag a new binary to your mbed, including any Jumentum-SOC BASIC programs stored in the flash, so back them up first!** If you have the mbed serial driver installed, you can open up a terminal program (e.g. [Teraterm for Windows](http://ttssh2.sourceforge.jp/index.html.en)) and see the initialization screen:

![](initializejumentum.jpg)

The first thing to do is to configure your Jumentum system. Hit the "C" key and it brings up a text editor to edit the internal configuration file. You will a text editor appear in the terminal:

![](jumentumtexteditor.jpg)

From here you type a list of directives to Jumentum to tell it how to initialize. The full list of these directives is [here](http://jumentum.sourceforge.net/configuration.html). The example given here turns of DHCP search for the IP address, and initializes the IP address to a fixed address. It also turns off SD card support (in case you don't have an SD card slot connected to your mbed). When you are done, hit control-W and "Y" to save.

Now we will enter a program and run it. Hit "E" to enter the text editor. We will enter a simple blinky program: ![](blinkyprog.jpg)

When you are finished entering it, hit control-"W" and "Y" again to save it. Then hit "R" to run the program. You should see the following when it executes:

![](blinkyprogrun.jpg)

At the same time, LED #1 should be blinking. For your convenience, the conversion table between pins on the mbed socket and the LPC microprocessor pins are in a table at the end of this document.

You can list the program currently on the mbed using the "L" command. This output can be captured by a terminal program to save your program. The "P" command can be used to program the mbed. Send the text of the program using XON/XOFF software flow control to the mbed through the mbed USB serial port. (NOTE: this is experimental, the mbed's serial port emulation does not seem to support XON/XOFF flow control). There are several program banks in the LPC1768 flash that can be used to simultaneously store many BASIC programs if desired. Press "B" to see the list of these banks and select a different bank.

Another way to use Jumentum-SOC is through its web interface. If you go to the web address (either found through DHCP or its default address 10.0.0.4) you can see the home page:

![](jumentumstartwebpage.jpg)

Click the "Stop" word at the top (to make sure any program that is running is stopped), and then click "Program" and you will see the web-based text editor for internal programs:

![](jumentumwebpagetexteditor.jpg)

You can edit the program text here and click "Upload Program" to change it from your web browser. You may have to click "Stop" first in another window if a program has started while editing before you click the program button. Once uploaded, you can click "Run" to run the program. If the program produces a web page, that can be viewed by clicking the "Access BASIC" icon, which passes the HTTP request to the BASIC program.

If you wish to use the text interface through the Internet, you can use TELNET (e.g. [PuTTY](http://www.chiark.greenend.org.uk/~sgtatham/putty/)). Simply TELNET to your mbed and you will see the same interface as available at the console:

![](jumentummbedtelnet.jpg)

The simplest way to get the mbed working on the network is to use these [instructions](http://mbed.org/users/rolf/notebook/ethernet/) to wire up a Magjack or similar Ethernet jack with magnetics to your mbed. Then plug your mbed into your router that does DHCP to automatically obtain an address. You then need to look at the mbed serial port output or your router's device table to figure out the address the mbed acquired.

There are many examples of programming Jumentum-SOC in the "bas" directory included with the Jumentum-SOC distribution, including web forms, mp3 streaming from an SD card, and an oscilloscope application that digitizes a waveform through the AD channels and outputs an oscilloscope trace image to a web page.

Jumentum-SOC is a work-in-progress, please be patient.

- - -

Conversion table between mbed pins and LPC microcontroller pins for use in SETPIN/OUTD/IND commands

| mbed Pin \| | Jumentum / LPC microcontroller pin |
| --- | --- |
| LED-1 | 118 |
| LED-2 | 120 |
| LED-3 | 121 |
| LED-4 | 123 |
| DIP5 | 9   |
| DIP6 | 8   |
| DIP7 | 7   |
| DIP8 | 6   |
| DIP9 | 0   |
| DIP10 | 1   |
| DIP11 | 18  |
| DIP12 | 17  |
| DIP13 | 15  |
| DIP14 | 16  |
| DIP15 | 23 (Analog In 0 fn #1) |
| DIP16 | 24 (Analog In 1 fn #1) |
| DIP17 | 25 (Analog In 2 fn #1) |
| DIP18 | 26 (Analog In 3 fn #1 / Analog Out fn #2) |
| DIP19 | 130 (Analog In 4 fn #2) |
| DIP20 | 131 (Analog In 5 fn #2) |
| DIP21 | 205 (Pulse Width Mod 6 fn #1) |
| DIP22 | 204 (Pulse Width Mod 5 fn #1) |
| DIP23 | 203 (Pulse Width Mod 4 fn #1) |
| DIP24 | 202 (Pulse Width Mod 3 fn #1) |
| DIP25 | 201 (Pulse Width Mod 2 fn #1) |
| DIP26 | 200 (Pulse Width Mod 1 fn #1) |
| DIP27 | 11  |
| DIP28 | 10  |
| DIP29 | 5   |
| DIP30 | 4   |

- - -

Dan Marks, last updated _December 26, 2010_
