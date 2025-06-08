#ifndef _USB_SERIAL_IRQ_H
#define _USB_SERIAL_IRQ_H

// forward declaration of interrupt handler
//void USBIntHandler(void) __attribute__ ((interrupt("IRQ")));
void USBIntHandler(void) __attribute__ ((naked));

#endif