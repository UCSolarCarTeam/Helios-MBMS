/*
 * DebugInterface.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 */

#include "../Inc/DebugInterfaceTask.h"
#include "MBMS.h"
#include "CANdefines.h"

void DebugInterfaceTask(void* arg)
{
    while(1)
    {
    	DebugInterface();
    }
}

void DebugInterface()
{

	// make a function to add things to message queues to test !!!!!! (goin around CAN ig..)

	// test contactor state updates
	if (1) {
		CANMsg contactorMsg;
		contactorMsg.DLC = 4;
		uint8_t state = 0x000100; // contactor is closed
		uint16_t current = 12;
		uint16_t voltage = 13;
		contactorMsg.data[0] = (state & 0x3f) + ((current & 0x3) << 6);
		contactorMsg.data[1] = (current & 0x3fc) >> 2;
		contactorMsg.data[2] = ((current & 0xc00) >> 10) + ((voltage & 0x3f) << 2);
		contactorMsg.data[3] = ((voltage & 0xfc0) >> 6);
		contactorMsg.extendedID = 0x210; //common


		osStatus status = osMessageQueuePut(contactorMessageQueueHandle, &contactorMsg, 0, osWaitForever); // idk maybe shouldnt wait forever tho..
			if(status != osOK){
				// also handle error here but idk do what :(
			}
	}
	// test contactor heartbeats
	if (1) {
		static uint16_t counter = 0;
		counter++;
		if (counter >= 65535) {
			counter = 0;
		}
		CANMsg contactorHeartbeatMsg;
		contactorHeartbeatMsg.DLC = 2;
		contactorHeartbeatMsg.data[0] = (counter & 0xff);
		contactorHeartbeatMsg.data[1] = (counter & 0xff00) >> 8;
		contactorHeartbeatMsg.extendedID = 0x200; //common heartbeat


		osStatus status = osMessageQueuePut(contactorMessageQueueHandle, &contactorHeartbeatMsg, 0, osWaitForever); // idk maybe shouldnt wait forever tho..
			if(status != osOK){
				// also handle error here but idk do what :(
			}
	}


}
