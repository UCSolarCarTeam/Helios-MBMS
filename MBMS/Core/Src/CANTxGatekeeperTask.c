/*
 * TxGatekeeperTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 */

#include "CANTxGatekeeperTask.h"
#include "cmsis_os.h"
#include <stdint.h>

//// below defines from site: https://mikrocontroller.ti.bfh.ch/halDoc/group__CAN__Identifier__Type.html
//#define 	CAN_ID_EXT   0x00000004U
//#define 	CAN_ID_STD   0x00000000U
//
//#define 	CAN_RTR_DATA   0x00000000U // maybe, telling im sending data out
//#define 	CAN_RTR_REMOTE   0x00000002U // maybe, telling that i want data sent to me

void CANTxGatekeeperTask(void* arg)
{	
	CANMsg msg = {0};

    while(1)
    {
    	CANTxGatekeeper(&msg);
    }
}

void CANTxGatekeeper(CANMsg* msg)
{
	// Wait for a message to send
	osMessageQueueGet(TxCANMessageQueueHandle, msg, 0, osWaitForever);

	// Message header
	CAN_TxHeaderTypeDef canTxHeader;	
	canTxHeader.IDE 	= CAN_ID_EXT;
	canTxHeader.RTR 	= CAN_RTR_DATA;
	canTxHeader.ExtId  	= msg->extendedID;
	canTxHeader.DLC 	= msg->DLC;

	// Message Data
	uint8_t  data[8]; 
	for (int i = 0; i < msg->DLC; i++) {
		data[i] = msg->data[i];
	}

	// IDK ABT THIS ONE ... how do mailboxes work :(
	uint32_t mailbox; // Used to keep track of message transmission status, useful to have for further debugging

	HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(&hcan1, &canTxHeader, data, &mailbox);
	if (status == HAL_OK)
	{
		// CAN message sent successfully, maybe blink an led or send a UART message for indication
		__NOP();
	}
	else if (status == HAL_BUSY)
	{
		// No available mailboxes to transmit message
		__NOP();
	}
	else {
		// Error has occured
		__NOP();
	}

}



