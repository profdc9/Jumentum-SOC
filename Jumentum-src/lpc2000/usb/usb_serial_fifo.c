/*
	LPCUSB, an USB device driver for LPC microcontrollers	
	Copyright (C) 2006 Bertrik Sikken (bertrik@sikken.nl)

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright
	   notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.
	3. The name of the author may not be used to endorse or promote products
	   derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
	IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "type.h"
#include "usb_serial_fifo.h"

#define MOD_FIFO(x) ((x) & (VCOM_FIFO_SIZE-1))

void usb_serial_fifo_init(fifo_t *fifo, U8 *buf)
{
	fifo->head = 0;
	fifo->tail = 0;
	fifo->buf = buf;
}


BOOL usb_serial_fifo_put(fifo_t *fifo, U8 c)
{
	int next;
	
	// check if FIFO has room
	next = MOD_FIFO(fifo->head + 1);
	if (next == fifo->tail) {
		// full
		return FALSE;
	}
	fifo->buf[fifo->head] = c;
	fifo->head = next;
	
	return TRUE;
}


BOOL usb_serial_fifo_get(fifo_t *fifo, U8 *pc)
{
	int next;
	
	// check if FIFO has data
	if (fifo->head == fifo->tail) {
		return FALSE;
	}
	next = MOD_FIFO(fifo->tail + 1);
	*pc = fifo->buf[fifo->tail];
	fifo->tail = next;

	return TRUE;
}


int usb_serial_fifo_avail(fifo_t *fifo)
{
	return MOD_FIFO(VCOM_FIFO_SIZE + fifo->head - fifo->tail);
}


int usb_serial_fifo_free(fifo_t *fifo)
{
	return (VCOM_FIFO_SIZE - 1 - usb_serial_fifo_avail(fifo));
}

