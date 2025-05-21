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

static uint32_t BCT_Counter = 0;

static uint8_t heartbeatLastUpdatedTime[6] = {0};

static uint16_t previousHeartbeats[6] = {0}; //check this !!! syntax !

static Permissions perms = {0};

static uint8_t carState = STARTUP;

static SoftBatteryTrip softBatteryTrip = {0};


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

	/* Updating structs */
	UpdateContactorInfoStruct();
	UpdatePowerSelectionStruct();
	UpdateOrionInfoStruct();


	/* Tracking states */
	SystemStateMachine();

	/* Opening/closing contactors */
	UpdateContactors();

	/* Updating BCT Counter */
	UpdateCounter(&BCT_Counter);


	// NEED TO CHECK CURRENT STUFF STILL.... need to do all the trip stuff lol, also check attributes of can msg for sumn idk

}

/* FUNCTIONS FOR UPDATING STRUCTS */

void UpdateContactorInfoStruct() {
	//static uint8_t counter = 0;

	CANMsg contactorMsg;
	osStatus status = osMessageQueueGet(contactorMessageQueueHandle, &contactorMsg, NULL, 0);

	if (status == osOK) {

		// if the message is about the contactor heartbeats
		if((contactorMsg.extendedID & 0xff0) == CONTACTOR_HEARTBEATS_IDS){
			uint16_t newHeartbeat = contactorMsg.data[0] + (contactorMsg.data[1] << 8);
			osStatus_t a = osMutexAcquire(ContactorInfoMutexHandle, 200);
			if (a == osOK) {
				contactorInfo[contactorMsg.extendedID - CONTACTOR_HEARTBEATS_IDS].heartbeat = newHeartbeat;
				osStatus_t r = osMutexRelease(ContactorInfoMutexHandle);
			}
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
	osStatus_t a = osMutexAcquire(ContactorInfoMutexHandle, UPDATING_MUTEX_TIMEOUT);
	if(a == osOK) {
		contactorInfo[contactor].prechargerClosed = prechargerClosed;
		contactorInfo[contactor].prechargerClosing = prechargerClosing;
		contactorInfo[contactor].prechargerError = prechargerError;
		contactorInfo[contactor].contactorClosed = contactorClosed;
		contactorInfo[contactor].contactorError = contactorError;
		contactorInfo[contactor].lineCurrent = lineCurrent;
		contactorInfo[contactor].chargeCurrent = chargeCurrent;
		contactorInfo[contactor].BPSerror = BPSerror;

		osStatus_t r = osMutexRelease(ContactorInfoMutexHandle);
	}

	return;
}

/*
 * PROBLEM
 * but honestly whats the point of ABATT disable now...
 * now that EN2 is always on and aux batt is always on,
 * ik u could maybe use it to turn the car off but it
 * would just turn right back on if the key is still on..?
 * also reminder that its an output why are you reading it LOL ???
 *
 */
void UpdatePowerSelectionStruct() {
	powerSelectionStatus.nMainPowerSwitch = read_nMPS();
	powerSelectionStatus.ExternalShutdown = read_ESD();
	powerSelectionStatus.EN1 = read_EN1();
	powerSelectionStatus.n3A_OC = read_n3A_OC();
	powerSelectionStatus.nDCDC_Fault = read_nDCDC_Fault();
	powerSelectionStatus.nCHG_Fault = read_nCHG_Fault();
	powerSelectionStatus.nCHG_On = read_nCHG_On();
	powerSelectionStatus.nCHG_LV_En = read_nCHG_LV_En();
	powerSelectionStatus.ABATT_Disable = read_ABATT_Disable(); //
	powerSelectionStatus.Key = read_Key();

}

/*
 * This function dequeues Orion CAN msg and updates battery info struct
 */
void UpdateOrionInfoStruct() {

	CANMsg orionMsg;
	osDelay(1000); // why there a delay here .... maybe from when i was testing...
	static uint8_t orionMessageCounter = 0;

	osStatus status = osMessageQueueGet(batteryControlMessageQueueHandle, &orionMsg, NULL, ORION_MSG_WAIT_TIMEOUT);

	if (status == osOK) {

		// reset counter to zero now that you've received message
		orionMessageCounter = 0;
		mbmsStatus.orionCANReceived = 1;


		uint8_t data[orionMsg.DLC];
		for (int i = 0; i < orionMsg.DLC; i ++) {
			data[i] = orionMsg.data[i];
		}

		if (orionMsg.extendedID == PACK_INFO_ID) {
			osStatus_t a = osMutexAcquire(BatteryInfoMutexHandle, 200);
			if(a == osOK) {
				// update batteryInfo instance for the pack info stuff
				batteryInfo.packCurrent = data[0] + (data[1] << 8);
				batteryInfo.packVoltage = data[2] + (data[3] << 8);
				batteryInfo.packSOC = data[4];
				batteryInfo.packAmphours = data[5] + (data[6] << 8);
				batteryInfo.packDOD = data[7];
				osStatus_t r = osMutexRelease(BatteryInfoMutexHandle);

			}

			// PROBLEM: is it?
			mbmsStatus.auxilaryBattVoltage = batteryInfo.packVoltage;

			// PROBLEM: look over this.. also change names
			// updating allow charge/discharge on mbmsStatus, based on SOC
			if ((batteryInfo.packSOC >= SOC_SAFE_FOR_DISCHARGE) && (read_Charge_Enable() == 1)) {
				mbmsStatus.allowDischarge = 1;
			}
			if ((batteryInfo.packSOC <= SOC_SAFE_FOR_CHARGE) && (read_Discharge_Enable() == 1)) {
				mbmsStatus.allowCharge = 1;
			}


		}
		else if (orionMsg.extendedID == TEMP_INFO_ID) {
			osStatus_t a = osMutexAcquire(BatteryInfoMutexHandle, 200);
			if(a == osOK) {
				batteryInfo.highTemp = data[0];
				batteryInfo.lowTemp = data[2];
				batteryInfo.avgTemp = data[4];
				osStatus_t r = osMutexRelease(BatteryInfoMutexHandle);
			}
		}

		else if (orionMsg.extendedID == MIN_MAX_VOLTAGES_ID) {
			osStatus_t a = osMutexAcquire(BatteryInfoMutexHandle, 200);
			if(a == osOK) {
				batteryInfo.maxCellVoltage = data[0] + (data[1] << 8);
				batteryInfo.minCellVoltage = data[2] + (data[3] << 8);
				batteryInfo.maxPackVoltage = data[4] + (data[5] << 8);
				batteryInfo.minPackVoltage = data[6] + (data[7] << 8);
				osStatus_t r = osMutexRelease(BatteryInfoMutexHandle);
			}

		}

	}

	else if (status == osErrorTimeout) // if timeout for orion (no message :0)
	{
		orionMessageCounter += 1;
	}
	if(orionMessageCounter >= 3){
		osStatus_t a = osMutexAcquire(MBMSStatusMutexHandle, 200);
		if (a == osOK) {
			mbmsStatus.orionCANReceived = 0; // no orion message recieved !!!
			osStatus_t r = osMutexRelease(MBMSStatusMutexHandle);
		}
	}

}


/* GENERAL FUNCTIONALITY FUNCTIONS */

/*
 * updates counter that tracks how many times BCT has run through
 * call this function at end of BCT
 * counter can be used for startup to check if checks have been done...
 */
void UpdateCounter(uint32_t * counter) {
	(*counter)++;

}

void SystemStateMachine() {

	// make var plugged for now to stand in for the CAN msg that charger is plugged in or not
	uint8_t plugged = 0;

	switch (carState) {
		case STARTUP:

			// will go to BPS_FAULT state if startup checks do not pass
			startupCheck();

			// checks MPS
			if(read_nMPS() == 1) {
				carState = MPS_DISCONNECTED;
				break;
			}

			if (mbmsStatus.startupState == COMPLETED){
				carState = FULLY_OPERATIONAL;
			}

			break;

		case FULLY_OPERATIONAL:

			if(read_nMPS() == 1) {
				carState = MPS_DISCONNECTED;
				break;
			}

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

			/* Running checks */
			CheckContactorHeartbeats();
			CheckSoftBatteryLimit();
			UpdateTripStatus();

			break;

		case CHARGING:
			// turns off car if key is off
			checkKeyShutdown();

			if(read_nMPS() == 1) {
				carState = MPS_DISCONNECTED;
				break;
			}

			// checks if charger is unplugged
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

			/* Running checks */
			CheckContactorHeartbeats();
			CheckSoftBatteryLimit();
			UpdateTripStatus();

			break;

		case BPS_FAULT:
			perms.faulted = 1;
			osEventFlagsSet(shutoffFlagHandle, (HARD_BL_FLAG | SHUTOFF_FLAG));
			// delay for shutdown to run.... although rn its a higher priority so..
			osDelay(1000);

			break;

		case MPS_DISCONNECTED:
			perms.faulted = 1; // stop contactors from closing...
			osEventFlagsSet(shutoffFlagHandle, (nMPS_FLAG | SHUTOFF_FLAG));
			osDelay(1000);

			break;

		case SOFT_TRIP:
			perms.faulted = 1;
			if (softBatteryTrip.cell_OV == 1){
				perms.charge = 0;
				perms.array = 0;

			}
			if(softBatteryTrip.cell_UV == 1) {
				perms.motor = 0;
			}
			if(read_nMPS() == 1) {
				carState = MPS_DISCONNECTED;
				break;
			}
			/* Running checks */
			CheckContactorHeartbeats();
			CheckSoftBatteryLimit();
			UpdateTripStatus();

			break;
	}


}

void UpdateContactors() {

//    uint8_t sendContactorCommand = 0;

	// we would want this reinitialized every time the function is called
	// and it is then checked below... so not static!!
    uint8_t contactorClosing = false;

    // Check if any contactors are currently closing
    for (int i = 0; i < 5; i++) {
        if (contactorInfo[i].contactorClosing == CLOSING_CONTACTOR) { // lowkey switch this back to an enum if u have time smh.. stoopid fr
            contactorClosing = true;
            break;
        }
    }

    // If no contactors are currently closing, and battery not in fault type state (MPS, BPS)
    if (!contactorClosing && !perms.faulted) {
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
    if ((!perms.common) && (contactorInfo[COMMON].contactorClosed != OPEN_CONTACTOR)) {
    	contactorCommand.motor = OPEN_CONTACTOR;
    }
    if ((!perms.motor) && (contactorInfo[MOTOR].contactorClosed != OPEN_CONTACTOR)) {
        contactorCommand.motor = OPEN_CONTACTOR;
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


/* FUNCTIONS RELATED TO STARTUP */

/*
 * This function should be called during the startup procedure
 * It goes through all the checks needed on startup, such as contactor heartbeats, contactors open,
 * and battery state (voltages and temperatures)
 * (Don't need to check current beacuse the contactors should be open anyways..)
 */
void startupCheck(){

	/* Waiting for contactor heartbeats */
	uint8_t heartbeatDead = 0;
	while ((previousHeartbeats[0] == 0) || (previousHeartbeats[1] == 0) || (previousHeartbeats[2] == 0) ||
		   (previousHeartbeats[3] == 0) || (previousHeartbeats[4] == 0) || (heartbeatDead != 0))
	{
		// set heartbeatDead so we can break out of while loop lol
		heartbeatDead = waitForFirstHeartbeats();
	}
	if (heartbeatDead == 1){
		initiateBPSFault();
	}

	/* Check to ensure no contactors are closed */
	if ((checkContactorsOpen() == 0) || checkPrechargersOpen() == 0){
		initiateBPSFault();
	}

	/* Battery check (orion) */
	uint8_t passedBatteryCheck = startupBatteryCheck(); // this func actually only checks hard limits for now...
	if (!passedBatteryCheck) {
		initiateBPSFault();
	}

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

/*
 * returns whether any contactors are closed (0) or not (1). want them all to be open on startup
 */

// PROBLEM: this does not have a specific trip for it yet... it just goes to BPS fault...
uint8_t checkContactorsOpen() {
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
// PROBLEM: this does not have a specific trip for it yet... it just goes to BPS fault...
uint8_t checkPrechargersOpen() {

	uint8_t allOpen = 1;
	osStatus_t acquire = osMutexAcquire(ContactorInfoMutexHandle, 200);
	if (acquire == osOK) {

		for (int i = 1; i < 5; i++) { //COMMON HAS NO PRECHARGER which is why i = 1
			if (contactorInfo[i].prechargerClosed == CLOSE_CONTACTOR) {
				allOpen = 0;
				break;
			}
		}
		osStatus_t release = osMutexRelease(ContactorInfoMutexHandle);

	}

	return allOpen;

}

uint8_t startupBatteryCheck() {
	// maybe set the flag for hard batt lim, soft batt lim here? or idk loll
	// or just have a var somewhere to keep track and check the var in the check if shutdown stuff?
	// in case theres multiple things wrong so you can store all the trips before yk, doing whatever BPS procedure

	uint8_t safe = 1;
	// check this mutex stuff ngl...
	osStatus_t acquire = osMutexAcquire(MBMSTripMutexHandle, 200);
	if(acquire == osOK) {
		uint8_t safe = 1;

		if(batteryInfo.maxCellVoltage > HARD_MAX_CELL_VOLTAGE){
			mbmsTrip.highCellVoltageTrip = 1;
			safe = 0;
		}

		if(batteryInfo.minCellVoltage < HARD_MIN_CELL_VOLTAGE) {
			mbmsTrip.lowCellVoltageTrip = 1;
			safe = 0;
		}


		if(batteryInfo.highTemp > HARD_MAX_TEMP) {
			mbmsTrip.highTemperatureTrip = 1;
			safe = 0;
		}

		if(batteryInfo.lowTemp < HARD_MIN_TEMP) {
			mbmsTrip.lowTemperatureTrip = 1;
			safe = 0;
		}

		//release the mutex !!!!
		osStatus_t release = osMutexRelease(MBMSTripMutexHandle);
	}

	return safe;

}


/* FUNCTIONS RELATED TO GENERAL TRIPS & SOFT LIMITS */


/*
 * This function turns off charging when key is off to shut off car
 */
void checkKeyShutdown() {
	if (read_Key() == 0) {
		// turn off charge LV enable to shutoff car..
		HAL_GPIO_WritePin(nCHG_LV_En_GPIO_Port, nCHG_LV_En_Pin, GPIO_PIN_SET);

	}
}


/*
 * This function runs when a BPS Fault should occur
 * It turns on the strobe light, and changes the mbms status
 * Switches car state to BPS_Fault !!!
 */
void initiateBPSFault() {
	// strpbe enable
	HAL_GPIO_WritePin(Strobe_En_GPIO_Port, Strobe_En_Pin, 1);

	// ADDED SOMETHING HERE:
	carState = BPS_FAULT;
	osStatus_t a = osMutexAcquire(MBMSStatusMutexHandle, 200);
	if(a == osOK) {
		// update mbms status
		mbmsStatus.strobeBMSLight = 1;
		osStatus_t r = osMutexRelease(MBMSStatusMutexHandle);

	}
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
void CheckContactorHeartbeats() {
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


/* This function checks the soft limits of voltages, currents, and temperatures
 * These warnings should be sent out in a CAN message (CANMessageSender -> CANTxGatekeeper)
 * You can think of these like a warning on your phone that it's low battery
 * Don't need to do anything for these soft limits, just send the warning!
 */
void CheckSoftBatteryLimit() {
	/// ummmmm be careful deadlock mauybe check everything once ur done all the mutexes
	osStatus_t acquire = osMutexAcquire(MBMSSoftLimitWarningMutexHandle, 200);
	if(acquire == osOK) {

		osStatus_t a1 = osMutexAcquire(BatteryInfoMutexHandle, 200);
		if (a1 == osOK){
			/* Checking the min/max cell voltages */
			if (batteryInfo.maxCellVoltage > SOFT_MAX_CELL_VOLTAGE) {
				softBatteryTrip.cell_OV = 1;
				carState = SOFT_TRIP;
				mbmsSoftBatteryLimitWarning.highCellVoltageWarning = 1;

			}
			if (batteryInfo.minCellVoltage < SOFT_MIN_CELL_VOLTAGE) {
				softBatteryTrip.cell_UV = 1;
				carState = SOFT_TRIP;
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
			if (batteryInfo.packCurrent > SOFT_MAX_COMMON_CONTACTOR_CURRENT){
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

void UpdateTripStatus() {

	static uint8_t BPS_Fault = 0;
	osStatus_t acquire = osMutexAcquire(MBMSTripMutexHandle, 200);
	if (acquire == osOK){

		osStatus_t a1 = osMutexAcquire(ContactorInfoMutexHandle, 200);
		if (a1 == osOK){

			if (batteryInfo.packCurrent > HARD_MAX_COMMON_CONTACTOR_CURRENT){
				mbmsTrip.commonHighCurrentTrip = 1;
				BPS_Fault = 1;
			}

			/* not using HIGH CURRENT TRIPS as of now. May 17. */
			/*
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

			 */


			/* Not using PROTECTION TRIP as of now. May 17. */
			/*
			if ((contactorInfo[CHARGE].lineCurrent > 0) || (contactorInfo[LOWV].lineCurrent < 0)){
				mbmsTrip.protectionTrip = 1;
				BPS_Fault = 1;
			}
			 */

			osStatus_t r1 = osMutexRelease(ContactorInfoMutexHandle);

		}

		osStatus_t a2 = osMutexAcquire(BatteryInfoMutexHandle, 200);
		if (a2 == osOK){

			/* checking for high/low cell voltage trips */
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

			/* Contactor disconnected unexpectedely */
			/* To check, we compare a minimum current draw with the state of the contactor */
			if(((		 contactorCommand.common == CLOSE_CONTACTOR) && (batteryInfo.packCurrent < NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.motor == CLOSE_CONTACTOR) && (contactorInfo[MOTOR].lineCurrent < NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.array  == CLOSE_CONTACTOR) && (contactorInfo[ARRAY].lineCurrent  < NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.LV     == CLOSE_CONTACTOR) && (contactorInfo[LOWV].lineCurrent   < NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.charge == CLOSE_CONTACTOR) && (contactorInfo[CHARGE].lineCurrent < NO_CURRENT_THRESHOLD))
				)
			{
				mbmsTrip.contactorDisconnectedUnexpectedlyTrip = 1;
				BPS_Fault = 1;
			}

			/* Contactor connected unexpectedly trip */
			if(((		 contactorCommand.common == OPEN_CONTACTOR) && (batteryInfo.packCurrent >= NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.motor == OPEN_CONTACTOR) && (contactorInfo[MOTOR].lineCurrent >= NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.array  == OPEN_CONTACTOR) && (contactorInfo[ARRAY].lineCurrent  >= NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.LV     == OPEN_CONTACTOR) && (contactorInfo[LOWV].lineCurrent   >= NO_CURRENT_THRESHOLD))
					|| ((contactorCommand.charge == OPEN_CONTACTOR) && (contactorInfo[CHARGE].lineCurrent >= NO_CURRENT_THRESHOLD))
				)
			{
				mbmsTrip.contactorConnectedUnexpectedlyTrip = 1;
				BPS_Fault = 1;

				// HEY YOU NEED TO ADD THE CONTACTOR WONT OPEN (FROM CONTACTOR CAN) STUFF HERE !!!!
			}



			osStatus_t r4 = osMutexRelease(ContactorCommandMutexHandle);

		}

		// this is techincally not a "trip" that will cause BPS....
		// its just for information purposes i suppose
		if(read_nMPS() == 1){
			mbmsTrip.MPSDisabledTrip = 1;

		}

		if(read_ESD() == 1){
			mbmsTrip.ESDEnabledTrip = 1;
			BPS_Fault = 1;
		}

		// release trip mutex
		osStatus_t release = osMutexRelease(MBMSTripMutexHandle);

		if(BPS_Fault) {
			initiateBPSFault();
		}

	}


}







