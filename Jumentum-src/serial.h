#ifndef SERIAL_H_
#define SERIAL_H_

int putchar_serial_c(int com, int ch);
int getkey_serial_c(int com);
int waitkey_serial_c(int com);
void putstring_serial_c (int com, const char *string);
int putc_serial_c(int com, int ch);
void init_serial_c (int com, unsigned long baudrate, int databits, int stopbits, int parity );

extern char console_uart;

#endif
