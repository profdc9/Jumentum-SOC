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

#ifdef MBED
#define LPC17XX_SSP1
#endif
   
#include "libdriver/LPC17xx.h"
#include "libdriver/lpc17xx_ssp.h"

#ifdef LPC17XX_SSP1
#define CS_PORT_NUM 0
#define CS_PIN_NUM 6
#define SSPx LPC_SSP1
#else
#define CS_PORT_NUM 0
#define CS_PIN_NUM 16
#define SSPx LPC_SSP0
#endif

#include "libdriver/lpc17xx_pinsel.h"
#include "libdriver/lpc17xx_gpio.h"
   
#include <stdio.h>
#include <stdlib.h>

#include "spi.h"
#include "lib.h"
#include "cpu.h"

void CS_Set(int32_t state)
{
	if (state){
		GPIO_SetValue(CS_PORT_NUM, (1<<CS_PIN_NUM));
	}else{
		GPIO_ClearValue(CS_PORT_NUM, (1<<CS_PIN_NUM));
	}
}

void spiInit(void)
{
	PINSEL_CFG_Type PinCfg;

	SSP_CFG_Type SSP_ConfigStruct;

	/*
	 * Initialize SPI pin connect
	 * P0.15 - SCK
	 * P0.16 - SSEL - used as GPIO
	 * P0.17 - MISO
	 * P0.18 - MOSI
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
#ifdef LPC17XX_SSP1
	PinCfg.Pinnum = 7;
#else	
	PinCfg.Pinnum = 15;
#endif
	PINSEL_ConfigPin(&PinCfg);
#ifdef LPC17XX_SSP1
	PinCfg.Pinnum = 8;
#else
	PinCfg.Pinnum = 17;
#endif
	PINSEL_ConfigPin(&PinCfg);
#ifdef LPC17XX_SSP1
	PinCfg.Pinnum = 9;
#else
	PinCfg.Pinnum = 18;
#endif
	PINSEL_ConfigPin(&PinCfg);
#ifdef LPC17XX_SSP1
	PinCfg.Pinnum = 6;
#else
	PinCfg.Pinnum = 16;
#endif
	PinCfg.Funcnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	GPIO_SetDir(CS_PORT_NUM, (1<<CS_PIN_NUM), 1);
	GPIO_SetValue(CS_PORT_NUM, (1<<CS_PIN_NUM));

	// initialize SSP configuration structure to default
	SSP_ConfigStructInit(&SSP_ConfigStruct);
	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(SSPx, &SSP_ConfigStruct);
	// Enable SSP peripheral
	SSP_Cmd(SSPx, ENABLE);
}

U16  SPIWrite(U8  * ptrBuffer, U16  ui_Len)
{
  U16  i,stat;

    for (i=0;i<ui_Len;i++)
    {
		while (!(SSPx->SR & SSP_SR_TNF));
		SSPx->DR = *ptrBuffer++;
		while (!(SSPx->SR & SSP_SR_RNE));
		stat = SSPx->DR;
	}
	return i;
}

/**********************************************************************/
U16  SPIRead(U8  * ptrBuffer, U16  ui_Len)
{
  U16  i,stat;

  for (i=0;i<ui_Len;i++)
  {
	while (!(SSPx->SR & SSP_SR_TNF));
	SSPx->DR = 0xFF;
	while (!(SSPx->SR & SSP_SR_RNE));
	*ptrBuffer = SSPx->DR;
	ptrBuffer++;
  }
  return i;
}

void spiMMCChipSelect (BOOL select)
{
  if (select) {
	SSPx->CPSR = SPI_MMC_SPEED;	
	CS_Set(0);
  }
  else
  {
	CS_Set(1);
    while (!(SSPx->SR & SSP_SR_TNF));
    SSPx->DR = 0xff;
    //
    // Wait until TX fifo and TX shift buffer are empty
    //
    while (SSPx->SR & SSP_SR_BSY);
    while (!(SSPx->SR & SSP_SR_RNE));
    do
    {
      select = SSPx->DR;
    } while (SSPx->SR & SSP_SR_RNE);
  }
}

void spiSendByte (U8 c)
{
  while (!(SSPx->SR & SSP_SR_TNF));
  SSPx->DR = c;
  while (!(SSPx->SR & SSP_SR_RNE));
  c = SSPx->DR;
}

U8 spiReceiveByte (void)
{
  while (!(SSPx->SR & SSP_SR_TNF));
  SSPx->DR = 0xFF;
  while (!(SSPx->SR & SSP_SR_RNE));
  return SSPx->DR;
}

void spiReceivePtr (U8 *c)
{
  while (!(SSPx->SR & SSP_SR_TNF));
  SSPx->DR = 0xFF;
  while (!(SSPx->SR & SSP_SR_RNE));
  *c = SSPx->DR;
}
