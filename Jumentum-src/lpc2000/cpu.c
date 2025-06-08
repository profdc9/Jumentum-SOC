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

/**********************************************************
                  Header files
 **********************************************************/
 
#include <stdio.h>
#include <stdlib.h>
 
#include "all.h"
#include "lpc210x.h"
#include "cpu.h"
#include "lib.h"
#include "main.h"
#include "vic.h"

register char *stack_ptr asm ("sp");

#ifdef PARANOIDPINSEL

static unsigned int gpio0_iodir_storage =0;
static unsigned int gpio1_iodir_storage =0;
static unsigned int pcb_pinsel0_storage =0;
static unsigned int pcb_pinsel1_storage =0;
#ifdef PARANOID2
static unsigned int pcb_pinsel2_storage =0;
#endif

unsigned long get_fattime (void);

void set_gpiodir0(unsigned int val)
{
	GPIO0_IODIR = val;
	gpio0_iodir_storage = val;
}

void set_gpiodir1(unsigned int val)
{
	GPIO1_IODIR = val;
	gpio1_iodir_storage = val;
}

void set_pinsel0(unsigned int val)
{
   PCB_PINSEL0 = val;
   pcb_pinsel0_storage = val;
}

void set_pinsel1(unsigned int val)
{
   PCB_PINSEL1 = val;
   pcb_pinsel1_storage = val;
}

#ifdef PARANOID2
void set_pinsel2(unsigned int val)
{
   PCB_PINSEL2 = val;
   pcb_pinsel2_storage = val;
}
#endif

static void reset_known_pinsel_values(void)
{
	GPIO0_IODIR = gpio0_iodir_storage;
	GPIO1_IODIR = gpio1_iodir_storage;
	PCB_PINSEL0 = pcb_pinsel0_storage;
	PCB_PINSEL1 = pcb_pinsel1_storage;
#ifdef PARANOID2
	PCB_PINSEL2 = pcb_pinsel2_storage;
#endif
}

void check_pinsel(void)
{
#ifdef FORCE_PINSEL_UPDATE
	reset_known_pinsel_values();
#else
	if ((GPIO0_IODIR != gpio0_iodir_storage) ||
		(GPIO1_IODIR != gpio1_iodir_storage) ||
#ifdef PARANOID2
		(PCB_PINSEL2 != pcb_pinsel2_storage) ||
#endif
		(PCB_PINSEL0 != pcb_pinsel0_storage) ||
		(PCB_PINSEL1 != pcb_pinsel1_storage))
	{
#ifdef REPORT_PINSEL_PROBLEM
		unsigned int oldval_gpio0_iodir = GPIO0_IODIR;
		unsigned int oldval_gpio1_iodir = GPIO1_IODIR;
		unsigned int oldval_pcb_pinsel0 = PCB_PINSEL0;
		unsigned int oldval_pcb_pinsel1 = PCB_PINSEL1;
#ifdef PARANOID2
		unsigned int oldval_pcb_pinsel2 = PCB_PINSEL2;
#endif
#endif
		
		reset_known_pinsel_values();

#ifdef REPORT_PINSEL_PROBLEM
		deboutstrhex("OLDVAL_GPIO0_IODIR=",oldval_gpio0_iodir);
		deboutstrhex("GPIO0_IODIR_STORAGE=",gpio0_iodir_storage);
		deboutstrhex("OLDVAL_GPIO1_IODIR=",oldval_gpio1_iodir);
		deboutstrhex("GPIO1_IODIR_STORAGE=",gpio1_iodir_storage);
		deboutstrhex("OLDVAL_PCB_PINSEL0=",oldval_pcb_pinsel0);
		deboutstrhex("PCB_PINSEL0_STORAGE=",pcb_pinsel0_storage);
		deboutstrhex("OLDVAL_PCB_PINSEL1=",oldval_pcb_pinsel1);
		deboutstrhex("PCB_PINSEL1_STORAGE=",pcb_pinsel1_storage);
#ifdef PARANOID2
		deboutstrhex("OLDVAL_PCB_PINSEL2=",oldval_pcb_pinsel2);
		deboutstrhex("PCB_PINSEL2_STORAGE=",pcb_pinsel2_storage);
#endif	
#endif
	}
#endif
}
#endif

void oops(const char *type)
{
    register int i;
	for (;;) {
		deboutstrhex("\n\nType:",0);
		deboutstrhex(type,0);
		deboutstrhex("SP=", (unsigned int)stack_ptr);
		deboutstrhex("heap_end=", (unsigned int)heap_end);
		for (i=0;i<256;i+=4) {
			if ( ((*((unsigned int *)(stack_ptr+i)) & 0xFF000000) == 0x00000000) &&
				 ((*((unsigned int *)(stack_ptr+i)) & 0x00FFFF00) != 0x00000000)) {
				deboutstrhex("stack=",i);
				deboutstrhex("calling_func=", *((unsigned int *)(stack_ptr+i)));
			}
			if ((*((unsigned int *)(stack_ptr+i)) & 0xFF000000) == 0x40000000) {
				deboutstrhex("stack=",i);
				deboutstrhex("varptr=", *((unsigned int *)(stack_ptr+i)));
			}
		}
		for (i=0;i<10000000;i++) GPIO0_IOSET = 0;
	}
}

inline void swi_watchdog (void)
{
  asm volatile(" swi 0x07 ");
}

inline U32 swi_fiq_disable(void)
{
  unsigned long retval;
  asm volatile(" swi 0x02 ");
  asm volatile (" mov  %0, r0" : "=r" (retval) : /* no inputs */  );
  return retval;
}

inline U32 swi_fiq_enable(void)
{
  unsigned long retval;
  asm volatile(" swi 0x03 ");
  asm volatile (" mov  %0, r0" : "=r" (retval) : /* no inputs */  );
  return retval;
}

inline U32 swi_irq_disable(void)
{
  unsigned long retval;
  asm volatile(" swi 0x00 ");
  asm volatile (" mov  %0, r0" : "=r" (retval) : /* no inputs */  );
  return retval;
}

inline U32 swi_irq_enable(void)
{
  unsigned long retval;
  asm volatile(" swi 0x01 ");
  asm volatile (" mov  %0, r0" : "=r" (retval) : /* no inputs */  );
  return retval;
}

inline U32 swi_get_cpsr(void)
{
  unsigned long retval;
  asm volatile(" swi 0x04 ");
  asm volatile (" mov  %0, r0" : "=r" (retval) : /* no inputs */  );
  return retval;
}

inline void swi_int_restore(U32 c)
{
  asm volatile(" swi 0x05 ");
}

inline void swi_fiq_restore(U32 c)
{
  asm volatile(" swi 0x06 ");
}

/**********************************************************
                      Initialize
**********************************************************/

#define PLOCK 0x400

static void rtcWrite (struct cpu_tm *newTime);
static void rtcRead (struct cpu_tm *theTime);
static void rtcInitClockCalendar (void);

static void pushpll(void)
{
	SCB_PLLFEED = 0xAA;
	SCB_PLLFEED = 0x55;
}

#if defined LPC2378

#define SCB_BASE_ADDR	0xE01FC000
#define SCB_CCLKCFG        (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x104))
#define SCB_CLKSRCSEL      (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x10C))
#define SCB_CLKSRCSEL      (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x10C))
#define SCB_PCLKSEL_0      (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x1A8))
#define SCB_PCLKSEL_1      (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x1AC))
#define SCB_PLLSTAT_ISLOCKED (1 << 26)
#define SCB_PLLSTAT_ISCONNECTED (1 << 25)
#define SCB_USBCLKCFG      (*(volatile unsigned long *)(SCB_BASE_ADDR + 0x108))

#define SCB_PLLCFG_MVALUE   (12-1)
#define SCB_PLLCFG_NVALUE   0
#define SCB_CCLKDIV_DIVIDER 5

#define SCB_SCS_ENABLE_OSC 0x20
#define SCB_SCS_OSCENABLED 0x40
#define SCB_CLKSRCSEL_MAINOSC 0x01

#define SCB_PLL_DISABLE 0
#define SCB_PLL_ENABLE 1
#define SCB_PLL_CONNECT 2
		
void cpuSetupHardware(void)
{
    int i;
	
	VIC_IntEnable = 0;
    VIC_VectAddr = 0;
    VIC_IntEnClr = 0xffffffff;
    VIC_IntSelect = 0;

    /* set all the vector and vector control register to 0 */
    for (i=0;i<32;i++)
    {
	   *(&VIC_VectCntl0 + i) = 0x0F;
	   *(&VIC_VectAddr0 + i) = 0;
	}
	
    SCB_PLLCON = SCB_PLL_DISABLE;				/* Disable PLL, disconnected */
	pushpll();
    
	SCB_SCS |= SCB_SCS_ENABLE_OSC;			/* Enable main OSC */
    while( !(SCB_SCS & SCB_SCS_OSCENABLED) );	/* Wait until main OSC is usable */
    SCB_CLKSRCSEL = SCB_CLKSRCSEL_MAINOSC;	/* select main OSC, 12MHz, as the PLL clock source */

    SCB_PLLCFG = SCB_PLLCFG_MVALUE | (SCB_PLLCFG_NVALUE << 16);
	pushpll();
      
    SCB_PLLCON = SCB_PLL_ENABLE;				/* Enable PLL, disconnected */
	pushpll();

    SCB_CCLKCFG = SCB_CCLKDIV_DIVIDER;	/* Set clock divider */

    while ( ((SCB_PLLSTAT & SCB_PLLSTAT_ISLOCKED) == 0) );	/* Check lock bit status */
    
    SCB_PLLCON = SCB_PLL_ENABLE|SCB_PLL_CONNECT;	/* enable and connect */
	pushpll();
	while ( ((SCB_PLLSTAT & SCB_PLLSTAT_ISCONNECTED) == 0) );	/* Check connect bit status */

    SCB_PCLKSEL_0 = 0x55555555;	/* PCLK is 1/2 CCLK */
    SCB_PCLKSEL_1 = 0x55555555;	 

    MAM_CR = MAM_CR_DISABLE;
	MAM_TIM = MAM_TIM_3;
    MAM_CR = MAM_CR_PARTIAL;  /* should be MAM_CR_FULL */

	SCB_PCONP |= ((1<<8)|(1<<10)|(1<<12)|(1<<19)|(1<<21)|(1<<30));
#ifdef LPCUSB
	usb_serial_init();
#endif
	return;
}
#endif

#if defined LPC2148 || defined LPC2106 || defined LPC2119
void cpuSetupHardware(void)  
{
	VIC_DefVectAddr = (unsigned int)Spurious_IRQ_Routine;
	VIC_IntEnable = 0;
    //
    //  Setup the PLL to multiply the 12Mhz XTAL input by 5, divide by 1
    //
#ifdef LPC2148
    SCB_PLLCFG = (SCB_PLLCFG_MUL5 | SCB_PLLCFG_DIV1);
#endif
#ifdef LPC2119
    SCB_PLLCFG = (SCB_PLLCFG_MUL3 | SCB_PLLCFG_DIV1);
#endif
#ifdef LPC2106
    SCB_PLLCFG = (SCB_PLLCFG_MUL4 | SCB_PLLCFG_DIV1);
#endif
  	pushpll();
  
    //
    //  Activate the PLL by turning it on then feeding the correct sequence of bytes
    //
	SCB_PLLCON  = SCB_PLLCON_PLLE;
	pushpll();
  
    //
    //  Wait for the PLL to lock...
    //
    while (!(SCB_PLLSTAT & SCB_PLLSTAT_PLOCK));
  
    //
    //  ...before connecting it using the feed sequence again
    //
    SCB_PLLCON  = SCB_PLLCON_PLLC | SCB_PLLCON_PLLE;
	pushpll();
  
    //
    //  Setup and turn on the MAM.  Three cycle access is used due to the fast
    //  PLL used.  It is possible faster overall performance could be obtained by
    //  tuning the MAM and PLL settings.
    //
	MAM_TIM = MAM_TIM_3;
    MAM_CR = MAM_CR_FULL;
  
    //
    //  Setup the peripheral bus to be the same as the PLL output (30Mhz)
    //
    SCB_VPBDIV = SCB_VPBDIV_100;

#ifdef LPCUSB
	usb_serial_init();
#endif
}
#endif

void force_reboot(void)
{
#ifdef LPC2378
    WD_CLKSEL = 1;
#endif
	SCB_RSIR = SCB_RSIR_MASK;
	WD_MOD = WD_MOD_WDEN | WD_MOD_RESET;
	WD_TC = 120000;
	WD_FEED = WD_FEED_FEED1;
	WD_FEED = WD_FEED_FEED2;
	for (;;);
}


#ifdef WATCHDOG
void watchdog_enable(void)
{
  SCB_RSIR = SCB_RSIR_MASK;
#ifdef LPC2378
  WD_CLKSEL = 1;
#endif
  WD_MOD |= WD_MOD_WDEN | WD_MOD_RESET;
  WD_TC = WD_TIMEOUT;
  WATCHDOG_UPDATE();
}

void watchdog_debug(void)
{
   static unsigned int count = 0;
   if (count++ >= 100000) {
		deboutstrint("WD_TC=",WD_TC);
		deboutstrint("WD_TV=",WD_TV);
		count=0;
	}
}

int watchdog_thrown(void)
{
  return (WD_MOD & WD_MOD_TOF);
}
#endif

#ifdef RTC
void rtcInit (void)
{
  SCB_PCONP |= SCB_PCONP_PCRTC;
  RTC_CCR = 0;
#if defined LPC2148 || defined LPC2378
  RTC_CCR |= RTC_CCR_CLKSRC;
#endif
  RTC_AMR = 0;
  RTC_CIIR = 0;
  RTC_ILR = 0;
  RTC_CCR |= RTC_CCR_CLKEN;
  rtcInitClockCalendar ();
}

//
//  Place RTC on 32kHz xtal and disconnect power.
//
static void rtcSleep (void)
{
#if defined LPC2148 || defined LPC2378
  RTC_CCR = (RTC_CCR_CLKEN | RTC_CCR_CLKSRC);
#else
  RTC_CCR = (RTC_CCR_CLKEN);
#endif
  SCB_PCONP &= ~SCB_PCONP_PCRTC;
}

//
//  Prepare clock for interactive use.
//
static void rtcWake (void)
{
  int i;
#if defined LPC2148 || defined LPC2378
  RTC_CCR = (RTC_CCR_CLKEN | RTC_CCR_CLKSRC);
#else
  RTC_CCR = (RTC_CCR_CLKEN);
#endif
  SCB_PCONP |= SCB_PCONP_PCRTC;
  for (i=0;i<10000;i++);
}

unsigned long get_fattime ()
{
  unsigned long tmr;
  struct cpu_tm tm;

  rtcRead(&tm);
  
  tmr = 0
    | ((tm.tm_year - 80) << 25)
    | ((tm.tm_mon + 1)   << 21)
    | (tm.tm_mday        << 16)
    | (tm.tm_hour        << 11)
    | (tm.tm_min         << 5)
    | (tm.tm_sec         >> 1);

  return tmr;
}

static int readdigs(const char *str, int dig, int *num)
{ 
  *num = 0;
  int iserror = 1;
  while ((dig-- > 0) && (*str != 0) && (*str != ' ')) {
	 *num=((*num<<3)+(*num<<1))+((*str++)-'0');
	 iserror = 0;
  }
  return iserror;
}

static void writedigs(char *str, int num, int dig)
{
  str += dig;
  while (dig-- > 0) {
	*(--str) =  (num % 10) + '0';
	num /= 10;
  }
}

//
//  Read clock registers and return tm structure.
//
static void rtcRead (struct cpu_tm *theTime)
{
  unsigned long irqstate,fiqstate;
  unsigned int ticks32Khz;
  
  rtcWake ();
  irqstate =  swi_irq_disable();
  fiqstate =  swi_fiq_disable();

  do
  {
	theTime->tm_sec   = RTC_SEC;
	theTime->tm_min   = RTC_MIN;
	theTime->tm_hour  = RTC_HOUR;
	theTime->tm_mday  = RTC_DOM;
	theTime->tm_mon   = RTC_MONTH - 1;
	theTime->tm_year  = RTC_YEAR - 1900;
	theTime->tm_wday  = RTC_DOW;
	theTime->tm_yday  = RTC_DOY - 1;
	theTime->tm_isdst = 0;
    do
      ticks32Khz = (RTC_CTC & 0xfffe);
    while (ticks32Khz != (RTC_CTC & 0xfffe));
  }
  while (theTime->tm_sec != RTC_SEC);

  swi_fiq_restore(fiqstate);
  swi_int_restore(irqstate);
  rtcSleep ();
}

static const char rtcformat[]="0000-00-00 00:00:00";

void rtcReadString(char *timestring)
{
  struct cpu_tm tm;
  rtcRead(&tm);

  strcpy(timestring,rtcformat);  
  writedigs(timestring, tm.tm_year + 1900, 4);
  writedigs(&timestring[5], tm.tm_mon + 1, 2);
  writedigs(&timestring[8], tm.tm_mday, 2);
  writedigs(&timestring[11], tm.tm_hour, 2);
  writedigs(&timestring[14], tm.tm_min, 2);
  writedigs(&timestring[17], tm.tm_sec, 2);
}

static void rtcWrite (struct cpu_tm *newTime)
{
  unsigned int irqstate,fiqstate;
  rtcWake ();

  irqstate =  swi_irq_disable();
  fiqstate =  swi_fiq_disable();

  RTC_CCR &= ~RTC_CCR_CLKEN;
  RTC_CCR |=  RTC_CCR_CTCRST;

  RTC_SEC   = newTime->tm_sec;
  RTC_MIN   = newTime->tm_min;
  RTC_HOUR  = newTime->tm_hour;
  RTC_DOM   = newTime->tm_mday;
  RTC_MONTH = newTime->tm_mon + 1;
  RTC_YEAR  = newTime->tm_year + 1900;
  RTC_DOW   = newTime->tm_wday;
  RTC_DOY   = newTime->tm_yday + 1;

  RTC_CCR &= ~RTC_CCR_CTCRST;
  RTC_CCR |=  RTC_CCR_CLKEN;

  swi_fiq_restore(fiqstate);
  swi_int_restore(irqstate);

  rtcSleep ();
}

int rtcWriteString(const char *timestring)
{
  int iserror;
  struct cpu_tm tm;

  if (strlen(timestring)<(RTC_STR_LEN-1))
	 return -1;
  iserror = readdigs(&timestring[0],4,&tm.tm_year);
  tm.tm_year -= 1900;
  iserror |= readdigs(&timestring[5],2,&tm.tm_mon);
  tm.tm_mon -= 1;
  iserror |= readdigs(&timestring[8],2,&tm.tm_mday);
  iserror |= readdigs(&timestring[11],2,&tm.tm_hour);
  iserror |= readdigs(&timestring[14],2,&tm.tm_min);
  iserror |= readdigs(&timestring[17],2,&tm.tm_sec);

  tm.tm_yday  = 0;
  tm.tm_isdst = 0;
  tm.tm_wday  = 2;

  if (!iserror) rtcWrite(&tm);
  return -iserror;
}

//
//  Start clock so that the sytsem may use it.
//
static void rtcInitClockCalendar (void)
{
  int nonsense = 0;
  struct cpu_tm tm;

  rtcRead(&tm);
  
  if ((tm.tm_sec < 0) || (tm.tm_sec > 59))  nonsense = 1;
  if ((tm.tm_min < 0) || (tm.tm_min > 59))  nonsense = 1;
  if ((tm.tm_hour < 0) || (tm.tm_hour > 23))    nonsense = 1;
  if ((tm.tm_mday < 1) || (tm.tm_mday > 31))      nonsense = 1;
  if ((tm.tm_mon < 0) || (tm.tm_mon > 11))    nonsense = 1;
  if ((tm.tm_year < 1980) || (tm.tm_year > 2050))   nonsense = 1;
  if ((tm.tm_wday < 0) || (tm.tm_wday > 6))       nonsense = 1;
  if ((tm.tm_yday < 0) || (tm.tm_yday > 366))     nonsense = 1;

  if (nonsense) {
	tm.tm_sec   = 0;
	tm.tm_min   = 0;
    tm.tm_hour  = 0;
	tm.tm_yday  = 0;
	tm.tm_isdst = 0;
	tm.tm_mon   = 0;
	tm.tm_mday  = 1;
	tm.tm_wday  = 2;
	tm.tm_year  = (2008 - 1900);
	rtcWrite (&tm);
  }
}
#else
void rtcInit (void)
{
}

unsigned long get_fattime ()
{
	return 0;
}
#endif
