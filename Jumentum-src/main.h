#ifndef _MAIN_H_
#define _MAIN_H_

#include "flash.h"
#include "basic.h"
#include "basicidle.h"
#ifdef INCFATFS
#include "fatfs/ff.h"
#endif

#define MSTATIC static

void syscallsInit (void);
int main (void);
#ifdef USEDLMALLOC
void *_mysbrk (int incr);
#else
caddr_t _sbrk (int incr);
#endif

#ifdef MBED
/* MBED has no flow control, needs huge buffer, bummer */
#define LINEBUFLEN          8192
#else
#define LINEBUFLEN          512
#endif

#define LINEBUFHALFLEN      (LINEBUFLEN/2)
#define LINEBUFTHRESHOLD    (LINEBUFHALFLEN+16)
#define ENDPROGCHAR 4

typedef struct _retoken
{
	tokstat tk;
	iapaddbuf adb;
	char buf[LINEBUFLEN];
	int ln, autostore, verbatim;
} retoken;

#define WORKBUFLEN 256

typedef struct _program_space_st
{
	untokstat uts;
	retoken rr;
	char workbuf[WORKBUFLEN];
} program_space_st;

extern char basic_running;
extern char run_basic;
extern char pause_basic;
extern program_space_st *program_space;
extern char *heap_end;

extern char *script;
extern char *endscript;
extern char *configuration_space;
extern char *configuration_space_end;
extern char auth_password[16];

program_space_st *allocate_program_space(void);
void deallocate_program_space(void);
void initretoken(retoken *r, unsigned int address, unsigned int endaddress, int verbatim, int autostore);
void retokenize(int c, retoken *r);
int untokenizeverbatim(untokstat *utk);
void select_bank_number(int bank);
int bankname(int n, int len, char *name);
const char *conf_string(const char *var, int *len);
int get_conf_string_max(const char *var, char *buf, int len);

#ifdef INCFATFS
extern char mmc_card_inserted;
extern FATFS fatfs[];
#define card_inserted() (mmc_card_inserted)
#else
#define card_inserted() 0
#endif

#endif
