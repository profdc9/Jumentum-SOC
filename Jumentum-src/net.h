#ifndef _NET_H_
#define _NET_H_

void dispatch_appcall(void);
int initialize_network(void);
int netmainloop(void);
void casualnetmainloop(void);
void outip(char *t);
void setup_net_configuration_parms(void);
extern unsigned short http_port;
extern unsigned short telnet_port;
extern unsigned short checkinterval;

#endif
