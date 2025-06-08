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

#include "libdriver/LPC17xx.h"
#include "libdriver/lpc17xx_pinsel.h"
#include "libdriver/lpc17xx_gpio.h"
#include "libdriver/lpc17xx_exti.h"
	
#include "keyboard.h"

//#define CLOCK_PIN 13
//#define DATA_PIN 4
#define CLOCK_PIN 5
#define DATA_PIN 4
#define KBD_PORT_NUM 2

#define FIFOSIZE 16

#define TICK_RESET 100000

static unsigned long last_tick;
static int state, paritybit;
static unsigned char curbyte;
static unsigned char shiftkey;
static unsigned char ctrlkey;
static unsigned char lastkeyup;

struct scancodetable 
{
  unsigned char scancode;
  unsigned char nonshift;
  unsigned char shifted;
  unsigned char ctrled;
  char *special;
};

static const struct scancodetable scancodes[]=
{
   { 0x1C, 'a', 'A', 'A'-64,  NULL },
   { 0x32, 'b', 'B', 'B'-64,  NULL },
   { 0x21, 'c', 'C', 'C'-64,  NULL },
   { 0x23, 'd', 'D', 'D'-64,  NULL },
   { 0x24, 'e', 'E', 'E'-64,  NULL },
   { 0x2B, 'f', 'F', 'F'-64,  NULL },
   { 0x34, 'g', 'G', 'G'-64,  NULL },
   { 0x33, 'h', 'H', 'H'-64,  NULL },
   { 0x43, 'i', 'I', 'I'-64,  NULL },
   { 0x3B, 'j', 'J', 'J'-64,  NULL },
   { 0x42, 'k', 'K', 'K'-64,  NULL },
   { 0x4B, 'l', 'L', 'L'-64,  NULL },
   { 0x3A, 'm', 'M', 'M'-64,  NULL },
   { 0x31, 'n', 'N', 'N'-64,  NULL },
   { 0x44, 'o', 'O', 'O'-64,  NULL },
   { 0x4D, 'p', 'P', 'P'-64,  NULL },
   { 0x15, 'q', 'Q', 'Q'-64,  NULL },
   { 0x2D, 'r', 'R', 'R'-64,  NULL },
   { 0x1B, 's', 'S', 'S'-64,  NULL },
   { 0x2C, 't', 'T', 'T'-64,  NULL },
   { 0x3C, 'u', 'U', 'U'-64,  NULL },
   { 0x2A, 'v', 'V', 'V'-64,  NULL },
   { 0x1D, 'w', 'W', 'W'-64,  NULL },
   { 0x22, 'x', 'X', 'X'-64,  NULL },
   { 0x35, 'y', 'Y', 'Y'-64,  NULL },
   { 0x1A, 'z', 'Z', 'Z'-64,  NULL },
   { 0x45, '0', ')', '0',     NULL },
   { 0x16, '1', '!', '1',     NULL },
   { 0x1E, '2', '@', '2',     NULL },
   { 0x26, '3', '#', '3',     NULL },
   { 0x25, '4', '$', '4',     NULL },
   { 0x2E, '5', '%', '5',     NULL },
   { 0x36, '6', '^', '6',     NULL },
   { 0x3D, '7', '&', '7',     NULL },
   { 0x3E, '8', '*', '8',     NULL },
   { 0x46, '9', '(', '9',     NULL },
   { 0x0E, '`', '~', '`'-64,  NULL },
   { 0x4E, '-', '_', '-',     NULL },
   { 0x55, '=', '+', '=',     NULL },
   { 0x5D, '\\', '|', '\\'-64,  NULL },
   { 0x66, 0x08, 0x08, 0x08,  NULL },
   { 0x29, ' ', ' ', ' ',     NULL },
   { 0x0D, 0x09, 0x09, 0x09,  NULL },
   { 0x5A, 0x0D, 0x0D, 0x0D,  NULL },
   { 0x76, 27,   27,   27,    NULL },
   { 0x54, '[', '{', '['-64,  NULL },
   { 0x5B, ']', '}', ']'-64,  NULL },
   { 0x4C, ';', ':', ';',     NULL },
   { 0x52, '\'', '\"', '\'',  NULL },
   { 0x41, ',', '<', ',',     NULL },
   { 0x49, '.', '>', '.',     NULL },
   { 0x4A, '/', '?', '/',     NULL },
   { 0x75, 0, 0, 0,          "\033[A" },
   { 0x6B, 0, 0, 0,          "\033[D" },
   { 0x72, 0, 0, 0,          "\033[B" },
   { 0x74, 0, 0, 0,          "\033[C" } };

#define KB_LEFTSHIFT 0x12
#define KB_RIGHTSHIFT 0x59
#define KB_CTRL 0x14
#define KB_ALT 0x11
#define KB_KEY_UP 0xF0
   
struct kbdfifo
{
  volatile unsigned char buf[FIFOSIZE];
  volatile int fifohead;
  volatile int fifotail;
};

struct kbdfifo kbdf;

static void initkbdfifo(struct kbdfifo *fifo)
{
	fifo->fifohead = fifo->fifotail = 0;
}

static int getkbdfifo(struct kbdfifo *fifo)
{	
	int ch;
	int newpos;
	if (fifo->fifotail == fifo->fifohead)
		return -1;
	ch = fifo->buf[fifo->fifotail];
	newpos = fifo->fifotail+1;
	if (newpos >= FIFOSIZE) newpos = 0;
	fifo->fifotail = newpos;
	return ch;
}

static void putkbdfifo(struct kbdfifo *fifo, int ch)
{
	LPC_UART0->THR = ch;
	int newpos = fifo->fifohead + 1;
	if (newpos >= FIFOSIZE) newpos = 0;	
	if (newpos == fifo->fifotail)
		return;
	fifo->buf[fifo->fifohead] = ch;
	fifo->fifohead = newpos;
}

int keyboard_getchar (void)
{
	return getkbdfifo(&kbdf);
}

int waitkey_keyboard (void)
{
    int ch;
    while ((ch=keyboard_getchar()) == -1);
	return ch;
}

void EINT3_IRQHandler (void) 
{
	unsigned long pres_value, dly_value;
	int databit, clockbit, i;
	
#if KBD_PORT_NUM == 0
	databit = LPC_GPIO0->FIOPIN & (1 << DATA_PIN);
	clockbit = LPC_GPIO0->FIOPIN & (1 << CLOCK_PIN);
#endif
#if KBD_PORT_NUM == 2
	databit = LPC_GPIO2->FIOPIN & (1 << DATA_PIN);
	clockbit = LPC_GPIO2->FIOPIN & (1 << CLOCK_PIN);
#endif
	
#if 0
	pres_value = SysTick->CURR;
	dly_value = (pres_value > last_tick) ? last_tick + SysTick->RELOAD - pres_value : last_tick - pres_value;
	if (dly_value > TICK_RESET)
		state = 0;
	last_tick = pres_value;
#endif
	
	if (state == 0) {
		if ((!clockbit) && (!databit)) {
			paritybit = 0;
			curbyte = 0;
			state++;
		}
	} else if (state <= 8) {
		curbyte >>= 1;
		if (databit) {
			curbyte |= 0x80;
			paritybit ^= 0x01;
		}
		state++;
	} else if (state == 9) {
		state = (paritybit != (databit != 0)) ? 10 : 0;
	} else if (state == 10) {
		if (databit) {
			if (curbyte == KB_KEY_UP) {
				lastkeyup = 1;
			} else {
				if ((curbyte == KB_LEFTSHIFT) || (curbyte == KB_RIGHTSHIFT)) {
					shiftkey = !lastkeyup;
				} else if (curbyte == KB_CTRL) {
					ctrlkey = !lastkeyup;
				} else {
					if (!lastkeyup) {
						for (i=0;i<(sizeof(scancodes)/sizeof(struct scancodetable));i++)
						{
							if (scancodes[i].scancode == curbyte) {
								if (scancodes[i].special != NULL) {
									char *c = scancodes[i].special;
									while (*c) putkbdfifo(&kbdf,*c++);
								} else {
									if (ctrlkey) 
										putkbdfifo(&kbdf,scancodes[i].ctrled);
									else
										putkbdfifo(&kbdf,shiftkey ? scancodes[i].shifted : scancodes[i].nonshift);
								}
								break;
							}
						}
					}
				}
				lastkeyup = 0;
			}
		}
		state=0;
	}
#if KBD_PORT_NUM == 0
	LPC_GPIOINT->IO0IntClr = (1<<CLOCK_PIN);
#endif
#if KBD_PORT_NUM == 2
	LPC_GPIOINT->IO2IntClr = (1<<CLOCK_PIN);
#endif
	LPC_SC->EXTINT |= (1 << EXTI_EINT3);	
}

int init_keyboard(void)
{
	PINSEL_CFG_Type PinCfg;

	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = KBD_PORT_NUM;
	
	PinCfg.Pinnum = CLOCK_PIN;
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Pinnum = DATA_PIN;
	PINSEL_ConfigPin(&PinCfg);
	
	GPIO_SetDir(KBD_PORT_NUM, (1<<CLOCK_PIN), 0);
	GPIO_SetDir(KBD_PORT_NUM, (1<<DATA_PIN), 0);
	
#if KBD_PORT_NUM == 0
	LPC_GPIOINT->IO0IntEnF |= (1<<CLOCK_PIN);
#endif
#if KBD_PORT_NUM == 2
	LPC_GPIOINT->IO2IntEnF |= (1<<CLOCK_PIN);
#endif
	
	LPC_SC->EXTMODE |= (1 << EXTI_EINT3);		
	LPC_SC->EXTPOLAR &= ~(1 << EXTI_EINT3);				
	LPC_SC->EXTINT |= (1 << EXTI_EINT3);	

	last_tick = SysTick->CURR;
	state = 0;
	curbyte = 0;
	paritybit = 0;
	shiftkey = 0;
	ctrlkey = 0;
	lastkeyup = 0;
	initkbdfifo(&kbdf);
	
	NVIC_SetPriority(EINT3_IRQn, ((0x01<<4)|0x01));
	NVIC_EnableIRQ(EINT3_IRQn);
	return 0;
}
