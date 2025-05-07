/*
 * CANMessageSenderTask.c
 *
 *  Created on: Mar 15, 2025
 *      Author: millainel
 */

#include "../Inc/CANMessageSenderTask.h"
#include <stdint.h>
#include "BatteryControlTask.h"
#include "MBMS.h"
#include "CANdefines.h"
#include "StartupTask.h"

// testing comment!! can delete

extern BatteryInfo batteryInfo;

extern MBMSStatus mbmsStatus;

extern MBMSTrip mbmsTrip;

extern uint16_t tripData;

extern ContactorCommand contactorCommand;

extern PowerSelectionStatus powerSelectionStatus;

extern volatile ContactorInfo contactorInfo[6];

void CANMessageSenderTask(void* arg)
{
    while(1)
    {
    	CANMessageSender();
    }
}

void CANMessageSender() {

	sendMBMSHeartbeatCanMessage();
	/* Send out trip status (update struct */
	sendTripStatusCanMessage(&tripData);
	sendMBMSStatusCanMessage(); // i just added this idk
	sendContactorsCanMessage();
}

void sendTripStatusCanMessage(uint16_t * tripData) {
	// should send CAN message of trips ???
	CANMsg tripMsg;
	*tripData = ((mbmsTrip.highCellVoltageTrip & 0x1) << 0)   			  +	((mbmsTrip.lowCellVoltageTrip & 0x1) << 1)
			+ ((mbmsTrip.commonHighCurrentTrip & 0x1) << 2)   			  + ((mbmsTrip.motorHighCurrentTrip & 0x1) << 3)
			+ ((mbmsTrip.arrayHighCurrentTrip & 0x1) << 4)    			  + ((mbmsTrip.LVHighCurrentTrip & 0x1) << 5)
			+ ((mbmsTrip.chargeHighCurrentTrip & 0x1) << 6)   			  + ((mbmsTrip.protectionTrip & 0x1) << 7)
			+ ((mbmsTrip.orionMessageTimeoutTrip & 0x1) << 8) 			  + ((mbmsTrip.contactorDisconnectedUnexpectedlyTrip & 0x1) << 9)
			+ ((mbmsTrip.contactorConnectedUnexpectedlyTrip & 0x1) << 10) + ((mbmsTrip.highBatteryTrip & 0x1) << 11)
			+ ((mbmsTrip.commonHeartbeatDeadTrip & 0x1) << 12) 			  + ((mbmsTrip.motorHeartbeatDeadTrip & 0x1) << 13)
			+ ((mbmsTrip.arrayHeartbeatDeadTrip & 0x1) << 14)			  + ((mbmsTrip.LVHeartbeatDeadTrip & 0x1) << 15)
			+ ((mbmsTrip.chargeHeartbeatDeadTrip & 0x1) << 16)		      + ((mbmsTrip.MPSDisabledTrip & 0x1) << 17)
			+ ((mbmsTrip.ESDEnabledTrip & 0x1) << 18);

	tripMsg.data[0] = (*tripData & 0xff);
	tripMsg.data[1] = (*tripData & 0xff00) >> 8;
	tripMsg.data[2] = (*tripData & 0xff0000) >> 16;
	tripMsg.DLC = 3; // 2 bytes
	tripMsg.extendedID = MBMS_TRIP_ID;
	tripMsg.ID = 0x0;
	osMessageQueuePut(TxCANMessageQueueHandle, &tripMsg, 0, osWaitForever);

}

void sendPowerSelectionStatus() {
	CANMsg powSelectStatusMsg;
	uint16_t data = ((powerSelectionStatus.nMainPowerSwitch & 0x1) << 0) + ((powerSelectionStatus.ExternalShutdown & 0x1) << 1)
			+ ((powerSelectionStatus.EN1 & 0x1) << 2) + ((powerSelectionStatus.nDCDC_Fault & 0x1) << 3)
			+ ((powerSelectionStatus.n3A_OC & 0x1) << 4) + ((powerSelectionStatus.nDCDC_On & 0x1) << 5)
			+ ((powerSelectionStatus.nCHG_Fault & 0x1) << 6) + ((powerSelectionStatus.nCHG_On & 0x1) << 7)
			+ ((powerSelectionStatus.nCHG_LV_En & 0x1) << 8) + ((powerSelectionStatus.ABATT_Disable & 0x1) << 9)
			+ ((powerSelectionStatus.Key & 0x1) << 10);
	powSelectStatusMsg.data[0] = (data & 0xff);
	powSelectStatusMsg.data[1] = (data & 0xff00) >> 8;
	powSelectStatusMsg.DLC = 2;
	powSelectStatusMsg.extendedID = POWER_SELECTION_STATUS_ID;
	powSelectStatusMsg.ID = 0x0;
	osMessageQueuePut(TxCANMessageQueueHandle, &powSelectStatusMsg, 0, osWaitForever);

}
void sendMBMSStatusCanMessage() {
	// should send CAN message of trips ???
	CANMsg mbmsStatusMsg;
	uint16_t mbmsStatusData = ((mbmsStatus.auxilaryBattVoltage & 0x1f) << 0) + ((mbmsStatus.strobeBMSLight & 0x1) << 5)
			+ ((mbmsStatus.allowCharge & 0x1) << 6) + ((mbmsStatus.chargeSafety & 0x1) << 7)
			+ ((mbmsStatus.allowDischarge & 0x1) << 8) + ((mbmsStatus.orionCANReceived & 0x1) << 9)
			+ ((mbmsStatus.dischargeShouldTrip & 0x1) << 10) + ((mbmsStatus.chargeShouldTrip & 0x1) << 11)
			+ ((mbmsStatus.startupState & 0xf) << 15);

	mbmsStatusMsg.data[0] = (mbmsStatusData & 0xff);
	mbmsStatusMsg.data[1] = (mbmsStatusData & 0xff00) >> 8;
	mbmsStatusMsg.DLC = 2; // 2 bytes
	mbmsStatusMsg.extendedID = MBMS_STATUS_ID;
	mbmsStatusMsg.ID = 0x0;
	osMessageQueuePut(TxCANMessageQueueHandle, &mbmsStatusMsg, 0, osWaitForever);

}

void sendMBMSHeartbeatCanMessage() {
	CANMsg mbmsHeartbeatMsg;
	mbmsHeartbeatMsg.data[0] = 0x1;
	mbmsHeartbeatMsg.DLC = 1;
	mbmsHeartbeatMsg.extendedID = MBMS_HEARTBEAT_ID;
	mbmsHeartbeatMsg.ID = 0x0;
	osMessageQueuePut(TxCANMessageQueueHandle, &mbmsHeartbeatMsg, 0, osWaitForever);
}

void sendContactorsCanMessage() {
	// should close contactors if perms given? OH PROBABLY ALSO CHECK IF ALLOWE DTO DISCHARGE

	/* Update contactor state based on permissions and other things */
	uint32_t flags = osEventFlagsGet(contactorPermissionsFlagHandle);
	CANMsg contactorCommandMsg; // NEED TO SET ATTRIBUTES OF CAN MSG
//	tripMsg.DLC = 2; // 2 bytes
//	tripMsg.extendedID = MBMS_TRIP_ID;
//	tripMsg.ID = 0x0;
	uint8_t sendContactorCommand = 0;

	static uint8_t contactorClosing = false;
	for(int i = 0; i < 6; i++){
		if (contactorInfo[i].contactorState == CLOSING_CONTACTOR){
			contactorClosing = true;
			break;
		}
	}

	// if no contactors are currently in the state of closing, you may close a contactor!
	if(!contactorClosing) {

		if (((flags & COMMON_FLAG) == COMMON_FLAG) && (contactorInfo[COMMON].contactorState != CLOSE_CONTACTOR) && (tripData == 0x0)) {
			// if perms are given, and contactor is not already closed, and there are no trips
			contactorCommand.common = CLOSE_CONTACTOR;
			sendContactorCommand = 1;
		}
		else if (((flags & MOTOR_FLAG) == MOTOR_FLAG) && (contactorInfo[MOTOR].contactorState != CLOSE_CONTACTOR) && (tripData == 0x0) && (mbmsStatus.allowDischarge == 1)) {
			contactorCommand.motor = CLOSE_CONTACTOR;
			sendContactorCommand = 1;
		}
		else if (((flags & ARRAY_FLAG) == ARRAY_FLAG) && (contactorInfo[ARRAY].contactorState != CLOSE_CONTACTOR) && (tripData == 0x0) && (mbmsStatus.allowCharge == 1)) {
			contactorCommand.array = CLOSE_CONTACTOR;
			sendContactorCommand = 1;
		}
		else if (((flags & LV_FLAG) == LV_FLAG) && (contactorInfo[LOWV].contactorState != CLOSE_CONTACTOR) && (tripData == 0x0) && (mbmsStatus.allowDischarge == 1)) {
			contactorCommand.LV = CLOSE_CONTACTOR;
			sendContactorCommand = 1;
		}
		else if (((flags & CHARGE_FLAG) == CHARGE_FLAG) && (contactorInfo[CHARGE].contactorState != CLOSE_CONTACTOR) && (tripData == 0x0) && (mbmsStatus.allowCharge == 1)) {
			contactorCommand.charge = CLOSE_CONTACTOR;
			sendContactorCommand = 1;
		}

	}


	// Open contactors as needed SHOULD I CHECK TRIPDATA AS WELL HERE?or naw cuz a diff func will check idkkk i wanna say noo ...

	if (((flags & MOTOR_FLAG) == MOTOR_FLAG) && (contactorInfo[MOTOR].contactorState != OPEN_CONTACTOR) && (mbmsStatus.allowDischarge == 0)){
		contactorCommand.motor = OPEN_CONTACTOR;
		sendContactorCommand = 1;
	}

	if (((flags & ARRAY_FLAG) == ARRAY_FLAG) && (contactorInfo[ARRAY].contactorState != OPEN_CONTACTOR) && (mbmsStatus.allowCharge == 0)) {
		contactorCommand.array = OPEN_CONTACTOR;
		sendContactorCommand = 1;
	}


	if (((flags & LV_FLAG) == LV_FLAG) && (contactorInfo[LOWV].contactorState != OPEN_CONTACTOR) && (mbmsStatus.allowDischarge == 0)) {
		contactorCommand.LV = OPEN_CONTACTOR;
		sendContactorCommand = 1;
	}


	if (((flags & CHARGE_FLAG) == CHARGE_FLAG) && (contactorInfo[CHARGE].contactorState != OPEN_CONTACTOR) && (mbmsStatus.allowCharge == 0)) {
		contactorCommand.charge = OPEN_CONTACTOR;
		sendContactorCommand = 1;
	}


	if (sendContactorCommand == 1) {
		contactorCommandMsg.data[0] = ((contactorCommand.common & 0x01) << COMMON) + ((contactorCommand.motor & 0x01) << MOTOR)
									+ ((contactorCommand.array & 0x01) << ARRAY)
									+ ((contactorCommand.LV & 0x01) << LOWV)       + ((contactorCommand.charge & 0x01) << CHARGE);
		osMessageQueuePut(TxCANMessageQueueHandle, &contactorCommandMsg, 0, osWaitForever);
	}
}



