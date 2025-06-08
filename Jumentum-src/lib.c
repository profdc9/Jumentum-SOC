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

/* terminal defintions and library functions */
/* DLM */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "all.h"
#include "lib.h"
#include "serial.h"
#include "net.h"
#include "basic.h"
#include "main.h"
#ifdef INCNETWORK
#include "uip/uip/uip.h"
#include "uip/apps/telnetd/telnetd.h"
#endif

void initstdout(long x)
{
	raw_initstdout(x);
}

int outchar(int ch)
{
#ifdef INCNETWORK
	telnetd_addsendfifo(ch);
#endif
	return raw_outchar(ch);
}

int inchar(void)
{
	int ch;
	do
	{
		ch = inkey();
	} while (ch < 0);
	return ch;
}

int inkey(void)
{
	int ch;
	if ((ch=raw_inkey()) >= 0) return ch;
#ifdef INCNETWORK
	if ((ch=telnetd_getrecvfifo()) >= 0) return ch;
#endif
	GENERALIDLE();
	return -1;
}

int inputstr(char *c, int len)
{
	int curlen = 0;
	int ch;
	len--;
	for (;;)
	{
		ch = inkey();
		if ((ch >= ' ') && (ch <= '~') && (curlen < len))
		{
			outchar(ch);
			c[curlen++] = ch;
		} else if (((ch == 127) || (ch == 8)) && (curlen > 0))
		{
			outstr("\010 \010");
			curlen--;
		} else if ((ch == '\r') || (ch == '\n')) {
			outstr("\n");
			break;
		}
	}
	c[curlen] = 0;
	return curlen;
}

#define fastmul10(x) (((x)<<3)+((x)<<1))

#ifdef USEFLOATS
#include <math.h>
#include <float.h>
#endif

#ifdef USEFLOATS
#if LONG_MAX==32767
#define MAXEXP 1E4
#define MINEXP 1E-4
#else
#define MAXEXP 1E9
#define MINEXP 1E-9
#endif

static double powint(double p, int n)
{
	double pl = 1;
	int neg;
		
	if (neg=(n<0)) n=-n;
	while (n) {
		if (n & 1)
			pl *= p;
		n >>= 1;
		p = p*p;
	}
	return neg ? 1.0/pl : pl;
}

/* cheezy but low resources dtoa */
char *mydtoa(char *p, double n)
{
	 int expn = 0;
	 char *pc = p;
	 long intpart;
	 double fracpart, minerr;
	 char s[ITOASIZE];
	 
	 if ((n == 0) || (n == -0))
		return strcpy(p,"0");
	 if (isinf(n))
		return strcpy(p,"Inf");
	 if (isnan(n))
		return strcpy(p,"Nan");
	 if (n < 0) {
	    n = -n;
	    *pc++ = '-';
	 }
	 if (n > MAXEXP) {
		while (n >= 1E32) {
			n *= 1E-32;
			expn+=32;
		}
		if (n >= 1E16) {
			n *= 1E-16;
			expn+=16;
		}
		if (n >= 1E8) {
			n *= 1E-8;
			expn+=8;
		}
		if (n >= 1E4) {
			n *= 1E-4;
			expn+=4;
		}
		if (n >= 1E2) {
			n *= 1E-2;
			expn+=2;
		}
		if (n >= 1E1) {
			n *= 1E-1;
			expn+=1;
		}	
	 } else if (n < MINEXP) {
		while (n < 1E-31) {
			n *= 1E32;
			expn-=32;
		}
		if (n < 1E-15) {
			n *= 1E16;
			expn-=16;
		}
		if (n < 1E-7) {
			n *= 1E8;
			expn-=8;
		}
		if (n < 1E-3) {
			n *= 1E4;
			expn-=4;
		}
		if (n < 1E-1) {
			n *= 1E2;
			expn-=2;
		}
		if (n < 1) {
			n *= 1E1;
			expn-=1;
		}
	 }
	 n += (n*(0.1*MINEXP));
	 if ((n >= 10.0) && (expn != 0)) {
		n *= 0.1;
		expn++;
	 }
	 intpart = (long)floor(n);
	 fracpart = n - intpart;
	 strcpy(pc,myltoa(s,intpart));
	 while (*pc != 0) pc++;
	 if ((minerr = n * MINEXP) < DBL_MIN)
		minerr = DBL_MIN;
	 if (fracpart > minerr) {
	    *pc++ = '.';
		do {
			int dig;
			fracpart *= 10.0;
			minerr *= 10.0;
			dig = (int)fracpart;
			*pc++ = ('0'+dig);
			fracpart -= dig;
		} while (fracpart > minerr);
	 }
	 if (expn != 0) {
		*pc++ = 'E';
		strcpy(pc,myltoa(s,expn));
	 } else {
		*pc = '\0';
	 }
	 return p;
}

double mystrtod(const char *str, const char **end)
{
	int neg;
	double d = 0.0;
	
	str = skipspaces(str);
	if (*str=='+') str++;
	if (neg=(*str=='-')) str++;
	if ((str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X'))) {
		d =  (double) ((unsigned long)mystrtol(str,end));
	} else {
		while ((*str >= '0') && (*str <= '9')) 
			d = (d * 10.0) + (*str++ - '0');
		if (*str == '.') {
			double cv = 1.0;
			str++;
			while ((*str >= '0') && (*str <= '9')) {
				cv *= 0.1;
				d = d + (cv*(*str++ - '0'));
			}
		}
		if ((*str == 'e') || (*str == 'E')) {
			int expn = 0, negexpn;
			str++;
			if (*str=='+') str++;
			if (negexpn=(*str=='-')) str++;
			while ((*str >= '0') && (*str <= '9')) 
				expn = fastmul10(expn)+(*str++ - '0');
			d *= powint(10,negexpn ? -expn : expn);
		}
		if (end) *end = str;
	}
	if (neg) d = -d;
	return d;
}

#endif

void clreol(void)
{
	outstr("\033[K");
}

void clrscr(void)
{
	outstr("\033[H\033[2J");
}

void gotoxy(int x, int y)
{
    outstr("\033[");
	outint(y);
	outchar(';');
	outint(x);
	outchar('H');
}

void highvideo(void)
{
	outstr("\033[1m");
}

void lowvideo(void)
{
	outstr("\033[0m");
}

void outstr(const char *x)
{
	while (*x)
		outchar(*x++);
 }

void outint(const int x)
{
    char s[ITOASIZE];
	outstr(myltoa(s,x));
}

#ifdef LPC1768
void check_failed(unsigned char *file, unsigned long line)
{
  outstr("Check failed ");
  outstr(file);
  outstr(" on line ");
  outint(line);
  outstr("\r\n");
}
#endif

char *myltoa(char *p, long n)
{
	int neg;

#if LONG_MIN==-2147483648
	if (n == LONG_MIN) 
		return "-2147483648";  /* ugly hack for 32 bit */
#endif
#if LONG_MIN==-32768
	if (n == LONG_MIN) 
		return "-32768";       /*  ugly hack for 16 bit*/
#endif
	p += (ITOASIZE-1);
    *p = '\0';
	if (neg = (n < 0)) n = -n;
	do {
		*(--p) = (n % 10) + '0';      
	} while ((n /= 10) > 0);
	if (neg) *(--p) = '-';
	return p;
}


char *myltoapad(char *p, long n, int dig)
{
	int neg;

#if LONG_MIN==-2147483648
	if (n == LONG_MIN) 
		return "-2147483648";  /* ugly hack for 32 bit */
#endif
#if LONG_MIN==-32768
	if (n == LONG_MIN) 
		return "-32768";       /*  ugly hack for 16 bit*/
#endif
	p += (ITOASIZE-1);
    *p = '\0';
	if (neg = (n < 0)) n = -n;
	do {
		*(--p) = (n % 10) + '0';      
		n /= 10;
	} while (--dig > 0);
	if (neg) *(--p) = '-';
	return p;
}

char *myltoahex(char *p, unsigned long n)
{
	static char const hexlist[16]="0123456789ABCDEF";
	
	p += (ITOASIZE-1);
    *p = '\0';
	do {
		*(--p) = hexlist[n & 0x0F];
	} while ((n >>= 4) > 0);
	return p;
}

unsigned long mystrtolhex(const char *str, const char **end)
{
    unsigned long n=0;
	for (;;) {
		if ((*str>='0') && (*str<='9'))
			n = (n << 4) + (*str++ - '0');
		else if ((*str>='A') && (*str<='F'))
			n = (n << 4) + (*str++ - 'A' + 10);
		else if ((*str>='a') && (*str<='f'))
			n = (n << 4) + (*str++ - 'a' + 10);
		else break;
	}
	if (end) *end=str;
	return n;
}

long mystrtol(const char *str, const char **end)
{
	unsigned long n=0;
	int neg;
	
	str = skipspaces(str);
	if (neg=(*str == '-')) str++;
	if (isdigit(*str)) {
		if ((str[0]=='0') && ((str[1]=='x')||(str[1]=='X'))) {
			str += 2;
			n = mystrtolhex(str,end);
			end = NULL;
		} else {
			for (;;) {
				if ((*str>='0') && (*str<='9'))
					n = fastmul10(n) + (*str++ - '0');
				else
					break;
			}
		}
	}
	if (end) *end=str;
	return neg ? -((long)n) : (long) n;
}

void deboutstrint(const char *s, int x)
{
    char t[ITOASIZE];
	while ((*s != 0) && (*s != '=')) raw_outchar(*s++);
	if (*s == '=') {
	    raw_outchar('=');
		s=myltoa(t,x);
		while (*s) raw_outchar(*s++);
	}
	raw_outchar('\n');
}

void deboutstrhex(const char *s, int x)
{
    char t[ITOASIZE];
	while ((*s != 0) && (*s != '=')) raw_outchar(*s++);
	if (*s == '=') {
	    raw_outchar('=');
		s=myltoahex(t,x);
		while (*s) raw_outchar(*s++);
	}
	raw_outchar('\n');
}

void deboutstr(const char *s, const char *d, int len)
{
	while (*s) raw_outchar(*s++);
	if (d != NULL) {
		while ((*d) && (*d != '\n') && (len>0)) 
		{
			raw_outchar(*d++);
			len--;
		}
	}
	raw_outchar('\n');
}

void outstrint(const char *s, int x)
{
	outstr(s);
	outint(x);
	outchar('\n');
}

void outstrhex(const char *s, unsigned int x)
{   
    char p[ITOASIZE];
	outstr(s);
	outstr(myltoahex(p,x));
	outchar('\n');
}

const char *skipspaceslf(const char *str)
{
	while ((isspace(*str)) && (*str != '\n')) str++;
	return str;
}

const char *skipspaces(const char *str)
{
	while (isspace(*str)) str++;
	return str;
}

char *mystrcpynonull(char *dest, const char *src, size_t n)
{
   char *d = dest;
   while ((*src) && (n>0)){
	  *d++ = *src++;
	  n--;
   }
   return dest;
}

#ifdef MYFUNC
int myisalnum(int ch)
{
	return (((ch >= 'a') && (ch <= 'z')) ||
	       ((ch >= 'A') && (ch <= 'Z')) ||
		   ((ch >= '0') && (ch <= '9')));
}

int myisalpha(int ch)
{
	return (((ch >= 'a') && (ch <= 'z')) ||
	       ((ch >= 'A') && (ch <= 'Z')));
}

int myisprint(int ch)
{
	return ((ch >= ' ') && (ch < 0x7F));
}

int myisdigit(int ch)
{
	return ((ch >= '0') && (ch <= '9'));
}

int myisspace(int ch)
{
	return ((ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n') || (ch == '\f'));
}

int mytoupper(int ch)
{
	return ((ch >= 'a') && (ch <='z')) ? (ch - 32) : ch;
}

int mytolower(int ch)
{
	return ((ch >= 'A') && (ch <='Z')) ? (ch + 32) : ch;
}

int mymemcmp(const void *s1, const void *s2, size_t n)
{
	const char *ss1 = (char *)s1, *ss2 = (char *)s2;
	int c;
	while (n-- > 0) {
		if (c = (((int)*ss1++) - ((int)*ss2++))) return c;
	}
	return 0;
}

void *mymemset(void *s1, int ch, size_t n)
{
	char *ss1 = (char *)s1;
	while (n-- > 0)
		*ss1++ = ch;
	return s1;
}

void *mymemmove(void *s1, const void *s2, size_t n)
{
	char *ss1 = (char *)s1;
	const char *ss2 = (const char *)s2;
	if (ss1 > ss2) {
		ss1 += n;
		ss2 += n;
		while (n-- > 0) *(--ss1) = *(--ss2);
	} else {
		while (n-- > 0) *ss1++ = *ss2++;
	}
	return s1;
}

void *mymemcpy(void *s1, const void *s2, size_t n)
{
	char *ss1 = (char *)s1;
	const char *ss2 = (const char *)s2;
	while (n-- > 0) *ss1++ = *ss2++;
	return s1;
}

char *mystrcpy(char *dest, const char *src)
{
   char *d = dest;
   do {
	  *d++ = *src;
   } while (*src++);
   return dest;
}

char *mystrncpy(char *dest, const char *src, size_t len)
{
   char *d = dest;
   while ((len > 0) && (*src)) {
	 *d++ = *src++;
	 len--;
   }
   while (len>0) {
	 *d++ = '\0';
	 len--;
   }
   return dest;
}

char *mystrcat(char *dest, const char *src)
{
	char *d = dest;
	while (*d) d++;
	mystrcpy(d, src);
	return dest;
}

char *mystrncat(char *dest, const char *src, size_t len)
{
	char *d = dest;
	while (*d) d++;
	mystrncpy(d, src, len);
	return dest;
}

size_t mystrlen(const char *s)
{
	const char *n = s;
	while (*n) n++;
	return n - s;
}

const char *mystrchr(const char *s, int ch)
{
	while (*s) {
		if (*s == ch) return s;
		s++;
	}
	return NULL;
}

const char *mystrrchr(const char *s, int ch)
{
    const char *t = s;
	
	while (*t) t++;
	while (t>=s) {
		if (*t == ch) return t;
		t--;
	}
	return NULL;
}

int mystrcmp(const char *s1, const char *s2)
{
	int c;
	while ((*s1) || (*s2)) {
		if (c = (((int)*s1++) - ((int)*s2++))) return c;
	}
	return 0;
}

int mystrcasecmp(const char *s1, const char *s2)
{
	int c;
	while ((*s1) || (*s2)) {
		if (c = (mytoupper(*s1++) - mytoupper(*s2++))) return c;
	}
	return 0;
}

int mystrncasecmp(const char *s1, const char *s2, size_t n)
{
	int c;
	while (((*s1) || (*s2)) && (n-- > 0)) {
		if (c = (mytoupper(*s1++) - mytoupper(*s2++))) return c;
	}
	return 0;
}

int mystrncmp(const char *s1, const char *s2, size_t n)
{
	int c;
	while (((*s1) || (*s2)) && (n-- > 0)) {
		if (c = ((*s1++) - (*s2++))) return c;
	}
	return 0;
}

#endif

static short randseed1 = 0;
static short randseed2 = 0;

#define RANDM1 32768
#define A1 21525l
#define C1 15373l
#define RANDM2 19683l
#define A2 9112l
#define C2 14033l

int myrand(void)
{	
	randseed1 = (((long)randseed1)*A1+C1) & 0x7FFF;
	randseed2 = (((long)randseed2)*A2+C2) % RANDM2;
	return (randseed1 ^ randseed2);
}

void mysrand(unsigned int seed)
{
	randseed1 = (seed >> 15) & 0x7FFF;
	randseed2 = (seed & 0x7FFF) ^ randseed1;
}
