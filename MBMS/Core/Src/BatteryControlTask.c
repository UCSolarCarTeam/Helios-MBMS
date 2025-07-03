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
#include "CANMessageSenderTask.h"
#include "MBMS.h"
#include "MBMS_Screen.h"
#include "main.h"

/*
 * External variables
 */
MBMSTrip mbmsTrip = {0};
MBMSSoftBatteryLimitWarning mbmsSoftBatteryLimitWarning = {0};
SoftBatteryTrip softBatteryTrip = {0};

uint32_t BCT_Counter = 0;
uint32_t startup_Check_Counter = 0;


uint8_t carState = BOOT;

ContactorCommand contactorCommand = {0};
BatteryInfo batteryInfo = {0};
PowerSelectionStatus powerSelectionStatus = {0};
MBMSStatus mbmsStatus; // made specific init function for this, may 21

Permissions perms = {0};
ContactorInfo contactorInfo[NUM_OF_CONTACTORS]; // one for each contactor        add volatile to the extern thing too

ScreenDataDictionary screenData; // this is the root data structure for the screen

uint8_t orionMessagesReceived = 0x0;

uint32_t heartbeat_check_count = 0;
extern uint32_t heartbeat_update_count;

extern uint32_t lastSentTime[6];

uint32_t pack_info_count = 0;
uint32_t temp_info_count = 0;
uint32_t cell_voltages_count = 0;

uint32_t orion_received_tick = 0;

uint32_t LV_OC_tick_count = 0;


uint32_t contactor_command_start_tick[NUM_OF_CONTACTORS] = {0};

uint32_t hard_high_current_count[NUM_OF_CONTACTORS] = {0};

/*
 * Local Variables
 */


// no init for this as of rn ... may 21

/* used for checking ummmm heartbeats */
static uint32_t heartbeatLastUpdatedTime[NUM_OF_CONTACTORS] = {0};
static uint32_t previousHeartbeats[NUM_OF_CONTACTORS] = {0}; //check this !!! syntax !


uint32_t orion_msg_from_queue = 0;

uint32_t BCT_start_tick = 0;
uint32_t BCT_end_tick = 0;
uint32_t BCT_difference_tick = 0;

uint32_t BCT_difference_seconds = 0;
uint32_t taskTickLastStart = 0;




// if trip, check if startup state is less than fully operational, if so, kill it, otherwise if its at fully operational it would've already killed itself

void BatteryControlTask(void* arg)
{
	taskTickLastStart = osKernelGetTickCount();
	for (int i = 0; i < NUM_OF_CONTACTORS; i++) {
		contactor_command_start_tick[i] = osKernelGetTickCount() + 15;
	}

#if 1
	//just added this june 23 for the lights LOLLLL dont need this if it fucks up the code ...
	enter_BOOT();
#endif

    while(1)
    {
    	BCT_start_tick = osKernelGetTickCount();
    	BatteryControl();
    	BCT_end_tick = osKernelGetTickCount();
    	BCT_difference_tick = BCT_end_tick - BCT_start_tick;
    	BCT_difference_seconds = taskTickLastStart;

		taskTickLastStart += 10;
		osDelayUntil(taskTickLastStart);
    }
}


void BatteryControl()
{

	// before u set shutdown flag, check startupState and terminate the task if its not finished... ?

	/* Updating structs */
	UpdateContactorInfoStruct();
	UpdatePowerSelectionStruct();
	UpdateOrionInfoStruct();
	UpdateScreenDataStructs();

	/* Tracking states */
	SystemStateMachine();

	/* Opening/closing contactors */
	UpdateContactors();

	/* Updating BCT Counter */
	UpdateCounter(&BCT_Counter);


}

uint32_t toggle_led(uint8_t LED, uint8_t period_ms, uint32_t start_tick) {

	uint32_t tick_diff = start_tick - osKernelGetTickCount();
	uint32_t tick_diff_ms = tick_diff * FREERTOS_TICK_PERIOD;
	if ((tick_diff * FREERTOS_TICK_PERIOD) >= period_ms) {
		switch(LED) {
			case BLU:
				HAL_GPIO_TogglePin(BLU_LED_GPIO_Port, BLU_LED_Pin);
				break;

			case GRN:
				HAL_GPIO_TogglePin(GRN_LED_GPIO_Port, GRN_LED_Pin);
				break;

			case RED:
				HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
				break;
		}
		return osKernelGetTickCount();
	}

	return start_tick;
}


void UpdateContactorInfoStruct() {
	//static uint8_t counter = 0;

	CANMsg contactorMsg;
	osStatus status = osMessageQueueGet(contactorMessageQueueHandle, &contactorMsg, NULL, 0);

	if (status == osOK) {

		// if the message is about the contactor heartbeats
		if((contactorMsg.extendedID & 0xff0) == CONTACTOR_HEARTBEATS_IDS){
			uint16_t newHeartbeat = contactorMsg.data[0] + (contactorMsg.data[1] << 8);
			osStatus_t a = osMutexAcquire(ContactorInfoMutexHandle, 55);
			if (a == osOK) {
				contactorInfo[contactorMsg.extendedID - CONTACTOR_HEARTBEATS_IDS].heartbeat = newHeartbeat;
				heartbeat_update_count++;
				osMutexRelease(ContactorInfoMutexHandle);
			}
		}

		// if the message is about the contactor info
		else{
			uint8_t data[contactorMsg.DLC];
			for (int i = 0; i < contactorMsg.DLC; i ++) {
				data[i] = contactorMsg.data[i];
			}

			uint8_t prechargerClosed = (data[0] & 0x01) ? CLOSE_CONTACTOR: OPEN_CONTACTOR; // extract bit 0
			uint8_t prechargerClosing = (data[0] & 0x02) ? CLOSE_CONTACTOR: OPEN_CONTACTOR; // extract bit 1
			uint8_t prechargerError = (data[0] & 0x04) ? CLOSE_CONTACTOR: OPEN_CONTACTOR; // extract bit 2
			uint8_t contactorClosed = (data[0] & 0x08) ? CLOSE_CONTACTOR: OPEN_CONTACTOR; // extract bit 3
			uint8_t contactorClosing = (data[0] & 0x10) ? CLOSE_CONTACTOR: OPEN_CONTACTOR; // extract bit 4
			uint8_t contactorError = (data[0] & 0x20) ? CLOSE_CONTACTOR: OPEN_CONTACTOR; // extract bit 5
			float lineCurrent = (float) (((data[0] & 0xc0) >> 6) |((data[1] & 0xff) << 2) | ((data[2] & 0x03) << 10)) / 10.0; // extract bits 6 to 17
			float chargeCurrent = (((data[2] & 0xfc) >> 2) | ((data[3] & 0x3f) << 6)) /10; // extract bits 18 to 29
			uint8_t contactorOpeningError = (data[3] & 0x80) ? CLOSE_CONTACTOR: OPEN_CONTACTOR; //extract bit 30
			updateContactorInfo((contactorMsg.extendedID - CONTACTORIDS), prechargerClosed, prechargerClosing, prechargerError,
					contactorClosed, contactorClosing, contactorError, lineCurrent, chargeCurrent, contactorOpeningError);

		}
	}

}

void updateContactorInfo(uint8_t contactor, uint8_t prechargerClosed, uint8_t prechargerClosing, uint8_t prechargerError,
	uint8_t contactorClosed, uint8_t contactorClosing, uint8_t contactorError, float lineCurrent, float chargeCurrent, uint8_t contactorOpeningError) {
	osStatus_t a = osMutexAcquire(ContactorInfoMutexHandle, UPDATING_MUTEX_TIMEOUT);
	if(a == osOK) {
		contactorInfo[contactor].prechargerClosed = prechargerClosed;
		contactorInfo[contactor].prechargerClosing = prechargerClosing;
		contactorInfo[contactor].prechargerError = prechargerError;
		contactorInfo[contactor].contactorClosed = contactorClosed;
		contactorInfo[contactor].contactorError = contactorError;
		if(lineCurrent < 0) {
			lineCurrent *= -1; // get absolute value i guess....
		}
		contactorInfo[contactor].lineCurrent = lineCurrent;
		contactorInfo[contactor].chargeCurrent = chargeCurrent;
		contactorInfo[contactor].contactorOpeningError = contactorOpeningError;

		osMutexRelease(ContactorInfoMutexHandle);
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
	powerSelectionStatus.nDCDC_Fault = read_nDCDC_Fault();
	powerSelectionStatus.n3A_OC = read_n3A_OC();
	powerSelectionStatus.nDCDC_On = read_nDCDC_On();
	powerSelectionStatus.nCHG_Fault = read_nCHG_Fault();
	powerSelectionStatus.nCHG_On = read_nCHG_On();
	powerSelectionStatus.nCHG_LV_En = read_nCHG_LV_En();
	powerSelectionStatus.ABATT_Disable = read_ABATT_Disable(); //
	powerSelectionStatus.Key = read_Key();

}

void MBMSStatus_init() {
	mbmsStatus.auxilaryBattVoltage = 0;
	mbmsStatus.strobeBMSLight = 0;
	mbmsStatus.chargeEnable = 0;
	mbmsStatus.nChargeSafety = 1;
	mbmsStatus.dischargeEnable = 0;
	//mbmsStatus.orionCANReceived = 0;
	mbmsStatus.dischargeShouldTrip = 0;
	mbmsStatus.chargeShouldTrip = 0;

}

void perms_init() {
	perms.common = 0;
	perms.motor = 0;
//	perms.array = 0;
//	perms.lv = 0;
	perms.charge = 0;
}

/*
 * This function dequeues Orion CAN msg and updates battery info struct
 */
uint16_t orionMessageCounter = 0;
void UpdateOrionInfoStruct() {

	CANMsg orionMsg;
	//osDelay(1000); // why there a delay here .... maybe from when i was testing...

	osStatus status = osMessageQueueGet(batteryControlMessageQueueHandle, &orionMsg, NULL, 5); //timeout is in timer ticks...... so ms?

	if (status == osOK) {

		orion_msg_from_queue++;

		orion_received_tick = osKernelGetTickCount(); // gets here every 10 ticks wow !!!!

		// reset counter to zero now that you've received message
		orionMessageCounter = 0;
		mbmsStatus.orionCANReceived = 1;


		uint8_t data[orionMsg.DLC];
		for (int i = 0; i < orionMsg.DLC; i ++) {
			data[i] = orionMsg.data[i];
		}

		if (orionMsg.extendedID == PACK_INFO_ID) {
			osStatus_t a = osMutexAcquire(BatteryInfoMutexHandle, 5);
			if(a == osOK) {
				// update batteryInfo instance for the pack info stuff
				float current = ((float) data[0] + (float) (data[1] << 8)) / 10.0;
				if (current < 0) {
					current = current * -1;
				}
				batteryInfo.packCurrent = current;
				batteryInfo.packVoltage = ((float) data[2] + (float) (data[3] << 8)) / 10.0;
				batteryInfo.packSOC =( data[4]) / 2;
				batteryInfo.packAmphours = (data[5] + (data[6] << 8)) / 10;
				batteryInfo.packDOD = (data[7]) /2;

				pack_info_count++;

				orionMessagesReceived |= 0x1;
				osMutexRelease(BatteryInfoMutexHandle);

			}

			// PROBLEM: is it?
			mbmsStatus.auxilaryBattVoltage = batteryInfo.packVoltage;

			// PROBLEM: look over this.. also change names
			// updating allow charge/discharge on mbmsStatus, based on SOC

			mbmsStatus.chargeEnable = (read_Charge_Enable() == CHARGE_ENABLE_ACTIVE);
			mbmsStatus.dischargeEnable = (read_Discharge_Enable() == DISCHARGE_ENABLE_ACTIVE);

//			if (read_Charge_Enable() == 1) {
//				mbmsStatus.nChargeEnable = 0;
////				mbmsStatus.nDischargeEnable = 0;
//			}
//			else {
//				mbmsStatus.nChargeEnable = 1;
//			}
//			if (read_Discharge_Enable() == 1) {
//				mbmsStatus.nDischargeEnable = 0;
////				mbmsStatus.nChargeEnable = 0;
//			}
//			else {
//				mbmsStatus.nDischargeEnable = 1;
//			}


		}
		else if (orionMsg.extendedID == TEMP_INFO_ID) {
			osStatus_t a = osMutexAcquire(BatteryInfoMutexHandle, 5);
			if(a == osOK) {
				batteryInfo.highTemp = data[0];
				batteryInfo.lowTemp = data[2];
				batteryInfo.avgTemp = data[4];

				temp_info_count++;

				orionMessagesReceived |= 0x2;
				osMutexRelease(BatteryInfoMutexHandle);
			}
		}
		else if (orionMsg.extendedID == CELL_VOLTAGES_ID) {
			osStatus_t a = osMutexAcquire(BatteryInfoMutexHandle, 5);
			if(a == osOK) {
				batteryInfo.lowCellVoltage = (float)(data[0] + (data[1] << 8)) / 10000.0;
				batteryInfo.lowCellVoltageID = data[2];
				batteryInfo.highCellVoltage= (float) (data[3] + (data[4] << 8)) /10000.0;
				batteryInfo.highCellVoltageID = data[5];

				cell_voltages_count++;

				orionMessagesReceived |= 0x4;
				osMutexRelease(BatteryInfoMutexHandle);
			}

		}

		// the below is not even used tbh but if u were to use it, check the units and do the proper conversions!
		// and do orion messages received stuff if u use this
//		else if (orionMsg.extendedID == MIN_MAX_VOLTAGES_ID) {
//			osStatus_t a = osMutexAcquire(BatteryInfoMutexHandle, 5);
//			if(a == osOK) {
//				batteryInfo.maxCellVoltage = data[0] + (data[1] << 8);
//				batteryInfo.minCellVoltage = data[2] + (data[3] << 8);
//				batteryInfo.maxPackVoltage = data[4] + (data[5] << 8);
//				batteryInfo.minPackVoltage = data[6] + (data[7] << 8);
//				osMutexRelease(BatteryInfoMutexHandle);
//			}
//
//		}

	}

	else // if timeout for orion (no message :0)
	{
		orionMessageCounter += 1;
	}
	if((orionMessageCounter) >= 200){ // idk hehe
		osStatus_t a = osMutexAcquire(MBMSStatusMutexHandle, UPDATING_MUTEX_TIMEOUT);
		if (a == osOK) {
			mbmsStatus.orionCANReceived = 0; // no orion message recieved !!!
			osMutexRelease(MBMSStatusMutexHandle);
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

void clear_Trips() {
	mbmsTrip.ESDEnabledTrip = 0;
	mbmsTrip.LVHeartbeatDeadTrip = 0;
	mbmsTrip.LVHighCurrentTrip = 0;
	mbmsTrip.MPSDisabledTrip = 0;
	mbmsTrip.arrayHeartbeatDeadTrip = 0;
	mbmsTrip.arrayHighCurrentTrip = 0;
	mbmsTrip.chargeHeartbeatDeadTrip = 0;
	mbmsTrip.chargeHighCurrentTrip = 0;
	mbmsTrip.commonHeartbeatDeadTrip = 0;
	mbmsTrip.commonHighCurrentTrip = 0;
	mbmsTrip.contactorConnectedUnexpectedlyTrip = 0;
	mbmsTrip.contactorDisconnectedUnexpectedlyTrip = 0;
	mbmsTrip.highBatteryTrip = 0;
	mbmsTrip.highCellVoltageTrip = 0;
	mbmsTrip.highTemperatureTrip = 0;
	mbmsTrip.lowCellVoltageTrip = 0;
	mbmsTrip.lowTemperatureTrip = 0;
	mbmsTrip.motorHeartbeatDeadTrip = 0;
	mbmsTrip.motorHighCurrentTrip = 0;
	mbmsTrip.orionMessageTimeoutTrip = 0;
	mbmsTrip.protectionTrip = 0;
}

void clear_Warnings() {
	mbmsSoftBatteryLimitWarning.LVHighCurrentWarning = 0;
	mbmsSoftBatteryLimitWarning.arrayHighCurrentWarning = 0;
	mbmsSoftBatteryLimitWarning.chargeHighCurrentWarning = 0;
	mbmsSoftBatteryLimitWarning.commonHighCurrentWarning = 0;
	mbmsSoftBatteryLimitWarning.highBatteryWarning = 0;
	mbmsSoftBatteryLimitWarning.highCellVoltageWarning = 0;
	mbmsSoftBatteryLimitWarning.lowCellVoltageWarning = 0;
	mbmsSoftBatteryLimitWarning.motorHighCurrentWarning = 0;
	mbmsSoftBatteryLimitWarning._12V_CAN_OC_Warning = 0;

}

void enter_BOOT() {
	//cyan
	HAL_GPIO_WritePin(GRN_LED_GPIO_Port, GRN_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BLU_LED_GPIO_Port, BLU_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);

	orionMessagesReceived = 0;
	startup_Check_Counter = 0;
	BCT_Counter = 0;
	pack_info_count = 0;
	temp_info_count = 0;
	cell_voltages_count = 0;
	orionMessageCounter = 0;
	perms.faulted = 0;

	mbmsStatus.carState = BOOT;

	for(int i = 0; i < NUM_OF_CONTACTORS; i++) {
		previousHeartbeats[i] = 0;
		heartbeatLastUpdatedTime[i] = osKernelGetTickCount() + 15;
		contactorInfo[i].heartbeat = 0;
	}

	clear_Trips();
	clear_Warnings();
	mbmsStatus.carState = BOOT;

}

void enter_MPS_DISCONNECTED() {
	mbmsTrip.MPSDisabledTrip = 1;
	mbmsStatus.carState = MPS_DISCONNECTED;

	// blue
	HAL_GPIO_WritePin(BLU_LED_GPIO_Port, BLU_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GRN_LED_GPIO_Port, GRN_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);

	perms.faulted = 1; // stop contactors from closing...
	osEventFlagsSet(shutoffFlagHandle, (nMPS_FLAG | SHUTOFF_FLAG));
	//osDelay(10);

}

/*
 * This function runs when a BPS Fault should occur
 * It turns on the strobe light, and changes the mbms status
 * Switches car state to BPS_Fault !!!
 */
void enter_BPS_FAULT() {
	// strpbe enable
	HAL_GPIO_WritePin(Strobe_En_GPIO_Port, Strobe_En_Pin, 1);

	//red
	HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GRN_LED_GPIO_Port, GRN_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(BLU_LED_GPIO_Port, BLU_LED_Pin, GPIO_PIN_SET);

//	perms.common = 0;
//	perms.motor = 0;
//	perms.array = 0;
//	perms.lv = 0;
//	perms.charge = 0;
	perms.faulted = 1;

	mbmsStatus.carState = BPS_FAULT;

	osStatus_t a = osMutexAcquire(MBMSStatusMutexHandle, 5);
	if(a == osOK) {
		// update mbms status
		mbmsStatus.strobeBMSLight = 1;
		osMutexRelease(MBMSStatusMutexHandle);

	}

	osEventFlagsSet(shutoffFlagHandle, (HARD_BL_FLAG | SHUTOFF_FLAG));
	// delay for shutdown to run.... although rn its a higher priority so..
	//osDelay(10);
	// idk if soft battery limit has any purpose in shutoff procedure anymore, since when i talked
	// to jenny today, she said soft battery limit should just be a warning thru CAN and thats it.... may 10

}

void enter_SOFT_TRIP() {
	mbmsStatus.carState = SOFT_TRIP;
	//magenta
	HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BLU_LED_GPIO_Port, BLU_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GRN_LED_GPIO_Port, GRN_LED_Pin, GPIO_PIN_SET);
	perms.faulted = 1;
}

void enter_CHARGING() {

	//orange/yellow
	HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GRN_LED_GPIO_Port, GRN_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BLU_LED_GPIO_Port, BLU_LED_Pin, GPIO_PIN_SET);

	mbmsStatus.carState = CHARGING;
//	HAL_GPIO_WritePin(GRN_LED_GPIO_Port, GRN_LED_Pin, GPIO_PIN_SET);
}

void enter_FULLY_OPERATIONAL() {
	mbmsStatus.carState = FULLY_OPERATIONAL;
	//green
	HAL_GPIO_WritePin(GRN_LED_GPIO_Port, GRN_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BLU_LED_GPIO_Port, BLU_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);
}

void SystemStateMachine() {

	for (int i = 0; i < NUM_OF_CONTACTORS; i++) {
		if(contactorInfo[i].contactorClosed == CLOSE_CONTACTOR) {
			switch(i) {
				case COMMON:
					HAL_GPIO_WritePin(G1_GPIO_Port, G1_Pin, CONTACTOR_LED_ACTIVE);
					break;
				case MOTOR:
					HAL_GPIO_WritePin(G2_GPIO_Port, G2_Pin, CONTACTOR_LED_ACTIVE);
					break;
				case CHARGE:
					HAL_GPIO_WritePin(G5_GPIO_Port, G5_Pin, CONTACTOR_LED_ACTIVE);
					break;

			}
		}
		else {
			switch(i) {
				case COMMON:
					HAL_GPIO_WritePin(G1_GPIO_Port, G1_Pin, !CONTACTOR_LED_ACTIVE);
					break;
				case MOTOR:
					HAL_GPIO_WritePin(G2_GPIO_Port, G2_Pin, !CONTACTOR_LED_ACTIVE);
					break;
				case CHARGE:
					HAL_GPIO_WritePin(G5_GPIO_Port, G5_Pin, !CONTACTOR_LED_ACTIVE);
					break;

			}
		}
	}

	// make var plugged for now to stand in for the CAN msg that charger is plugged in or not
	uint8_t plugged = (read_CHARGE_PLUGGED() == CHARGE_PLUGGED_ACTIVE);



	switch (mbmsStatus.carState) {
		case BOOT:
			static uint32_t boot_counter = 0;
			boot_counter++;
//			if((orionMessagesReceived == 0x7) && boot_counter >= 500) { //ik i dont have to check here but i just am
//				carState = STARTUP;
//			}

			if((pack_info_count >= 5) && (temp_info_count >= 5)
					&& (cell_voltages_count >=5) && heartbeat_update_count >= 15
					&& mbmsStatus.orionCANReceived)
			{
				mbmsStatus.carState = STARTUP;
			}

			break;

		case STARTUP:

			// will go to BPS_FAULT state if startup checks do not pass
			startupCheck();

			// checks MPS
			if(read_nMPS() == nMPS_ACTIVE) {
				enter_MPS_DISCONNECTED();
				break;
			}

			if(read_ESD() == ESD_ACTIVE) {
				mbmsTrip.ESDEnabledTrip = 1;
				enter_BPS_FAULT();
			}

			if (mbmsStatus.startupState == COMPLETED){
				enter_FULLY_OPERATIONAL();
			}

			break;

		case FULLY_OPERATIONAL:

//			static uint32_t start_tick_fully_op = 0;
//			start_tick_fully_op = osKernelGetTickCount();
//
//			start_tick_fully_op = toggle_led(GRN, 500, start_tick_fully_op);

			// DEBUG : REMOVE THIS AFTER GORL
#if 0
			perms.charge = 1;
#endif

			if(read_nMPS() == nMPS_ACTIVE) {
				enter_MPS_DISCONNECTED();
				break;
			}


			if (plugged && (read_Charge_Enable() == CHARGE_ENABLE_ACTIVE)) {
//				perms.lv = 0;
				perms.motor = 0;
				HAL_GPIO_WritePin(_12V_CAN_En_GPIO_Port, _12V_CAN_En_Pin, !(_12V_CAN_EN_ACTIVE)); // disable 12V CAN
			}
			else {
//				perms.lv = 1;
				perms.motor = 1;
				HAL_GPIO_WritePin(_12V_CAN_En_GPIO_Port, _12V_CAN_En_Pin, _12V_CAN_EN_ACTIVE);
	            /* Khadeeja: Added below lines because we now have 4 CCP boards and array and charge are connected */
				/* ERM I COMMENTED THIS OUT BUT IDK WHAT THE CODE USED TO BE BUT PROBABLY CHILL
				if (read_Charge_Enable() == CHARGE_ENABLE_ACTIVE){
					// turn on the array
					perms.array = 1;
					perms.charge = 1;
				}
				*/
				/* end of Khadeeja edit */
			}

			if( plugged && (contactorInfo[MOTOR].contactorClosed == OPEN_CONTACTOR)) {
				HAL_GPIO_WritePin(nCHG_LV_En_GPIO_Port, nCHG_LV_En_Pin, !nCHG_LV_EN_ACTIVE); // enable charging
//				perms.charge = 1;
//				perms.array = 1; // NEW MILLAINE
				//enter_CHARGING(); //DEBUG!
			}

//			if (plugged && (contactorInfo[CHARGE].contactorClosed == CLOSE_CONTACTOR) && (contactorInfo[ARRAY].contactorClosed == CLOSE_CONTACTOR)) {
			if (plugged && (contactorInfo[CHARGE].contactorClosed == CLOSE_CONTACTOR)) {
				enter_CHARGING(); //DEBUG!
			}

			/* Running checks */
			if (heartbeat_check_count <= heartbeat_update_count) {
				CheckContactorHeartbeats();
			}

			CheckSoftBatteryLimit();
			UpdateTripStatus();

			break;

		case CHARGING:
			// turns off car if key is off
			checkKeyShutdown();
			HAL_GPIO_WritePin(GRN_LED_GPIO_Port, GRN_LED_Pin, GPIO_PIN_RESET);

			if(read_nMPS() == nMPS_ACTIVE) {
				enter_MPS_DISCONNECTED();
				break;
			}

			// checks if charger is unplugged
			if (!plugged && (read_Discharge_Enable() == DISCHARGE_ENABLE_ACTIVE)) {
				HAL_GPIO_WritePin(nCHG_LV_En_GPIO_Port, nCHG_LV_En_Pin, nCHG_LV_EN_ACTIVE); // disable charging
				perms.motor = 1;
				//perms.charge = 0;
			}
//			if(contactorInfo[CHARGE].contactorClosed == OPEN_CONTACTOR) {
//				HAL_GPIO_WritePin(_12V_CAN_En_GPIO_Port, _12V_CAN_En_Pin, GPIO_PIN_SET); // enable 12V CAN PAUSE I SHOULD PRECHARGE STOOPID
////				perms.lv = 1;
//				perms.motor = 1;
//			}
			if((contactorInfo[MOTOR].contactorClosed == CLOSE_CONTACTOR)) {
				HAL_GPIO_WritePin(_12V_PCHG_En_GPIO_Port, _12V_PCHG_En_Pin, _12V_PCHG_EN_ACTIVE); // turn on precharge
				if(read_Critical_OV_UV() == CRITICAL_OV_UV_ACTIVE) { // if equals 0 good to go
					HAL_GPIO_WritePin(_12V_CAN_En_GPIO_Port, _12V_CAN_En_Pin, _12V_CAN_EN_ACTIVE); // anable 12V CAN
					HAL_GPIO_WritePin(_12V_PCHG_En_GPIO_Port, _12V_PCHG_En_Pin, !(_12V_PCHG_EN_ACTIVE));
					enter_FULLY_OPERATIONAL();
				}
			}



			/* Running checks */
			CheckContactorHeartbeats();
			CheckSoftBatteryLimit();
			UpdateTripStatus();

			break;

		case BPS_FAULT:
			break;

		case MPS_DISCONNECTED:
			break;

		case SOFT_TRIP:

			if (softBatteryTrip.cell_OV == 1){
				perms.charge = 0;
//				perms.array = 0;

			}
			if(softBatteryTrip.cell_UV == 1) {
				perms.motor = 0;
			}
			if(read_nMPS() == nMPS_ACTIVE) {
				enter_MPS_DISCONNECTED();
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
    for (int i = 0; i < NUM_OF_CONTACTORS; i++) {
        if (contactorInfo[i].contactorClosing == CLOSING_CONTACTOR) { // lowkey switch this back to an enum if u have time smh.. stoopid fr
            contactorClosing = true;
            break;
        }
    }

    // If no contactors are currently closing, and battery not in fault type state (MPS, BPS)
    if (!contactorClosing && !perms.faulted) {
        if ((perms.common) && (contactorInfo[COMMON].contactorClosed != CLOSE_CONTACTOR) && (contactorCommand.common != CLOSE_CONTACTOR)) {
            contactorCommand.common = CLOSE_CONTACTOR;
            contactor_command_start_tick[COMMON] = osKernelGetTickCount();
//            sendContactorCommand = 1; // had this here before but i think ill just consistently send lowkey..
        }
//        else if ((perms.lv) && (contactorInfo[LOWV].contactorClosed != CLOSE_CONTACTOR)
//        		&& (mbmsStatus.dischargeEnable == DISCHARGE_ENABLE_ACTIVE) && (contactorCommand.LV != CLOSE_CONTACTOR)) {
//
//            contactorCommand.motor = CLOSE_CONTACTOR;
//            contactor_command_start_tick[MOTOR] = osKernelGetTickCount();
////        	contactorCommand.LV = CLOSE_CONTACTOR;
////            contactor_command_start_tick[LOWV] = osKernelGetTickCount();
//
//        }
        else if ((perms.motor) && (contactorInfo[MOTOR].contactorClosed != CLOSE_CONTACTOR)
        		&& (mbmsStatus.dischargeEnable == DISCHARGE_ENABLE_ACTIVE) && (contactorCommand.motor != CLOSE_CONTACTOR)) {
            contactorCommand.motor = CLOSE_CONTACTOR;
            contactor_command_start_tick[MOTOR] = osKernelGetTickCount();

        }
//        else if ((perms.array) && (contactorInfo[ARRAY].contactorClosed != CLOSE_CONTACTOR)
//        		&& (mbmsStatus.chargeEnable == CHARGE_ENABLE_ACTIVE) && (contactorCommand.array != CLOSE_CONTACTOR)) {
//        	contactorCommand.array = CLOSE_CONTACTOR;
//            contactor_command_start_tick[ARRAY] = osKernelGetTickCount();
//
//        }
        else if ((perms.charge) && (contactorInfo[CHARGE].contactorClosed != CLOSE_CONTACTOR)
        		&& (mbmsStatus.chargeEnable == CHARGE_ENABLE_ACTIVE) && (contactorCommand.charge != CLOSE_CONTACTOR)) {
            contactorCommand.charge = CLOSE_CONTACTOR;
            contactor_command_start_tick[CHARGE] = osKernelGetTickCount();

        }
    }

    // Open contactors as needed
    if ((!perms.common) && (contactorCommand.common != OPEN_CONTACTOR)) { // tbh i lowkey do not even want to check contactor state, just perms but idk hehe
    	contactorCommand.common = OPEN_CONTACTOR;
    	contactor_command_start_tick[COMMON] = osKernelGetTickCount();
    }
    if ((!perms.motor) && (contactorCommand.motor != OPEN_CONTACTOR)) {
        contactorCommand.motor = OPEN_CONTACTOR;
        contactor_command_start_tick[MOTOR] = osKernelGetTickCount();
    }

//    if ((!perms.array) && (contactorCommand.array != OPEN_CONTACTOR)) {
//        contactorCommand.array = OPEN_CONTACTOR;
//
//        contactor_command_start_tick[ARRAY] = osKernelGetTickCount();
//
//    }

//    if ((!perms.lv) && (contactorCommand.LV != OPEN_CONTACTOR)) {
//        contactorCommand.motor = OPEN_CONTACTOR;
//        contactor_command_start_tick[MOTOR] = osKernelGetTickCount();
//
////        contactorCommand.LV = OPEN_CONTACTOR;
////        contactor_command_start_tick[LOWV] = osKernelGetTickCount();
//
//    }

    if ((!perms.charge) && (contactorCommand.charge != OPEN_CONTACTOR)) {
        contactorCommand.charge = OPEN_CONTACTOR;
        contactor_command_start_tick[CHARGE] = osKernelGetTickCount();

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
	if (((previousHeartbeats[0] == 0) || (previousHeartbeats[1] == 0) || (previousHeartbeats[2] == 0) ||
		   (previousHeartbeats[3] == 0) || (previousHeartbeats[4] == 0))) // was != 0 oops!
	{
		// set heartbeatDead so we can break out of while loop lol
		if( heartbeat_check_count <= heartbeat_update_count) {
			heartbeatDead = waitForFirstHeartbeats();
		}

	}
	if (heartbeatDead == 1){


		// KHADEEJA: CHANGE if 0 to 1
#if 0
		enter_BPS_FAULT();
#endif
	}

	/* Check to ensure no contactors are closed */
	if ((checkContactorsOpen() == 0) || checkPrechargersOpen() == 0){
		enter_BPS_FAULT();
	}

	/* Battery check (orion) */
	uint8_t passedBatteryCheck = startupBatteryCheck(); // this func actually only checks hard limits for now...
	if (!passedBatteryCheck) {
		enter_BPS_FAULT();
	}

}


uint8_t waitForFirstHeartbeats() {

	static uint8_t heartbeatFailCounter[NUM_OF_CONTACTORS] = {0};
	uint8_t dead = 0;


	for(int i = 0; i < NUM_OF_CONTACTORS; i++) {
		heartbeat_check_count++;

		if(heartbeatFailCounter[i] > 3) {
			osStatus_t a = osMutexAcquire(MBMSTripMutexHandle, 5);
			if(a == osOK) {
				switch (i) {
					case COMMON:
						mbmsTrip.commonHeartbeatDeadTrip = 1;
						break;
					case MOTOR:
						mbmsTrip.motorHeartbeatDeadTrip = 1;
						break;
					case CHARGE:
						mbmsTrip.chargeHeartbeatDeadTrip = 1;
						break;
				}
				osMutexRelease(MBMSTripMutexHandle);

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
				if(((osKernelGetTickCount() - heartbeatLastUpdatedTime[i])) > CONTACTOR_HEARTBEAT_TIMEOUT) { // where contactor_heartbeat_timeout is how often a heartbeat is sent out/recieved
					heartbeatFailCounter[i]++;

				}
			}
			else {
				heartbeatLastUpdatedTime[i] = osKernelGetTickCount();
				heartbeatFailCounter[i] = 0;
			}
				previousHeartbeats[i] = (contactorInfo[i].heartbeat);
			osMutexRelease(ContactorInfoMutexHandle);

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
	osStatus_t acquire = osMutexAcquire(ContactorInfoMutexHandle, 5);
	if (acquire == osOK) {

		for (int i = 0; i < NUM_OF_CONTACTORS; i++) {
			if (contactorInfo[i].contactorClosed == CLOSE_CONTACTOR) {
				allOpen = 0;
				switch(i) {
					case COMMON:
						mbmsTrip.commonHeartbeatDeadTrip = 1;
						break;
					case MOTOR:
						mbmsTrip.motorHeartbeatDeadTrip = 1;
						break;

					case CHARGE:
						mbmsTrip.chargeHeartbeatDeadTrip = 1;
						break;
				}
				break;
			}
		}

		osMutexRelease(ContactorInfoMutexHandle);

	}

	return allOpen;
}

/*
 * returns whether any prechargers are closed (0) or not (1).
 */
// PROBLEM: this does not have a specific trip for it yet... it just goes to BPS fault...
uint8_t checkPrechargersOpen() {

	uint8_t allOpen = 1;
	osStatus_t acquire = osMutexAcquire(ContactorInfoMutexHandle, 5);
	if (acquire == osOK) {

		for (int i = 1; i < NUM_OF_CONTACTORS; i++) { //COMMON HAS NO PRECHARGER which is why i = 1
			if (contactorInfo[i].prechargerClosed == CLOSE_CONTACTOR) {
				allOpen = 0;
				break;
			}
		}
		osMutexRelease(ContactorInfoMutexHandle);

	}

	return allOpen;

}

uint8_t startupBatteryCheck() {
	// maybe set the flag for hard batt lim, soft batt lim here? or idk loll
	// or just have a var somewhere to keep track and check the var in the check if shutdown stuff?
	// in case theres multiple things wrong so you can store all the trips before yk, doing whatever BPS procedure

	uint8_t safe = 1;
	// check this mutex stuff ngl...
	osStatus_t acquire = osMutexAcquire(MBMSTripMutexHandle, 5);
	if(acquire == osOK) {

		if(batteryInfo.highCellVoltage > HARD_MAX_CELL_VOLTAGE){
			mbmsTrip.highCellVoltageTrip = 1;
			safe = 0;
		}

		if(batteryInfo.lowCellVoltage < HARD_MIN_CELL_VOLTAGE) {
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

		osMutexRelease(MBMSTripMutexHandle);
	}

	startup_Check_Counter++; // lowkey i feel like is huld putr a mutex around this but maybe dis can be a later problems

	return safe;

}


/* FUNCTIONS RELATED TO GENERAL TRIPS & SOFT LIMITS */


/*
 * This function turns off charging when key is off to shut off car
 */
void checkKeyShutdown() {
	if (read_Key() == !KEY_ENABLE_ACTIVE) {
		// turn off charge LV enable to shutoff car..
		HAL_GPIO_WritePin(nCHG_LV_En_GPIO_Port, nCHG_LV_En_Pin, nCHG_LV_EN_ACTIVE);

	}
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
	for(int i = 0; i < NUM_OF_CONTACTORS; i++) {

		heartbeat_check_count++;

		if(previousHeartbeats[i] >= 65535) { // check this logic lol
			previousHeartbeats[i] = 0;
		}


		if(previousHeartbeats[i] >= contactorInfo[i].heartbeat){
			uint32_t difference_ticks = osKernelGetTickCount() - heartbeatLastUpdatedTime[i];
			float difference_ms = (float) difference_ticks;
			if((difference_ms) > CONTACTOR_HEARTBEAT_TIMEOUT) {

				osStatus_t acquire = osMutexAcquire(MBMSTripMutexHandle, UPDATING_MUTEX_TIMEOUT);
				if(acquire == osOK) {
					// set heartbeat dead trip
					switch (i) {
						case COMMON:
							mbmsTrip.commonHeartbeatDeadTrip = 1;
							break;
						case MOTOR:
							mbmsTrip.motorHeartbeatDeadTrip = 1;
							break;
//
						case CHARGE:
							mbmsTrip.chargeHeartbeatDeadTrip = 1;
							break;
						default:
							// do nothing
							break;

					}
					osMutexRelease(MBMSTripMutexHandle);
					BPSFault = 1;

				}
			}
		}
		else {
			heartbeatLastUpdatedTime[i] = osKernelGetTickCount();
		}

		osStatus_t a = osMutexAcquire(ContactorInfoMutexHandle, READING_MUTEX_TIMEOUT);
		if (a == osOK) {
			previousHeartbeats[i] = (contactorInfo[i].heartbeat);
			osMutexRelease(ContactorInfoMutexHandle);
		}

	}

	if(BPSFault) {

		// KHADEEJA: CHANGE IF 0 to 1
#if 0
		enter_BPS_FAULT();
#endif
	}
}


/* This function checks the soft limits of voltages, currents, and temperatures
 * These warnings should be sent out in a CAN message (CANMessageSender -> CANTxGatekeeper)
 * You can think of these like a warning on your phone that it's low battery
 * Don't need to do anything for these soft limits, just send the warning!
 */
void CheckSoftBatteryLimit() {

	uint8_t trip = 0;

	/// ummmmm be careful deadlock mauybe check everything once ur done all the mutexes
	osStatus_t acquire = osMutexAcquire(MBMSSoftLimitWarningMutexHandle, UPDATING_MUTEX_TIMEOUT);
	if(acquire == osOK) {

		osStatus_t a1 = osMutexAcquire(BatteryInfoMutexHandle, UPDATING_MUTEX_TIMEOUT);
		if (a1 == osOK){
			/* Checking the min/max cell voltages */
			if (batteryInfo.highCellVoltage > SOFT_MAX_CELL_VOLTAGE) {
				softBatteryTrip.cell_OV = 1;
				trip = 1;
				mbmsSoftBatteryLimitWarning.highCellVoltageWarning = 1;

			}
			if (batteryInfo.lowCellVoltage < SOFT_MIN_CELL_VOLTAGE) {
				softBatteryTrip.cell_UV = 1;
				trip = 1;
				mbmsSoftBatteryLimitWarning.lowCellVoltageWarning = 1;
			}

			/* Checking high/low temperature */
			if (batteryInfo.highTemp > SOFT_MAX_TEMP) {
				mbmsSoftBatteryLimitWarning.highTemperatureWarning = 1;
			}
			if (batteryInfo.lowTemp < SOFT_MIN_TEMP) {
				mbmsSoftBatteryLimitWarning.lowTemperatureWarning = 1;
			}

			osMutexRelease(BatteryInfoMutexHandle);

		}

		osStatus_t a2 = osMutexAcquire(ContactorInfoMutexHandle, READING_MUTEX_TIMEOUT);
		if (a2 == osOK) {
			/* Checking contactors' high current */
			if (batteryInfo.packCurrent > SOFT_MAX_COMMON_CONTACTOR_CURRENT){
				mbmsSoftBatteryLimitWarning.commonHighCurrentWarning = 1;
			}
			if (contactorInfo[MOTOR].lineCurrent > SOFT_MAX_MOTORS_CONTACTOR_CURRENT){
				mbmsSoftBatteryLimitWarning.motorHighCurrentWarning = 1;
			}
//			if (contactorInfo[ARRAY].lineCurrent > SOFT_MAX_ARRAY_CONTACTOR_CURRENT){
//				mbmsSoftBatteryLimitWarning.arrayHighCurrentWarning = 1;
//			}
//			if (contactorInfo[LOWV].lineCurrent > SOFT_MAX_LV_CONTACTOR_CURRENT){
//				mbmsSoftBatteryLimitWarning.LVHighCurrentWarning = 1;
//			}
			if (contactorInfo[CHARGE].lineCurrent > SOFT_MAX_CHARGE_CONTACTOR_CURRENT){
				mbmsSoftBatteryLimitWarning.chargeHighCurrentWarning = 1;
			}
			osMutexRelease(ContactorInfoMutexHandle);
		}

		osMutexRelease(MBMSSoftLimitWarningMutexHandle);
	}

	if (trip) {
		enter_SOFT_TRIP();
	}

}

void UpdateTripStatus() {

	static uint8_t BPS_Fault = 0;
	osStatus_t acquire = osMutexAcquire(MBMSTripMutexHandle, UPDATING_MUTEX_TIMEOUT);
	if (acquire == osOK){

		osStatus_t a1 = osMutexAcquire(ContactorInfoMutexHandle, 5);
		if (a1 == osOK){

			if (batteryInfo.packCurrent > HARD_MAX_COMMON_CONTACTOR_CURRENT){
				hard_high_current_count[COMMON]++;
				if((hard_high_current_count[COMMON] * 10) > HARD_CURRENT_TRIP_TIMEOUT) { // because runs every 10 seconds lol lowkey i may have not done this for other things but just adjust them as needed i guess... ?
					mbmsTrip.commonHighCurrentTrip = 1;
					BPS_Fault = 1;
				}

			}
			else {
				hard_high_current_count[COMMON] = 0;
			}

			/* not using HIGH CURRENT TRIPS as of now. May 17. */
			/* ugh using them again june 19 smh */
#if 1
			if ((contactorInfo[MOTOR].lineCurrent > HARD_MAX_MOTORS_CONTACTOR_CURRENT)){
				hard_high_current_count[MOTOR]++;
				if((hard_high_current_count[MOTOR] * 10) > HARD_CURRENT_TRIP_TIMEOUT) {
					mbmsTrip.motorHighCurrentTrip = 1;
					BPS_Fault = 1;
				}
			}
			else {
				hard_high_current_count[MOTOR] = 0;
			}

//			if (contactorInfo[ARRAY].lineCurrent > HARD_MAX_ARRAY_CONTACTOR_CURRENT){
//				hard_high_current_count[ARRAY]++;
//				if((hard_high_current_count[ARRAY] * 10) > HARD_CURRENT_TRIP_TIMEOUT) {
//					mbmsTrip.arrayHighCurrentTrip = 1;
//
//#if 1
//					BPS_Fault = 1;
//
//#endif
//				}
//			}
//			else {
//				hard_high_current_count[ARRAY] = 0;
//			}

//			if (contactorInfo[LOWV].lineCurrent > HARD_MAX_LV_CONTACTOR_CURRENT){
//				hard_high_current_count[LOWV]++;
//				if((hard_high_current_count[LOWV] * 10) > HARD_CURRENT_TRIP_TIMEOUT) {
//					mbmsTrip.LVHighCurrentTrip = 1;
//					BPS_Fault = 1;
//				}
//			}
//			else {
//				hard_high_current_count[LOWV] = 0;
//			}


			if (contactorInfo[CHARGE].lineCurrent > HARD_MAX_CHARGE_CONTACTOR_CURRENT){
				hard_high_current_count[CHARGE]++;
				if((hard_high_current_count[CHARGE] * 10) > HARD_CURRENT_TRIP_TIMEOUT) {
					mbmsTrip.chargeHighCurrentTrip = 1;
					BPS_Fault = 1;
				}
			}
			else {
				hard_high_current_count[CHARGE] = 0;
			}
#endif


			/* Not using PROTECTION TRIP as of now. May 17. */
			/*
			if ((contactorInfo[CHARGE].lineCurrent > 0) || (contactorInfo[LOWV].lineCurrent < 0)){
				mbmsTrip.protectionTrip = 1;
				BPS_Fault = 1;
			}
			 */

			osMutexRelease(ContactorInfoMutexHandle);

		}

		osStatus_t a2 = osMutexAcquire(BatteryInfoMutexHandle, 5);
		if (a2 == osOK){

			/* checking for high/low cell voltage trips */
			if(batteryInfo.highCellVoltage > HARD_MAX_CELL_VOLTAGE){
				mbmsTrip.highCellVoltageTrip = 1;
				BPS_Fault = 1;
			}

			if(batteryInfo.lowCellVoltage < HARD_MIN_CELL_VOLTAGE) {
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

			osMutexRelease(BatteryInfoMutexHandle);

		}

		osStatus_t a3 = osMutexAcquire(MBMSStatusMutexHandle, 5);
		if (a3 == osOK) {
			// if orion can message wasn't received recently, set trip
			if (!(mbmsStatus.orionCANReceived)) {
				mbmsTrip.orionMessageTimeoutTrip = 1;
				BPS_Fault = 1;
			}

			osMutexRelease(MBMSStatusMutexHandle);

		}

		osStatus_t a4 = osMutexAcquire(ContactorCommandMutexHandle, 5);
		if(a4 == osOK) {

			/* Contactor disconnected unexpectedely */
			/* To check, we compare a minimum current draw with the state of the contactor */
//			if(		   ((contactorCommand.common == CLOSE_CONTACTOR) && (batteryInfo.packCurrent < NO_CURRENT_THRESHOLD)
//						 && ((osKernelGetTickCount() - contactor_command_start_tick[COMMON]) >= CLOSE_CONTACTOR_TIMEOUT))
//					|| ((contactorCommand.motor == CLOSE_CONTACTOR) && (contactorInfo[MOTOR].lineCurrent < NO_CURRENT_THRESHOLD)
//						 && ((osKernelGetTickCount() - contactor_command_start_tick[MOTOR]) >= CLOSE_CONTACTOR_TIMEOUT))
//					|| ((contactorCommand.array  == CLOSE_CONTACTOR) && (contactorInfo[ARRAY].lineCurrent  < NO_CURRENT_THRESHOLD)
//						 && ((osKernelGetTickCount() - contactor_command_start_tick[ARRAY]) >= CLOSE_CONTACTOR_TIMEOUT))
//					|| ((contactorCommand.LV     == CLOSE_CONTACTOR) && (contactorInfo[LOWV].lineCurrent   < NO_CURRENT_THRESHOLD)
//						 && ((osKernelGetTickCount() - contactor_command_start_tick[LOWV]) >= CLOSE_CONTACTOR_TIMEOUT))
//					|| ((contactorCommand.charge == CLOSE_CONTACTOR) && (contactorInfo[CHARGE].lineCurrent < NO_CURRENT_THRESHOLD)
//						 && ((osKernelGetTickCount() - contactor_command_start_tick[CHARGE]) >= CLOSE_CONTACTOR_TIMEOUT))
//				)

			if(		   ((contactorCommand.common == CLOSE_CONTACTOR) && (batteryInfo.packCurrent < NO_CURRENT_THRESHOLD)
						 && ((osKernelGetTickCount() - contactor_command_start_tick[COMMON]) >= CLOSE_CONTACTOR_TIMEOUT))
					|| ((contactorCommand.motor == CLOSE_CONTACTOR) && (contactorInfo[MOTOR].lineCurrent < NO_CURRENT_THRESHOLD)
						 && ((osKernelGetTickCount() - contactor_command_start_tick[MOTOR]) >= CLOSE_CONTACTOR_TIMEOUT))
					|| ((contactorCommand.charge == CLOSE_CONTACTOR) && (contactorInfo[CHARGE].lineCurrent < NO_CURRENT_THRESHOLD)
						 && ((osKernelGetTickCount() - contactor_command_start_tick[CHARGE]) >= CLOSE_CONTACTOR_TIMEOUT))
				)
			{
				mbmsTrip.contactorDisconnectedUnexpectedlyTrip = 1;
				//BPS_Fault = 1;

			}

			/* Contactor connected unexpectedly trip */
//			if(((		 contactorCommand.common == OPEN_CONTACTOR) && (batteryInfo.packCurrent >= NO_CURRENT_THRESHOLD)
//						 && ((osKernelGetTickCount() - contactor_command_start_tick[COMMON]) >= OPEN_CONTACTOR_TIMEOUT))
//					|| ((contactorCommand.motor == OPEN_CONTACTOR) && (contactorInfo[MOTOR].lineCurrent >= NO_CURRENT_THRESHOLD)
//						 && ((osKernelGetTickCount() - contactor_command_start_tick[MOTOR]) >= OPEN_CONTACTOR_TIMEOUT))
//					|| ((contactorCommand.array  == OPEN_CONTACTOR) && (contactorInfo[ARRAY].lineCurrent  >= NO_CURRENT_THRESHOLD)
//						 && ((osKernelGetTickCount() - contactor_command_start_tick[ARRAY]) >= OPEN_CONTACTOR_TIMEOUT))
//					|| ((contactorCommand.LV     == OPEN_CONTACTOR) && (contactorInfo[LOWV].lineCurrent   >= NO_CURRENT_THRESHOLD)
//						 && ((osKernelGetTickCount() - contactor_command_start_tick[LOWV]) >= OPEN_CONTACTOR_TIMEOUT))
//					|| ((contactorCommand.charge == OPEN_CONTACTOR) && (contactorInfo[CHARGE].lineCurrent >= NO_CURRENT_THRESHOLD)
//						 && ((osKernelGetTickCount() - contactor_command_start_tick[CHARGE]) >= OPEN_CONTACTOR_TIMEOUT))
//				)
			if(((		 contactorCommand.common == OPEN_CONTACTOR) && (batteryInfo.packCurrent >= NO_CURRENT_THRESHOLD)
						 && ((osKernelGetTickCount() - contactor_command_start_tick[COMMON]) >= OPEN_CONTACTOR_TIMEOUT))
					|| ((contactorCommand.motor == OPEN_CONTACTOR) && (contactorInfo[MOTOR].lineCurrent >= NO_CURRENT_THRESHOLD)
						 && ((osKernelGetTickCount() - contactor_command_start_tick[MOTOR]) >= OPEN_CONTACTOR_TIMEOUT))
					|| ((contactorCommand.charge == OPEN_CONTACTOR) && (contactorInfo[CHARGE].lineCurrent >= NO_CURRENT_THRESHOLD)
						 && ((osKernelGetTickCount() - contactor_command_start_tick[CHARGE]) >= OPEN_CONTACTOR_TIMEOUT))
				)
			{
				mbmsTrip.contactorConnectedUnexpectedlyTrip = 1;
#if 1
				BPS_Fault = 1;
#endif

//make it aboslute, make cyrrent threshold 1
			}

			/* Here, it is also a contactor connected unexpectedly trip if the contactor won't open when told to */
			for (int i = 0; i < NUM_OF_CONTACTORS; i++) {
				if(contactorInfo[i].contactorOpeningError == 1) {
					mbmsTrip.contactorConnectedUnexpectedlyTrip = 1;
					BPS_Fault = 1;
					break;
				}
			}



			osMutexRelease(ContactorCommandMutexHandle);

		}

		if(read_LV_OC() == LV_OC_ACTIVE) {
			mbmsSoftBatteryLimitWarning._12V_CAN_OC_Warning = 1;
			LV_OC_tick_count++; // erm actually does it get here every millisecond tho.. idont think so
			if ((LV_OC_tick_count) >= LV_OC_TIMEOUT ) {
				// turn off 12V Can.....
				HAL_GPIO_WritePin(_12V_CAN_En_GPIO_Port, _12V_CAN_En_Pin, !(_12V_CAN_EN_ACTIVE));
				// not a bps fault, lowkey do nothing else, driver should deal with it ...
			}
		}
		else {
			LV_OC_tick_count = 0;
			mbmsSoftBatteryLimitWarning._12V_CAN_OC_Warning = 0;
		}

		// this is techincally not a "trip" that will cause BPS....
		// its just for information purposes i suppose
		if(read_nMPS() == nMPS_ACTIVE){
			mbmsTrip.MPSDisabledTrip = 1;
			enter_MPS_DISCONNECTED();

		}

		if(read_ESD() == ESD_ACTIVE){
			mbmsTrip.ESDEnabledTrip = 1;
			BPS_Fault = 1;
		}

		osMutexRelease(MBMSTripMutexHandle);

		if(BPS_Fault) {
			enter_BPS_FAULT();
		}

	}


}




void UpdateScreenDataStructs(void){
	screenData.batteryInfo = convertToScreenBatteryInfo(&batteryInfo);
	screenData.powerStatus = powerSelectionStatus;
	screenData.tripScreen =  convertToTripScreen(&mbmsTrip);
	screenData.mbmsStatus = convertToMBMSStatusScreen(&mbmsStatus);
	screenData.contactorScreen = convertToContactorScreen(contactorInfo);
}



