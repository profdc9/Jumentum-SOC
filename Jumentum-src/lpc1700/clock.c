#include "libdriver/LPC17xx.h"
#include "clock.h"

clock_time_t ClockTimer;

/*----------------------------------------------------------------------------
  SysTick_Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
  ClockTimer++;                        /* increment counter necessary in Delay() */
}

clock_time_t clock_time(void)
{
  return ClockTimer;
}

void clock_init(void)
{
//  SysTick_Config(SystemCoreClock / 100);
}
