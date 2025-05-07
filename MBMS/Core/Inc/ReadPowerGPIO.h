/*
 * readGPIO.h
 *
 *  Created on: Jan 11, 2025
 *      Author: m
 */

#ifndef INC_READPOWERGPIO_H_
#define INC_READPOWERGPIO_H_

#include <stdint.h>
#include "main.h"

uint16_t read_nMPS(void);

uint16_t read_ESD(void);

uint16_t read_EN1(void);

uint16_t read_nDCDC_Fault(void);

uint16_t read_n3A_OC(void);

uint16_t read_nDCDC_On(void);

uint16_t read_nCHG_Fault(void);

uint16_t read_nCHG_On(void);

uint16_t read_nCHG_LV_En(void);

uint16_t read_ABATT_Disable(void);

uint16_t read_Key(void);

uint16_t read_Charge_Enable(void);

uint16_t read_Discharge_Enable(void);

#endif /* INC_READPOWERGPIO_H_ */
