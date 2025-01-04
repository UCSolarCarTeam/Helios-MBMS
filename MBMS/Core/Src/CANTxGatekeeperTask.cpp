/*
 * TxGatekeeperTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 */


#include "CANTxGatekeeperTask.hpp"
#include "CAN.h" // idk if i need this anymore tbh
#include "stm32f4xx_hal.h"


// below defines from site: https://mikrocontroller.ti.bfh.ch/halDoc/group__CAN__Identifier__Type.html
#define 	CAN_ID_EXT   0x00000004U
#define 	CAN_ID_STD   0x00000000U

#define 	CAN_RTR_DATA   0x00000000U // maybe, telling im sending data out
#define 	CAN_RTR_REMOTE   0x00000002U // maybe, telling that i want data sent to me

typedef struct {
    uint16_t ID;
    uint32_t extendedID;
    uint8_t DLC;
    uint8_t data[8];
} CANMsg;


void CANTxGatekeeperTask(void* arg)
{
    while(1)
    {
    	CANTxGatekeeper();
    }
}

void CANTxGatekeeper(void* arg)
{

	CANmsg msg; // CANmsg is struct (defined in CAN.h)

	CAN_TxHeaderTypeDef CANTxHeader;
	uint8_t  data[8]; // can hold 8 bytes of data
	uint32_t mailbox; // IDK ABT THIS ONE ... how do mailboxes work :(

	CANTxHeader.IDE = CAN_ID_EXT;
	CANTxHeader.RTR = CAN_RTR_DATA;


	status = osMessageQueueGet(TxCANMessageQueueHandle, &msg, 0, osWaitForever);
	while (status != osOK){
		// do nothing, wait for it to be okay lol (?)
	}

	CANTxHeader.ExtId  = msg.extendedID;
	CANTxHeader.DLC = msg.DLC;
	for (int i = 0; i < msg.DLC; i++) {
		data[i] = msg.data[i];
	}

	HAL_CAN_AddTxMessage(&hcan1, &CANTxHeader, data, &mailbox);  //????????


}



// i think theres just one peripheral, and the peripheral is like the pins and stuff thats on THISSSSSSS board that does the CAN stuff
// like each board thingy has its own (one) peripheral


// old thing but no longer relevant bc we cant use those files w those functions but logic might be useful :)

//void CANTxGatekeeper(void* arg)
//{
//	CANMsg msg;
//	status = osMessageQueueGet(CANSPIMutexHandle, &msg, 0, osWaitForever);
//	while (status != osOK){
//		// do nothing, wait for it to be okay lol (?)
//	}
//
//	osStatus_t acquire = osMutexAcquire(CANSPIMutexHandle, osWaitForever);
//	if(acquire == osOK) {
//		sendExtendedCANMessage(&msg, &peripheral); // NEED TO MAKE PERIPHERAL STILL!!!!!!
//		osStatus_t release = osMutexRelease(CANSPIMutexHandle);
//	}
//
//	osDelay(100);
//
//}


