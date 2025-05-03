/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "GatekeeperTask.hpp"

#include <BatteryControlTask.h>
#include <CANRxGatekeeperTask.h>
#include <CANTxGatekeeperTask.h>
#include <CANMessageSenderTask.h>
#include <DebugInterfaceTask.h>
#include <DisplayTask.h>
#include <stdint.h>

#include "StartupTask.h"
#include "ShutoffTask.h"
#include "CANdefines.h"


#include "stm32f4xx_hal.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define QUEUE_SIZE 10


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart3;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */

// mutex for CAN transmit and receive CAN I DELETE THIS??????
osMutexId_t CANSPIMutexHandle;
const osMutexAttr_t CANSPIMutex_attributes = {
		.name = "CANSPIMutex",
		.attr_bits = osMutexPrioInherit,
};
// change name of this to not SPI... this is queue im currently using for sending CAN messages
osMessageQueueId_t TxCANMessageQueueHandle;
const osMessageQueueAttr_t TxCANMessageQueue_attributes = {
		.name = "TxCANMessageQueue",
		.attr_bits = 0, // idk i just set it to zero for now but idk help
};

osMessageQueueId_t RxCANMessageQueueHandle;
const osMessageQueueAttr_t RxCANMessageQueue_attributes = {
		.name = "RxCANMessageQueue",
		.attr_bits = 0, // idk i just set it to zero for now but idk help
};

osMessageQueueId_t batteryControlMessageQueueHandle;
const osMessageQueueAttr_t batteryControlMessageQueue_attributes = {
		.name = "batteryControlMessageQueue",
		.attr_bits = 0, // idk i just set it to zero for now but idk help
};

osMessageQueueId_t contactorMessageQueueHandle;
const osMessageQueueAttr_t contactorMessageQueue_attributes = {
		.name = "contactorMessageQueue",
		.attr_bits = 0, // idk i just set it to zero for now but idk help
};

// flag that BatteryControlTask will set, ShutdownTask will wait for
osEventFlagsId_t shutoffFlagHandle;
const osEventFlagsAttr_t shutoffFlag_attributes = {
		.name = "shutoffFlag"
		// default cb_mem,
		// default cb_size ?
};

// flag that startup sets
osEventFlagsId_t contactorPermissionsFlagHandle;
const osEventFlagsAttr_t contactorPermissionsFlag_attributes = {
		.name = "contactorPermissionsFlag"
		// default cb_mem,
		// default cb_size ?
};

osThreadId_t batteryControlTaskHandle;
const osThreadAttr_t batteryControlTask_attributes = {
  .name = "batteryControlTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime5,
};

osThreadId_t CANRxGatekeeperTaskHandle;
const osThreadAttr_t CANRxGatekeeperTask_attributes = {
  .name = "CANRxGatekeeperTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime6,
};

osThreadId_t CANTxGatekeeperTaskHandle;
const osThreadAttr_t CANTxGatekeeperTask_attributes = {
  .name = "CANTxGatekeeperTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime4,
};

osThreadId_t debugInterfaceTaskHandle;
const osThreadAttr_t debugInterfaceTask_attributes = {
  .name = "debugInterfaceTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh2, //Khadeeja originally put osPriorityBelowNormal
};

osThreadId_t displayTaskHandle;
const osThreadAttr_t displayTask_attributes = {
  .name = "displayTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t shutoffTaskHandle;
const osThreadAttr_t shutoffTask_attributes = {
  .name = "shutoffTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime7,
};

osThreadId_t startupTaskHandle;
const osThreadAttr_t startupTask_attributes = {
  .name = "startupTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh2,
};

osThreadId_t CANMessageSenderTaskHandle;
const osThreadAttr_t CANMessageSenderTask_attributes = {
  .name = "CANMessageSenderTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh2, // idk what priority to put ngl
};



//// IS THIS THE CORRECT PLACE TO PUT IT??
//osMessageQueueId_t msgQueueID;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_UART4_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN1_Init();
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
#if 1
  /* add mutexes, ... */
  CANSPIMutexHandle = osMutexNew(&CANSPIMutex_attributes);
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  TxCANMessageQueueHandle = osMessageQueueNew(QUEUE_SIZE, sizeof(CANMsg), &TxCANMessageQueue_attributes);
  if(TxCANMessageQueueHandle == NULL){
	  uint8_t x = 0;
	  //
  }
  RxCANMessageQueueHandle = osMessageQueueNew(QUEUE_SIZE, sizeof(CANMsg), &RxCANMessageQueue_attributes);
  batteryControlMessageQueueHandle = osMessageQueueNew(QUEUE_SIZE, sizeof(CANMsg), &batteryControlMessageQueue_attributes);
  if(batteryControlMessageQueueHandle == NULL){
	  uint8_t x = 0;
	  //
  }
  contactorMessageQueueHandle = osMessageQueueNew(QUEUE_SIZE, sizeof(CANMsg), &contactorMessageQueue_attributes);
  // IS THIS CORRECT??? THE NUMBER IN QUEUE AND SIZE

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

//  CANTxGatekeeperTaskHandle = osThreadNew(CANTxGatekeeperTask, NULL, &CANTxGatekeeperTask_attributes);
//  CANRxGatekeeperTaskHandle = osThreadNew(CANRxGatekeeperTask, NULL, &CANRxGatekeeperTask_attributes);

  batteryControlTaskHandle = osThreadNew(BatteryControlTask, NULL, &batteryControlTask_attributes);
  if(batteryControlTaskHandle == NULL)
  {
	  uint8_t x = 0;
  }
//  shutoffTaskHandle = osThreadNew(ShutoffTask, NULL, &shutoffTask_attributes);
//  startupTaskHandle = osThreadNew(StartupTask, NULL, &startupTask_attributes);

  debugInterfaceTaskHandle = osThreadNew(DebugInterfaceTask, NULL, &debugInterfaceTask_attributes);
  if(debugInterfaceTaskHandle == NULL) {
	  uint8_t x = 0;
  }

//  CANMessageSenderTaskHandle = osThreadNew(CANMessageSenderTask, NULL, &CANMessageSenderTask_attributes);



  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  // IS THIS WHERE I SHOULD CREATE AN EVENT FLAG ????
//  shutoffFlagHandle = osEventFlagsNew(&shutoffFlag_attributes);

  contactorPermissionsFlagHandle = osEventFlagsNew(&contactorPermissionsFlag_attributes);


#endif
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();
  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  HAL_GPIO_TogglePin(G1_GPIO_Port, G1_Pin); // pin PE7
	  HAL_GPIO_TogglePin(A3_GPIO_Port, A3_Pin); // pin PE7
	  // Delay in milliseconds
	  HAL_Delay(500);

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 4;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_15TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_5TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  //put filters here

  // filtering IDs from orion

  CAN_FilterTypeDef packInfoFilter;

  packInfoFilter.FilterBank = 0;  // filter bank 0
  packInfoFilter.FilterMode = CAN_FILTERMODE_IDLIST;  // ID list mode,,, make it match this exact ID
  packInfoFilter.FilterScale = CAN_FILTERSCALE_32BIT;
  packInfoFilter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  packInfoFilter.FilterActivation = CAN_FILTER_ENABLE;

  packInfoFilter.FilterIdHigh = PACKINFOID >> 13; //
  packInfoFilter.FilterIdLow = (PACKINFOID & 0x1fff) << 3;  // shift left 3 bits because last 13 bits of EXID in low reg, and zero out last 3 bits of low reg (RTR, IDE, 0)
  if (HAL_CAN_ConfigFilter(&hcan1, &packInfoFilter) != HAL_OK) {
      // handle error!
  }

  CAN_FilterTypeDef tempInfoFilter;

    tempInfoFilter.FilterBank = 1;  // filter bank 1
    tempInfoFilter.FilterMode = CAN_FILTERMODE_IDLIST;  // ID list mode,,, make it match this exact ID
    tempInfoFilter.FilterScale = CAN_FILTERSCALE_32BIT;
    tempInfoFilter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    tempInfoFilter.FilterActivation = CAN_FILTER_ENABLE;

  tempInfoFilter.FilterIdHigh = (TEMPINFOID >> 13); // would be zero when u shift it 13 bits left lol
  tempInfoFilter.FilterIdLow = (TEMPINFOID & 0x1fff) << 3;
  if (HAL_CAN_ConfigFilter(&hcan1, &tempInfoFilter) != HAL_OK) {
      Error_Handler();
  }

  CAN_FilterTypeDef maxMinVoltagesFilter;

	maxMinVoltagesFilter.FilterBank = 2;  // filter bank 2
	maxMinVoltagesFilter.FilterMode = CAN_FILTERMODE_IDLIST;  // ID list mode,,, make it match this exact ID
	maxMinVoltagesFilter.FilterScale = CAN_FILTERSCALE_32BIT;
	maxMinVoltagesFilter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	maxMinVoltagesFilter.FilterActivation = CAN_FILTER_ENABLE;


	maxMinVoltagesFilter.FilterIdHigh = MAXMINVOLTAGESID >> 13;
	maxMinVoltagesFilter.FilterIdLow = (MAXMINVOLTAGESID & 0x1fff) << 3;

	if (HAL_CAN_ConfigFilter(&hcan1, &maxMinVoltagesFilter) != HAL_OK) {
	Error_Handler();
	}

  // filtering IDs from the individual contactor boards

  CAN_FilterTypeDef contactorFilter;
  contactorFilter.FilterBank = 3;  // filter bank 1
  contactorFilter.FilterMode = CAN_FILTERMODE_IDMASK;  // mask mode !!! can accept range of IDs
  contactorFilter.FilterScale = CAN_FILTERSCALE_32BIT;
  contactorFilter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  // 0 is dont care, 1 is compare them !!!!
  //uint32_t mask = 0x1fffffe0; // want it to let thru EXIDs 0x0000020X = 0x20X to 0x21X :)
  contactorFilter.FilterMaskIdHigh = CONTACTORMASK >> 13;
  contactorFilter.FilterMaskIdLow = (CONTACTORMASK & 0x1fff) << 3;

  contactorFilter.FilterActivation = CAN_FILTER_ENABLE;


  contactorFilter.FilterIdHigh = CONTACTORIDS >> 13; // shift right by 13 bits to get rid of
  contactorFilter.FilterIdLow = (CONTACTORIDS & 0x1fff) << 3; // zero out the first 16-bits, so only rightmost 13 bits left, shift left by 3 to make room for IDE, RTR, 0
  if (HAL_CAN_ConfigFilter(&hcan1, &contactorFilter) != HAL_OK) {
	  Error_Handler();
  }

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, BLU_LED_Pin|GRN_LED_Pin|RED_LED_Pin|STROBE_EN_Pin
                          |CAN1_MODE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ABATT_DISABLE_Pin|A1_Pin|_12V_PCHG_EN_Pin|_12V_CAN_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, G1_Pin|A2_Pin|G2_Pin|A3_Pin
                          |G3_Pin|A4_Pin|G4_Pin|A5_Pin
                          |G5_Pin|nCHG_En_Pin|EN1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : NC_Pin n3A_OC_UC_Pin nDCDC_ON_Pin nCHG_FAULT_Pin
                           nCHG_ON_Pin */
  GPIO_InitStruct.Pin = NC_Pin|n3A_OC_UC_Pin|nDCDC_ON_Pin|nCHG_FAULT_Pin
                          |nCHG_ON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : nDCDC_Fault_Pin nMPS_Pin ESD_Pin CHARGE_SAFETY_SENSE_Pin
                           DISCHARGE_ENABLE_SENSE_Pin CHARGE_ENABLE_SENSE_Pin */
  GPIO_InitStruct.Pin = nDCDC_Fault_Pin|nMPS_Pin|ESD_Pin|CHARGE_SAFETY_SENSE_Pin
                          |DISCHARGE_ENABLE_SENSE_Pin|CHARGE_ENABLE_SENSE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : BLU_LED_Pin GRN_LED_Pin RED_LED_Pin STROBE_EN_Pin
                           CAN1_MODE_Pin */
  GPIO_InitStruct.Pin = BLU_LED_Pin|GRN_LED_Pin|RED_LED_Pin|STROBE_EN_Pin
                          |CAN1_MODE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : ABATT_DISABLE_Pin A1_Pin _12V_PCHG_EN_Pin _12V_CAN_EN_Pin */
  GPIO_InitStruct.Pin = ABATT_DISABLE_Pin|A1_Pin|_12V_PCHG_EN_Pin|_12V_CAN_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Key_Pin CRITICAL_OV_UV_Pin */
  GPIO_InitStruct.Pin = Key_Pin|CRITICAL_OV_UV_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : G1_Pin A2_Pin G2_Pin A3_Pin
                           G3_Pin A4_Pin G4_Pin A5_Pin
                           G5_Pin nCHG_En_Pin EN1_Pin */
  GPIO_InitStruct.Pin = G1_Pin|A2_Pin|G2_Pin|A3_Pin
                          |G3_Pin|A4_Pin|G4_Pin|A5_Pin
                          |G5_Pin|nCHG_En_Pin|EN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : LV_OC_Pin */
  GPIO_InitStruct.Pin = LV_OC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LV_OC_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
	  HAL_GPIO_TogglePin(G1_GPIO_Port, G1_Pin); // pin PE7
	  HAL_GPIO_TogglePin(A3_GPIO_Port, A3_Pin); // pin PE7
	  // Delay in milliseconds
//	  HAL_Delay(100);
	  osDelay(1000);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
