SRC_FILES=clock.c cpu.c iap.c spi.c io.c i2c.c serial.c lpcmac.c rs232dev.c flashbanks.o sbrk.c 
VIDEO_SRC_FILES=vt100.c font.c video.c keyboard.c
ASM_FILES=startup_LPC17xx.s
SUBDIRS = libdriver

#
# Define all object files.
#
ARM_OBJ =  $(ASM_FILES:.s=.o) $(SRC_FILES:.c=.o) $(VIDEO_SRC_FILES:.c=.o)

.PHONY: all
all: subdirs $(ARM_OBJ)

subdirs:
	@for i in $(SUBDIRS); do \
	(cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) all); done

clean:
	rm -f *.o

io.o: io.c
	$(CC) -o io.o $(CFLAGS) ${THUMBMODE} -c io.c
	$(AR) r ../libcpu.a $@

i2c.o: i2c.c
	$(CC) -o i2c.o $(CFLAGS) ${THUMBMODE} -c i2c.c
	$(AR) r ../libcpu.a $@

cpu.o: cpu.c
	$(CC) -o cpu.o $(CFLAGS) ${THUMBMODE} -c cpu.c
	$(AR) r ../libcpu.a $@

system_LPC17xx.o: system_LPC17xx.c
	$(CC) -o system_LPC17xx.o $(CFLAGS) ${THUMBMODE} -c system_LPC17xx.c
	$(AR) r ../libcpu.a $@

core_cm3.o: core_cm3.c
	$(CC) -o core_cm3.o $(CFLAGS) ${THUMBMODE} -c core_cm3.c
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
	$(CC) -o serial-irql.o $(CFLAGS) ${THUMBMODE} -c serial-irql.c
	$(AR) r ../libcpu.a $@

clock.o: clock.c
	$(CC) -o clock.o $(CFLAGS) ${THUMBMODE} -c clock.c
	$(AR) r ../libcpu.a $@
	
lpcmac.o: lpcmac.c
	$(CC) -o lpcmac.o -Ilibdriver $(CFLAGS) ${THUMBMODE} -c lpcmac.c
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

video.o: video.c
	$(CC) -o video.o $(CFLAGS) ${THUMBMODE} -c video.c
	$(AR) r ../libcpu.a $@

vt100.o: vt100.c
	$(CC) -o vt100.o $(CFLAGS) ${THUMBMODE} -c vt100.c
	$(AR) r ../libcpu.a $@

font.o: font.c
	$(CC) -o font.o $(CFLAGS) ${THUMBMODE} -c font.c
	$(AR) r ../libcpu.a $@

keyboard.o: keyboard.c
	$(CC) -o keyboard.o $(CFLAGS) ${THUMBMODE} -c keyboard.c
	$(AR) r ../libcpu.a $@

startup_LPC17xx.o: startup_LPC17xx.s
	@ echo ".assembling"
	$(CC) -c -Wa,-ahlms=crt.lst  $(CRTSDEFINES) startup_LPC17xx.s -o startup_LPC17xx.o
#	$(AR) r ../libcpu.a $@
