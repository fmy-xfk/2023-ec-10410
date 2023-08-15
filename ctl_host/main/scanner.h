#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "iot_uart.h"
#include "hi_uart.h"
#include "hi_io.h"

extern unsigned char scanner_data[1024];

unsigned int scanner_init();
unsigned int scanner_wakeup();
unsigned int scanner_start();
unsigned int scanner_stop();
unsigned int scanner_read();

#endif