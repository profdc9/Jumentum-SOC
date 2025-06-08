Jumentum-SOC 

# Jumentum-SOC for the NXP ARM7TDMI and Cortex-M3 Microcontrollers

![](jumentum-mini-logo.jpg)

by Daniel Marks (profdc9 at gmail.com)

### Supports the LPC1768/LPC2368 microcontrollers.

### Also emulates a USB serial port for easier interfacing.

### NTSC/PAL composite video output from the LPC1768 with a PS/2 keyboard for standalone operation.

Jumentum-SOC is a programming environment for LPC2000-based microcontrollers. It turns your LPC1700/LPC2000 into an autonomous controller that is remotely programmable through Ethernet using a Basic-derived language. Jumentum-SOC can be accessed and programmed through a web browser, a TELNET text-based session, and the serial port. Jumentum-SOC Basic programs can be downloaded into the microcontroller and programmed into the flash so that the programs are automatically run on power up of the microcontroller. The programs can be started, stopped, paused, and the program variables inspected at any time through the web page. Jumentum-SOC environment is written mostly in C, and is provided as a project for the [CodeSourcery G++](http://www.codesourcery.com/sgpp/lite/arm) GNU toolchain development environment. An LPC1700/LPC2000-based processor is currently required for the Jumentum-SOC environment. The LPC1768 and LPC2378 is supported with built-in ethernet, or the LPC2106, LPC2119, and LPC2148 processors are supported connected to Microchip's ENC28J60 SPI-based ethernet. For details on connecting an ENC28J60 to the LPC21xx microcontroller, see the setups detailed in the "ENC28J60-wiring.txt" file included in this distribution. Jumentum-SOC is provided under a zlib-derived open source license that allows commercial or non-commercial use. Jumentum-SOC is intended to lower both the cost and complexity of microcontroller embedding by using the capabilities of the new generation of 32-bit self-contained microcontrollers from NXP, Atmel, and Microchip. Jumentum-SOC is excellent to help teach how to integrate microcontrollers into projects as well as a means of automating simple projects.

- - -

[An introduction to the Jumentum System](Introduction.md)  
[Commands for the Basic language](basic-commands.md)  
[Configuration information](configuration.md)  
[How to use the USB support](USB.md)  
[Connecting a television and a keyboard to the mbed or LPC1768](lpc1768-video.md)  
[Wiring instruction to connect the ENC28J60 to the ARM microcontroller](ENC28J60-wiring.md)  
[Getting started with the mbed](mbed.md)  
[Screenshots of Jumentum-SOC in action](screenshots.md)  

- - -

Some code examples in Basic:  
[Simplest example of a web page](simplest-web-example.bas)  
[Example of using forms](webtest-forms.bas)  
[Oscilloscope trace program](oscope-ad.bas)  
[Serving web pages from MMC/SD card](serve.bas)  
[An FTP server to and from a MMC/SD card](ftpserv.bas)  
[Streaming MP3 Server from MMC/SD card](streaming-mp3-2.bas)  
[Control of a HD44780 LCD](hd44780.bas)  
[Serial terminal](dumbterminal.bas)  
[Program to control](oppo-switch-hm31.bas) [OPPO Digital HM-31](http://www.oppodigital.com/hm31/) HDMI switch through a web page/telnet and the serial port  

- - -

Daniel Marks _profdc9 at gmail.com_ May 7, 2011
