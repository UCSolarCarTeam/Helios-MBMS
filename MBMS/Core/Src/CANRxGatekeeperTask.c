/*
 * RxGatekeeperTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millaineli
 */
#include "CANRxGatekeeperTask.h"

#include <stdint.h>
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

#include "BatteryControlTask.h"
#include "MBMS.h"
#include "CANdefines.h"

extern volatile ContactorInfo contactorInfo[6];
extern MBMSStatus mbmsStatus;
extern BatteryInfo batteryInfo;

uint32_t It_messages_received = 0;
uint32_t messages_received = 0;

uint32_t common_heartbeat_count = 0;
uint32_t motor_heartbeat_count = 0;
uint32_t array_heartbeat_count = 0;
uint32_t lv_heartbeat_count = 0;
uint32_t charge_heartbeat_count = 0;

uint32_t common_msg_count = 0;
uint32_t motor_msg_count = 0;
uint32_t array_msg_count = 0;
uint32_t lv_msg_count = 0;
uint32_t charge_msg_count = 0;

uint32_t pack_msg_count = 0;
uint32_t temp_msg_count = 0;
uint32_t cell_msg_count = 0;

uint32_t heartbeat_update_count;

uint32_t orion_msg_added = 0;

uint32_t RxCanIntQueueFull = 0;
uint32_t batteryControlQueueFull = 0;
uint32_t contactorQueueFull = 0;

void CANRxGatekeeperTask(void* arg)
{
	uint32_t taskTickLastStart = osKernelGetTickCount();
    while(1)
    {

    	CANRxGatekeeper();
//		taskTickLastStart += 10;
//		osDelayUntil(taskTickLastStart);
    }
}

// PROBLEM: WHEN I HAD THIS IN THE OTHER FILE AND IT WAS A SEPERATE QUEUE I WAS ABLE TO USE THE TIMEOUT TO
// CHECK FOR THE ORION MESSAGE NOT RECEIVED BUT HERE I CANT CUZ ITS THE SAME QUEUE FOR EVERYTHING
// honestly just use the tick count stuff if u must :<, but i still made the other file just in case

void CANRxGatekeeper()
{

	if ( (0x304 & CONTACTORMASK) == (0x200 & CONTACTORMASK)) {
		uint8_t x = 0;
	}
	CANMsg msg; // CANmsg is struct (defined in CAN.h)
	osStatus_t status = osMessageQueueGet(RxCANMessageQueueHandle, &msg, 0, osWaitForever);
	if (status != osOK){
		// handle error but idk what to do here
		Error_Handler();
	}
	// otherwise if its okay then...
	else if (status == osOK) {
		uint32_t eID = msg.extendedID;
		messages_received++;

		if ((eID == PACK_INFO_ID) ||( eID == TEMP_INFO_ID) || (eID == CELL_VOLTAGES_ID) || (eID == MIN_MAX_VOLTAGES_ID)) {
			// add to queue for battery control task
			status = osMessageQueuePut(batteryControlMessageQueueHandle, &msg, 0, 0); // idk maybe shouldnt wait forever tho..
			if(status != osOK){
				// also handle error here but idk do what :(
				//Error_Handler();
				batteryControlQueueFull++;
			}
			else {
				orion_msg_added++;
			}
		}
		else if ((eID & CONTACTORMASK) == CONTACTOR_MASKED_IDS )
		{ // if id is 0x20X or 0x21X
			// add to queue for battery control task
			status = osMessageQueuePut(contactorMessageQueueHandle, &msg, 0, 0); // idk maybe shouldnt wait forever tho..
			if(status != osOK){
				// also handle error here but idk do what :(
				//Error_Handler();
				contactorQueueFull++;
			}
		}
	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
	// Receive header and data
	CAN_RxHeaderTypeDef canRxHeader;
	uint8_t  			data[8]; 

	// get CAN message from the FIFO 0 queue and store its header and data, return from interrupt if it fails
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &canRxHeader, data) != HAL_OK)
	{
		return;
	}

	CANMsg msg;
	msg.extendedID = canRxHeader.ExtId; // set CANmsg extended ID
	msg.DLC = canRxHeader.DLC; // set CANmsg DLC
	for (int i = 0; i < msg.DLC; i++) { // set CANmsg data
		msg.data[i] = data[i];
	}
	switch(msg.extendedID) {
		case 0x200:
			common_heartbeat_count++;
			break;
		case 0x201:
			motor_heartbeat_count++;
			break;
		case 0x202:
			array_heartbeat_count++;
			break;
		case 0x203:
			lv_heartbeat_count++;
			break;
		case 0x204:
			charge_heartbeat_count++;
			break;

		case 0x210:
			common_msg_count++;
			break;
		case 0x211:
			motor_msg_count++;
			break;
		case 0x212:
			array_msg_count++;
			break;
		case 0x213:
			lv_msg_count++;
			break;
		case 0x214:
			charge_msg_count++;
			break;

		case 0x302:
			pack_msg_count++;
			break;
		case 0x304:
			temp_msg_count++;
			break;
		case 0x305:
			cell_msg_count++;
			break;
		default:
			return;

	}

	It_messages_received++;

	osStatus_t status = osMessageQueuePut(RxCANMessageQueueHandle, &msg, 0, 0); // timeout should be 0
	if(status != osOK){
		//Error_Handler();
		RxCanIntQueueFull++;
		// need to handle error ,,
	}
}
