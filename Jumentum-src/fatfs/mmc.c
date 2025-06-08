/*-----------------------------------------------------------------------*/
/* MMC3/SD1/SD2 (in SPI mode) control module  (C)ChaN, 2009              */
/*-----------------------------------------------------------------------*/


#include "diskio.h"
//#include "../lpc210x.h"
#include "../spi.h"


/* Definitions for MMC/SDC command */
#define CMD0   (0)			/* GO_IDLE_STATE */
#define CMD1   (1)			/* SEND_OP_COND */
#define ACMD41 (41|0x80)	/* SEND_OP_COND (SDC) */
#define CMD8   (8)			/* SEND_IF_COND */
#define CMD9   (9)			/* SEND_CSD */
#define CMD10  (10)			/* SEND_CID */
#define CMD12  (12)			/* STOP_TRANSMISSION */
#define ACMD13 (13|0x80)	/* SD_STATUS (SDC) */
#define CMD16  (16)			/* SET_BLOCKLEN */
#define CMD17  (17)			/* READ_SINGLE_BLOCK */
#define CMD18  (18)			/* READ_MULTIPLE_BLOCK */
#define CMD23  (23)			/* SET_BLOCK_COUNT */
#define ACMD23 (23|0x80)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)			/* WRITE_BLOCK */
#define CMD25  (25)			/* WRITE_MULTIPLE_BLOCK */
#define CMD32  (32)			/* ERASE_ER_BLK_START */
#define CMD33  (33)			/* ERASE_ER_BLK_END */
#define CMD38  (38)			/* ERASE */
#define CMD41  (41)			/* SEND_OP_COND (ACMD) */
#define CMD55  (55)			/* APP_CMD */
#define CMD58  (58)			/* READ_OCR */


/* Control signals (Platform dependent) */
#define CS_LOW()	spiMMCChipSelect(TRUE)	/* MMC CS = L */
#define	CS_HIGH()	spiMMCChipSelect(FALSE)	/* MMC CS = H */

#define SOCKPORT	0			/* Socket contact port */
#define SOCKWP		0x04		/* Write protect switch (bit2) */
#define SOCKINS		0x08		/* Card detect switch (bit3) */

#define	FCLK_SLOW()			/* Set slow clock (100k-400k) */
#define	FCLK_FAST()		    /* Set fast clock (depends on the CSD) */


/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static volatile
UINT Timer1, Timer2;	/* 1000Hz decrement timer */

/* Card type definitions (CardType) */

#define CT_MMC				0x01
#define CT_SD1				0x02
#define CT_SD2				0x04
#define CT_SDC				(CT_SD1|CT_SD2)
#define CT_BLOCK			0x08

static
UINT CardType;			/* b0:MMC, b1:SDC, b2:Block addressing */


/*-----------------------------------------------------------------------*/
/* Transmit a byte to MMC via SPI  (Platform dependent)                  */
/*-----------------------------------------------------------------------*/

#define xmit_spi(dat) spiSendByte(dat)

/*-----------------------------------------------------------------------*/
/* Receive a byte from MMC via SPI  (Platform dependent)                 */
/*-----------------------------------------------------------------------*/

#define rcvr_spi() spiReceiveByte()

/* Alternative macro to receive data fast */
#define rcvr_spi_m(dst)	spiReceivePtr(dst)


/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
int wait_ready (	/* 1:Ready, 0:Timeout */
	UINT wt			/* Timeout [ms] */
)
{
	Timer2 = (WORD)(wt / 10);

	rcvr_spi();		/* Read a byte (Force enable DO output) */
	do {
		if (rcvr_spi() == 0xFF) return 1;	/* Card goes ready */
		/* This loop takes a time. Insert rot_rdq() here for multitask envilonment. */
	} while (Timer2);	/* Wait until card goes ready or timeout */

	return 0;	/* Timeout occured */
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void deselect (void)
{
	CS_HIGH();
	rcvr_spi();
}



/*-----------------------------------------------------------------------*/
/* Select the card and wait ready                                        */
/*-----------------------------------------------------------------------*/

static
int select (void)	/* 1:Successful, 0:Timeout */
{
	CS_LOW();
	if (!wait_ready(500)) {
		deselect();
		return 0;
	}
	return 1;
}



/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions and chk_power always returns 1.   */

static
void power_on (void)
{
	spiInit ();
}

static
void power_off (void)
{
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static
int rcvr_datablock (
	BYTE *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count (must be even number) */
)
{
	BYTE token;
	WORD *wp;


	Timer1 = 200;
	do {								/* Wait for data packet in timeout of 200ms */
		token = rcvr_spi();
	} while ((token == 0xFF) && Timer1);

	if(token != 0xFE) return 0;		/* If not valid data token, retutn with error */

	do {								/* Receive the data block into buffer */
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
	} while (btr -= 2);
	rcvr_spi();							/* Discard CRC */
	rcvr_spi();

	return 1;						/* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
static
int xmit_datablock (
	const BYTE *buff,	/* 512 byte data block to be transmitted */
	BYTE token			/* Data/Stop token */
)
{
	BYTE resp;
	const WORD *rp;
	UINT bc = 512;


	if (!wait_ready(500)) return 0;

	xmit_spi(token);			/* Xmit data token */
	if (token != 0xFD) {	/* Is data token */
		do {						/* Xmit the 512 byte data block to MMC */
			xmit_spi(*buff++);
			xmit_spi(*buff++);
		} while (bc -= 2);
		xmit_spi(0xFF);				/* CRC (Dummy) */
		xmit_spi(0xFF);
		resp = rcvr_spi();			/* Receive data response */
		if ((resp & 0x1F) != 0x05)	/* If not accepted, return with error */
			return 0;
	}

	return 1;
}
#endif	/* _READONLY */



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, res;


	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x3F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	deselect();
	if (!select()) return 0xFF;

	/* Send command packet */
	xmit_spi(0x40 | cmd);				/* Start + Command */
	xmit_spi((BYTE)(arg >> 24));		/* Argument[31..24] */
	xmit_spi((BYTE)(arg >> 16));		/* Argument[23..16] */
	xmit_spi((BYTE)(arg >> 8));			/* Argument[15..8] */
	xmit_spi((BYTE)arg);				/* Argument[7..0] */
	n = 0;								/* CRC */
	if (cmd == CMD0) n = 0x95;			/* CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* CRC for CMD8(0x1AA) */
	xmit_spi(n);

	/* Receive command response */
	if (cmd == CMD12) rcvr_spi();		/* Skip a stuff byte when stop reading */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do
		res = rcvr_spi();
	while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive nmuber (0) */
)
{
	BYTE n, cmd, ty, ocr[4];


	if (drv) return STA_NOINIT;			/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	power_on();							/* Force socket power on */
	FCLK_SLOW();
	for (n = 10; n; n--) rcvr_spi();	/* 80 dummy clocks */

	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		Timer1 = 1000;						/* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDC Ver2+ */
			for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();		/* Get trailng data of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at vdd range of 2.7-3.6V */
				while (Timer1 && send_cmd(ACMD41, 1UL << 30));	/* ACMD41 with HCS bit */
				if (Timer1 && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
					ty = (ocr[0] & 0x40) ? CT_BLOCK|CT_SD2 : CT_SD2;
				}
			}
		} else {							/* SDC Ver1 or MMC */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDC */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMC */
			}
			while (Timer1 && send_cmd(cmd, 0));			/* Wait for leaving idle state */
			if (!Timer1 || send_cmd(CMD16, 512) != 0)	/* Select R/W block length */
				ty = 0;
		}
	}
	CardType = ty;
	deselect();

	if (ty) {			/* Initialization succeded */
		FCLK_FAST();
		Stat &= ~STA_NOINIT;		/* Clear STA_NOINIT */
	} else {			/* Initialization failed */
		power_off();
	}

	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,			/* Physical drive nmuber (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block read */
		if ((send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512))
			count = 0;
	}
	else {				/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) break;
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else {				/* Multiple block write */
		if (CardType & CT_SDC) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
				count = 1;
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	DRESULT res;
	BYTE n, csd[16];
	DWORD csize, *dp, st, ed;


	if (drv) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	res = RES_ERROR;
	switch (ctrl) {
		case CTRL_SYNC :	/* Wait for end of internal write process */
			if (select()) {
				deselect();
				res = RES_OK;
			}
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (WORD) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
				if ((csd[0] >> 6) == 1) {	/* SDv2? */
					csize = csd[9] + ((WORD)csd[8] << 8) + 1;
					*(DWORD*)buff = (DWORD)csize << 10;
				} else {					/* SDv1 or MMCv2 */
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
					*(DWORD*)buff = (DWORD)csize << (n - 9);
				}
				res = RES_OK;
			}
			break;

		case GET_SECTOR_SIZE :	/* Get sectors on the disk (WORD) */
			*(WORD*)buff = 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sectors (DWORD) */
			if (CardType & CT_SD2) {	/* SDv2? */
				if (send_cmd(ACMD13, 0) == 0) {		/* Read SD status */
					rcvr_spi();
					if (rcvr_datablock(csd, 16)) {				/* Read partial block */
						for (n = 64 - 16; n; n--) rcvr_spi();	/* Purge trailing data */
						*(DWORD*)buff = 16UL << (csd[10] >> 4);
						res = RES_OK;
					}
				}
			} else {					/* SDv1 or MMCv3 */
				if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
					if (CardType & CT_SD1) {	/* SDv1 */
						*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
					} else {					/* MMCv3 */
						*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
					}
					res = RES_OK;
				}
			}
			break;

		case CTRL_ERASE_SECTOR :	/* Erase a block of sectors (used when _USE_ERASE == 1) */
			if (!(CardType & CT_SDC)) break;				/* Check if the card is SDC */
			if (disk_ioctl(drv, MMC_GET_CSD, csd)) break;	/* Get CSD */
			if (!(csd[0] >> 6) && !(csd[10] & 0x40)) break;	/* Check if sector erase can be applied to the card */
			dp = buff; st = dp[0]; ed = dp[1];				/* Load sector block */
			if (!(CardType & CT_BLOCK)) {
				st *= 512; ed *= 512;
			}
			if (send_cmd(CMD32, st) == 0 && send_cmd(CMD33, ed) == 0 && send_cmd(CMD38, 0) == 0 && wait_ready(30000))	/* Erase sector block */
				res = RES_OK;
			break;

		case MMC_GET_CSD :	/* Receive CSD as a data block (16 bytes) */
			if ((send_cmd(CMD9, 0) == 0)	/* READ_CSD */
				&& rcvr_datablock(buff, 16))
				res = RES_OK;
			break;

		case MMC_GET_CID :	/* Receive CID as a data block (16 bytes) */
			if ((send_cmd(CMD10, 0) == 0)	/* READ_CID */
				&& rcvr_datablock(buff, 16))
				res = RES_OK;
			break;

		case MMC_GET_OCR :	/* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
				for (n = 0; n < 4; n++)
					*((BYTE*)buff+n) = rcvr_spi();
				res = RES_OK;
			}
			break;

		default:
			res = RES_PARERR;
	}
	deselect();

	return res;
}



/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 1ms                         */


void disk_timerproc (void)
{
	static BYTE pv;
	BYTE s, p;
	UINT n;


	n = Timer1;						/* 1000Hz decrement timer */
	if (n) Timer1 = --n;
	n = Timer2;
	if (n) Timer2 = --n;

	p = pv;
	pv = SOCKPORT & (SOCKWP | SOCKINS);	/* Sample socket switch */

	if (p == pv) {					/* Have contacts stabled? */
		s = Stat;

		if (p & SOCKWP)				/* WP is H (write protected) */
			s |= STA_PROTECT;
		else						/* WP is L (write enabled) */
			s &= ~STA_PROTECT;

		if (p & SOCKINS)			/* INS = H (Socket empty) */
			s |= (STA_NODISK | STA_NOINIT);
		else						/* INS = L (Card inserted) */
			s &= ~STA_NODISK;

		Stat = s;
	}
}

