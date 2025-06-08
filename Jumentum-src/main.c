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

//
//  Standard includes
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#ifdef INCFATFS
#include "fatfs/ff.h"
#define FATFSCOMMANDS
/* undefine to save space */
#endif

#include "all.h"

#include "serial.h"
#include "basic.h"
#include "processor.h"
#include "flash.h"
#include "lib.h"
#include "editor.h"
#ifdef INCNETWORK
#include "net.h"
#endif
#include "main.h"

char *script;
char *endscript;

char cur_flash_bank_number = 0;
char basic_running = 0;
char run_basic = 0;
char pause_basic = 0;
char enable_watchdog = 0;
#ifdef INCNETWORK
unsigned char nonet = 0;
#endif
#ifdef INCFATFS
unsigned char nofatfs = 0;
#endif
unsigned char startup_time = 0;
unsigned long console_bps = 38400;
program_space_st *program_space = NULL;
char auth_password[16];

void retokenize(int c, retoken *r);

void select_bank_number(int bank)
{
	if ((!basic_running) && (program_space == NULL))
	{
		if ((bank<0) || (bank>=num_flashbanks()))
			bank=0;		
		cur_flash_bank_number = bank;
		script = (char *)startbank(bank);
		endscript = (char *)endbank(bank);
	}
}

MSTATIC void outcurrentlyprogramming(void)
{
	outstr("Currently programming\n");
}

program_space_st *allocate_program_space(void)
{
	if ((basic_running) || (program_space != NULL))
		return NULL;
	program_space = malloc(sizeof(program_space_st));
	return program_space;
}

void deallocate_program_space(void)
{
	if (program_space != NULL) {
		free(program_space);
		program_space = NULL;	
	}	
}

#ifdef INCFATFS

char mmc_card_inserted = 0;

FATFS fatfs[_VOLUMES];		/* File system object for each logical drive */

#define FINFOLEN 51
#define FSLINESPERSCREEN 20

MSTATIC void outcharln(void)
{
	outchar('\n');
}

MSTATIC void printfatfserror(int c)
{
	if (c == FR_OK)
		return;
	outstr("FATFS error: ");
	outstr(fatfserrortext[c]);
	outcharln();
}

MSTATIC void printkilobytesfree(void)
{
	int c;
	unsigned long p1;
	FATFS *f = &fatfs[0];
	
	if ((c = f_getfree("", (DWORD*)&p1, &f)) == FR_OK) 
		outstrint("Kbytes free: ",p1 * fatfs[0].csize / 2);
	else
		printfatfserror(c);
}


extern void spiInit(void);

MSTATIC void mountdisk(void)
{
	int c;
	static unsigned char spiinited = 0;
	
	if (!spiinited) {
		spiInit();
	}
    outstr("Mounting SD card...\n");
	c = f_mount(0, &fatfs[0]);
	mmc_card_inserted = (c == FR_OK);
	if (!mmc_card_inserted)
	{
		if (!spiinited)
			printfatfserror(c);
	} else
		printkilobytesfree();
	spiinited = 1;
	outcharln();
}

MSTATIC void printcurdir()
{
	if (!card_inserted()) {
		outstr("MMC/SD Absent\n");
		return;
	}
#ifdef FATFSCOMMANDS
	{
		char s[80];
		if (f_getcwd(s,sizeof(s)-1) == FR_OK)
		{
			outstr("CWD: ");
			outstr(s);
			outcharln();
		}
	}
#else
	outstr("MMC/SD Mounted\n");
#endif
}

#ifdef FATFSCOMMANDS
MSTATIC char *dosdirstring(char *str, FILINFO *finfo)
{
    char s[ITOASIZE];

	memset(str,' ',FINFOLEN);
	mystrcpynonull(&str[0],myltoapad(s,(finfo->fdate >> 9) + 1980,4),4);
	str[4]='/';
	mystrcpynonull(&str[5],myltoapad(s,(finfo->fdate >> 5) & 15,2),2);
	str[7]='/';
	mystrcpynonull(&str[8],myltoapad(s,finfo->fdate & 31,2),2);
	mystrcpynonull(&str[11],myltoapad(s,(finfo->ftime >> 11),2),2);
	str[13]=':';
	mystrcpynonull(&str[14],myltoapad(s,(finfo->ftime >> 5) & 63,2),2);

	if (finfo->fattrib & AM_DIR) {
		mystrcpynonull(&str[18], "<DIR>", 5);
	} else {
		mystrcpynonull(&str[18],myltoa(s,finfo->fsize),9);
	}
	mystrcpynonull(&str[30],&(finfo->fname[0]),12);
	str[42]=0;
	return str;
}

MSTATIC int pauselisting(void)
{
	static const char message[] = "Press a key or Q to quit";
	const char *b;
	int ch;
	
	outstr(message);
	ch = inchar();
	b = message;
	while (*b++) outstr("\010 \010");
	return (ch == 'Q') || (ch == 'q');
}

MSTATIC void directory(void)
{
	DIR dir;				/* Directory object */
	FILINFO finfo;
	unsigned res;
	char s[FINFOLEN+1];
	int ct, tct;

	printcurdir();
	outcharln();
	res = f_opendir(&dir, "");
	if (res)
	{	
		printfatfserror(res);
		return;
	}
	ct = tct = 0;
	for(;;) {
		res = f_readdir(&dir, &finfo);
		if ((res != FR_OK) || !finfo.fname[0]) break;
		outstr(dosdirstring(s,&finfo));
		outchar('\n');
		if (finfo.fattrib & AM_DIR) tct++;
		if ((++ct) > FSLINESPERSCREEN) {
			ct = 0;
			if (pauselisting()) {
				tct = -1;
				break;
			}
		}
	}
	if (tct != -1) {
		outcharln();
		outint(tct);
		outstr(" Files, ");
		printkilobytesfree();
	}
	outcharln();
}

MSTATIC void unmountdisk(void)
{
	int c;
	
	c = f_mount(0, NULL);
	if (c != FR_OK)
		printfatfserror(c);
	else
		outstr("Filesystem unmounted\n");
}

MSTATIC void changedir(void)
{
	char c[80];
	const char *d;
	int	res;
	
    outstr("New Directory: ");
	inputstr(c,sizeof(c)-1);
	d = skipspaces(c);
	if (!(*d))
		return;
	if ((res = f_chdir(c)) != FR_OK)
		printfatfserror(res);
	outcharln();
}

MSTATIC void loadprogram(void)
{
	char buf[80];
	const char *filename;
	int	res, c;
	unsigned int i, bw;
	retoken *rr;
	FIL fil;

	if (allocate_program_space() == NULL) {
		outcurrentlyprogramming();
		return;
	}
    outstr("Load Program: ");
	inputstr(buf,sizeof(buf)-1);
	filename = skipspaces(buf);
	if (!(*filename)) {
		deallocate_program_space();
		return;
	}
	if ((res = f_open(&fil, filename, FA_READ)) != FR_OK)
	{
		printfatfserror(res);
		deallocate_program_space();
		return;
	}
    rr = &program_space->rr;
	initretoken(rr, (unsigned int)script, (unsigned int)endscript, 0, 1);	
	do
	{
		if ((res = f_read(&fil, buf, sizeof(buf), &bw)) != FR_OK)
		{
			printfatfserror(res);
			break;
		}
		for (i=0;i<bw;i++)
		{
			c = buf[i];
			if ((c != 0) && (c != '\r')) retokenize(c, rr);
		}
	} while (bw == sizeof(buf));
	retokenize(0,rr);
	retokenize(-1,rr);
	f_close(&fil);
	outstr(res == FR_OK ? "Program loaded\n" : "WARNING: Program loading failed\n");
	deallocate_program_space();
	outcharln();
}

MSTATIC int queryyesorno(void)
{
	int c;
	do
	{
		c = toupper(inkey());
	} while ((c != 'Y') && (c != 'N'));
	outchar(c);
	outcharln();
	return (c == 'Y');
}


MSTATIC void saveprogram(void)
{
	char buf[80];
	const char *filename;
	int	res;
	int c;
	unsigned int bw;
	untokstat *u;
	FIL fil;

	if (allocate_program_space() == NULL) {
		outcurrentlyprogramming();
		return;
	}	
    outstr("Save Program: ");
	inputstr(buf,sizeof(buf)-1);
	filename = skipspaces(buf);
	if (!(*filename)) {
		deallocate_program_space();
		return;
	}		
	if (f_open(&fil, filename, FA_READ) == FR_OK)
	{
		f_close(&fil);
		outstr("File exists. Overwrite? (y/n): ");
		if (!queryyesorno())
		{
			deallocate_program_space();
			return;
		}
	}
	if ((res = f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) 
	{
		printfatfserror(res);
		deallocate_program_space();
		return;
	}
	
	u = &program_space->uts;
	inituntokenizestate(u, script);
	while ((c=untokenizecode(u)) != 0)
	{
		res = f_write(&fil,&c,1,&bw);
		if (res != FR_OK) break;
	}
	if (res != FR_OK)
		printfatfserror(res);
	else
		outstr("Program saved\n");
	f_close(&fil);
	deallocate_program_space();
	outcharln();
}

MSTATIC void viewfile(void)
{
	char buf[80];
	const char *filename;
	int	res;
	FIL fil;
	unsigned int i, bw;
	int lfs;
	
    outstr("View File: ");
	inputstr(buf,sizeof(buf)-1);
	filename = skipspaces(buf);
	if (!(*filename))
		return;
		
	if ((res = f_open(&fil, filename, FA_READ)) != FR_OK)
	{
		printfatfserror(res);
		return;
	}
    outstr("---File Listing---\n");	
	lfs = 0;
	do
	{
		if ((res = f_read(&fil, buf, sizeof(buf), &bw)) != FR_OK)
		{
			printfatfserror(res);
			break;
		}
		for (i=0;i<bw;i++)
		{
			outchar(buf[i]);
			if (buf[i] == '\n')
			{
				if ((++lfs) >= FSLINESPERSCREEN)
				{
					lfs = 0;
					if (pauselisting()) {
						bw = sizeof(buf)+1;
						break;
					}
				}
			}
		}
	} while (bw == sizeof(buf));
    outstr("---End of Listing---\n");		
	f_close(&fil);
	outcharln();
}

#ifdef EDITOR
struct editfilestate 
{
	int error;
	int writing;
	FIL *filptr;
};

MSTATIC int readeditfile(void *v)
{
	struct editfilestate *e = (struct editfilestate *)v;
	char ch;
	int res;
	unsigned int bw;
	
	do
	{
		res = f_read(e->filptr, &ch, sizeof(char), &bw);
		if ((res != FR_OK) || (bw != sizeof(char))) {
			if (res != FR_OK) 
				e->error = 1;	
			ch = 0;
			break;
		}
	}	while ((ch == 0) || (ch == '\r'));
	return ch;
}

MSTATIC int writeeditfile(int ch, void *v)
{
	struct editfilestate *e = (struct editfilestate *)v;
	int res;
	unsigned int bw;
	
	if (!e->writing)
	{
		e->writing = 1;
		if (f_lseek(e->filptr, 0) != FR_OK)
			e->error = 1;
	}
	if ((!e->error) && (ch > 0))
	{
		res = f_write(e->filptr, &ch, sizeof(char), &bw);
		if ((res != FR_OK) || (bw != sizeof(char)))
			e->error = 1;
	}
	return e->error;
}

MSTATIC void editfile(void)
{
	char buf[80];
	const char *filename;
	int	res;
	FIL fil;
	struct editfilestate filestate;
	
    outstr("Edit File: ");
	inputstr(buf,sizeof(buf)-1);
	filename = skipspaces(buf);
	if (!(*filename))
		return;
		
	if ((res = f_open(&fil, filename, FA_READ | FA_WRITE)) != FR_OK)
	{
		if (res == FR_NO_FILE) {
			outstr("File does not exist.  Create it? (Y/N): ");
			if (!queryyesorno())
				return;
			if ((res = f_open(&fil, filename, FA_READ | FA_WRITE | FA_CREATE_NEW)) != FR_OK) {
				printfatfserror(res);
				return;
			}	
		} else {
			printfatfserror(res);
			return;
		}
	}
	filestate.error = 0;
	filestate.writing = 0;
	filestate.filptr = &fil;
	rf_ptr = (void *)&filestate;
	wf_ptr = (void *)&filestate;
	rf = (readfile)readeditfile;
	wf = (writefile)writeeditfile;

	editor();
	
	if (!filestate.error) {
		f_truncate(&fil);
		outstr("File saved\n");
	} else {
		outstr("WARNING: File not saved\n");
	}	
	f_close(&fil);
	outcharln();
}
#endif
#endif
#endif

MSTATIC void listprogram(void)
{
	int c;
	untokstat *u;
 
	if (allocate_program_space() != NULL) {
		outstr("\nProgram Listing:\n");
		u = &program_space->uts;
		inituntokenizestate(u, script);
		while ((c=untokenizecode(u)) != 0)
			outchar(c);
		outstr("\nEnd of Listing\n");
		deallocate_program_space();
	} else outcurrentlyprogramming();
}

int untokenizeverbatim(untokstat *utk)
{
	int c;
	c = *(utk->s);
	if (c != 0) (utk->s)++;
	return c;
}

void initretoken(retoken *r, unsigned int address, unsigned int endaddress, int verbatim, int autostore)
{
	r->ln = 0;
	r->autostore = autostore;
	r->verbatim = verbatim;
	inittokenizestate(&r->tk);
	initiapaddbuf(&r->adb,address,endaddress);
}

const char *conf_string(const char *var, int *len)
{
	int varlen = strlen(var);
	const char *checkstr = configuration_space;
	
	while ((checkstr) && (*checkstr != 0) && (*checkstr != 255)) {
		if (*checkstr == '\n') checkstr++;
		checkstr=skipspaceslf(checkstr);
		if (*checkstr != '#') {
			if ((!strncasecmp(checkstr,var,varlen)) && (checkstr[varlen] == '=')) {
				const char *beg = skipspaceslf(checkstr+varlen+1);
				const char *end = beg;
				while ((*end != '\n') && (*end)) end++;
				if (len) *len = (end-beg);
				return beg;
			}
		}
		checkstr = strchr(checkstr,'\n');
	}
	return NULL;
}

int get_conf_string_max(const char *var, char *buf, int len)
{
	int readlen;
	const char *getstr = conf_string(var, &readlen);
	if (getstr == NULL) {
		*buf = '\0';
		return 0;
	}
	len--;
	if (readlen > len)
		readlen = len;
	strncpy(buf,getstr,readlen);
	buf[readlen] = 0;
	return 1;
}

static int checkbool(unsigned char *check)
{
   return ((check) && ((toupper(*check) == 'Y') || (toupper(*check) == 'T')));
}


static void get_password(void)
{
	int len;
	const char *conf;

	conf=conf_string("WATCHDOG", &len);
	if (conf) enable_watchdog = checkbool(conf);
#ifdef INCNETWORK
	conf=conf_string("NONET", &len);
	if (conf) nonet = checkbool(conf);
#endif
#if INCFATFS
	conf=conf_string("NOFATFS", &len);
	if (conf) nofatfs = checkbool(conf);
#endif
	conf=conf_string("STARTUPTIME", &len);
	if (conf) startup_time = mystrtol(conf,NULL);
	conf=conf_string("BANK",&len);
	if (conf) cur_flash_bank_number = mystrtol(conf,NULL);
	get_conf_string_max("PASSWORD", auth_password, sizeof(auth_password));
	conf=conf_string("CONSOLE",&len);
	if (conf) console_uart = mystrtol(conf,NULL);
	conf=conf_string("BAUDRATE",&len);
	if (conf) console_bps = mystrtol(conf,NULL);
}

void verbatimline(char *str, char **end, addbasictoken abt, void *v)
{

  while (str < *end)
	(*abt)(*str++,v);
  *end = str;
}

void retokenize(int c, retoken *r)
{
	char *end;
	if (c == -1) {
		end = r->buf + r->ln;
		if (r->verbatim) 
			verbatimline(r->buf,&end,addchar,(void *)&r->adb);
		else
			tokenizeline(r->buf,&end,&r->tk,addchar,(void *)&r->adb);
		addchar(-1, &r->adb);
		return;
	}
	if ((c >= 0) && (r->ln<(LINEBUFLEN-1)))
		r->buf[r->ln++] = c;
	if ((c == -2) || ((r->autostore) && (r->ln >= LINEBUFTHRESHOLD)))
	{
		end = &r->buf[LINEBUFHALFLEN];
		if (r->verbatim)
			verbatimline(r->buf,&end,addchar,(void *)&r->adb);
		else
			tokenizeline(r->buf,&end,&r->tk,addchar,(void *)&r->adb); 
		memmove(r->buf,end,&r->buf[LINEBUFLEN]-end);
		r->ln -= (end - r->buf);
	}
}

MSTATIC void doprogram(void)
{
  int c, xoff, sec;
  retoken *rr;
  
  if (allocate_program_space() == NULL) {
		outcurrentlyprogramming();
		return;
  }
  rr = &program_space->rr;
  initretoken(rr, (unsigned int)script, (unsigned int)endscript, 0, 0);
  xoff = 0;
  outstr("Send program\nUse XON/XOFF flow control\nCTL-D ends file\n");
  do 
  {
	if (rr->ln == LINEBUFTHRESHOLD) {
		outchar('S'-'A'+1);  /* send XOFF */
		xoff=1;
	} 
    sec = 300000;
	do {
		 if (rr->ln >= (LINEBUFLEN-2)) {
			c = -1;
			sec = 0;
			break;
		}
		 c = inkey();
	} while ((c == -1) && (--sec > 0));
	if ((sec == 0) && (xoff)) {
	    retokenize(-2,rr);
	    outchar('Q'-'A'+1);  /* send XON */
		xoff = 0;
	}
	if (c == '\r') c = '\n';
	if (c > 0)
		retokenize((c == ENDPROGCHAR) ? 0 : c, rr);
  } while (c != ENDPROGCHAR);
  retokenize(-1,rr);
  outstrhex("\nProgrammed bytes 0x",rr->adb.address-((unsigned int)script));
  deallocate_program_space();
 }

MSTATIC void endloop(void)
{
  int c;
  outstr("CTL-D continues\n");
  do {
    c = inchar();
  } while (c != ENDPROGCHAR);
}

MSTATIC char const greeting[] = 
#ifdef INCNETWORK
"Jumentum-SOC 0.99-4-net\n"
#else
"Jumentum-SOC 0.99-4\n"
#endif
"Press ? or H for help\n\n";
 
MSTATIC char const helpgreeting[] = 
"Jumentum Options\n\n"
"R-Run          L-List\n"
"B-Bank         P-Program\n"
"Ctrl-Q Reset   H-Help\n"
#ifdef EDITOR
"E-Edit         K-Configure\n"
#endif
#if defined (INCFATFS) && defined(FATFSCOMMANDS)
"O-Load         S-Save\n"
"D-Directory    C-Change Dir\n"
"M-Mount        U-Unmount\n"
#ifdef EDITOR
"V-View File    F-Edit File\n"
#else
"V-View File\n"
#endif
#endif
"\n";
 
#ifdef LPC1768
#define COUNT_CONSTANT 500000
#else
#define COUNT_CONSTANT 500000
#endif
#define AFTER_PROGRAM_CONSTANT (10*COUNT_CONSTANT)
 
extern unsigned int ClockTimer;

int idle(int alwayscheck)
{
   WATCHDOG_UPDATE();
#ifdef INCNETWORK
   if ((alwayscheck) || (idlestate() == 0)) casualnetmainloop();
#endif
   return run_basic == 0;
}

int bankname(int n, int len, char *name)
{
	unsigned char *d;
	if ((n<0) || (n>=num_flashbanks()))
		return -1;
	d = (unsigned char *) startbank(n);
	while ((*d != 0) && (*d != 0xFF ) && (*d != '\n') && (len>0)) {
		if ((*d >= ' ') && (*d < '~')) {
			*name++ = *d;
		}
		d++;
		len--;
	}
	*name = '\0';
	return 0;
}

void changebank(void)
{
   int c;
   char bankn[80];
   outstr("Current bank: ");
   outint(cur_flash_bank_number);
   c = 0;
   for (;;) {
	 if (bankname(c,sizeof(bankn)-1,bankn) < 0) break;
	 outstr("\nBank #");
	 outint(c);
	 outstr(" contents: ");
	 outstr(bankn);
	 c++;
   }
   outstr("\nSelect new bank: ");
   c = inchar();
   if ((c>='0') && (c<(num_flashbanks()+'0'))) {
		outchar(c);
		cur_flash_bank_number=c-'0';
		select_bank_number(cur_flash_bank_number);
   } else outstr("aborted");
   outstr("\n");
}

int main (void)
{
  int c;
  long count;

  cpuSetupHardware();
  heap_end = NULL;
  get_password();
  initstdout(console_bps);
  outstr("Jumentum starting...\n\n");
  rtcInit();
  select_bank_number(cur_flash_bank_number);
#ifdef INCNETWORK
  if (!nonet) {
	setup_net_configuration_parms();
    outstr("Initialize PHY/network...\n");
	initialize_network();
  }
#endif
#ifdef INCFATFS
  if (!nofatfs) {
	mountdisk();
  }
#endif
  if (enable_watchdog) {
    outstr("Watchdog enabled.\n");
	WATCHDOG_ENABLE();
  }
  for (;;) {
	if (startup_time != 0)  {
		count = ((long)startup_time) * COUNT_CONSTANT;
	} 
	if (WATCHDOG_THROWN()) outstr("Watchdog Timeout\n");
#ifdef INCNETWORK
	outip("IP: "); 
#endif
#ifdef INCFATFS
	printcurdir();
#endif
	outstr(greeting);
	for (;;)
	{
		c = toupper(inkey());
		count--;
		if ((c == 'H') || (c == '?')) {
			outstr(helpgreeting);
		} 
		else if ((c == 'R') || ((count == 0) && (startup_time != 0)) || (run_basic))
		{
			if (program_space != NULL) {
				outcurrentlyprogramming();
			} else {
				outstr("Start program\n");
				if ((*((int *)script) != -1) && ( setup(script) != -1 )) {
					run_basic = 1;
					pause_basic = 0;
					basic_running = 1;
					while(!BASICIDLE())
					{
						if (!pause_basic) {
							if (mainloop()) break;
						}
					}
					if(run_basic)
					{
						outstr("Program Ended, Press CTRL-D\n");
						count = AFTER_PROGRAM_CONSTANT;
						do {
							c = inkey();
						} while ((c != ENDPROGCHAR) && (run_basic) && (count-- > 0));
					}
					cleanup();
					basic_running = 0;
				}
				outstr("End program\n");
			}
			run_basic = 0;
			break;
		}
		else if (c == ('Q'-64))
		{
			force_reboot();
		}
		else if (c == 'P') 
		{ 
			doprogram();
			break;
		}
		else if (c == 'B') 
		{ 
			changebank();
			break;
		}
		else if (c == 'L') 
		{
			listprogram();
			endloop();
			break;
		} 
#ifdef EDITOR
		else if ((c == 'E') || (c == 'K'))
		{	
			if (allocate_program_space() != NULL) {
				if (c == 'K') {
					inituntokenizestate(&program_space->uts, configuration_space);
					initretoken(&program_space->rr, (unsigned int)configuration_space, (unsigned int)configuration_space_end, 1, 1);
					rf = (readfile)untokenizeverbatim;
				} else {
					inituntokenizestate(&program_space->uts, script);
					initretoken(&program_space->rr, (unsigned int)script, (unsigned int)endscript, 0, 1);
					rf = (readfile)untokenizecode;
				}
				rf_ptr = &program_space->uts;
				wf = (writefile)retokenize;
				wf_ptr = &program_space->rr;
				editor();
				deallocate_program_space();
			} else outcurrentlyprogramming();
			break;
		}
#endif
#if defined (INCFATFS) && defined(FATFSCOMMANDS)
	    else if (c == 'D') {
			directory();
			break;
		} else if (c == 'M') {
			mountdisk();
			break;
		} else if (c == 'U') {
			unmountdisk();
			break;
		} else if (c == 'C') {
			changedir();
			break;
		} else if (c == 'O') {
			loadprogram();
			break;
		} else if (c == 'S') {
			saveprogram();
			break;
		} else if (c == 'V') {
			viewfile();
			break;
		} 
#ifdef EDITOR
		else if (c == 'F') {
			editfile();
			break;
		}
#endif
#endif
		else if (c != -1) {
			break;
		}
		GENERALIDLE();
	}
  }
  return 0;
}
