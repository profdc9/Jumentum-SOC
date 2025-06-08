#ifndef SERIAL_H_
#define SERIAL_H_

#include "../serial.h"

void serial0_isr (void)   __attribute__ ((naked));
void serial1_isr (void)   __attribute__ ((naked));

#define UART0_RX_BUFFER_SIZE 128
#define UART1_RX_BUFFER_SIZE 128

extern unsigned char  uart0_rx_buffer[];
extern unsigned short uart0_rx_insert_idx, uart0_rx_extract_idx;

extern unsigned char  uart1_rx_buffer[];
extern unsigned short uart1_rx_insert_idx, uart1_rx_extract_idx;


#endif
