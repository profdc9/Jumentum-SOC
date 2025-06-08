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

/*
 * This is a generic implementation of the SLIP protocol over an RS232
 * (serial) device.
 */
 
#include <stdio.h>
#include <stdlib.h>
 
#include "all.h"
#include "serial.h"
#include "lib.h"
#include "uip/uip/uip.h"
#include "rs232dev.h"


#define SLIP_END     0300
#define SLIP_ESC     0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335

#define MAX_SIZE UIP_BUFSIZE

static u8_t slip_buf[MAX_SIZE];
char slip_uart = 1;

#if MAX_SIZE > 255
static u16_t len, tmplen;
#else
static u8_t len, tmplen;
#endif /* MAX_SIZE > 255 */
static unsigned short last_tick = 0;
static u8_t clientbuf[6] = {0,0,0,0,0,0};

#define SIGNALSEND()    
#define SIGNALSENDOFF() 

#define SIGNALREAD()    
#define SIGNALREADOFF() 


/*
 * rs232dev_send():
 *
 * Sends the packet in the uip_buf and uip_appdata buffers. The first
 * 40 bytes of the packet (the IP and TCP headers) are read from the
 * uip_buf buffer, and the following bytes (the application data) are
 * read from the uip_appdata buffer.
 */
void rs232dev_send(void)
{
#if MAX_SIZE > 255
   u16_t i;
#else
   u8_t i;
#endif /* MAX_SIZE > 255 */
   u8_t *ptr;
   u8_t c;

   SIGNALSEND();
   
   rs232_putc_serial(SLIP_END);
   ptr = uip_buf;
   for (i = 0; i < uip_len; ++i)
   {
		if (i == 40)
		{
			ptr = (u8_t *)uip_appdata;
		}
		c = *ptr++;
		switch (c)
		{
			case SLIP_END:
				rs232_putc_serial(SLIP_ESC);
				rs232_putc_serial(SLIP_ESC_END);
				break;
			case SLIP_ESC:
				rs232_putc_serial(SLIP_ESC);
				rs232_putc_serial(SLIP_ESC_ESC);
				break;
			default:
				rs232_putc_serial(c);
			break;
		}
   }
   rs232_putc_serial(SLIP_END);

   SIGNALSENDOFF();

}

/*
 * rs232dev_poll():
 *
 * Read all avaliable bytes from the RS232 interface into the slip_buf
 * buffer. If no more bytes are avaliable, it returns with 0 to
 * indicate that no packet was immediately ready. When a full packet
 * has been read into the buffer, the packet is copied into the
 * uip_buf buffer and the length of the packet is returned.
 */
#if MAX_SIZE > 255
u16_t
#else 
u8_t
#endif /* MAX_SIZE > 255 */
rs232dev_poll(unsigned short tick)
{
   u8_t c;
   static u8_t last_esc = 0;
   int com_ret;

   while ((com_ret = rs232_getkey_serial()) != -1)
   {
	  SIGNALREAD();
      c = (u8_t)com_ret;
	  
	  if (((unsigned short)(tick-last_tick)) > CLIENT_TICK_THRESHOLD) {
		 clientbuf[0] = clientbuf[1];
		 clientbuf[1] = clientbuf[2];
		 clientbuf[2] = clientbuf[3];
		 clientbuf[3] = clientbuf[4];
		 clientbuf[4] = clientbuf[5];
		 clientbuf[5] = c;
		 if ((clientbuf[0] == 'C') && (clientbuf[1] == 'L') &&
			 (clientbuf[2] == 'I') && (clientbuf[3] == 'E') &&
			 (clientbuf[4] == 'N') && (clientbuf[5] == 'T')) {
			 rs232_putstring_serial("CLIENTSERVER");
		}
	  }
	  if (last_esc) {
			last_esc = 0;
			switch (c) {
				case SLIP_ESC_END: c = SLIP_END; break;
				case SLIP_ESC_ESC: c = SLIP_ESC; break;		
			}
			if (len < MAX_SIZE) slip_buf[len++] = c;
	  } else {
		switch (c) {
			case SLIP_END:
				last_tick = tick;
				memcpy(uip_buf, slip_buf, len);
				tmplen = len;
				len = 0;
				last_esc = 0;
				SIGNALREADOFF();
				return tmplen;
			case SLIP_ESC:
				last_esc = 1;
				break;
			default:
				if (len < MAX_SIZE) slip_buf[len++] = c;
				break;
			}
		}
	}
   SIGNALREADOFF();
   return 0;
}

/*
 * rs232dev_init():
 *
 * Initializes the RS232 device and sets the parameters of the device.
 */ 
void rs232dev_init(void)
{
   rs232_init_serial();
   len = 0;
}

/*
 * rs232dev_close():
 *
 * Closes the RS232 device.
 */ 
void rs232dev_close(void)
{
}

#if 0
void rs232dev_wait_for_connect(void)
{
	char s[30];
	int c, itr=0;
	for (;;) {
		c = rs232_waitkey_serial();
		memmove(s,&s[1],sizeof(s)-1);
		if (c == '\001') {
			for (c=0;c<sizeof(s);c++)
				rs232_putc_serial(s[c]);
		}
		s[sizeof(s)-1] = c;
		if (strncasecmp(&s[sizeof(s)-6],"CLIENT",6) == 0) {
			if (++itr >= 2) {
				rs232_putstring_serial("CLIENTSERVER");
				break;
			}
		}
	}
}
#endif
