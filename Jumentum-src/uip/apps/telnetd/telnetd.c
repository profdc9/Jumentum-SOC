/*
 * Copyright (c) 2003, Adam Dunkels->
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * $Id: telnetd.c,v 1.2 2006/06/07 09:43:54 adam Exp $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "../../uip/uip.h"
#include "telnetd.h"
#include "../../lib/memb.h"
#include "../../all.h"
#include "../../lib.h"
#include "../../net.h"
#include "../../main.h"

#define ISO_nl       0x0a
#define ISO_cr       0x0d

static struct telnetdfifo recvfifo;
static struct telnetdfifo sendfifo;
static char connected = 0;
static signed char authenticated = 0;

struct  telnetd_state *s;

static void inittelnetfifo(struct telnetdfifo *fifo)
{
	fifo->fifohead = fifo->fifotail = fifo->fifoaheadtail = 0;
}

static void puttelnetfifo(struct telnetdfifo *fifo, int ch)
{
	int newpos = fifo->fifohead + 1;
	if (newpos >= FIFOSIZE) newpos = 0;	
	if ((newpos == fifo->fifotail) || (newpos == fifo->fifoaheadtail))
		return;
	fifo->buf[fifo->fifohead] = ch;
	fifo->fifohead = newpos;
}

static void puttelnetfifostring(struct telnetdfifo *fifo, char *string)
{
	while (*string != '\0')
		puttelnetfifo(fifo, *string++);
}

static int gettelnetfifoahead(struct telnetdfifo *fifo)
{	
	int ch;
	if (fifo->fifoaheadtail == fifo->fifohead)
		return -1;
	ch = fifo->buf[fifo->fifoaheadtail++];
	if (fifo->fifoaheadtail >= FIFOSIZE)
		fifo->fifoaheadtail = 0;
	return ch;
}

static int gettelnetfifo(struct telnetdfifo *fifo)
{	
	int ch;
	if (fifo->fifotail == fifo->fifohead)
		return -1;
	ch = fifo->buf[fifo->fifotail++];
	if (fifo->fifotail >= FIFOSIZE)
		fifo->fifotail = 0;
	return ch;
}

static int fifoused(struct telnetdfifo *fifo)
{
	int rem = fifo->fifohead - fifo->fifotail;
	return (rem < 0) ? (rem + FIFOSIZE) : rem;
}

static int fifoaheadused(struct telnetdfifo *fifo)
{
	int rem = fifo->fifohead - fifo->fifoaheadtail;
	return (rem < 0) ? (rem + FIFOSIZE) : rem;
}

#define UART0_BASE_ADDR		0xE000C000
#define U0THR          (*(volatile unsigned long *)(UART0_BASE_ADDR + 0x00))

int telnetd_addsendfifo(int ch)
{
	if ((connected) && (authenticated == -1)) {
		if (ch == '\n') puttelnetfifo(&sendfifo, '\r');	
		puttelnetfifo(&sendfifo, ch);
		if ((connected) && (fifoused(&sendfifo) > FIFOACTIONTHRESHOLD)) {
				while ((connected) && (fifoused(&sendfifo) > 0))
					netmainloop();
		}
		return 0;
	}
	return -1;
}

int telnetd_getrecvfifo()
{
	int ch;
	if ((!connected) || (authenticated != -1)) return -1;
	recvfifo.fifoaheadtail = recvfifo.fifohead;
	ch = gettelnetfifo(&recvfifo);
#ifdef TELNETFIFOFLOW
	if (s->paused) {
		if (fifoused(&recvfifo) < FIFORESTARTTHRESHOLD) {
			s->paused = 0;
			uip_restart();
		}
	}
#endif
	return ch == '\n' ? -1 : ch;
}

#define STATE_NORMAL 0
#define STATE_IAC    1
#define STATE_WILL   2
#define STATE_WONT   3
#define STATE_DO     4
#define STATE_DONT   5
#define STATE_CLOSE  6

#define TELNET_IAC   255
#define TELNET_WILL  251
#define TELNET_WONT  252
#define TELNET_DO    253
#define TELNET_DONT  254

/*---------------------------------------------------------------------------*/
static void
senddata(void)
{
  int used, wrt;
  u8_t *bufptr;
   
  if (s->waiting_for == 0) {
	used = fifoaheadused(&sendfifo);
  } else {
	sendfifo.fifoaheadtail -= s->waiting_for;
	if (sendfifo.fifoaheadtail < 0)
		sendfifo.fifoaheadtail += FIFOSIZE;
	used = s->waiting_for;
	s->waiting_for = 0;
  }
  bufptr = uip_appdata;
  if (used > uip_mss())
	used = uip_mss();
  wrt = 0;
  while (wrt < used)
	bufptr[wrt++] = gettelnetfifoahead(&sendfifo);
  if (wrt > 0) {
	uip_send(uip_appdata, wrt);
	s->waiting_for = wrt;
  }
}

/*---------------------------------------------------------------------------*/
static void acked(void)
{
  
  sendfifo.fifotail += s->waiting_for;
  if (sendfifo.fifotail >= FIFOSIZE)
		sendfifo.fifotail -= FIFOSIZE;
  s->waiting_for = 0;
  //deboutstrint("fifousedacked=",fifoused(&sendfifo));
}

/*---------------------------------------------------------------------------*/
static void
closed(void)
{
}

/*---------------------------------------------------------------------------*/
static void
get_char(u8_t c)
{
  puttelnetfifo(&recvfifo, c);  
#ifdef TELNETFIFOFLOW
  if ((fifoused(&recvfifo) >= FIFORECEIVETHRESHOLD) && (!s->paused)) {
	uip_stop();
	s->paused = 1;	
  }
#endif
}
/*---------------------------------------------------------------------------*/
static void
sendopt(u8_t option, u8_t value)
{
  puttelnetfifo(&sendfifo, TELNET_IAC);
  puttelnetfifo(&sendfifo, option);
  puttelnetfifo(&sendfifo, value);
}

#define OPT_WILL 251
#define OPT_TELOPT_BINARY 0
#define OPT_TELOPT_ECHO 1
#define OPT_TELOPT_SGA 3

static int telnetoptions(u8_t c)
{
	return ((c == OPT_TELOPT_ECHO) || (c == OPT_TELOPT_SGA));
}

/*---------------------------------------------------------------------------*/
static void
newdata(void)
{
  u16_t len;
  u8_t c;
  char *dataptr;
    
  len = uip_datalen();
  dataptr = (char *)uip_appdata;

  while(len > 0) {
    c = *dataptr;
    ++dataptr;
    --len;
    switch(s->state) 
    {
		case STATE_IAC:
		if(c == TELNET_IAC) 
		{
			if (authenticated == -1) 
				get_char(c);
			s->state = STATE_NORMAL;
		} 
		else 
		{
			switch(c) 
			{
				case TELNET_WILL:
					s->state = STATE_WILL;
					break;
				case TELNET_WONT:
					s->state = STATE_WONT;
					break;
				case TELNET_DO:
					s->state = STATE_DO;
					break;
				case TELNET_DONT:
					s->state = STATE_DONT;
					break;
				default:
					s->state = STATE_NORMAL;
				break;
			}
		}
		break;
	case STATE_WONT:
    case STATE_WILL:
      sendopt(telnetoptions(c) ? TELNET_DO : TELNET_DONT, c);
      s->state = STATE_NORMAL;
      break;
#if 0
    case STATE_WONT:
      /* Reply with a DONT */
      sendopt(TELNET_DONT, c);
      s->state = STATE_NORMAL;
      break;
#endif
    case STATE_DONT:
    case STATE_DO:
      /* Reply with a WONT */
      sendopt(telnetoptions(c) ? TELNET_WILL : TELNET_WONT, c);
      s->state = STATE_NORMAL;
      break;
#if 0
    case STATE_DONT:
      /* Reply with a WONT */
      sendopt(TELNET_WONT, c);
      s->state = STATE_NORMAL;
      break;
#endif
    case STATE_NORMAL:
      if(c == TELNET_IAC) {
			s->state = STATE_IAC;
	  } else {
		if (authenticated != -1) {
			if (c == '\r') {
				if ((authenticated >= 0) && (auth_password[authenticated] == 0)) { 
						puttelnetfifostring(&sendfifo,"\r\nAccepted\r\n");
						authenticated = -1;
				} else uip_close();
			} else if (authenticated >= 0) {
				if ((c != 0) && (c == auth_password[authenticated]))
					authenticated++;
				else
					authenticated = -2;
			} 
		} else
			get_char(c);
      }
      break;
    }    
  }
}

/*---------------------------------------------------------------------------*/
void telnetd_init(void)
{
  uip_listen(telnet_port);
  connected = 0;
}

static void sendterminaloptions(void)
{
	sendopt(OPT_WILL, OPT_TELOPT_ECHO);
	sendopt(OPT_WILL, OPT_TELOPT_SGA);
}

/*---------------------------------------------------------------------------*/
void
telnetd_appcall(void)
{
  s = (struct telnetd_state *)&uip_conn->appstate.telnetd;
  
  if(uip_connected()) 
  {
    if (connected)
		uip_abort();
	authenticated = (*auth_password) ? 0 : -1;
	inittelnetfifo(&recvfifo);
	inittelnetfifo(&sendfifo);
	s->waiting_for = 0;
	if (connected == 0) {
		s->didconnect = 1;
		connected = 1;
	} else s->didconnect = 0;
    s->state = STATE_NORMAL;
#ifdef TELNETFIFOFLOW
	s->paused = 0;
#endif
	sendterminaloptions();
	if (authenticated == 0)
		puttelnetfifostring(&sendfifo, "Password:");
  }

  if(s->state == STATE_CLOSE) 
  {
    if (s->didconnect)
		connected = 0;
    s->state = STATE_NORMAL;
    uip_close();
    return;
  }
  
  if(uip_closed() || uip_aborted() || uip_timedout())  
  {
    if (s->didconnect)
		connected = 0;
    closed();
  }
  
  if(uip_acked()) 
  {
    acked();
  }
  
  if(uip_newdata()) 
  {
    newdata();
  }
  
  if(uip_rexmit() ||uip_newdata() || uip_acked() || uip_connected() ||uip_poll()) 
  {
    senddata();
  }
}
/*---------------------------------------------------------------------------*/
