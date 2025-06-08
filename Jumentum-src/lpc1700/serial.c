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

#include "libdriver/LPC17xx.h"
#include "libdriver/lpc17xx_uart.h"
//#include "libdriver/lpc17xx_libcfg.h"
#include "libdriver/lpc17xx_pinsel.h"
#ifdef LPCUSB
#define USB_CONSOLE 0x7E
#include "libdriver/usb/usb_serial.h"
#endif
#ifdef LPCVIDEO
#define VIDEO_CONSOLE 0x7D
#include "video.h"
#include "keyboard.h"
#endif


#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"

//#include "all.h"

#define CR     0x0D

#define U0 ((LPC_UART_TypeDef *)LPC_UART0)
#define U1 ((LPC_UART_TypeDef *)LPC_UART1)

#ifdef LPCUSB
char console_uart = USB_CONSOLE;
#else
#ifdef LPCVIDEO
char console_uart = VIDEO_CONSOLE;
#else
#ifdef STDOUTSER0
char console_uart = 0;
#endif
#ifdef STDOUTSER1
char console_uart = 1;
#endif
#endif
#endif

#define UART0_RX_BUFFER_SIZE 128
#define UART1_RX_BUFFER_SIZE 128

#define SERIALDATALOC 0x20080000

typedef struct _serial_data {
	unsigned char  e_uart0_rx_buffer[UART0_RX_BUFFER_SIZE];
	unsigned short e_uart0_rx_insert_idx, e_uart0_rx_extract_idx;
	unsigned char  e_uart1_rx_buffer[UART1_RX_BUFFER_SIZE];
	unsigned short e_uart1_rx_insert_idx, e_uart1_rx_extract_idx;
} serial_data;

#define SERIALMEM ((serial_data *)(SERIALDATALOC))

#define uart0_rx_buffer (SERIALMEM)->e_uart0_rx_buffer
#define uart0_rx_insert_idx (SERIALMEM)->e_uart0_rx_insert_idx
#define uart0_rx_extract_idx (SERIALMEM)->e_uart0_rx_extract_idx
#define uart1_rx_buffer (SERIALMEM)->e_uart1_rx_buffer
#define uart1_rx_insert_idx (SERIALMEM)->e_uart1_rx_insert_idx
#define uart1_rx_extract_idx (SERIALMEM)->e_uart1_rx_extract_idx

void UART0_IRQHandler(void)
{
  register unsigned char iid;

  // loop until not more interrupt sources
  while (((iid = U0->IIR) & UART_IIR_INTSTAT_PEND) == 0)
    {
    // identify & process the highest priority interrupt
    switch (iid & UART_IIR_INTID_MASK)
      {
      case UART_IIR_INTID_RLS:                // Receive Line Status
        U0->LSR;                          // read LSR to clear
        break;
      case UART_IIR_INTID_CTI:                // Character Timeout Indicator
      case UART_IIR_INTID_RDA:                // Receive Data Available
        do
          {
          unsigned short temp;

          // calc next insert index & store character
          temp = (uart0_rx_insert_idx + 1);
		  if (temp >= UART0_RX_BUFFER_SIZE) temp = 0;
          if (temp != uart0_rx_extract_idx) {
			uart0_rx_buffer[uart0_rx_insert_idx] = U0->RBR;
            uart0_rx_insert_idx = temp; // update insert index
          } else { U0->RBR; }
        } while (U0->LSR & UART_LSR_RDR);
        break;
      default:                          // Unknown
        U0->LSR;
        U0->RBR;
        break;
      }
    }
}

void UART1_IRQHandler(void)
{
  register unsigned char iid;
  // loop until not more interrupt sources
  while (((iid = U1->IIR) &  UART_IIR_INTSTAT_PEND) == 0)
    {
    // identify & process the highest priority interrupt
    switch (iid & UART_IIR_INTID_MASK)
      {
      case UART_IIR_INTID_RLS:                // Receive Line Status
        U1->LSR;                          // read LSR to clear
        break;
      case UART_IIR_INTID_CTI:                // Character Timeout Indicator
      case UART_IIR_INTID_RDA:                // Receive Data Available
        do
          {
          unsigned short temp;

          // calc next insert index & store character
          temp = (uart1_rx_insert_idx + 1);
		  if (temp >= UART1_RX_BUFFER_SIZE) temp = 0;
          // check for more room in queue
          if (temp != uart1_rx_extract_idx) { 
			uart1_rx_buffer[uart1_rx_insert_idx] = U1->RBR;
            uart1_rx_insert_idx = temp; // update insert index
          } else { U1->RBR; }
        } while (U1->LSR & UART_LSR_RDR);
        break;
      default:                          // Unknown
        U1->LSR;
        U1->RBR;
        break;
      }
    }
}

static void fill_config_struct( unsigned long baudrate, int databits, int stopbits, int parity, UART_CFG_Type *ucs )
{
	ucs->Baud_rate = baudrate;	
	switch (parity) {
		case 1: ucs->Parity = UART_PARITY_ODD; break;
		case 2: ucs->Parity = UART_PARITY_EVEN; break;
		case 0: 
		default:  ucs->Parity = UART_PARITY_NONE; break;
	}
	switch (databits) {
		case 7: ucs->Databits = UART_DATABIT_7; break;
		case 8: 
		default: ucs->Databits = UART_DATABIT_8; break;
	}
	switch (stopbits) {
		case 2: ucs->Stopbits = UART_STOPBIT_2; break;
		case 1:
		default: ucs->Stopbits = UART_STOPBIT_1; break;
	}
}

/* Initialize Serial Interface UART0 */
void init_serial0 ( unsigned long baudrate, int databits, int stopbits, int parity )
{
	// UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	// Pin configuration for UART0
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize UART0 pin connect
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 2;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 3;
	PINSEL_ConfigPin(&PinCfg);

	UART_ConfigStructInit(&UARTConfigStruct);
	fill_config_struct(baudrate,databits,stopbits,parity,&UARTConfigStruct);
	UART_Init(U0, &UARTConfigStruct);
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	UART_FIFOConfig(U0, &UARTFIFOConfigStruct);
	UART_TxCmd(U0, ENABLE);

    // initialize the receive data queue
	uart0_rx_extract_idx = uart0_rx_insert_idx = 0;

    /* Enable UART Rx interrupt */
	UART_IntConfig(U0, UART_INTCFG_RBR, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig(U0, UART_INTCFG_RLS, ENABLE);
	
	    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(UART0_IRQn, ((0x01<<3)|0x01));
	/* Enable Interrupt for UART0 channel */
    NVIC_EnableIRQ(UART0_IRQn);
}

/* Write character to Serial Port 0 without \n -> \r\n  */
static int putc_serial0 (int ch)
{
	while (!(U0->LSR & UART_LSR_THRE));
	return (U0->THR = ch);
}

/* Write character to Serial Port 0 with \n -> \r\n  */
static int putchar_serial0 (int ch)
{
    if (ch == '\n') putc_serial0(CR);
	return putc_serial0(ch);
}


void putstring_serial0 (const char *string)
{
	char ch;

	while ((ch = *string)) {
		putchar_serial0(ch);
		string++;
	}
}

int getkey_serial0(void)
{
  int ch, newpos;

  WATCHDOG_UPDATE();
  if (uart0_rx_insert_idx == uart0_rx_extract_idx) // check if character is available
    return -1;

  ch = uart0_rx_buffer[uart0_rx_extract_idx]; // get character, bump pointer
  newpos = uart0_rx_extract_idx + 1;
  if (newpos >= UART0_RX_BUFFER_SIZE)
      newpos = 0;
  uart0_rx_extract_idx = newpos;
  return ch;
}

/* Wait for  character from Serial Port   */
int waitkey_serial0 (void)
{
    int ch;
    while ((ch=getkey_serial0()) == -1);
	return ch;
}

/* Initialize Serial Interface UART1 */
void init_serial1 ( unsigned long baudrate, int databits, int stopbits, int parity )
{
	// UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	// Pin configuration for UART0
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize UART1 pin connect
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 0;
	PinCfg.Portnum = 2;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 1;
	PINSEL_ConfigPin(&PinCfg);

	UART_ConfigStructInit(&UARTConfigStruct);
	fill_config_struct(baudrate,databits,stopbits,parity,&UARTConfigStruct);
	UART_Init(U1, &UARTConfigStruct);
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	UART_FIFOConfig(U1, &UARTFIFOConfigStruct);
	UART_TxCmd(U1, ENABLE);

    // initialize the receive data queue
	uart1_rx_extract_idx = uart1_rx_insert_idx = 0;

    /* Enable UART Rx interrupt */
	UART_IntConfig(U1, UART_INTCFG_RBR, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig(U1, UART_INTCFG_RLS, ENABLE);
	
	    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(UART1_IRQn, ((0x01<<3)|0x01));
	/* Enable Interrupt for UART0 channel */
    NVIC_EnableIRQ(UART1_IRQn);
}

int putc_serial1 (int ch)
{
	while (!(U1->LSR & UART_LSR_THRE));
	return (U1->THR = ch);
}


/* Write character to Serial Port 0 with \n -> \r\n  */
int putchar_serial1 (int ch)
{
    if (ch == '\n') putc_serial1(CR);
	return putc_serial1(ch);
}

void putstring_serial1 (const char *string)
{
	char ch;

	while ((ch = *string)) {
		putchar_serial1(ch);
		string++;
	}
}

int getkey_serial1(void)
{
  int ch, newpos;

  WATCHDOG_UPDATE();
  if (uart1_rx_insert_idx == uart1_rx_extract_idx) // check if character is available
    return -1;
  ch = uart1_rx_buffer[uart1_rx_extract_idx]; // get character, bump pointer
  newpos = uart1_rx_extract_idx + 1;
  if (newpos >= UART1_RX_BUFFER_SIZE)
      newpos = 0;
  uart1_rx_extract_idx = newpos;
  return ch;
}

/* Wait for  character from Serial Port   */
int waitkey_serial1 (void)
{
    int ch;
    while ((ch=getkey_serial1()) == -1);
	return ch;
}

#ifdef LPCUSB
/* Write character to Serial Port 0 with \n -> \r\n  */
static int putchar_serialusb (int ch)
{
    if (ch == '\n') VCOM_putchar(CR);
	return VCOM_putchar(ch);
}

/* Wait for  character from Serial Port   */
int waitkey_serialusb (void)
{
    int ch;
    while ((ch=VCOM_getchar()) == -1);
	return ch;
}

void putstring_serialusb (const char *string)
{
	char ch;

	while ((ch = *string)) {
		putchar_serialusb(ch);
		string++;
	}
}
#endif

/* Holder functions for future expansion to multiple serial ports */
int putchar_serial_c(int com, int ch)
{
	switch (com) {
		case 0: return putchar_serial0(ch);
		case 1: return putchar_serial1(ch);
#ifdef LPCUSB
		case USB_CONSOLE:  return putchar_serialusb(ch);
#endif
#ifdef LPCVIDEO
		case VIDEO_CONSOLE:  return putchar_video(ch);
#endif
	}
	return -1;
}

int putc_serial_c(int com, int ch)
{
	switch (com) {
		case 0: return putc_serial0(ch);
		case 1: return putc_serial1(ch);
#ifdef LPCUSB
		case USB_CONSOLE:  return VCOM_putchar(ch);
#endif
#ifdef LPCVIDEO
		case VIDEO_CONSOLE:  return putc_video(ch);
#endif
	}
	return -1;
}

int getkey_serial_c(int com)
{
  switch (com) {
	  case 0:  return getkey_serial0();
	  case 1:  return getkey_serial1();
#ifdef LPCUSB
	  case USB_CONSOLE:  return VCOM_getchar();
#endif
#ifdef LPCVIDEO
	  case VIDEO_CONSOLE:  return keyboard_getchar();
#endif
  }
  return -1;
}

int waitkey_serial_c(int com)
{
  switch (com) {
	  case 0:  return waitkey_serial0();
	  case 1:  return waitkey_serial1();
#ifdef LPCUSB
	  case USB_CONSOLE:  return waitkey_serialusb();
#endif
#ifdef LPCVIDEO
	  case VIDEO_CONSOLE:  return waitkey_keyboard();
#endif
  }
  return -1;
}

void init_serial_c (int com, unsigned long baudrate, int databits, int stopbits, int parity )
{
  switch (com) {
	  case 0: init_serial0(baudrate,databits,stopbits,parity);
			  break;
	  case 1: init_serial1(baudrate,databits,stopbits,parity);
			  break;
  }
  return;
}

void putstring_serial_c (int com, const char *string)
{
  switch (com) {
		case 0:  putstring_serial0(string);
				 return;
		case 1:  putstring_serial1(string);
				 return;
#ifdef LPCUSB
		case USB_CONSOLE:  putstring_serialusb(string);
				           return;
#endif
#ifdef LPCVIDEO
		case VIDEO_CONSOLE:  putstring_video(string);
				             return;
#endif
  }
}
