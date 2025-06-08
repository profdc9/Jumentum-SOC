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

/* something to protect mbed flash eventually */
#if 0
#define USER_FLASH_AREA_START 0x20000
#define USER_FLASH_AREA_END 0x80000
#define USER_FLASH_AREA_SIZE (USER_FLASH_AREA_END-USER_FLASH_AREA_START)
#define     USER_FLASH_AREA_START_STR( x )      STR( x )
#define     STR( x )                            #x
unsigned char user_area[ USER_FLASH_AREA_SIZE ] __attribute__((section( ".ARM.__at_" USER_FLASH_AREA_START_STR( USER_FLASH_AREA_START ) ), zero_init));
//unsigned char user_area[ USER_FLASH_AREA_SIZE ] __attribute__((section( ".userrom"),zero_init));
#endif

#include "iap.h"
#include "flashbanks.h"

const iap_flash_banks const flash_banks[] =
{
  { 0x20000, 0x30000 - IAPBUFLEN},
  { 0x30000, 0x40000 - IAPBUFLEN},
  { 0x40000, 0x50000 - IAPBUFLEN},
  { 0x50000, 0x60000 - IAPBUFLEN},
  { 0x60000, 0x68000 - IAPBUFLEN},
  { 0x68000, 0x70000 - IAPBUFLEN}
  /* for some reason can't write to sector 28 on LPC1768 flash.  why? */
};

#define CONFIGURATION_SPACE_START  0x00078000
#define CONFIGURATION_SPACE_LENGTH 0x00004000

char *configuration_space = (char *) CONFIGURATION_SPACE_START;
char *configuration_space_end = (char *) (CONFIGURATION_SPACE_START + CONFIGURATION_SPACE_LENGTH - IAPBUFLEN);

#define NUM_FLASH_BANKS (sizeof(flash_banks)/sizeof(iap_flash_banks))

int num_flashbanks()
{
	return NUM_FLASH_BANKS;
}

unsigned long startbank(int numbank)
{
	return flash_banks[numbank].startbank;
}

unsigned long endbank(int numbank)
{
	return flash_banks[numbank].endbank;
}

