/*
 * readGPIO.h
 *
 *  Created on: Jan 11, 2025
 *      Author: m
 */

#ifndef INC_READPOWERGPIO_H_
#define INC_READPOWERGPIO_H_

#define read_nMPS()               HAL_GPIO_ReadPin(nMPS_GPIO_Port, nMPS_Pin)               // PC4
#define read_ESD()                HAL_GPIO_ReadPin(ESD_GPIO_Port, ESD_Pin)                 // PC5
#define read_EN1()                HAL_GPIO_ReadPin(EN1_GPIO_Port, EN1_Pin)                 // PE1
#define read_nDCDC_Fault()        HAL_GPIO_ReadPin(nDCDC_Fault_GPIO_Port, nDCDC_Fault_Pin) // PC13
#define read_n3A_OC()             HAL_GPIO_ReadPin(n3A_OC_GPIO_Port, n3A_OC_Pin)
#define read_nDCDC_On()           HAL_GPIO_ReadPin(nDCDC_On_GPIO_Port, nDCDC_On_Pin)
#define read_nCHG_Fault()         HAL_GPIO_ReadPin(nCHG_Fault_GPIO_Port, nCHG_Fault_Pin)
#define read_nCHG_On()            HAL_GPIO_ReadPin(nCHG_On_GPIO_Port, nCHG_On_Pin)
#define read_nCHG_LV_En()         HAL_GPIO_ReadPin(nCHG_LV_En_GPIO_Port, nCHG_LV_En_Pin)
#define read_ABATT_Disable()      HAL_GPIO_ReadPin(ABATT_Disable_GPIO_Port, ABATT_Disable_Pin)
#define read_Key()                HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin)                 // PB1
#define read_Charge_Enable()      HAL_GPIO_ReadPin(CHARGE_ENABLE_SENSE_GPIO_Port, CHARGE_ENABLE_SENSE_Pin)
#define read_Discharge_Enable()   HAL_GPIO_ReadPin(DISCHARGE_ENABLE_SENSE_GPIO_Port, DISCHARGE_ENABLE_SENSE_Pin)
#define read_LV_OC()              HAL_GPIO_ReadPin(LV_OC_GPIO_Port, LV_OC_Pin)
#define read_CHARGE_PLUGGED() 	  HAL_GPIO_ReadPin(CHARGE_PLUGGED_GPIO_Port, CHARGE_PLUGGED_Pin)



//GPIO_PinState read_nMPS(void);
//
//GPIO_PinState read_ESD(void);
//
//GPIO_PinState read_EN1(void);
//
//GPIO_PinState read_nDCDC_Fault(void);
//
//GPIO_PinState read_n3A_OC(void);
//
//GPIO_PinState read_nDCDC_On(void);
//
//GPIO_PinState read_nCHG_Fault(void);
//
//GPIO_PinState read_nCHG_On(void);
//
//GPIO_PinState read_nCHG_LV_En(void);
//
//GPIO_PinState read_ABATT_Disable(void);
//
//GPIO_PinState read_Key(void);
//
//GPIO_PinState read_Charge_Enable(void);
//
//GPIO_PinState read_Discharge_Enable(void);
//
//GPIO_PinState read_LV_OC(void);

#endif /* INC_READPOWERGPIO_H_ */
