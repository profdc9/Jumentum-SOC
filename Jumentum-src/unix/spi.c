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

#define SPISPEED 2

#define SETMOSI() {}
#define CLRMOSI() {}
#define SETCLK() {}
#define CLRCLK() {}
#define READMISO() (0)

void spiResetEthernet(void)
{
}

void spiInit(void)
{
}

U16  SPIWrite(U8  * ptrBuffer, U16  ui_Len)
{
	return 0;
}

void spiEthChipSelect(BOOL select)
{
}

U16  SPIRead(U8  * ptrBuffer, U16  ui_Len)
{
	return 0;
}

void spiMMCChipSelect (BOOL select)
{
}

void spiSendByte (U8 c)
{
}

void spiReceivePtr (U8 *c)
{
  *c = 0;
}
U8 SPIBitBang(U8 c)
{
  U8 b = 0;
  int i = 8,j;
  do {
	 if (c & 0x80) { SETMOSI() }
	    else { CLRMOSI() }
	 c <<= 1;
	 for (j=SPISPEED;j>0;j--) CLRCLK();
	 for (j=SPISPEED;j>0;j--) SETCLK();
	 b = (b<<1) | READMISO();
	 for (j=SPISPEED;j>0;j--) CLRCLK();
  } while ((--i)>0);
  CLRMOSI();
  return b;
}
