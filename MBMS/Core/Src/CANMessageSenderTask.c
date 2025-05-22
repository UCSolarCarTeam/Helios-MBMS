/*
 * CANMessageSenderTask.c
 *
 *  Created on: Mar 15, 2025
 *      Author: millainel
 */

#include "CANMessageSenderTask.h"
#include "BatteryControlTask.h"
#include "MBMS.h"
#include "StartupTask.h"
#include "CANdefines.h"

// Shared status and info structs
extern BatteryInfo batteryInfo;
extern MBMSStatus mbmsStatus;
extern MBMSTrip mbmsTrip;

//extern uint16_t tripData;
extern ContactorCommand contactorCommand;
extern PowerSelectionStatus powerSelectionStatus;
extern ContactorInfo contactorInfo[6];
extern MBMSSoftBatteryLimitWarning mbmsSoftBatteryLimitWarning;

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

	sendMBMSStatusCanMessage(); // i just added this idk
	sendContactorsCanMessage();
}

void sendTripStatusCanMessage() {
	// should send CAN message of trips ???
	CANMsg tripMsg;
	uint32_t tripData = ((mbmsTrip.highCellVoltageTrip & 0x1) << 0)   		  +	((mbmsTrip.lowCellVoltageTrip & 0x1) << 1)
			+ ((mbmsTrip.commonHighCurrentTrip & 0x1) << 2)   			  + ((mbmsTrip.motorHighCurrentTrip & 0x1) << 3)
			+ ((mbmsTrip.arrayHighCurrentTrip & 0x1) << 4)    			  + ((mbmsTrip.LVHighCurrentTrip & 0x1) << 5)
			+ ((mbmsTrip.chargeHighCurrentTrip & 0x1) << 6)   			  + ((mbmsTrip.protectionTrip & 0x1) << 7)
			+ ((mbmsTrip.orionMessageTimeoutTrip & 0x1) << 8) 			  + ((mbmsTrip.contactorDisconnectedUnexpectedlyTrip & 0x1) << 9)
			+ ((mbmsTrip.contactorConnectedUnexpectedlyTrip & 0x1) << 10) + ((mbmsTrip.highBatteryTrip & 0x1) << 11)
			+ ((mbmsTrip.commonHeartbeatDeadTrip & 0x1) << 12) 			  + ((mbmsTrip.motorHeartbeatDeadTrip & 0x1) << 13)
			+ ((mbmsTrip.arrayHeartbeatDeadTrip & 0x1) << 14)			  + ((mbmsTrip.LVHeartbeatDeadTrip & 0x1) << 15)
			+ ((mbmsTrip.chargeHeartbeatDeadTrip & 0x1) << 16)		      + ((mbmsTrip.MPSDisabledTrip & 0x1) << 17)
			+ ((mbmsTrip.ESDEnabledTrip & 0x1) << 18)					  + ((mbmsTrip.highTemperatureTrip & 0x1 << 19))
			+ ((mbmsTrip.lowTemperatureTrip & 0x1 << 20));

	tripMsg.data[0] = (tripData & 0xff);
	tripMsg.data[1] = (tripData & 0xff00) >> 8;
	tripMsg.data[2] = (tripData & 0xff0000) >> 16;
	tripMsg.DLC = 3; // 3 bytes
	tripMsg.extendedID = MBMS_TRIP_ID;
	tripMsg.ID = 0x0;
	osMessageQueuePut(TxCANMessageQueueHandle, &tripMsg, 0, osWaitForever);

}

void sendSoftBatteryLimitCanMessage() {
	CANMsg tripMsg;
	uint16_t tripData = ((mbmsSoftBatteryLimitWarning.highCellVoltageWarning & 0x1) << 0)   	+ ((mbmsSoftBatteryLimitWarning.lowCellVoltageWarning & 0x1) << 1)
			+ ((mbmsSoftBatteryLimitWarning.commonHighCurrentWarning & 0x1) << 2)   			+ ((mbmsSoftBatteryLimitWarning.motorHighCurrentWarning & 0x1) << 3)
			+ ((mbmsSoftBatteryLimitWarning.arrayHighCurrentWarning & 0x1) << 4)    			+ ((mbmsSoftBatteryLimitWarning.LVHighCurrentWarning & 0x1) << 5)
			+ ((mbmsSoftBatteryLimitWarning.chargeHighCurrentWarning & 0x1) << 6)   		    + ((mbmsSoftBatteryLimitWarning.highBatteryWarning & 0x1) << 7)
		    + ((mbmsSoftBatteryLimitWarning.highTemperatureWarning & 0x1 << 8))		     	+ ((mbmsSoftBatteryLimitWarning.lowTemperatureWarning & 0x1 << 9));

	tripMsg.data[0] = (tripData & 0xff);
	tripMsg.data[1] = (tripData & 0xff00) >> 8;
	tripMsg.DLC = 2; // 2 bytes
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
	CANMsg mbmsStatusMsg;
	uint16_t mbmsStatusData = ((mbmsStatus.auxilaryBattVoltage & 0x1f) << 0) + ((mbmsStatus.strobeBMSLight & 0x1) << 5)
			+ ((mbmsStatus.nChargeEnable & 0x1) << 6)   				     + ((mbmsStatus.nChargeSafety & 0x1) << 7)
			+ ((mbmsStatus.nDischargeEnable & 0x1) << 8)    				 + ((mbmsStatus.orionCANReceived & 0x1) << 9)
			+ ((mbmsStatus.dischargeShouldTrip & 0x1) << 10) 				 + ((mbmsStatus.chargeShouldTrip & 0x1) << 11)
			+ ((mbmsStatus.startupState & 0xf) << 12)						 + ((mbmsStatus.carState & 0x7) << 15);

	mbmsStatusMsg.data[0] = (mbmsStatusData & 0xff);
	mbmsStatusMsg.data[1] = (mbmsStatusData & 0xff00) >> 8;
	mbmsStatusMsg.data[2] = (mbmsStatusData & 0xff0000) >> 16;
	mbmsStatusMsg.DLC = 3; // 3 bytes as of may 21 (added carState..)
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
	CANMsg contactorCommandMsg; // NEED TO SET ATTRIBUTES OF CAN MSG
	contactorCommandMsg.DLC = 1;
	contactorCommandMsg.extendedID = CONTACTOR_COMMAND_ID;
	contactorCommandMsg.ID = 0x0;

	contactorCommandMsg.data[0] = ((contactorCommand.common & 0x01) << COMMON) + ((contactorCommand.motor & 0x01) << MOTOR)
								+ ((contactorCommand.array & 0x01) << ARRAY)   + ((contactorCommand.LV & 0x01) << LOWV)
								+ ((contactorCommand.charge & 0x01) << CHARGE);

	osMessageQueuePut(TxCANMessageQueueHandle, &contactorCommandMsg, 0, osWaitForever);

}











