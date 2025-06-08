#ifndef NETDEV_H
#define NETDEV_H

#include "uip/uip/uip.h"

//initialisation routine
int initMAC(void);
// function to write and send a packet from the ENC28J60_H
u16_t MACWrite();
// function to read a byte (if there) into a buffer
u16_t MACRead();

void rs232dev_init(void);
void rs232dev_close(void);
u8_t rs232dev_read(void);
void rs232dev_send(void);
void rs232dev_wait_for_connect(void);

#endif  /* NETDEV_H */
