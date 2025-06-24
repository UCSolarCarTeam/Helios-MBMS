/*
 * StartupTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millaineli
 */
#include "../Inc/StartupTask.h"
#include <stdint.h>
#include "main.h"
#include "CANdefines.h"
#include "BatteryControlTask.h"
#include "ReadPowerGPIO.h"
#include "MBMS.h"

//extern ContactorState contactorState;
extern ContactorInfo contactorInfo[6];
extern MBMSStatus mbmsStatus;
extern uint32_t BCT_Counter;
extern uint32_t startup_Check_Counter;

// all structs important for more than one file, contactor

// wait times in seconds i think ..
#define MPS_WAIT_TIME 1
#define DCDC0_WAIT_TIME 10
#define DCDC1_WAIT_TIME 10
#define MOTOR_WAIT_TIME 10
#define ARRAY_WAIT_TIME 10
#define CHARGE_WAIT_TIME 10

extern MBMSTrip mbmsTrip;
extern ContactorCommand contactorCommand;
extern Permissions perms;
extern uint8_t carState;

void StartupTask(void* arg)
{
    while(1)
    {
    	Startup();
    }
}

void Startup()
{

	//aux battery has started up and is powering the MBMS

	perms_init();
	MBMSStatus_init();

	HAL_GPIO_WritePin(GRN_LED_GPIO_Port, GRN_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(BLU_LED_GPIO_Port, BLU_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);



	mbmsStatus.startupState = nMPS_ENABLED;

#if 0
	uint8_t nMPS = read_nMPS();
	while (read_nMPS() == nMPS_ACTIVE) {
		uint8_t nMPS = read_nMPS();
		osDelay(200);
		// SET TRIP HERE
		// wait for MPS to be on/enabled
	}
#endif
	uint8_t nMPS2 = read_nMPS2();
	while (read_nMPS2() == nMPS_ACTIVE) {
		uint8_t nMPS2 = read_nMPS2();
		osDelay(200);
		// SET TRIP HERE
		// wait for MPS to be on/enabled
	}

	mbmsStatus.startupState = nMPS_DISABLED;



	while (read_ESD() == ESD_ACTIVE) {
		// do BPS fault
		// instead should i just have an osDelay, for BCT to run and set the trip/fault..? bc we have to keep track of that in the struct

		// SET TRIP HERE naw i dont like this im just gonna add a delay and BCT can figure it out tbh
		//osEventFlagsSet(shutoffFlagHandle, HARD_BL_FLAG);
		uint8_t ESD = read_ESD();
		osDelay(200);
		// um idk lol lets hope it will be running BCT
		if (carState == BPS_FAULT) {
			osThreadTerminate(startupTaskHandle);
		}

	}

	mbmsStatus.startupState = ESD_DISABLED;



	while(startup_Check_Counter < 5) {
		osDelay(50);
	}

	mbmsStatus.startupState = CHECKS_PASSED;

	// dont worry, discharge and charge enable are checked for their respective contactors in BCT
	perms.common = 1;
	// commented out for testing only
#if 0
	while ((contactorInfo[COMMON].contactorClosed != CLOSE_CONTACTOR)) {

	}
#endif
	if (contactorInfo[COMMON].contactorError) {
		// TO DO: handle error
		//Error_Handler();
	}
	mbmsStatus.startupState = COMMON_CLOSED;

	// set flag to give permission to precharge/close LV
	perms.lv = 1;

	// commented out for testing only
#if 0
	while ((contactorInfo[LOWV].contactorClosed != CLOSE_CONTACTOR)) {
		// wait for LV contactor to close
	}
#endif

	if (contactorInfo[LOWV].contactorError) {
		// TO DO: handle error
		//Error_Handler();
	}
	mbmsStatus.startupState = LV_CLOSED;
/* UNCOMMENT WHEN DONE DEBUG
	// enable DCDC HV through EN1
	HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, EN1_ACTIVE);

	mbmsStatus.startupState = EN1_ON;

	//precharge 12V CAN
	HAL_GPIO_WritePin(_12V_PCHG_En_GPIO_Port, _12V_PCHG_En_Pin, _12V_PCHG_EN_ACTIVE);
	//wait to finish precharging!
	while(read_Critical_OV_UV() != CRITICAL_OV_UV_ACTIVE) { // 0 is good to go

	}
	//Enable 12V CAN
	HAL_GPIO_WritePin(_12V_CAN_En_GPIO_Port, _12V_CAN_En_Pin, _12V_CAN_EN_ACTIVE);
	HAL_GPIO_WritePin(_12V_PCHG_En_GPIO_Port, _12V_PCHG_En_Pin, !(_12V_PCHG_EN_ACTIVE));

*/

	// set flag to give permission to precharge/close motor contactor
	// just check that everything is good still (doesnt HAVE to close motor before moving on to next part)
	perms.motor = 1;

	mbmsStatus.startupState = MOTORS_PERMS;

	// set flag to give permission to precharge/close array contactor
	// wait until array contactor done (same as above, make sure everything okay still, doesnt NEED it to bed closed...)
	perms.array = 1;

	mbmsStatus.startupState = ARRAY_PERMS;

	// MAYBE MAKE SURE BCT HAS RUN A COUPLE TIMES FIRST BEFORE SAYING COMPLETED >>> idk

	// set flag that everything is done (all perms given!!!)
	mbmsStatus.startupState = COMPLETED;

	// end of startup
	osThreadTerminate(startupTaskHandle);

}







