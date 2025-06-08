
#ifndef _cpu_h
#define _cpu_h
#include "lpc210x.h"
#include "vic.h"
#include "usb/usb_serial.h"

#include "../processor.h"

#undef PARANOIDPINSEL
#undef REPORT_PINSEL_PROBLEM
#undef FORCE_PINSEL_UPDATE

inline void swi_watchdog (void);
inline U32 swi_fiq_disable(void);
inline U32 swi_fiq_enable(void);
inline U32 swi_irq_disable(void);
inline U32 swi_irq_enable(void);
inline U32 swi_get_cpsr(void);
inline void swi_fiq_restore(U32);
inline void swi_int_restore(U32);

void oops(const char *type);

#ifdef PARANOIDPINSEL
void set_gpiodir0(unsigned int val);
void set_gpiodir1(unsigned int val);
void set_pinsel0(unsigned int val);
void set_pinsel1(unsigned int val);
void set_pinsel2(unsigned int val);
void check_pinsel(void);
#define SET_PINSEL0(x) set_pinsel0((x))
#define SET_PINSEL1(x) set_pinsel1((x))
#define SET_PINSEL2(x) set_pinsel2((x))
#define SET_GPIODIR0(x) set_gpiodir0((x))
#define SET_GPIODIR1(x) set_gpiodir1((x))
#define CHECK_PINSEL() check_pinsel()
#else
#define SET_PINSEL0(x) PCB_PINSEL0=(x)
#define SET_PINSEL1(x) PCB_PINSEL1=(x)
#define SET_PINSEL2(x) PCB_PINSEL2=(x)
#define SET_GPIODIR0(x) GPIO0_IODIR=(x)
#define SET_GPIODIR1(x) GPIO1_IODIR=(x)
#define CHECK_PINSEL()
#endif

#define WD_TIMEOUT 200000000UL

#endif  /* _cpu_h */
