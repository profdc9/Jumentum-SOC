
#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "../../uip/psock.h"

void socket_appcall(void);

#define MAGIC 0xB4C29FBD

struct socketfifo
{
  u8_t *buf;
  unsigned short fifosize;
  volatile unsigned short fifohead;
  volatile unsigned short fifotail;
};

#define RECV_TIMEOUT  1000
#define TRANS_TIMEOUT 1000

struct socket_state {
  unsigned long magic;
  
  char *senddata;
  u16_t senddatalen;
  u16_t sendamt;
  u16_t recvthr;
  
  char connected;
  char closesock;
  char accepted;
  char stopped;
  
  unsigned short recvtimecount;
  unsigned short transtimecount; 
  
  unsigned short recvtimeout;
  unsigned short transtimeout; 

  struct socketfifo *fifo;
  
  struct psock psock;
};


typedef struct socket_state uip_tcp_appstate_t4;

int socket_connect(char *host, unsigned short port, int len);
int socket_accept(int port, int len);
int socket_newdata(int conn);
int socket_close(int conn);
int socket_write(int conn, char *data, int len);
int socket_read(int conn, char *data, int len, int endchar);
int socket_init(void);
int socket_isconnected(int conn);
void socket_free(void);

#endif /* __SMTP_H__ */

/** @} */
