/*
	LPCUSB, an USB device driver for LPC microcontrollers	
	Copyright (C) 2006 Bertrik Sikken (bertrik@sikken.nl)

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright
	   notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.
	3. The name of the author may not be used to endorse or promote products
	   derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
	IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/**
	Hardware definitions for the LPC214x USB controller

	These are private to the usbhw module
*/

#if defined(LPC214x) || defined(LPC2106) || defined(LPC2119)
/* Common LPC2148 definitions, related to USB */
#define	PCONP			*(volatile unsigned int *)0xE01FC0C4
#define	PLL1CON			*(volatile unsigned int *)0xE01FC0A0
#define	PLL1CFG			*(volatile unsigned int *)0xE01FC0A4
#define	PLL1STAT		*(volatile unsigned int *)0xE01FC0A8
#define	PLL1FEED		*(volatile unsigned int *)0xE01FC0AC

/* USB register definitions */
#define USBIntSt		*(volatile unsigned int *)0xE01FC1C0

#define USBDevIntSt		*(volatile unsigned int *)0xE0090000
#define USBDevIntEn		*(volatile unsigned int *)0xE0090004
#define USBDevIntClr	*(volatile unsigned int *)0xE0090008
#define USBDevIntSet	*(volatile unsigned int *)0xE009000C
#define USBDevIntPri	*(volatile unsigned int *)0xE009002C

#define USBEpIntSt		*(volatile unsigned int *)0xE0090030
#define USBEpIntEn		*(volatile unsigned int *)0xE0090034
#define USBEpIntClr		*(volatile unsigned int *)0xE0090038
#define USBEpIntSet		*(volatile unsigned int *)0xE009003C
#define USBEpIntPri		*(volatile unsigned int *)0xE0090040

#define USBReEP			*(volatile unsigned int *)0xE0090044
#define USBEpInd		*(volatile unsigned int *)0xE0090048
#define USBMaxPSize		*(volatile unsigned int *)0xE009004C

#define USBRxData		*(volatile unsigned int *)0xE0090018
#define USBRxPLen		*(volatile unsigned int *)0xE0090020
#define USBTxData		*(volatile unsigned int *)0xE009001C
#define USBTxPLen		*(volatile unsigned int *)0xE0090024
#define USBCtrl			*(volatile unsigned int *)0xE0090028

#define USBCmdCode		*(volatile unsigned int *)0xE0090010
#define USBCmdData		*(volatile unsigned int *)0xE0090014
#endif

#ifdef LPC23xx

#define	PCONP			*(volatile unsigned int *)0xE01FC0C4

#ifndef SCB_BASE_ADDR
#define SCB_BASE_ADDR	   0xE01FC000
#endif
#ifndef USB_BASE_ADDR
#define USB_BASE_ADDR      0xFFE0C000
#endif

/* USB Controller */
#define USB_BASE_ADDR      0xFFE0C000

/* USBPortSel only available on the LPC2378 */
#define USBPortSel         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x110))

/* USB Clock Control Registers */
#define USBClkCtrl         (*(volatile unsigned int *)(USB_BASE_ADDR + 0xFF4))
#define USBClkSt           (*(volatile unsigned int *)(USB_BASE_ADDR + 0xFF8))
#define USBCLKCFG          (*(volatile unsigned int *)(SCB_BASE_ADDR + 0x108))

/* USB Device Interrupt Registers */
#define USBIntSt           (*(volatile unsigned int *)(USB_BASE_ADDR + 0x1C0))
#define USBDevIntSt        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x200))
#define USBDevIntEn        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x204))
#define USBDevIntClr       (*(volatile unsigned int *)(USB_BASE_ADDR + 0x208))
#define USBDevIntSet       (*(volatile unsigned int *)(USB_BASE_ADDR + 0x20C))
#define USBDevIntPri       (*(volatile unsigned int *)(USB_BASE_ADDR + 0x22C))

/* USB Device Endpoint Interrupt Registers */
#define USBEpIntSt         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x230))
#define USBEpIntEn         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x234))
#define USBEpIntClr        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x238))
#define USBEpIntSet        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x23C))
#define USBEpIntPri        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x240))

/* USB Device Endpoint Realization Registers */
#define USBReEP            (*(volatile unsigned int *)(USB_BASE_ADDR + 0x244))
#define USBEpInd           (*(volatile unsigned int *)(USB_BASE_ADDR + 0x248))
#define USBMaxPSize        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x24C))

/* USB Device Data Transfer Registers */
#define USBRxData          (*(volatile unsigned int *)(USB_BASE_ADDR + 0x218))
#define USBRxPLen          (*(volatile unsigned int *)(USB_BASE_ADDR + 0x220))
#define USBTxData          (*(volatile unsigned int *)(USB_BASE_ADDR + 0x21C))
#define USBTxPLen          (*(volatile unsigned int *)(USB_BASE_ADDR + 0x224))
#define USBCtrl            (*(volatile unsigned int *)(USB_BASE_ADDR + 0x228))

/* USB SIE Command Reagisters */
#define USBCmdCode         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x210))
#define USBCmdData         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x214))

/* USB Device DMA Registers */
#define USBDMARSt          (*(volatile unsigned int *)(USB_BASE_ADDR + 0x250))
#define USBDMARClr         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x254))
#define USBDMARSet         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x258))
#define USBUDCAH           (*(volatile unsigned int *)(USB_BASE_ADDR + 0x280))
#define USBEpDMASt         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x284))
#define USBEpDMAEn         (*(volatile unsigned int *)(USB_BASE_ADDR + 0x288))
#define USBEpDMADis        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x28C))
#define USBDMAIntSt        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x290))
#define USBDMAIntEn        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x294))
#define USBEoTIntSt        (*(volatile unsigned int *)(USB_BASE_ADDR + 0x2A0))
#define USBEoTIntClr       (*(volatile unsigned int *)(USB_BASE_ADDR + 0x2A4))
#define USBEoTIntSet       (*(volatile unsigned int *)(USB_BASE_ADDR + 0x2A8))
#define USBNDDRIntSt       (*(volatile unsigned int *)(USB_BASE_ADDR + 0x2AC))
#define USBNDDRIntClr      (*(volatile unsigned int *)(USB_BASE_ADDR + 0x2B0))
#define USBNDDRIntSet      (*(volatile unsigned int *)(USB_BASE_ADDR + 0x2B4))
#define USBSysErrIntSt     (*(volatile unsigned int *)(USB_BASE_ADDR + 0x2B8))
#define USBSysErrIntClr    (*(volatile unsigned int *)(USB_BASE_ADDR + 0x2BC))
#define USBSysErrIntSet    (*(volatile unsigned int *)(USB_BASE_ADDR + 0x2C0))

#endif

#ifdef LPC1768
/* USB register definitions */
#define	PCONP			(LPC_SC->PCONP)

#define PINSEL0			(LPC_PINCON->PINSEL0)
#define PINSEL1			(LPC_PINCON->PINSEL1)
#define PINSEL2			(LPC_PINCON->PINSEL2)
#define PINSEL3			(LPC_PINCON->PINSEL3)
#define PINSEL4			(LPC_PINCON->PINSEL4)

#define USBIntSt		(LPC_USB->USBIntSt)

#define USBDevIntSt		(LPC_USB->USBDevIntSt)
#define USBDevIntEn		(LPC_USB->USBDevIntEn)
#define USBDevIntClr	(LPC_USB->USBDevIntClr)
#define USBDevIntSet	(LPC_USB->USBDevIntSet)
#define USBDevIntPri	(LPC_USB->USBDevIntPri)

#define USBEpIntSt		(LPC_USB->USBEpIntSt)
#define USBEpIntEn		(LPC_USB->USBEpIntEn)
#define USBEpIntClr		(LPC_USB->USBEpIntClr)
#define USBEpIntSet		(LPC_USB->USBEpIntSet)
#define USBEpIntPri		(LPC_USB->USBEpIntPri)

#define USBReEP			(LPC_USB->USBReEp)
#define USBEpInd		(LPC_USB->USBEpInd)
#define USBMaxPSize		(LPC_USB->USBMaxPSize)

#define USBRxData		(LPC_USB->USBRxData)
#define USBRxPLen		(LPC_USB->USBRxPLen)
#define USBTxData		(LPC_USB->USBTxData)
#define USBTxPLen		(LPC_USB->USBTxPLen)
#define USBCtrl			(LPC_USB->USBCtrl)

#define USBCmdCode		(LPC_USB->USBCmdCode)
#define USBCmdData		(LPC_USB->USBCmdData)
#define USBClkCtrl      (LPC_USB->USBClkCtrl)
#define USBClkSt        (LPC_USB->USBClkSt)
#endif

/* USBIntSt bits */
#define USB_INT_REQ_LP				(1<<0)
#define USB_INT_REQ_HP				(1<<1)
#define USB_INT_REQ_DMA				(1<<2)
#define USB_need_clock				(1<<8)
#define EN_USB_BITS					(1<<31)

/* USBDevInt... bits */
#define FRAME						(1<<0)
#define EP_FAST						(1<<1)
#define EP_SLOW						(1<<2)
#define DEV_STAT					(1<<3)
#define CCEMTY						(1<<4)
#define CDFULL						(1<<5)
#define RxENDPKT					(1<<6)
#define TxENDPKT					(1<<7)
#define EP_RLZED					(1<<8)
#define ERR_INT						(1<<9)

/* USBRxPLen bits */
#define PKT_LNGTH					(1<<0)
#define PKT_LNGTH_MASK				0x3FF
#define DV							(1<<10)
#define PKT_RDY						(1<<11)

/* USBCtrl bits */
#define RD_EN						(1<<0)
#define WR_EN						(1<<1)
#define LOG_ENDPOINT				(1<<2)

/* protocol engine command codes */
	/* device commands */
#define CMD_DEV_SET_ADDRESS			0xD0
#define CMD_DEV_CONFIG				0xD8
#define CMD_DEV_SET_MODE			0xF3
#define CMD_DEV_READ_CUR_FRAME_NR	0xF5
#define CMD_DEV_READ_TEST_REG		0xFD
#define CMD_DEV_STATUS				0xFE		/* read/write */
#define CMD_DEV_GET_ERROR_CODE		0xFF
#define CMD_DEV_READ_ERROR_STATUS	0xFB
	/* endpoint commands */
#define CMD_EP_SELECT				0x00
#define CMD_EP_SELECT_CLEAR			0x40
#define CMD_EP_SET_STATUS			0x40
#define CMD_EP_CLEAR_BUFFER			0xF2
#define CMD_EP_VALIDATE_BUFFER		0xFA

/* set address command */
#define DEV_ADDR					(1<<0)
#define DEV_EN						(1<<7)

/* configure device command */
#define CONF_DEVICE					(1<<0)

/* set mode command */
#define AP_CLK						(1<<0)
#define INAK_CI						(1<<1)
#define INAK_CO						(1<<2)
#define INAK_II						(1<<3)
#define INAK_IO						(1<<4)
#define INAK_BI						(1<<5)
#define INAK_BO						(1<<6)

/* set get device status command */
#define CON							(1<<0)
#define CON_CH						(1<<1)
#define SUS							(1<<2)
#define SUS_CH						(1<<3)
#define RST							(1<<4)

/* get error code command */
// ...

/* Select Endpoint command read bits */
#define EPSTAT_FE					(1<<0)
#define EPSTAT_ST					(1<<1)
#define EPSTAT_STP					(1<<2)
#define EPSTAT_PO					(1<<3)
#define EPSTAT_EPN					(1<<4)
#define EPSTAT_B1FULL				(1<<5)
#define EPSTAT_B2FULL				(1<<6)

/* CMD_EP_SET_STATUS command */
#define EP_ST						(1<<0)
#define EP_DA						(1<<5)
#define EP_RF_MO					(1<<6)
#define EP_CND_ST					(1<<7)

/* read error status command */
#define PID_ERR						(1<<0)
#define UEPKT						(1<<1)
#define DCRC						(1<<2)
#define TIMEOUT						(1<<3)
#define EOP							(1<<4)
#define B_OVRN						(1<<5)
#define BTSTF						(1<<6)
#define TGL_ERR						(1<<7)








