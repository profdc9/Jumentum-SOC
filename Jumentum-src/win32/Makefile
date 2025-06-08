SRC_FILES=clock.c io.c i2c.c cpu.c serial.c spi.c flashbanks.c pcapdev.c sbrk.c

#
# Define all object files.
#
ARM_OBJ = $(SRC_FILES:.c=.o) 

.PHONY: all
all: $(ARM_OBJ)

clean:
	rm -f *.o 

io.o: io.c
	$(CC) -o io.o $(CFLAGS)  -c io.c
	$(AR) r ../libcpu.a $@

i2c.o: i2c.c
	$(CC) -o i2c.o $(CFLAGS)  -c i2c.c
	$(AR) r ../libcpu.a $@

cpu.o: cpu.c
	$(CC) -o cpu.o $(CFLAGS)  -c cpu.c
	$(AR) r ../libcpu.a $@
	
iap.o: iap.c
	$(CC) -o iap.o $(CFLAGS)  -c iap.c
	$(AR) r ../libcpu.a $@

flashbanks.o: flashbanks.c
	$(CC) -o flashbanks.o $(CFLAGS)  -c flashbanks.c
	$(AR) r ../libcpu.a $@
	
serial.o: serial.c
	$(CC) -o serial.o $(CFLAGS)  -c serial.c
	$(AR) r ../libcpu.a $@

tapdriver.o: tapdriver.c
	$(CC) -o tapdriver.o $(CFLAGS) -c tapdriver.c
	$(AR) r ../libcpu.a $@

pcapdev.o: pcapdev.c
	$(CC) -I./wpd/include -o pcapdev.o $(CFLAGS) -c pcapdev.c
	$(AR) r ../libcpu.a $@

clock.o: clock.c
	$(CC) -o clock.o $(CFLAGS) -c clock.c
	$(AR) r ../libcpu.a $@
	
spi.o: spi.c
	$(CC) -o spi.o $(CFLAGS)  -c spi.c
	$(AR) r ../libcpu.a $@

sbrk.o: sbrk.c
	$(CC) -o sbrk.o $(CFLAGS)  -c sbrk.c
	$(AR) r ../libcpu.a $@
	
