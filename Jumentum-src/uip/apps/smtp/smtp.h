
/**
 * \addtogroup smtp
 * @{
 */


/**
 * \file
 * SMTP header file
 * \author Adam Dunkels <adam@dunkels.com>
 */

/*
 * Copyright (c) 2002, Adam Dunkels.
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
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: smtp.h,v 1.4 2006/06/11 21:46:37 adam Exp $
 *
 */
#ifndef __SMTP_H__
#define __SMTP_H__

#include "../../uip/psock.h"

/**
 * Error number that signifies a non-error condition.
 */
#define SMTP_ERR_OK 0

void smtp_init(void);

/* Functions. */
//void smtp_configure(char *localhostname, u16_t *smtpserver);
unsigned char smtp_send(char *to, char *cc, char *from,
	                    char *subject, char *msg, u16_t msglen);
#define SMTP_SEND(to, cc, from, subject, msg) \
        smtp_send(to, cc, from, subject, msg, strlen(msg))

void smtp_appcall(void);

struct smtp_state {
  u8_t connected;
  char *to;
  char *from;
  char *subject;
  char *msg;
  char *cc;
  u16_t msglen;
  
  u16_t sentlen, textlen;
  u16_t sendptr;
  char inputbuffer[60];
  struct psock psock;
};


typedef struct smtp_state uip_tcp_appstate_t3;


#endif /* __SMTP_H__ */

/** @} */
