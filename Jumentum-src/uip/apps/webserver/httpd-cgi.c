/**
 * \addtogroup httpd
 * @{
 */

/**
 * \file
 *         Web server script interface
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 */

/*
 * Copyright (c) 2001-2006, Adam Dunkels.
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
 * $Id: httpd-cgi.c,v 1.2 2006/06/11 21:46:37 adam Exp $
 *
 */

#include "../../uip/uip.h"
#include "../../uip/psock.h"
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"

#include <stdio.h>
#include <string.h>

#include "../../all.h"
#include "../../lib.h"
#include "../../main.h"
#include "../../basic.h"

HTTPD_CGI_CALL(tcp, "tcp-connections", tcp_stats);
HTTPD_CGI_CALL(prgbas, "program-basic", program_basic);
HTTPD_CGI_CALL(cnfbas, "conf-basic", conf_basic);
HTTPD_CGI_CALL(runbas, "run-basic", run_basic_prg);
HTTPD_CGI_CALL(pausebas,"pause-basic",pause_basic_prg);
HTTPD_CGI_CALL(banksw, "bank-sw", bank_sw);
HTTPD_CGI_CALL(showvar,"show-var",show_variables);
HTTPD_CGI_CALL(stopbas, "stop-basic", stop_basic_prg);
HTTPD_CGI_CALL(resetbas, "reset-basic", reset_basic_prg);

static const struct httpd_cgi_call *calls[] = { &tcp, &prgbas, &runbas, &banksw, &cnfbas, &stopbas, &pausebas, &showvar, &resetbas,  NULL };

/*---------------------------------------------------------------------------*/
static
PT_THREAD(nullfunction(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_cgifunction
httpd_cgi(char *name)
{
  const struct httpd_cgi_call **f;

  /* Find the matching name in the table, return the function. */
  for(f = calls; *f != NULL; ++f) {
    if(strncmp((*f)->name, name, strlen((*f)->name)) == 0) {
      return (*f)->function;
    }
  }
  return nullfunction;
}

static void strcatint(char *t, char *r, int n)
{
  char s[ITOASIZE];
  strcat(t, r);
  strcat(t, myltoa(s,n));
}

/*---------------------------------------------------------------------------*/
static const char closed[] =   /*  "CLOSED",*/
{0x43, 0x4c, 0x4f, 0x53, 0x45, 0x44, 0};
static const char syn_rcvd[] = /*  "SYN-RCVD",*/
{0x53, 0x59, 0x4e, 0x2d, 0x52, 0x43, 0x56,
 0x44,  0};
static const char syn_sent[] = /*  "SYN-SENT",*/
{0x53, 0x59, 0x4e, 0x2d, 0x53, 0x45, 0x4e,
 0x54,  0};
static const char established[] = /*  "ESTABLISHED",*/
{0x45, 0x53, 0x54, 0x41, 0x42, 0x4c, 0x49, 0x53, 0x48,
 0x45, 0x44, 0};
static const char fin_wait_1[] = /*  "FIN-WAIT-1",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49,
 0x54, 0x2d, 0x31, 0};
static const char fin_wait_2[] = /*  "FIN-WAIT-2",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49,
 0x54, 0x2d, 0x32, 0};
static const char closing[] = /*  "CLOSING",*/
{0x43, 0x4c, 0x4f, 0x53, 0x49,
 0x4e, 0x47, 0};
static const char time_wait[] = /*  "TIME-WAIT,"*/
{0x54, 0x49, 0x4d, 0x45, 0x2d, 0x57, 0x41,
 0x49, 0x54, 0};
static const char last_ack[] = /*  "LAST-ACK"*/
{0x4c, 0x41, 0x53, 0x54, 0x2d, 0x41, 0x43,
 0x4b, 0};

static const char *states[] = {
  closed,
  syn_rcvd,
  syn_sent,
  established,
  fin_wait_1,
  fin_wait_2,
  closing,
  time_wait,
  last_ack};
  

static unsigned short
generate_tcp_stats(void *arg)
{
  static const char tdtd[]="</td><td>";
  struct uip_conn *conn;
  struct httpd_state *s = (struct httpd_state *)arg;
  char *u = (char *)uip_appdata;
  char p[2] = " ";
    
  conn = &uip_conns[s->count];
  *u = '\0';
  
  strcatint(u,"<tr><td>",htons(conn->lport));
  strcatint(u,tdtd,htons(conn->ripaddr[0]) >> 8);
  strcatint(u,".",htons(conn->ripaddr[0]) & 0xff);
  strcatint(u,".",htons(conn->ripaddr[1]) >> 8);
  strcatint(u,".",htons(conn->ripaddr[1]) & 0xff);
  strcatint(u,":",htons(conn->rport));
  strcat(u,tdtd);
  strcat(u,states[conn->tcpstateflags & UIP_TS_MASK]);
  strcatint(u,tdtd,conn->nrtx);
  strcatint(u,tdtd,conn->timer);
  p[0] = (uip_outstanding(conn))? '*':' ';
  strcat(u,tdtd);
  strcat(u,p);
  p[0] = (uip_stopped(conn))? '!':' ';
  strcat(u," ");
  strcat(u,p);
  strcat(u,"</td></tr>\r\n");
  return strlen(u);
}

/*---------------------------------------------------------------------------*/
static
PT_THREAD(tcp_stats(struct httpd_state *s, char *ptr))
{
  
  PSOCK_BEGIN(&s->sout);

  for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
    if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
      PSOCK_GENERATOR_SEND(&s->sout, generate_tcp_stats, s);
    }
  }

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
char const ampesc[5] = "&amp;";
char const ltesc[4] = "&lt;";
char const gtesc[4] = "&gt;";
char const quotesc[6] = "&quot;";

static int unpack_program(void)
{
  untokstat *uts =  &program_space->uts;
  char *u = program_space->workbuf;
  int len = 0, c, mss = WORKBUFLEN - 10;

  while (len < mss) {
	  if ((c=untokenizecode(uts)) == 0)
		break;
	  switch (c) {
			case '&':  memcpy(&u[len],ampesc,5);
					   len += 5;
					   break;
			case '<':  memcpy(&u[len],ltesc,4);
					   len += 4;
					   break;
			case '>':  memcpy(&u[len],gtesc,4);
					   len += 4;
					   break;
			case '\"': memcpy(&u[len],quotesc,6);
					   len += 6;
					   break;
			default:   u[len++] = c;
					   break;
	  }
  }  
  return len;
}

static const char unavailable[] = "Program Operation Unavailable";
static const char runprogram[] = "Running Program";
static const char switchingbank[] = "Switching To Program Bank ";
static const char stopprogram[] = "Stopping Program";
static const char pauseprogram[] = "Pausing Program";
static const char resumeprogram[] = "Resuming Program";
static const char notrunprogram[] = "Program is not running";
static const char alreadyrunprogram[] = "Program is already running";
static const char programispaused[] = "Program is already paused, use Run to resume";

static
PT_THREAD(program_basic(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  if (allocate_program_space()) {
	s->is_special_server = SPECIAL_SERVER_UNTOK;
	inituntokenizestate(&program_space->uts, script);
	for (;;) {
		s->count = unpack_program();
		if (s->count > 0)
			PSOCK_SEND(&s->sout, program_space->workbuf, s->count);
		else break;
	}
	cleanup_special_state(s);
  } else {
	PSOCK_SEND(&s->sout, alreadyrunprogram, sizeof(alreadyrunprogram)-1);
  }
  
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static int unpack_conf(void)
{
  untokstat *uts =  &program_space->uts;
  char *u = program_space->workbuf;
  int len = 0, c, mss = WORKBUFLEN - 10;

  while (len < mss) {
	  if ((c=untokenizeverbatim(uts)) == 0)
		break;
	  switch (c) {
			case '&':  memcpy(&u[len],ampesc,5);
					   len += 5;
					   break;
			case '<':  memcpy(&u[len],ltesc,4);
					   len += 4;
					   break;
			case '>':  memcpy(&u[len],gtesc,4);
					   len += 4;
					   break;
			case '\"': memcpy(&u[len],quotesc,6);
					   len += 6;
					   break;
			default:   u[len++] = c;
					   break;
	  }
  }  
  return len;
}

static
PT_THREAD(conf_basic(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  if (allocate_program_space()) {
	s->is_special_server = SPECIAL_SERVER_UNTOK;
	inituntokenizestate(&program_space->uts, configuration_space);
	for (;;) {
		s->count = unpack_conf();
		if (s->count > 0)
			PSOCK_SEND(&s->sout, program_space->workbuf, s->count);
		else break;
	}
	cleanup_special_state(s);
  } else {
	PSOCK_SEND(&s->sout, alreadyrunprogram, sizeof(alreadyrunprogram)-1);
  }
  
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(run_basic_prg(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  if ((!basic_running) && (program_space == NULL)) {
    PSOCK_SEND(&s->sout, runprogram, sizeof(runprogram)-1);
	run_basic = 1;
  } else {
	if ((basic_running) && (program_space == NULL) && (pause_basic != 0)) {
		PSOCK_SEND(&s->sout, resumeprogram, sizeof(resumeprogram)-1);
		pause_basic = 0;
	} else {
		PSOCK_SEND(&s->sout, alreadyrunprogram, sizeof(alreadyrunprogram)-1);
	}
  }
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(stop_basic_prg(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  if ((basic_running) && (program_space == NULL)) {
    PSOCK_SEND(&s->sout, stopprogram, sizeof(stopprogram)-1);
	run_basic = 0;
  } else {
	PSOCK_SEND(&s->sout, notrunprogram, sizeof(notrunprogram)-1);
  }
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(pause_basic_prg(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  if ((basic_running) && (program_space == NULL) && (pause_basic == 0)) {
    PSOCK_SEND(&s->sout, pauseprogram, sizeof(pauseprogram)-1);
	pause_basic = 1;
  } else {
	if (pause_basic) {
		PSOCK_SEND(&s->sout, programispaused, sizeof(programispaused)-1);
	} else {
		PSOCK_SEND(&s->sout, notrunprogram, sizeof(notrunprogram)-1);
	}
  }
  PSOCK_END(&s->sout);
}

static const char swval[] = "/banksw.shtml?";
static const char bankval[] = "<br><A HREF=\"banksw.shtml?X\">Bank #X, </A> Contents: ";
static const char endbankbr[] = "<br>";
/*---------------------------------------------------------------------------*/
static unsigned short
generate_bank_sw(void *arg)
{
  struct httpd_state *s = (struct httpd_state *)arg;
  char *u = (char *)uip_appdata;

  strcpy(u,bankval);
  if (bankname(s->count,80,&u[sizeof(bankval)-1]) < 0) {
		strcpy(u,endbankbr);
		s->count = 10;
  } else {
		u[26] = u[35] = s->count + '0';
		s->count++;
  }
  return strlen(u);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(bank_sw(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  if ((!basic_running) && (program_space == NULL)) {
	if (strncmp(s->filename,swval,sizeof(swval)-1)) {
		s->count = 0;
		while ((s->count < 10) && (!basic_running) && (program_space == NULL))
			PSOCK_GENERATOR_SEND(&s->sout, generate_bank_sw, s);
	} else {
		select_bank_number(s->filename[sizeof(swval)-1]-'0');
		PSOCK_SEND(&s->sout, switchingbank, sizeof(switchingbank)-1);
		PSOCK_SEND(&s->sout, &s->filename[sizeof(swval)-1], 1);
	}
  } else {
	PSOCK_SEND(&s->sout, alreadyrunprogram, sizeof(alreadyrunprogram)-1);
  }
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static unsigned short
generate_show_variables(void *arg)
{
  struct httpd_state *s = (struct httpd_state *)arg;
  char *u = (char *)uip_appdata;
  int len = uip_mss() - 5, valid;

  memset(u,' ',len);
  u[0] = 0;
  if ((s->count < BASIC_DEBUG_TYPES) && (basic_running) && (program_space == NULL)) 
  {
	do
	{
		valid = basic_debug_interface(s->count, s->len, u, 10, &u[14], len - 14);
		u[12] = '=';
		if (valid) {
			u[strlen(u)]=' ';
			s->len++;
		} else {
			s->count++;
			s->len = 0;
		}
	} while ((!valid) && (s->count < BASIC_DEBUG_TYPES));
  }
  strcat(u,"\r\n");
  return strlen(u);
}

static
PT_THREAD(show_variables(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);

  s->count = 0;
  s->len = 0;
  while ((s->count < BASIC_DEBUG_TYPES) && (basic_running) && (program_space == NULL)) 
		PSOCK_GENERATOR_SEND(&s->sout, generate_show_variables, s);
  if ((!basic_running)||(program_space != NULL))
    PSOCK_SEND(&s->sout, notrunprogram, sizeof(notrunprogram)-1);
  
  PSOCK_END(&s->sout);
}
/** @} */
/*---------------------------------------------------------------------------*/
static
PT_THREAD(reset_basic_prg(struct httpd_state *s, char *ptr))
{
  PSOCK_BEGIN(&s->sout);
  force_reboot();  
  PSOCK_END(&s->sout);
}
