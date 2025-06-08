#ifndef __RS232DEV_H__
#define __RS232DEV_H__

#include "../uip/uip/uip.h"

#include "../netdev.h"

#define rs232_waitkey_serial() waitkey_serial_c(slip_uart)
#define rs232_init_serial() init_serial_c(slip_uart,38400,8,1,0)
#define rs232_getkey_serial() getkey_serial_c(slip_uart)
#define rs232_putc_serial(x) putc_serial_c(slip_uart,x)
#define rs232_putstring_serial(x) putstring_serial_c(slip_uart,x)

#if UIP_BUFSIZE > 255
u16_t rs232dev_poll(unsigned short tick);
#else 
u8_t rs232dev_poll(unsigned short tick);
#endif /* UIP_BUFSIZE > 255 */

#define CLIENT_TICK_THRESHOLD 10

#endif /* __RS232DEV_H__ */
