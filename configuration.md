Configuration format for Jumentum BASIC for ARM 

# Configuration format for Jumentum BASIC for ARM

**PASSWORD**\=password  
Configures a password for the web server and the telnet server.

**CONSOLE**\=serial port number  
Configures the serial port that should be the console, 0 or 1 (e.g. CONSOLE=1)

**BAUDRATE**\=bps  
Configures the initialization bits per second of the console serial port, the default is 38400.

**BANK**\=bank number  
Program bank number to use upon startup. The default is bank zero. The number of banks depends on the part number used.

**STARTUPTIME**\=seconds  
Number of seconds before the BASIC program is automatically run. The default is 10 seconds.

**WATCHDOG**\=y or n  
Enables the watchdog feature that resets the processor if the watchdog is not fed for 10 seconds. Under normal circumstances, the watchdog is fed automatically by the system and will only reset if it crashes or locks up.

**NOFATFS**\=y or n  
Configures whether the controller attempts to initalize the SD card/FAT filesystem. NOFATFS=y indicates do not initialize or use the SD card, otherwise Jumentum will try if FATFS is compiled in. The default is NOFATFS=n.

**NONET**\=y or n  
Configures whether the controller attempts to initalize the network. NONET=y indicates do not initialize or use the network, otherwise Jumentum will try if the network is compiled in. The default is NONET=n.

**DHCP**\=y or n  
Configures whether the controller attempts to use DHCP to configure the IP address of the unit. If the unit is not able to acquire an address after a couple minutes, it defaults to the values given by HOSTADDR/NETMASK/ROUTER. You must set DHCP=n if you do not want Jumentum to use DHCP.

**MACADDR**\=&lt;macaddr&gt; e.g. 00:22:33:44:55:66  
Configures the MAC address of the unit. This is useful if there will be more than one Jumentum on your network, so that they do not share the same MAC address. Note that the numerals in the MAC address are hexadecimal.

**HOSTADDR**\=&lt;ipaddr&gt; e.g. 10.0.0.4  
Configures the IP address of the unit. If DHCP is enabled, the unit first attempts to use DHCP to acquire an address, otherwise it uses this value. If HOSTADDR is not specified, the default is 10.0.0.4.

**NETMASK**\=&lt;netmask&gt; e.g. 255.255.255.0  
Configures the netmask of the unit. If DHCP is enabled, the unit first uses DHCP to acquire the netmask, otherwise this value is used. If NETMASK is not specified, the default is 255.0.0.0.

**ROUTER**\=&lt;router&gt; e.g. 10.0.0.1  
Configures the default router (also called gateway) for the unit to use. First attempts to acquire by DHCP if DHCP is enabled. If ROUTER is not specified, the default is 10.0.0.1

**SMTP**\=&lt;smtp server address&gt;  
Configures the address of the SMTP server to send mail to.

**HOSTNAME**\=&lt;hostname&gt;  
Configures the host name that appears in outgoing E-mails.

**TELNETPORT**\=&lt;port #&gt;  
Configures the port number to use for the telnet server. Defaults to 23.

**HTTPPORT**\=&lt;port #&gt;  
Configures the port number to use for the web server. Defaults to 80.

Other data can be placed into the configuration file, and can be read with the CONF$ function from Basic, so that one can avoid modifying the Basic program itself if one wishes to modify its behavior.
