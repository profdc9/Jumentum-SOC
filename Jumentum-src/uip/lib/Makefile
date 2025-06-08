SRC_FILES=memb.c

#
# Define all object files.
#
ARM_OBJ = $(SRC_FILES:.c=.o)

.PHONY: all
all: $(ARM_OBJ)

$(ARM_OBJ) : %.o : %.c Makefile 
	$(CC) -c $(CFLAGS) ${THUMBMODE} $< -o $@
	$(AR) r ../../libuip.a $@

clean:
	rm -f *.o 

