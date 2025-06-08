
#ifndef _FLASH_H
#define _FLASH_H

#define vs ((iapaddbuf *)v)

#if defined LPC2148 || defined LPC2378 || defined LPC1768
#define IAPBUFLEN 256
#endif
#if defined LPC2119 || defined LPC2106
#define IAPBUFLEN 512
#endif

#ifndef IAPBUFLEN
#define IAPBUFLEN 512
#endif

typedef struct _iapaddbuf
{
  int ct, lastsec;
  int noerase;
  unsigned int address, endaddress;
  unsigned char buf[IAPBUFLEN];
} iapaddbuf;

void initiapaddbuf(iapaddbuf *adb, unsigned int address, unsigned int endaddress);
void addchar(int c, void *v);

extern int num_flashbanks();
extern unsigned long startbank(int numbank);
extern unsigned long endbank(int numbank);

#endif /* _FLASH_H */
