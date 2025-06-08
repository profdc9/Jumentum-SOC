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

/* some vic control code by jcwren and various sources */
/* modified by DLM */
//
//
//
#include <stdio.h>
#include <stdlib.h>

#include "all.h"
#include "sysdefs.h"
#include "lpc210x.h"
#include "vic.h"

/*  we don't have handlers, so these are the defaults */

void IRQ_Routine (void)   __attribute__ ((interrupt("IRQ")));
void Spurious_IRQ_Routine (void)   __attribute__ ((naked));
void FIQ_Routine (void)   __attribute__ ((interrupt("FIQ")));
void UNDEF_Routine (void) __attribute__ ((interrupt("UNDEF")));

void IRQ_Routine (void) {
    oops("IRQ");
}

void FIQ_Routine (void)  {
    oops("FIQ");
}

void UNDEF_Routine (void) {
    oops("UNDEF");
}

#define ISR_ENTRY() asm volatile(" sub   lr, lr,#4\n" \
                                 " stmfd sp!,{r0-r12,lr}\n" \
                                 " mrs   r1, spsr\n" \
                                 " stmfd sp!,{r1}")
#define ISR_EXIT()  asm volatile(" ldmfd sp!,{r1}\n" \
                                 " msr   spsr_c,r1\n" \
                                 " ldmfd sp!,{r0-r12,pc}^")

void Spurious_IRQ_Routine (void)
{
  ISR_ENTRY();
  if (T0_IR != 0) T0_IR = 1;   //clear irq
  AD0_GDR;
#ifdef LPC2378
  VIC_NewVectAddr = 0;   // Clear VIC Interrupt Source
#else
  VIC_VectAddr = 0;   // Clear VIC Interrupt Source
#endif
  ISR_EXIT();
}
