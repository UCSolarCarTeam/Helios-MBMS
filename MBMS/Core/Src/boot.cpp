/*
 * boot.cpp
 *
 *	Master BMS
 *
 *  Created on: Aug 21, 2024
 *      Author: khadeejaabbas
 *      System: Core
 *      Role: Boot Code
 */

//Includes
#include "boot.hpp"

	//------------------------------
	//System Initialization Function

//SystemInitialize
//System Initialize Function
QA_Result SystemInitialize(void) {


	  //------------------------------------
	  //Enable CPU Instruction & Data Caches
	  SCB_EnableICache();
	  SCB_EnableDCache();

		//----------------------
		//Enable ART Accelerator
	  	//Allows CPU to quickly access data in the chip (where our code is)
	  __HAL_FLASH_ART_ENABLE();

	  //---------------------
	  //Enable Flash Prefetch
	  __HAL_FLASH_PREFETCH_BUFFER_ENABLE();

	  //--------------------------
	  //Set NVIC Priority Grouping
	  //NVIC handles all the interrupts. We're using priority group 4 which allows us to have priority between 0 & 15. 0 being the highest priority and 15 being the lowest
	  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	  //------------
	  //Init SysTick
	  //This is related to the SysTick_Handler. We're making this a high priority
	  HAL_InitTick(TICK_INT_PRIORITY);

	  //------------------------------
	  //Configure LSE Drive Capability
	  //LSE means lower speed external crystal and is used for the real-time clock

	  //enabling backup:
	  //backup is a bit of memory and a bunch of registers in the backup domain of the chip. This continues running even if the reset button is hit or upload code to it. This runs with the real-time clock which will now keep running even if the power is cut.
	  HAL_PWR_EnableBkUpAccess();
	  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

	  //------------------------------------------------
	  //Configure Main Internal Regulator Output Voltage

	  //Power management clock:
	  __HAL_RCC_PWR_CLK_ENABLE();

	  //Voltage scaling:
	  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	    //------------
	    //Setup Clocks
	  	//To setup the clocks there are 3 sections:
	  	  // 1) turning on oscillators and setting them up. Also setting up PLL (Phase-locked loop) so we're getting the frequency we need from that.
	  	  // 2) Making sure bus clock and bus clock dividers are set up correctly
	  	  // 3) Roughly set up peripheral clocks. More individual clock stuff will be done in that specific driver
	  //------------------
	  //Enable Oscillators
	  //
	  //Configure High Speed External (HSE) oscillator to be used for primary system clocks, and setup primary PLL to provide a 216MHz clock
	  //Configure Low Speed External (LSE) oscillator to be used for clock source for Real Timer Clock (RTC) peripheral
	  //NOTE: HSE Oscillator on STM32F769I-DISCO/STM32F769I-DISC1 board is provided by a 25MHz oscillator (labeled X2 on the PCB)
	  //      LSE Oscillator on STM32F769I-DISCO/STM32F769I-DISC1 board is provided by a 32.768kHz oscillator (labeled X3 on the PCB)
	  RCC_OscInitTypeDef RCC_OscInit = {0};
	  RCC_OscInit.OscillatorType = RCC_OSCILLATORTYPE_HSE | //Define HSE (High Speed External) oscillator to be configured (used for main system clock)
	  		                         RCC_OSCILLATORTYPE_LSE;  //Define LSE (Low Speed External) oscillator to be configured (used for RTC - Real Time Clock)
	  RCC_OscInit.HSEState       = RCC_HSE_ON;              //Enable HSE
	  RCC_OscInit.LSEState       = RCC_LSE_ON;              //Enable LSE
	  RCC_OscInit.PLL.PLLState   = RCC_PLL_ON;              //Enable primary PLL. Takes 25MHz from HSE and bump it up 216 MHz so we can drive the system with that
	  RCC_OscInit.PLL.PLLSource  = RCC_PLLSOURCE_HSE;       //Set primary PLL source to HSE crystal

	    //Set primary PLL values required for 168Mhz system clock (set up for 216)
	  RCC_OscInit.PLL.PLLM       = 25;
	  RCC_OscInit.PLL.PLLN       = 432;
	  RCC_OscInit.PLL.PLLP       = RCC_PLLP_DIV2;
	  RCC_OscInit.PLL.PLLQ       = 4;

	  //return a fail value if it fails (if we can't set up the clock we don't want to be doing anything else)
	  if (HAL_RCC_OscConfig(&RCC_OscInit) != HAL_OK) {  //Initialize Oscillators using values in init structure
	  	return QA_Fail;
	  }

	  //-------------------
	  //Enable SysCfg Clock
	  //SysCfg = System Configuration Clock = So we can configure the system a bit more
	  __HAL_RCC_SYSCFG_CLK_ENABLE();

	  //---------------------
	  //Enable Overdrive Mode
	  //
	  //This is required to provide best system stability given that we are running at the maximum clock rate of 168MHz for the STM32F407VGT6J device
	  if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
	  	return QA_Fail;
	  }

	  //---------------------------
	  //Initialize CPU & Bus Clocks
	  RCC_ClkInitTypeDef RCC_ClkInit = {0};
	  RCC_ClkInit.ClockType      = RCC_CLOCKTYPE_HCLK |      //Define which clocks are to be configured (HCLK, SYSCLK, PCLK1, PCLK2)
	  		                         RCC_CLOCKTYPE_SYSCLK |
	                               RCC_CLOCKTYPE_PCLK1 |
																 RCC_CLOCKTYPE_PCLK2;

	  RCC_ClkInit.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;  //Define primary PLL clock as system clock source

	  RCC_ClkInit.AHBCLKDivider  = RCC_SYSCLK_DIV1;          //Set clock divider for host bus (AHB), DIV1 provides an AHB frequency of 216MHz

	  RCC_ClkInit.APB1CLKDivider = RCC_HCLK_DIV4;            //Set clock divider for peripheral bus 1 (APB1), DIV4 provides an APB1 frequency of 54MHz
	                                                         //NOTE: APB1 timer clocks are clock doubled, providing APB1 timers with 108MHz

	  RCC_ClkInit.APB2CLKDivider = RCC_HCLK_DIV2;            //Set clock divider for peripheral bus 2 (APB2), DIV2 provides an APB2 frequency of 108MHz
	                                                         //NOTE: APB2 timer clocks are clock doubled, providing APB2 timers with 216MHz clocks

	  if (HAL_RCC_ClockConfig(&RCC_ClkInit, FLASH_LATENCY_7) != HAL_OK) { //Initialize system clocks using required values, and setting Flash latency to 7 cycles
	  	return QA_Fail;
	  }

	  //----------------------------
	  //Initialize Peripheral Clocks
	  RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit = {0};
	  RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC |
	  		                                     RCC_PERIPHCLK_LTDC;

	    //Define settings for PLLSAI, as this PLL is also used as the clock source for the LTDC peripheral which we require to run the LCD screen
		RCC_PeriphClkInit.PLLSAI.PLLSAIN       = 384;
		RCC_PeriphClkInit.PLLSAI.PLLSAIP       = RCC_PLLSAIP_DIV8;
		RCC_PeriphClkInit.PLLSAI.PLLSAIR       = 7;
		RCC_PeriphClkInit.PLLSAI.PLLSAIQ       = 8;
		RCC_PeriphClkInit.PLLSAIDivQ           = 1;
		RCC_PeriphClkInit.PLLSAIDivR           = RCC_PLLSAIDIVR_2;

	  RCC_PeriphClkInit.RTCClockSelection    = RCC_RTCCLKSOURCE_LSE; //Set the LSE oscillator as the clock source for the Real Time Clock

	  if (HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit) != HAL_OK) { //Initialize the peripheral clocks used the required values
	  	return QA_Fail;
	  }

	  //------------------
	  //Enable GPIO Clocks
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	  __HAL_RCC_GPIOD_CLK_ENABLE();
	  __HAL_RCC_GPIOE_CLK_ENABLE();
	  __HAL_RCC_GPIOF_CLK_ENABLE();
	  __HAL_RCC_GPIOG_CLK_ENABLE();
	  __HAL_RCC_GPIOH_CLK_ENABLE();
	  __HAL_RCC_GPIOI_CLK_ENABLE();
	  __HAL_RCC_GPIOJ_CLK_ENABLE();
	  __HAL_RCC_GPIOK_CLK_ENABLE();

	  //-----------------
	  //Enable DMA Clocks
	  __HAL_RCC_DMA1_CLK_ENABLE();
	  __HAL_RCC_DMA2_CLK_ENABLE();

	  //Return
	  return QA_OK;

}

