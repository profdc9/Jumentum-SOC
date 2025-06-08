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
#include "libdriver/lpc17xx_ssp.h"
#include "libdriver/lpc17xx_pinsel.h"
#include "libdriver/lpc17xx_gpdma.h"
#include "libdriver/lpc17xx_gpio.h"
#include "libdriver/lpc17xx_clkpwr.h"
#include "video.h"
#include "font.h"
#include "vt100.h"

#ifdef MBED
#define VIDEO_SSP0
#else
#define VIDEO_SSP1
#endif

#ifdef VIDEO_SSP1
#define CS_PORT_NUM 0
#define CS_PIN_NUM 6
#define SSPx LPC_SSP1
#else
#define CS_PORT_NUM 0
#define CS_PIN_NUM 16
#define SSPx LPC_SSP0
#endif

#define NTSC

#ifdef PAL
#define VBLSCANLINE 292
#define TOPBLANKLINE 30
#define FRAMELINES 312
#define SYNCLEN 100
#define VBLSYNCLEN 100
#define LINERATEUSSEC 6350
#define VIDCOLUMNTOTALLEN 104
#define VIDCOLUMNTRANSMIT 97
#define VIDCOLUMNOFFSET 18
#define PIXCLOCK 12500000
#endif

#ifdef NTSC
#define VBLSCANLINE 242
#define TOPBLANKLINE 25
#define FRAMELINES 262
#define SYNCLEN 100
#define VBLSYNCLEN 100
#define LINERATEUSSEC 6350
#define VIDCOLUMNTOTALLEN 104
#define VIDCOLUMNTRANSMIT 97
#define VIDCOLUMNOFFSET 18
#define PIXCLOCK 12500000
#endif

#define ENDSCREENSCANLINE (TOPBLANKLINE + (VIDHACKROWS*VIDSCANLINES))

#define VIDEODATALOC 0x2007C000
#define VIDCOLUMNS 80
#define VIDROWS 25
#define VIDSCANLINES 8
#define VIDFONTWIDTH 8
#define VIDNUMCOLUMNS 2

#define VIDHACKROWS (VIDROWS+1)

typedef struct _video_data {
	unsigned char  e_vidcolumns[VIDNUMCOLUMNS][VIDCOLUMNTOTALLEN];
	unsigned char  e_screendata[VIDHACKROWS][VIDCOLUMNS];
} video_data;


//static video_data videodata;
static unsigned char *next_line;
static int curvidcolumn;
static int curvidrow;
static int curscanline;

static int curtotalscanline;
static int curframe;

static void (*nextpulse)();

//#define VIDEOMEM ((video_data *)(&videodata))
#define VIDEOMEM ((video_data *)(VIDEODATALOC))

#define vidcolumns (VIDEOMEM)->e_vidcolumns
#define screendata (VIDEOMEM)->e_screendata
virtscreen terminalscreen;

#define CR 0x0D

int putc_video(int ch)
{
	terminalscreen.next_char_send(&terminalscreen,ch);
	return 0;
}

void putstring_video (const char *string)
{
	char ch;

	while ((ch = *string)) {
		putchar_video(ch);
		string++;
	}
}

int putchar_video (int ch)
{
    if (ch == '\n') putc_video(CR);
	return putc_video(ch);
}

void negative_pulse(void);
void positive_pulse(void);
void nopulse(void);

void initiate_dma(unsigned char *buf, unsigned int size)
{
    GPDMA_Channel_CFG_Type GPDMACfg;
	
	GPDMACfg.ChannelNum = 0;
	GPDMACfg.SrcMemAddr = (uint32_t) buf;
	GPDMACfg.DstMemAddr = 0;
	GPDMACfg.TransferSize = size;
	GPDMACfg.TransferWidth = 0;
	GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
	GPDMACfg.SrcConn = 0;
#ifdef VIDEO_SSP1
	GPDMACfg.DstConn = GPDMA_CONN_SSP1_Tx;
#else
	GPDMACfg.DstConn = GPDMA_CONN_SSP0_Tx;
#endif
	GPDMACfg.DMALLI = 0;
	GPDMA_Setup(&GPDMACfg);
	SSP_DMACmd (SSPx, SSP_DMA_TX, ENABLE);
}

void DMA_IRQHandler (void)
{
	unsigned char *offset_line;
	unsigned char *videorow;
	int i;

	LPC_GPDMA->DMACIntTCClear = GPDMA_DMACIntTCClear_Ch(0);
	//LPC_GPDMA->DMACIntErrClr = GPDMA_DMACIntErrClr_Ch(0);
	//LPC_GPDMACH0->DMACCConfig &= ~GPDMA_DMACCxConfig_E;
	//LPC_GPDMACH0->DMACCControl = 0x00;
	//LPC_GPDMACH0->DMACCLLI = 0;
	LPC_GPDMACH0->DMACCSrcAddr = (uint32_t)next_line;
	LPC_GPDMACH0->DMACCDestAddr = (uint32_t) &SSPx->DR;
	LPC_GPDMACH0->DMACCControl
				= GPDMA_DMACCxControl_TransferSize((uint32_t)VIDCOLUMNTRANSMIT) \
						| GPDMA_DMACCxControl_SBSize((uint32_t)GPDMA_BSIZE_4) \
						| GPDMA_DMACCxControl_DBSize((uint32_t)GPDMA_BSIZE_4) \
						| GPDMA_DMACCxControl_SWidth((uint32_t)GPDMA_WIDTH_BYTE) \
						| GPDMA_DMACCxControl_DWidth((uint32_t)GPDMA_WIDTH_BYTE) \
						| GPDMA_DMACCxControl_SI \
						| GPDMA_DMACCxControl_I;
	// SSPx->DMACR |= SSP_DMA_TX;
	if (curtotalscanline == TOPBLANKLINE) {
		curframe++;
		curvidcolumn = 0;
		curvidrow = 0;
		curscanline = 0;
	}
	nextpulse = (curtotalscanline < VBLSCANLINE) ? positive_pulse : negative_pulse;
	if ((++curtotalscanline) > FRAMELINES)
		curtotalscanline = 0;

	// compose next line
	if ((++curvidcolumn) >= VIDNUMCOLUMNS)
		curvidcolumn = 0;
	next_line = vidcolumns[curvidcolumn];
	offset_line = next_line + VIDCOLUMNOFFSET;
	videorow = screendata[curvidrow];
	for (i=0;i<VIDCOLUMNS;i++)
		offset_line[i] = raster88_font[videorow[i] & 0x7f][curscanline];
	if ((terminalscreen.ypos == curvidrow) && (curframe & 0x10))
		offset_line[terminalscreen.xpos] ^= 0xFF;
	if ((++curscanline) >= VIDSCANLINES) {
		curscanline = 0;
		if ((++curvidrow) >= VIDHACKROWS)
			curvidrow = VIDHACKROWS-1;
	}
}

#ifdef INLINEDELAY
static __inline __attribute__ ((always_inline)) void softdelay(int tick)			
{
	int i;
	asm volatile (  "mov r1, %1\n\t"
				"2: sub r1, #1\n\t"
				"cmp r1, #0\n\t"
				"bgt 2b\n\t" : "=r" (i) : "r" (tick) : "r1"
				);
}
#endif

void positive_pulse(void)
{ 
	LPC_GPDMACH0->DMACCConfig |= GPDMA_DMACCxConfig_E;
	LPC_GPIO0->FIOCLR = (1 << CS_PIN_NUM);
#ifdef INLINEDELAY
	softdelay(SYNCLEN);
#else
	{
		volatile int wt;
		for (wt=(SYNCLEN/2);(--wt)>0;);
	}
#endif
	LPC_GPIO0->FIOSET = (1 << CS_PIN_NUM);
}

void negative_pulse(void)
{ 
	LPC_GPDMACH0->DMACCConfig |= GPDMA_DMACCxConfig_E;
	LPC_GPIO0->FIOSET = (1 << CS_PIN_NUM);
#ifdef INLINEDELAY
	softdelay(VBLSYNCLEN);
#else
	{
		volatile int wt;
		for (wt=(VBLSYNCLEN/2);(--wt)>0;);
	}
#endif
	LPC_GPIO0->FIOCLR = (1 << CS_PIN_NUM);
}

void nopulse(void)
{
}

void RIT_IRQHandler(void)
{
	LPC_RIT->RICTRL |= 1;		
	nextpulse();
	nextpulse = nopulse;
}

#if 0
void video_debug(void)
{
	deboutstrint("curvidcolumn=",curvidcolumn);
	deboutstrint("curvidrow=",curvidrow);
	deboutstrint("curscanline=",curscanline);
	deboutstrint("curtotalscanline=",curtotalscanline);
}
#endif

int init_video(void)
{
	int i,j;
	PINSEL_CFG_Type PinCfg;
	SSP_CFG_Type SSP_ConfigStruct;

	nextpulse = positive_pulse;

	for (i=0;i<VIDNUMCOLUMNS;i++)
		for (j=0;j<VIDCOLUMNTOTALLEN;j++)
			vidcolumns[i][j] = 0;

	init_virtscreen(&terminalscreen, screendata, VIDHACKROWS-1, VIDCOLUMNS);
	for (j=0;j<VIDCOLUMNS;j++)
		screendata[VIDHACKROWS-1][j] = ' ';

#if 0
	for (i=0;i<(VIDHACKROWS-1);i++)
		for (j=0;j<VIDCOLUMNS;j++)
			screendata[i][j] = ((i+j) % 90) + 33;
#endif
	
	curvidcolumn = 0;
	curvidrow = 0;
	curscanline = 0;	
	curtotalscanline = 0;
	curframe = 0;
		
	next_line = vidcolumns[curvidcolumn];
	
	/*
	 * Initialize SSP pin connect
	 * P0.15 - SCK;
	 * P0.16 - SSEL
	 * P0.17 - MISO
	 * P0.18 - MOSI
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
#ifdef VIDEO_SSP1
	PinCfg.Pinnum = 7;
#else	
	PinCfg.Pinnum = 15;
#endif
	PINSEL_ConfigPin(&PinCfg);
#ifdef VIDEO_SSP1
	PinCfg.Pinnum = 8;
#else
	PinCfg.Pinnum = 17;
#endif
	PINSEL_ConfigPin(&PinCfg);
#ifdef VIDEO_SSP1
	PinCfg.Pinnum = 9;
#else
	PinCfg.Pinnum = 18;
#endif
	PINSEL_ConfigPin(&PinCfg);
#ifdef VIDEO_SSP1
	PinCfg.Pinnum = 6;
#else
	PinCfg.Pinnum = 16;
#endif
	PinCfg.Funcnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	GPIO_SetDir(CS_PORT_NUM, (1<<CS_PIN_NUM), 1);
	GPIO_SetValue(CS_PORT_NUM, (1<<CS_PIN_NUM));
	
	SSP_ConfigStructInit(&SSP_ConfigStruct);
	SSP_ConfigStruct.ClockRate = PIXCLOCK;
	SSP_ConfigStruct.CPHA = SSP_CPHA_SECOND;
	SSP_ConfigStruct.CPOL = SSP_CPOL_LO;
	SSP_Init(SSPx, &SSP_ConfigStruct);
	SSP_Cmd(SSPx, ENABLE);

	NVIC_SetPriority(DMA_IRQn, ((0x01<<3)|0x01));
	NVIC_EnableIRQ(DMA_IRQn);
	GPDMA_Init();
    NVIC_DisableIRQ (DMA_IRQn);

 	initiate_dma(next_line,VIDCOLUMNTRANSMIT);

	SSP_DMACmd (SSPx, SSP_DMA_TX, ENABLE);
//	GPDMA_ChannelCmd(0, ENABLE);
    NVIC_EnableIRQ (DMA_IRQn);

	CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCRIT, ENABLE);
	LPC_RIT->RICOUNTER = 0;
	LPC_RIT->RIMASK	= 0;
	LPC_RIT->RICOMPVAL = LINERATEUSSEC/4;
	LPC_RIT->RICTRL	= 0x0E;
	NVIC_SetPriority(RIT_IRQn, ((0x01<<2)|0x01));
	NVIC_EnableIRQ(RIT_IRQn);

	return 0;
}
