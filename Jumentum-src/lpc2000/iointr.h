
#ifndef __IOINTR_H__
#define __IOINTR_H__

#include "lpc210x.h"

//void adcISR (void)   __attribute__ ((interrupt("IRQ")));
void adcISR (void)   __attribute__ ((naked));

// we use the USB RAM for the FIFO buffers on the LPC2148
// because we don't use USB yet.
#if defined LPC2148 || defined LPC2378
#define ADC_InBuf           ((pREG16 (0x7fd00000)))
#define ADC_InBufLen        3072
#define DAC_OutBuf          ((pREG16 (0x7fd01800)))
#define DAC_OutBufLen       1024
#endif

#ifdef LPC2106
extern unsigned short ADC_InBuf[];
#define ADC_InBufLen 3072
extern unsigned short DAC_OutBuf[];
#define DAC_OutBufLen 1024
#endif

#ifdef LPC2119
extern unsigned short ADC_InBuf[];
#define ADC_InBufLen 128
extern unsigned short DAC_OutBuf[];
#define DAC_OutBufLen 64
#endif

extern int adc_fifo_head;
extern int adc_fifo_tail;
extern int adc_indiv;
extern int adc_indiv_ctr;
extern int adc_num_chans;
extern int adc_count_chans;
extern unsigned int adc_last_samp ;

extern int dac_fifo_head;
extern int dac_fifo_tail;
extern int dac_outdiv;
extern int dac_outdiv_ctr;

extern unsigned int trig_mode;
extern unsigned int trig_pause;
extern unsigned int trig_chan;
extern unsigned int trig_level;

#endif /* __IOINTR_H__ */

/** @} */
