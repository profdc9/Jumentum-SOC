/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: tapdev.c,v 1.8 2006/06/07 08:39:58 adam Exp $
 */

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/socket.h>

#ifdef linux
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#define DEVTAP "/dev/net/tun"
#else  /* linux */
#define DEVTAP "/dev/tap0"
#endif /* linux */

#include "../uip/uip/uip.h"
#include "netdev.h"
#include "net.h"

static int drop = 0;
static int fd;

extern uip_ipaddr_t default_hostaddr;

/*---------------------------------------------------------------------------*/
int
initMAC(void)
{
  u16_t adr1;
  u16_t adr2;
  char buf[1024];
  
  fd = open(DEVTAP, O_RDWR);
  if(fd == -1) {
    perror("tapdev: tapdev_init: open");
    exit(1);
  }

#ifdef linux
  {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP|IFF_NO_PI;
    if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
      perror(buf);
      exit(1);
    }
    /*   if (ioctl(fd, TUNSETNOCSUM, 1)< 0) {
      perror(buf);
      exit(1);
      } */
  }
#endif /* Linux */
  
  adr1=HTONS(default_hostaddr[0]);
  adr2=HTONS(default_hostaddr[1])+1;

  snprintf(buf, sizeof(buf), "ifconfig tap0 inet %d.%d.%d.%d",
	   (adr1 & 0xFF00) >> 8,
	   (adr1 & 0xFF),
	   (adr2 & 0xFF00) >> 8,
	   (adr2 & 0xFF));
  printf("command=%s\n",buf);
  system(buf);

#if 0
  system("echo 1 > /proc/sys/net/ipv4/ip_forward");

  snprintf(buf, sizeof(buf), "route add -host %d.%d.%d.%d dev tap0",
	   (adr1 & 0xFF00) >> 8,
	   (adr1 & 0xFF),
	   (adr2 & 0xFF00) >> 8,
	   (adr2 & 0xFF)-1);
  
  printf("echo command=%s\n",buf);
  system(buf);

  system("echo 1 > /proc/sys/net/ipv4/conf/tap0/proxy_arp");

  snprintf(buf, sizeof(buf), "arp -Ds %d.%d.%d.%d wlan0 pub",
	   (adr1 & 0xFF00) >> 8,
	   (adr1 & 0xFF),
	   (adr2 & 0xFF00) >> 8,
	   (adr2 & 0xFF)-1);
  
  printf("echo command=%s\n",buf);
  system(buf);
#endif

  return 0;

}
/*---------------------------------------------------------------------------*/
u16_t
MACRead(void)
{
  fd_set fdset;
  struct timeval tv, now;
  int ret;
  
  tv.tv_sec = 0;
  tv.tv_usec = 1000;


  FD_ZERO(&fdset);
  FD_SET(fd, &fdset);

  ret = select(fd + 1, &fdset, NULL, NULL, &tv);
  if(ret == 0) {
    return 0;
  }
  ret = read(fd, uip_buf, UIP_BUFSIZE);
  if(ret == -1) {
    perror("tap_dev: tapdev_read: read");
  }

  /*     printf("--- tap_dev: tapdev_read: read %d bytes\n", ret); 
        {
    int i;
    for(i = 0; i < 20; i++) {
      printf("%x ", uip_buf[i]);
    }
    printf("\n");
    } */
  /*  check_checksum(uip_buf, ret);*/
  return ret;
}
/*---------------------------------------------------------------------------*/
u16_t
MACWrite(void)
{
  int ret;
  /*     printf("tapdev_send: sending %d bytes\n", uip_len);  */
  /*  check_checksum(uip_buf, size);*/

  /*  drop++;
  if(drop % 8 == 7) {
    printf("Dropped a packet!\n");
    return;
    }*/
  ret = write(fd, uip_buf, uip_len);
  if(ret == -1) {
    perror("tap_dev: tapdev_send: writev");
    exit(1);
  }
}
/*---------------------------------------------------------------------------*/
