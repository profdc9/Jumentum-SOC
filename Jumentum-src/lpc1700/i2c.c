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

/*  Portable implementation of i2c bus */
/* amalgamated from i2c example codes for microcontrollers */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "all.h"
#include "lib.h"
#include "i2c.h"

#undef DEBUGI2C
#define SDAOUTREV

#ifdef DEBUGI2C
int sdabit;
int lastio = 1;
#endif

static void sda_out(i2cstate *state, i2c_bit out)
{
#ifdef SDAOUTREV
	if (out != 0) 
		*(state->sdawriteregwrite) = *(state->sdawriteregread) & ~state->sdawritebit;
	else
		*(state->sdawriteregwrite) = *(state->sdawriteregread) | state->sdawritebit;
#else
	if (out != 0) 
		*(state->sdawriteregwrite) = *(state->sdawriteregread) | state->sdawritebit;
	else
		*(state->sdawriteregwrite) = *(state->sdawriteregread) & ~state->sdawritebit;
#endif
#ifdef DEBUGI2C
	if (lastio == 1) {
		putch('D');
		lastio = 0;
	}
	putch('0'+(out!=0));
    sdabit=out;
#endif
}

static i2c_bit sda_in(i2cstate *state)
{
#ifdef DEBUGI2C
	i2c_software_register bit = *(state->sdareadregread) & state->sdareadbit;
	
    if (sdabit == 0)
		putch('*');
	if (lastio == 0) {
		putch('I');
		lastio = 1;
	}
	putch('0'+(bit!=0));
	return bit != 0;
#else
	return ((*(state->sdareadregread) & state->sdareadbit) != 0);
#endif
}

static void scl_out(i2cstate *state, i2c_bit out)
{
	if (out != 0) 
		*(state->sclwriteregwrite) = *(state->sclwriteregread) | state->sclwritebit;
	else
		*(state->sclwriteregwrite) = *(state->sclwriteregread) & ~state->sclwritebit;
#ifdef DEBUGI2C
    putch('C');
	putch('0'+(out!=0));
#endif
}

#ifdef MULTI_MASTER
static i2c_bit scl_in(i2cstate *state)
{
	return ((*(state->sclreadregread) & state->sclreadbit)) != 0);
}
#endif

static void delay(i2cstate *state)
{
	volatile i2c_delay i;
	for (i=state->delay;i--;i>0);
}

#ifdef MULTI_MASTER
static int i2c_bus_busy(i2cstate *state)
{
	return ((scl_in(state) != 0) && (sda_in(state) != 0));
}
#endif

/* Initiate i2c transmission */

static void start_i2c_transmission(i2cstate *state)
{
	delay(state);
	sda_out(state,0);
	delay(state);
	scl_out(state,0);
	delay(state);
#ifdef DEBUGI2C
	deboutstrhex("start",1);
#endif
}


/* End i2c transmission */

static int end_i2c_transmission(i2cstate *state)
{
	scl_out(state,1);
	delay(state);
	sda_out(state,1);
	delay(state);
#ifdef DEBUGI2C
	deboutstrhex("end=",1);
#endif
#ifdef MULTI_MASTER
	return i2c_bus_busy(state);
#else
	return 0;
#endif
}

static int i2c_clock_bit_out(i2cstate *state, i2c_bit bit)
{
#ifdef MULTI_MASTER
	i2c_delay timeout = state->timeout;
#endif

	sda_out(state,bit);
	delay(state);
	scl_out(state,1);
#ifdef MULTI_MASTER
	while ((scl_in(state) == 0) && (timeout>0))
		timeout--;
	if (timeout == 0)
		return TIMEOUT;
	if (sda_in(state) != bit)
	{
		sda_out(state,1);
		return LOSTBUS;
	}
#endif
	delay(state);
	scl_out(state,0);
	delay(state);
	return 0;
}
	
static int i2c_clock_bit_in(i2cstate *state)
{
	i2c_bit bit;
#ifdef MULTI_MASTER
	i2c_delay timeout = state->timeout;
#endif
	
	scl_out(state,1);
#ifdef MULTI_MASTER
	while ((scl_in(state) == 0) && (timeout>0))
		timeout--;
	if (timeout == 0)
		return TIMEOUT;
#else
	delay(state);
#endif
	bit = sda_in(state);
	delay(state);
	scl_out(state,0);
	delay(state);
	return bit;
}

static int i2c_send_byte(i2cstate *state, i2c_byte byt)
{
	int bits;
	int err;
	
	for (bits=8;bits>0;bits--)
	{
		if (err=(i2c_clock_bit_out(state,byt & 0x80)))
			return err;
		byt <<= 1;
	}
	sda_out(state,1);  /* dan changed */
	delay(state);      /* dan changed */
	if (i2c_clock_bit_in(state) != 0) {
#ifdef DEBUGI2C
        deboutstrhex("nak encountered=",1);
#endif
		return NAK;
	}
	return 0;
}

static int i2c_read_byte(i2cstate *state, i2c_bit ack)
{
	i2c_byte byt = 0;
	int bits;
	int err;
	for (bits=8;bits>0;bits--)
	{
		err = i2c_clock_bit_in(state);
		if (err<0) 
			return err;
		byt = (byt << 1) | ((i2c_byte)(err != 0));
	}
	i2c_clock_bit_out(state,ack);
	sda_out(state,1);
	return byt;
}

int i2c_send_bytes(i2cstate *state, i2c_byte addr, i2c_byte count, i2c_byte *data, int end_transmission)
{
	int err;

#ifdef DEBUGI2C	
    deboutstrhex("send_bytes=",count);
#endif
	start_i2c_transmission(state);
	if (err=i2c_send_byte(state,addr & 0xFE)) {
		if ((err == NAK) && (end_transmission))
			end_i2c_transmission(state);
		return err;
	}
	while (count > 0)
	{
		count--;
		if (err=i2c_send_byte(state,*data++)) {
#ifdef DEBUGI2C
		    deboutstrhex("err=",err);
#endif
			if ((err == NAK) && (end_transmission))
				end_i2c_transmission(state);
			return err;
		}
	}
	if (end_transmission)
		end_i2c_transmission(state);
	return 0;
}

int i2c_recv_bytes(i2cstate *state, i2c_byte addr, i2c_byte count, i2c_byte *data)
{
	int err;
	
#ifdef DEBUGI2C
    deboutstrhex("recv_bytes=",count);
#endif
	start_i2c_transmission(state);
	if (err=i2c_send_byte(state,addr | 0x01)) {
		if (err == NAK)
			end_i2c_transmission(state);
		return err;
	}
	while (count>0)
	{
		count--;
		err = i2c_read_byte(state, count == 0);
		if (err < 0) {
			end_i2c_transmission(state);
			return err;
		}
		*data++ = ((i2c_byte)err);
	}
	end_i2c_transmission(state);
	return 0;
}

int i2c_send_recv_bytes(i2cstate *state, i2c_byte addr, i2c_byte txcount, i2c_byte *txdata,
                        i2c_byte rxcount, i2c_byte *rxdata)
{
	int err;
#ifdef DEBUGI2C
    deboutstrhex("send_recv_bytes=",txcount);
#endif
	if (err=i2c_send_bytes(state,addr,txcount,txdata,0)) {
#ifdef MULTI_MASTER
		if (err == LOSTBUS) 
			end_i2c_transmission(state);
#endif
		return err;
	}
	sda_out(state,1);
	scl_out(state,1);
	delay(state);
	if (err=i2c_recv_bytes(state,addr,rxcount,rxdata)) {
		return err;
	}
	return 0;
}
