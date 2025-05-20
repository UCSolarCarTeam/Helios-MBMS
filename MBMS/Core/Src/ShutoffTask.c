/*
 * ShutoffTask.c
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millaine li
 */

#include <stdint.h>


#include "../Inc/ShutoffTask.h"
#include "CANdefines.h"
#include "cmsis_os.h"
#include "main.h"
#include "BatteryControlTask.h"
#include "ReadPowerGPIO.h"
#include "MBMS.h"

extern MBMSStatus mbmsStatus;
extern MBMSTrip mbmsTrip;
extern ContactorInfo contactorInfo[6];
extern Permissions perms;

//extern ContactorState contactorState;

// PRIORITIES FOR SHUTDOWN PROCEDURE
// HIGHEST: HARD BATTERY LIMIT
// THEN ITS MPS/EPCOS, SOFT BATT LIMIT
// LOWEST: KEY

void ShutoffTask(void* arg)
{
    while(1)
    {
    	Shutoff();
    }
}


void Shutoff()
{

	uint32_t flags; //
	while (1) {


		// wait for shutoff flag
		flags = osEventFlagsWait(shutoffFlagHandle, SHUTOFF_FLAG, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

		if ((flags & HARD_BL_FLAG) == HARD_BL_FLAG) {
			HAL_GPIO_WritePin(Strobe_En_GPIO_Port, Strobe_En_Pin, GPIO_PIN_SET);
		}

		//disable 12V CAN
		HAL_GPIO_WritePin(_12V_CAN_En_GPIO_Port, _12V_CAN_En_Pin, GPIO_PIN_RESET);

		// ensure charging is disabled
		HAL_GPIO_WritePin(nCHG_LV_En_GPIO_Port, nCHG_LV_En_Pin, GPIO_PIN_SET);

		// open common
		perms.common = 0;

		// wait to open CHECK THIS
		while(contactorInfo[COMMON].contactorClosed == CLOSE_CONTACTOR) {

		}

		perms.motor = 0;
		// wait to open CHECK THIS
		while(contactorInfo[MOTOR].contactorClosed == CLOSE_CONTACTOR) {

		}

		perms.array = 0;
		// wait to open CHECK THIS
		while(contactorInfo[ARRAY].contactorClosed == CLOSE_CONTACTOR) {

		}

		perms.lv = 0;
		// wait to open CHECK THIS
		while(contactorInfo[LOWV].contactorClosed == CLOSE_CONTACTOR) {

		}

		perms.charge = 0;
		// wait to open CHECK THIS
		while(contactorInfo[CHARGE].contactorClosed == CLOSE_CONTACTOR) {

		}

		// Disable EN1...
		HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_RESET);


		if((flags & HARD_BL_FLAG) == HARD_BL_FLAG) {
			while(1){
				// wait for driver to turn off car using key
			}
		}

		else {
			// start thread for startup!!!
		}
	}

}







