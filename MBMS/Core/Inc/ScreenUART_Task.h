#ifndef SCREEN_UART_H
#define SCREEN_UART_H

#include "MBMS_Screen.h"
#include <stdint.h>
#include <stddef.h>
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

extern ScreenDataDictionary screenData;
extern UART_HandleTypeDef huart4;

#define SCREEN_DATA_BUFFER_SIZE 256  // Adjust as needed

void serializeScreenData(uint8_t *buffer, const ScreenDataDictionary *data, size_t *out_len);
void UART_ScreenTask(void* arg);

#endif