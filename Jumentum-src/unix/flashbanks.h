#ifndef _FLASHBANKS_H
#define _FLASHBANKS_H

#include "../flash.h"

typedef struct _iap_flash_banks
{
   unsigned long startbank;
   unsigned long endbank;
} iap_flash_banks;

void setup_flash_banks();

extern int num_flashbanks();
extern unsigned long startbank(int numbank);
extern unsigned long endbank(int numbank);

extern const iap_flash_banks const flash_banks[];

extern char *configuration_space;
extern char *configuration_space_end;

void load_flash_from_disk(void);
void save_flash_to_disk(void);

#endif   /* _FLASHBANKS_H */
