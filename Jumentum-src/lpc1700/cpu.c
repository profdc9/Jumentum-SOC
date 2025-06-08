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

#include "libdriver/lpc17xx_wdt.h"
#include "libdriver/lpc17xx_rtc.h"
#ifdef LPCUSB
#include "libdriver/usb/usb_serial.h"
#endif
#ifdef LPCVIDEO
#include "video.h"
#include "keyboard.h"
#endif
 
#include <stdio.h>
#include <stdlib.h>
 
#include "all.h"
#include "cpu.h"
#include "lib.h"

/**********************************************************
                      Initialize
**********************************************************/

static void rtcWrite (struct cpu_tm *newTime);
static void rtcRead (struct cpu_tm *theTime);
static void rtcInitClockCalendar (void);

void abort(void)
{
}

void swi_watchdog(void)
{
   WDT_Feed();
}

inline void swi_irq_disable(void)
{
   __disable_irq();
}

inline void swi_irq_enable(void)
{
    __enable_irq();
}

void cpuSetupHardware(void)
{
	SysTick_Config(SystemCoreClock / 100);
#ifdef LPCUSB
	usb_serial_init();
#endif
#ifdef LPCVIDEO
    init_video();
	init_keyboard();
#endif
}

static char watchdoginit = 0;

void force_reboot(void)
{
  WDT_Init(WDT_CLKSRC_IRC, WDT_MODE_RESET);
  WDT_Start(10000);
  for (;;);
}

void watchdog_enable(void)
{
  WDT_Init(WDT_CLKSRC_IRC, WDT_MODE_RESET);
  WDT_Start(WD_TIMEOUT);
}

void watchdog_debug(void)
{
}

int watchdog_thrown(void)
{
  return WDT_ReadTimeOutFlag ();
}

#ifdef RTC
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
	RTC_TIME_Type pFullTime;
  
	RTC_GetFullTime(LPC_RTC, &pFullTime);
	theTime->tm_sec = pFullTime.SEC;
	theTime->tm_min = pFullTime.MIN;
	theTime->tm_yday = pFullTime.DOY;
	theTime->tm_mday = pFullTime.DOM;
	theTime->tm_hour = pFullTime.HOUR;
	theTime->tm_min = pFullTime.MIN;
	theTime->tm_sec = pFullTime.SEC;
	theTime->tm_mon = pFullTime.MONTH;
	theTime->tm_year = pFullTime.YEAR;
	theTime->tm_isdst = 0;
}
 
static const char rtcformat[]="0000-00-00 00:00:00";

void rtcReadString(char *timestring)
{
  struct cpu_tm tm;
  rtcRead(&tm);

  strcpy(timestring,rtcformat);  
  writedigs(timestring, tm.tm_year, 4);
  writedigs(&timestring[5], tm.tm_mon, 2);
  writedigs(&timestring[8], tm.tm_mday, 2);
  writedigs(&timestring[11], tm.tm_hour, 2);
  writedigs(&timestring[14], tm.tm_min, 2);
  writedigs(&timestring[17], tm.tm_sec, 2);
}

static void rtcWrite (struct cpu_tm *newTime)
{
	RTC_TIME_Type pFullTime;
   
	pFullTime.DOM = newTime->tm_mday;
	pFullTime.DOW = newTime->tm_wday;
	pFullTime.DOY = newTime->tm_yday;
	pFullTime.HOUR = newTime->tm_hour;
	pFullTime.MIN = newTime->tm_min;
	pFullTime.SEC = newTime->tm_sec;
	pFullTime.MONTH = newTime->tm_mon;
	pFullTime.YEAR = newTime->tm_year;
	RTC_SetFullTime(LPC_RTC, &pFullTime);
}

int rtcWriteString(const char *timestring)
{
  int iserror;
  struct cpu_tm tm;

  if (strlen(timestring)<(RTC_STR_LEN-1))
	 return -1;
  iserror = readdigs(&timestring[0],4,&tm.tm_year);
  iserror |= readdigs(&timestring[5],2,&tm.tm_mon);
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
void rtcInit (void)
{
  int nonsense = 0;
  struct cpu_tm tm;

  RTC_Init (LPC_RTC);
  RTC_ResetClockTickCounter(LPC_RTC);
  RTC_Cmd(LPC_RTC, ENABLE);

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
	tm.tm_mon   = 1;
	tm.tm_mday  = 1;
	tm.tm_wday  = 2;
	tm.tm_year  = 2008;
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
