Jumentum-SOC USB Serial Support 

# Jumentum-SOC USB Serial Support

  
by Daniel L. Marks (profdc9 at gmail.com)

- - -

Jumentum-SOC as of version 0.99.1 is able to emulate a USB Communication Device Class device (USB Serial Port). This obviates the need for a USB to serial converter, and allows for faster transfers of data to and from the host PC. Also, most development boards natively power and communicate to the microcontroller through USB, also making USB communication more convenient.

Linux and MAC OS X have native support for USB CDC serial ports. In theory, you should just be able to plug the board into those and go. For example, under Linux it should appear as a device such as /dev/ttyACM1. However, under some Linux distributions (e.g. under Ubuntu) it thinks the device is a USB modem and attempts to initialize it as such. This will send "AT commands" to your Jumentum board which may confuse it. This feature should be disabled to use Jumentum as a non-modem serial device under Linux.

For Windows, to install you must select the correct INF file for the driver needed. There are two INF files, one for Windows XP, and another for Windows 7 and Vista, each in their respective directories under the "usbdrivers" directory. For Vista and Windows 7, the usbser.sys driver needed is built-in and should automatically be recognized. For Windows XP, you need to search the Windows CAB files on the XP installation CD and extract the usbser.sys driver (it is likely in sp2.cab file for example). For XP, copy the usbser.sys file into the directory with the inf file. After you have flashed your device with the firmware, reset it and/or plug in the USB cable to power it. Choose install from the correct directory for your operating system and the .inf file should be recognized. Alternately, you can right click on the .inf file and choose "Install" before plugging in your device.

If the network is not connected, sometimes after replugging in your USB device you may need to wait for 30 seconds for the ethernet initialization to cease attempts to initialize the network interface. To prevent this delay, put "NONET=y" in the configuration file.

- - -

Dan Marks, last updated _May 7, 2011_
