/* Jumentum-SOC

  Copyright (C) 2007 by Daniel Marks

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
  
  Daniel L. Marks profdc9@gmail.com

*/


#include <stdio.h>
#include <stdlib.h>

#include "all.h"
#include "basic.h"
#include "io.h"
#include "i2c.h"
#include "cpu.h"
#include "main.h"
#include "serial.h"

/* subroutines for I/O */

void setpin(int pinno, unsigned int mode)
{
}

static unsigned long temp = 0;

void getpinregs(int pinno,
			  volatile unsigned int **setreg, volatile unsigned int **readreg,
			  unsigned int *mask)
{
	*setreg = &temp;
	*readreg = &temp;
	*mask = 0;
}

int serbng(int pinno, unsigned char *data, unsigned int len, 
		    unsigned int dly)
{
  int bit;
  volatile int j;
  unsigned char c;
  volatile unsigned int *setreg, *readreg;
  unsigned int mask;
  
  getpinregs(pinno, &setreg, &readreg, &mask);
  while (len>0) {
	len--;
	c = *data++;
	*setreg = *readreg & ~mask;
	for (j=dly;j>0;j--);
	for (bit=8;bit>0;bit--) {
		if (c & 0x01) {
			*setreg = *readreg | mask;
		} else {
			*setreg = *readreg & ~mask;
		}
		for (j=dly;j>0;j--);
		c>>=1;
	}
	*setreg = *readreg | mask;
	for (j=dly;j>0;j--);
  }
  return 0;
}


unsigned char serspi(int mosipin, int misopin, 
					 int clkpin, unsigned char c, unsigned int dly)
{
  unsigned char b = 0;
  int i = 8;
  volatile int j;

  volatile unsigned int *misosetreg, *misoreadreg;
  unsigned int misomask;
  volatile unsigned int *mosisetreg, *mosireadreg;
  unsigned int mosimask;
  volatile unsigned int *clksetreg, *clkreadreg;
  unsigned int clkmask;
  
  getpinregs(misopin, &misosetreg, &misoreadreg, &misomask);
  getpinregs(mosipin, &mosisetreg, &mosireadreg, &mosimask);
  getpinregs(clkpin, &clksetreg, &clkreadreg, &clkmask);
  
  do {
	 if (c & 0x80) *mosisetreg = *mosireadreg | mosimask;
	    else *mosisetreg = *mosireadreg & ~mosimask;
	 c <<= 1;
	 *clksetreg = *clkreadreg & ~clkmask;
	 for (j=dly;j>0;j--);
	 *clksetreg = *clkreadreg | clkmask;
	 for (j=dly;j>0;j--);
	 b = (b<<1) | ((*misoreadreg & misomask) != 0);
	 *clksetreg = *clkreadreg & ~clkmask;
	 for (j=dly;j>0;j--);
  } while ((--i)>0);
  *mosisetreg = *mosireadreg & ~mosimask;
  return b;
}

int seri2c(int sdawritepin, int sdareadpin, int sclpin, int addr, int end_trans,
		   int numoutbytes, unsigned char *outbytes,
		   int numinbytes, unsigned char *inbytes)
{
	i2cstate i2c;
	volatile i2c_software_register *temp;
	
	i2c.delay = 50;
	i2c.timeout = 100000;
	
    getpinregs(sdawritepin, &i2c.sdawriteregwrite, &i2c.sdawriteregread, &i2c.sdawritebit);
    getpinregs(sdareadpin, &temp, &i2c.sdareadregread, &i2c.sdareadbit);
    getpinregs(sclpin, &i2c.sclwriteregwrite, &i2c.sclwriteregread, &i2c.sclwritebit);
	
	if ((numinbytes == 0) || (end_trans < 0)) {
		i2c_send_bytes(&i2c, addr, numoutbytes, outbytes, end_trans != 0);
	} else if (numoutbytes == 0) {
		i2c_recv_bytes(&i2c, addr, numinbytes, inbytes);
	} else {
		i2c_send_recv_bytes(&i2c, addr, numoutbytes, outbytes, numinbytes, inbytes);
	}
	return 0;
}

unsigned int ind(int pinno)
{
  volatile unsigned int *setreg, *readreg;
  unsigned int mask;
  
  getpinregs(pinno, &setreg, &readreg, &mask);
  return (*readreg & mask) != 0;
}

void outd(int pinno, unsigned int state)
{
  volatile unsigned int *setreg, *readreg;
  unsigned int mask;
  
  getpinregs(pinno, &setreg, &readreg, &mask);
  if (state)
	*setreg = *readreg | mask;
  else
	*setreg = *readreg & ~mask;
}

#ifdef LPC2378
#define CPUCLKMHZ 48
#else
#define CPUCLKMHZ 60
#endif
#define FREQSCAL ((1250000*CPUCLKMHZ)/60)
/* you probably have to fiddle with FREQSCAL to get the frequency conversion
    right since its just a software loop, the speed of which will depend on exactly
    where in the flash/RAM the code is executing from */

void tone(int pinno, unsigned int val1, unsigned int val2)
{
  unsigned int i;
  volatile unsigned int j;
  volatile unsigned int *setreg, *readreg;
  unsigned int mask;
  
  getpinregs(pinno, &setreg, &readreg, &mask);
  val2 = FREQSCAL / val2;
  val1 = (val1*1000) / val2;
  for (i=0;i<val1;i++)
  {
  	*setreg = *readreg | mask;
	 for (j=val2;j>0;j--);
	*setreg = *readreg & ~mask;
	 for (j=val2;j>0;j--);
  }
}

void pwm(int pinno, unsigned int val1, unsigned int val2)
{
}

static char intdriv = 0;

void outdac(int pinno, unsigned int value)
{
}

void inadc_normal_mode(void)
{
}

unsigned int inadc(int pinno)
{
}

int read_adc(unsigned short *buf, int samples)
{
}

int adc_readlen(void)
{
}

int write_dac(unsigned short *buf, int samples)
{
}

int dac_wrtleft(void)
{
}

static int countbits(unsigned int n)
{
}

void adc_init(int inadcchans, int outdacchans, int clkdiv, int indiv, int outdiv) 
{
} 

void adc_shutdown(void)
{
}

void adc_trig(int mode, int chan, int level)
{
}

void serial_init(int port, int baud, int databits, int stopbits, int parity)
{
	init_serial_c(port,baud,databits,stopbits,parity);
}

int serial_read(int port, int len, int charend, int timeout, char *data)
{
	int n = 0, c;
	while ((timeout > 0) && (n < len)) {
		c = getkey_serial_c(port);
		if (c >= 0) {
			if (data == NULL) {
				n++;
			} else {
				data[n++] = c;
			}
			if (c == charend)
				break;
		} else {
			timeout--;
		}
		if (BASICIDLE()) break;
	}
	return n;
}

int serial_write(int port, char *data, int len)
{
	int i;
	for (i=0;i<len;i++) putc_serial_c(port,data[i]);
	return 0;
}
