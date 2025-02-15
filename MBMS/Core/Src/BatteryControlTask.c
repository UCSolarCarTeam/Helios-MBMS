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
// Define boolean as an enum or typedef in a header
typedef enum { false = 0, true = 1 } boolean;

ContactorState contactorState = {0};

BatteryInfo batteryInfo;

MBMSStatus mbmsStatus;

MBMSTrip mbmsTrip;

ContactorCommand contactorCommand;

ContactorInfo contactorInfo[6]; // one for each contactor

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

// if something goes wrong set event flag, shutdown has a wait event flag, and will shutdown when its set ...

// maybe use a message queue to send a number corresponding to a contactor from STARTUP
// battery control recieves message queue thing, checks what number it is, and will close respective contactor

// nooooo to above

// startup tasks should just GIVE permission to batterycontrol (for specific contactor), then batterycontrol will check
// if things r chill (orion and stuff idk) then itll close contactor for thing it has permissin for
// should i just use a flag for each contactor ???
// maybe flag to see if contactor is already closed, (so u dont close if alreaady close) only close if things r chill
// does startup only just run onceee????

// make a struct to hold contactor states (open, closed, precharging)
// this struct should be a global var, but only batterycontroltask should be able to write to it!!!!
void BatteryControl()
{

	static uint32_t previousHeartbeat; // compare with struct heartbeat ( the most recent one, check if its greater than the previous heartbeat, if yes, things r good and update previous heartbeat, if not... things r bad, and trip

	osStatus_t status;
	CANMsg orionMsg;
	CANMsg contactorMsg;

	// before u set shutdown flag, check startupState and terminate the task if its not finished... ?

	status = osMessageQueueGet(batteryControlMessageQueueHandle, &orionMsg, NULL, ORION_MSG_WAIT_TIMEOUT *1000);  // Timeout = 0 means non-blocking
	if (status == osOK) {

		mbmsStatus.orionCANReceived = 0;

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

		// checking for trips ?
		if(batteryInfo.maxCellVoltage >= MAX_CELL_VOLTAGE){
			mbmsTrip.highCellVoltageTrip = 1;
		}
		if(batteryInfo.minCellVoltage <= MIN_CELL_VOLTAGE) {
			mbmsTrip.lowCellVoltageTrip = 1;
		}



		// QUESTION: why is there no hard batt lim or soft batt lim trips ????
		// and do we not care abt the other things like batt temp etc.?? theres no trips for those??


	}

	else if (status == osErrorTimeout) // if timeout for orion (no message :0)
    {
		mbmsTrip.orionMessageTimeoutTrip = 1;
        mbmsStatus.orionCANReceived = 0; // no orion message recieved !!!
    }

	// Non blocking check for messages from contactors !
	status = osMessageQueueGet(contactorMessageQueueHandle, &contactorMsg, NULL, 0);
	if (status == osOK) {
		uint8_t data[contactorMsg.DLC];
		for (int i = 0; i < contactorMsg.DLC; i ++) {
			data[i] = contactorMsg.data[i];
		}

		uint8_t prechargerClosed = data[0] & 0x01; // extract bit 0
		uint8_t prechargerClosing = data[0] & 0x02; // extract bit 1
		uint8_t prechargerError = data[0] & 0x04; // extraxt bit 2
		uint8_t contactorClosed = data[0] & 0x08; // extract bit 3
		uint8_t contactorClosing = data[0] & 0x10; // extract bit 4
		uint8_t contactorError = data[0] & 0x20; // extract bit 5
		uint16_t contactorCurrent = ((data[0] & 0xc0) >> 6) + ((data[1] & 0xff) << 2) + ((data[2] & 0x03) << 10); // extract bits 6 to 17
		uint16_t contactorVoltage = ((data[2] & 0xfc) >> 2) + ((data[3] & 0x3f) << 6); // extract bits 18 to 29

		updateContactorInfo((contactorMsg.extendedID - CONTACTORIDS), prechargerClosed, prechargerClosing, prechargerError, contactorClosed, contactorClosing, contactorError, contactorCurrent, contactorVoltage);

		// do update contactor state contactor info stuff !!!!

		// DO HEARTBEATS TOO BUT CLARIFY HOW? IN SAME STRUCT? OR DIFF ONE ? IDK , also should i keep contactorstate, or no, bc now i have contactor info

		// check trips
		if (contactorInfo[COMMON].current >= MAX_CONTACTOR_CURRENT){
			mbmsTrip.highCommonCurrentTrip = 1;
		}
		if ((contactorInfo[MOTOR1].current >= MAX_CONTACTOR_CURRENT) || (contactorInfo[MOTOR2].current >= MAX_CONTACTOR_CURRENT)){
			mbmsTrip.motorHighTempCurrentTrip = 1;
		}
		if (contactorInfo[ARRAY].current >= MAX_CONTACTOR_CURRENT){
			mbmsTrip.arrayHighTempCurrentTrip = 1;
		}
		if (contactorInfo[LOWV].current >= MAX_CONTACTOR_CURRENT){
			mbmsTrip.LVHighTempCurrentTrip = 1;
		}
		if (contactorInfo[CHARGE].current >= MAX_CONTACTOR_CURRENT){
			mbmsTrip.chargeHighTempTrip = 1;
		}


	}

	// should send CAN message of trips ???
	CANMsg tripMsg;
	uint16_t tripData = ((mbmsTrip.highCellVoltageTrip & 0x1) << 0) + ((mbmsTrip.lowCellVoltageTrip & 0x1) << 1)
			+ ((mbmsTrip.highCommonCurrentTrip & 0x1) << 2) + ((mbmsTrip.motorHighTempCurrentTrip & 0x1) << 3)
		+ ((mbmsTrip.arrayHighTempCurrentTrip & 0x1) << 4) + ((mbmsTrip.LVHighTempCurrentTrip & 0x1) << 5)
		+ ((mbmsTrip.chargeHighTempTrip & 0x1) << 6) + ((mbmsTrip.protectionTrip & 0x1) << 7)
		+ ((mbmsTrip.orionMessageTimeoutTrip & 0x1) << 8) + ((mbmsTrip.contactorDisconnectedTrip & 0x1) << 9);
	tripMsg.data[0] = (tripData & 0xff);
	tripMsg.data[1] = (tripData & 0xff00) >> 8;
	tripMsg.DLC = 2; // 2 bytes
	tripMsg.extendedID = MBMS_TRIP_ID;
	tripMsg.ID = 0x0;
	osMessageQueuePut(TxCANMessageQueueHandle, &tripMsg, 0, osWaitForever);



	// should close contactors if perms given? OH PROBABLY ALSO CHECK IF ALLOWE DTO DISCHARGE

	uint32_t flags = osEventFlagsGet(contactorPermissionsFlagHandle);
	CANMsg contactorCommandMsg;
	tripMsg.DLC = 2; // 2 bytes
	tripMsg.extendedID = MBMS_TRIP_ID;
	tripMsg.ID = 0x0;
	uint8_t sendContactorCommand = 0;
	if (((flags & COMMON_FLAG) == COMMON_FLAG) && !(contactorInfo[COMMON].contactorClosed) && (tripData == 0x0)) {
		// if perms are given, and contactor is not already closed, and there are no trips
		contactorCommand.common = CLOSE_CONTACTOR;
		sendContactorCommand = 1;
	}
	if (((flags & MOTOR1_FLAG) == MOTOR1_FLAG) && !(contactorInfo[MOTOR1].contactorClosed) && (tripData == 0x0) && (mbmsStatus.allowDischarge == 1)) {
		contactorCommand.motor1 = CLOSE_CONTACTOR;
		sendContactorCommand = 1;
	}
	if (((flags & MOTOR2_FLAG) == MOTOR2_FLAG) && !(contactorInfo[MOTOR2].contactorClosed) && (tripData == 0x0) && (mbmsStatus.allowDischarge == 1)) {
		contactorCommand.motor2 = CLOSE_CONTACTOR;
		sendContactorCommand = 1;
	}

	if (((flags & ARRAY_FLAG) == ARRAY_FLAG) && !(contactorInfo[ARRAY].contactorClosed) && (tripData == 0x0) && (mbmsStatus.allowCharge == 1)) {
		contactorCommand.array = CLOSE_CONTACTOR;
		sendContactorCommand = 1;
	}

	if (((flags & LV_FLAG) == LV_FLAG) && !(contactorInfo[LOWV].contactorClosed) && (tripData == 0x0)) {
		contactorCommand.LV = CLOSE_CONTACTOR;
		sendContactorCommand = 1;
	}
	if (((flags & CHARGE_FLAG) == CHARGE_FLAG) && !(contactorInfo[CHARGE].contactorClosed) && (tripData == 0x0) && (mbmsStatus.allowCharge == 1)) {
		contactorCommand.charge = CLOSE_CONTACTOR;
		sendContactorCommand = 1;
	}

	if (sendContactorCommand == 1) {
		osMessageQueuePut(TxCANMessageQueueHandle, &contactorCommandMsg, 0, osWaitForever);
	}

	// check key and mps and hard and soft batt limit to possibly set shutdown flags !!!!


	uint32_t shutoffFlagsSet;
	if (readKeySwitch() == KEY_OFF){
		shutoffFlagsSet = osEventFlagsSet(shutoffFlagHandle, KEY_FLAG | SHUTOFF_FLAG);
	}
	if (readMainPowerSwitch() == MPS_ENABLED){
		shutoffFlagsSet = osEventFlagsSet(shutoffFlagHandle, MPS_FLAG | SHUTOFF_FLAG);
	}




	// should communicate with startup and shutdown tasks, queue? mutex? flag? ... event flag for shutdown

	// checking everything is ok through orion

	// get the flag, (uint32_t startUpFlag = osEventFlagsGet(eventFlag_id)
	// var = orionCheck(flag) //giving it what the permissions are
	// var1 = 0; // idk
	// if (var says stuff is BAD || MPS is opened || key turned off || external button pressed) // this means u should shut down !!!
		// var1 = 1; // only do this check when closing contactors
		// set flag to something to indicate what has happened (soft batt lim, hard batt lim, external button, MPS, or key off )
		// this should be flag that shutdown procedure is waiting for... ?

	// else // not shutdown procedure !!!! just keep checking stuff,,, and if u need to close any contactors
	// if var1 == 0 && flag startUpFlag indicates startup is not over (not all perms given
		// compare it to see if common contactor permisiion given, if yes..
			// send message through CAN to connect precharger for common contactor
			// send message (CAN) to close/connect common contactor(the one that igs connect to them all)
			// send message (CAN) to disconnect CC precharger

		// compare it to see if LV perm given, if yes..
			// send message through CAN to connect precharger for  LV
			// send message (CAN) to close/connect LV
			// send message (CAN) to disconnect LV precharger

		// compare to see if motor perm given, if yes..
			// send CAN message to close/connect precharger for  motor
			// send CAN message to connect/close motor contactor
			// send CAN message to open precharger for motor

		// compare to see if all array perm given, if yes..
			// send CAN message to close precharge for array
			// send CAN message to close (connect) to array contactor(solar panels so they can charge battery!!!)
			// send CAN message to open/disconnect precharger for array


	// otherwise its already all permissions
	// maybe just check all the things that ae supposed to be closed are closed...

	// checking everything is ok through orion
	// if something is wrong
		// set flag for shutdwon task (in shutdown task have a wait event flag, and itll just do stuff if that flag is set

}

//should return 1, for everything is ok, and 0 for something (anything) has gone wrong...
uint32_t orionCheck(uint32_t permissions) {
	// MAYBE THIS IS SUPPOSED TO RECEIVE CAN MESSAGES SENT BY ORION AND DECIPHER IF ANYTHING IS WRONG?
	// idk do all the checks???
	// if not good,
		// set flag for shutdown !!!!!
		//return 0
	// if things r fine
	// check if CC closed
	// if not closed

}

void updateContactorInfo(uint8_t contactor, uint8_t prechargerClosed, uint8_t prechargerClosing, uint8_t prechargerError, uint8_t contactorClosed, uint8_t contactorClosing, uint8_t contactorError, uint16_t contactorCurrent, uint16_t contactorVoltage) {
	contactorInfo[contactor].prechargerClosed = prechargerClosed;
	contactorInfo[contactor].prechargerClosing = prechargerClosing;
	contactorInfo[contactor].prechargerError = prechargerError;
	contactorInfo[contactor].contactorClosed = contactorClosed;
	contactorInfo[contactor].contactorClosing = contactorClosing;
	contactorInfo[contactor].contactorError = contactorError;
	contactorInfo[contactor].current = contactorCurrent;
	contactorInfo[contactor].voltage = contactorVoltage;
	return;
}

// motor and array cont6actors might still need to open or close during.....
// but i think LV and common can just stay closed ... ?
// also should set flag in this function for shutdown procedure in case anything goes wrong


// also should not be closing more contactors if stuff is wrong!!!!!!

// gatekeeper tasks should be letting battery tasks know if it should close/open contactor
// ofc battery task should check w permissions setup from startup AND var1 (if shutdown procedure should
// occur, bc if so, shouldn't be able to close more contactors)



// ok i think in battery control task i should have if statements checking MPS, External button, and key (each correlating to one gpio pin !!!!!
// and i read that pin and see what it is and if theyve wanted to shutdown something
// then you should set the flag !!!!! so for flags, you should OR the shutoff flag with the reason flag
// shutodwn taks will then wait for any of those flags to be set, check the flags set (find reason why) then do what it needs to do
// i believe orionCheck function should maybe receive CAN messages from orionBMS with info abt the battery state (like is everything ok etc.
// and maybe just return a var id things are okay...???


// should also receive CAN message from the diff contactor board thingies, so THIS task will know if theyre closed or open
// and then maybe you can update a struct of contactors
// other tasks should be able to read this struct probably




// Dec 4
// have two code block/sections, one for orion, one for contactor
//  check if contcator states CAN message is receieved... if yes,  go into that if/else type block and update the struct of contactor states
// check if orion bms has sent message, if yes,  do whatever checks and flag setting u need to do (like settung shutdown flag stuff idk)
// oDO IF FOR BOTH SO CAN GO THREU BOTH< OR SKIP BOTH OR JUST GO ONE ETC






