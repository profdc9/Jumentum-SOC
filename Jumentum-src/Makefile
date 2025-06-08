APPNAME = main

# Tool definitions
export CC		= arm-none-eabi-gcc
export LD		= arm-none-eabi-ld -v
export AR		= arm-none-eabi-ar
export AS		= arm-none-eabi-as
export CP		= arm-none-eabi-objcopy
export OD		= arm-none-eabi-objdump
export NM		= arm-none-eabi-nm
export RM		= rm
export OPENOCD  = K:/Program\ Files/Codesourcery/openocd-pp 
export OPENOCDCFG =   K:/Program\ Files/Codesourcery/u.cfg 
export THUMBMODE =

#BOARDTYPE = -DLPC1768 -DSPI1
# Special stuff for MBED
#BOARDTYPE = -DLPC1768
#BOARDTYPE = -DLPC1768 -DMBED -DLPCUSB
#BOARDTYPE = -DLPC1768 -DMBED -DLPCVIDEO
BOARDTYPE = -DLPC1768 -DMBED
#BOARDTYPE = -DLPC1768 -DLPC17XX_SSP1
LINKFILE	= lpc1700/LPC17xx.ld
export CRTSDEFINES = -Wa,--defsym -Wa,CPUTYPE=2378

# Configure whether serial console is on UART0 or UART1 - must not be
# the same as the port SLIP network interface is on (if configured)
CONSOLE = -DSTDOUTSER0
# CONSOLE = -DSTDOUTSER1

# Configure the type of network interface, either SLIP or ENC28J60 ethernet.
# Comment out one or the other.  With SLIP, one must choose if the SLIP
# is on UART0 or UART1

# Configuration for LPC1768 with built-in ethernet
NETWORKTYPE = -DLPCMAC -DINCNETWORK -DETHERNET
NETINTERFACE=net.o
ADDNETL = libuip.a

# Configuration for SLIP over a serial port
# could be SLIPSER0, SLIPSER1
# NETWORKTYPE = -DSLIPSER1 -DINCNETWORK
# NETINTERFACE=o net.o
# ADDNETL = libuip.a

# Include for FAT FS
ADDFATFS = -DINCFATFS
ADDFATFSL = libfatfs.a
# or not
#ADDFATFS =
#ADDFATFSL = 

# Comment out for no editor
ADDEDITOR = -DEDITOR
ADDEDITORO = editor.o
# Comment out for editor
#ADDEDITOR = 
#ADDEDITORO =

#Floating point support
#ADDFLOAT = 
#ADDFLOATL = lpc1700/lib/libm.a
#Uncomment for int only
ADDFLOAT = -DINTONLY
ADDFLOATL = 

#USE modified DLMalloc and built-in function alternatives that takes less space
ADDDLMALLOC = -DUSEDLMALLOC -DMYFUNC
ADDDLMALLOCO = malloc.o
ADDDLMALLOCL = 
# Uncomment to use newlib malloc
#ADDDLMALLOC = 
#ADDDLMALLOCO =
#ADDDLMALLOCL = lib/libc.a

# Tool flags
export CFLAGS  = -I./ -I../ -c -W -Wall -fomit-frame-pointer -Os -g -DDEBUG $(ADDFLOAT) $(ADDEDITOR) $(ADDDLMALLOC) $(BOARDTYPE) $(CONSOLE) $(NETWORKTYPE) $(ADDFATFS) -mcpu=cortex-m3 -mthumb
export ASFLAGS = -ahls -mapcs-32 -mcpu=cortex-m3 -mthumb
export LFLAGS  =  -nostartfiles -mcpu=cortex-m3 -mthumb -nostdlib $(ADDFATFSL) $(ADDDLMALLOCL) $(ADDFLOATL) $(ADDNETL) libcpu.a libdrivers.a lpc1700/lib/libgcc.a  --warn-common 
export CPFLAGS = -O ihex
#export ODFLAGS	= -x --syms
export ODFLAGS	= -x --syms -d -S
export ODFLAGSLSS	= -h -S -C
export NMFLAGS = -n

CSRCS	= 
OBJS 	= lpc1700/startup_LPC17xx.o $(CSRCS:.c=.o)

EXAMPLES = basic
SUBDIRS = uip fatfs lpc1700

all: subdirs $(EXAMPLES)

subdirs:
	@for i in $(SUBDIRS); do \
	(cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) all); done

basic:	$(OBJS) main.o basic.o lib.o $(NETINTERFACE) $(ADDDLMALLOCO) $(ADDEDITORO)

prg :
	lpc21isp\lpc21isp -control basic.hex com6 38400 12000
	
flash:
	$(OPENOCD) -d -f $(OPENOCDCFG)
	
$(EXAMPLES):
	@ echo "Building $@ example..."
	$(CC) -T $(LINKFILE) $^ -o $@.elf -Wl,-Map,$@.map $(LFLAGS) 
	$(CP) $(CPFLAGS) $@.elf $@.hex
	$(CP) -I elf32-little -O binary $@.elf $@.bin
	$(OD) $(ODFLAGS) $@.elf > $@.dmp
	$(OD) $(ODFLAGSLSS) $@.elf > $@.lss
	$(NM) $(NMFLAGS) $@.elf > $@.sym

malloc.o: malloc.c
	$(CC) -o malloc.o $(CFLAGS) ${THUMBMODE} -c malloc.c
	
main.o: main.c
	$(CC) -o main.o $(CFLAGS) ${THUMBMODE} -c main.c
	
basic.o: basic.c
	$(CC) -o basic.o $(CFLAGS) ${THUMBMODE} -c basic.c

editor.o: editor.c
	$(CC) -o editor.o $(CFLAGS) ${THUMBMODE} -c editor.c

lib.o: lib.c
	$(CC) -o lib.o $(CFLAGS) ${THUMBMODE} -c lib.c

net.o: net.c
	$(CC) -o net.o $(CFLAGS) ${THUMBMODE} -c net.c
	
clean:
#	for i in $(SUBDIRS); do \
#	(cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) clean); done
	clean.bat

# recompile if the Makefile changes
$(OBJS): Makefile

# phony targets
.PHONY: clean


