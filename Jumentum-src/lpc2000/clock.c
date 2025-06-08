#include "lpc210x.h"
#include "clock.h"

void timerISR (void)   __attribute__ ((naked));

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
  ClockTimer=0;
  
  // Setup VIC to fire on timer0 IRQ's
#ifdef LPC2378
  VIC_VectAddr4 = (unsigned int)&timerISR;    //address of IRQ
  VIC_VectCntl4 = 0x02;
  VIC_IntEnable |= VIC_IntSelect_Timer0;
#else
  VIC_VectAddr0 = (unsigned int)&timerISR;    //address of IRQ
  VIC_VectCntl0 = VIC_Channel_Timer0 | VIC_VectCntl_ENABLE;  
                 // Slot 0 assigned to irq 4 (timer 0 )
  VIC_IntEnable |= VIC_IntSelect_Timer0;
#endif
  
  // Set up timer to fire periodic IRQ
  T0_PR = 29491200/120;      // Debug setup so that T0TC increment every 16 msecond
  T0_MCR = 3;                // Generate an interrupt on Match 0 and clear timer 0
  T0_MR0 = 1;                // 
  T0_TCR = 1;                // start timer
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
  return ClockTimer;
}

#define ISR_ENTRY() asm volatile(" sub   lr, lr,#4\n" \
                                 " stmfd sp!,{r0-r12,lr}\n" \
                                 " mrs   r1, spsr\n" \
                                 " stmfd sp!,{r1}")
#define ISR_EXIT()  asm volatile(" ldmfd sp!,{r1}\n" \
                                 " msr   spsr_c,r1\n" \
                                 " ldmfd sp!,{r0-r12,pc}^")



/***********************************************************************/
/** \brief ISR for Timer.
 *
 * Description: Increments the system clock     
 * \author Iain Derrington
 * \date WIP
 */
/**********************************************************************/
#define UART0_BASE_ADDR		0xE000C000
#define U0THR          (*(volatile unsigned long *)(UART0_BASE_ADDR + 0x00))
void timerISR(void)
{
  ISR_ENTRY();
  //U0THR = (ClockTimer & 0x07)+'0';
  ClockTimer++;

  T0_IR = 1;   //clear irq
#ifdef LPC2378
  VIC_NewVectAddr = 0;
#else
  VIC_VectAddr = 0;
#endif
  ISR_EXIT();
}
