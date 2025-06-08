SRC_FILES=httpd.c httpd-cgi.c httpd-fs.c httpd-fsdata.c http-strings.c base64.c

#
# Define all object files.
#
ARM_OBJ = $(SRC_FILES:.c=.o)

.PHONY: all
all: $(ARM_OBJ)

$(ARM_OBJ) : %.o : %.c Makefile 
	$(CC) -c $(CFLAGS) -I../../uip -I../../lib ${THUMBMODE} $< -o $@
	$(AR) r ../../../libuip.a $@

clean:
	rm -f *.o 

