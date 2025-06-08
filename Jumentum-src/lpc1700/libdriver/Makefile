

SRC_FILES=lpc17xx_adc.c lpc17xx_can.c lpc17xx_clkpwr.c lpc17xx_dac.c lpc17xx_emac.c lpc17xx_exti.c lpc17xx_gpdma.c lpc17xx_gpio.c lpc17xx_i2c.c lpc17xx_i2s.c lpc17xx_mcpwm.c lpc17xx_nvic.c lpc17xx_pinsel.c lpc17xx_pwm.c lpc17xx_qei.c lpc17xx_rit.c lpc17xx_rtc.c lpc17xx_spi.c lpc17xx_ssp.c lpc17xx_systick.c lpc17xx_timer.c lpc17xx_uart.c lpc17xx_wdt.c core_cm3.o system_LPC17xx.c usb\usbcontrol.c usb\usbhw_lpc.c usb\usbinit.c usb\usbstdreq.c usb\usb_serial.c usb\usb_serial_fifo.c usb\usbprintf.c

#
# Define all object files.
#
ARM_OBJ = $(SRC_FILES:.c=.o)

.PHONY: all
all: $(ARM_OBJ)

$(ARM_OBJ) : %.o : %.c Makefile 
	$(CC) -c $(CFLAGS) ${THUMBMODE} $< -o $@
	$(AR) r ../../libdrivers.a $@
	
clean:
	rm -f *.o

	
