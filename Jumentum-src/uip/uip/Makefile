SRC_FILES=psock.c timer.c uip-fw.c uip-neighbor.c uip-split.c uip.c uiplib.c uip_arp.c uip_arch.c

#
# Define all object files.
#
ARM_OBJ = $(SRC_FILES:.c=.o)

.PHONY: all
all: $(ARM_OBJ)

$(ARM_OBJ) : %.o : %.c Makefile 
	$(CC) -c $(CFLAGS) ${THUMBMODE} $< -o $@
	$(AR) r ../../libuip.a $@

#clock.o: clock.c
#	$(CC) -o clock.o $(CFLAGS) -c clock.c
#	$(AR) r ../../libuip.a clock.o
	
clean:
	rm -f *.o 

