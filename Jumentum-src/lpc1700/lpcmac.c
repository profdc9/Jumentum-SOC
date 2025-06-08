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

#ifdef LPCMAC

#include "libdriver/lpc17xx_pinsel.h"
#include "libdriver/lpc17xx_emac.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "all.h"
#include "lib.h"
#include "uip/uip/uip.h"
#include "lpcmac.h"

int initMAC(void)
{
	/* EMAC configuration type */
	EMAC_CFG_Type Emac_Config;
	/* pin configuration */
	PINSEL_CFG_Type PinCfg;
	/* EMAC address */

	uint8_t *EMACAddr = uip_ethaddr.addr;
  
#if AUTO_NEGOTIATION_ENA != 0
	Emac_Config.Mode = EMAC_MODE_AUTO;
#else
	#if (FIX_SPEED == SPEED_100)
		#if (FIX_DUPLEX == FULL_DUPLEX)
			Emac_Config.Mode = EMAC_MODE_100M_FULL;
		#elif (FIX_DUPLEX == HALF_DUPLEX)
			Emac_Config.Mode = EMAC_MODE_100M_HALF;
		#else
			#error Does not support this duplex option
		#endif
	#elif (FIX_SPEED == SPEED_10)
		#if (FIX_DUPLEX == FULL_DUPLEX)
				Emac_Config.Mode = EMAC_MODE_10M_FULL;
		#elif (FIX_DUPLEX == HALF_DUPLEX)
				Emac_Config.Mode = EMAC_MODE_10M_HALF;
		#else
			#error Does not support this duplex option
		#endif
	#else
		#error Does not support this speed option
	#endif
#endif

	/*
	 * Enable P1 Ethernet Pins:
	 * P1.0 - ENET_TXD0
	 * P1.1 - ENET_TXD1
	 * P1.4 - ENET_TX_EN
	 * P1.8 - ENET_CRS
	 * P1.9 - ENET_RXD0
	 * P1.10 - ENET_RXD1
	 * P1.14 - ENET_RX_ER
	 * P1.15 - ENET_REF_CLK
	 * P1.16 - ENET_MDC
	 * P1.17 - ENET_MDIO
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 1;

	PinCfg.Pinnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 1;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 4;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 10;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 14;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 15;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 16;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 17;
	PINSEL_ConfigPin(&PinCfg);


	Emac_Config.Mode = EMAC_MODE_AUTO;
	Emac_Config.pbEMAC_Addr = EMACAddr;
	// Initialize EMAC module with given parameter
	if (EMAC_Init(&Emac_Config) == ERROR){
		return (FALSE);
	}

	return (TRUE);
}

u16_t MACWrite(void)
{
	EMAC_PACKETBUF_Type TxPack;
	UNS_32 size = uip_len;
	
	// Check size
	if(size == 0){
		return(TRUE);
	}

	// check Tx Slot is available
	if (EMAC_CheckTransmitIndex() == FALSE){
		return (FALSE);
	}

	size = MIN(size,EMAC_MAX_PACKET_SIZE);

	// Setup Tx Packet buffer
	TxPack.ulDataLen = size;
	TxPack.pbDataBuf = (uint32_t *)uip_buf;
	EMAC_WritePacketBuffer(&TxPack);
	EMAC_UpdateTxProduceIndex();

	return(TRUE);
}

u16_t MACRead(void)
{
	UNS_32 Size = EMAC_MAX_PACKET_SIZE;
	UNS_32 in_size;
	EMAC_PACKETBUF_Type RxPack;

	// Check Receive status
	if (EMAC_CheckReceiveIndex() == FALSE){
		return (0);
	}

	// Get size of receive data
	in_size = EMAC_GetReceiveDataSize() + 1;

	Size = MIN(Size,in_size);

	// Setup Rx packet
	RxPack.pbDataBuf = (uint32_t *)uip_buf;
	RxPack.ulDataLen = Size;
	EMAC_ReadPacketBuffer(&RxPack);

	// update receive status
	EMAC_UpdateRxConsumeIndex();
	return(Size);
}

#endif  /* LPCMAC */
