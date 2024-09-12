/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Array_Current_Sensor_1_Pin GPIO_PIN_0
#define Array_Current_Sensor_1_GPIO_Port GPIOC
#define Array_Current_Sensor_2_Pin GPIO_PIN_1
#define Array_Current_Sensor_2_GPIO_Port GPIOC
#define Array_Current_Sensor_3_Pin GPIO_PIN_2
#define Array_Current_Sensor_3_GPIO_Port GPIOC
#define Array_Current_Sensor_4_Pin GPIO_PIN_3
#define Array_Current_Sensor_4_GPIO_Port GPIOC
#define LV_Current_Pin GPIO_PIN_0
#define LV_Current_GPIO_Port GPIOA
#define LV_Charge_Pin GPIO_PIN_1
#define LV_Charge_GPIO_Port GPIOA
#define HV_Current_Pin GPIO_PIN_2
#define HV_Current_GPIO_Port GPIOA
#define HV_Charge_Pin GPIO_PIN_3
#define HV_Charge_GPIO_Port GPIOA
#define Charge_Current_Pin GPIO_PIN_4
#define Charge_Current_GPIO_Port GPIOA
#define Charge_Voltage_Pin GPIO_PIN_5
#define Charge_Voltage_GPIO_Port GPIOA
#define Array_Current_Pin GPIO_PIN_6
#define Array_Current_GPIO_Port GPIOA
#define Array_Voltage_Pin GPIO_PIN_7
#define Array_Voltage_GPIO_Port GPIOA
#define Common_Current_Sensor_Pin GPIO_PIN_0
#define Common_Current_Sensor_GPIO_Port GPIOB
#define nDCDC1_fault_Pin GPIO_PIN_15
#define nDCDC1_fault_GPIO_Port GPIOA
#define nDCDC1_ON_Pin GPIO_PIN_10
#define nDCDC1_ON_GPIO_Port GPIOC
#define ncharge_fault_Pin GPIO_PIN_11
#define ncharge_fault_GPIO_Port GPIOC
#define ncharge_ON_Pin GPIO_PIN_12
#define ncharge_ON_GPIO_Port GPIOC
#define nCurrent_Sense_Pin GPIO_PIN_0
#define nCurrent_Sense_GPIO_Port GPIOD
#define nCharger_12V_Line_Enable_Pin GPIO_PIN_1
#define nCharger_12V_Line_Enable_GPIO_Port GPIOD
#define nDCDC1_Enable_Pin GPIO_PIN_2
#define nDCDC1_Enable_GPIO_Port GPIOD
#define DCDC0_OV_Fault_Pin GPIO_PIN_3
#define DCDC0_OV_Fault_GPIO_Port GPIOD
#define DCDC0_UV_Fault_Pin GPIO_PIN_4
#define DCDC0_UV_Fault_GPIO_Port GPIOD
#define Debug_LEDs_Pin GPIO_PIN_5
#define Debug_LEDs_GPIO_Port GPIOD
#define Debug_LEDsD6_Pin GPIO_PIN_6
#define Debug_LEDsD6_GPIO_Port GPIOD
#define Contactor_LEDs_Pin GPIO_PIN_7
#define Contactor_LEDs_GPIO_Port GPIOD
#define Array_Contactor_Pin GPIO_PIN_3
#define Array_Contactor_GPIO_Port GPIOB
#define Charge_Contactor_Pin GPIO_PIN_4
#define Charge_Contactor_GPIO_Port GPIOB
#define Motor_Contactor_Pin GPIO_PIN_5
#define Motor_Contactor_GPIO_Port GPIOB
#define LV_Contactor_Pin GPIO_PIN_6
#define LV_Contactor_GPIO_Port GPIOB
#define Common_Contactor_Pin GPIO_PIN_7
#define Common_Contactor_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */