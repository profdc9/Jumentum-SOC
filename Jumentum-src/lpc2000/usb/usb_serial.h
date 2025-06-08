#ifndef _USB_SERIAL_H
#define _USB_SERIAL_H

int usb_serial_init(void);
int VCOM_putchar(int c);
int VCOM_getchar(void);

#endif  /* _USB_SERIAL_H */