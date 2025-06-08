/*
    enc28j60.c driver by Iain Derrington
	modified by Daniel Marks
*/

#ifdef ENC28J60

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "lpc210x.h"
#include "enc28j60.h"
#include "spi.h"
#include "lib.h"
#include "uip/uip/uip.h"


/******************************************************************************/
/** \file enc28j60.c
 *  \brief Driver code for enc28j60.
 *  \author Iain Derrington (www.kandi-electronics.com)
 *  \date  0.1 20/06/07 First Draft \n
 *         0.2 11/07/07 Removed CS check macros. Fixed bug in writePhy
 *         0.3 12.07/07 Altered for uIP 1.0 
 */
/*******************************************************************************/



// define private variables

TXSTATUS TxStatus;

// define private functions
static u8_t ReadETHReg(u8_t);         // read a ETX reg
static u8_t ReadMacReg(u8_t);         // read a MAC reg
static u16_t ReadPhyReg(u8_t);         // read a PHY reg
static u16_t ReadMacBuffer(u8_t * ,u16_t);    //read the mac buffer (ptrBuffer, no. of bytes)
static u8_t WriteCtrReg(u8_t,u8_t);               // write to control reg
static u8_t WritePhyReg(u8_t,u16_t);               // write to a phy reg
static u16_t WriteMacBuffer(u8_t *,u16_t);    // write to mac buffer
static void ResetMac(void);

static u8_t SetBitField(u8_t, u8_t);
static u8_t ClrBitField(u8_t, u8_t);
static void BankSel(u8_t);
static void spitest(void);

//define usefull macros

/** MACRO for selecting or deselecting chip select for the ENC28J60. Some HW dependancy.*/
#define SEL_MAC(x)  spiEthChipSelect((x))
/** MACRO for rev B5 fix.*/
#define ERRATAFIX   SetBitField(ECON1, ECON1_TXRST);ClrBitField(ECON1, ECON1_TXRST);ClrBitField(EIR, EIR_TXERIF | EIR_TXIF)



/***********************************************************************/
/** \brief Initialise the MAC.
 *
 * Description: \n
 * a) Setup SPI device. Assume Reb B5 for sub 8MHz operation \n
 * b) Setup buffer ptrs to devide memory in In and Out mem \n
 * c) Setup receive filters (accept only unicast).\n
 * d) Setup MACON registers (MAC control registers)\n
 * e) Setup MAC address
 * f) Setup Phy registers
 * \author Iain Derrington
 * \date 0.1 20/06/07 First draft
 */
/**********************************************************************/

int initMAC(void)
{
  u16_t test;
  spiInit();        // initialise the SPI
  spiResetEthernet();
  
  ResetMac();       // erm. Resets the MAC.
  
                    // setup memory by defining ERXST and ERXND
  BankSel(0);       // select bank 0
  WriteCtrReg(ERXSTL,(u8_t)( RXSTART & 0x00ff));    
  WriteCtrReg(ERXSTH,(u8_t)((RXSTART & 0xff00)>> 8));
  WriteCtrReg(ERXNDL,(u8_t)( RXEND   & 0x00ff));
  WriteCtrReg(ERXNDH,(u8_t)((RXEND   & 0xff00)>>8));

                    // Make sure Rx Read ptr is at the start of Rx segment
  WriteCtrReg(ERXRDPTL, (u8_t)( RXSTART & 0x00ff));
  WriteCtrReg(ERXRDPTH, (u8_t)((RXSTART & 0xff00)>> 8));

  BankSel(1);                             // select bank 1
  WriteCtrReg(ERXFCON,( ERXFCON_UCEN + ERXFCON_CRCEN + ERXFCON_BCEN));


                // Initialise the MAC registers
  BankSel(2);                             // select bank 2
  SetBitField(MACON1, MACON1_MARXEN);     // Enable reception of frames
  WriteCtrReg(MACLCON2, 63);
  WriteCtrReg(MACON3, MACON3_FRMLNEN +    // Type / len field will be checked
                      MACON3_TXCRCEN +    // MAC will append valid CRC
                      MACON3_PADCFG0);    // All small packets will be padded
                      

  SetBitField(MACON4, 0 /*MACON4_DEFER*/ );      
  WriteCtrReg(MAMXFLL, (u8_t)( MAXFRAMELEN & 0x00ff));     // set max frame len
  WriteCtrReg(MAMXFLH, (u8_t)((MAXFRAMELEN & 0xff00)>>8));
  WriteCtrReg(MABBIPG, 0x12);             // back to back interpacket gap. set as per data sheet
  WriteCtrReg(MAIPGL , 0x12);             // non back to back interpacket gap. set as per data sheet
  WriteCtrReg(MAIPGH , 0x0C);

  
            //Program our MAC address
  BankSel(3);              
  WriteCtrReg(MAADR1,uip_ethaddr.addr[0]);   
  WriteCtrReg(MAADR2,uip_ethaddr.addr[1]);  
  WriteCtrReg(MAADR3,uip_ethaddr.addr[2]);
  WriteCtrReg(MAADR4,uip_ethaddr.addr[3]);
  WriteCtrReg(MAADR5,uip_ethaddr.addr[4]);
  WriteCtrReg(MAADR6,uip_ethaddr.addr[5]);

  // Initialise the PHY registes
  WritePhyReg(PHCON1, 0x000);
  test =ReadPhyReg(PHCON1);
  WritePhyReg(PHCON2, PHCON2_HDLDIS);
  WriteCtrReg(ECON1,  ECON1_RXEN);     //Enable the chip for reception of packets
  //SetBitField(EIE, EIE_TXIE |EIE_INTIE);
  SetBitField(EIE, 0);
  return 0;
}

/***********************************************************************/
/** \brief Writes a packet to the ENC28J60.
 *
 * Description: Writes ui_len bytes of data from ptrBufffer into ENC28J60.
 *              puts the necessary padding around the packet to make it a legit
                MAC packet.\n \n 
                1) Program ETXST.   \n
                2) Write per packet control byte.\n
                3) Program ETXND.\n
                4) Set ECON1.TXRTS.\n
                5) Check ESTAT.TXABRT. \n

 * \author Iain Derrington
 * \param ptrBuffer ptr to byte buffer. 
 * \param ui_Len Number of bytes to write from buffer. 
 * \return uint True or false. 
 */
/**********************************************************************/
u16_t MACWrite()
{
  volatile u16_t i;
  volatile u16_t address = TXSTART;
  u8_t  bytControl;
  
  bytControl = 0x00;
  
  BankSel(0);                                          // select bank 0

  WriteCtrReg(EWRPTL,(u8_t)( TXSTART & 0x00ff));        // Set write buffer to point to start of Tx Buffer
  WriteCtrReg(EWRPTH,(u8_t)((TXSTART & 0xff00)>>8));

  address += WriteMacBuffer(&bytControl,1);                       // write per packet control byte  
  
  address+=WriteMacBuffer(&uip_buf[0], UIP_LLH_LEN);
  if(uip_len <= UIP_LLH_LEN + UIP_TCPIP_HLEN) 
  {
    address+=WriteMacBuffer(&uip_buf[UIP_LLH_LEN], uip_len - UIP_LLH_LEN);
  } 
  else 
  {
    address+=WriteMacBuffer(&uip_buf[UIP_LLH_LEN], UIP_TCPIP_HLEN);
    address+=WriteMacBuffer(uip_appdata, uip_len - UIP_TCPIP_HLEN - UIP_LLH_LEN);
  }

  WriteCtrReg(ETXSTL,(u8_t)( TXSTART & 0x00ff));        // write ptr to start of Tx packet
  WriteCtrReg(ETXSTH,(u8_t)((TXSTART & 0xff00)>>8));
  
  WriteCtrReg(ETXNDL, (u8_t)( address & 0x00ff));       // Tell MAC when the end of the packet is
  WriteCtrReg(ETXNDH, (u8_t)((address & 0xff00)>>8));

  ClrBitField(EIR,EIR_TXIF);

  ERRATAFIX;    
  SetBitField(ECON1, ECON1_TXRTS);                     // begin transmitting;

  {
    int count = 30000;
	do
	{      
	}while ((!(ReadETHReg(EIR) & (EIR_TXIF))) && (count-- > 0));           // kill some time. Note: Nice place to block? 
  }
  ClrBitField(ECON1, ECON1_TXRTS);
  ClrBitField(EIR, EIR_TXIF);
  
  BankSel(0);                                         // read tx status bytes
#ifdef STATUSSTRUCT
  address++;                                          // increment ptr to address to start of status struc
  WriteCtrReg(ERDPTL, (u8_t)( address & 0x00ff));      // Setup the buffer read ptr to read status struc
  WriteCtrReg(ERDPTH, (u8_t)((address & 0xff00)>>8));
  ReadMacBuffer(&TxStatus.v[0],7);
#endif
  
  if (ReadETHReg(ESTAT) & ESTAT_TXABRT)                // did transmission get interrupted?
  {
//    deboutstrhex("txabrt=",ReadETHReg(ESTAT));
#ifdef STATUSSTRUCT
//    deboutstrhex("TxStatus.v[0]=",TxStatus.v[0]);
//    deboutstrhex("TxStatus.v[1]=",TxStatus.v[1]);
//    deboutstrhex("TxStatus.v[2]=",TxStatus.v[2]);
//    deboutstrhex("TxStatus.v[3]=",TxStatus.v[3]);
//    deboutstrhex("TxStatus.v[4]=",TxStatus.v[4]);
//    deboutstrhex("TxStatus.v[5]=",TxStatus.v[5]);
//    if (TxStatus.bits.LateCollision)
//    {
//      ClrBitField(ECON1, ECON1_TXRTS);
//      SetBitField(ECON1, ECON1_TXRTS);    
//      ClrBitField(ESTAT,ESTAT_TXABRT | ESTAT_LATECOL);
//    }
#endif
    ClrBitField(EIR, EIR_TXERIF | EIR_TXIF);
    ClrBitField(ESTAT,ESTAT_TXABRT);
    return FALSE;                                          // packet transmit failed. Inform calling function
  }                                                        // calling function may inquire why packet failed by calling [TO DO] function
  return TRUE;
}

/***********************************************************************/
/** \brief Tries to read a packet from the ENC28J60. 
 *
 * Description: If a valid packet is available in the ENC28J60 this function reads the packet into a
 *              buffer. The memory within the ENC28J60 will then be released. This version of the
                driver does not use interrupts so this function needs to be polled.\n \n
 * 
 * 1) Read packet count register. If >0 then continue else return. \n
 * 2) Read the current ERXRDPTR value. \n           
 * 3) Write this value into ERDPT.     \n
 * 4) First two bytes contain the ptr to the start of next packet. Read this value in. \n
 * 5) Calculate length of packet. \n
 * 6) Read in status byte into private variable. \n
 * 7) Read in packet and place into buffer. \n
 * 8) Free up memory in the ENC. \n
 *
 * \author Iain Derrington
 * \param ptrBuffer ptr to buffer of bytes where the packet should be read into. 
 * \return u16_t, the number of complete packets in the buffer -1.

 */
/**********************************************************************/
u16_t MACRead()
{
  volatile u16_t u16_t_pckLen,test;
  volatile u16_t u16_t_rdptr,u16_t_wrtptr;
  static u16_t nextpckptr = RXSTART;
  volatile RXSTATUS ptrRxStatus;
  volatile u8_t bytPacket;

  BankSel(1);
  
  bytPacket = ReadETHReg(EPKTCNT);          // How many packets have been received
  
  if(bytPacket == 0)
    return bytPacket;                       // No full packets received
  
  BankSel(0);

  WriteCtrReg(ERDPTL,(u8_t)( nextpckptr & 0x00ff));   //write this value to read buffer ptr
  WriteCtrReg(ERDPTH,(u8_t)((nextpckptr & 0xff00)>>8));

  ReadMacBuffer((u8_t*)&ptrRxStatus.v[0],6);             // read next packet ptr + 4 status bytes
  nextpckptr = ptrRxStatus.bits.NextPacket;
  
  uip_len=ptrRxStatus.bits.ByteCount;
  if (uip_len > UIP_BUFSIZE) uip_len = UIP_BUFSIZE;
  ReadMacBuffer(uip_buf,uip_len);   // read packet into buffer
  
                                        // ptrBuffer should now contain a MAC packet
  BankSel(0);
  WriteCtrReg(ERXRDPTL,ptrRxStatus.v[0]);  // free up ENC memory my adjustng the Rx Read ptr
  WriteCtrReg(ERXRDPTH,ptrRxStatus.v[1]);
 
  // decrement packet counter
  SetBitField(ECON2, ECON2_PKTDEC);
 
  return uip_len;
}

/*------------------------Private Functions-----------------------------*/

/***********************************************************************/
/** \brief ReadETHReg.
 *
 * Description: Reads contents of the addressed ETH reg over SPI bus. Assumes correct bank selected.
 *              
 *              
 * \author Iain Derrington
 * \param bytAddress Address of register to be read
 * \return byte Value of register.
 */
/**********************************************************************/
static u8_t ReadETHReg(u8_t bytAddress)
{
  u8_t bytData;
  
  if (bytAddress > 0x1F)    
    return FALSE;                 // address invalid, [TO DO] 

  SEL_MAC(TRUE);                 // ENC CS low
  SPIWrite(&bytAddress,1);       // write opcode
  SPIRead(&bytData, 1);          // read value
  SEL_MAC(FALSE);
  
  return bytData;

}

/***********************************************************************/
/** \brief ReadMacReg.
 *
 * Description: Read contents of addressed MAC register over SPI bus. Assumes correct bank selected.
 *                    
 * \author Iain Derrington
 * \param bytAddress Address of register to read.
 * \return byte Contens of register just read.
 */
/**********************************************************************/
static u8_t ReadMacReg(u8_t bytAddress)
{
  u8_t bytData;

  if (bytAddress > 0x1F)    
    return FALSE;                 // address invalid, [TO DO] 

  SEL_MAC(TRUE);                 // ENC CS low
 
  SPIWrite(&bytAddress,1);    // write opcode
  SPIRead(&bytData, 1);       // read dummy byte
  SPIRead(&bytData, 1);       // read value
  SEL_MAC(FALSE);
 

  return bytData;
}

/***********************************************************************/
/** \brief Write to phy Reg. 
 *
 * Description:  Writing to PHY registers is different to writing the other regeisters in that
                the registers can not be accessed directly. This function wraps up the requirements
                for writing to the PHY reg. \n \n
                  
                1) Write address of phy reg to MIREGADR. \n
                2) Write lower 8 bits of data to MIWRL. \n
                3) Write upper 8 bits of data to MIWRL. \n \n            
 *              
 *              
 * \author Iain Derrington
 * \param address
 * \param data
 * \return byte
 */
/**********************************************************************/
static u8_t WritePhyReg(u8_t address, u16_t data)
{ 
  if (address > 0x14)
    return FALSE;
  
  BankSel(2);

  WriteCtrReg(MIREGADR,address);        // Write address of Phy reg 
  WriteCtrReg(MIWRL,(u8_t)data);        // lower phy data 
  WriteCtrReg(MIWRH,((u8_t)(data >>8)));    // Upper phydata

  return TRUE;
}

/***********************************************************************/
/** \brief Read from PHY reg.
 *
 * Description: No direct access allowed to phy registers so the folling process must take place. \n \n
 *              1) Write address of phy reg to read from into MIREGADR. \n
 *              2) Set MICMD.MIIRD bit and wait 10.4uS. \n
 *              3) Clear MICMD.MIIRD bit. \n
 *              4) Read data from MIRDL and MIRDH reg. \n
 * \author Iain Derrington
 * \param address
 * \return uint
 */
/**********************************************************************/
static u16_t ReadPhyReg(u8_t address)
{
 volatile u16_t uiData;
 volatile u8_t bytStat;

  do {
    int count=0;
	BankSel(2);
	WriteCtrReg(MIREGADR,address);    // Write address of phy register to read
	SetBitField(MICMD, MICMD_MIIRD);  // Set rd bit
	do                                  
	{
		bytStat = ReadMacReg(MISTAT);
	}while ((bytStat & MISTAT_BUSY) && (count++ < 3000));
  } while (bytStat & MISTAT_BUSY);

  ClrBitField(MICMD,MICMD_MIIRD);   // Clear rd bit
  uiData = (u16_t)ReadMacReg(MIRDL);       // Read low data byte
  uiData |=((u16_t)ReadMacReg(MIRDH)<<8); // Read high data byte

  return uiData;
}

/***********************************************************************/
/** \brief Write to a control reg .
 *
 * Description: Writes a byte to the address register. Assumes that correct bank has
 *              all ready been selected
 *              
 * \author Iain Derrington
 * \param bytAddress Address of register to be written to. 
 * \param bytData    Data to be written. 
 * \returns byte
 */
/**********************************************************************/
static u8_t WriteCtrReg(u8_t bytAddress,u8_t bytData)
{
  if (bytAddress > 0x1f)
  {
    return FALSE;
  }

  bytAddress |= WCR_OP;       // Set opcode
  SEL_MAC(TRUE);              // ENC CS low
  SPIWrite(&bytAddress,1);    // Tx opcode and address
  SPIWrite(&bytData,1);       // Tx data
  SEL_MAC(FALSE);
  
  return TRUE;
}

/***********************************************************************/
/** \brief Read bytes from MAC data buffer.
 *
 * Description: Reads a number of bytes from the ENC28J60 internal memory. Assumes auto increment
 *              is on. 
 *              
 * \author Iain Derrington
 * \param bytBuffer  Buffer to place data in. 
 * \param byt_length Number of bytes to read.
 * \return uint  Number of bytes read.
 */
/**********************************************************************/
static u16_t ReadMacBuffer(u8_t * bytBuffer,u16_t byt_length)
{
  u8_t bytOpcode;
  volatile u16_t len;

  bytOpcode = RBM_OP;
  SEL_MAC(TRUE);            // ENC CS low
  
  SPIWrite(&bytOpcode,1);   // Tx opcode 
  len = SPIRead(bytBuffer, byt_length);   // read bytes into buffer
  SEL_MAC(FALSE);           // release CS
  

  return len;

}
/***********************************************************************/
/** \brief Write bytes to MAC data buffer.
 *
 * Description: Reads a number of bytes from the ENC28J60 internal memory. Assumes auto increment
 *              is on.
 *             
 * \author Iain Derrington
 * \param bytBuffer
 * \param ui_len
 * \return uint
 * \date WIP
 */
/**********************************************************************/
static u16_t WriteMacBuffer(u8_t * bytBuffer,u16_t ui_len)
{
  u8_t bytOpcode;
  volatile u16_t len;

  bytOpcode = WBM_OP;
  SEL_MAC(TRUE);            // ENC CS low
  
  SPIWrite(&bytOpcode,1);   // Tx opcode 
  len = SPIWrite(bytBuffer, ui_len);   // read bytes into buffer
  SEL_MAC(FALSE);           // release CS
 
  return len;

}

/***********************************************************************/
/** \brief Set bit field. 
 *
 * Description: Sets the bit/s at the address register. Assumes correct bank has been selected.
 *                           
 * \author Iain Derrington
 * \param bytAddress Address of registed where bit is to be set
 * \param bytData    Sets all the bits high.
 * \return byte      True or false
 */
/**********************************************************************/
static u8_t SetBitField(u8_t bytAddress, u8_t bytData)
{
  if (bytAddress > 0x1f)
  {
    return FALSE;
  }

  bytAddress |= BFS_OP;       // Set opcode
  SEL_MAC(TRUE);              // ENC CS low
  
  SPIWrite(&bytAddress,1);    // Tx opcode and address
  SPIWrite(&bytData,1);       // Tx data
  SEL_MAC(FALSE);
  
  return TRUE;
}

/***********************************************************************/
/** \brief Clear bit field on ctr registers.
 *
 * Description: Sets the bit/s at the address register. Assumes correct bank has been selected.
 *             
 * \author Iain Derrington
 * \param bytAddress Address of registed where bit is to be set
 * \param bytData    Sets all the bits high.
 * \return byte      True or false
 */
/**********************************************************************/
static u8_t ClrBitField(u8_t bytAddress, u8_t bytData)
{
 if (bytAddress > 0x1f)
  {
    return FALSE;
  }

  bytAddress |= BFC_OP;       // Set opcode
  SEL_MAC(TRUE);              // ENC CS low
  
  SPIWrite(&bytAddress,1);    // Tx opcode and address
  SPIWrite(&bytData,1);       // Tx data
  SEL_MAC(FALSE);
  
  return TRUE;
}

/***********************************************************************/
/** \brief Bank Select.
 *
 * Description: Select the required bank within the ENC28J60
 *              
 *              
 * \author Iain Derrington
 * \param bank Value between 0 and 3.
 * \date 0.1 09/06/07 First draft
 */
/**********************************************************************/
static void BankSel(u8_t bank)
{
  volatile u8_t temp;

  if (bank >3)
    return;
  
  temp = ReadETHReg(ECON1);       // Read ECON1 register
  temp &= ~ECON1_BSEL;            // mask off the BSEL bits
  temp |= bank;                   // set BSEL bits
  WriteCtrReg(ECON1, temp);       // write new values back to ENC28J60
}
/***********************************************************************/
/** \brief ResetMac.
 *
 * Description: Sends command to reset the MAC.
 *              
 *              
 * \author Iain Derrington
 */
/**********************************************************************/
static void ResetMac(void)
{
  u8_t bytOpcode = RESET_OP;
  SEL_MAC(TRUE);              // ENC CS low
  
  SPIWrite(&bytOpcode,1);     // Tx opcode and address
  SEL_MAC(FALSE);
  
}

#endif  /* ENC28J60 */
