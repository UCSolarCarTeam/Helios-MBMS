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


	mbmsStatus.startupState = nMPS_ENABLED;

	while (read_nMPS() == 1) {
		// wait for MPS to be on/enabled
	}

	mbmsStatus.startupState = nMPS_DISABLED;


	while (read_ESD() == 1) {
		// do BPS fault
		// instead should i just have an osDelay, for BCT to run and set the trip/fault..? bc we have to keep track of that in the struct
		uint32_t shutoffFlagsSet = osEventFlagsSet(shutoffFlagHandle, ESD_FLAG| SHUTOFF_FLAG);
		osThreadTerminate(startupTaskHandle);
	}

	mbmsStatus.startupState = ESD_DISABLED;

	// DO NEW CHECKS!!!!! if fails, either BPS fault, or keeps checking
	// maybe call BCT functionto do checks... make a function specific to startup checks
	mbmsStatus.startupState = CHECKS_PASSED;

	// clear all saved faults
	mbmsStatus.startupState = FAULTS_CLEARED;

	// wait for discharge enable sense

	uint32_t flagsSet;

	flagsSet = osEventFlagsSet(contactorPermissionsFlagHandle, COMMON_FLAG);
	while ((contactorInfo[COMMON].contactorClosed != CLOSE_CONTACTOR)) {
		// wait for common contactor to close
		//should i add an osDelay here so BCT can run? ask nathan probably (same w lv, or anywhere u want a diff task to be able to run)
	}
	if (contactorInfo[COMMON].contactorError) {
		// TO DO: handle error
		Error_Handler();
	}
	mbmsStatus.startupState = COMMON_CLOSED;

	// set flag to give permission to precharge/close LV
	flagsSet = osEventFlagsSet(contactorPermissionsFlagHandle, LV_FLAG); // set LV contactor flag
	while ((contactorInfo[LOWV].contactorClosed != CLOSE_CONTACTOR)) {
		// wait for LV contactor to close
	}
	mbmsStatus.startupState = LV_CLOSED;


	// enable/connect to DCDC1 and wait until DCDC1 on
	HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_SET); // enable DCDC HV

	/* OLD THING WITH DCDC1 TIMEOUT, keeping for now in case they decide we need a timeout for EN1 but they said no checks for it so

	 // wait for DCDC1 to connect!
	uint32_t DCDC1_Start_Count = osKernelGetTickCount();
	while(read_nDCDC1_ON() == 1) { // n stands for NOT
		//set a timeout, if this fails, trip
		uint32_t DCDC1_Time_Passed = (osKernelGetTickCount() - DCDC1_Start_Count) * FREERTOS_TICK_PERIOD; // USE THE GIUVEN SECONDS PER TICK MACRO FROM FREE RTOS
		if(DCDC1_Time_Passed >= DCDC1_WAIT_TIME){
			// TO DO: trip
			// but idk which type of trip this is...
			osThreadTerminate(startupTaskHandle);
		}
	}


	 */

	mbmsStatus.startupState = EN1_ON;


	// set flag to give permission to precharge/close motor contactor
	// just check that everything is good still (doesnt HAVE to close motor before moving on to next part)
	flagsSet = osEventFlagsSet(contactorPermissionsFlagHandle, MOTOR_FLAG ); // set both motors contactor flag

	mbmsStatus.startupState = MOTORS_PERMS;

	//add a delay for 10 seconds
	// to give time for batt control to check things r ok, close contactors or not, decide if there needs to be a trip or not etc.
	osDelay(MOTOR_WAIT_TIME * 1000);

	flagsSet = osEventFlagsSet(contactorPermissionsFlagHandle, CHARGE_FLAG); // set LV contactor flag

	mbmsStatus.startupState = CHARGE_PERMS;
	// give time to battery control task to make sure battery state is still safe
	osDelay(CHARGE_WAIT_TIME * 1000);

	// set flag to give permission to precharge/close array contactor
	// wait until array contactor done (same as above, make sure everything okay still, doesnt NEED it to bed closed...)
	flagsSet = osEventFlagsSet(contactorPermissionsFlagHandle, ARRAY_FLAG); // set LV contactor flag

	mbmsStatus.startupState = ARRAY_PERMS;
	// give time to battery control task to make sure battery state is still safe
	osDelay(ARRAY_WAIT_TIME * 1000);
	// if battery is fully charged, don't do the array!!!!
	// motor charge and array, just give permissions, dont need to wait for contactors
	// because if battery is fully discharged, then motors shouldnt go
	// if car is fully charged, dont need array


	/* OLD CODE TO DISCONNECT FROM AUX BATTERY

	// turn off DCDC0 (no longer connect to aux battery)
	HAL_GPIO_WritePin(ABATT_DISABLE_GPIO_Port, ABATT_DISABLE_Pin, GPIO_PIN_SET); // assuming disable is disconnect/open,
	// wait until DCDC0 has been disconnected (while DCDC0 == closed) {}
	uint32_t DCDC0_Start_Count = osKernelGetTickCount();
	while(read_nDCDC0_ON() == 0) {
		uint32_t DCDC0_Time_Passed = (osKernelGetTickCount() - DCDC0_Start_Count) * FREERTOS_TICK_PERIOD;
		if(DCDC0_Time_Passed >= DCDC0_WAIT_TIME){
			// TO DO: trip
			osThreadTerminate(startupTaskHandle);
		}
	}
	mbmsStatus.startupState = DCDC0_OFF;

	 */

	// set flag that everything is done (all perms given!!!)
	mbmsStatus.startupState = FULLY_OPERATIONAL;

	// end of startup
	osThreadTerminate(startupTaskHandle);

}







