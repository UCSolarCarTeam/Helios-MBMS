/*
 * RxGatekeeperTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millaineli
 */
#include "../Inc/CANRxGatekeeperTask.h"
#include "BatteryControlTask.h"
#include "MBMS.h"
#include "CANdefines.h"
#include <stdint.h>
#include "main.h"


#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

extern volatile ContactorInfo contactorInfo[6];
extern MBMSStatus mbmsStatus;
extern BatteryInfo batteryInfo;

void CANRxGatekeeperTask(void* arg)
{
    while(1)
    {
    	CANRxGatekeeper();
    }
}

// PROBLEM: WHEN I HAD THIS IN THE OTHER FILE AND IT WAS A SEPERATE QUEUE I WAS ABLE TO USE THE TIMEOUT TO
// CHECK FOR THE ORION MESSAGE NOT RECEIVED BUT HERE I CANT CUZ ITS THE SAME QUEUE FOR EVERYTHING

// honestly just use the tick count stuff if u must :<, but i still made the other file just in case



void CANRxGatekeeper()
{

	CANMsg msg; // CANmsg is struct (defined in CAN.h)

	osStatus_t status = osMessageQueueGet(RxCANMessageQueueHandle, &msg, 0, osWaitForever);
	if (status != osOK){
		// handle error but idk what to do here
		Error_Handler();
	}
	// otherwise if its okay then...
	else if (status == osOK) {
		uint32_t eID = msg.extendedID;

		if (eID == PACK_INFO_ID || eID == TEMP_INFO_ID || eID == MIN_MAX_VOLTAGES_ID) {
			// add to queue for battery control task
			status = osMessageQueuePut(batteryControlMessageQueueHandle, &msg, 0, osWaitForever); // idk maybe shouldnt wait forever tho..
			if(status != osOK){
				// also handle error here but idk do what :(
				Error_Handler();
			}
		}

		else if (eID && CONTACTORMASK == CONTACTOR_HEARTBEATS_IDS) { // if id is 0x20X or 0x21X
			// add to queue for battery control task
			status = osMessageQueuePut(contactorMessageQueueHandle, &msg, 0, osWaitForever); // idk maybe shouldnt wait forever tho..
			if(status != osOK){
				// also handle error here but idk do what :(
				Error_Handler();
			}
		}

	}
}



void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
	CANMsg msg;

	CAN_RxHeaderTypeDef CANRxHeader;
	uint8_t  data[8]; // can hold 8 bytes of data
	HAL_CAN_GetRxMessage(&hcan1, 0, &CANRxHeader, (unsigned char*)data); // get CAN message from the FIFO 0 queue and store its header and data

	msg.extendedID = CANRxHeader.ExtId; // set CANmsg extended ID
	msg.DLC = CANRxHeader.DLC; // set CANmsg DLC

	for (int i = 0; i < msg.DLC; i++) { // set CANmsg data
		msg.data[i] = data[i];
	}


	osStatus_t status = osMessageQueuePut(RxCANMessageQueueHandle, &msg, 0, 0); // timeout should be 0
	if(status != osOK){
		// need to handle error ,,
	}

}
