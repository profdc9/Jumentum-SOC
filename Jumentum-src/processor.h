
#ifndef _PROCESSOR_H
#define _PROCESSOR_H

#undef WATCHDOG
#define RTC

struct cpu_tm
{
  int	tm_sec;
  int	tm_min;
  int	tm_hour;
  int	tm_mday;
  int	tm_mon;
  int	tm_year;
  int	tm_wday;
  int	tm_yday;
  int	tm_isdst;
};

#define RTC_STR_LEN 20

void rtcInit (void);
int rtcWriteString(const char *timestring);
void rtcReadString(char *timestring);
void watchdog_enable(void);
int watchdog_thrown(void);
void watchdog_debug(void);

void cpuSetupHardware(void);
void force_reboot(void);

#ifdef WATCHDOG
#define WATCHDOG_ENABLE() watchdog_enable()
#define WATCHDOG_UPDATE() swi_watchdog()
#define WATCHDOG_THROWN() watchdog_thrown()
#define WATCHDOG_DEBUG() watchdog_debug()
#else
#define WATCHDOG_ENABLE()
#define WATCHDOG_UPDATE() 
#define WATCHDOG_THROWN() 0
#define WATCHDOG_DEBUG()
#endif

#endif  /* PROCESSOR_H */
