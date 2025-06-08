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

#include "libdriver/lpc17xx_pinsel.h"
#include "libdriver/lpc17xx_pwm.h"
#include "libdriver/lpc17xx_gpio.h"
#include "libdriver/lpc17xx_adc.h"
#include "libdriver/lpc17xx_dac.h"
#include "libdriver/lpc17xx_uart.h"

#include <stdio.h>
#include <stdlib.h>

#include "all.h"
#include "basic.h"
#include "io.h"
#include "i2c.h"
#include "cpu.h"
#include "basicidle.h"
#include "serial.h"

/* subroutines for I/O */

void setpin(int pinno, unsigned int mode)
{
  PINSEL_CFG_Type PinCfg;
  
  PinCfg.Funcnum = mode & 0x03;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;
  PinCfg.Pinnum = (pinno % 100) & 0x1F;
  PinCfg.Portnum = (pinno / 100);
  PINSEL_ConfigPin(&PinCfg);
  if (PinCfg.Funcnum == 0)
	  GPIO_SetDir(PinCfg.Portnum,((unsigned long)1)<<PinCfg.Pinnum,(mode !=0));
}

void blinky(void)
{
  static int pinno = 200;
  unsigned int i;
  volatile unsigned int j;
  volatile unsigned int *setreg, *readreg;
  unsigned int mask;
  
   setpin(pinno,4);
  getpinregs(pinno, &setreg, &readreg, &mask);
  for (;;)
  {
  	*setreg = *readreg | mask;
	 for (j=2000000;j>0;j--);
	*setreg = *readreg & ~mask;
	 for (j=2000000;j>0;j--);
  }
}

void getpinregs(int pinno,
			  volatile unsigned int **setreg, volatile unsigned int **readreg,
			  unsigned int *mask)
{
  int subpin = (pinno % 100) & 0x1F;
  int group = pinno / 100;
  LPC_GPIO_TypeDef *pGPIO;  
  
    *mask = ((unsigned int)1) << subpin;
	switch (group) {
	case 0:
		pGPIO = LPC_GPIO0;
		break;
	case 1:
		pGPIO = LPC_GPIO1;
		break;
	case 2:
		pGPIO = LPC_GPIO2;
		break;
	case 3:
		pGPIO = LPC_GPIO3;
		break;
	case 4:
		pGPIO = LPC_GPIO4;
		break;
	default:
		pGPIO = LPC_GPIO0;
		*mask = 0;
		break;
	}

  if (setreg != NULL)
	*setreg = &pGPIO->FIOPIN;
  if (readreg != NULL)
	*readreg = &pGPIO->FIOPIN;
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

#define CPUCLKMHZ 100

#define FREQSCAL ((2000000*CPUCLKMHZ)/60)
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
  val1 = (val1*4000) / val2;
 
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
  static char inited = 0;
  int chan;
  PWM_TIMERCFG_Type PWMCfgDat;
  PWM_MATCHCFG_Type PWMMatchCfgDat;
  
  if ((pinno<1)||(pinno>6))
	return;
  if (!inited) {
  	PWMCfgDat.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;
	PWMCfgDat.PrescaleValue = 1;
	PWM_Init(LPC_PWM1, PWM_MODE_TIMER, (void *) &PWMCfgDat);

	PWM_MatchUpdate(LPC_PWM1, 0, val1, PWM_MATCH_UPDATE_NOW);
	/* PWM Timer/Counter will be reset when channel 0 matching
	 * no interrupt when match
	 * no stop when match */
	PWMMatchCfgDat.IntOnMatch = DISABLE;
	PWMMatchCfgDat.MatchChannel = 0;
	PWMMatchCfgDat.ResetOnMatch = ENABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);

	for (chan = 1; chan < 7; chan++)
	{
		/* Set up match value */
		if (chan > 1)
			PWM_ChannelConfig(LPC_PWM1, chan, PWM_CHANNEL_SINGLE_EDGE);
		PWM_MatchUpdate(LPC_PWM1, chan, val1, PWM_MATCH_UPDATE_NOW);
		/* Configure match option */
		PWMMatchCfgDat.IntOnMatch = DISABLE;
		PWMMatchCfgDat.MatchChannel = chan;
		PWMMatchCfgDat.ResetOnMatch = DISABLE;
		PWMMatchCfgDat.StopOnMatch = DISABLE;
		PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);
		/* Enable PWM Channel Output */
		//PWM_ChannelCmd(LPC_PWM1, temp, ENABLE);
		/* Increase match value by 10 */
	}
	/* Reset and Start counter */
	PWM_ResetCounter(LPC_PWM1);
	PWM_CounterCmd(LPC_PWM1, ENABLE);
	/* Start PWM now */
	PWM_Cmd(LPC_PWM1, ENABLE);
	inited = 1;
  }
  PWM_MatchUpdate(LPC_PWM1, 0, val1, PWM_MATCH_UPDATE_NOW);
  PWM_MatchUpdate(LPC_PWM1, pinno, val2, PWM_MATCH_UPDATE_NOW);
  PWM_ChannelCmd(LPC_PWM1, pinno, ENABLE);
}

static char intdriv = 0;

static char dacinited = 0;

void outdac(int pinno, unsigned int value)
{
  if (!intdriv)
  {  
	 if (!dacinited) {
		DAC_Init(LPC_DAC);
		dacinited = 1;
	 }
	 DAC_UpdateValue (LPC_DAC,value);
  }
}

void inadc_normal_mode(void)
{
	//int temp;
	ADC_Init(LPC_ADC, 200000);
	ADC_IntConfig(LPC_ADC,ADC_ADINTEN0|ADC_ADINTEN1|ADC_ADINTEN2|ADC_ADINTEN3|ADC_ADINTEN4|ADC_ADINTEN5|ADC_ADINTEN6|ADC_ADINTEN7,DISABLE);
	//for (temp=0;temp<8;temp++)
	//	ADC_ChannelCmd(LPC_ADC,temp,ENABLE);
}

unsigned int inadc(int pinno)
{
   static char inited = 0;
   unsigned int value;

   if (intdriv)
	   return 0; 
   if (!inited) {
      inited = 1;
	  inadc_normal_mode();
   } 
   ADC_ChannelCmd(LPC_ADC,pinno,ENABLE);
   ADC_StartCmd(LPC_ADC,ADC_START_NOW);
   while (!(ADC_ChannelGetStatus(LPC_ADC,pinno,ADC_DATA_DONE)));
   value = ADC_ChannelGetData(LPC_ADC,pinno);
   ADC_ChannelCmd(LPC_ADC,pinno,DISABLE);
   return value;
}

int adc_fifo_head = 0;
int adc_fifo_tail = 0;
int adc_indiv = 1;
int adc_indiv_ctr = 0;
int adc_num_chans = 1;
int adc_count_chans = 0;
unsigned int adc_last_samp = 0;

int dac_fifo_head = 0;
int dac_fifo_tail = 0;
int dac_outdiv = 1;
int dac_outdiv_ctr = 0;
	
unsigned int trig_mode = 0;
unsigned int trig_pause = 0;
unsigned int trig_chan = 0;
unsigned int trig_level = 0;

#define ADCBUFLOC 0x20080200

#define ADC_InBufLen 0x400
#define DAC_OutBufLen 0x400

typedef struct _adc_buf
{
	unsigned short e_ADC_InBuf[ADC_InBufLen];
	unsigned short e_DAC_OutBuf[DAC_OutBufLen];
} adc_buf;

#define ADCBUFPTR ((adc_buf *)(ADCBUFLOC))

//adc_buf adcbuf;
//#define ADCBUFPTR (&adcbuf)

#define ADC_InBuf (ADCBUFPTR->e_ADC_InBuf)
#define DAC_OutBuf (ADCBUFPTR->e_DAC_OutBuf)

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

void ADC_IRQHandler(void)
{
  unsigned int newpos;

  LPC_ADC->ADGDR;
  
  if (trig_pause) {
    if ((ADC_GDR_CH(LPC_ADC->ADGDR)) == trig_chan) {
		newpos = ADC_GDR_RESULT(LPC_ADC->ADGDR);
		if (trig_mode == TRIG_MODE_ABOVE) {
			if ((newpos >= trig_level) && (adc_last_samp < trig_level)) {
				trig_pause = 0;
				goto adcISRcap;
			}
		} else if (trig_mode == TRIG_MODE_BELOW) {
			if ((newpos <= trig_level) && (adc_last_samp > trig_level)) {
				trig_pause = 0;
				goto adcISRcap;
			}
		}
		adc_last_samp = newpos;
	}
    goto endadcISR;
  }
  
adcISRcap:
  if (adc_indiv_ctr == 0) {
	newpos = adc_fifo_head + 1;
	if (newpos >= ADC_InBufLen)
		newpos = 0;
	if (newpos != adc_fifo_tail) {
		ADC_InBuf[adc_fifo_head] = ((LPC_ADC->ADGDR & 0xFFF0) >> 4) |
			((LPC_ADC->ADGDR & 0x07000000) >> 12);
		adc_fifo_head = newpos;
	}
  }
  if ((++adc_count_chans) >= adc_num_chans) {
	adc_count_chans = 0;
	adc_indiv_ctr++;
	if (adc_indiv_ctr >= adc_indiv)
		adc_indiv_ctr = 0;
  }
  
  if ((++dac_outdiv_ctr) >= dac_outdiv) {
	  dac_outdiv_ctr = 0;
	  if (dac_fifo_tail != dac_fifo_head) {
		  LPC_DAC->DACR = (LPC_DAC->DACR & ~0x3FF) | (DAC_OutBuf[dac_fifo_tail] & 0x3FF);
		  newpos = dac_fifo_tail + 1;
		  if (newpos >= DAC_OutBufLen)
			 newpos = 0;
		  dac_fifo_tail = newpos;
	  }
  }
  endadcISR:
  return;
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

  ADC_Init(LPC_ADC, 200000);
  ADC_IntConfig(LPC_ADC,ADC_ADGINTEN,ENABLE);
  ADC_BurstCmd(LPC_ADC, ENABLE);
  LPC_ADC->ADCR = (LPC_ADC->ADCR & ~0xFF) | inadcchans;
  LPC_ADC->ADCR = (LPC_ADC->ADCR & ~0xFF00) | (((unsigned int)clkdiv) << 8);
  NVIC_SetPriority(ADC_IRQn, ((0x01<<3)|0x01));
  ADC_StartCmd(LPC_ADC,ADC_START_NOW);
  NVIC_EnableIRQ(ADC_IRQn);
  LPC_ADC->ADGDR;
} 

void adc_shutdown(void)
{
  if (!intdriv)
		return;
 
  ADC_IntConfig(LPC_ADC,ADC_ADGINTEN,DISABLE);
  NVIC_DisableIRQ(ADC_IRQn);
  ADC_DeInit(LPC_ADC);

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
