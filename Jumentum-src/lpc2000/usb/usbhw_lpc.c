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


/** @file
	USB hardware layer
 */

#ifdef LPC2148
#define LPC214x
#endif

#ifdef LPC2378
#define LPC23xx
#endif 
 
#if defined LPC214x || defined LPC23xx
#include "../lpc210x.h"
#endif

#ifdef LPC1768
#include "../LPC17xx.h"
#endif

#include "type.h"
#include "usbdebug.h"
#include "usbhw_lpc.h"
#include "usbapi.h"


#undef DEBUG
#ifdef DEBUG
// comment out the following line if you don't want to use debug LEDs
#define DEBUG_LED
#endif

#ifdef DEBUG_LED
#if defined LPC2148 || defined LPC2378
#define DEBUG_LED_ON(x)		IOCLR0 = (1 << x);
#define DEBUG_LED_OFF(x)	IOSET0 = (1 << x);
#define DEBUG_LED_INIT(x)	PINSEL0 &= ~(0x3 << (2*x)); IODIR0 |= (1 << x); DEBUG_LED_OFF(x);
#endif
#ifdef LPC1768
#define DEBUG_LED_ON(x)		LPC_GPIO2->FIOCLR = (1 << x);
#define DEBUG_LED_OFF(x)	LPC_GPIO2->FIOSET = (1 << x);
#define DEBUG_LED_INIT(x)	LPC_PINCON->PINSEL4 &= ~(0x3 << (2*x)); LPC_GPIO2->FIODIR |= (1 << x); DEBUG_LED_OFF(x);
#endif
#else
#define DEBUG_LED_INIT(x)	/**< LED initialisation macro */
#define DEBUG_LED_ON(x)		/**< turn LED on */
#define DEBUG_LED_OFF(x)	/**< turn LED off */
#endif

/** Installed device interrupt handler */
static TFnDevIntHandler *_pfnDevIntHandler = NULL;
/** Installed endpoint interrupt handlers */
static TFnEPIntHandler	*_apfnEPIntHandlers[16];
/** Installed frame interrupt handlers */
static TFnFrameHandler	*_pfnFrameHandler = NULL;

/** convert from endpoint address to endpoint index */
#define EP2IDX(bEP)	((((bEP)&0xF)<<1)|(((bEP)&0x80)>>7))
/** convert from endpoint index to endpoint address */
#define IDX2EP(idx)	((((idx)<<7)&0x80)|(((idx)>>1)&0xF))



/**
	Local function to wait for a device interrupt (and clear it)
		
	@param [in]	dwIntr		Bitmask of interrupts to wait for	
 */
static void Wait4DevInt(U32 dwIntr)
{
	// wait for specific interrupt
	while ((USBDevIntSt & dwIntr) != dwIntr);
	// clear the interrupt bits
	USBDevIntClr = dwIntr;
}


/**
	Local function to send a command to the USB protocol engine
		
	@param [in]	bCmd		Command to send
 */
static void USBHwCmd(U8 bCmd)
{
	// clear CDFULL/CCEMTY
	USBDevIntClr = CDFULL | CCEMTY;
	// write command code
	USBCmdCode = 0x00000500 | (bCmd << 16);
	Wait4DevInt(CCEMTY);
}


/**
	Local function to send a command + data to the USB protocol engine
		
	@param [in]	bCmd		Command to send
	@param [in]	bData		Data to send
 */
static void USBHwCmdWrite(U8 bCmd, U16 bData)
{
	// write command code
	USBHwCmd(bCmd);

	// write command data
	USBCmdCode = 0x00000100 | (bData << 16);
	Wait4DevInt(CCEMTY);
}


/**
	Local function to send a command to the USB protocol engine and read data
		
	@param [in]	bCmd		Command to send

	@return the data
 */
static U8 USBHwCmdRead(U8 bCmd)
{
	// write command code
	USBHwCmd(bCmd);
	
	// get data
	USBCmdCode = 0x00000200 | (bCmd << 16);
	Wait4DevInt(CDFULL);
	return USBCmdData;
}


/**
	'Realizes' an endpoint, meaning that buffer space is reserved for
	it. An endpoint needs to be realised before it can be used.
		
	From experiments, it appears that a USB reset causes USBReEP to
	re-initialise to 3 (= just the control endpoints).
	However, a USB bus reset does not disturb the USBMaxPSize settings.
		
	@param [in]	idx			Endpoint index
	@param [in] wMaxPSize	Maximum packet size for this endpoint
 */
static void USBHwEPRealize(int idx, U16 wMaxPSize)
{
	USBReEP |= (1 << idx);
	USBEpInd = idx;
	USBMaxPSize = wMaxPSize;
	Wait4DevInt(EP_RLZED);
}


/**
	Enables or disables an endpoint
		
	@param [in]	idx		Endpoint index
	@param [in]	fEnable	TRUE to enable, FALSE to disable
 */
static void USBHwEPEnable(int idx, BOOL fEnable)
{
	USBHwCmdWrite(CMD_EP_SET_STATUS | idx, fEnable ? 0 : EP_DA);
}


/**
	Configures an endpoint and enables it
		
	@param [in]	bEP				Endpoint number
	@param [in]	wMaxPacketSize	Maximum packet size for this EP
 */
void USBHwEPConfig(U8 bEP, U16 wMaxPacketSize)
{
	int idx;
	
	idx = EP2IDX(bEP);
	
	// realise EP
	USBHwEPRealize(idx, wMaxPacketSize);

	// enable EP
	USBHwEPEnable(idx, TRUE);
}


/**
	Registers an endpoint event callback
		
	@param [in]	bEP				Endpoint number
	@param [in]	pfnHandler		Callback function
 */
void USBHwRegisterEPIntHandler(U8 bEP, TFnEPIntHandler *pfnHandler)
{
	int idx;
	
	idx = EP2IDX(bEP);

	ASSERT(idx<32);

	/* add handler to list of EP handlers */
	_apfnEPIntHandlers[idx / 2] = pfnHandler;
	
	/* enable EP interrupt */
	USBEpIntEn |= (1 << idx);
	USBDevIntEn |= EP_SLOW;
	
	DBG("Registered handler for EP 0x%x\n", bEP);
}


/**
	Registers an device status callback
		
	@param [in]	pfnHandler	Callback function
 */
void USBHwRegisterDevIntHandler(TFnDevIntHandler *pfnHandler)
{
	_pfnDevIntHandler = pfnHandler;
	
	// enable device interrupt
	USBDevIntEn |= DEV_STAT;

	DBG("Registered handler for device status\n");
}


/**
	Registers the frame callback
		
	@param [in]	pfnHandler	Callback function
 */
void USBHwRegisterFrameHandler(TFnFrameHandler *pfnHandler)
{
	_pfnFrameHandler = pfnHandler;
	
	// enable device interrupt
	USBDevIntEn |= FRAME;

	DBG("Registered handler for frame\n");
}


/**
	Sets the USB address.
		
	@param [in]	bAddr		Device address to set
 */
void USBHwSetAddress(U8 bAddr)
{
	USBHwCmdWrite(CMD_DEV_SET_ADDRESS, DEV_EN | bAddr);
}


/**
	Connects or disconnects from the USB bus
		
	@param [in]	fConnect	If TRUE, connect, otherwise disconnect
 */
void USBHwConnect(BOOL fConnect)
{
#ifdef LPC23xx
#ifndef LPC2378_PORTB
  if(fConnect)
    FIO2_CLR = (1<<9);
  else
    FIO2_SET = (1<<9);
#else
  if(fConnect)
    FIO0_CLR = (1<<14);
  else
    FIO0_SET = (1<<14);
#endif
#endif
	USBHwCmdWrite(CMD_DEV_STATUS, fConnect ? CON : 0);
}


/**
	Enables interrupt on NAK condition
		
	For IN endpoints a NAK is generated when the host wants to read data
	from the device, but none is available in the endpoint buffer.
	For OUT endpoints a NAK is generated when the host wants to write data
	to the device, but the endpoint buffer is still full.
	
	The endpoint interrupt handlers can distinguish regular (ACK) interrupts
	from NAK interrupt by checking the bits in their bEPStatus argument.
	
	@param [in]	bIntBits	Bitmap indicating which NAK interrupts to enable
 */
void USBHwNakIntEnable(U8 bIntBits)
{
	USBHwCmdWrite(CMD_DEV_SET_MODE, bIntBits);
}


/**
	Gets the status from a specific endpoint.
		
	@param [in]	bEP		Endpoint number
	@return Endpoint status byte (containing EP_STATUS_xxx bits)
 */
U8	USBHwEPGetStatus(U8 bEP)
{
	int idx = EP2IDX(bEP);

	return USBHwCmdRead(CMD_EP_SELECT | idx);
}


/**
	Sets the stalled property of an endpoint
		
	@param [in]	bEP		Endpoint number
	@param [in]	fStall	TRUE to stall, FALSE to unstall
 */
void USBHwEPStall(U8 bEP, BOOL fStall)
{
	int idx = EP2IDX(bEP);

	USBHwCmdWrite(CMD_EP_SET_STATUS | idx, fStall ? EP_ST : 0);
}


/**
	Writes data to an endpoint buffer
		
	@param [in]	bEP		Endpoint number
	@param [in]	pbBuf	Endpoint data
	@param [in]	iLen	Number of bytes to write
			
	@return the number of bytes written.
*/
int USBHwEPWrite(U8 bEP, U8 *pbBuf, int iLen)
{
	int idx;
	
	idx = EP2IDX(bEP);
	
	// set write enable for specific endpoint
	USBCtrl = WR_EN | ((bEP & 0xF) << 2);
	
	// set packet length
	USBTxPLen = iLen;
	
	// write data
	while (USBCtrl & WR_EN) {
		USBTxData = (pbBuf[3] << 24) | (pbBuf[2] << 16) | (pbBuf[1] << 8) | pbBuf[0];
		pbBuf += 4;
	}

    USBCtrl = 0;
	// select endpoint and validate buffer
	USBHwCmd(CMD_EP_SELECT | idx);
	USBHwCmd(CMD_EP_VALIDATE_BUFFER);
	
	return iLen;
}

/**
	Polls endpoint buffer if packet ready
		
	@param [in]	bEP		Endpoint number
			
	@return TRUE if packet ready, FALSE otherwise
 */
BOOL USBHwEPReadDataReady(U8 bEP)
{
	return (USBHwEPGetStatus(bEP) & EPSTAT_FE) != 0;
}

/**
	Reads data from an endpoint buffer
		
	@param [in]	bEP		Endpoint number
	@param [in]	pbBuf	Endpoint data
	@param [in]	iMaxLen	Maximum number of bytes to read
			
	@return the number of bytes available in the EP (possibly more than iMaxLen),
	or <0 in case of error.
 */
int USBHwEPRead(U8 bEP, U8 *pbBuf, int iMaxLen)
{
	int i, idx;
	U32	dwData, dwLen;
	
	idx = EP2IDX(bEP);
	
	// set read enable bit for specific endpoint
	USBCtrl = RD_EN | ((bEP & 0xF) << 2);
	
	// wait for PKT_RDY
	do {
		dwLen = USBRxPLen;
	} while ((dwLen & PKT_RDY) == 0);
	
	// packet valid?
	if ((dwLen & DV) == 0) {
		return -1;
	}
	
	// get length
	dwLen &= PKT_LNGTH_MASK;
	
	// get data
	dwData = 0;
	for (i = 0; i < dwLen; i++) {
		if ((i & 0x03) == 0) {
			dwData = USBRxData;
		}
		if ((pbBuf != NULL) && (i < iMaxLen)) {
			pbBuf[i] = dwData & 0xFF;
		}
		dwData >>= 8;
	}

	// make sure RD_EN is clear
	USBCtrl = 0;

	// select endpoint and clear buffer
	USBHwCmd(CMD_EP_SELECT | idx);
	USBHwCmd(CMD_EP_CLEAR_BUFFER);
	
	return dwLen;
}



int USBHwISOCEPRead(const U8 bEP, U8 *pbBuf, const int iMaxLen)
{
    int i, idx,q;
    U32 dwData, dwLen;

    idx = EP2IDX(bEP);

    // set read enable bit for specific endpoint
    USBCtrl = RD_EN | ((bEP & 0xF) << 2);
    
    //Note: for some reason the USB perepherial needs a cycle to set bits in USBRxPLen before 
    //reading, if you remove this ISOC wont work. This may be a but in the chip, or due to 
    //a mis-understanding of how the perepherial is supposed to work.    
    asm volatile("nop\n"); 

    
    dwLen = USBRxPLen;
    if( (dwLen & PKT_RDY) == 0 ) {
        USBCtrl = 0;// make sure RD_EN is clear
        return(-1);
    }

    // packet valid?
    if ((dwLen & DV) == 0) {
        USBCtrl = 0;// make sure RD_EN is clear
        return -1;
    }

    // get length
    dwLen &= PKT_LNGTH_MASK;

    // get data
    dwData = 0;
    for (i = 0; i < dwLen; i++) {
        if ((i & 0x03) == 0) {
            dwData = USBRxData;
        }
        if ((pbBuf != NULL) && (i < iMaxLen)) {
            pbBuf[i] = dwData & 0xFF;
        }
        dwData >>= 8;
    }

    // make sure RD_EN is clear
    USBCtrl = 0;

    // select endpoint and clear buffer
    USBHwCmd(CMD_EP_SELECT | idx);
    USBHwCmd(CMD_EP_CLEAR_BUFFER);

    return dwLen;
}

/**
	Sets the 'configured' state.
		
	All registered endpoints are 'realised' and enabled, and the
	'configured' bit is set in the device status register.
		
	@param [in]	fConfigured	If TRUE, configure device, else unconfigure
 */
void USBHwConfigDevice(BOOL fConfigured)
{
	// set configured bit
	USBHwCmdWrite(CMD_DEV_CONFIG, fConfigured ? CONF_DEVICE : 0);
}


/**
	USB interrupt handler
		
	@todo Get all 11 bits of frame number instead of just 8

	Endpoint interrupts are mapped to the slow interrupt
 */
#if defined LPC214x || defined LPC2106 || defined LPC2119 || defined LPC23xx 
void USBHwISR(void)
#endif
#ifdef LPC1768
void USB_IRQHandler(void)
#endif
{
	U32	dwStatus;
	U32 dwIntBit;
	U8	bEPStat, bDevStat, bStat;
	int i;
	U16	wFrame;

// LED9 monitors total time in interrupt routine
DEBUG_LED_ON(9);

	// handle device interrupts
	dwStatus = USBDevIntSt;
	
	// frame interrupt
	if (dwStatus & FRAME) {
		// clear int
		USBDevIntClr = FRAME;
		// call handler
		if (_pfnFrameHandler != NULL) {
			wFrame = USBHwCmdRead(CMD_DEV_READ_CUR_FRAME_NR);
			_pfnFrameHandler(wFrame);
		}
	}
	
	// device status interrupt
	if (dwStatus & DEV_STAT) {
		/*	Clear DEV_STAT interrupt before reading DEV_STAT register.
			This prevents corrupted device status reads, see
			LPC2148 User manual revision 2, 25 july 2006.
		*/
		USBDevIntClr = DEV_STAT;
		bDevStat = USBHwCmdRead(CMD_DEV_STATUS);
		if (bDevStat & (CON_CH | SUS_CH | RST)) {
			// convert device status into something HW independent
			bStat = ((bDevStat & CON) ? DEV_STATUS_CONNECT : 0) |
					((bDevStat & SUS) ? DEV_STATUS_SUSPEND : 0) |
					((bDevStat & RST) ? DEV_STATUS_RESET : 0);
			// call handler
			if (_pfnDevIntHandler != NULL) {
DEBUG_LED_ON(8);		
				_pfnDevIntHandler(bStat);
DEBUG_LED_OFF(8);		
			}
		}
	}
	
	// endpoint interrupt
	if (dwStatus & EP_SLOW) {
		// clear EP_SLOW
		USBDevIntClr = EP_SLOW;
		// check all endpoints
		for (i = 0; i < 32; i++) {
			dwIntBit = (1 << i);
			if (USBEpIntSt & dwIntBit) {
				// clear int (and retrieve status)
				USBEpIntClr = dwIntBit;
				Wait4DevInt(CDFULL);
				bEPStat = USBCmdData;
				// convert EP pipe stat into something HW independent
				bStat = ((bEPStat & EPSTAT_FE) ? EP_STATUS_DATA : 0) |
						((bEPStat & EPSTAT_ST) ? EP_STATUS_STALLED : 0) |
						((bEPStat & EPSTAT_STP) ? EP_STATUS_SETUP : 0) |
						((bEPStat & EPSTAT_EPN) ? EP_STATUS_NACKED : 0) |
						((bEPStat & EPSTAT_PO) ? EP_STATUS_ERROR : 0);
				// call handler
				if (_apfnEPIntHandlers[i / 2] != NULL) {
DEBUG_LED_ON(10);		
					_apfnEPIntHandlers[i / 2](IDX2EP(i), bStat);
DEBUG_LED_OFF(10);
				}
			}
		}
	}
	
DEBUG_LED_OFF(9);		
}



/**
	Initialises the USB hardware
		
	This function assumes that the hardware is connected as shown in
	section 10.1 of the LPC2148 data sheet:
	* P0.31 controls a switch to connect a 1.5k pull-up to D+ if low.
	* P0.23 is connected to USB VCC.
	
	Embedded artists board: make sure to disconnect P0.23 LED as it
	acts as a pull-up and so prevents detection of USB disconnect.
		
	@return TRUE if the hardware was successfully initialised
 */
BOOL USBHwInit(void)
{

	
#ifdef LPC2148
	// configure P0.23 for Vbus sense
	PINSEL1 = (PINSEL1 & ~(3 << 14)) | (1 << 14);	// P0.23
	// configure P0.31 for CONNECT
	PINSEL1 = (PINSEL1 & ~(3 << 30)) | (2 << 30);	// P0.31

	// enable PUSB
	PCONP |= (((unsigned long)1) << 31);		

    // initialise PLL
    PLL1CON = 1;            // enable PLL
    PLL1CFG = (1 << 5) | 3; // P = 2, M = 4
    PLL1FEED = 0xAA;
    PLL1FEED = 0x55;
    while ((PLL1STAT & (1 << 10)) == 0);

    PLL1CON = 3;            // enable and connect
    PLL1FEED = 0xAA;
    PLL1FEED = 0x55;
#endif

#ifdef LPC23xx
#ifdef LPC2378_PORTB
    PCB_PINSEL1 = (PCB_PINSEL1 & ~(3 << 30)) | (1 << 30);
    PCB_PINSEL3 = (PCB_PINSEL3 & ~(3 << 28)) | (2 << 28);
    /* Due to a bug in the LPC23xx chips, the connection functionality must be
    * simulated using GPIO. Hopefully for production this will be fixed and the
    * commented out code will work */
    //PINSEL0 = (PINSEL0 & ~((3 << 26) | (3 << 28))) | (1 << 26) | (1 << 28); /* Doesn't work due to bug in chip */
    PINSEL0 = (PINSEL0 & ~((3 << 26) | (3 << 28))) | (1 << 26);
    FIO0_DIR |= (1<<14); /* Set pin to output */
    FIO0_SET = (1<<14); /* Set output high to disconnect */
    
#else
    PCB_PINSEL1 = (PCB_PINSEL1 & ~((3 << 26) | (3 << 28))) | (1 << 26) | (1 << 28);
    PCB_PINSEL3 = (PCB_PINSEL3 & ~((3 << 4) | (3 << 28))) | (1 << 4) | (2 << 28);
    /* Due to a bug in the LPC23xx chips, the connection functionality must be
    * simulated using GPIO. Hopefully for production this will be fixed and the
    * commented out code will work */
    //PINSEL4 = (PINSEL4 & ~(3 << 18)) | (1 << 18); /* Doesn't work due to bug in chip */
    PCB_PINSEL4 = (PCB_PINSEL4 & ~(3 << 18)); /* Use pin as GPIO */
    FIO2_DIR |= (1<<9); /* Set pin to output */
    FIO2_SET = (1<<9); /* Set output high to disconnect */
#endif

    // enable PUSB
    PCONP |= (1 << 31);     

  /* The LPC23xx uses a single PLL, and has multiple clock dividers for each
   * peripheral. These settings assume a PLL frequency of 288 MHz */

    USBCLKCFG = 5; /* 288 MHz / 48 MHz = 6 */

#ifdef LPC2378_PORTB
    USBClkCtrl = (1 << 1) | (1 << 3) | (1 << 4); /* Enable the clocks */
    while (!(USBClkSt & ((1 << 1) | (1 << 3) | (1 << 4))));
    USBPortSel = 0x3; /* Set LPC to use USB Port B pins */
#else
    USBClkCtrl = (1 << 1) | (1 << 4); /* Enable the clocks */
    while (!(USBClkSt & ((1 << 1) | (1 << 4))));
#endif

#endif

#ifdef LPC1768
	// P2.9 -> USB_CONNECT  LPC_PINCON->PINSEL1 &= ~((3<<26)|(3<<28));   /* P0.29 D+, P0.30 D- */
	PINSEL4 &= ~(3<<18);
	PINSEL4 |= (1<<18);
	PINSEL3 &= ~((3<<4)|(3<<28));
	PINSEL3 |= ((1<<4)|(2<<28));
	PINSEL1 |= ~((3<<26)|(3<<28));
	PINSEL1 |= ((1<<26)|(1<<28));

	// enable PUSB
	PCONP |= (((unsigned long)1) << 31);		

	USBClkCtrl = 0x1A;
	while ((USBClkSt & 0x1A) != 0x1A); 
#endif
	
	// disable/clear all interrupts for now
	USBDevIntEn = 0;
	USBDevIntClr = 0xFFFFFFFF;
	USBDevIntPri = 0;

	USBEpIntEn = 0;
	USBEpIntClr = 0xFFFFFFFF;
	USBEpIntPri = 0;

	// by default, only ACKs generate interrupts
	USBHwNakIntEnable(0);
	
	// init debug leds
	DEBUG_LED_INIT(8);
	DEBUG_LED_INIT(9);
	DEBUG_LED_INIT(10);

	return TRUE;
}

