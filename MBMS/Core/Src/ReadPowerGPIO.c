/*
 * ReadPowerGPIO.c
 *
 *  Created on: Jan 11, 2025
 *      Author: m
 */

#include "../Inc/ReadPowerGPIO.h"
#include <stdint.h>
#include "main.h"
#include "CANdefines.h"


uint16_t readMainPowerSwitch(void) {
    return HAL_GPIO_ReadPin(MAIN_PWR_SW_GPIO_Port, MAIN_PWR_SW_Pin);  // PC4
}

// um maybe dont need this EPCOS no more
uint16_t readEPCOSwitch(void) {
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8);  // PB8 NOT SURE ABT THIS ONE!!! ASK SOMEBODY!!!!
}

uint16_t read_nDCDC0_ON(void) {
    return HAL_GPIO_ReadPin(nDCDC0_ON_GPIO_Port, nDCDC0_ON_Pin);  // PE2
}

uint16_t read_nDCDC1_ON(void) {
    return HAL_GPIO_ReadPin(nDCDC1_ON_GPIO_Port, nDCDC1_ON_Pin);  // PE4
}

uint16_t readKeySwitch(void) {
    return HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin);  // PB1
}
