
#ifndef _SPI_H
#define _SPI_H

#include "sysdefs.h"

// public functions
void spiInit (void);
U16  SPIWrite(U8 *, U16 );
U16  SPIRead(U8 *, U16 );
void spiEthChipSelect(BOOL select);
void spiMMCChipSelect (BOOL select);
void spiSendByte (U8 c);
U8 spiReceiveByte (void);
void spiReceivePtr (U8 *c);
void spiResetEthernet(void);

#endif   /* SPI_H */
