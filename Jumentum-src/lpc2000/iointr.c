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

#include "lpc210x.h"
#include "io.h"
#include "iointr.h"

#define ISR_ENTRY() asm volatile(" sub   lr, lr,#4\n" \
                                 " stmfd sp!,{r0-r12,lr}\n" \
                                 " mrs   r1, spsr\n" \
                                 " stmfd sp!,{r1}")
#define ISR_EXIT()  asm volatile(" ldmfd sp!,{r1}\n" \
                                 " msr   spsr_c,r1\n" \
                                 " ldmfd sp!,{r0-r12,pc}^")

#define IENABLE()   asm volatile(" mrs   lr, spsr\n" \
                                 " stmfd sp!,{lr}\n" \
                                 " msr   cpsr_c, #0x1F\n" \
                                 " stmfd sp!,{lr}^")
#define IDISABLE()  asm volatile(" ldmfd sp!,{lr}\n" \
                                 " msr   cpsr_c,#0x92\n" \
                                 " ldmfd sp!,{lr}\n" \
								 " msr   spsr_cxsf, lr")

#if defined LPC2106 || defined LPC2119
unsigned short ADC_InBuf[ADC_InBufLen];
unsigned short DAC_OutBuf[DAC_OutBufLen];
#endif
								 
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
	
#define UART0_BASE_ADDR		0xE000C000
#define U0THR          (*(volatile unsigned long *)(UART0_BASE_ADDR + 0x00))
void adcISR(void)
{
  unsigned int newpos;
  ISR_ENTRY();

  //IENABLE();
  AD0_GDR;

  if (trig_pause) {
	if (((AD0_GDR & AD_GDR_CHNMASK) >> 24) == trig_chan) {
		newpos = ((AD0_GDR & AD_GDR_RESULT) >> 6);
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
		ADC_InBuf[adc_fifo_head] = ((AD0_GDR & AD_GDR_RESULT) >> 6) |
			((AD0_GDR & AD_GDR_CHNMASK) >> 12);
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
		  DAC_CR = ((DAC_OutBuf[dac_fifo_tail] << DAC_CR_VALUESHIFT) & DAC_CR_VALUEMASK);
		  newpos = dac_fifo_tail + 1;
		  if (newpos >= DAC_OutBufLen)
			 newpos = 0;
		  dac_fifo_tail = newpos;
	  }
  }

  //IDISABLE();
endadcISR:
#ifdef LPC2378
  VIC_NewVectAddr = 0x00000000;          // clear this interrupt from the VIC
#else
  VIC_VectAddr = 0;
#endif  
  ISR_EXIT();
}
