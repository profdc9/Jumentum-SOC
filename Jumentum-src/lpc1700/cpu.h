
#ifndef _cpu_h
#define _cpu_h

#include "../processor.h"

inline void swi_watchdog (void);
inline void swi_irq_disable(void);
inline void swi_irq_enable(void);

#define WD_TIMEOUT 10000000UL

#endif  /* _cpu_h */
