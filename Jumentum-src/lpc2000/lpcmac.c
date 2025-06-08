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

// MAC driver for LPC2300 series microcontrollers

#ifdef LPCMAC

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "all.h"
#include "lpc210x.h"
#include "lib.h"
#include "uip/uip/uip.h"
#include "lpcmac.h"

#undef LM_DEBUG

#define LM_IP_HEADER_LENGTH 54

#define LM_MCMD            (*(pREG32 (0xffe00024)))
#define LM_MCMD_READ       0x01
#define LM_MADR            (*(pREG32 (0xffe00028)))
#define LM_MWTD            (*(pREG32 (0xffe0002c)))
#define LM_MRDD            (*(pREG32 (0xffe00030)))
#define LM_MIND            (*(pREG32 (0xffe00034)))
#define LM_MIND_BUSY       0x01

#define LM_MODULEID               (*(pREG32 (0xffe00ffc)))
#define LM_MODULEID_OLD           ((0x3902 << 16) | 0x2000)

#define LM_MAC1                   (*(pREG32 (0xffe00000)))
#define LM_MAC1_INIT              0x2
#define LM_MAC1_ENABLE_RECEIVE    0x1
// enable pass all bit
#define LM_MAC1_RESET             0xCF00
// reset TX,RX,mac control sublayer, simulation reset, soft reset
#define LM_MAC2                   (*(pREG32 (0xffe00004)))
#define LM_MAC2_INIT              0x30
// enable CRC generation and automatic padding
#define LM_MAC2_FULL_DUP          0x01
#define LM_COMMAND                (*(pREG32 (0xffe00100)))
#define LM_COMMAND_INIT           0x240
// pass runt frame, and enable reduced MII interface
#define LM_COMMAND_FULL_DUP       0x400
#define LM_COMMAND_ENABLE_RECEIVE 0x03
#define LM_COMMAND_RESET          0x38
#define LM_STATUS          (*(pREG32 (0xffe00104)))
#define LM_MCFG_RESET      0x8000
#define LM_MAXFRAMESIZE    (*(pREG32 (0xffe00014)))
#define LM_ETH_FRAME_SIZE  1536
#define LM_CLRT            (*(pREG32 (0xffe00010)))
#define LM_CLRT_INIT       0x370F
#define LM_IPGR            (*(pREG32 (0xffe0000c)))
#define LM_IPGR_INIT       0x12
#define LM_IPGT            (*(pREG32 (0xffe00008)))
#define LM_IPGT_FULL_DUP   0x15
#define LM_IPGT_HALF_DUP   0x12
#define LM_MCFG            (*(pREG32 (0xffe00020)))
#define LM_MCFG_RESET      0x8000
#define LM_MCFG_INIT       0x1C 
// divide host clock by 28
#define LM_SUPP            (*(pREG32 (0xffe00018)))
#define LM_SUPP_100MBPS    0x100
// reduced MII logic speed

#define LM_RXFILTERCTRL    (*(pREG32 (0xffe00200)))
#define LM_RXFILTERCTRL_INIT    0x23
// initialize receiving unicast, broadcast, and station address matching frames
#define LM_SA0             (*(pREG32 (0xffe00040)))
#define LM_SA1             (*(pREG32 (0xffe00044)))
#define LM_SA2             (*(pREG32 (0xffe00048)))
#define LM_INTENABLE       (*(pREG32 (0xffe00fe4)))
#define LM_INTCLEAR        (*(pREG32 (0xffe00fe8)))

#define LM_TXDESCRIPTORNUM (*(pREG32 (0xffe00124)))
#define LM_TXDESCRIPTOR    (*(pREG32 (0xffe0011c)))
#define LM_TXSTATUS        (*(pREG32 (0xffe00120)))
#define LM_TXPRODUCEINDEX  (*(pREG32 (0xffe00128)))
#define LM_TXCONSUMEINDEX  (*(pREG32 (0xffe0012c)))

#define LM_RXDESCRIPTORNUM (*(pREG32 (0xffe00110)))
#define LM_RXDESCRIPTOR    (*(pREG32 (0xffe00108)))
#define LM_RXSTATUS        (*(pREG32 (0xffe0010c)))
#define LM_RXPRODUCEINDEX  (*(pREG32 (0xffe00114)))
#define LM_RXCONSUMEINDEX  (*(pREG32 (0xffe00118)))

#define LM_PHY_BCR              0x00
#define LM_PHY_BCR_AUTONEG      0x3000
#define LM_PHY_BSR              0x01
#define LM_PHY_ID1              0x02
#define LM_PHY_ID2              0x03
#define LM_PHY_ESTAT            0x1F
#define LM_PHY_ESTAT_MASK       0x1C
#define LM_PHY_ESTAT_10HALF     0x04
#define LM_PHY_ESTAT_100HALF    0x08
#define LM_PHY_ESTAT_10FULL     0x14
#define LM_PHY_ESTAT_100FULL    0x18
#define LM_PHY_STS              0x10
#define LM_PHY_STS_100MBPS      0x02
#define LM_PHY_STS_FULLDUP      0x04

#define DEFAULT_PHY_ADR 0x0100
#define KS8721_ID 2233872
#define DP83848_ID 536894608

static unsigned short phyadr = DEFAULT_PHY_ADR;

void send_phy_command(int reg, int data)
{
	int i = 0;

	LM_MADR = phyadr | reg;
	LM_MCMD = 0;
	LM_MWTD = data;
	while ((++i < 100000) && ((LM_MIND & LM_MIND_BUSY) != 0));
}

unsigned short read_phy_data(int reg)
{
	int i = 0;
	LM_MADR = phyadr | reg;
	LM_MCMD = LM_MCMD_READ;
	while ((++i < 100000) && ((LM_MIND & LM_MIND_BUSY) != 0));
	LM_MCMD = 0;
	return (LM_MRDD);
}

void find_phy_address(int reg)
{
	for (phyadr=0;phyadr<(32*256);phyadr+=256) {
		if (read_phy_data(reg) != 0xFFFF)
			return;
	}
	phyadr = DEFAULT_PHY_ADR;
}


static void delay(unsigned long dly)
{
  unsigned long i;
  dly *= 3000;
  for (i=0;i<dly;i++) {
    GPIO0_IOSET = 0;
  }
}

int phy_configure(void)
{
  int i,j;
  unsigned long phy_id, dat, bps100=1, fulldup=1;

#ifdef LM_DEBUG
  deboutstrhex("finding phy address=",0);
#endif
  
  find_phy_address (LM_PHY_BCR);

#ifdef LM_DEBUG
  deboutstrhex("find phy address=",phyadr);
#endif

  // Reset the PHY
  i = 10;
  do {
    send_phy_command (LM_PHY_BCR, 0x8000);
	delay(10);
	if ((read_phy_data(LM_PHY_BCR) & 0x8000) == 0)
		break;
  } while ((--i)>0);
  if (i == 0) return -1;

#ifdef LM_DEBUG
  deboutstrhex("initialized phy=",0);
#endif

  // read out the PHY ID
  phy_id = (((unsigned long)read_phy_data(LM_PHY_ID1)) << 16) | (read_phy_data(LM_PHY_ID2) & 0xFFF0);

#ifdef LM_DEBUG
  deboutstrint("phy_id=",phy_id);
#endif

  // autonegotiate link
  j = 5;
  do {
	send_phy_command (LM_PHY_BCR, LM_PHY_BCR_AUTONEG);
	i = 20;
	do {
		delay(100);
		if ((read_phy_data (LM_PHY_BSR) & 0x0020) != 0) {
			j=1;
			break;
		}
	} while ((--i)>0);
  } while ((--j)>0);
  if (i == 0) return -1;

#ifdef LM_DEBUG
  deboutstrhex("autonegotiate=",0);
#endif
  
  // check if the link is on
  i = 50;
  do {
     delay(100);
	 if ((read_phy_data (LM_PHY_BSR) & 0x0004) != 0)
		break;
  } while ((--i)>0);
  if (i == 0) return -1;

#ifdef LM_DEBUG
  deboutstrhex("link up=",0);
#endif

  if (phy_id == KS8721_ID) {	
	dat = read_phy_data (LM_PHY_ESTAT) & LM_PHY_ESTAT_MASK;
	bps100=((dat == LM_PHY_ESTAT_100HALF) || (dat == LM_PHY_ESTAT_100FULL));
	fulldup=((dat == LM_PHY_ESTAT_10FULL) || (dat == LM_PHY_ESTAT_100FULL)); 
  } else if (phy_id == DP83848_ID) {
	dat = read_phy_data (LM_PHY_STS);
#ifdef LM_DEBUG	
	deboutstrhex("LM_PHY_STS=",dat);
#endif
	bps100=!(dat & LM_PHY_STS_100MBPS);
	fulldup=(dat & LM_PHY_STS_FULLDUP);
  }
  
#ifdef LM_DEBUG
  deboutstrhex("bps100=",bps100);
  deboutstrhex("fulldup=",fulldup);
#endif
  if (bps100) {
		LM_SUPP     = LM_SUPP_100MBPS;
	} else {
		LM_SUPP     = 0;
	}
  if (fulldup) {
		LM_IPGT     = LM_IPGT_FULL_DUP;
		LM_MAC2    |= LM_MAC2_FULL_DUP;
		LM_COMMAND |= LM_COMMAND_FULL_DUP;
	} else {
		LM_IPGT     = LM_IPGT_HALF_DUP;
	}
  return 0;
}

#define LM_RAMSTART      0x7fe00000
#define LM_RX_QUEUE   4
#define LM_TX_QUEUE   2

struct tx_descriptor {
   volatile unsigned long packet, ctrl;
}; 
struct tx_status {
   volatile unsigned long stat;
};
struct rx_descriptor {
   volatile unsigned long packet, ctrl;
};
struct rx_status {
   volatile unsigned long statusinfo, crc;
};
struct eth_frame {
   char frame[LM_ETH_FRAME_SIZE+16];
};

struct eth_memory {
   struct tx_descriptor tx_descriptor[LM_TX_QUEUE];
   struct tx_status     tx_status[LM_TX_QUEUE];
   struct rx_descriptor rx_descriptor[LM_RX_QUEUE];
   struct rx_status     rx_status[LM_RX_QUEUE];
   struct eth_frame     tx_packet_buf[LM_TX_QUEUE];
   struct eth_frame     rx_packet_buf[LM_RX_QUEUE];
};

#define ETHSPC ((struct eth_memory *)LM_RAMSTART)

int initMAC(void)
{
  int i,rval;

#ifdef LM_DEBUG
  deboutstrhex("init begin=",0);
#endif

  // power up ethernet controller
  SCB_PCONP |=  SCB_PCONP_ETH;
  delay( 10 );

  // enable external ethernet pins to PHY (with errata fix)
  PCB_PINSEL2 = (LM_MODULEID == LM_MODULEID_OLD) ? 0x50151105 : 0x50150105;
  PCB_PINSEL3 = (PCB_PINSEL3 & ~0xF) | 0x5;

#ifdef LM_DEBUG
  deboutstrhex("enable pins=",0);
#endif
  
  // reset the ethernet controller
  LM_MAC1 =    LM_MAC1_RESET;
  LM_COMMAND = LM_COMMAND_RESET;
  LM_MCFG =    LM_MCFG_RESET;

  // wait for reset to occur
  delay( 10 );

  // set default ethernet configuration
  LM_MAC1 = LM_MAC1_INIT;
  LM_COMMAND = LM_COMMAND_INIT;
  LM_MCFG = LM_MCFG_INIT; 
  LM_CLRT = LM_CLRT_INIT;
  LM_IPGR = LM_IPGR_INIT;
  LM_MAC2 = LM_MAC2_INIT;
  LM_RXFILTERCTRL = LM_RXFILTERCTRL_INIT;
  LM_SUPP = 0;
  LM_MAXFRAMESIZE = LM_ETH_FRAME_SIZE;

#ifdef LM_DEBUG
  deboutstrhex("init ethernet=",0);
#endif
  
  delay( 10 );

  // configure pointers into ethernet RAM
  for (i=0;i<LM_TX_QUEUE;i++) {
	ETHSPC->tx_descriptor[i].packet = (unsigned long)&(ETHSPC->tx_packet_buf[i]);
	ETHSPC->tx_descriptor[i].ctrl = 0;
	ETHSPC->tx_status[i].stat = 0;
  }
#ifdef LM_DEBUG
  deboutstrhex("tx descriptors=",0);
#endif
  for (i=0;i<LM_RX_QUEUE;i++) {
	ETHSPC->rx_descriptor[i].packet = (unsigned long)&(ETHSPC->rx_packet_buf[i]);
	ETHSPC->rx_descriptor[i].ctrl = 0x80000000 | (LM_ETH_FRAME_SIZE-1);
	ETHSPC->rx_status[i].statusinfo = ETHSPC->rx_status[i].crc = 0;
  }
#ifdef LM_DEBUG
  deboutstrhex("rx descriptors=",0);
#endif

  LM_TXDESCRIPTORNUM = LM_TX_QUEUE-1;
  LM_TXDESCRIPTOR    = (unsigned long) ETHSPC->tx_descriptor;
  LM_TXSTATUS        = (unsigned long) ETHSPC->tx_status;

  LM_RXDESCRIPTORNUM = LM_RX_QUEUE-1;
  LM_RXDESCRIPTOR    = (unsigned long) ETHSPC->rx_descriptor;
  LM_RXSTATUS        = (unsigned long) ETHSPC->rx_status;

  LM_TXPRODUCEINDEX = 0;
  LM_RXCONSUMEINDEX = 0;

#ifdef LM_DEBUG
  deboutstrhex("indices=",0);
#endif

  // configure the MAC address
  LM_SA0 = (((unsigned short)uip_ethaddr.addr[0]) << 8) | ((unsigned short)uip_ethaddr.addr[1]);
  LM_SA1 = (((unsigned short)uip_ethaddr.addr[2]) << 8) | ((unsigned short)uip_ethaddr.addr[3]);
  LM_SA2 = (((unsigned short)uip_ethaddr.addr[4]) << 8) | ((unsigned short)uip_ethaddr.addr[5]);
  
  // configure the PHY
  rval = phy_configure();

  // reset and disable interrupts
  LM_INTENABLE = 0;
  LM_INTCLEAR  = 0x3FFF;
 
  // enable reception and transmission of packets
  LM_COMMAND  |= LM_COMMAND_ENABLE_RECEIVE;
  LM_MAC1     |= LM_MAC1_ENABLE_RECEIVE;

#ifdef LM_DEBUG
  deboutstrhex("end init=",0);
#endif

  return rval;
}

u16_t MACWrite(void)
{
	int tx_queue_index;
	unsigned char *txbuf;
	
	tx_queue_index = LM_TXPRODUCEINDEX;
	txbuf = (unsigned char*)(&(ETHSPC->tx_packet_buf[tx_queue_index]));
    memcpy(txbuf, uip_buf, LM_IP_HEADER_LENGTH);
    if( uip_len > LM_IP_HEADER_LENGTH )
        memcpy(txbuf + LM_IP_HEADER_LENGTH, uip_appdata, uip_len - LM_IP_HEADER_LENGTH);
	// transmit size is encoded length minus one
	ETHSPC->tx_descriptor[tx_queue_index].ctrl = 0x40000000 | (uip_len-1);
	tx_queue_index++;
	if (tx_queue_index == LM_TX_QUEUE) tx_queue_index = 0;
	LM_TXPRODUCEINDEX = tx_queue_index;
	return TRUE;
}

u16_t MACRead(void)
{
	unsigned int packet_len = 0;
	int rx_queue_index;

    if (LM_RXPRODUCEINDEX != LM_RXCONSUMEINDEX)
    {
	   rx_queue_index = LM_RXCONSUMEINDEX;
	   // length is encoded minus one, plus we subtract four ethernet CRC bytes
	   // to get -3 to length of packet payload plus frame header
	   packet_len = (ETHSPC->rx_status[rx_queue_index].statusinfo & 0x7FF) - 3;
	   if (packet_len > UIP_BUFSIZE) packet_len = UIP_BUFSIZE;
	   memcpy(uip_buf,(void *)(&(ETHSPC->rx_packet_buf[rx_queue_index])),packet_len);
	   rx_queue_index++;
	   if (rx_queue_index == LM_RX_QUEUE) rx_queue_index = 0;
	   LM_RXCONSUMEINDEX = rx_queue_index;
	}
	return packet_len;
}

#endif  /* LPCMAC */
