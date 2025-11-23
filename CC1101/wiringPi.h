/* Minimal wiringPi stubs for build-only (no hardware). */
#ifndef WIRINGPI_STUB_H
#define WIRINGPI_STUB_H

#include <stdint.h>

#define INPUT 0
#define OUTPUT 1
#define HIGH 1
#define LOW 0

static inline int wiringPiSetup(void) { return 0; }
static inline void pinMode(int pin, int mode) { (void)pin; (void)mode; }
static inline void digitalWrite(int pin, int value) { (void)pin; (void)value; }
static inline int digitalRead(int pin) { (void)pin; return 0; }
static inline void delayMicroseconds(unsigned int howLong) { (void)howLong; }
static inline void delay(unsigned int howLong) { (void)howLong; }

#endif // WIRINGPI_STUB_H
