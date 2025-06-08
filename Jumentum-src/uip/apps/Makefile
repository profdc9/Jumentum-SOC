SRC_FILES=

#SUBDIRS=dhcpc hello-world resolv smtp telnetd webclient webserver
SUBDIRS=webserver telnetd dhcpc smtp socket
#
# Define all object files.
#
ARM_OBJ = $(SRC_FILES:.c=.o)

.PHONY: all
all: subdirs $(ARM_OBJ)

subdirs:
	@for i in $(SUBDIRS); do \
	(cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) all); done

clean:
	@for i in $(SUBDIRS); do \
	(cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) clean); done
	
$(ARM_OBJ) : %.o : %.c Makefile 
	$(CC) -c $(CFLAGS) ${THUMBMODE} $< -o $@
	$(AR) r ../../libuip.a $@

