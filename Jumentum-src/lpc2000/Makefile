SRC_FILES=clock.c io.c iointr.c i2c.c cpu.c iap.c serial.c serial-irql.c enc28j60.c lpcmac.c spi.c rs232dev.c flashbanks.o vic.c sbrk.c  usb/usbcontrol.c usb/usbhw_lpc.c usb/usbinit.c usb/usbstdreq.c usb/usb_serial.c usb/usb_serial_fifo.c usb/usbprintf.c usb/usb_serial_irq.c
ASM_FILES=crt.s swi-handler.s 

#
# Define all object files.
#
ARM_OBJ = $(SRC_FILES:.c=.o) $(ASM_FILES:.s=.o)

.PHONY: all
all: $(ARM_OBJ)

clean:
	rm -f *.o

io.o: io.c
	$(CC) -o io.o $(CFLAGS) ${THUMBMODE} -c io.c
	$(AR) r ../libcpu.a $@

iointr.o: iointr.c
	$(CC) -o iointr.o $(CFLAGS) -c iointr.c
	$(AR) r ../libcpu.a $@

i2c.o: i2c.c
	$(CC) -o i2c.o $(CFLAGS) ${THUMBMODE} -c i2c.c
	$(AR) r ../libcpu.a $@

cpu.o: cpu.c
	$(CC) -o cpu.o $(CFLAGS) ${THUMBMODE} -c cpu.c
	$(AR) r ../libcpu.a $@
	
iap.o: iap.c
	$(CC) -o iap.o $(CFLAGS) ${THUMBMODE} -c iap.c
	$(AR) r ../libcpu.a $@

flashbanks.o: flashbanks.c
	$(CC) -o flashbanks.o $(CFLAGS) ${THUMBMODE} -c flashbanks.c
	$(AR) r ../libcpu.a $@
	
serial.o: serial.c
	$(CC) -o serial.o $(CFLAGS) ${THUMBMODE} -c serial.c
	$(AR) r ../libcpu.a $@

sbrk.o: sbrk.c
	$(CC) -o sbrk.o $(CFLAGS) ${THUMBMODE} -c sbrk.c
	$(AR) r ../libcpu.a $@
	

serial-irql.o: serial-irql.c
	$(CC) -o serial-irql.o $(CFLAGS) -c serial-irql.c
	$(AR) r ../libcpu.a $@

vic.o: vic.c
	$(CC) -o vic.o $(CFLAGS) -c vic.c
	$(AR) r ../libcpu.a $@

clock.o: clock.c
	$(CC) -o clock.o $(CFLAGS) -c clock.c
	$(AR) r ../libcpu.a $@
	
enc28j60.o: enc28j60.c
	$(CC) -o enc28j60.o $(CFLAGS) ${THUMBMODE} -c enc28j60.c
	$(AR) r ../libcpu.a $@

lpcmac.o: lpcmac.c
	$(CC) -o lpcmac.o $(CFLAGS) ${THUMBMODE} -c lpcmac.c
	$(AR) r ../libcpu.a $@

rs232dev.o: rs232dev.c
	$(CC) -o rs232dev.o $(CFLAGS) ${THUMBMODE} -c rs232dev.c
	$(AR) r ../libcpu.a $@
	
spi.o: spi.c
	$(CC) -o spi.o $(CFLAGS) ${THUMBMODE} -c spi.c
	$(AR) r ../libcpu.a $@
	
crt.o: crt.s
	@ echo ".assembling"
	$(CC) -c -Wa,-ahlms=crt.lst  $(CRTSDEFINES) crt.s -o crt.o
#	$(AR) r ../libcpu.a $@

swi-handler.o: swi-handler.s
	@ echo ".assembling"
	$(CC) -c -Wa,-ahlms=swi-handler.lst swi-handler.s -o swi-handler.o
	$(AR) r ../libcpu.a $@

usb/usbcontrol.o: usb/usbcontrol.c
	$(CC) -o usb/usbcontrol.o $(CFLAGS) ${THUMBMODE} -c usb/usbcontrol.c
	$(AR) r ../libcpu.a $@

usb/usbhw_lpc.o: usb/usbhw_lpc.c
	$(CC) -o usb/usbhw_lpc.o $(CFLAGS) ${THUMBMODE} -c usb/usbhw_lpc.c
	$(AR) r ../libcpu.a $@
	
usb/usbinit.o: usb/usbinit.c
	$(CC) -o usb/usbinit.o $(CFLAGS) ${THUMBMODE} -c usb/usbinit.c
	$(AR) r ../libcpu.a $@
	
usb/usbstdreq.o: usb/usbstdreq.c
	$(CC) -o usb/usbstdreq.o $(CFLAGS) ${THUMBMODE} -c usb/usbstdreq.c
	$(AR) r ../libcpu.a $@
	
usb/usb_serial.o: usb/usb_serial.c
	$(CC) -o usb/usb_serial.o $(CFLAGS) $(THUMBMODE) -c usb/usb_serial.c
	$(AR) r ../libcpu.a $@

usb/usb_serial_irq.o: usb/usb_serial_irq.c
	$(CC) -o usb/usb_serial_irq.o $(CFLAGS) -c usb/usb_serial_irq.c
	$(AR) r ../libcpu.a $@
	
usb/usb_serial_fifo.o: usb/usb_serial_fifo.c
	$(CC) -o usb/usb_serial_fifo.o $(CFLAGS) ${THUMBMODE} -c usb/usb_serial_fifo.c
	$(AR) r ../libcpu.a $@

usb/usbprintf.o: usb/usbprintf.c
	$(CC) -o usb/usbprintf.o $(CFLAGS) ${THUMBMODE} -c usb/usbprintf.c
	$(AR) r ../libcpu.a $@
