#ifdef LPCMAC

#ifndef LPCMAC_H
#define LPCMAC_H

#define EMAC_MAX_PACKET_SIZE (UIP_CONF_BUFFER_SIZE + 16)	// 1536 bytes

#define ENET_DMA_DESC_NUMB   	3
#define AUTO_NEGOTIATION_ENA 	1  		// Enable PHY Auto-negotiation
#define PHY_TO               	200000  // ~10sec
#define RMII					1		// If zero, it's a MII interface

#include "../netdev.h"

#endif /* LPCMAC_H */

#endif  /* LPCMAC */
