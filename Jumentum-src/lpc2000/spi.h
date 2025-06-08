#ifndef SPI_H
#define SPI_H

#include "sysdefs.h"
#include "../spi.h"

#ifdef SPIBITBANG
#define SPI_ETH_SEL GPIO_IO_P8
#define SPI_ETH_RST GPIO_IO_P9
#define SPI_MMC_SEL GPIO_IO_P10
#endif

#ifdef SPI0
#define SPI_ETH_SEL GPIO_IO_P8
#define SPI_ETH_RST GPIO_IO_P9
#define SPI_MMC_SEL GPIO_IO_P10
#endif

#ifdef SPI1
#define SPI_MMC_SEL GPIO_IO_P20
#define SPI_ETH_SEL GPIO_IO_P21
#define SPI_ETH_RST GPIO_IO_P22
#endif

#define SPI_ETH_SPEED 0x10
#define SPI_MMC_SPEED 0x10

// public functions
#ifdef SPIBITBANG
U8 SPIBitBang(U8 c);
#endif

#endif
