#ifndef __PCAPDEV_H__
#define __PCAPDEV_H__

#include "../uip/uip/uip.h"

//initialisation routine
int initMAC(void);
// function to write and send a packet from the ENC28J60_H
u16_t MACWrite();
// function to read a byte (if there) into a buffer
u16_t MACRead();

#endif /* __PACAPDEV_H__ */
