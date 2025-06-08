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
#include "iointr.h"
#include "cpu.h"
#include "lpc210x.h"
#include "main.h"
#include "serial.h"

/* subroutines for I/O */

void setpin(int pinno, unsigned int mode)
{
  int subpin = (pinno % 100) & 0x1F;
  int group = pinno / 100;
  int shft = (2*(subpin & 0x0F));
  unsigned int mask = ~(3 << shft);
  unsigned int modebit = (mode & 0x03) << shft;
  
  switch ((group*2)+((subpin & 0x10) != 0)) {
		case 0: PCB_PINSEL0 = (PCB_PINSEL0 & mask) | modebit; break;
		case 1: PCB_PINSEL1 = (PCB_PINSEL1 & mask) | modebit; break;
		case 2: PCB_PINSEL2 = (PCB_PINSEL2 & mask) | modebit; break;
		case 3: PCB_PINSEL3 = (PCB_PINSEL3 & mask) | modebit; break;
		case 4: PCB_PINSEL4 = (PCB_PINSEL4 & mask) | modebit; break;
		case 5: PCB_PINSEL5 = (PCB_PINSEL5 & mask) | modebit; break;
		case 6: PCB_PINSEL6 = (PCB_PINSEL6 & mask) | modebit; break;
		case 7: PCB_PINSEL7 = (PCB_PINSEL7 & mask) | modebit; break;
		case 8: PCB_PINSEL8 = (PCB_PINSEL8 & mask) | modebit; break;
		case 9: PCB_PINSEL9 = (PCB_PINSEL9 & mask) | modebit; break;
  }
  if ((mode==0)||(mode==4)) {
	mask = ~(1 << subpin);
	modebit = (mode == 4) << subpin;
	switch (group) {
		case 0: GPIO0_IODIR = (GPIO0_IODIR & mask) | modebit; break;
		case 1: GPIO1_IODIR = (GPIO1_IODIR & mask) | modebit; break;
		case 2: FIO2_DIR = (FIO2_DIR & mask) | modebit; break;
		case 3: FIO3_DIR = (FIO3_DIR & mask) | modebit; break;
		case 4: FIO4_DIR = (FIO4_DIR & mask) | modebit; break;
	}
  }
}

void getpinregs(int pinno,
			  volatile unsigned int **setreg, volatile unsigned int **readreg,
			  unsigned int *mask)
{
  int subpin = (pinno % 100) & 0x1F;
  int group = pinno / 100;
  volatile unsigned int *temp;
  if (setreg == NULL)
	setreg = &temp;
  if (readreg == NULL)
	readreg = &temp;
  *mask = ((unsigned int)1) << subpin;
  switch (group) {
	case 1: *setreg = &GPIO1_IOPIN; *readreg = &GPIO1_IOPIN; break;
	case 2: *setreg = &FIO2_PIN; *readreg = &FIO2_PIN; break;
	case 3: *setreg = &FIO3_PIN; *readreg = &FIO3_PIN; break;
	case 4: *setreg = &FIO4_PIN; *readreg = &FIO4_PIN; break;
 	case 0: *setreg = &GPIO0_IOPIN; *readreg = &GPIO0_IOPIN; break;
	default: *setreg = &GPIO0_IOPIN; *readreg = &GPIO0_IOPIN; *mask = 0; break;
  }
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
  U8 b = 0;
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

#define TCR_CNT_EN		0x00000001
#define TCR_RESET		0x00000002
#define TCR_PWM_EN		0x00000008

#define PWMMR0I			1 << 0
#define PWMMR0R			1 << 1
#define PWMMR0S			1 << 2
#define PWMMR1I			1 << 3
#define PWMMR1R			1 << 4
#define PWMMR1S			1 << 5
#define PWMMR2I			1 << 6
#define PWMMR2R			1 << 7
#define PWMMR2S			1 << 8
#define PWMMR3I			1 << 9
#define PWMMR3R			1 << 10
#define PWMMR3S			1 << 11
#define PWMMR4I			1 << 12
#define PWMMR4R			1 << 13
#define PWMMR4S			1 << 14
#define PWMMR5I			1 << 15
#define PWMMR5R			1 << 16
#define PWMMR5S			1 << 17
#define PWMMR6I			1 << 18
#define PWMMR6R			1 << 19
#define PWMMR6S			1 << 20

#define PWMSEL2			1 << 2
#define PWMSEL3			1 << 3
#define PWMSEL4			1 << 4
#define PWMSEL5			1 << 5
#define PWMSEL6			1 << 6
#define PWMENA1			1 << 9
#define PWMENA2			1 << 10
#define PWMENA3			1 << 11
#define PWMENA4			1 << 12
#define PWMENA5			1 << 13
#define PWMENA6			1 << 14

#define LER0_EN			1 << 0
#define LER1_EN			1 << 1
#define LER2_EN			1 << 2
#define LER3_EN			1 << 3
#define LER4_EN			1 << 4
#define LER5_EN			1 << 5
#define LER6_EN			1 << 6

void pwm(int pinno, unsigned int val1, unsigned int val2)
{
  static char inited = 0;
  
  if ((pinno<1)||(pinno>6))
	return;
  if (!inited) {
    int i;
	PWM_TCR = TCR_RESET;		/* Counter Reset */	
    PWM_PR = 0x00;		/* count frequency:Fpclk */
    PWM_MCR = PWMMR0R;	/* reset on PWMMR0, reset TC if PWM0 matches */
	for (i=0;i<=3;i++) *(&PWM_MR0 + i)= val1;
	for (i=4;i<=6;i++) *(&PWM_MR4 + (i-4))= val1;
    PWM_LER = LER0_EN | LER1_EN | LER2_EN | LER3_EN | LER4_EN | LER5_EN | LER6_EN;
	PWM_PCR = PWMENA1 | PWMENA2 | PWMENA3 | PWMENA4 | PWMENA5 | PWMENA6;
    PWM_TCR = TCR_CNT_EN | TCR_PWM_EN;	/* counter enable, PWM enable */
	inited = 1;
  }
  PWM_MR0 = val1;
  *((pinno<4) ? (&PWM_MR1 + (pinno-1)) : (&PWM_MR4 + (pinno-4))) = val2;
  PWM_LER = LER0_EN | (1 << pinno);
}

static char intdriv = 0;

void outdac(int pinno, unsigned int value)
{
  if (!intdriv)
  {  
#if defined LPC2148 || defined LPC2378
	if (pinno == 0)
		DAC_CR = ((value << DAC_CR_VALUESHIFT) & DAC_CR_VALUEMASK);
#endif
  }
}

void inadc_normal_mode(void)
{
	SCB_PCONP |= SCB_PCONP_PCAD0;
	AD0_INTEN = 0;
	AD0_CR = AD_CR_CLKS10 | AD_CR_PDN | ((14 - 1) << AD_CR_CLKDIVSHIFT) | 0xFF | AD_CR_BURST;
#if defined LPC2148
    SCB_PCONP |= SCB_PCONP_PCAD1;
	AD1_INTEN = 0;
	AD1_CR = AD_CR_CLKS10 | AD_CR_PDN | ((14 - 1) << AD_CR_CLKDIVSHIFT) | 0xFF | AD_CR_BURST;
#endif
}

unsigned int inadc(int pinno)
{
   static char inited = 0;
   REG32 *AD_DR;

   if (intdriv)
	   return 0; 
   if (!inited) {
      inited = 1;
	  inadc_normal_mode();
   } 
   if (pinno <= 7) {
   	  AD_DR = (&AD0_DR0)+pinno;
	  while (!(*AD_DR & AD_DR_DONE)); 
      return ((*AD_DR & AD_DR_RESULTMASK) >> AD_DR_RESULTSHIFT);
   }
#if defined LPC2148
   if ((pinno >= 8) && (pinno <= 15)) {
   	  AD_DR = (&AD1_DR0)+(pinno-8);
	  while (!(*AD_DR & AD_DR_DONE)); 
      return ((*AD_DR & AD_DR_RESULTMASK) >> AD_DR_RESULTSHIFT);
   }
#endif
   return 0;
}

int read_adc(unsigned short *buf, int samples)
{
	int readsamp = 0, newpos;
	while (readsamp < samples)
	{
		if (adc_fifo_tail == adc_fifo_head) break;
		buf[readsamp++] = ADC_InBuf[adc_fifo_tail];
		newpos = adc_fifo_tail + 1;
		if (newpos >= ADC_InBufLen)
			newpos = 0;
		adc_fifo_tail = newpos;
	}
	return readsamp;
}

int adc_readlen(void)
{
	int lft = adc_fifo_head - adc_fifo_tail;
	if (lft < 0)
		lft += ADC_InBufLen;
	return lft;
}

int write_dac(unsigned short *buf, int samples)
{
	int writesamp = 0, newpos;
	while (writesamp < samples)
	{
		newpos = dac_fifo_head + 1;
		if (newpos >= DAC_OutBufLen)
			newpos = 0;
		if (newpos == dac_fifo_tail)
			break;
		DAC_OutBuf[dac_fifo_head] = buf[writesamp++];
		dac_fifo_head = newpos;
	}
	return writesamp;
}

int dac_wrtleft(void)
{
	int lft = dac_fifo_head - dac_fifo_tail;
	if (lft < 0)
		lft += DAC_OutBufLen;
	return DAC_OutBufLen - lft + 1;
}

static int countbits(unsigned int n)
{
	int b = 0;
	while (n != 0) {
		b += (n & 0x01);
		n >>= 1;
	}
	return b;
}

void adc_init(int inadcchans, int outdacchans, int clkdiv, int indiv, int outdiv) 
{
  if (intdriv)
	adc_shutdown();
  
  intdriv=1;
  
  adc_fifo_head = 0;
  adc_fifo_tail = 0;
  adc_count_chans = 0;

  dac_fifo_head = 0;
  dac_fifo_tail = 0;

  adc_indiv = indiv;
  dac_outdiv = outdiv;
  
  inadcchans &= 0xFF;
  if (inadcchans == 0)
	inadcchans = 1;
  adc_num_chans = countbits(inadcchans);
  
  outdacchans &= 0x01;
  if (outdacchans == 0)
	outdacchans = 1;
  clkdiv &= 0xFF;
  if (clkdiv < 14)
	clkdiv = 14;
  SCB_PCONP |= SCB_PCONP_PCAD0;
  AD0_CR = AD_CR_CLKS10 | AD_CR_PDN | ((clkdiv - 1) << AD_CR_CLKDIVSHIFT) | inadcchans | AD_CR_BURST;
  AD0_INTEN = AD_INTEN_DONE;
  AD0_GDR;
#if defined LPC2148 || defined LPC2378
  SCB_PCONP |= SCB_PCONP_PUSB;
#endif

  // Setup VIC to fire on timer0 IRQ's
#ifdef LPC2378
  VIC_VectAddr18 = (unsigned int)adcISR;    //address of IRQ
  VIC_VectCntl18 = 0x04;
  VIC_IntEnable |= VIC_IntSelect_AD0; 
#else
  VIC_VectAddr1 = (unsigned int)adcISR;    //address of IRQ
  VIC_VectCntl1 = VIC_Channel_AD0 | VIC_VectCntl_ENABLE;  
                 // Slot 0 assigned to irq 4 (timer 0 )
  VIC_IntEnable |= VIC_IntSelect_AD0; 
#endif
  
} 

void adc_shutdown(void)
{
  if (!intdriv)
		return;
#ifdef LPC2378
  VIC_IntEnClr = VIC_IntEnClr_AD0;
  VIC_VectCntl18 = 0x0F;
  VIC_VectAddr18 = 0;    //address of IRQ
#else
  VIC_VectCntl1 = 0;
  VIC_VectAddr1 = 0;
#endif
  
  inadc_normal_mode();
  intdriv=0;
}

void adc_trig(int mode, int chan, int level)
{
   trig_chan = chan;
   trig_level = level;
   trig_mode = mode;
   trig_pause = (mode != TRIG_MODE_NONE);
   if (mode == TRIG_MODE_ABOVE) adc_last_samp = 0x1000000;
   if (mode == TRIG_MODE_BELOW) adc_last_samp = 0;
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
