/******************************************************************************
 *
 * $RCSfile: $
 * $Revision: $
 *
 * This module provides interface routines to the LPC ARM UARTs.
 * Copyright 2004, R O SoftWare
 * No guarantees, warrantees, or promises, implied or otherwise.
 * May be used for hobby or commercial purposes provided copyright
 * notice remains intact.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "lpc210x.h"
#include "seriallpc2000.h"
#include "sysdefs.h"
#include "vic.h"

void serial0_isr (void)   __attribute__ ((naked));
void serial1_isr (void)   __attribute__ ((naked));

unsigned char  uart0_rx_buffer[UART0_RX_BUFFER_SIZE];
unsigned short uart0_rx_insert_idx, uart0_rx_extract_idx;

unsigned char  uart1_rx_buffer[UART1_RX_BUFFER_SIZE];
unsigned short uart1_rx_insert_idx, uart1_rx_extract_idx;

///////////////////////////////////////////////////////////////////////////////
// UART defines

// Interrupt Enable Register bit definitions
#define UIER_ERBFI          (1 << 0)    // Enable Receive Data Available Interrupt
#define UIER_ETBEI          (1 << 1)    // Enable Transmit Holding Register Empty Interrupt
#define UIER_ELSI           (1 << 2)    // Enable Receive Line Status Interrupt
#define UIER_EDSSI          (1 << 3)    // Enable MODEM Status Interrupt

// Interrupt ID Register bit definitions
#define UIIR_NO_INT         (1 << 0)    // NO INTERRUPTS PENDING
#define UIIR_MS_INT         (0 << 1)    // MODEM Status
#define UIIR_THRE_INT       (1 << 1)    // Transmit Holding Register Empty
#define UIIR_RDA_INT        (2 << 1)    // Receive Data Available
#define UIIR_RLS_INT        (3 << 1)    // Receive Line Status
#define UIIR_CTI_INT        (6 << 1)    // Character Timeout Indicator
#define UIIR_ID_MASK        0x0E

// FIFO Control Register bit definitions
#define UFCR_FIFO_ENABLE    (1 << 0)    // FIFO Enable
#define UFCR_RX_FIFO_RESET  (1 << 1)    // Reset Receive FIFO
#define UFCR_TX_FIFO_RESET  (1 << 2)    // Reset Transmit FIFO
#define UFCR_FIFO_TRIG1     (0 << 6)    // Trigger @ 1 character in FIFO
#define UFCR_FIFO_TRIG4     (1 << 6)    // Trigger @ 4 characters in FIFO
#define UFCR_FIFO_TRIG8     (2 << 6)    // Trigger @ 8 characters in FIFO
#define UFCR_FIFO_TRIG14    (3 << 6)    // Trigger @ 14 characters in FIFO

// Line Control Register bit definitions
#define ULCR_CHAR_5         (0 << 0)    // 5-bit character length
#define ULCR_CHAR_6         (1 << 0)    // 6-bit character length
#define ULCR_CHAR_7         (2 << 0)    // 7-bit character length
#define ULCR_CHAR_8         (3 << 0)    // 8-bit character length
#define ULCR_STOP_1         (0 << 2)    // 1 stop bit
#define ULCR_STOP_2         (1 << 2)    // 2 stop bits
#define ULCR_PAR_NO         (0 << 3)    // No Parity
#define ULCR_PAR_ODD        (1 << 3)    // Odd Parity
#define ULCR_PAR_EVEN       (3 << 3)    // Even Parity
#define ULCR_PAR_MARK       (5 << 3)    // MARK "1" Parity
#define ULCR_PAR_SPACE      (7 << 3)    // SPACE "0" Paruty
#define ULCR_BREAK_ENABLE   (1 << 6)    // Output BREAK line condition
#define ULCR_DLAB_ENABLE    (1 << 7)    // Enable Divisor Latch Access

// Modem Control Register bit definitions
#define UMCR_DTR            (1 << 0)    // Data Terminal Ready
#define UMCR_RTS            (1 << 1)    // Request To Send
#define UMCR_LB             (1 << 4)    // Loopback

// Line Status Register bit definitions
#define ULSR_RDR            (1 << 0)    // Receive Data Ready
#define ULSR_OE             (1 << 1)    // Overrun Error
#define ULSR_PE             (1 << 2)    // Parity Error
#define ULSR_FE             (1 << 3)    // Framing Error
#define ULSR_BI             (1 << 4)    // Break Interrupt
#define ULSR_THRE           (1 << 5)    // Transmit Holding Register Empty
#define ULSR_TEMT           (1 << 6)    // Transmitter Empty
#define ULSR_RXFE           (1 << 7)    // Error in Receive FIFO
#define ULSR_ERR_MASK       0x1E

// Modem Status Register bit definitions
#define UMSR_DCTS           (1 << 0)    // Delta Clear To Send
#define UMSR_DDSR           (1 << 1)    // Delta Data Set Ready
#define UMSR_TERI           (1 << 2)    // Trailing Edge Ring Indicator
#define UMSR_DDCD           (1 << 3)    // Delta Data Carrier Detect
#define UMSR_CTS            (1 << 4)    // Clear To Send
#define UMSR_DSR            (1 << 5)    // Data Set Ready
#define UMSR_RI             (1 << 6)    // Ring Indicator
#define UMSR_DCD            (1 << 7)    // Data Carrier Detect

#define CR     0x0D


/******************************************************************************
 *
 * Function Name: uart0ISR()
 *
 * Description:
 *    This function implements the ISR for UART0.
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    void
 *
 *****************************************************************************/
void serial0_isr(void)
{
  register unsigned char iid;

  // perform proper ISR entry so thumb-interwork works properly
  ISR_ENTRY();

  // loop until not more interrupt sources
  while (((iid = UART0_IIR) & UIIR_NO_INT) == 0)
    {
    // identify & process the highest priority interrupt
    switch (iid & UIIR_ID_MASK)
      {
      case UIIR_RLS_INT:                // Receive Line Status
        UART0_LSR;                          // read LSR to clear
        break;
      case UIIR_CTI_INT:                // Character Timeout Indicator
      case UIIR_RDA_INT:                // Receive Data Available
        do
          {
          unsigned short temp;

          // calc next insert index & store character
          temp = (uart0_rx_insert_idx + 1);
		  if (temp >= UART0_RX_BUFFER_SIZE) temp = 0;
          if (temp != uart0_rx_extract_idx) {
			uart0_rx_buffer[uart0_rx_insert_idx] = UART0_RBR;
            uart0_rx_insert_idx = temp; // update insert index
          } else { UART0_RBR; }
        } while (UART0_LSR & ULSR_RDR);
        break;
      default:                          // Unknown
        UART0_LSR;
        UART0_RBR;
        break;
      }
    }
	
#ifdef LPC2378
  VIC_NewVectAddr = 0x00000000;          // clear this interrupt from the VIC
#else
  VIC_VectAddr = 0x00000000;             // clear this interrupt from the VIC
#endif
  ISR_EXIT();                           // recover registers and return
}


/******************************************************************************
 *
 * Function Name: uart1ISR()
 *
 * Description:
 *    This function implements the ISR for UART0.
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    void
 *
 *****************************************************************************/
void serial1_isr(void)
{
  register unsigned char iid;

  // perform proper ISR entry so thumb-interwork works properly
  ISR_ENTRY();

  // loop until not more interrupt sources
  while (((iid = UART1_IIR) & UIIR_NO_INT) == 0)
    {
    // identify & process the highest priority interrupt
    switch (iid & UIIR_ID_MASK)
      {
      case UIIR_RLS_INT:                // Receive Line Status
        UART1_LSR;                          // read LSR to clear
        break;
      case UIIR_CTI_INT:                // Character Timeout Indicator
      case UIIR_RDA_INT:                // Receive Data Available
        do
          {
          unsigned short temp;

          // calc next insert index & store character
          temp = (uart1_rx_insert_idx + 1);
		  if (temp >= UART1_RX_BUFFER_SIZE) temp = 0;
          // check for more room in queue
          if (temp != uart1_rx_extract_idx) { 
			uart1_rx_buffer[uart1_rx_insert_idx] = UART1_RBR;
            uart1_rx_insert_idx = temp; // update insert index
          } else { UART1_RBR; }
        } while (UART1_LSR & ULSR_RDR);
        break;
      default:                          // Unknown
        UART1_LSR;
        UART1_RBR;
        break;
      }
    }

#ifdef LPC2378
  VIC_NewVectAddr = 0x00000000;          // clear this interrupt from the VIC
#else
  VIC_VectAddr = 0x00000000;             // clear this interrupt from the VIC
#endif
  ISR_EXIT();                           // recover registers and return
}


