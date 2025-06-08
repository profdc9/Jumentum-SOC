#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "socket.h"
#ifdef USEDNS
#include "../resolv/resolv.h"
#endif
#include "../../uip/uip.h"
#include "../../uip/uiplib.h"
#include "../../uip/timer.h"
#include "../../uip/pt.h"

#include "../../clock.h"
#include "../../all.h"
#include "../../lib.h"
#include "../../main.h"
#include "../../net.h"

static u16_t acceptlen = 128;
static struct socketfifo sockarray[UIP_CONNS];

static void freesocketfifo(struct socketfifo *fifo)
{
	if (fifo->buf != NULL)
		free(fifo->buf);
	fifo->buf = NULL;
}

static int initsocketfifo(struct socketfifo *fifo, int len)
{
	freesocketfifo(fifo);
	fifo->buf = malloc(len);
	if (fifo->buf == NULL) return -1;
	fifo->fifosize = len;
	fifo->fifohead = fifo->fifotail =  0;
	return 0;
}

static void putsocketfifo(struct socketfifo *fifo, int ch)
{
	int newpos = fifo->fifohead + 1;
	if (fifo->buf == NULL) return;
	if (newpos >= fifo->fifosize) newpos = 0;	
	if (newpos == fifo->fifotail)
		return;
	fifo->buf[fifo->fifohead] = ch;
	fifo->fifohead = newpos;
}

static int getsocketfifo(struct socketfifo *fifo)
{	
	int ch;
	if (fifo->buf == NULL) return -1;
	if (fifo->fifotail == fifo->fifohead)
		return -1;
	ch = fifo->buf[fifo->fifotail++];
	if (fifo->fifotail >= fifo->fifosize)
		fifo->fifotail = 0;
	return ch;
}

static int fifoused(struct socketfifo *fifo)
{
	int rem = fifo->fifohead - fifo->fifotail;
	if (fifo->buf == NULL) return 0;
	return (rem < 0) ? (rem + fifo->fifosize) : rem;
}

static void init_socket_conn(struct socket_state *s) 
{
  s->accepted = s->connected = 0 ;
  s->sendamt = s->senddatalen = s->recvthr = 0;
  s->senddata = NULL;
  s->fifo = NULL;
  s->closesock = s->stopped = 0;
  s->recvtimeout = RECV_TIMEOUT;
  s->transtimeout = TRANS_TIMEOUT;
  s->magic = MAGIC;
}

static int whichconn(struct uip_conn *conn) 
{
  int i;
  for (i=0;i<UIP_CONNS;i++) {
	 if (conn == &uip_conns[i])
		return i;
  }
  return -1;
}

void socket_appcall(void)
{
  int i;
  struct socket_state *s = (struct socket_state *)&(uip_conn->appstate.socketst);

  if(uip_connected()) {
    if ((i = whichconn(uip_conn)) < 0)
		uip_abort();
	else {
		init_socket_conn(s);	
		s->connected = 1;
		s->transtimecount = s->recvtimecount = 0;
		s->fifo = &sockarray[i];
		if (s->fifo->buf ==  NULL) 
			initsocketfifo(s->fifo, acceptlen);
		i = s->fifo->fifosize - UIP_BUFSIZE;
		s->recvthr = (i < 0) ? 0 : i;
	}
  }
  if (s->magic == MAGIC) {
	if (uip_poll())
	{
		s->transtimecount++;
		s->recvtimecount++;
	}
	if (uip_rexmit() && (s->senddata != NULL)) {
		uip_send(s->senddata, s->sendamt);
		s->transtimecount = 0;
	} else if (s->senddata != NULL) {
		if (uip_acked()) {
			s->senddatalen -= s->sendamt;
			s->senddata += s->sendamt;
			s->sendamt = 0;
			if (s->senddatalen == 0)
				s->senddata = NULL;
		}
		if ((s->senddatalen > 0) && (s->sendamt == 0)) {
			s->sendamt = s->senddatalen > uip_mss() ? uip_mss() : s->senddatalen;
			uip_send(s->senddata, s->sendamt);
			s->transtimecount = 0;
		}
	}
	if (uip_newdata()) {
		s->recvtimecount = 0;
		for (i=0;i<uip_datalen();i++)
			putsocketfifo(s->fifo, ((u8_t *)uip_appdata)[i]);
		if (fifoused(s->fifo) >= s->recvthr) {
			uip_stop();
			s->stopped = 1;
		}
	}
	if ((s->fifo != NULL) && (fifoused(s->fifo) < s->recvthr) && (s->stopped)) {
		s->stopped = 0;
		uip_restart();
	}
	if(uip_closed() || uip_aborted() || uip_timedout()) {
		s->magic = s->accepted = s->connected = 0;
	}
	if ((s->fifo == NULL) || (s->fifo->buf == NULL)) 
		s->closesock = 1;
	if (s->closesock) {
		uip_close();
	}
  }
}

static struct socket_state *get_sock(int conn)
{
   struct socket_state *s;
   
   if ((conn<0) || (conn>=UIP_CONNS))
		return NULL;
   s=(struct socket_state *)&(uip_conns[conn].appstate.socketst);
   return (s->magic == MAGIC) && (s->connected) ? s : NULL;
}

int socket_read(int conn, char *data, int len, int endchar)
{
	int c,  ilen;
	struct socketfifo *fifo;
    struct socket_state *s;
	
	if ((conn<0)||(conn>=UIP_CONNS))
		return -1;
	fifo = &sockarray[conn];
    s=(struct socket_state *)&(uip_conns[conn].appstate.socketst);
	ilen = len;
	s->recvtimecount = 0;
	while (len>0) {
		c = getsocketfifo(fifo);
		if (c<0) {
			if ((endchar != -2) && (s->magic == MAGIC) && (s->connected) &&
					((s->recvtimecount < s->recvtimeout) || (s->recvtimeout != 0)))
			{
				netmainloop();
				continue;
			} else break;
		}
		*data++ = c;
		len--;
		if (c == endchar) break;
	}
	return ilen-len;
}

int socket_write(int conn, char *data, int len)
{
	struct socket_state *s;
	if ((s=get_sock(conn)) == NULL)
		return -1;
	s->senddatalen = len;
	s->senddata = data;
	s->transtimecount = 0;
	while ((s->senddata != NULL) && (s->magic == MAGIC) && (s->connected) && 
				((s->transtimecount < s->transtimeout) || (s->transtimeout == 0)))
		netmainloop();
	return (s->senddata == NULL) ? 0 : -1;
}

void socket_free(void)
{
	int i;
	for (i=0;i<UIP_CONNS;i++) 
		freesocketfifo(&sockarray[i]);
}

int socket_close(int conn)
{
	struct socket_state *s;
	
	if ((conn >= 0)&&(conn < UIP_CONNS))
		freesocketfifo(&sockarray[conn]);
	if ((s=get_sock(conn)) == NULL)
		return -1;
	s->closesock = 1;
	netmainloop();
	return 0;
}

int socket_isconnected(int conn)
{
	struct socket_state *s;
	if ((s=get_sock(conn)) == NULL)
		return 0;
	return s->connected;
}

int socket_newdata(int conn)
{
	if ((conn >= 0) && (conn < UIP_CONNS))
		return fifoused(&sockarray[conn]);
	return -1;
}

int socket_accept(int port, int len)
{
  int i;
  struct uip_conn *conn;
  struct socket_state *s;
  
  acceptlen = len;
  port=htons(port);
  uip_unlisten(port);
  uip_listen(port);
  netmainloop();
  for (i=0;i<UIP_CONNS;i++) {
	 conn = &uip_conns[i];
	 s=(struct socket_state *)&(conn->appstate.socketst);
	 if ((s->magic == MAGIC) && (!s->accepted) &&
		(s->connected) && (conn->lport == port)) break;
  }
  if (i < UIP_CONNS) {
      s->accepted = 1;
	  return i;
  }
  return -1;
}

int socket_connect(char *host, unsigned short port, int len)
{
  int i;
  struct uip_conn *conn;
  uip_ipaddr_t *ipaddr;
  static uip_ipaddr_t addr;
  
  /* First check if the host is an IP address. */
  ipaddr = &addr;
  if(uiplib_ipaddrconv(host, (unsigned char *)addr) == 0) {
#ifdef USEDNS
    ipaddr = (uip_ipaddr_t *)resolv_lookup(host);
    
    if(ipaddr == NULL) {
      return -1;
    }
#else
	return -1;
#endif
  }
  conn = uip_connect(ipaddr, htons(port));
  if ((i = whichconn(conn)) < 0)
		return -1;
  init_socket_conn((struct socket_state *)&(conn->appstate.socketst)); 
  initsocketfifo(&sockarray[i], len);
  
  return i;
}

int socket_init(void)
{
	memset(sockarray, '\0', sizeof(sockarray));
	return 0;
}
