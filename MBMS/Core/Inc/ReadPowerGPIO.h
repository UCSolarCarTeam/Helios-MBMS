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

uint16_t readMainPowerSwitch(void);

uint16_t readEPCOSwitch(void);
uint16_t read_nDCDC0_ON(void);

uint16_t read_nDCDC1_ON(void);
uint16_t readKeySwitch(void);

#endif /* INC_READPOWERGPIO_H_ */
