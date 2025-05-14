/*
 * readGPIO.h
 *
 *  Created on: Jan 11, 2025
 *      Author: m
 */

#ifndef INC_READPOWERGPIO_H_
#define INC_READPOWERGPIO_H_

GPIO_PinState read_nMPS(void);

GPIO_PinState read_ESD(void);

GPIO_PinState read_EN1(void);

GPIO_PinState read_nDCDC_Fault(void);

GPIO_PinState read_n3A_OC(void);

GPIO_PinState read_nDCDC_On(void);

GPIO_PinState read_nCHG_Fault(void);

GPIO_PinState read_nCHG_On(void);

GPIO_PinState read_nCHG_LV_En(void);

GPIO_PinState read_ABATT_Disable(void);

GPIO_PinState read_Key(void);

GPIO_PinState read_Charge_Enable(void);

GPIO_PinState read_Discharge_Enable(void);

GPIO_PinState read_LV_OC(void);

#endif /* INC_READPOWERGPIO_H_ */
