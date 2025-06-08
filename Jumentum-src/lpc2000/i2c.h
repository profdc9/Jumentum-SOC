
#ifndef _I2C_H
#define _I2C_H

#define TIMEOUT -1
#define LOSTBUS -2
#define NAK -3

typedef unsigned long i2c_software_register;
typedef unsigned char i2c_bit;
typedef unsigned char i2c_byte;
typedef unsigned long i2c_delay;

#undef MULTI_MASTER

typedef struct _i2cstate
{
	i2c_delay delay, timeout;
	
	volatile i2c_software_register *sdawriteregwrite;
	volatile i2c_software_register *sdawriteregread;
	int sdawritebit;

	volatile i2c_software_register *sdareadregread;
	int sdareadbit;

	volatile i2c_software_register *sclwriteregwrite;
	volatile i2c_software_register *sclwriteregread;
	int sclwritebit;

#ifdef MULTI_MASTER
    volatile i2c_software_register *sclreadregread;
	int sclreadbit;
#endif
	
} i2cstate;

int i2c_send_bytes(i2cstate *state, i2c_byte addr, i2c_byte count, i2c_byte *data, int end_transmission);

int i2c_recv_bytes(i2cstate *state, i2c_byte addr, i2c_byte count, i2c_byte *data);

int i2c_send_recv_bytes(i2cstate *state, i2c_byte addr, i2c_byte txcount, i2c_byte *txdata,
                        i2c_byte rxcount, i2c_byte *rxdata);

#endif /* _I2C_H */
