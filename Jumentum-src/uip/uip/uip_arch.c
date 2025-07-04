/*
 * Copyright (c) 2001, Adam Dunkels.
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
 * $Id: uip_arch.c,v 1.2.2.1 2003/10/04 22:54:17 adam Exp $
 *
 */


#include "uip.h"
#include "uip_arch.h"
#define BUF ((struct __attribute__((packed))uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define IP_PROTO_TCP    6

/*-----------------------------------------------------------------------------------*/
void
uip_add32(u8_t *op32, u16_t op16)
{
  
  uip_acc32[3] = op32[3] + (op16 & 0xff);
  uip_acc32[2] = op32[2] + (op16 >> 8);
  uip_acc32[1] = op32[1];
  uip_acc32[0] = op32[0];
  
  if(uip_acc32[2] < (op16 >> 8)) {
    ++uip_acc32[1];    
    if(uip_acc32[1] == 0) {
      ++uip_acc32[0];
    }
  }
  
  
  if(uip_acc32[3] < (op16 & 0xff)) {
    ++uip_acc32[2];  
    if(uip_acc32[2] == 0) {
      ++uip_acc32[1];    
      if(uip_acc32[1] == 0) {
	++uip_acc32[0];
      }
    }
  }
}
/*-----------------------------------------------------------------------------------*/
u16_t
uip_chksum(u16_t *sdata, u16_t len)
{
  u16_t acc;
  
  for (acc = 0; len > 1; len -= 2) {
    u16_t u = ((unsigned char *)sdata)[0] + (((unsigned char *)sdata)[1] << 8);
    if ((acc += u) < u) {
      /* Overflow, so we add the carry to acc (i.e., increase by
         one). */
      ++acc;
    }
    ++sdata;
  }

  /* add up any odd byte */
  if(len == 1) {
    acc += htons(((u16_t)(*(u8_t *)sdata)) << 8);
    if(acc < htons(((u16_t)(*(u8_t *)sdata)) << 8)) {
      ++acc;
    }
  }

  return acc;
}
/*-----------------------------------------------------------------------------------*/
u16_t
uip_ipchksum(void)
{
  return uip_chksum((u16_t *)&uip_buf[UIP_LLH_LEN], 20);
}
/*-----------------------------------------------------------------------------------*/
u16_t
uip_tcpchksum(void)
{
  u16_t hsum, sum;

  /* Compute the checksum of the TCP header. */
  hsum = uip_chksum((u16_t *)&uip_buf[20 + UIP_LLH_LEN], 20);

  /* Compute the checksum of the data in the TCP packet and add it to
     the TCP header checksum. */
  sum = uip_chksum((u16_t *)uip_appdata,
		   (u16_t)(((((u16_t)(BUF->len[0]) << 8) + BUF->len[1]) - 40)));

  if((sum += hsum) < hsum) {
    ++sum;
  }
  
  if((sum += BUF->srcipaddr[0]) < BUF->srcipaddr[0]) {
    ++sum;
  }
  if((sum += BUF->srcipaddr[1]) < BUF->srcipaddr[1]) {
    ++sum;
  }
  if((sum += BUF->destipaddr[0]) < BUF->destipaddr[0]) {
    ++sum;
  }
  if((sum += BUF->destipaddr[1]) < BUF->destipaddr[1]) {
    ++sum;
  }
  if((sum += (u16_t)htons((u16_t)UIP_PROTO_TCP)) < (u16_t)htons((u16_t)UIP_PROTO_TCP)) {
    ++sum;
  }

  hsum = (u16_t)htons((((u16_t)(BUF->len[0]) << 8) + BUF->len[1]) - 20);
  
  if((sum += hsum) < hsum) {
    ++sum;
  }

  return sum;
}

/*---------------------------------------------------------------------------*/
static u16_t
chksum(u16_t sum, const u8_t *data, u16_t len)
{
  u16_t t;
  const u8_t *dataptr;
  const u8_t *last_byte;

  dataptr = data;
  last_byte = data + len - 1;
  
  while(dataptr < last_byte) {	/* At least two more bytes */
    t = (dataptr[0] << 8) + dataptr[1];
    sum += t;
    if(sum < t) {
      sum++;		/* carry */
    }
    dataptr += 2;
  }
  
  if(dataptr == last_byte) {
    t = (dataptr[0] << 8) + 0;
    sum += t;
    if(sum < t) {
      sum++;		/* carry */
    }
  }

  /* Return sum in host byte order. */
  return sum;
}

u16_t
uip_udpchksum(void)
{
  u16_t upper_layer_len;
  u16_t sum;
  
#if UIP_CONF_IPV6
  upper_layer_len = (((u16_t)(BUF->len[0]) << 8) + BUF->len[1]);
#else /* UIP_CONF_IPV6 */
  upper_layer_len = (((u16_t)(BUF->len[0]) << 8) + BUF->len[1]) - UIP_IPH_LEN;
#endif /* UIP_CONF_IPV6 */
  
  /* First sum pseudoheader. */
  
  /* IP protocol and length fields. This addition cannot carry. */
  sum = upper_layer_len + UIP_PROTO_UDP;
  /* Sum IP source and destination addresses. */
  sum = chksum(sum, (u8_t *)&BUF->srcipaddr[0], 2 * sizeof(uip_ipaddr_t));

  /* Sum TCP header and data. */
  sum = chksum(sum, &uip_buf[UIP_IPH_LEN + UIP_LLH_LEN],
	       upper_layer_len);
    
  return (sum == 0) ? 0xffff : htons(sum);
}

#if 0
/*-----------------------------------------------------------------------------------*/
u16_t
uip_udpchksum(void)
{
  u16_t hsum, sum;

  /* Compute the checksum of the UDP header. */
  hsum = uip_chksum((u16_t *)&uip_buf[20 + UIP_LLH_LEN], 8);

  /* Compute the checksum of the data in the UDP packet and add it to
     the UDP header checksum. */
  sum = uip_chksum((u16_t *)uip_appdata,
		   (u16_t)(((((u16_t)(BUF->len[0]) << 8) + BUF->len[1]) - 24)));

  if((sum += hsum) < hsum) {
    ++sum;
  }
  
  if((sum += BUF->srcipaddr[0]) < BUF->srcipaddr[0]) {
    ++sum;
  }
  if((sum += BUF->srcipaddr[1]) < BUF->srcipaddr[1]) {
    ++sum;
  }
  if((sum += BUF->destipaddr[0]) < BUF->destipaddr[0]) {
    ++sum;
  }
  if((sum += BUF->destipaddr[1]) < BUF->destipaddr[1]) {
    ++sum;
  }
  if((sum += (u16_t)htons((u16_t)UIP_PROTO_UDP)) < (u16_t)htons((u16_t)UIP_PROTO_UDP)) {
    ++sum;
  }

  hsum = (u16_t)htons((((u16_t)(BUF->len[0]) << 8) + BUF->len[1]) - 20);
  
  if((sum += hsum) < hsum) {
    ++sum;
  }

  return sum;
}
#endif
