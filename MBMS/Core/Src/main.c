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
#include "StartupTask.hpp"
#include "ShutoffTask.hpp"
#include "BatteryControlTask.hpp"
#include "CANRxGatekeeperTask.hpp"
#include "CANTxGatekeeperTask.hpp"
#include "DebugInterfaceTask.hpp"
#include "DisplayTask.hpp"
#include "CANdefines.h"
#include "CAN.h"

#include "stm32f4xx_hal.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CONTACTOR_QUEUE_COUNT 2 // Anticipating 2 messages being received. 1) the contactor number 2) the desired action (closed/open)

#define SHUTOFF_FLAG 0b00000001U // just making the flag an arbitrary number (should be uint32_t,,, this is = 1 in decimal)
// what was the cause of the shutdown??
#define EPCOS_FLAG 0b00000010U // external power cut off switch (push button outside car), starts soft shutdown
#define MPS_FLAG 0b00000100U // main power switch is the cause of shutoff
#define KEY_FLAG 0b00001000U // turning car key is cause of shutoff
#define HARD_BL_FLAG 0b00010000U // hard battery limit is cause of shutoff
#define SOFT_BL_FLAG 0b00100000U // soft battery limit is cause of shutoff


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart3;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */

// mutex for CAN transmit and receive
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

// flag that BatteryControlTask will set, ShutdownTask will wait for
osEventFlagsId_t shutoffFlagHandle;
const osEventFlagsAttr_t shutoffFlag_attributes = {
		.name = "shutoffFlag"
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
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
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


// IS THIS THE CORRECT PLACE TO PUT IT??
osMessageQueueId_t msgQueueID;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART3_UART_Init(void);
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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
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
  TxCANMessageQueueHandle = osMessageQueueNew(5, sizeof(CANMsg), &TxCANMessageQueue_attributes);
  RxCANMessageQueueHandle = osMessageQueueNew(5, sizeof(CANMsg), &RxCANMessageQueue_attributes);
  batteryControlMessageQueueHandle = osMessageQueueNew(5, sizeof(CANMsg), &batteryControlMessageQueue_attributes);

  // IS THIS CORRECT??? THE NUMBER IN QUEUE AND SIZE
//  msgConactorQueueID = osMessageQueueNew(15, sizeof(uint16_t), NULL);
//
//  prechargerThreadID = osThreadNew(prechargerThread, NULL, &prechargerThread_attr);
//
//  gatekeeperThreadID = osThreadNew(gatekeeperThread, NULL, &gatekeeperThread_attr);

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  CANTxGatekeeperTaskHandle = osThreadNew(CANTxGatekeeperTask, NULL, &CANTxGatekeeperTask_attributes);
  CANRxGatekeeperTaskHandle = osThreadNew(CANRxGatekeeperTask, NULL, &CANRxGatekeeperTask_attributes);

  batteryControlTaskHandle = osThreadNew(BatteryControlTask, NULL, &batteryControlTask_attributes);

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  // IS THIS WHERE I SHOULD CREATE AN EVENT FLAG ????
  shutoffFlagHandle = osEventFlagsNew(&shutoffFlag_attributes);
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

  //put filter here
  CAN_FilterTypeDef filter;

  filter.FilterBank = 0;  // filter bank 0
  filter.FilterMode = CAN_FILTERMODE_IDLIST;  // ID list mode,,, make it match this exact ID
  filter.FilterScale = CAN_FILTERSCALE_32BIT;
  filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;

  filter.FilterActivation = CAN_FILTER_ENABLE;

  // filtering IDs from orion

  filter.FilterIdHigh = (0x302 << 5) & 0xFFFF; // shifting by 5 bits under the assumption that its standard 11 bit ID, would be diff if using extended ID!!!
  filter.FilterIdLow = 0x0000;  //
  if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK) {
      // handle error!
  }


  filter.FilterIdHigh = (0x304 << 5) & 0xFFFF;
  if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK) {
      // handle error!
  }


  filter.FilterIdHigh = (0x30A << 5) & 0xFFFF;
  if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK) {
      // handle error!
  }

  // filtering IDs from the individual contactor boards

  filter.FilterIdHigh = (0x700 << 5) & 0xFFFF;
  if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK) {
        // handle error!
  }

  filter.FilterIdHigh = (0x701 << 5) & 0xFFFF;
  if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK) {
	  // handle error!
  }

  filter.FilterIdHigh = (0x702 << 5) & 0xFFFF;
  if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK) {
	  // handle error!
  }

  filter.FilterIdHigh = (0x703 << 5) & 0xFFFF;
  if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK) {
	  // handle error!
  }

  filter.FilterIdHigh = (0x704 << 5) & 0xFFFF;
  if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK) {
  	  // handle error!
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
  HAL_GPIO_WritePin(DCDC1_EN_GPIO_Port, DCDC1_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ABATT_DISABLE_Pin|GPIO_PIN_2|_12V_PCHG_EN_Pin|_12V_CAN_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, G1_Pin|A2_Pin|G2_Pin|A3_Pin
                          |G3_Pin|A4_Pin|G4_Pin|A5_Pin
                          |G5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : nDCDC0_ON_Pin _3A_OC_UC_Pin nDCDC1_ON_Pin nCHG_FAULT_Pin
                           nCHG_ON_Pin DCDC0_OV_FAULT_Pin DCDC0_UV_FAULT_Pin */
  GPIO_InitStruct.Pin = nDCDC0_ON_Pin|_3A_OC_UC_Pin|nDCDC1_ON_Pin|nCHG_FAULT_Pin
                          |nCHG_ON_Pin|DCDC0_OV_FAULT_Pin|DCDC0_UV_FAULT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC13 MAIN_PWR_SW_Pin CHARGE_SAFETY_SENSE_Pin DISCHARGE_ENABLE_SENSE_Pin
                           CHARGE_ENABLE_SENSE_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_13|MAIN_PWR_SW_Pin|CHARGE_SAFETY_SENSE_Pin|DISCHARGE_ENABLE_SENSE_Pin
                          |CHARGE_ENABLE_SENSE_Pin;
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

  /*Configure GPIO pin : DCDC1_EN_Pin */
  GPIO_InitStruct.Pin = DCDC1_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DCDC1_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ABATT_DISABLE_Pin PB2 _12V_PCHG_EN_Pin _12V_CAN_EN_Pin */
  GPIO_InitStruct.Pin = ABATT_DISABLE_Pin|GPIO_PIN_2|_12V_PCHG_EN_Pin|_12V_CAN_EN_Pin;
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
                           G5_Pin */
  GPIO_InitStruct.Pin = G1_Pin|A2_Pin|G2_Pin|A3_Pin
                          |G3_Pin|A4_Pin|G4_Pin|A5_Pin
                          |G5_Pin;
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
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
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
