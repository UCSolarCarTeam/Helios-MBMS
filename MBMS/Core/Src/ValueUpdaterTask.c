/*
 * ValueUpdater.c
 *
 *  Created on: May 7, 2025
 *      Author: millaine
 */

#include "../Inc/ValueUpdaterTask.h"
#include "BatteryControlTask.h"
#include "MBMS.h"
#include "ReadPowerGPIO.h"
#include "CANdefines.h"
#include <stdint.h>
#include "main.h"


#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

extern volatile ContactorInfo contactorInfo[6];
extern MBMSStatus mbmsStatus;
extern BatteryInfo batteryInfo;
extern PowerSelectionStatus powerSelectionStatus;

void ValueUpdaterTask(void* arg)
{
    while(1)
    {
    	ValueUpdater();
    }
}


void ValueUpdater(){

	updateContactorInfoStruct();
	updateOrionInfoStruct();
	updatePowerSelectionStruct();

}


void updateContactorInfoStruct() {
	static uint8_t counter = 0;

	CANMsg contactorMsg;
	// Non blocking check for messages from contactors !
	osStatus status = osMessageQueueGet(contactorMessageQueueHandle, &contactorMsg, NULL, 0);
	if (status == osOK) {
		// if the message is about the contactor heartbeats
		if((contactorMsg.extendedID & 0xff0) == CONTACTOR_HEARTBEATS_IDS){
			uint16_t newHeartbeat = contactorMsg.data[0] + (contactorMsg.data[1] << 8);
			contactorInfo[contactorMsg.extendedID - CONTACTOR_HEARTBEATS_IDS].heartbeat = newHeartbeat;
		}
		// if the message is about the contactor info
		else{
			uint8_t data[contactorMsg.DLC];
			for (int i = 0; i < contactorMsg.DLC; i ++) {
				data[i] = contactorMsg.data[i];
			}

			uint8_t prechargerClosed = data[0] & 0x01; // extract bit 0
			uint8_t prechargerClosing = data[0] & 0x02; // extract bit 1
			uint8_t prechargerError = data[0] & 0x04; // extract bit 2
			uint8_t contactorClosed = data[0] & 0x08; // extract bit 3
			uint8_t contactorClosing = data[0] & 0x10; // extract bit 4
			uint8_t contactorError = data[0] & 0x20; // extract bit 5
			int16_t lineCurrent = ((data[0] & 0xc0) >> 6) + ((data[1] & 0xff) << 2) + ((data[2] & 0x03) << 10); // extract bits 6 to 17
			int16_t chargeCurrent = ((data[2] & 0xfc) >> 2) + ((data[3] & 0x3f) << 6); // extract bits 18 to 29
			uint8_t BPSerror = data[3] & 0x80; //extract bit 30
			updateContactorInfo((contactorMsg.extendedID - CONTACTORIDS), prechargerClosed, prechargerClosing, prechargerError,
					contactorClosed, contactorClosing, contactorError, lineCurrent, chargeCurrent, BPSerror);

		}
	}

}

void updateContactorInfo(uint8_t contactor, uint8_t prechargerClosed, uint8_t prechargerClosing, uint8_t prechargerError,
		uint8_t contactorClosed, uint8_t contactorClosing, uint8_t contactorError, int16_t lineCurrent, int16_t chargeCurrent, uint8_t BPSerror) {
	contactorInfo[contactor].prechargerClosed = prechargerClosed;
	contactorInfo[contactor].prechargerClosing = prechargerClosing;
	contactorInfo[contactor].prechargerError = prechargerError;
	contactorInfo[contactor].contactorClosed = contactorClosed;
	contactorInfo[contactor].contactorError = contactorError;
	contactorInfo[contactor].lineCurrent = lineCurrent;
	contactorInfo[contactor].chargeCurrent = chargeCurrent;
	contactorInfo[contactor].BPSerror = BPSerror;
	return;
}

void updatePowerSelectionStruct() {
	powerSelectionStatus.nMainPowerSwitch = read_nMPS();
	powerSelectionStatus.ExternalShutdown = read_ESD();
	powerSelectionStatus.EN1 = read_EN1();
	powerSelectionStatus.n3A_OC = read_n3A_OC();
	powerSelectionStatus.nDCDC_Fault = read_nDCDC_Fault();
	powerSelectionStatus.nCHG_Fault = read_nCHG_Fault();
	powerSelectionStatus.nCHG_On = read_nCHG_On();
	powerSelectionStatus.nCHG_LV_En = read_nCHG_LV_En();
	powerSelectionStatus.ABATT_Disable = read_ABATT_Disable();
	powerSelectionStatus.Key = read_Key();

}

void updateOrionInfoStruct() {
	//Update all information regarding the battery
	CANMsg orionMsg;
	osDelay(1000);
	static uint8_t orionMessageCounter = 0;
	osStatus status = osMessageQueueGet(batteryControlMessageQueueHandle, &orionMsg, NULL, ORION_MSG_WAIT_TIMEOUT);  // Timeout = 0 means non-blocking
	if (status == osOK) {
		orionMessageCounter = 0; // reset counter to zero now that you've received message
		mbmsStatus.orionCANReceived = 1;

		// update the struct w the info
		// do the checks u need w info given by orion
		uint8_t data[orionMsg.DLC];
		for (int i = 0; i < orionMsg.DLC; i ++) {
			data[i] = orionMsg.data[i];
		}

		if (orionMsg.extendedID == PACK_INFO_ID) {
			// update batteryInfo instance for the pack info stuff
			batteryInfo.packCurrent = data[0] + (data[1] << 8);
			batteryInfo.packVoltage = data[2] + (data[3] << 8);
			batteryInfo.packSOC = data[4];
			batteryInfo.packAmphours = data[5] + (data[6] << 8);
			batteryInfo.packDOD = data[7];

			mbmsStatus.auxilaryBattVoltage = batteryInfo.packVoltage;

			// updating allow charge/discharge on mbmsStatus, based on SOC
			if ((batteryInfo.packSOC >= SOC_SAFE_FOR_DISCHARGE) && (read_Charge_Enable() == 1)) {
				mbmsStatus.allowDischarge = 1;
			}
			if ((batteryInfo.packSOC <= SOC_SAFE_FOR_CHARGE) && (read_Discharge_Enable() == 1)) {
				mbmsStatus.allowCharge = 1;
			}


		}
		else if (orionMsg.extendedID == TEMP_INFO_ID) {
			batteryInfo.highTemp = data[0];
			batteryInfo.lowTemp = data[2];
			batteryInfo.avgTemp = data[4];
		}

		else if (orionMsg.extendedID == MIN_MAX_VOLTAGES_ID) {
			batteryInfo.maxCellVoltage = data[0] + (data[1] << 8);
			batteryInfo.minCellVoltage = data[2] + (data[3] << 8);
			batteryInfo.maxPackVoltage = data[4] + (data[5] << 8);
			batteryInfo.minPackVoltage = data[6] + (data[7] << 8);
		}

	}

	else if (status == osErrorTimeout) // if timeout for orion (no message :0)
	{
		orionMessageCounter += 1;
	}
	if(orionMessageCounter >= 3){
		mbmsStatus.orionCANReceived = 0; // no orion message recieved !!!
	}

}












