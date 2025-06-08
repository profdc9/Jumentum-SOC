/*****************************************************************************
*   swi_handler.s: SWI handler file ARM7TDMI-(S) (not just LPC2000)
*
*   by Martin Thomas 
*   http://www.siwawi.arubi.uni-kl.de/avr_projects
*   based on information from ARM documents
*
*   (I think there is nothing left from the Philips example-code beside
*   of the filename and some comments.)
*
*****************************************************************************/

.set SWI_IRQ_DIS,	0
.set SWI_IRQ_EN,	1
.set SWI_FIQ_DIS,	2
.set SWI_FIQ_EN,	3
.set SWI_GET_CPSR,	4
.set SWI_IRQ_REST,	5
.set SWI_FIQ_REST,	6
.set SWI_WDOG,	 	7

.set I_Bit, 0x80
.set F_Bit, 0x40
.set T_Bit, 0x20


  
/*********************************************************************
*  SWI interrupt handler                                             
*  Function :  SoftwareInterrupt(SWI_Number)                         
*              See below "SwiFunction" table                         
*  Parameters:    None                                               
*  input  :       SWI_Number (extracted from SWI command automaticly)
*  output :       states for some SWIs - see below
**********************************************************************/
.text
.arm

.section .text

.global SoftwareInterrupt
.func   SoftwareInterrupt
SoftwareInterrupt:
SWI_HandlerMT:
	STMFD   sp!, {r4, lr}      /* store regs. */
	MRS     r4, spsr
	TST     r4, #T_Bit             /* test for thumb */
	LDRNEH  r4, [lr, #-2]          /* NE->thumb - get swi instruction code */
	BICNE   r4, r4, #0xff00        /* NE->thumb - clear top 8 bits leaving swi "comment field"=number */
	LDREQ   r4, [lr, #-4]          /* EQ->arm - get swi instruction code */
	BICEQ   r4, r4, #0xff000000    /* EQ->arm - clear top 8 bits leaving swi "comment field"=number */
	CMP     r4, #MAX_SWI           /* range-check */
	LDRLS   pc, [pc, r4, LSL #2]   /* jump to routine if <= MAX (LS) */
SWIOutOfRange:
	B       SWIOutOfRange

/* Jump-Table */
SwiTableStart:
	.word IRQDisable	
	.word IRQEnable		
	.word FIQDisable	
	.word FIQEnable		
	.word CPSRget		
	.word IRQRestore	
	.word FIQRestore	
	.word WDOGUpdate	
SwiTableEnd:
.set MAX_SWI, ((SwiTableEnd-SwiTableStart)/4)-1

IRQDisable:
	MRS     r0, SPSR                        /* Get SPSR = return value */
	ORR     r4, r0, #I_Bit                  /* I_Bit set */
	MSR     SPSR_c, r4                      /* Set SPSR */
	B       EndofSWI

IRQEnable:
	MRS     r0, SPSR                        /* Get SPSR = return value */
	BIC     r4, r0, #I_Bit                  /* I_Bit clear */
	MSR     SPSR_c, r4                      /* Set SPSR */
	B       EndofSWI                       

FIQDisable:
	MRS     r0, SPSR
	ORR     r4, r0, #F_Bit
	AND     r0, r0, #F_Bit
	MSR     SPSR_c, r4
	B       EndofSWI

FIQEnable:
	MRS     r0, SPSR
	BIC     r4, r0, #F_Bit
	AND     r0, r0, #F_Bit
	MSR     SPSR_c, r4
	B       EndofSWI

CPSRget:
	MRS     r0, SPSR                        /* Get SPSR */
	B       EndofSWI                       

IRQRestore:
	MRS     r4, SPSR                        /* Get SPSR */
	AND     r0, r0, #I_Bit
	TST     r0, #I_Bit             /* Test input for I_Bit */
	BICEQ   r4, r4, #I_Bit
	ORRNE   r4, r4, #I_Bit
	MSR     SPSR_c, r4
	B       EndofSWI

FIQRestore:
	MRS     r4, SPSR                        /* Get SPSR */
	AND     r0, r0, #F_Bit
	TST     r0, #F_Bit             /* Test input for F_Bit */
	BICEQ   r4, r4, #F_Bit
	ORRNE   r4, r4, #F_Bit
	MSR     SPSR_c, r4
	B       EndofSWI

WDOGUpdate:
    MOV		R4, #-536870912	/* 0xe0000000 */
    MOV		R0, #170
    STR		R0, [R4, #8]
    MOV		R0, #85
    Str		R0, [R4, #8]
	B       EndofSWI                       

EndofSWI:
	LDMFD   sp!, {r4,pc}^
.endfunc


.end

/*************************************************************************
**                            End Of File
**************************************************************************/

