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


// need to fix these after i redo config after elec changes it i guess ?
uint16_t read_nMPS(void) {
    return HAL_GPIO_ReadPin(nMPS_GPIO_Port, nMPS_Pin);  // PC4
}

uint16_t read_ESD(void) {
    return HAL_GPIO_ReadPin(ESD_GPIO_Port, ESD_Pin);  // PC5
}


//uint16_t read_nDCDC0_ON(void) {
//    return HAL_GPIO_ReadPin(nDCDC0_ON_GPIO_Port, nDCDC0_ON_Pin);  // PE2
//}
//
//uint16_t read_nDCDC1_ON(void) {
//    return HAL_GPIO_ReadPin(nDCDC1_ON_GPIO_Port, nDCDC1_ON_Pin);  // PE4
//}

uint16_t read_KeySwitch(void) {
    return HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin);  // PB1
}
