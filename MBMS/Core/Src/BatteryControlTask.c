/*
 * BatteryControlTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millainel
 */

// look at orion interface task from old code to see what they did

#include "BatteryControlTask.h"
#include <stdint.h>
#include "cmsis_os.h"
#include "CANdefines.h"
#include "StartupTask.h"
#include "ShutoffTask.h"
#include "ReadPowerGPIO.h"
#include "MBMS.h"

//ContactorState contactorState = {0};
MBMSSoftBatteryLimitWarning mbmsSoftBatteryLimitWarning;
BatteryInfo batteryInfo;
MBMSStatus mbmsStatus;
MBMSTrip mbmsTrip;
ContactorCommand contactorCommand;
PowerSelectionStatus powerSelectionStatus;
ContactorInfo contactorInfo[6]; // one for each contactor        add volatile to the extern thing too
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

	checkSoftBatteryLimit();

	/* Check for trips based on information at hand */
	updateTripStatus();

	checkIfShutdown();



	//updateContactors();

	// NEED TO CHECK CURRENT STUFF STILL.... need to do all the trip stuff lol, also check attributes of can msg for sumn idk

}
uint8_t waitForFirstHeartbeats() {
	static uint8_t heartbeatFailCounter[5] = {0};
	uint8_t dead = 0;


	for(int i = 0; i < 5; i++) {
		if(heartbeatFailCounter[i] > 3) {
			osStatus_t a = osMutexAcquire(MBMSTripMutexHandle, 200);
			if(a == osOK) {
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
				osStatus_t r = osMutexRelease(MBMSTripMutexHandle);

			}

			// MAYBE STRAIGHT UP SET SHUTDOWN FLAG HERE ! um maybe not #modularity or sumn
			dead = 1;
			return dead;

		}
		if(previousHeartbeats[i] >= 65535) { // check this logic lol
			previousHeartbeats[i] = 0;
		}
		osStatus_t a = osMutexAcquire(ContactorInfoMutexHandle, READING_MUTEX_TIMEOUT);
		if (a == osOK) {
			if(previousHeartbeats[i] >= contactorInfo[i].heartbeat){
				if(((osKernelGetTickCount() - heartbeatLastUpdatedTime[i]) / FREERTOS_TICK_PERIOD) > CONTACTOR_HEARTBEAT_TIMEOUT) { // where contactor_heartbeat_timeout is how often a heartbeat is sent out/recieved
					heartbeatFailCounter[i]++;

				}
			}
			else {
				heartbeatLastUpdatedTime[i] = osKernelGetTickCount();

				previousHeartbeats[i] = contactorInfo[i].heartbeat;
				heartbeatFailCounter[i] = 0;
			}
			osStatus_t r = osMutexRelease(ContactorInfoMutexHandle);

		}


	}
	return dead;

}

void clearAllFaults(){

	osStatus_t acquire = osMutexAcquire(MBMSTripMutexHandle, 200);
	if(acquire == osOK) {
		// now its acquired, can transmit over
		mbmsTrip.highCellVoltageTrip = 0;
		mbmsTrip.lowCellVoltageTrip = 0;
		mbmsTrip.commonHighCurrentTrip = 0;
		mbmsTrip.motorHighCurrentTrip = 0;
		mbmsTrip.arrayHighCurrentTrip = 0;
		mbmsTrip.LVHighCurrentTrip = 0;
		mbmsTrip.chargeHighCurrentTrip = 0;
		mbmsTrip.protectionTrip = 0;
		mbmsTrip.orionMessageTimeoutTrip = 0;
		mbmsTrip.contactorDisconnectedUnexpectedlyTrip = 0;
		mbmsTrip.contactorConnectedUnexpectedlyTrip = 0;
		mbmsTrip.highBatteryTrip = 0;
		mbmsTrip.commonHeartbeatDeadTrip = 0;
		mbmsTrip.motorHeartbeatDeadTrip = 0;
		mbmsTrip.arrayHeartbeatDeadTrip = 0;
		mbmsTrip.LVHeartbeatDeadTrip = 0;
		mbmsTrip.chargeHeartbeatDeadTrip = 0;
		mbmsTrip.MPSDisabledTrip = 0; //
		mbmsTrip.ESDEnabledTrip = 0;
		mbmsTrip.highTemperatureTrip = 0;
		mbmsTrip.lowTemperatureTrip = 0;

		//release the mutex !!!!
		osStatus_t release = osMutexRelease(MBMSTripMutexHandle);
	}

}

/*
 * returns whether any contactors are closed (0) or not (1). want them all to be open on startup
 */
uint8_t checkContactorsOpen() {
	// BUT SHOULD I OPEN THE CONTACTOR IF THEY ARE CLOSED? maybe yeah. btw contactors r non-latching so theyll open if car powers off yk

	uint8_t allOpen = 1;
	osStatus_t acquire = osMutexAcquire(ContactorInfoMutexHandle, 200);
	if (acquire == osOK) {

		for (int i = 0; i < 5; i++) {
			if (contactorInfo[i].contactorClosed == CLOSE_CONTACTOR) {
				allOpen = 0;
				break;
			}
		}

		osStatus_t release = osMutexRelease(ContactorInfoMutexHandle);

	}

	return allOpen;
}

/*
 * returns whether any prechargers are closed (0) or not (1).
 */
uint8_t checkPrechargersOpen() {

	uint8_t allOpen = 1;
	osStatus_t acquire = osMutexAcquire(ContactorInfoMutexHandle, 200);
	if (acquire == osOK) {

		for (int i = 0; i < 5; i++) {
			if (contactorInfo[i].prechargerClosed == CLOSE_CONTACTOR) {
				allOpen = 0;
				break;
			}
		}
		osStatus_t release = osMutexRelease(ContactorInfoMutexHandle);

	}

	return allOpen;

}

uint8_t batteryCheck() {
	// maybe set the flag for hard batt lim, soft batt lim here? or idk loll
	// or just have a var somewhere to keep track and check the var in the check if shutdown stuff?
	// in case theres multiple things wrong so you can store all the trips before yk, doing whatever BPS procedure

	uint8_t safe = 1;
	// check this mutex stuff ngl...
	osStatus_t acquire = osMutexAcquire(MBMSTripMutexHandle, 200);
	if(acquire == osOK) {
		uint8_t safe = 1;

		if(batteryInfo.maxCellVoltage > HARD_MAX_CELL_VOLTAGE){
			// hard battery limit
			mbmsTrip.highCellVoltageTrip = 1;
			safe = 0;
		}

		if(batteryInfo.minCellVoltage < HARD_MIN_CELL_VOLTAGE) {
			// hard battery limit
			mbmsTrip.lowCellVoltageTrip = 1;
			safe = 0;
		}

		//NOTE THIS IS NOT HIGH BATT TRIP !!!! FIX this
		if (batteryInfo.packVoltage > HARD_MAX_PACK_VOLTAGE) {
			// hard battery limit
			mbmsTrip.highBatteryTrip = 1;
			safe = 0;
		}

		//DO TEMP


		//release the mutex !!!!
		osStatus_t release = osMutexRelease(MBMSTripMutexHandle);
	}

	return safe;

}


/*
 * This function should be called during the startup procedure
 * It goes through all the checks needed on startup, such as contactor heartbeats, contactors open,
 * and battery state (voltages and temperatures)
 * (Don't need to check current beacuse the contactors should be open anyways..)
 */
void startupCheck(){

	/* Waiting for contactor heartbeats */
	// idk this feels kinda sketchy, maybe look at it ...
	uint8_t heartbeatDead = 0;
	while ((previousHeartbeats[0] == 0) || (previousHeartbeats[1] == 0) || (previousHeartbeats[2] == 0) ||
		   (previousHeartbeats[3] == 0) || (previousHeartbeats[4] == 0) || (heartbeatDead != 0))
	{
		// maybe add an osDelay so a diff task can run? CAN Rx.. to actually receive heartbeat messages, and also the one that sets trips and does shutdown idk :(
        //updateContactorInfoStruct(); // YAY DONT NEED THIS ANYMORE CUZ u moved the updating to a diff task phew
		heartbeatDead = waitForFirstHeartbeats();
		// set heartbeatDead so we can break out of while loop lol
	}
	if (heartbeatDead == 1){
		// bps fault
		initiateBPSFault();
	}

	/* Check to ensure no contactors are closed */
	if ((checkContactorsOpen() == 0) || checkPrechargersOpen() == 0){
		//bps fault
		initiateBPSFault();
	}

	/* Battery check (orion) */
	uint8_t passedBatteryCheck = batteryCheck();
	if (!passedBatteryCheck) {
		initiateBPSFault();
	}
}

/*
 * This function runs when a BPS Fault should occur
 * It turns on the strobe light, and changes the mbms status
 * Should set a flag for shutdown...
 */
void initiateBPSFault() {
	// strpbe enable
	HAL_GPIO_WritePin(Strobe_En_GPIO_Port, Strobe_En_Pin, 1);

	osStatus_t a = osMutexAcquire(MBMSStatusMutexHandle, 200);
	if(a == osOK) {
		// update mbms status
		mbmsStatus.strobeBMSLight = 1;
		osStatus_t r = osMutexRelease(MBMSStatusMutexHandle);

	}

	osEventFlagsSet(shutoffFlagHandle, (HARD_BL_FLAG | SHUTOFF_FLAG));
	// idk if soft battery limit has any purpose in shutoff procedure anymore, since when i talked
	// to jenny today, she said soft battery limit should just be a warning thru CAN and thats it.... may 10

}

/*
 * This function checks that all the contactor heartbeats are still being received
 * If they are not, a contactor has possibly died and a trip should occur which should initiate
 * a BPS Fault !
 * The way I did BPS Fault rn is that it iterates through every contactor before going to BPS
 * that way the trips will track all dead ones (not just the first one....)
 * But honestly I think it would be okay to just call it right after the switch case directly.. idk
 */
void checkContactorHeartbeats() {
	/* The reason i'm checking heartbeats this way and not with a timeout for the message queue (like orion)
	 * is because there's multiple contactor boards all sending their heartbeats so the timeout wouldn't
	 * be accurate of what the problem is, or if there is a problem. For example if a contactor dies, other
	 * contactor would still be sending messages.
	 */
	static uint8_t BPSFault = 0;
	for(int i = 0; i < 5; i++) {
		if(previousHeartbeats[i] >= 65535) { // check this logic lol
			previousHeartbeats[i] = 0;
		}


		if(previousHeartbeats[i] >= contactorInfo[i].heartbeat){
			if(((osKernelGetTickCount() - heartbeatLastUpdatedTime[i]) / FREERTOS_TICK_PERIOD) > CONTACTOR_HEARTBEAT_TIMEOUT) {

				osStatus_t acquire = osMutexAcquire(MBMSTripMutexHandle, UPDATING_MUTEX_TIMEOUT);
				if(acquire == osOK) {
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
					osStatus_t release = osMutexRelease(MBMSTripMutexHandle);
					BPSFault = 1;

				}
			}
		}
		else {
			heartbeatLastUpdatedTime[i] = osKernelGetTickCount();
			osStatus_t a = osMutexAcquire(ContactorInfoMutexHandle, READING_MUTEX_TIMEOUT);
			if (a == osOK) {
				previousHeartbeats[i] = contactorInfo[i].heartbeat;
				osStatus_t r = osMutexRelease(ContactorInfoMutexHandle);

			}

		}

	}

	if(BPSFault) {
		initiateBPSFault();
	}
}

// this stuff is diff now i think
void checkIfShutdown() {
	/* Update System status */
	if(tripData != 0) { // was og written as tripStruct by violet
		/* go to shutdown */
		/* Update contactor permission to none or less based on type of shutdown */

	}
	// check key and mps and hard and soft batt limit to possibly set shutdown flags !!!!
	uint32_t shutoffFlagsSet;
	if (read_Key() == KEY_OFF){
		shutoffFlagsSet = osEventFlagsSet(shutoffFlagHandle, KEY_FLAG | SHUTOFF_FLAG);
	}
	if (read_nMPS() == 1){
		shutoffFlagsSet = osEventFlagsSet(shutoffFlagHandle, nMPS_FLAG | SHUTOFF_FLAG);
	}

	if (read_ESD() == 1){
		shutoffFlagsSet = osEventFlagsSet(shutoffFlagHandle, ESD_FLAG | SHUTOFF_FLAG);
	}

}


/*
void switchChargerState() {
	//dequeue CAN message to see if charger is plugged in or not???
	// lets just call the var plugged for now
	uint8_t NOT_CHARGING = 0; // putting temp var here rn cuz i commented out enum for now lol may 17

	uint8_t plugged = 0; // UPDATE THIS WHEN DYLAN/JENNY FIGURE OUT HOW MBMS KNOWS CHARGER IS PLUGGED IN

	static uint8_t chargeState = NOT_CHARGING;
	if(plugged && (chargeState == NOT_CHARGING) && (read_Charge_Enable() == 1)) {
		//open motor contactor
		contactorCommand.motor = OPEN_CONTACTOR;
		while(contactorInfo[MOTOR].contactorClosed == CLOSE_CONTACTOR){
			// wait to open
			// CANMessageSender task will run and send the command
			// ValueUpdater task will update contactorInfo[MOTOR]
		}
		// disable 12V CAN to save power consumption..
		HAL_GPIO_WritePin(_12V_CAN_En_GPIO_Port, _12V_CAN_En_Pin, GPIO_PIN_RESET);
		// open LV contactor
		contactorCommand.LV = OPEN_CONTACTOR;
		while(contactorInfo[LOWV].contactorClosed == CLOSE_CONTACTOR){
			// wait to open
		}
		// enable CHG_LV_En to allow charging
		HAL_GPIO_WritePin(nCHG_LV_En_GPIO_Port, nCHG_LV_En_Pin, GPIO_PIN_RESET);
		// close charge contactor to start charging
		contactorCommand.charge = CLOSE_CONTACTOR;
		while(contactorInfo[CHARGE].contactorClosed == OPEN_CONTACTOR){
			// wait to close
		}
		// update charge state
		chargeState = CHARGING;
	}

	else if (!plugged && (chargeState == CHARGING) && (read_Discharge_Enable() == 1)
					  && (mbmsStatus.startupState == FULLY_OPERATIONAL))
	{
		// open charge contactor
		contactorCommand.charge = OPEN_CONTACTOR;
		while(contactorInfo[CHARGE].contactorClosed == CLOSE_CONTACTOR){
			// wait to open
		}
		// close LV Contactor
		contactorCommand.LV = CLOSE_CONTACTOR;
		while(contactorInfo[LOWV].contactorClosed == OPEN_CONTACTOR){
			// wait to close
		}
		// open 12V Charge through nCHG_En
		HAL_GPIO_WritePin(nCHG_LV_En_GPIO_Port, nCHG_LV_En_Pin, GPIO_PIN_SET);

		// enable 12V CAN
		HAL_GPIO_WritePin(_12V_CAN_En_GPIO_Port, _12V_CAN_En_Pin, GPIO_PIN_SET);

		// close motor contactor
		contactorCommand.motor = CLOSE_CONTACTOR;
		while(contactorInfo[MOTOR].contactorClosed == OPEN_CONTACTOR){
			// wait to close
		}
		// update charge state
		chargeState = NOT_CHARGING;

	}

	// I'll just check for shutdown during charging here as well..
	if ((chargeState == CHARGING) && (read_Key() == 0)) {
		// turn off charge LV enable to shutoff car..
		HAL_GPIO_WritePin(nCHG_LV_En_GPIO_Port, nCHG_LV_En_Pin, GPIO_PIN_SET);

	}

	// go to can message sender and change the contactor stuff cuz contactors should only change here, or w mps and bps stuff etc..
}
*/


/* This function checks the soft limits of voltages, currents, and temperatures
 * These warnings should be sent out in a CAN message (CANMessageSender -> CANTxGatekeeper)
 * You can think of these like a warning on your phone that it's low battery
 * Don't need to do anything for these soft limits, just send the warning!
 */
void checkSoftBatteryLimit() {
	/// ummmmm be careful deadlock mauybe check everything once ur done all the mutexes
	osStatus_t acquire = osMutexAcquire(MBMSSoftLimitWarningMutexHandle, 200);
	if(acquire == osOK) {

		osStatus_t a1 = osMutexAcquire(BatteryInfoMutexHandle, 200);
		if (a1 == osOK){
			/* Checking the min/max cell voltages */
			if (batteryInfo.maxCellVoltage > SOFT_MAX_CELL_VOLTAGE) {
				mbmsSoftBatteryLimitWarning.highCellVoltageWarning = 1;
			}
			if (batteryInfo.minCellVoltage < SOFT_MIN_CELL_VOLTAGE) {
				mbmsSoftBatteryLimitWarning.lowCellVoltageWarning = 1;
			}

			/* Checking high/low temperature */
			if (batteryInfo.highTemp > SOFT_MAX_TEMP) {
				mbmsSoftBatteryLimitWarning.highTemperatureWarning = 1;
			}
			if (batteryInfo.lowTemp < SOFT_MIN_TEMP) {
				mbmsSoftBatteryLimitWarning.lowTemperatureWarning = 1;
			}

			osStatus_t r1 = osMutexRelease(BatteryInfoMutexHandle);

		}

		osStatus_t a2 = osMutexAcquire(ContactorInfoMutexHandle, 200);
		if (a2 == osOK) {
			/* Checking contactors' high current */
			if (contactorInfo[COMMON].lineCurrent > SOFT_MAX_COMMON_CONTACTOR_CURRENT){
				mbmsSoftBatteryLimitWarning.commonHighCurrentWarning = 1;
			}
			if (contactorInfo[MOTOR].lineCurrent > SOFT_MAX_MOTORS_CONTACTOR_CURRENT){
				mbmsSoftBatteryLimitWarning.motorHighCurrentWarning = 1;
			}
			if (contactorInfo[ARRAY].lineCurrent > SOFT_MAX_ARRAY_CONTACTOR_CURRENT){
				mbmsSoftBatteryLimitWarning.arrayHighCurrentWarning = 1;
			}
			if (contactorInfo[LOWV].lineCurrent > SOFT_MAX_LV_CONTACTOR_CURRENT){
				mbmsSoftBatteryLimitWarning.LVHighCurrentWarning = 1;
			}
			if (contactorInfo[CHARGE].lineCurrent > SOFT_MAX_CHARGE_CONTACTOR_CURRENT){
				mbmsSoftBatteryLimitWarning.chargeHighCurrentWarning = 1;
			}
			osStatus_t r2 = osMutexRelease(ContactorInfoMutexHandle);
		}

		//release mutex
		osStatus_t release = osMutexRelease(MBMSSoftLimitWarningMutexHandle);
	}

}

void updateTripStatus() {

	static uint8_t BPS_Fault = 0;
	osStatus_t acquire = osMutexAcquire(MBMSTripMutexHandle, 200);
	if (acquire == osOK){

		osStatus_t a1 = osMutexAcquire(ContactorInfoMutexHandle, 200);
		if (a1 == osOK){
			// checking for high current trips
			if (contactorInfo[COMMON].lineCurrent > HARD_MAX_COMMON_CONTACTOR_CURRENT){
				mbmsTrip.commonHighCurrentTrip = 1;
			}

			if ((contactorInfo[MOTOR].lineCurrent > HARD_MAX_MOTORS_CONTACTOR_CURRENT)){
				mbmsTrip.motorHighCurrentTrip = 1;
			}

			if (contactorInfo[ARRAY].lineCurrent > HARD_MAX_ARRAY_CONTACTOR_CURRENT){
				mbmsTrip.arrayHighCurrentTrip = 1;
			}

			if (contactorInfo[LOWV].lineCurrent > HARD_MAX_LV_CONTACTOR_CURRENT){
				mbmsTrip.LVHighCurrentTrip = 1;
			}


			if (contactorInfo[CHARGE].lineCurrent > HARD_MAX_CHARGE_CONTACTOR_CURRENT){
				mbmsTrip.chargeHighCurrentTrip = 1;
			}

			// checking for Protection Trip
			// CHECK THIS !! but  its made from april 5 notes
			if ((contactorInfo[CHARGE].lineCurrent > 0) || (contactorInfo[LOWV].lineCurrent < 0)){
				mbmsTrip.protectionTrip = 1;
				BPS_Fault = 1;
			}

			osStatus_t r1 = osMutexRelease(ContactorInfoMutexHandle);

		}

		osStatus_t a2 = osMutexAcquire(BatteryInfoMutexHandle, 200);
		if (a2 == osOK){
			// checking for high/low cell voltage trips
			if(batteryInfo.maxCellVoltage > HARD_MAX_CELL_VOLTAGE){
				mbmsTrip.highCellVoltageTrip = 1;
				BPS_Fault = 1;
			}

			if(batteryInfo.minCellVoltage < HARD_MIN_CELL_VOLTAGE) {
				mbmsTrip.lowCellVoltageTrip = 1;
				BPS_Fault = 1;
			}


			/* Checking high/low temperature */
			if (batteryInfo.highTemp > HARD_MAX_TEMP) {
				mbmsTrip.highTemperatureTrip = 1;
				BPS_Fault = 1;
			}
			if(batteryInfo.lowTemp < HARD_MIN_TEMP) {
				mbmsTrip.lowTemperatureTrip = 1;
				BPS_Fault = 1;
			}

			// checking for high battery trip (voltage?) THIS IS NOT PACK VOLTAGE!!! redo this
			if (batteryInfo.packVoltage > MAX_PACK_VOLTAGE) {
				mbmsTrip.highBatteryTrip = 1;
				BPS_Fault = 1;
			}

			osStatus_t r2 = osMutexRelease(BatteryInfoMutexHandle);

		}

		osStatus_t a3 = osMutexAcquire(MBMSStatusMutexHandle, 200);
		if (a3 == osOK) {
			// if orion can message wasn't received recently, set trip
			if (!(mbmsStatus.orionCANReceived)) {
				mbmsTrip.orionMessageTimeoutTrip = 1;
				BPS_Fault = 1;
			}

			osStatus_t r3 = osMutexRelease(MBMSStatusMutexHandle);

		}

		osStatus_t a4 = osMutexAcquire(ContactorCommandMutexHandle, 200);
		if(a4 == osOK) {
			// checking for Contactor Connected/Disconnected Unexpectedly Trip
			/* To check, we compare a minimum current draw with the state of the contactor */
			if(((		 contactorCommand.common == CLOSE_CONTACTOR) && (contactorInfo[COMMON].lineCurrent < NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.motor == CLOSE_CONTACTOR) && (contactorInfo[MOTOR].lineCurrent < NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.array  == CLOSE_CONTACTOR) && (contactorInfo[ARRAY].lineCurrent  < NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.LV     == CLOSE_CONTACTOR) && (contactorInfo[LOWV].lineCurrent   < NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.charge == CLOSE_CONTACTOR) && (contactorInfo[CHARGE].lineCurrent < NO_CURRENT_THRESHOLD))
				)
			{
				// if supposed to be closed but theres no current (means its unexpectedly opened/disconnected
				mbmsTrip.contactorDisconnectedUnexpectedlyTrip = 1;
				BPS_Fault = 1;
			}
			if(((		 contactorCommand.common == OPEN_CONTACTOR) && (contactorInfo[COMMON].lineCurrent >= NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.motor == OPEN_CONTACTOR) && (contactorInfo[MOTOR].lineCurrent >= NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.array  == OPEN_CONTACTOR) && (contactorInfo[ARRAY].lineCurrent  >= NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.LV     == OPEN_CONTACTOR) && (contactorInfo[LOWV].lineCurrent   >= NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.charge == OPEN_CONTACTOR) && (contactorInfo[CHARGE].lineCurrent >= NO_CURRENT_THRESHOLD))
				)
			{
				// if supposed to be closed but theres no current (means its unexpectedly opened/disconnected
				mbmsTrip.contactorConnectedUnexpectedlyTrip = 1;
				BPS_Fault = 1;
			}

			osStatus_t r4 = osMutexRelease(ContactorCommandMutexHandle);

		}


		// Dead contactor heartbeat trips are done in a diff function

		if(read_nMPS() == 1){
			mbmsTrip.MPSDisabledTrip = 1;

		}

		if(read_ESD() == 1){
			mbmsTrip.ESDEnabledTrip = 1;
			BPS_Fault = 1;
		}

		// rlrease trip mutex
		osStatus_t release = osMutexRelease(MBMSTripMutexHandle);

		if(BPS_Fault) {
			initiateBPSFault();
		}
	}


}

/*
 * Saturday May 17
 * Redoing things
 * Note: Maybe make the perms a struct instead of flags lol... so its easier to give & take
 */

// move this elsehwere, just here for now to keep things easily readable
Permissions perms = {0};

// make var plugged for now

// the functions that update battery info struct and contactor info array structs!!! (from valueUpdater)

// need to fix enums that have same name... but only if this is good ish ..
void func() {
	uint8_t plugged = 0;
	uint8_t carState = STARTUP;
	switch (carState) {
		case STARTUP:
			if(read_Discharge_Enable() == 1){
				perms.common = 1;
				perms.motor = 1;
				perms.lv = 1;

			}
			if(read_Charge_Enable() == 1) {
				perms.array = 1;
			}

			//perms.charge = 1; NAW NOT CHARGE NOT HERE assuming perms means u can close it
			if (mbmsStatus.startupState == FULLY_OPERATIONAL){ // maybe change this to COMPLETED so its diff
				carState = FULLY_OPERATIONAL;
			}
			break;
		case FULLY_OPERATIONAL:
			if (plugged && (read_Charge_Enable() == 1)) {
				perms.lv = 0;
				perms.motor = 0;
				HAL_GPIO_WritePin(_12V_CAN_En_GPIO_Port, _12V_CAN_En_Pin, GPIO_PIN_RESET); // disable 12V CAN
			}
			if((contactorInfo[LOWV].contactorClosed == OPEN_CONTACTOR) && (contactorInfo[MOTOR].contactorClosed == OPEN_CONTACTOR)) {
				HAL_GPIO_WritePin(nCHG_LV_En_GPIO_Port, nCHG_LV_En_Pin, GPIO_PIN_RESET); // enable charging
				perms.charge = 1;
			}
			if(contactorInfo[CHARGE].contactorClosed == CLOSE_CONTACTOR) {
				carState = CHARGING;
			}
			// if discharge En no longer enabled then...
			// open the motor, lv, contactors??? idk.. idk..

			break;
		case CHARGING:
			if (!plugged && (read_Discharge_Enable() == 1)) {
				HAL_GPIO_WritePin(nCHG_LV_En_GPIO_Port, nCHG_LV_En_Pin, GPIO_PIN_SET); // disable charging
				perms.charge = 0;
			}
			if(contactorInfo[CHARGE].contactorClosed == OPEN_CONTACTOR) {
				HAL_GPIO_WritePin(_12V_CAN_En_GPIO_Port, _12V_CAN_En_Pin, GPIO_PIN_SET); // enable 12V CAN
				perms.lv = 1;
				perms.motor = 1;
			}
			if((contactorInfo[LOWV].contactorClosed == CLOSE_CONTACTOR) && (contactorInfo[MOTOR].contactorClosed == CLOSE_CONTACTOR)) {
				carState = FULLY_OPERATIONAL;
			}
			// if charge En no longer enabled then...
			// open the charge, array, contactors??? idk.. idk
			break;
		case BPS_FAULT: // SET TO THIS CASE for all the checking stuffs :3
			// idk does this open them all at same time.... sus how to do specific order? and for shutdown....
			HAL_GPIO_WritePin(Strobe_En_GPIO_Port, Strobe_En_Pin, GPIO_PIN_SET);
			mbmsStatus.strobeBMSLight = 1;
			//set flag for shutdwon procedure.... ? mm idk

			perms.common = 0;
			perms.lv = 0;
			perms.motor = 0;
			perms. array = 0;
			perms.charge = 0;
			break;
		case MPS_DISCONNECTED:
			// reset all the variables so we can restart at startup...?

	}


}

void updateContactors() {


    uint8_t sendContactorCommand = 0;
    static uint8_t contactorClosing = false;

    // Check if any contactors are currently closing
    for (int i = 0; i < 5; i++) {
        if (contactorInfo[i].contactorClosing == CLOSING_CONTACTOR) { // lowkey switch this back to an enum if u have time smh.. stoopid fr
            contactorClosing = true;
            break;
        }
    }

    // If no contactors are currently closing, you may close a contactor
    if (!contactorClosing) {
        if ((perms.common) && (contactorInfo[COMMON].contactorClosed != CLOSE_CONTACTOR)) {
            contactorCommand.common = CLOSE_CONTACTOR;
//            sendContactorCommand = 1; // had this here before but i think ill just consistently send lowkey..
        }
        else if ((perms.lv) && (contactorInfo[LOWV].contactorClosed != CLOSE_CONTACTOR) && (mbmsStatus.allowDischarge == 1)) {
            contactorCommand.LV = CLOSE_CONTACTOR;

        }
        else if ((perms.motor) && (contactorInfo[MOTOR].contactorClosed != CLOSE_CONTACTOR) && (mbmsStatus.allowDischarge == 1)) {
            contactorCommand.motor = CLOSE_CONTACTOR;

        }
        else if ((perms.array) && (contactorInfo[ARRAY].contactorClosed != CLOSE_CONTACTOR) && (mbmsStatus.allowCharge == 1)) {
            contactorCommand.array = CLOSE_CONTACTOR;

        }
        else if ((perms.charge) && (contactorInfo[CHARGE].contactorClosed != CLOSE_CONTACTOR) && (mbmsStatus.allowCharge == 1)) {
            contactorCommand.charge = CLOSE_CONTACTOR;

        }
    }

    // Open contactors as needed
    if ((!perms.motor) && (contactorInfo[MOTOR].contactorClosed != OPEN_CONTACTOR)) {
        contactorCommand.motor = OPEN_CONTACTOR;
//        sendContactorCommand = 1;
    }

    if ((!perms.array) && (contactorInfo[ARRAY].contactorClosed != OPEN_CONTACTOR)) {
        contactorCommand.array = OPEN_CONTACTOR;

    }

    if ((!perms.lv) && (contactorInfo[LOWV].contactorClosed != OPEN_CONTACTOR)) {
        contactorCommand.LV = OPEN_CONTACTOR;

    }

    if ((!perms.charge) && (contactorInfo[CHARGE].contactorClosed != OPEN_CONTACTOR)) {
        contactorCommand.charge = OPEN_CONTACTOR;

    }
}


// motor and array cont6actors might still need to open or close during.....
// but i think LV and common can just stay closed ... ?
// also should set flag in this function for shutdown procedure in case anything goes wrong



// gatekeeper tasks should be letting battery tasks know if it should close/open contactor
// ofc battery task should check w permissions setup from startup AND var1 (if shutdown procedure should
// occur, bc if so, shouldn't be able to close more contactors)








