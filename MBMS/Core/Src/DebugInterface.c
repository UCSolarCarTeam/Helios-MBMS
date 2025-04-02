/*
 * DebugInterface.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 */

#include "../Inc/DebugInterfaceTask.h"
#include "MBMS.h"
#include "CANdefines.h"
#include "main.h"

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
	// Khadeeja's testing comment <-- u can delete this later
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

	//test update pack info orion
	if (1) {

		CANMsg orionMsg;

		int16_t packCurrent = 14;
		uint16_t packVoltage = 20;
		uint8_t packSOC = 99;
		uint16_t packAmphours = 15;
		uint8_t packDOD = 18; // Depth of Discharge, 1-byte


		orionMsg.data[0] = packCurrent & 0xff; // first eight bits
		orionMsg.data[1] = (packCurrent & 0xff00) >> 8;
		orionMsg.data[2] = packVoltage & 0xff;
		orionMsg.data[3] = (packVoltage & 0xff00) >> 8;
		orionMsg.data[4] = packSOC & 0xff;
		orionMsg.data[5] = packAmphours & 0xff;
		orionMsg.data[6] = (packAmphours & 0xff00) >> 8;
		orionMsg.data[7] = packDOD & 0xff;

		orionMsg.DLC = 8;
		orionMsg.ID = 0x0;
		orionMsg.extendedID = PACKINFOID;

		osStatus status = osMessageQueuePut(batteryControlMessageQueueHandle, &orionMsg, 0, osWaitForever); // idk maybe shouldnt wait forever tho..
		if(status != osOK){
			// also handle error here but idk do what :(
		}

	}

	// test update temp info orion
	if(1) {
		CANMsg orionMsg;

		uint8_t highTemp = 40;
		uint8_t lowTemp = 5;
		uint8_t avgTemp = 20;

		orionMsg.data[0] = highTemp & 0xff;
		orionMsg.data[2] = lowTemp & 0xff;
		orionMsg.data[4] = avgTemp & 0xff;

		orionMsg.DLC = 8;

		orionMsg.ID = 0x0;
		orionMsg.extendedID = TEMPINFOID;

		osStatus status = osMessageQueuePut(batteryControlMessageQueueHandle, &orionMsg, 0, osWaitForever); // idk maybe shouldnt wait forever tho..
		if(status != osOK){
			// also handle error here but idk do what :(
		}

	}

	//test update min max voltages orion
	if(1) {
		CANMsg orionMsg;

		// voltage info (each 2-bytes)
		uint16_t maxCellVoltage = 80;
		uint16_t minCellVoltage = 1;
		uint16_t maxPackVoltage = 80;
		uint16_t minPackVoltage = 10;

		orionMsg.data[0] = maxCellVoltage & 0xff;
		orionMsg.data[1] = (maxCellVoltage & 0xff00) >> 8;
		orionMsg.data[2] = minCellVoltage & 0xff;
		orionMsg.data[3] = (minCellVoltage & 0xff00) >> 8;
		orionMsg.data[4] = maxPackVoltage & 0xff;
		orionMsg.data[5] = (maxPackVoltage & 0xff00) >> 8;
		orionMsg.data[6] = minPackVoltage & 0xff;
		orionMsg.data[7] = (minPackVoltage & 0xff00) >> 8;

		orionMsg.DLC = 8;

		orionMsg.ID = 0x0;
		orionMsg.extendedID = MAXMINVOLTAGESID;

		osStatus status = osMessageQueuePut(batteryControlMessageQueueHandle, &orionMsg, 0, osWaitForever); // idk maybe shouldnt wait forever tho..
		if(status != osOK){
			// also handle error here but idk do what :(
		}


	}


}







