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

/* in-circuit application programming code by jcwren */
/* modified by DLM */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "all.h"
#include "lpc210x.h"
#include "sysdefs.h"
#include "iap.h"
#include "vic.h"
#include "lib.h"
#include "cpu.h"

//
//
//
#define CPUCLK_IN_KHZ 60000
//
//
//
typedef struct flashSectorToAddress_s
{
  const unsigned long address;
  const int sizeInBytes;
}
flashSectorToAddress_t;

#if defined LPC2148 || defined LPC2378
static const flashSectorToAddress_t flashSectorToAddress [] = 
{
  { 0x00000000, 4096 },   // 0
  { 0x00001000, 4096 },   // 1
  { 0x00002000, 4096 },   // 2
  { 0x00003000, 4096 },   // 3
  { 0x00004000, 4096 },   // 4
  { 0x00005000, 4096 },   // 5
  { 0x00006000, 4096 },   // 6
  { 0x00007000, 4096 },   // 7
  { 0x00008000, 32768 },  // 8
  { 0x00010000, 32768 },  // 9
  { 0x00018000, 32768 },  // 10
  { 0x00020000, 32768 },  // 11
  { 0x00028000, 32768 },  // 12
  { 0x00030000, 32768 },  // 13
  { 0x00038000, 32768 },  // 14
  { 0x00040000, 32768 },  // 15
  { 0x00048000, 32768 },  // 16
  { 0x00050000, 32768 },  // 17
  { 0x00058000, 32768 },  // 18
  { 0x00060000, 32768 },  // 19
  { 0x00068000, 32768 },  // 20
  { 0x00070000, 32768 },  // 21
  { 0x00078000, 4096 },   // 22
  { 0x00079000, 4096 },   // 23
  { 0x0007a000, 4096 },   // 24
  { 0x0007b000, 4096 },   // 25
  { 0x0007c000, 4096 },   // 26
};
#endif
#if defined LPC2119 || defined LPC2106
static const flashSectorToAddress_t flashSectorToAddress [] = 
{
  { 0x00000000, 8192 },   // 0
  { 0x00002000, 8192 },   // 1
  { 0x00004000, 8192 },   // 2
  { 0x00006000, 8192 },   // 3
  { 0x00008000, 8192 },   // 4
  { 0x0000A000, 8192 },   // 5
  { 0x0000C000, 8192 },   // 6
  { 0x0000E000, 8192 },   // 7
  { 0x00010000, 8192 },  // 8
  { 0x00012000, 8192 },  // 9
  { 0x00014000, 8192 },  // 10
  { 0x00016000, 8192 },  // 11
  { 0x00018000, 8192 },  // 12
  { 0x0001A000, 8192 },  // 13
  { 0x0001C000, 8192 },  // 14
  { 0x0001E000, 8192 }  // 15
};
#endif

typedef struct iapErrnoStr_s
{
  const int errno;
  const char *text;
}
iapErrnoStr_t;


#ifdef IAPSTRERROR
static const iapErrnoStr_t iapErrnoStr [] =
{
  { IAP_RESULT_CMD_SUCCESS,         "success" },
  { IAP_RESULT_INVALID_COMMAND,     "invalid command" },
  { IAP_RESULT_SRC_ADDR_ERROR,      "source address error" },
  { IAP_RESULT_DST_ADDR_ERROR,      "destination address error" },
  { IAP_RESULT_SRC_ADDR_NOT_MAPPED, "source address not mapped" },
  { IAP_RESULT_DST_ADDR_NOT_MAPPED, "destination address not mapped" },
  { IAP_RESULT_COUNT_ERROR,         "count error" },
  { IAP_RESULT_INVALID_SECTOR,      "invalid sector" },
  { IAP_RESULT_SECTOR_NOT_BLANK,    "sector not blank" },
  { IAP_RESULT_SECTOR_NOT_PREPARED, "sector not prepared" },
  { IAP_RESULT_COMPARE_ERROR,       "compare error" },
  { IAP_RESULT_BUSY,                "busy" },
  { IAP_RESULT_PARAM_ERROR,         "parameter error" },
  { IAP_RESULT_ADDR_ERROR,          "address error" },
  { IAP_RESULT_ADDR_NOT_MAPPED,     "address not mapped" },
  { IAP_RESULT_CMD_LOCKED,          "command locked" },
  { IAP_RESULT_INVALID_CODE,        "invalid code" },
  { IAP_RESULT_INVALID_BAUD_RATE,   "invalid baud rate" },
  { IAP_RESULT_ANVALID_STOP_BIT,    "invalid stop bit" },
  { IAP_RESULT_CRP_ENABLED,         "CRP enabled" },
  { IAP_RESULT_X_NOTSAFEREGION,     "sector or address not in safe region" },
  { IAP_RESULT_X_NOSAFEREGIONAVAIL, "no safe sectors available (all of memory used?)" },
};
#endif

//
//
//
static unsigned int iapCommands [5];
static unsigned int iapResults [2];
int iapErrno = 0;
extern unsigned long __end_of_text__;

//
//  Convert address to sector, or -1 if address not in flash area
//
int iapAddressToSector (unsigned long address)
{
  int i;

  for (i = 0; i < (int) arrsizeof (flashSectorToAddress); i++)
    if (address < (flashSectorToAddress [i].address + flashSectorToAddress [i].sizeInBytes))
      return i;

  iapErrno = IAP_RESULT_INVALID_SECTOR;
  return -1;
}

#ifdef IAPCHECKSAFE
//
static unsigned int end_of_text = (unsigned int) &__end_of_text__;
//
//
int iapSectorToAddress (int sectorNumber, unsigned long *address, int *sectorSize)
{
  if (sectorNumber >= (int) arrsizeof (flashSectorToAddress))
    return -1;

  if (address)
    *address = flashSectorToAddress [sectorNumber].address;
  if (sectorSize)
    *sectorSize = flashSectorToAddress [sectorNumber].sizeInBytes;

  return 0;
}

//
//  1 == address in safe region, 0 if not
//
int iapIsSafeAddress (unsigned long address)
{
  int eotSector;
  int addressSector;

  if ((eotSector = iapAddressToSector (end_of_text)) == -1)
    return 0;
  if ((addressSector = iapAddressToSector (address)) == -1)
    return 0;

  if (addressSector <= eotSector)
  {
    iapErrno = IAP_RESULT_X_NOTSAFEREGION;
    return 0;
  }

  return 1;
}

//
//  0 == not safe sector, 1 == safe (not in a code sector)
//
int iapIsSafeSector (int sector)
{
  int eotSector;

  if ((eotSector = iapAddressToSector (end_of_text)) == -1)
    return 0;

  if (sector <= eotSector)
  {
    iapErrno = IAP_RESULT_X_NOTSAFEREGION;
    return 0;
  }

  return 1;
}

//
//  1 == sector is safe, 0 == not safe
//
int iapIsValidSector (int sector)
{
  if ((sector < 0) || (sector >= (int) arrsizeof (flashSectorToAddress)))
  {
    iapErrno = IAP_RESULT_INVALID_SECTOR;
    return 0;
  }

  return 1;
}
#endif
//
//
//
int iapGetErrno (void)
{
  return iapErrno;
}


#ifdef IAPSTRERROR
const char *iapStrerror (int err)
{
  unsigned int i;

  for (i = 0; i < arrsizeof (iapErrnoStr); i++)
    if (iapErrnoStr [i].errno == err)
      return iapErrnoStr [i].text;

  return NULL;
}
#endif

//
//
//
static void iapCall (void) __attribute ((naked));
static void iapCall (void)
{
  register void *pFP0 asm("r0") = iapCommands;
  register void *pFP1 asm("r1") = iapResults;

  asm volatile(" bx  %[iapLocation]"
               :
               : "r" (pFP0), "r" (pFP1), [iapLocation] "r" (IAP_LOCATION) );
}

//
//
//
int iapPrepareSectors (int startingSector, int endingSector)
{
  unsigned int fiqstate,irqstate;

#ifdef IAPCHECKSAFE
  if (!iapIsSafeSector (startingSector) || !iapIsSafeSector (endingSector))
    return -1;
#endif

  iapCommands [0] = IAP_CMD_PREPARE;
  iapCommands [1] = startingSector;
  iapCommands [2] = endingSector;

  irqstate =  swi_irq_disable();
  fiqstate =  swi_fiq_disable();

  iapCall ();

  swi_fiq_restore(fiqstate);
  swi_int_restore(irqstate);

  return ((iapErrno = iapResults [0]) == IAP_RESULT_CMD_SUCCESS) ? 0 : -1;
}

//
//  IAP_CMD_COPYRAMTOFLASH can span multiple sectors (2, at any rate, since bufferLen
//  must be 256, 512, 1024 or 4096, and the smallest sectors are 4096 bytes).  Although
//  more than 2 sectors can be prepared for writing, it's useless to do so, since
//  after each IAP_CMD_COPYRAMTOFLASH, the sectors are re-locked.
//
int iapWriteSectors (unsigned int address, unsigned char *buffer, int bufferLen)
{
  unsigned int fiqstate,irqstate;

  WATCHDOG_UPDATE();

  iapCommands [0] = IAP_CMD_COPYRAMTOFLASH;
  iapCommands [1] = address;
  iapCommands [2] = (int) buffer;
  iapCommands [3] = bufferLen;
  iapCommands [4] = CPUCLK_IN_KHZ;

  irqstate =  swi_irq_disable();
  fiqstate =  swi_fiq_disable();

  iapCall ();

  swi_fiq_restore(fiqstate);
  swi_int_restore(irqstate);

  if ((iapErrno = iapResults [0]) != IAP_RESULT_CMD_SUCCESS)
    return -1;

  return 0;
}

int iapEraseSectors (int startingSector, int endingSector)
{
  unsigned int fiqstate,irqstate;

  WATCHDOG_UPDATE();
  
#ifdef IAPCHECKSAFE
  if (!iapIsSafeSector (startingSector) || !iapIsSafeSector (endingSector))
    return -1;
#endif

  if (iapPrepareSectors (startingSector, endingSector) == -1)
    return -1;

  iapCommands [0] = IAP_CMD_ERASE;
  iapCommands [1] = startingSector;
  iapCommands [2] = endingSector;
  iapCommands [3] = CPUCLK_IN_KHZ;

  irqstate =  swi_irq_disable();
  fiqstate =  swi_fiq_disable();

  iapCall ();

  swi_fiq_restore(fiqstate);
  swi_int_restore(irqstate);

  return ((iapErrno = iapResults [0]) == IAP_RESULT_CMD_SUCCESS) ? 0 : -1;
}

void doiap(int er)
{
	if (er < 0) {
#ifdef IAPSTRERROR
		outstr("iap error ");
		outstr(iapStrerror(iapErrno));
		outstr("\n");
#else
		outstrhex("iap error ",iapErrno);
#endif
	}
}

#define vs ((iapaddbuf *)v)

void initiapaddbuf(iapaddbuf *adb, unsigned int address, unsigned int endaddress)
{
  adb->lastsec = -1;
  adb->address = address;
  adb->endaddress = endaddress;
  adb->ct = adb->noerase = 0;
}

void addchar(int c, void *v)
{
	int sec;
	
	if (c>=0) 
		vs->buf[vs->ct++] = c;
	if (((vs->ct == IAPBUFLEN) || (c<0)) && (vs->ct>0)) {
		sec = iapAddressToSector(vs->address);
		if ((sec != vs->lastsec) && (!vs->noerase)) {
			doiap(iapPrepareSectors(sec, sec));
			doiap(iapEraseSectors (sec, sec));
			vs->lastsec = sec;
		}
		doiap(iapPrepareSectors(sec, sec));
		doiap(iapWriteSectors(vs->address, vs->buf, IAPBUFLEN));
		vs->address += IAPBUFLEN;
		if (vs->address > vs->endaddress)
			vs->address = vs->endaddress;
		vs->ct = 0;
	}
}

