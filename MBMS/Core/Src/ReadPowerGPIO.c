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


uint16_t read_nMPS(void) {
    return HAL_GPIO_ReadPin(nMPS_GPIO_Port, nMPS_Pin);  // PC4
}

uint16_t read_ESD(void) {
    return HAL_GPIO_ReadPin(ESD_GPIO_Port, ESD_Pin);  // PC5
}

uint16_t read_EN1(void) {
    return HAL_GPIO_ReadPin(EN1_GPIO_Port, EN1_Pin);  // PE1
}

uint16_t read_nDCDC_Fault(void) {
    return HAL_GPIO_ReadPin(nDCDC_Fault_GPIO_Port, nDCDC_Fault_Pin);  // PC13
}

uint16_t read_n3A_OC(void) {
    return HAL_GPIO_ReadPin(n3A_OC_GPIO_Port, n3A_OC_Pin);
}

uint16_t read_nDCDC_On(void) {
    return HAL_GPIO_ReadPin(nDCDC_On_GPIO_Port, nDCDC_On_Pin);
}

uint16_t read_nCHG_Fault(void) {
    return HAL_GPIO_ReadPin(nCHG_Fault_GPIO_Port, nCHG_Fault_Pin);
}

uint16_t read_nCHG_On(void) {
    return HAL_GPIO_ReadPin(nCHG_On_GPIO_Port, nCHG_On_Pin);
}

uint16_t read_nCHG_LV_En(void) {
    return HAL_GPIO_ReadPin(nCHG_LV_En_GPIO_Port, nCHG_LV_En_Pin);
}

uint16_t read_ABATT_Disable(void) {
    return HAL_GPIO_ReadPin(ABATT_Disable_GPIO_Port, ABATT_Disable_Pin);



uint16_t read_Key(void) {
    return HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin);  // PB1
}
