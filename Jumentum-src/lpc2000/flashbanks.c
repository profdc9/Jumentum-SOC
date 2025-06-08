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

#include "iap.h"
#include "flashbanks.h"

#if defined LPC2148 || defined LPC2378
const iap_flash_banks const flash_banks[] =
{
  { 0x20000, 0x30000 - IAPBUFLEN},
  { 0x30000, 0x40000 - IAPBUFLEN},
  { 0x40000, 0x50000 - IAPBUFLEN},
  { 0x50000, 0x60000 - IAPBUFLEN},
  { 0x60000, 0x70000 - IAPBUFLEN},
  { 0x70000, 0x78000 - IAPBUFLEN}
};

char *configuration_space = (char *) 0x0007c000;
char *configuration_space_end = (char *) (0x0007c000 + 0x1000);
#endif

#if defined LPC2106 || defined LPC2119
const iap_flash_banks const flash_banks[] =
{
  { 0x14000, 0x1A000 - IAPBUFLEN},
  { 0x1A000, 0x1C000 - IAPBUFLEN}
};

char *configuration_space = (char *) 0x0001C000;
char *configuration_space_end = (char *) (0x0001E000 - IAPBUFLEN);
#endif

#define NUM_FLASH_BANKS (sizeof(flash_banks)/sizeof(iap_flash_banks))

int num_flashbanks()
{
	return NUM_FLASH_BANKS;
}

unsigned long startbank(int numbank)
{
	return flash_banks[numbank].startbank;
}

extern unsigned long endbank(int numbank)
{
	return flash_banks[numbank].endbank;
}

