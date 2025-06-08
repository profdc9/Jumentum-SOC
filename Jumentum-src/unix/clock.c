#include "clock.h"
#include <sys/time.h>

clock_time_t ClockTimer;

/***********************************************************************/
/** \brief initialise the clock.
 *
 * Description: initialise timer0 on the LPC2106. Timer shall fire every
 *              100mS. Assume Peripheral Clock = 29.4912 MHz 
 *              
 *              
 * \author Iain Derrington
 * \date 0.1 01/07/2007
 */
/**********************************************************************/
void clock_init(void)
{
} 

/***********************************************************************/
/** \brief Returns counter value.
 *
 * \author Iain Derrington
 * \return clock_time_t
 * \date 
 */
/**********************************************************************/
clock_time_t clock_time(void)
{
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

#define CLICKS 60

  return tv.tv_sec * CLICKS + tv.tv_usec / (1000000 / CLICKS);
}
