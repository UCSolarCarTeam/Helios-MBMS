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

#define FREERTOS_TICK_PERIOD 1/configTICK_RATE_HZ //USE THIS INSTEAD OF SECONFS PER TICK
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


	mbmsStatus.startupState = nMPS_ENABLED;

	while (read_nMPS() == 1) {

		// SET TRIP HERE
		// wait for MPS to be on/enabled
	}

	mbmsStatus.startupState = nMPS_DISABLED;



	while (read_ESD() == 1) {
		// do BPS fault
		// instead should i just have an osDelay, for BCT to run and set the trip/fault..? bc we have to keep track of that in the struct

		// SET TRIP HERE
		osEventFlagsSet(shutoffFlagHandle, HARD_BL_FLAG);
		osThreadTerminate(startupTaskHandle);
	}

	mbmsStatus.startupState = ESD_DISABLED;


	while(BCT_Counter < 1) {
		osDelay(1000);
	}

	mbmsStatus.startupState = CHECKS_PASSED;

	// dont worry, discharge and charge enable are checked for their respective contactors in BCT
	perms.common = 1;
	while ((contactorInfo[COMMON].contactorClosed != CLOSE_CONTACTOR)) {

	}
	if (contactorInfo[COMMON].contactorError) {
		// TO DO: handle error
		Error_Handler();
	}
	mbmsStatus.startupState = COMMON_CLOSED;

	// set flag to give permission to precharge/close LV
	perms.lv = 1;
	while ((contactorInfo[LOWV].contactorClosed != CLOSE_CONTACTOR)) {
		// wait for LV contactor to close
	}
	if (contactorInfo[LOWV].contactorError) {
		// TO DO: handle error
		Error_Handler();
	}
	mbmsStatus.startupState = LV_CLOSED;

	// enable DCDC HV through EN1
	HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_SET);

	mbmsStatus.startupState = EN1_ON;

	//precharge 12V CAN
	HAL_GPIO_WritePin(_12V_PCHG_En_GPIO_Port, _12V_PCHG_En_Pin, GPIO_PIN_SET);
	//wait to finish precharging!
	while(read_LV_OC() == 1) { // 0 is good to go

	}
	//Enable 12V CAN
	HAL_GPIO_WritePin(_12V_CAN_En_GPIO_Port, _12V_CAN_En_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(_12V_PCHG_En_GPIO_Port, _12V_PCHG_En_Pin, GPIO_PIN_RESET);



	// set flag to give permission to precharge/close motor contactor
	// just check that everything is good still (doesnt HAVE to close motor before moving on to next part)
	perms.motor = 1;

	mbmsStatus.startupState = MOTORS_PERMS;

	//add a delay for 10 seconds
	// to give time for batt control to check things r ok, close contactors or not, decide if there needs to be a trip or not etc.
	osDelay(MOTOR_WAIT_TIME * 1000);


	// set flag to give permission to precharge/close array contactor
	// wait until array contactor done (same as above, make sure everything okay still, doesnt NEED it to bed closed...)
	perms.array = 1;

	mbmsStatus.startupState = ARRAY_PERMS;
	// give time to battery control task to make sure battery state is still safe
	osDelay(ARRAY_WAIT_TIME * 1000);
	// if battery is fully charged, don't do the array!!!!
	// motor charge and array, just give permissions, dont need to wait for contactors
	// because if battery is fully discharged, then motors shouldnt go
	// if car is fully charged, dont need array


	// MAYBE MAKE SURE BCT HAS RUN A COUPLE TIMES FIRST BEFORE SAYING COMPLETED >>> idk

	// set flag that everything is done (all perms given!!!)
	mbmsStatus.startupState = COMPLETED;

	// end of startup
	osThreadTerminate(startupTaskHandle);

}







