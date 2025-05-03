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
#include "cmsis_os.h"
#include <string.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern CAN_HandleTypeDef hcan1;
extern osMessageQueueId_t TxCANMessageQueueHandle;
extern osMessageQueueId_t RxCANMessageQueueHandle;
extern osMessageQueueId_t batteryControlMessageQueueHandle;
extern osMessageQueueId_t contactorMessageQueueHandle;
extern osThreadId_t batteryControlTaskHandle;
extern osEventFlagsId_t shutoffFlagHandle;
extern osEventFlagsId_t contactorPermissionsFlagHandle;
extern osThreadId_t startupTaskHandle;
extern osThreadId_t CANMessageSenderTaskHandle;

extern osThreadId_t debugInterfaceTaskHandle;

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
#define NC_Pin GPIO_PIN_2
#define NC_GPIO_Port GPIOE
#define n3A_OC_UC_Pin GPIO_PIN_3
#define n3A_OC_UC_GPIO_Port GPIOE
#define nDCDC_ON_Pin GPIO_PIN_4
#define nDCDC_ON_GPIO_Port GPIOE
#define nCHG_FAULT_Pin GPIO_PIN_5
#define nCHG_FAULT_GPIO_Port GPIOE
#define nCHG_ON_Pin GPIO_PIN_6
#define nCHG_ON_GPIO_Port GPIOE
#define nDCDC_Fault_Pin GPIO_PIN_13
#define nDCDC_Fault_GPIO_Port GPIOC
#define BLU_LED_Pin GPIO_PIN_0
#define BLU_LED_GPIO_Port GPIOA
#define GRN_LED_Pin GPIO_PIN_1
#define GRN_LED_GPIO_Port GPIOA
#define RED_LED_Pin GPIO_PIN_2
#define RED_LED_GPIO_Port GPIOA
#define STROBE_EN_Pin GPIO_PIN_3
#define STROBE_EN_GPIO_Port GPIOA
#define nMPS_Pin GPIO_PIN_4
#define nMPS_GPIO_Port GPIOC
#define ESD_Pin GPIO_PIN_5
#define ESD_GPIO_Port GPIOC
#define ABATT_DISABLE_Pin GPIO_PIN_0
#define ABATT_DISABLE_GPIO_Port GPIOB
#define Key_Pin GPIO_PIN_1
#define Key_GPIO_Port GPIOB
#define A1_Pin GPIO_PIN_2
#define A1_GPIO_Port GPIOB
#define G1_Pin GPIO_PIN_7
#define G1_GPIO_Port GPIOE
#define A2_Pin GPIO_PIN_8
#define A2_GPIO_Port GPIOE
#define G2_Pin GPIO_PIN_9
#define G2_GPIO_Port GPIOE
#define A3_Pin GPIO_PIN_10
#define A3_GPIO_Port GPIOE
#define G3_Pin GPIO_PIN_11
#define G3_GPIO_Port GPIOE
#define A4_Pin GPIO_PIN_12
#define A4_GPIO_Port GPIOE
#define G4_Pin GPIO_PIN_13
#define G4_GPIO_Port GPIOE
#define A5_Pin GPIO_PIN_14
#define A5_GPIO_Port GPIOE
#define G5_Pin GPIO_PIN_15
#define G5_GPIO_Port GPIOE
#define CHARGE_SAFETY_SENSE_Pin GPIO_PIN_6
#define CHARGE_SAFETY_SENSE_GPIO_Port GPIOC
#define DISCHARGE_ENABLE_SENSE_Pin GPIO_PIN_7
#define DISCHARGE_ENABLE_SENSE_GPIO_Port GPIOC
#define CHARGE_ENABLE_SENSE_Pin GPIO_PIN_8
#define CHARGE_ENABLE_SENSE_GPIO_Port GPIOC
#define CAN1_MODE_Pin GPIO_PIN_8
#define CAN1_MODE_GPIO_Port GPIOA
#define LV_OC_Pin GPIO_PIN_1
#define LV_OC_GPIO_Port GPIOD
#define _12V_PCHG_EN_Pin GPIO_PIN_7
#define _12V_PCHG_EN_GPIO_Port GPIOB
#define _12V_CAN_EN_Pin GPIO_PIN_8
#define _12V_CAN_EN_GPIO_Port GPIOB
#define CRITICAL_OV_UV_Pin GPIO_PIN_9
#define CRITICAL_OV_UV_GPIO_Port GPIOB
#define nCHG_En_Pin GPIO_PIN_0
#define nCHG_En_GPIO_Port GPIOE
#define EN1_Pin GPIO_PIN_1
#define EN1_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
