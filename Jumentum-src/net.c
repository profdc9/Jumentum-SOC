/* Jumentum-SOC

  Copyright (C) 2007 by Daniel Marks

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
  
  Daniel L. Marks profdc9@gmail.com

*/

#include <stdio.h>
#include <stdlib.h>

#include "all.h"
#include "netdev.h"
#include "processor.h"
#include "uip/uip/uip.h"
#include "uip/uip/uip_arp.h"
#include "uip/uip/timer.h"
#include "uip/apps/webserver/webserver.h"
#include "uip/apps/telnetd/telnetd.h"
#include "uip/apps/smtp/smtp.h"
#include "uip/apps/socket/socket.h"
#include "uip/apps/dhcpc/dhcpc.h"
#include "serial.h"
#include "lib.h"
#include "net.h"
#include "main.h"

void timerISR(void);

unsigned short http_port = HTONS(80);
unsigned short telnet_port = HTONS(23);
unsigned char dodhcp = 1;
unsigned short checkinterval = 200;

uip_ipaddr_t default_hostaddr = { HTONS(0x0A00),HTONS(0x0004) };
uip_ipaddr_t default_netmask = { HTONS(0xFF00),HTONS(0x0000) };
uip_ipaddr_t default_router = { HTONS(0x0A00),HTONS(0x0001) };
#ifdef NEEDDNS
uip_ipaddr_t default_dns1 = { HTONS(0x0402),HTONS(0x0202) };
uip_ipaddr_t default_dns2 = { HTONS(0x0402),HTONS(0x0203) };
#endif
uip_ipaddr_t smtpserver = { HTONS(0x0A00),HTONS(0x0005) };
char localhostname[40] = "controller";

static void get_mac_addr(const char *adr, struct uip_eth_addr *mac)
{
	int n;
	const char *end;
	long v;
    for (n=0;n<6;n++) {
		v=mystrtolhex(adr,&end);
		mac->addr[n] = (u8_t) v;
		if (*end != ':') break;
		adr = end+1;
	}
}

static void get_ip_addr(char *adr, uip_ipaddr_t ipaddr)
{
	int n1, n2;
	const char *end;
	
	n1=mystrtol(adr,&end);
	if (*end == '.') end++;
	n2=mystrtol(end,&end);
	if (*end == '.') end++;
	ipaddr[0] = HTONS((n1 << 8) | n2);
	n1=mystrtol(end,&end);
	if (*end == '.') end++;
	n2=mystrtol(end,&end);	
	ipaddr[1] = HTONS((n1 << 8) | n2);
}


void setup_net_configuration_parms(void)
{
	char buf[80];

	if (get_conf_string_max("DHCP",buf,sizeof(buf))) {
		dodhcp = ((toupper(*buf) == 'Y') || (toupper(*buf) == 'T'));
	}
	if (get_conf_string_max("TELNETPORT",buf,sizeof(buf))) {
		telnet_port = HTONS(mystrtol(buf,NULL));
	}
	if (get_conf_string_max("HTTPPORT",buf,sizeof(buf))) {
		http_port = HTONS(mystrtol(buf,NULL));
	}
	if (get_conf_string_max("CHECKINTERVAL",buf,sizeof(buf))) {
		checkinterval = mystrtol(buf,NULL);
	}
	if (get_conf_string_max("MACADDR",buf,sizeof(buf))) {
		get_mac_addr(buf,&uip_ethaddr);
	}
	if (get_conf_string_max("HOSTADDR",buf,sizeof(buf))) {
		get_ip_addr(buf,default_hostaddr);
	}
	if (get_conf_string_max("NETMASK",buf,sizeof(buf))) {
		get_ip_addr(buf,default_netmask);
	}
	if (get_conf_string_max("ROUTER",buf,sizeof(buf))) {
		get_ip_addr(buf,default_router);
	}	
#ifdef NEEDDNS
	if (get_conf_string_max("DNS1",buf,sizeof(buf))) {
		get_ip_addr(buf,default_dns1);
	}	
	if (get_conf_string_max("DNS2",buf,sizeof(buf))) {
		get_ip_addr(buf,default_dns2);
	}	
#endif
	if (get_conf_string_max("SMTP",buf,sizeof(buf))) {
		get_ip_addr(buf,smtpserver);
	}	
	get_conf_string_max("HOSTNAME",localhostname,sizeof(localhostname));
}

void dispatch_appcall(void)
{
	if (uip_conn->lport == telnet_port)
		telnetd_appcall();
	else if (uip_conn->lport == http_port)
		httpd_appcall();
	else if (uip_conn->rport == HTONS(25))
		smtp_appcall();
	else socket_appcall();
}

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
/***********************************************************************/
/** \brief Main loop for controlling uIP
 *
 * Description: Test Code for uip. Primarily based upon uIP example code
 *              
 *              
 * \author Iain Derrington
 * \date WIP
 */
/**********************************************************************/

static struct timer periodic_timer,arp_timer;

#undef VDEBUG

static char netinit = 0;

static void remainder_init(void)
{
  telnetd_init();
  httpd_init();
  smtp_init();
  socket_init();  
}

static void set_default_addr(void)
{
#ifdef VDEBUG
  outstr("uip_ipaddr\n");
#endif
  uip_sethostaddr(&default_hostaddr);
  uip_setnetmask(&default_netmask);
  uip_setdraddr(&default_router);
}

int initialize_network(void)
{
  //cpuSetupHardware();
#ifdef ETHERNET
  if (initMAC() < 0) return -1;     // Setup ethernet 
#else
  rs232dev_init();
#endif
  
  uip_init();                    // Setup TCP/IP stack
#ifdef ETHERNET
  uip_arp_init();
#endif

  clock_init();   // Start Clock

  timer_set(&periodic_timer, CLOCK_SECOND/60);
  timer_set(&arp_timer, CLOCK_SECOND*10);

if (!dodhcp) {
  set_default_addr();
  remainder_init();
} else {
  dhcpc_init(uip_ethaddr.addr, sizeof(uip_ethaddr.addr));
  dhcpc_request();
}
  netinit = 1;
  return 0;
}


void dhcpc_configured(const struct dhcpc_state *s)
{
  if ((s->ipaddr[0] == 0) && (s->ipaddr[1] == 0)) {
	set_default_addr();
  } else {
	uip_sethostaddr(s->ipaddr);
	uip_setnetmask(s->netmask);
	uip_setdraddr(s->default_router);
  }
  remainder_init();
}

void outip(char *t)
{
  uip_ipaddr_t ipaddr;
  uip_gethostaddr(&ipaddr);
  outstr(t);
  outint(uip_ipaddr1(&ipaddr));
  outchar('.');
  outint(uip_ipaddr2(&ipaddr));
  outchar('.');
  outint(uip_ipaddr3(&ipaddr));
  outchar('.');
  outint(uip_ipaddr4(&ipaddr));
  outchar('\n');
}

static short check = 0;
void casualnetmainloop(void)
{
	if (((timer_expired(&periodic_timer))) || (++check > checkinterval))
		while (netmainloop());
}

int netmainloop(void)
{
	volatile int i;
#ifndef ETHERNET
    static unsigned short tick = 0;
#endif
    int didwork = 0;

	check = 0;
    WATCHDOG_UPDATE();
	if (!netinit) return 0;
#ifdef ETHERNET
		uip_len = MACRead();
#else
		uip_len = rs232dev_poll(tick);
#endif
		if(uip_len > 0) 
		{
			didwork = 1;
#ifdef ETHERNET
			if(BUF->type == htons(UIP_ETHTYPE_IP)) 
			{
				uip_arp_ipin();
				uip_input();
			/* If the above function invocation resulted in data that
			should be sent out on the network, the global variable
			uip_len is set to a value > 0. */
				if(uip_len > 0) 
				{		
					uip_arp_out();
					MACWrite();
				}    
			}
			else if (BUF->type == htons(UIP_ETHTYPE_ARP))
			{
				uip_arp_arpin();
				/* If the above function invocation resulted in data that
				should be sent out on the network, the global variable
				uip_len is set to a value > 0. */
				if(uip_len > 0) 
					MACWrite();
			}
#else
			uip_process(UIP_DATA);
			if (uip_len > 0)
				rs232dev_send();
#endif
		}
		else if ((timer_expired(&periodic_timer)))
		{
			timer_reset(&periodic_timer);
#ifndef ETHERNET
			tick++;
#endif
			for(i = 0; i < UIP_CONNS; i++) 
			{
				uip_periodic(i);
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
				if(uip_len > 0) 
				{
#ifdef ETHERNET
					uip_arp_out();
					MACWrite();
#else
					rs232dev_send();
#endif
					didwork = 1;
				}
			}
			for(i = 0; i < UIP_UDP_CONNS; i++) {
				uip_udp_periodic(i);
				if(uip_len > 0) 
				{
#ifdef ETHERNET
					uip_arp_out();
					MACWrite();
#else
					rs232dev_send();
#endif
					didwork = 1;
				}
			}
		} 
		if (timer_expired(&arp_timer)) 
		{
			timer_reset(&arp_timer);
#ifdef ETHERNET
			uip_arp_timer();
#endif
		}
	return didwork;
}

void uip_log(char *msg)
{
   //debug_printf("%s \n",msg);
}
