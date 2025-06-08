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

/******************************************************************************/
/*                                                                            */
/*  SERIAL.C:  Low Level Serial Routines                                      */
/*  modified and extended by Martin Thomas                                    */
/*                                                                            */
/******************************************************************************/

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

//#include "all.h"
#include "lpc210x.h"
#include "seriallpc2000.h"
#include "cpu.h"
#ifdef LPCUSB
#define USB_CONSOLE 0x7E
#include "usb/usb_serial.h"
#endif

/* Pin Connect Block */
#define PINSEL_BASE_ADDR	0xE002C000
#define PINSEL0        (*(volatile unsigned long *)(PINSEL_BASE_ADDR + 0x00))
#define PINSEL1        (*(volatile unsigned long *)(PINSEL_BASE_ADDR + 0x04))
#define PINSEL2        (*(volatile unsigned long *)(PINSEL_BASE_ADDR + 0x14))

#define UIER_ERBFI          (1 << 0)    // Enable Receive Data Available Interrupt

/* System configuration: Fosc, Fcclk, Fcco, Fpclk must be defined */
/* Crystal frequence,10MHz~25MHz should be the same as actual status. */
#define Fosc	12000000

/* System frequence,should be (1~32)multiples of Fosc,and should be equal or 
less than 60MHz. */
#ifdef LPC2378
#define Fcclk	(Fosc * 4)
#else
#define Fcclk	(Fosc * 5)
#endif

/* CCO frequence,should be 2/4/8/16 multiples of Fcclk, ranged from 156MHz to 
320MHz. */
#define Fcco	(Fcclk * 4)

/* VPB clock frequence , must be 1/2/4 multiples of (Fcclk / 4). */
#define Fpclk	(Fcclk / 1) * 1

#define CR     0x0D

#ifdef LPCUSB
char console_uart = USB_CONSOLE;
#else
#ifdef STDOUTSER0
char console_uart = 0;
#endif
#ifdef STDOUTSER1
char console_uart = 1;
#endif
#endif
static unsigned long calc_lcr(int databits, int stopbits, int parity)
{
	int lcr = 0x0;
	switch (databits) {
		case 6: lcr |= 0x01; break;
		case 7: lcr |= 0x02; break;
		case 8: lcr |= 0x03; break;
	}
	if (stopbits == 2) lcr |= 0x04;
	switch (parity) {
		case 1: lcr |= 0x08; break;
		case 2: lcr |= 0x18; break;
	}
	return lcr;
}

/* Initialize Serial Interface UART0 */
void init_serial0 ( unsigned long baudrate, int databits, int stopbits, int parity )
{
  unsigned long Fdiv;
  unsigned long lcr;
 
  swi_irq_disable();
  // set port pins for UART0
#ifdef LPC2378
  SET_PINSEL0(PINSEL0 & ~0x0F0);
  SET_PINSEL0(PINSEL0 | 0x050);                       /* Enable RxD0 and TxD0              */
#else
  SET_PINSEL0(PINSEL0 & ~0x0F);
  SET_PINSEL0(PINSEL0 | 0x05);                        /* Enable RxD0 and TxD0              */
#endif

  UART0_IER = 0x00;                         // disable all interrupts
  UART0_IIR;                                // clear interrupt ID
  UART0_RBR;                                // clear receive register
  UART0_LSR;                                // clear line status register

  // set the baudrate
  lcr = calc_lcr(databits,stopbits,parity);
  UART0_LCR = lcr | 0x80; 
  Fdiv = ( Fpclk / 16 ) / baudrate;          /* baud rate                        */
  UART0_DLL = (unsigned char)Fdiv;                // set for baud low byte
  UART0_DLM = (unsigned char)(Fdiv >> 8);         // set for baud high byte

  // set the number of characters and other
  // user specified operating parameters
  UART0_LCR = lcr;                           /* 8 bits, no stop bit, no parity */
  UART0_FCR = 0x07;

  // initialize the interrupt vector
#ifdef LPC2378
  VIC_IntSelect &= ~VIC_BIT(VIC_UART0);  // UART0 selected as IRQ
  VIC_IntEnable = VIC_BIT(VIC_UART0);    // UART0 interrupt enabled
  VIC_VectCntl6 = 0x03;
  VIC_VectAddr6 = (unsigned long)serial0_isr;    // address of the ISR
#else
  VIC_IntSelect &= ~VIC_BIT(VIC_UART0);  // UART0 selected as IRQ
  VIC_IntEnable = VIC_BIT(VIC_UART0);    // UART0 interrupt enabled
  VIC_VectCntl2 = VIC_ENABLE | VIC_UART0;
  VIC_VectAddr2 = (unsigned long)serial0_isr;    // address of the ISR
#endif

  // initialize the receive data queue
  uart0_rx_extract_idx = uart0_rx_insert_idx = 0;

  // enable receiver interrupts
  UART0_IER = UIER_ERBFI;
  swi_irq_enable();
}

/* Write character to Serial Port 0 without \n -> \r\n  */
static int putc_serial0 (int ch)
{
	while (!(UART0_LSR & 0x20));
	return (UART0_THR = ch);
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

/******************************************************************************
 *
 * Function Name: uart0Getch()
 *
 * Description:  
 *    This function gets a character from the UART receive queue
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    character on success, -1 if no character is available
 *
 *****************************************************************************/
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

#if 0
/* Read character from Serial Port   */
int getkey_serial0 (void)
{
    WATCHDOG_UPDATE();
	return (UART0_LSR & 0x01 ? UART0_RBR : -1);
}

/* Wait for  character from Serial Port   */
int waitkey_serial0 (void)
{
	do {
		WATCHDOG_UPDATE();
	} while (!(UART0_LSR & 0x01));
	return (UART0_RBR);
}
#endif


/* Initialize Serial Interface UART1 */
void init_serial1 ( unsigned long baudrate, int databits, int stopbits, int parity )
{
  unsigned long Fdiv;
  unsigned long lcr;
 
  swi_irq_disable();
  // set port pins for UART1
#ifdef LPC2378
  SET_PINSEL0(PINSEL0 & ~0xC0000000);
  SET_PINSEL0(PINSEL0 | 0x40000000);
  SET_PINSEL1(PINSEL1 & ~0x00000003);
  SET_PINSEL1(PINSEL1 | 0x00000001);
#else
  SET_PINSEL0(PINSEL0 & ~0x0F0000);
  SET_PINSEL0(PINSEL0 | 0x050000);          /* Enable RxD0 and TxD0 */
#endif
  UART1_IER = 0x00;                         // disable all interrupts
  UART1_IIR;                                // clear interrupt ID
  UART1_RBR;                                // clear receive register
  UART1_LSR;                                // clear line status register

  // set the baudrate
  lcr = calc_lcr(databits,stopbits,parity);
  UART1_LCR = lcr | 0x80; 
  Fdiv = ( Fpclk / 16 ) / baudrate;               /* baud rate */
  UART1_DLL = (unsigned char)Fdiv;                // set for baud low byte
  UART1_DLM = (unsigned char)(Fdiv >> 8);         // set for baud high byte

  // set the number of characters and other
  // user specified operating parameters
  UART1_LCR = lcr;                           /* 8 bits, no stop bit, no parity */
  UART1_FCR = 0x07;

  // initialize the interrupt vector
#ifdef LPC2378
  VIC_IntSelect &= ~VIC_BIT(VIC_UART1);  // UART1 selected as IRQ
  VIC_IntEnable = VIC_BIT(VIC_UART1);    // UART1 interrupt enabled
  VIC_VectCntl7 = 0x03;
  VIC_VectAddr7 = (unsigned long)serial1_isr;    // address of the ISR
#else
  VIC_IntSelect &= ~VIC_BIT(VIC_UART1);  // UART1 selected as IRQ
  VIC_IntEnable = VIC_BIT(VIC_UART1);    // UART1 interrupt enabled
  VIC_VectCntl3 = VIC_ENABLE | VIC_UART1;
  VIC_VectAddr3 = (unsigned long)serial1_isr;    // address of the ISR
#endif

  // initialize the receive data queue
  uart1_rx_extract_idx = uart1_rx_insert_idx = 0;

  // enable receiver interrupts
  UART1_IER = UIER_ERBFI;
  swi_irq_enable();
}

/* Write character to Serial Port 0 without \n -> \r\n  */
int putc_serial1 (int ch)
{
	while (!(UART1_LSR & 0x20));
	return (UART1_THR = ch);
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

#if 0
/* Read character from Serial Port   */
int getkey_serial1 (void)
{
    WATCHDOG_UPDATE();
	return (UART1_LSR & 0x01 ? UART1_RBR : -1);
}

/* Wait for  character from Serial Port   */
int waitkey_serial1 (void)
{ 
	do {
      WATCHDOG_UPDATE();
    } while (!(UART1_LSR & 0x01));
	return (UART1_RBR);
}
#endif

/******************************************************************************
 *
 * Function Name: uart1Getch()
 *
 * Description:  
 *    This function gets a character from the UART receive queue
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    character on success, -1 if no character is available
 *
 *****************************************************************************/
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
		case USB_CONSOLE:  putchar_serialusb(ch);
				           return;
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
  }
}
