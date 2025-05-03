/*
 * BatteryControlTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millainel
 */

// look at orion interface task from old code to see what they did

#include "../Inc/BatteryControlTask.h"
#include <stdint.h>
#include "cmsis_os.h"
#include "CANdefines.h"
#include "StartupTask.h"
#include "ShutoffTask.h"
#include "ReadPowerGPIO.h"
#include "MBMS.h"

//ContactorState contactorState = {0};

BatteryInfo batteryInfo;

MBMSStatus mbmsStatus;

MBMSTrip mbmsTrip;

ContactorCommand contactorCommand;

PowerSelectionStatus powerSelectionStatus;

volatile ContactorInfo contactorInfo[6]; // one for each contactor        add volatile to the extern thing too

uint16_t tripData = 0;

static uint8_t heartbeatLastUpdatedTime[6] = {0};

static uint16_t previousHeartbeats[6] = {0}; //check this !!! syntax !


// STROBE ENABLE IS NOW GPIO STRAIGHT FROM MBMS, need CAN message to let rest of car know that its currently strobing
// actiive low ??????
// strobe en pulled low
// pull down, we want it to strobe if not connected??


// if trip, check if startup state is less than fully operational, if so, kill it, otherwise if its at fully operational it would've already killed itself

void BatteryControlTask(void* arg)
{

    while(1)
    {
    	BatteryControl();
    }
}


void BatteryControl()
{




	// before u set shutdown flag, check startupState and terminate the task if its not finished... ?

	checkContactorHeartbeats();

	updatePackInfoStruct();
	updateContactorInfoStruct();


	/* Check for trips based on information at hand */
	updateTripStatus();



	checkIfShutdown();



	//updateContactors();

	// NEED TO CHECK CURRENT STUFF STILL.... need to do all the trip stuff lol, also check attributes of can msg for sumn idk

}
void waitForFirstHeartbeats() {
	static uint8_t heartbeatFailCounter[5] = {0};


	for(int i = 0; i < 5; i++) {
		if(heartbeatFailCounter[i] > 3) {
			switch (i) {
				case 0:
					mbmsTrip.commonHeartbeatDeadTrip = 1;
					break;
				case 1:
					mbmsTrip.motorHeartbeatDeadTrip = 1;
					break;
				case 2:
					mbmsTrip.arrayHeartbeatDeadTrip = 1;
					break;
				case 3:
					mbmsTrip.LVHeartbeatDeadTrip = 1;
					break;
				case 4:
					mbmsTrip.chargeHeartbeatDeadTrip = 1;
					break;
			}
			// MAYBE STRAIGHT UP SET SHUTDOWN FLAG HERE ! um maybe not #modularity or sumn

		}
		if(previousHeartbeats[i] >= 65535) { // check this logic lol
			previousHeartbeats[i] = 0;
		}
		if(previousHeartbeats[i] >= contactorInfo[i].heartbeat){
			if(((osKernelGetTickCount() - heartbeatLastUpdatedTime[i]) * FREERTOS_TICK_PERIOD) > CONTACTOR_HEARTBEAT_TIMEOUT) { // where contactor_heartbeat_timeout is how often a heartbeat is sent out/recieved
				heartbeatFailCounter[i]++;

			}
		}
		else {
			heartbeatLastUpdatedTime[i] = osKernelGetTickCount();
			previousHeartbeats[i] = contactorInfo[i].heartbeat;
			heartbeatFailCounter[i] = 0;
		}
	}

}

/*
 * returns whether any contactors are closed (0) or not (1). want them all to be open on startup
 */
uint8_t checkContactorsOpen() {
	// BUT SHOULD I OPEN THE CONTACTOR IF THEY ARE CLOSED? maybe yeah. btw contactors r non-latching so theyll open if car powers off yk
	uint8_t open = 1;
	for (int i = 0; i < 5; i++) {
		if (contactorInfo[i].contactorState == CLOSE_CONTACTOR) {
			open = 0;
			break;
		}
	}
	return open;
}

/*
 * returns whether any prechargers are closed (0) or not (1).
 */
uint8_t checkPrechargersOpen() {
	uint8_t open = 1;
	for (int i = 0; i < 5; i++) {
		if (contactorInfo[i].prechargerClosed == CLOSE_CONTACTOR) {
			open = 0;
			break;
		}
	}
	return open;


}

void startupCheck(){

	/* Waiting for contactor heartbeats */
	// idk this feels kinda sketchy, maybe look at it ...
	while ((previousHeartbeats[0] == 0) || (previousHeartbeats[1] == 0) || (previousHeartbeats[2] == 0) ||
		   (previousHeartbeats[3] == 0) || (previousHeartbeats[4] == 0))
	{
		// maybe add an osDelay so a diff task can run? CAN Rx.. to actually receive heartbeat messages, and also the one that sets trips and does shutdown idk :(
		updateContactorInfoStruct();
		waitForFirstHeartbeats();
	}

	/* Check contactors & prechargers are open */
	if ((checkContactorsOpen() == 0) || checkPrechargersOpen() == 0){
		//bps fault
	}

	/* Battery check (orion) */



}

void checkContactorHeartbeats() {
	/* The reason i'm checking heartbeats this way and not with a timeout for the message queue (like orion)
	 * is because there's multiple contactor boards all sending their heartbeats so how the timeout wouldn't
	 * be accurate of what the problem is, or if there is a proble. For example if a contactor dies, other
	 * contactor would still be sending messages.
	 */



	for(int i = 0; i < 5; i++) {
		if(previousHeartbeats[i] >= 65535) { // check this logic lol
			previousHeartbeats[i] = 0;
		}
		if(previousHeartbeats[i] >= contactorInfo[i].heartbeat){
			if(((osKernelGetTickCount() - heartbeatLastUpdatedTime[i]) * FREERTOS_TICK_PERIOD) > CONTACTOR_HEARTBEAT_TIMEOUT) {

				// set heartbeat dead trip
				switch (i) {
					case 0:
						mbmsTrip.commonHeartbeatDeadTrip = 1;
						break;
					case 1:
						mbmsTrip.motorHeartbeatDeadTrip = 1;
						break;
					case 2:
						mbmsTrip.arrayHeartbeatDeadTrip = 1;
						break;
					case 3:
						mbmsTrip.LVHeartbeatDeadTrip = 1;
						break;
					case 4:
						mbmsTrip.chargeHeartbeatDeadTrip = 1;
						break;
				}

			}
		}
		else {
			heartbeatLastUpdatedTime[i] = osKernelGetTickCount();
			previousHeartbeats[i] = contactorInfo[i].heartbeat;
		}
	}
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
			uint8_t prechargerError = data[0] & 0x04; // extraxt bit 2
			uint8_t contactorState = (data[0] & 0x08) + ((data[0] & 0x10) << 1); // extract bit 3 & 4 and put into one variable
			uint8_t contactorError = data[0] & 0x20; // extract bit 5
			uint16_t contactorCurrent = ((data[0] & 0xc0) >> 6) + ((data[1] & 0xff) << 2) + ((data[2] & 0x03) << 10); // extract bits 6 to 17
			uint16_t contactorVoltage = ((data[2] & 0xfc) >> 2) + ((data[3] & 0x3f) << 6); // extract bits 18 to 29

			updateContactorInfo((contactorMsg.extendedID - CONTACTORIDS), prechargerClosed, prechargerClosing, prechargerError, contactorState, contactorError, contactorCurrent, contactorVoltage);

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

void updatePowerSelectionStruct() {

}



void checkIfShutdown() {
	/* Update System status */
	if(tripData != 0) { // was og written as tripStruct by violet
		/* go to shutdown */
		/* Update contactor permission to none or less based on type of shutdown */

	}
	// check key and mps and hard and soft batt limit to possibly set shutdown flags !!!!
	uint32_t shutoffFlagsSet;
	if (read_KeySwitch() == KEY_OFF){
		shutoffFlagsSet = osEventFlagsSet(shutoffFlagHandle, KEY_FLAG | SHUTOFF_FLAG);
	}
	if (read_nMPS() == 1){
		shutoffFlagsSet = osEventFlagsSet(shutoffFlagHandle, nMPS_FLAG | SHUTOFF_FLAG);
	}

	if (read_ESD() == 1){
		shutoffFlagsSet = osEventFlagsSet(shutoffFlagHandle, ESD_FLAG | SHUTOFF_FLAG);
	}


}

void updateTripStatus() {
	// checking for high current trips
	if (contactorInfo[COMMON].current >= MAX_COMMON_CONTACTOR_CURRENT){
		mbmsTrip.commonHighCurrentTrip = 1;
	}
	if ((contactorInfo[MOTOR].current >= MAX_MOTORS_CONTACTOR_CURRENT)){
		mbmsTrip.motorHighCurrentTrip = 1;
	}
	if (contactorInfo[ARRAY].current >= MAX_ARRAY_CONTACTOR_CURRENT){
		mbmsTrip.arrayHighCurrentTrip = 1;
	}
	if (contactorInfo[LOWV].current >= MAX_LV_CONTACTOR_CURRENT){
		mbmsTrip.LVHighCurrentTrip = 1;
	}
	if (contactorInfo[CHARGE].current >= MAX_CHARGE_CONTACTOR_CURRENT){
		mbmsTrip.chargeHighCurrentTrip = 1;
	}

	// checking for high/low cell voltage trips
	if(batteryInfo.maxCellVoltage >= MAX_CELL_VOLTAGE){
		mbmsTrip.highCellVoltageTrip = 1;
	}
	if(batteryInfo.minCellVoltage <= MIN_CELL_VOLTAGE) {
		mbmsTrip.lowCellVoltageTrip = 1;
	}

	// checking for Protection Trip
	// CHECK THIS !!!!!!!!
	if ((contactorInfo[CHARGE].current < 0) || (contactorInfo[LOWV].current < 0) || (contactorInfo[ARRAY].current < 0) || (contactorInfo[COMMON].current < 0)){
		mbmsTrip.protectionTrip = 1;
	}

	// if orion can message wasn't received recently, set trip
	if (!(mbmsStatus.orionCANReceived)) {
		mbmsTrip.orionMessageTimeoutTrip = 1;
	}


	// checking for Contactor Connected/Disconnected Unexpectedly Trip
	/* To check, we compare a minimum current draw with the state of the contactor */
	if(((		 contactorCommand.common == CLOSE_CONTACTOR) && (contactorInfo[COMMON].current < NO_CURRENT_THRESHOLD))
			|| ((contactorCommand.motor == CLOSE_CONTACTOR) && (contactorInfo[MOTOR].current < NO_CURRENT_THRESHOLD))
			|| ((contactorCommand.array  == CLOSE_CONTACTOR) && (contactorInfo[ARRAY].current  < NO_CURRENT_THRESHOLD))
			|| ((contactorCommand.LV     == CLOSE_CONTACTOR) && (contactorInfo[LOWV].current   < NO_CURRENT_THRESHOLD))
			|| ((contactorCommand.charge == CLOSE_CONTACTOR) && (contactorInfo[CHARGE].current < NO_CURRENT_THRESHOLD))
		)
	{
		// if supposed to be closed but theres no current (means its unexpectedly opened/disconnected
		mbmsTrip.contactorDisconnectedUnexpectedlyTrip = 1;
	}
	if(((		 contactorCommand.common == OPEN_CONTACTOR) && (contactorInfo[COMMON].current >= NO_CURRENT_THRESHOLD))
			|| ((contactorCommand.motor == OPEN_CONTACTOR) && (contactorInfo[MOTOR].current >= NO_CURRENT_THRESHOLD))
			|| ((contactorCommand.array  == OPEN_CONTACTOR) && (contactorInfo[ARRAY].current  >= NO_CURRENT_THRESHOLD))
			|| ((contactorCommand.LV     == OPEN_CONTACTOR) && (contactorInfo[LOWV].current   >= NO_CURRENT_THRESHOLD))
			|| ((contactorCommand.charge == OPEN_CONTACTOR) && (contactorInfo[CHARGE].current >= NO_CURRENT_THRESHOLD))
		)
	{
		// if supposed to be closed but theres no current (means its unexpectedly opened/disconnected
		mbmsTrip.contactorConnectedUnexpectedlyTrip = 1;
	}

	// checking for high battery trip (voltage?)
	if (batteryInfo.packVoltage > MAX_PACK_VOLTAGE) {
		mbmsTrip.highBatteryTrip = 1;
	}

	// Dead contactor heartbeat trips are done in a diff function

	if(read_nMPS() == 1){
		mbmsTrip.MPSDisabledTrip = 1;
	}

	if(read_ESD() == 1){
		mbmsTrip.ESDEnabledTrip = 1;
	}


}

// maybe change name of ths lol oop
void updatePackInfoStruct() {
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

		if (orionMsg.extendedID == PACKINFOID) {
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
		else if (orionMsg.extendedID == TEMPINFOID) {
			batteryInfo.highTemp = data[0];
			batteryInfo.lowTemp = data[2];
			batteryInfo.avgTemp = data[4];
		}

		else if (orionMsg.extendedID == MAXMINVOLTAGESID) {
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

// motor and array cont6actors might still need to open or close during.....
// but i think LV and common can just stay closed ... ?
// also should set flag in this function for shutdown procedure in case anything goes wrong



// gatekeeper tasks should be letting battery tasks know if it should close/open contactor
// ofc battery task should check w permissions setup from startup AND var1 (if shutdown procedure should
// occur, bc if so, shouldn't be able to close more contactors)








