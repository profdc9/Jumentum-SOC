/**
 * \addtogroup apps
 * @{
 */

/**
 * \defgroup httpd Web server
 * @{
 * The uIP web server is a very simplistic implementation of an HTTP
 * server. It can serve web pages and files from a read-only ROM
 * filesystem, and provides a very small scripting language.

 */

/**
 * \file
 *         Web server
 * \author
 *         Adam Dunkels <adam@sics.se>
 */


/*
 * Copyright (c) 2004, Adam Dunkels.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: httpd.c,v 1.2 2006/06/11 21:46:38 adam Exp $
 */

#include <string.h>

#include "httpd.h"
#include "httpd-fs.h"
#include "httpd-cgi.h"
#include "http-strings.h"
#include "../../uip/uip.h"
#include "../../uip/timer.h"
#include "../../uip/pt.h"

#include "../../clock.h"
#include "../../all.h"
#include "../../lib.h"
#include "../../main.h"
#include "../../net.h"

#define STATE_WAITING 0
#define STATE_OUTPUT  1

#define ISO_nl      0x0a
#define ISO_space   0x20
#define ISO_bang    0x21
#define ISO_percent 0x25
#define ISO_period  0x2e
#define ISO_slash   0x2f
#define ISO_colon   0x3a

static unsigned short cur_conn_id = 0;
static unsigned short basic_state_conn_id = 0;
struct httpd_state   *basic_state_conn;
static unsigned char *basic_state_data;
static int            basic_state_len;
static char	          basic_state_active;
static char           basic_state_term;
static unsigned char *basic_state_post_data;
static int            basic_state_post_len;
static int            basic_state_actual_post_len;


#if 0
#define DEBUGHTTPD(x) putstring_serial0(x)
#else
#define DEBUGHTTPD(x)
#endif

/*---------------------------------------------------------------------------*/
static void reset_basic_state(void)
{
  basic_state_data = NULL;
  basic_state_conn = NULL;
  basic_state_len = 0;
  basic_state_active = 0;
  basic_state_term = 0;
  basic_state_post_data = NULL;
  basic_state_post_len = 0;
  basic_state_actual_post_len = 0;
}
/*---------------------------------------------------------------------------*/
static unsigned short
generate_part_of_file(void *state)
{
  struct httpd_state *s = (struct httpd_state *)state;

  if(s->file.len > uip_mss()) {
    s->len = uip_mss();
  } else {
    s->len = s->file.len;
  }
  memcpy(uip_appdata, s->file.data, s->len);
  
  return s->len;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_file(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);
  
  do {
    PSOCK_GENERATOR_SEND(&s->sout, generate_part_of_file, s);
    s->file.len -= s->len;
    s->file.data += s->len;
  } while(s->file.len > 0);
      
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_part_of_file(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);

  PSOCK_SEND(&s->sout, s->file.data, s->len);
  
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static void
next_scriptstate(struct httpd_state *s)
{
  char *p;
  p = strchr(s->scriptptr, ISO_nl) + 1;
  s->scriptlen -= (unsigned short)(p - s->scriptptr);
  s->scriptptr = p;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_script(struct httpd_state *s))
{
  char *ptr;
  
  PT_BEGIN(&s->scriptpt);


  while(s->file.len > 0) {

    /* Check if we should start executing a script. */
    if(*s->file.data == ISO_percent &&
       *(s->file.data + 1) == ISO_bang) {
      s->scriptptr = s->file.data + 3;
      s->scriptlen = s->file.len - 3;
      if(*(s->scriptptr - 1) == ISO_colon) {
	httpd_fs_open(s->scriptptr + 1, &s->file);
	PT_WAIT_THREAD(&s->scriptpt, send_file(s));
      } else {
	PT_WAIT_THREAD(&s->scriptpt,
		       httpd_cgi(s->scriptptr)(s, s->scriptptr));
      }
      next_scriptstate(s);
      
      /* The script is over, so we reset the pointers and continue
	 sending the rest of the file. */
      s->file.data = s->scriptptr;
      s->file.len = s->scriptlen;
    } else {
      /* See if we find the start of script marker in the block of HTML
	 to be sent. */

      if(s->file.len > uip_mss()) {
	s->len = uip_mss();
      } else {
	s->len = s->file.len;
      }

      if(*s->file.data == ISO_percent) {
	ptr = strchr(s->file.data + 1, ISO_percent);
      } else {
	ptr = strchr(s->file.data, ISO_percent);
      }
      if(ptr != NULL &&
	 ptr != s->file.data) {
	s->len = (int)(ptr - s->file.data);
	if(s->len >= uip_mss()) {
	  s->len = uip_mss();
	}
      }
      PT_WAIT_THREAD(&s->scriptpt, send_part_of_file(s));
      s->file.data += s->len;
      s->file.len -= s->len;
      
    }
  }
  
  PT_END(&s->scriptpt);
}
/*---------------------------------------------------------------------------*/
char *httpd_basic_webreq(unsigned short conn_id, int datanum)
{
   static char empty[1] = { 0 };
   if ((!basic_state_active) || (basic_state_term) || (conn_id != basic_state_conn_id))
		return empty;
   switch (datanum) {
		case 0: basic_state_conn->filename[sizeof(basic_state_conn->filename)-1] = 0;
				return basic_state_conn->filename;
		case 1: return basic_state_conn->request_type == HTTP_POST ? "P" : "G";
   }
   return empty;
}
/*---------------------------------------------------------------------------*/
int httpd_basic_connection(void)
{
   netmainloop();
   return ((basic_state_active) && (!basic_state_term)) ? basic_state_conn_id : -1;
}
/*---------------------------------------------------------------------------*/
int httpd_relay_html(unsigned char *data, int len, unsigned short conn_id)
{
   if ((!basic_state_active) || (basic_state_term) || (conn_id != basic_state_conn_id))
		return -1;
   if (data == NULL) {
		basic_state_term = 1;
		netmainloop();
		return 0;
   }
   if (len > 0) {
		basic_state_data = data;
		basic_state_len = len;
		while (basic_state_data != NULL) {
			if ((!basic_state_active) || (conn_id != basic_state_conn_id)) {
				return 1;
			}
			netmainloop();
		}
   }
   return 0;
}
/*---------------------------------------------------------------------------*/
int httpd_get_post(unsigned char *data, int len, unsigned short conn_id)
{
   if ((!basic_state_active) || (basic_state_term) || (conn_id != basic_state_conn_id))
		return 0;
   basic_state_post_data = data;
   basic_state_post_len = len;
   basic_state_actual_post_len = 0;
   while (basic_state_post_data != NULL) {
		if ((!basic_state_active) || (conn_id != basic_state_conn_id))
			return basic_state_actual_post_len;
		netmainloop();
	}
   return basic_state_actual_post_len;
}
/*---------------------------------------------------------------------------*/
int httpd_get_form(unsigned char *cval, unsigned char *var, unsigned char *data)
{
   int len = 0, varlen = strlen(var);
	
   while (cval != NULL) {
		if (*cval == '&') cval++;
		if ((strncmp(cval, var, varlen)==0) && (cval[varlen] == '=')) {
			cval += (varlen + 1);
			while ((*cval) && (*cval != '&')) {
				if (*cval == '+') {
					if (data) *data++ = ' ';
					len++;
				} else if (*cval == '%') {
					unsigned char ch0, ch1;
					if (( (ch0 = toupper(cval[1])) != 0) && ( (ch1 = toupper(cval[2])) != 0)) {
						if (data) {
							*data++ = (((ch0 > '9') ? (ch0 - 'A' + 10) : (ch0 - '0')) << 4) +
										((ch1 > '9') ? (ch1 - 'A' + 10) : (ch1 - '0'));
						}
						len++;
						cval+=2;
					}
				} else {
					if (data) *data++ = *cval;
					len++;
				}
				cval++;
			}
			break;
		}
		cval = strchr(cval, '&');
   }   
   return len;
}
/*---------------------------------------------------------------------------*/
int httpd_escape_form(unsigned char *cval, unsigned char *var)
{
	int len = 0;
	while (*cval) {
		switch (*cval) {
			case '&':  if (var) memcpy(&var[len],ampesc,5);
					   len += 5;
					   break;
			case '<':  if (var) memcpy(&var[len],ltesc,4);
					   len += 4;
					   break;
			case '>':  if (var) memcpy(&var[len],gtesc,4);
					   len += 4;
					   break;
			case '\"': if (var) memcpy(&var[len],quotesc,6);
					   len += 6;
					   break;
			default:   if (var) var[len] = *cval;
					   len++;
					   break;
		}
		cval++;
	}
	return len;
}

static
PT_THREAD(send_basic_packet(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);
  for (;;) {
	DEBUGHTTPD("WAITING\n");
	PT_WAIT_THREAD(&s->sout.pt,((!basic_running) || (basic_state_term) || (basic_state_data != NULL)));
	if ((!basic_running) || (basic_state_term))
		break;
	while (basic_state_len > 0) {
		s->len = (basic_state_len > uip_mss()) ? uip_mss() : basic_state_len;
		PSOCK_SEND(&s->sout,basic_state_data,basic_state_len);
		basic_state_len -= s->len;
		basic_state_data += s->len;	
	}
	basic_state_data = NULL;
  }
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_headers(struct httpd_state *s, const char *statushdr, int dofil))
{
  char *ptr;

  PSOCK_BEGIN(&s->sout);

  PSOCK_SEND_STR(&s->sout, statushdr);

  if (dofil) {
	ptr = strrchr(s->filename, ISO_period);
	if(ptr == NULL) {
		PSOCK_SEND_STR(&s->sout, http_content_type_binary);
	} else if(strncmp(http_html, ptr, 5) == 0 ||
			strncmp(http_shtml, ptr, 6) == 0) {
		PSOCK_SEND_STR(&s->sout, http_content_type_html);
	} else if(strncmp(http_css, ptr, 4) == 0) {
		PSOCK_SEND_STR(&s->sout, http_content_type_css);
	} else if(strncmp(http_png, ptr, 4) == 0) {
		PSOCK_SEND_STR(&s->sout, http_content_type_png);
	} else if(strncmp(http_gif, ptr, 4) == 0) {
		PSOCK_SEND_STR(&s->sout, http_content_type_gif);
	} else if(strncmp(http_jpg, ptr, 4) == 0) {
		PSOCK_SEND_STR(&s->sout, http_content_type_jpg);
	} else {
		PSOCK_SEND_STR(&s->sout, http_content_type_plain);
	}
  }
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_basic(struct httpd_state *s, int raw))
{
  char *ptr;
  
  PT_BEGIN(&s->basicpt);

  PT_WAIT_THREAD(&s->basicpt,
			send_headers(s,
			http_header_200, 0));
  if (!raw) {
			PT_WAIT_THREAD(&s->basicpt,
		 	send_headers(s,
			http_content_type_html, 0));
  }
  PT_WAIT_THREAD(&s->basicpt, send_basic_packet(s));
  PT_END(&s->basicpt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_prg(struct httpd_state *s))
{
  char *ptr;
  
  PT_BEGIN(&s->basicpt);

  PT_WAIT_THREAD(&s->basicpt,
			send_headers(s,
			http_header_200, 0));
  PT_WAIT_THREAD(&s->basicpt,
		 	send_headers(s,
			http_content_type_html, 0));
  PT_WAIT_THREAD(&s->basicpt, s->count == 4);
  PT_WAIT_THREAD(&s->basicpt,
		 	send_headers(s, http_redirect_prg, 0));
  PT_END(&s->basicpt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_output(struct httpd_state *s))
{
  char *ptr;
  
  PT_BEGIN(&s->outputpt);

  if (*auth_password) {
	if (!(s->is_authorized)) {
		PT_WAIT_THREAD(&s->outputpt,
					send_headers(s,
					http_header_401, 0));
		goto endhandleoutput;
	}
  }
  if (s->is_special_server == SPECIAL_SERVER_BASIC) {
	PT_INIT(&s->basicpt);
	PT_WAIT_THREAD(&s->outputpt, handle_basic(s,s->filename[4]=='r'));
    cleanup_special_state(s);
  } else if (s->is_special_server == SPECIAL_SERVER_PRG) {
	PT_INIT(&s->basicpt);
	PT_WAIT_THREAD(&s->outputpt, handle_prg(s));
    cleanup_special_state(s);
  } else {
	if (s->request_type == HTTP_SEND_503) {
		PT_WAIT_THREAD(&s->outputpt,
					send_headers(s,
					http_header_503, 0));
		goto endhandleoutput;
	} else if(!httpd_fs_open(s->filename, &s->file)) {
		httpd_fs_open(http_404_html, &s->file);
		strcpy(s->filename, http_404_html);
		PT_WAIT_THREAD(&s->outputpt,
			send_headers(s,
			http_header_404, 1));
		PT_WAIT_THREAD(&s->outputpt,
			send_file(s));
	} else {
		PT_WAIT_THREAD(&s->outputpt,
			send_headers(s,
			http_header_200, 1));
		ptr = strchr(s->filename, ISO_period);
		if(ptr != NULL && strncmp(ptr, http_shtml, 6) == 0) {
			PT_INIT(&s->scriptpt);
			PT_WAIT_THREAD(&s->outputpt, handle_script(s));
		} else {
		PT_WAIT_THREAD(&s->outputpt, send_file(s));
		}
	}
  }
endhandleoutput:
  PSOCK_CLOSE(&s->sout);
  PT_END(&s->outputpt);
}
/*---------------------------------------------------------------------------*/
void do_program_basic(struct httpd_state *s, char *buf, int len)
{
   int c;
   while (len>0) {
	  c = *buf++;
	  len--;
	  switch (s->count) {
		  case 0:  if (c == '=') 
				     s->count = 1;
				   break;
	  	  case 1:  if (c == '%')
				   {
					 s->count = 2;
					 s->temp = 0;
					 break;
				   }
   				   retokenize(c == '+' ? ' ' : c, &program_space->rr);
				   break;
		  case 2:
		  case 3:  c = toupper(c);
		    	   s->temp = (s->temp << 4) + (((c > '9') ? (c - 'A' + 10) : (c - '0')));
				   if (s->count == 3) {
					  s->count = 1;
					  if (s->temp != '\r'){
						retokenize(s->temp, &program_space->rr);
					  }
				   } else s->count = 3;
				   break;
	  }
   }
}

static int check_authorization(const char *base64pass)
{
	int authed, len;
	static char mypass64[sizeof(auth_password)*2+2] = { 0 };
	if (!(*mypass64)) {
		char mypass[sizeof(auth_password)+2];
		mypass[0]=':';
		strcpy(&mypass[1],auth_password);
		encode64(mypass,mypass64);
	}
	len = strlen(mypass64);
	authed = (strncmp(base64pass,mypass64,len) == 0);
	if ((base64pass[len] != 0) && (!isspace(base64pass[len])))
		authed = 0;
	return authed;
}

/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_input(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sin);

  PSOCK_READTO(&s->sin, ISO_space);

  if(strncmp(s->inputbuf, http_get, 4) == 0) {
    s->request_type = HTTP_GET;
  } else if(strncmp(s->inputbuf, http_post, 5) == 0) {
	s->request_type = HTTP_POST;
  } else {
    PSOCK_CLOSE_EXIT(&s->sin);
  }
  PSOCK_READTO(&s->sin, ISO_space);

  if(s->inputbuf[0] != ISO_slash) {
    PSOCK_CLOSE_EXIT(&s->sin);
  }
  if(s->inputbuf[1] == ISO_space) {
    strncpy(s->filename, http_index_shtml, sizeof(s->filename));
  } else {
    s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
    strncpy(s->filename, &s->inputbuf[0], sizeof(s->filename));
  }

  for (;;) {
    PSOCK_READTO(&s->sin, ISO_nl);

	if ((s->inputbuf[0] == '\n') || (s->inputbuf[0] == '\r')) {
		break;
	}
    if(strncmp(s->inputbuf, http_content_length, sizeof(http_content_length)-1) == 0) {
	  s->content_length = mystrtol(&s->inputbuf[sizeof(http_content_length)-1], NULL);
    }
	if(strncmp(s->inputbuf, http_authorization, sizeof(http_authorization)-1) == 0) {
	  s->is_authorized = check_authorization(&s->inputbuf[ sizeof(http_authorization)-1 ]);
	}
  }

  if (strncmp(s->filename,http_bas_prefix,4) == 0) {
    PSOCK_WAIT_UNTIL(&s->sin, (((!basic_state_active) && (basic_running)) || (s->timer > 1000)));
    if ((basic_state_active) || (!basic_running)) {
		s->request_type = HTTP_SEND_503;
	} else {
		reset_basic_state();
		s->is_special_server = SPECIAL_SERVER_BASIC;
		basic_state_active = 1;
		basic_state_conn_id = s->conn_id;
		basic_state_conn = s;		
	}
  } else if ((strncmp(s->filename,http_prg_prefix,4) == 0) ||
			 (strncmp(s->filename,http_cnf_prefix,4) == 0))
  {
	if (allocate_program_space() == NULL) 
		s->request_type = HTTP_SEND_503;
	else {
		s->is_special_server = SPECIAL_SERVER_PRG;
		s->temp = 0;
		s->count = 0;
		if (s->filename[1] == 'c')
			initretoken(&program_space->rr, (unsigned int)configuration_space,
					(unsigned int)configuration_space_end, 1, 1);
		else
			initretoken(&program_space->rr, (unsigned int)script, (unsigned int)endscript, 0, 1);
	}
  }

  if(s->request_type != HTTP_POST) 
		s->content_length = 0;

  s->state = STATE_OUTPUT;
  while (s->content_length > 0) {
	s->sin.bufsize = s->content_length > (sizeof(s->inputbuf)-1) ? (sizeof(s->inputbuf)-1) : s->content_length;
	PSOCK_READBUF(&s->sin);
	s->content_length -= PSOCK_DATALEN(&s->sin);
	if (s->is_special_server == SPECIAL_SERVER_PRG) {
		do_program_basic(s, s->inputbuf, PSOCK_DATALEN(&s->sin));
		if (s->content_length == 0) {
			s->count = 4;
			retokenize(0, &program_space->rr);
			retokenize(-1, &program_space->rr);
		}
	} else if (s->is_special_server == SPECIAL_SERVER_BASIC) {
		PT_WAIT_THREAD(&s->sin.pt,((!basic_running) || (basic_state_term) || (basic_state_post_data != NULL)));
		if ((basic_running) && (!basic_state_term) && (basic_state_post_data != NULL)) {
			memcpy(basic_state_post_data, s->inputbuf, 
				PSOCK_DATALEN(&s->sin) >= basic_state_post_len ? basic_state_post_len : PSOCK_DATALEN(&s->sin));
			if (PSOCK_DATALEN(&s->sin) > basic_state_post_len) {
				basic_state_actual_post_len += basic_state_post_len;
				basic_state_post_len = 0;
				basic_state_post_data = NULL;
				s->content_length = 0;
			} else {
				basic_state_post_data += PSOCK_DATALEN(&s->sin);
				basic_state_actual_post_len += PSOCK_DATALEN(&s->sin);
				basic_state_post_len -= PSOCK_DATALEN(&s->sin);
				if (s->content_length == 0) 
					basic_state_post_data = NULL;
			}
		}
    }
  }
  PSOCK_END(&s->sin);
}
/*---------------------------------------------------------------------------*/
static void
handle_connection(struct httpd_state *s)
{
  handle_input(s);
  if(s->state == STATE_OUTPUT) {
    handle_output(s);
  }
}
/*---------------------------------------------------------------------------*/
void cleanup_special_state(struct httpd_state *s)
{ 
   if (s->is_special_server == SPECIAL_SERVER_BASIC)
		reset_basic_state();
   else if ((s->is_special_server == SPECIAL_SERVER_PRG) ||
			(s->is_special_server == SPECIAL_SERVER_UNTOK)) {
		deallocate_program_space();
   }	
   s->is_special_server = SPECIAL_SERVER_NOT;
}
/*---------------------------------------------------------------------------*/
void
httpd_appcall(void)
{
  struct httpd_state *s = (struct httpd_state *)&(uip_conn->appstate.httpd);

  if(uip_closed() || uip_aborted() || uip_timedout()) {
	DEBUGHTTPD("uip_closed\n");
	cleanup_special_state(s);
  } else if(uip_connected()) {
    PSOCK_INIT(&s->sin, s->inputbuf, sizeof(s->inputbuf) - 1);
    PSOCK_INIT(&s->sout, s->inputbuf, sizeof(s->inputbuf) - 1);
    PT_INIT(&s->outputpt);
    s->state = STATE_WAITING;
    s->timer = 0;
	s->content_length = 0;
	s->is_authorized = 0;
	s->is_special_server = SPECIAL_SERVER_NOT;
	s->conn_id = ++cur_conn_id;
    handle_connection(s);
  } else if(s != NULL) {
	if(uip_poll()) {
		++s->timer;
		if(s->timer >= 1500) {
		   DEBUGHTTPD("S_TIMER_OUT\n");
		   cleanup_special_state(s);
		   uip_abort();
		}
    } else {
      s->timer = 0;
    }
    handle_connection(s);
  } else {
	DEBUGHTTPD("uip_abort\n");
	cleanup_special_state(s);
    uip_abort();
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Initialize the web server
 *
 *             This function initializes the web server and should be
 *             called at system boot-up.
 */
void
httpd_init(void)
{
  basic_state_conn_id = cur_conn_id = 0;
  reset_basic_state();
  uip_listen(http_port);
}
/*---------------------------------------------------------------------------*/
/** @} */
