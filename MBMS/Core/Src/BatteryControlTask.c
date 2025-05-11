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
			dead = 1;
			return dead;

		}
		if(previousHeartbeats[i] >= 65535) { // check this logic lol
			previousHeartbeats[i] = 0;
		}
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
	}
	return dead;

}

/*
 * returns whether any contactors are closed (0) or not (1). want them all to be open on startup
 */
uint8_t checkContactorsOpen() {
	// BUT SHOULD I OPEN THE CONTACTOR IF THEY ARE CLOSED? maybe yeah. btw contactors r non-latching so theyll open if car powers off yk
	uint8_t allOpen = 1;
	for (int i = 0; i < 5; i++) {
		if (contactorInfo[i].contactorClosed == CLOSE_CONTACTOR) {
			allOpen = 0;
			break;
		}
	}
	return allOpen;
}

/*
 * returns whether any prechargers are closed (0) or not (1).
 */
uint8_t checkPrechargersOpen() {
	uint8_t allOpen = 1;
	for (int i = 0; i < 5; i++) {
		if (contactorInfo[i].prechargerClosed == CLOSE_CONTACTOR) {
			allOpen = 0;
			break;
		}
	}
	return allOpen;

}

uint8_t batteryCheck() {
	// maybe set the flag for hard batt lim, soft batt lim here? or idk loll
	// or just have a var somewhere to keep track and check the var in the check if shutdown stuff?
	// in case theres multiple things wrong so you can store all the trips before yk, doing whatever BPS procedure

	uint8_t safe = 1;

	if(batteryInfo.maxCellVoltage > HARD_MAX_CELL_VOLTAGE){
		// hard battery limit
		mbmsTrip.highCellVoltageTrip = 1;
		safe = 0;
	}
	else if (batteryInfo.maxCellVoltage > SOFT_MAX_CELL_VOLTAGE){
		// soft battery limit
		mbmsTrip.highCellVoltageTrip = 1;
		safe = 0;
	}
	if(batteryInfo.minCellVoltage < HARD_MIN_CELL_VOLTAGE) {
		// hard battery limit
		mbmsTrip.lowCellVoltageTrip = 1;
		safe = 0;
	}
	else if (batteryInfo.minCellVoltage < SOFT_MIN_CELL_VOLTAGE){
		// soft battery limit
		mbmsTrip.lowCellVoltageTrip = 1;
		safe = 0;
	}

	if (batteryInfo.packVoltage > HARD_MAX_PACK_VOLTAGE) {
		// hard battery limit
		mbmsTrip.highBatteryTrip = 1;
		safe = 0;
	}
	else if (batteryInfo.packVoltage > SOFT_MAX_PACK_VOLTAGE) {
		// soft battery limit
		mbmsTrip.highBatteryTrip = 1;
		safe = 0;
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
	// update mbms status
	mbmsStatus.strobeBMSLight = 1;
	osEventFlagsSet(shutoffFlagHandle, (HARD_BL_FLAG | SHUTOFF_FLAG));
	// idk if soft battery limit has any purpose in shutoff procedure anymore, since when i talked
	// to jenny today, she said soft battery limit should just be a warning thru CAN and thats it.... may 10

}

/*
 * This function checks that all the contactor heartbeats are still being received
 * If they are not, a contactor has possibly died and a trip should occur which should initiate
 * a BPS Fault !
 */
void checkContactorHeartbeats() {
	/* The reason i'm checking heartbeats this way and not with a timeout for the message queue (like orion)
	 * is because there's multiple contactor boards all sending their heartbeats so the timeout wouldn't
	 * be accurate of what the problem is, or if there is a problem. For example if a contactor dies, other
	 * contactor would still be sending messages.
	 */

	for(int i = 0; i < 5; i++) {
		if(previousHeartbeats[i] >= 65535) { // check this logic lol
			previousHeartbeats[i] = 0;
		}
		if(previousHeartbeats[i] >= contactorInfo[i].heartbeat){
			if(((osKernelGetTickCount() - heartbeatLastUpdatedTime[i]) / FREERTOS_TICK_PERIOD) > CONTACTOR_HEARTBEAT_TIMEOUT) {

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



void checkChargerState() {
	//dequeue CAN message to see if charger is plugged in or not???

}

/* This function checks the soft limits of voltages, currents, and temperatures
 * These warnings should be sent out in a CAN message (CANMessageSender -> CANTxGatekeeper)
 * You can think of these like a warning on your phone that it's low battery
 * Don't need to do anything for these soft limits, just send the warning!
 */
void checkSoftBatteryLimit() {

	/* Checking the min/max cell voltages */
	if (batteryInfo.maxCellVoltage > SOFT_MAX_CELL_VOLTAGE) {
		mbmsSoftBatteryLimitWarning.highCellVoltageWarning = 1;
	}
	if (batteryInfo.minCellVoltage < SOFT_MIN_CELL_VOLTAGE) {
		mbmsSoftBatteryLimitWarning.lowCellVoltageWarning = 1;
	}

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

	/* Checking high/low temperature */
	if (batteryInfo.highTemp > SOFT_MAX_TEMP) {
		mbmsSoftBatteryLimitWarning.highTemperatureWarning = 1;
	}
	if (batteryInfo.lowTemp < SOFT_MIN_TEMP) {
		mbmsSoftBatteryLimitWarning.lowTemperatureWarning = 1;
	}

}

void updateTripStatus() {
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


	// checking for high/low cell voltage trips
	if(batteryInfo.maxCellVoltage > HARD_MAX_CELL_VOLTAGE){
		mbmsTrip.highCellVoltageTrip = 1;
	}

	if(batteryInfo.minCellVoltage < HARD_MIN_CELL_VOLTAGE) {
		mbmsTrip.lowCellVoltageTrip = 1;
	}


	/* Checking high/low temperature */
	if (batteryInfo.highTemp > HARD_MAX_TEMP) {
		mbmsTrip.highTemperatureTrip = 1;
	}
	if(batteryInfo.lowTemp < HARD_MIN_TEMP) {
		mbmsTrip.lowTemperatureTrip = 1;
	}

	// checking for Protection Trip
	// CHECK THIS !!!!!!!!
	if ((contactorInfo[CHARGE].lineCurrent < 0) || (contactorInfo[LOWV].lineCurrent < 0) || (contactorInfo[ARRAY].lineCurrent < 0) || (contactorInfo[COMMON].lineCurrent < 0)){
		mbmsTrip.protectionTrip = 1;
	}

	// if orion can message wasn't received recently, set trip
	if (!(mbmsStatus.orionCANReceived)) {
		mbmsTrip.orionMessageTimeoutTrip = 1;
	}


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



// motor and array cont6actors might still need to open or close during.....
// but i think LV and common can just stay closed ... ?
// also should set flag in this function for shutdown procedure in case anything goes wrong



// gatekeeper tasks should be letting battery tasks know if it should close/open contactor
// ofc battery task should check w permissions setup from startup AND var1 (if shutdown procedure should
// occur, bc if so, shouldn't be able to close more contactors)








