// Global definitions for Raspberry Pi build
#include <stdint.h>

// Mode and frequency selectors used by cc1100_raspi.cpp
int cc1100_freq_select = 0x03;    // default 868 MHz
int cc1100_mode_select = 0x03;    // default GFSK 100 kb
int cc1100_channel_select = 0;    // default channel 0

// Default device address
volatile uint8_t My_addr = 1;

// Debug flag
uint8_t cc1100_debug = 0;
