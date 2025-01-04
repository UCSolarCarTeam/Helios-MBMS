/*
 * RxGatekeeperTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 */
#include "CANRxGatekeeperTask.hpp"
#include "stm32f4xx_hal.h"
#include "CANdefines.h"


typedef struct {
    uint16_t ID;
    uint32_t extendedID;
    uint8_t DLC;
    uint8_t data[8];
} CANmsg;



void CANRxGatekeeperTask(void* arg)
{
    while(1)
    {
    	CANRxGatekeeper();
    }
}

void CANRxGatekeeper(void* arg)
{

	CANmsg msg; // CANmsg is struct (defined in CAN.h)

	osStatus_t status = osMessageQueueGet(RxCANMessageQueueHandle, &msg, 0, osWaitForever);
	if (status != osOK){
		// handle error but idk what to do here
	}
	// otherwise if its okay then...
	else if (status == osOK) {
		uint32_t eID = msg.extendedID;

		if (eID == packInfoID || eID == tempInfoID || eID == maxMinVoltagesID) {
			// add to queue for battery control task
			status = osMessageQueuePut(batteryControlMessageQueueHandle, &msg, 0, osWaitForever); // idk maybe shouldnt wait forever tho..
				if(status != osOK){
					// also handle error here but idk do what :(
				}
		}

	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
	CANmsg msg;

	CAN_RxHeaderTypeDef CANRxHeader;
	uint8_t  data[8]; // can hold 8 bytes of data
	HAL_CAN_GetRxMessage(hcan1, 0, &CANRxHeader, &data); // get CAN message from the FIFO 0 queue and store its header and data

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
