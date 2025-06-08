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

//#include "iap.h"
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "flashbanks.h"

#define NUM_FLASH_BANKS 8
#define BANKLEN 65536
#define CONFIG_LEN 65536

char flash_bank_memory[NUM_FLASH_BANKS*BANKLEN];
char flash_config_memory[CONFIG_LEN];

char *configuration_space = NULL;
char *configuration_space_end = NULL;
char *flash_memory = NULL;

void load_flash_from_disk(void)
{	
	FILE *fp=fopen("memory.fsh","rb");
	if (fp == NULL)
		return;
	fread(configuration_space,sizeof(char),CONFIG_LEN,fp);
	fread(flash_memory,sizeof(char),(NUM_FLASH_BANKS*BANKLEN),fp);
	fclose(fp);
}

void save_flash_to_disk(void)
{	
	FILE *fp=fopen("memory.fsh","wb");
	if (fp == NULL)
		return;
	fwrite(configuration_space,sizeof(char),CONFIG_LEN,fp);
	fwrite(flash_memory,sizeof(char),(NUM_FLASH_BANKS*BANKLEN),fp);
	fclose(fp);
}

void setup_flash_banks(void)
{
	printf("Setting up flash banks\n");
	flash_memory = flash_bank_memory;
	configuration_space = flash_config_memory;
	configuration_space_end = configuration_space + CONFIG_LEN;
	load_flash_from_disk();
}

int num_flashbanks()
{
	return NUM_FLASH_BANKS;
}

unsigned long startbank(int numbank)
{
	return ((unsigned long)(flash_memory+(numbank*BANKLEN)));
}

unsigned long endbank(int numbank)
{
	return ((unsigned long)(flash_memory+(numbank*BANKLEN)+BANKLEN));
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
	
	/*	printf("%c",c); */
	
	if (c>=0) {
		*((unsigned char *)vs->address) = c;
		vs->address++;
		if (vs->address >= vs->endaddress)
			vs->address = vs->endaddress-1;
		}
	else
		save_flash_to_disk();		
}
