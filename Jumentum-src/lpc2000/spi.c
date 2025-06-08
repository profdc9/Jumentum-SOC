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

/* Support for SSP added by DLM */

/* BITBANG SPI support mostly added as a hack for LPC2378 to access SD/MMC
   card through the MMC/SD port since I don't know how to control it using
   their interface yet, and I would have to write my own driver to interface
   to ELM FAT FS */

#include <stdio.h>
#include <stdlib.h>

#include "spi.h"
#include "lpc210x.h"
#include "lib.h"
#include "cpu.h"


#ifdef SPIBITBANG
#define SPISPEED 2

#ifndef LPC2378
#define SETMOSI() GPIO0_IOSET = GPIO_IO_P6
#define CLRMOSI() GPIO0_IOCLR = GPIO_IO_P6
#define SETCLK() GPIO0_IOSET = GPIO_IO_P4
#define CLRCLK() GPIO0_IOCLR = GPIO_IO_P4
#define READMISO() ((GPIO0_IOPIN & GPIO_IO_P5) != 0)
#else
#define SETMOSI() GPIO0_IOSET = GPIO_IO_P20
#define CLRMOSI() GPIO0_IOCLR = GPIO_IO_P20
#define SETCLK() GPIO0_IOSET = GPIO_IO_P19
#define CLRCLK() GPIO0_IOCLR = GPIO_IO_P19
#define READMISO() ((GPIO0_IOPIN & GPIO_IO_P22) != 0)

#define PCB_PINSEL4 (*(pREG32 (0xe002c010)))
#define FIO2DIR (*(pREG32 (0x3fffc040)))
#define FIO2SET (*(pREG32 (0x3fffc058)))
#define FIO2CLR (*(pREG32 (0x3fffc05c)))
#define FIO2PIN (*(pREG32 (0x3fffc054)))

#define SELBITMASK (1<<13)
#define OTHERBITMASK ((1<<11)|(1<<12))  // set MCIDAT1 and MCIDAT2 to high
#define SETUPSSEL() { PCB_PINSEL4 = PCB_PINSEL4 & ~(0x0FC00000); FIO2DIR = FIO2DIR | (SELBITMASK | OTHERBITMASK); FIO2SET = (SELBITMASK | OTHERBITMASK); }
#define SETSEL() FIO2SET = SELBITMASK;
#define CLRSEL() FIO2CLR = SELBITMASK;
#endif
#endif

/******************************************************************************/
/** \file spi.c
 *  \brief Driver code lpc2106 SPI peripheral.
 *  \author Iain Derrington (www.kandi-electronics.com)
 *  \date 0.1 20/06/07 First draft
 */
/*******************************************************************************/


/***********************************************************************/
/** \brief Initilialise the SPI peripheral
 *
 * Description: init the LPC2106 SPI peripheral. Simple non flexible version
 *              
 *              
 * \author Iain Derrington

 */
/**********************************************************************/

#define ABRT		1 << 3		/* SPI0 interrupt status */
#define MODF		1 << 4
#define ROVR		1 << 5
#define WCOL		1 << 6
#define SPIF		1 << 7

void spiResetEthernet(void)
{
    int i;

	for (i=0;i<10000;i++) GPIO0_IOCLR = SPI_ETH_RST;
	for (i=0;i<10000;i++) GPIO0_IOSET = SPI_ETH_RST;
}

void spiInit(void)
{

#ifdef SPIBITBANG
#ifndef LPC2378
    SET_PINSEL0(PCB_PINSEL0 & ~(PCB_PINSEL0_P04_MASK | PCB_PINSEL0_P05_MASK | PCB_PINSEL0_P06_MASK | PCB_PINSEL0_P07_MASK));
    SET_PINSEL0(PCB_PINSEL0 & ~(PCB_PINSEL0_P08_MASK | PCB_PINSEL0_P09_MASK | PCB_PINSEL0_P010_MASK));
    GPIO0_IOSET = SPI_MMC_SEL | SPI_ETH_SEL | SPI_ETH_RST;
	GPIO0_IOCLR = GPIO_IO_P4 | GPIO_IO_P6;
    SET_GPIODIR0(GPIO0_IODIR | (SPI_MMC_SEL | SPI_ETH_SEL | SPI_ETH_RST |
	              GPIO_IO_P4 | GPIO_IO_P6));
	SET_GPIODIR0(GPIO0_IODIR & ~(GPIO_IO_P5 | GPIO_IO_P7));
#else
    U16  i,j;
	SET_PINSEL1(PCB_PINSEL1 & ~(PCB_PINSEL1_P019_MASK | PCB_PINSEL1_P020_MASK | PCB_PINSEL1_P021_MASK | PCB_PINSEL1_P022_MASK));
	GPIO0_IOCLR = GPIO_IO_P19 | GPIO_IO_P20;
	GPIO0_IOSET = GPIO_IO_P21;
	SET_GPIODIR0(GPIO0_IODIR | (GPIO_IO_P20 | GPIO_IO_P19 | GPIO_IO_P21));
	SET_GPIODIR0(GPIO0_IODIR & ~(GPIO_IO_P22));
	SETUPSSEL();
	for (i=0;i<10000;i++) GPIO0_IOSET = GPIO_IO_P21;
#endif
#endif

#ifdef SPI0
    U16  i;

	S0SPCR = 0x00;

    SET_PINSEL0(PCB_PINSEL0 & ~(PCB_PINSEL0_P04_MASK | PCB_PINSEL0_P05_MASK | PCB_PINSEL0_P06_MASK | PCB_PINSEL0_P07_MASK));
	SET_PINSEL0(PCB_PINSEL0 | (PCB_PINSEL0_P04_SCK0 | PCB_PINSEL0_P05_MISO0 | PCB_PINSEL0_P06_MOSI0 | PCB_PINSEL0_P07_SSEL0));
    SET_PINSEL0(PCB_PINSEL0 & ~(PCB_PINSEL0_P08_MASK | PCB_PINSEL0_P09_MASK | PCB_PINSEL0_P010_MASK));
    GPIO0_IOSET = SPI_MMC_SEL | SPI_ETH_SEL | SPI_ETH_RST;
    SET_GPIODIR0(GPIO0_IODIR | (SPI_MMC_SEL | SPI_ETH_SEL | SPI_ETH_RST));

    S0SPCCR = SPI_MMC_SPEED;               
    S0SPCR  = SPI0_MSTR;      // spi module in master mode, CPOL = 0, CCPHA = 0. MSB first
	
    i = S0SPSR;               // read SPI status reg to clear the flags.
#endif
#ifdef SPI1
  U32 i;
  volatile U32 dummy;

  GPIO0_IOSET = SPI_MMC_SEL | SPI_ETH_SEL | SPI_ETH_RST;
  SET_GPIODIR0(GPIO0_IODIR | (SPI_MMC_SEL | SPI_ETH_SEL | SPI_ETH_RST));

  //
  //  SPI init
  //
  SCB_PCONP |= SCB_PCONP_PCSPI1;

  SSP_CR0  = SSP_CR0_DSS_8 | SSP_CR0_FRF_SPI;
  SSP_CR1  = 0x00;
  SSP_IMSC = 0x00;

  SSP_CPSR = SPI_MMC_SPEED;

  SET_PINSEL1(PCB_PINSEL1 &  ~(PCB_PINSEL1_P017_MASK | PCB_PINSEL1_P018_MASK | PCB_PINSEL1_P019_MASK));
  SET_PINSEL1(PCB_PINSEL1 |  (PCB_PINSEL1_P017_SCK1 | PCB_PINSEL1_P018_MISO1 | PCB_PINSEL1_P019_MOSI1));
  SET_PINSEL1(PCB_PINSEL1 & ~(PCB_PINSEL1_P020_MASK | PCB_PINSEL1_P021_MASK | PCB_PINSEL1_P022_MASK));

  //
  //  Enable SPI
  //
  SSP_CR1 |= SSP_CR1_SSE;

  dummy = SSP_SR;
  
  for (i = 0; i < 8; i++)
    dummy = SSP_DR;

#endif
}

/***********************************************************************/
/** \brief SPiWrite 
 *
 * Description: Writes bytes from buffer to SPI tx reg

 * \author Iain Derrington
 * \param ptrBuffer Pointer to buffer containing data.
 * \param ui_Len number of bytes to transmit.
 * \return uint Number of bytes transmitted.
 */
/**********************************************************************/
U16  SPIWrite(U8  * ptrBuffer, U16  ui_Len)
{
  U16  i,stat;
  
#ifdef SPI0
  stat= S0SPSR;
#endif
  
  if (ui_Len == 0)           // no data no send
    return 0;

    for (i=0;i<ui_Len;i++)
    {
#ifdef SPIBITBANG
      SPIBitBang(*ptrBuffer++);
#endif
#ifdef SPI0
      S0SPDR= *ptrBuffer++;     // load spi tx reg
      while(!(S0SPSR & SPIF));   // wait for transmission to complete
#endif
#ifdef SPI1
	  while (!(SSP_SR & SSP_SR_TNF));
	  SSP_DR = *ptrBuffer++;
	  while (!(SSP_SR & SSP_SR_RNE));
	  stat = SSP_DR;
#endif
    }
    return i;
}

void spiEthChipSelect(BOOL select)
{
  if (select) {
#ifdef SPIBITBANG
    GPIO0_IOCLR = SPI_ETH_SEL;
#endif
#ifdef SPI0
	S0SPCCR = SPI_ETH_SPEED;
    GPIO0_IOCLR = SPI_ETH_SEL;
#endif
#ifdef SPI1
	SSP_CPSR = SPI_ETH_SPEED;
    GPIO0_IOCLR = SPI_ETH_SEL;
#endif
  } 
  else
  {
#ifdef SPIBITBANG
    GPIO0_IOSET = SPI_ETH_SEL;
#endif
#ifdef SPI0
    GPIO0_IOSET = SPI_ETH_SEL;
#endif
#ifdef SPI1    
    GPIO0_IOSET = SPI_ETH_SEL;
    while (!(SSP_SR & SSP_SR_TNF));
    SSP_DR = 0xff;
    //
    // Wait until TX fifo and TX shift buffer are empty
    //
    while (SSP_SR & SSP_SR_BSY);
    while (!(SSP_SR & SSP_SR_RNE));
    do
    {
      select = SSP_DR;
    } while (SSP_SR & SSP_SR_RNE);
#endif
  }
}

/***********************************************************************/
/** \brief SPIRead
 *
 * Description: Will read ui_Length bytes from SPI. Bytes placed into ptrbuffer

 * \author Iain Derrington
 * \param ptrBuffer Buffer containing data.
 * \param ui_Len Number of bytes to read.
 * \return uint Number of bytes read.
 */
/**********************************************************************/
U16  SPIRead(U8  * ptrBuffer, U16  ui_Len)
{
U16  i,stat;

#ifdef SPI0
  stat= S0SPSR;
#endif

  for (i=0;i<ui_Len;i++)
  {
#ifdef SPIBITBANG
    *ptrBuffer++ = SPIBitBang(0xFF);
#endif
#ifdef SPI0
    S0SPDR= 0xff;                  // dummy transmit byte    
    while(!(S0SPSR & SPIF));                // wait for transmission to complete         
    *ptrBuffer = S0SPDR;              // read data from SPI data reg, place into buffer
	ptrBuffer++;
#endif
#ifdef SPI1
	while (!(SSP_SR & SSP_SR_TNF));
	SSP_DR = 0xFF;
	while (!(SSP_SR & SSP_SR_RNE));
	*ptrBuffer = SSP_DR;
	ptrBuffer++;
#endif
  }
  return i;
}

void spiMMCChipSelect (BOOL select)
{
  if (select) {
#ifdef SPIBITBANG
#ifndef LPC2378
    GPIO0_IOCLR = SPI_MMC_SEL;
#else
	CLRSEL();
#endif
#endif
#ifdef SPI0
	S0SPCCR = SPI_MMC_SPEED;
    GPIO0_IOCLR = SPI_MMC_SEL;
#endif
#ifdef SPI1
	SSP_CPSR = SPI_MMC_SPEED;	
    GPIO0_IOCLR = SPI_MMC_SEL;
#endif
  }
  else
  {
#ifdef SPIBITBANG
#ifndef LPC2378
    GPIO0_IOSET = SPI_MMC_SEL;
#else
	SETSEL();
#endif
#endif
#ifdef SPI0
    GPIO0_IOSET = SPI_MMC_SEL;
#endif
#ifdef SPI1    
    GPIO0_IOSET = SPI_MMC_SEL;
    while (!(SSP_SR & SSP_SR_TNF));
    SSP_DR = 0xff;
    //
    // Wait until TX fifo and TX shift buffer are empty
    //
    while (SSP_SR & SSP_SR_BSY);
    while (!(SSP_SR & SSP_SR_RNE));
    do
    {
      select = SSP_DR;
    } while (SSP_SR & SSP_SR_RNE);
#endif
  }
}

void spiSendByte (U8 c)
{
#ifdef SPIBITBANG
  SPIBitBang(c);
#endif
#ifdef SPI0
  S0SPDR = c;     
  while(!(S0SPSR & SPIF));  
#endif
#ifdef SPI1
  while (!(SSP_SR & SSP_SR_TNF));
  SSP_DR = c;
  while (!(SSP_SR & SSP_SR_RNE));
  c = SSP_DR;
#endif
}

U8 spiReceiveByte (void)
{
#ifdef SPIBITBANG
  return SPIBitBang(0xFF);
#endif
#ifdef SPI0
  S0SPDR= 0xff;              
  while(!(S0SPSR & SPIF));   
  return S0SPDR;              
#endif
#ifdef SPI1
  while (!(SSP_SR & SSP_SR_TNF));
  SSP_DR = 0xFF;
  while (!(SSP_SR & SSP_SR_RNE));
  return SSP_DR;
#endif
}

void spiReceivePtr (U8 *c)
{
#ifdef SPIBITBANG
  *c = SPIBitBang(0xFF);
#endif
#ifdef SPI0
  S0SPDR= 0xff;              
  while(!(S0SPSR & SPIF));   
  *c = S0SPDR;              
#endif
#ifdef SPI1
  while (!(SSP_SR & SSP_SR_TNF));
  SSP_DR = 0xFF;
  while (!(SSP_SR & SSP_SR_RNE));
  *c = SSP_DR;
#endif
}

#ifdef SPIBITBANG

U8 SPIBitBang(U8 c)
{
  U8 b = 0;
  int i = 8,j;
  do {
	 if (c & 0x80) SETMOSI();
	    else CLRMOSI();
	 c <<= 1;
	 for (j=SPISPEED;j>0;j--) CLRCLK();
	 for (j=SPISPEED;j>0;j--) SETCLK();
	 b = (b<<1) | READMISO();
	 for (j=SPISPEED;j>0;j--) CLRCLK();
  } while ((--i)>0);
  CLRMOSI();
  return b;
}
#endif
