/* Minimal wiringPiSPI stubs for build-only (no hardware). */
#ifndef WIRINGPI_SPI_STUB_H
#define WIRINGPI_SPI_STUB_H

#include <stdint.h>

static inline int wiringPiSPISetup(int channel, int speed) { (void)channel; (void)speed; return 0; }
static inline int wiringPiSPIDataRW(int channel, unsigned char *data, int len) { (void)channel; (void)data; return len; }

#endif // WIRINGPI_SPI_STUB_H
