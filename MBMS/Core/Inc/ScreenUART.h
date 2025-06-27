#ifndef SCREEN_UART_H
#define SCREEN_UART_H

#include "MBMS_Screen.h"
#include <stdint.h>
#include <stddef.h>

#define SCREEN_DATA_BUFFER_SIZE 256  // Adjust as needed

void serializeScreenData(uint8_t *buffer, const ScreenDataDictionary *data, size_t *out_len);

#endif