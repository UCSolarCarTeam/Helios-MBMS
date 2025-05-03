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

void CANRxGatekeeper()
{

	CANMsg msg; // CANmsg is struct (defined in CAN.h)

	osStatus_t status = osMessageQueueGet(RxCANMessageQueueHandle, &msg, 0, osWaitForever);
	if (status != osOK){
		// handle error but idk what to do here
	}
	// otherwise if its okay then...
	else if (status == osOK) {



		// if the message is contactor states
		if ((msg.extendedID & 0xff0) == CONTACTORIDS){
			uint8_t data[msg.DLC];
			for (int i = 0; i < msg.DLC; i ++) {
				data[i] = msg.data[i];
			}

			uint8_t prechargerClosed = data[0] & 0x01; // extract bit 0
			uint8_t prechargerClosing = data[0] & 0x02; // extract bit 1
			uint8_t prechargerError = data[0] & 0x04; // extraxt bit 2
			uint8_t contactorState = (data[0] & 0x08) + ((data[0] & 0x10) << 1); // extract bit 3 & 4 and put into one variable
			uint8_t contactorError = data[0] & 0x20; // extract bit 5
			uint16_t contactorCurrent = ((data[0] & 0xc0) >> 6) + ((data[1] & 0xff) << 2) + ((data[2] & 0x03) << 10); // extract bits 6 to 17
			uint16_t contactorVoltage = ((data[2] & 0xfc) >> 2) + ((data[3] & 0x3f) << 6); // extract bits 18 to 29

			updateContactorInfo((msg.extendedID - CONTACTORIDS), prechargerClosed, prechargerClosing, prechargerError, contactorState, contactorError, contactorCurrent, contactorVoltage);

		}
		// if the message is a contactor heartbeat
		else if((msg.extendedID & 0xff0) == CONTACTOR_HEARTBEATS_IDS){
			uint16_t newHeartbeat = msg.data[0] + (msg.data[1] << 8);
			contactorInfo[msg.extendedID - CONTACTOR_HEARTBEATS_IDS].heartbeat = newHeartbeat;
		}

		else if (msg.extendedID == PACKINFOID || msg.extendedID == TEMPINFOID || msg.extendedID == MAXMINVOLTAGESID) {

			uint8_t data[msg.DLC];
			for (int i = 0; i < msg.DLC; i ++) {
				data[i] = msg.data[i];
			}

			if (msg.extendedID == PACKINFOID) {
				// update batteryInfo instance for the pack info stuff
				batteryInfo.packCurrent = data[0] + (data[1] << 8);
				batteryInfo.packVoltage = data[2] + (data[3] << 8);
				batteryInfo.packSOC = data[4];
				batteryInfo.packAmphours = data[5] + (data[6] << 8);
				batteryInfo.packDOD = data[7];

				mbmsStatus.auxilaryBattVoltage = batteryInfo.packVoltage;

				// updating allow charge/discharge on mbmsStatus, based on SOC
				if (batteryInfo.packSOC >= SOC_SAFE_FOR_DISCHARGE) {
					mbmsStatus.allowDischarge = 1;
				}
				if (batteryInfo.packSOC <= SOC_SAFE_FOR_CHARGE) {
					mbmsStatus.allowCharge = 1;
				}


			}
			else if (msg.extendedID == TEMPINFOID) {
				batteryInfo.highTemp = data[0];
				batteryInfo.lowTemp = data[2];
				batteryInfo.avgTemp = data[4];
			}

			else if (msg.extendedID == MAXMINVOLTAGESID) {
				batteryInfo.maxCellVoltage = data[0] + (data[1] << 8);
				batteryInfo.minCellVoltage = data[2] + (data[3] << 8);
				batteryInfo.maxPackVoltage = data[4] + (data[5] << 8);
				batteryInfo.minPackVoltage = data[6] + (data[7] << 8);
			}

		}


	}
}

void updateContactorInfo(uint8_t contactor, uint8_t prechargerClosed, uint8_t prechargerClosing, uint8_t prechargerError, uint8_t contactorState, uint8_t contactorError, uint16_t contactorCurrent, uint16_t contactorVoltage) {
	contactorInfo[contactor].prechargerClosed = prechargerClosed;
	contactorInfo[contactor].prechargerClosing = prechargerClosing;
	contactorInfo[contactor].prechargerError = prechargerError;
	contactorInfo[contactor].contactorState = contactorState;
	contactorInfo[contactor].contactorError = contactorError;
	contactorInfo[contactor].current = contactorCurrent;
	contactorInfo[contactor].voltage = contactorVoltage;
	return;
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
