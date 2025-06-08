
#ifndef _io_h
#define _io_h

void setpin(int pinno, unsigned int mode);
void outd(int pinno, unsigned int state);
void outdac(int pinno, unsigned int value);
void pwm(int pinno, unsigned int val1, unsigned int val2);
void tone(int pinno, unsigned int val1, unsigned int val2);
unsigned int ind(int pinno);
unsigned int inadc(int pinno);
int read_adc(unsigned short *buf, int samples);
int adc_readlen(void);
int write_dac(unsigned short *buf, int samples);
int dac_wrtleft(void);
void adc_init(int inadcchans, int outdacchans, int clkdiv, int indiv, int outdiv);
void adc_shutdown(void);
void adc_trig(int mode, int chan, int level);

#define TRIG_MODE_NONE    0
#define TRIG_MODE_PAUSE   1
#define TRIG_MODE_ABOVE   2
#define TRIG_MODE_BELOW   3

void serial_init(int port, int baud, int databits, int stopbits, int parity);
int serial_read(int port, int len, int charend, int timeout, char *data);
int serial_write(int port, char *data, int len);
unsigned char serspi(int mosipin, int misopin, 
					 int clkpin, unsigned char c, int unsigned dly);
int serbng(int pinno, unsigned char *data, unsigned int len, unsigned int dly);
int seri2c(int sdawritepin, int sdareadpin, int sclpin, int addr, int end_trans,
		   int numoutbytes, unsigned char *outbytes,
		   int numinbytes, unsigned char *inbytes);


#endif  /* _io_h */
