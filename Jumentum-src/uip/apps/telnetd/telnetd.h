/*
 * Copyright (c) 2003, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
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
 * $Id: telnetd.h,v 1.2 2006/06/07 09:43:54 adam Exp $
 *
 */
#ifndef __TELNETD_H__
#define __TELNETD_H__

#include "../../uip/uipopt.h"

void telnetd_init(void);
void telnetd_appcall(void);
int telnetd_addsendfifo(int ch);
int telnetd_getrecvfifo(); 

#define FIFOSIZE 256
#define FIFOACTIONTHRESHOLD (FIFOSIZE-64)
#define FIFORECEIVETHRESHOLD (FIFOSIZE/2)
#define FIFORESTARTTHRESHOLD 10

struct telnetdfifo
{
  volatile u8_t buf[FIFOSIZE];
  volatile int fifohead;
  volatile int fifotail;
  volatile int fifoaheadtail;
};

struct __attribute__((packed))telnetd_state {
  volatile u8_t state;
  volatile u8_t didconnect;
  int waiting_for;
#ifdef TELNETFIFOFLOW
  volatile u8_t paused;
#endif
};

typedef struct telnetd_state uip_tcp_appstate_t1;

extern struct uip_conn *uip_conn;

#ifndef UIP_APPCALL
#define UIP_APPCALL     telnetd_appcall
#endif

#endif /* __TELNETD_H__ */
