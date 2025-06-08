/*
 * Copyright (c) 2001-2005, Adam Dunkels.
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
 * $Id: httpd.h,v 1.2 2006/06/11 21:46:38 adam Exp $
 *
 */

#ifndef __HTTPD_H__
#define __HTTPD_H__

#include "../../uip/psock.h"
#include "httpd-fs.h"

extern char const ampesc[];
extern char const ltesc[];
extern char const gtesc[];
extern char const quotesc[];

#define HTTP_GET 1
#define HTTP_POST 2
#define HTTP_SEND_503 3

#define SPECIAL_SERVER_NOT 0
#define SPECIAL_SERVER_BASIC 1
#define SPECIAL_SERVER_PRG 2
#define SPECIAL_SERVER_UNTOK 3

struct httpd_state {
  int len;
  unsigned long conn_id;
  char *scriptptr;
  int scriptlen;
  unsigned short count, temp;
  unsigned short content_length;
  char state;
  char request_type;
  char is_special_server;
  char is_authorized;
  unsigned short timer;
  char inputbuf[60];
  char filename[40];
  struct psock sin, sout;
  struct pt outputpt, scriptpt, basicpt;
  struct httpd_fs_file file;
};

void httpd_init(void);
void httpd_appcall(void);
int httpd_relay_html(unsigned char *data, int len, unsigned short conn_id);
int httpd_get_post(unsigned char *data, int len, unsigned short conn_id);
int httpd_get_form(unsigned char *postdata, unsigned char *var, unsigned char *data);
int httpd_escape_form(unsigned char *cval, unsigned char *var);
char *httpd_basic_webreq(unsigned short conn_id, int datanum);
int httpd_basic_connection(void);

void httpd_log(char *msg);
void httpd_log_file(u16_t *requester, char *file);
void encode64( const char *instr, char *outstr);

void cleanup_special_state(struct httpd_state *s);


#endif /* __HTTPD_H__ */
