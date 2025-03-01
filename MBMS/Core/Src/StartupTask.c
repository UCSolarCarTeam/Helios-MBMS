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

extern ContactorState contactorState;
extern ContactorInfo contactorInfo[6];
extern MBMSStatus mbmsStatus;

#define FREERTOS_TICK_PERIOD 1/configTICK_RATE_HZ //USE THIS INSTEAD OF SECONFS PER TICK
// MBMSSys.h
// all structs important for more than one file, contactor

// wait times in seconds !
#define MPS_WAIT_TIME 1
#define DCDC0_WAIT_TIME 10
#define DCDC1_WAIT_TIME 10
#define MOTOR_WAIT_TIME 10
#define ARRAY_WAIT_TIME 10

extern MBMSTrip mbmsTrip;

void StartupTask(void* arg)
{
    while(1)
    {
    	Startup();
    }
}

void Startup()
{
	uint32_t seconds_per_tick = 1/osKernelGetTickFreq();

	mbmsStatus.startupState = MPS_OPEN;
	//aux battery has started up and is powering the MBMS

	while ( readMainPowerSwitch() == MPS_ENABLED) {
		//if EPCOS/MPS is enabled, we need to trip
		// TO DO: trip
		//mbmsTrip.protectionTrip = 1; // not sure if this is the right trip... check NUH UH
		// MAYBE JUST DELAY THIS TASK SO BATT CONTROL CAN DETECT IT AND SET FOLAG AND GO TO SHUTDOWN ETC.
		osDelay(MPS_WAIT_TIME * 1000);
		osThreadTerminate(startupTaskHandle);
	}
	// otherwise everything is good to continue
	mbmsStatus.startupState = MPS_CLOSED;

	// delay to enable battery control task
	// delay 1 second, wait for battery control task to settle and decide if battery state is safe
	osDelay(MPS_WAIT_TIME * 1000);

	uint32_t flagsSet;
	// set flag to give permission to precharge/close common contactor
	flagsSet = osEventFlagsSet(contactorPermissionsFlagHandle, COMMON_FLAG);
	while ((contactorInfo[COMMON].contactorClosed == OPEN_CONTACTOR)) { //
		// wait for common contactor to close
	}
	if (contactorInfo[COMMON].contactorError) {
		// TO DO: handle error
		Error_Handler();
	}
	mbmsStatus.startupState = COMMON_CLOSED;

	// set flag to give permission to precharge/close LV
	flagsSet = osEventFlagsSet(contactorPermissionsFlagHandle, LV_FLAG); // set LV contactor flag
	while ((contactorInfo[LOWV].contactorClosed == OPEN_CONTACTOR)) { //
		// wait for LV contactor to close
	}
	mbmsStatus.startupState = LV_CLOSED;

	// enable/connect to DCDC1 and wait until DCDC1 on
	HAL_GPIO_WritePin(DCDC1_EN_GPIO_Port, DCDC1_EN_Pin, GPIO_PIN_SET);
	// wait for DCDC1 to connect!
	uint32_t DCDC1_Start_Count = osKernelGetTickCount();
	while(read_nDCDC1_ON() == 1) { // n stands for NOT
		//set a timeout, if this fails, trip
		uint32_t DCDC1_Current_Count = osKernelGetTickCount();
		uint32_t DCDC1_Time_Passed = (osKernelGetTickCount() - DCDC1_Start_Count) / seconds_per_tick; // USE THE GIUVEN SECONDS PER TICK MACRO FROM FREE RTOS
		if(DCDC1_Time_Passed >= DCDC1_WAIT_TIME){
			// TO DO: trip
			// but idk which type of trip this is...
			osThreadTerminate(startupTaskHandle);
		}
	}

	mbmsStatus.startupState = DCDC1_CLOSED;

	// disconnect DCDC0 (no longer connect to aux battery)
	HAL_GPIO_WritePin(ABATT_DISABLE_GPIO_Port, ABATT_DISABLE_Pin, GPIO_PIN_SET); // assuming disable is disconnect/open,
	// wait until DCDC0 has been disconnected (while DCDC0 == closed) {}
	uint32_t DCDC0_Start_Count = osKernelGetTickCount();
	while(read_nDCDC0_ON() == 0) {

		uint32_t DCDC0_Current_Count = osKernelGetTickCount();
		uint32_t DCDC0_Time_Passed = (osKernelGetTickCount() - DCDC0_Start_Count) * seconds_per_tick;
		if(DCDC0_Time_Passed >= DCDC0_WAIT_TIME){
			// TO DO: trip
			osThreadTerminate(startupTaskHandle);
		}
	}
	mbmsStatus.startupState = DCDC0_OPEN;

	// set flag to give permission to precharge/close motor contactor
	// just check that everything is good still (doesnt HAVE to close motor before moving on to next part)
	flagsSet = osEventFlagsSet(contactorPermissionsFlagHandle, (MOTOR1_FLAG | MOTOR2_FLAG)); // set both motors contactor flag

	mbmsStatus.startupState = MOTORS_ENABLED;

	//add a delay for 10 seconds
	// to give time for batt control to check things r ok, close contactors or not, decide if there needs to be a trip or not etc.
	osDelay(MOTOR_WAIT_TIME * 1000);

	// set flag to give permission to precharge/close array contactor
	// wait until array contactor done (same as above, make sure everything okay still, doesnt NEED it to bed closed...)
	flagsSet = osEventFlagsSet(contactorPermissionsFlagHandle, (ARRAY_FLAG | CHARGE_FLAG)); // set LV contactor flag
	// give time to battery control task to make sure battery state is still safe
	osDelay(ARRAY_WAIT_TIME * 1000);
	// if battery is fully charged, don't do the array!!!!
	// motor charge and array, just give permissions, dont need to wait for contactors
	// because if battery is fully discharged, then motors shouldnt go
	// if car is fully charged, dont need array


	// set flag that everything is done (all perms given!!!)
	mbmsStatus.startupState = FULLY_OPERATIONAL;

	// end of startup
	osThreadTerminate(startupTaskHandle);

}







