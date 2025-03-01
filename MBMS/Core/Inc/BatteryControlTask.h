/*
 * BatteryControlTask.hpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millaineli
 */

#ifndef INC_TASK_H_FILES_BATTERYCONTROLTASK_H_
#define INC_TASK_H_FILES_BATTERYCONTROLTASK_H_

#include <stdint.h>
#include "main.h"

void BatteryControlTask(void* arg);
void BatteryControl();
void updateContactorInfo(uint8_t contactor, uint8_t prechargerClosed, uint8_t prechargerCLosing, uint8_t prechargerError, uint8_t contactorClosed, uint8_t contactorClosing, uint8_t contactorError, uint16_t contactorCurrent, uint16_t contactorVoltage);
void updateContactorInfoStruct();
void sendTripStatusCanMessage(uint16_t * tripData);
void updatePackInfoStruct();
void updateTripStatus();
void sendMBMSStatusCanMessage();
void checkIfShutdown();
void updateContactors();


//#define CLOSED 0x0
//#define OPEN 0x1

// can change these later, discuss w khadeeja
#define OPEN_CONTACTOR GPIO_PIN_RESET // 0
#define CLOSE_CONTACTOR GPIO_PIN_SET // 1

// These are random values i made for the batt limit, must be updated
#define HARD_BATTERY_LIMIT 75
#define SOFT_BATTERY_LIMIT 73
// wait.... are these the limits for the cell or the pack ????

#define MAX_CELL_VOLTAGE 10
#define MIN_CELL_VOLTAGE 2

#define MAX_CONTACTOR_CURRENT 5

#define ORION_MSG_WAIT_TIMEOUT 600 //ms

#define SOC_SAFE_FOR_CHARGE 90 // maybe can do if SOC is less than 90, safe to charge
#define SOC_SAFE_FOR_DISCHARGE 25 // maybe if SOC greater than this, safe to discharge ?

#define KEY_OFF 0
#define KEY_ON 1

#define MPS_ENABLED 1
#define MPS_DISABLED 0

#define NO_CURRENT_THRESHOLD 3 // (AMPS). So if less than this, consider no current, if more than this, consider there is current

#define MAX_PACK_VOLTAGE 75 // (Volts)

enum Contactor {
	COMMON = 0,
	MOTOR1,
	MOTOR2,
	ARRAY,
	LOWV,
	CHARGE
};



enum startupStates {
	MPS_OPEN = 0,
	MPS_CLOSED,
	COMMON_CLOSED,
	LV_CLOSED,
	DCDC1_CLOSED,
	DCDC0_OPEN,
	MOTORS_ENABLED,
	FULLY_OPERATIONAL // this one is like array and charge perms given :)
};


typedef struct {
	// pack info
    int16_t packCurrent; // current can be -ve, 2-bytes
    uint16_t packVoltage; // 2-bytes
    uint8_t packSOC; // state of charge, 1-byte
    uint16_t packAmphours; // 2-bytes
    uint8_t packDOD; // Depth of Discharge, 1-byte
    // temperature info (each 1-byte)
    uint8_t highTemp;
    uint8_t lowTemp;
    uint8_t avgTemp;
    // voltage info (each 2-bytes)
    uint16_t maxCellVoltage;
    uint16_t minCellVoltage;
    uint16_t maxPackVoltage;
    uint16_t minPackVoltage;
} BatteryInfo;

typedef struct {
	uint8_t prechargerClosed;
	uint8_t prechargerClosing;
	uint8_t prechargerError;
	uint8_t contactorClosed;
	uint8_t contactorClosing;
	uint8_t contactorError;
	uint16_t voltage;
	uint16_t current;
	uint16_t heartbeat; // go up to 65000 ?
} ContactorInfo;

// add startup state
typedef struct {
	uint8_t auxilaryBattVoltage; // bits 0-4
	uint8_t strobeBMSLight;
	uint8_t allowCharge;
	uint8_t chargeSafety;
	uint8_t highVoltageEnableState; // what is this, why doesnt it exist anymore in the excel???
	uint8_t allowDischarge;
	uint8_t orionCANReceived;
	uint8_t dischargeShouldTrip;
	uint8_t chargeShouldTrip;
	uint8_t startupState;

} MBMSStatus;

typedef struct {
	uint8_t highCellVoltageTrip;
	uint8_t lowCellVoltageTrip;
	uint8_t commonHighCurrentTrip;
	uint8_t motorHighCurrentTrip;
	uint8_t arrayHighCurrentTrip;
	uint8_t LVHighCurrentTrip;
	uint8_t chargeHighCurrentTrip;
	uint8_t protectionTrip;
	uint8_t orionMessageTimeoutTrip;
	uint8_t contactorDisconnectedUnexpectedlyTrip;
	uint8_t contactorConnectedUnexpectedlyTrip;
	uint8_t highBatteryTrip;
} MBMSTrip;

typedef struct {
	uint8_t mainPowerSwitch;
	uint8_t DCDC1Enable;
	uint8_t nDCDC1Fault;
	uint8_t DCDC0_OV_Fault;
	uint8_t DCDC0_UV_Fault;
	uint8_t nDCDC0_On; // um this feels like an input to the bms
	uint8_t _3A_OC_UC;
	uint8_t nDCDC1_On;
	uint8_t nCHG_Fault;
	uint8_t nCHG_On;
	uint8_t ABATTDisable;
	uint8_t Key;
} PowerSelectionStatus;

typedef struct{
	uint8_t common;
	uint8_t motor1;
	uint8_t motor2;
	uint8_t array;
	uint8_t LV;
	uint8_t charge;
} ContactorState; // maybe i could use this struct for when its proper closed/proper open.. ? idk i might not even rly need this tsruct anymnore but idk

typedef struct{
	uint8_t common;
	uint8_t motor1;
	uint8_t motor2;
	uint8_t array;
	uint8_t LV;
	uint8_t charge;
} ContactorCommand;


// make struct w precharge, current, voltage, then have an array of 4 (for the 4 contactors) of the struct type
// add the individual contactor voltage and current




//// but this struct should be readable by diff tasks !!!! e.g. startup, shutoff
//// zero for closed (connected), one for open (disconnected)
//typedef struct{
//	uint8_t common;
//	uint8_t motor;
//	uint8_t array;
//	uint8_t LV;
//	uint8_t charge;
//} ContactorState;

//extern ContactorState contactorState = {0};


// so should probably dequeue CAN messages and update the values in this struct, and do appropriate checks of values


#endif /* INC_TASK_H_FILES_BATTERYCONTROLTASK_H_ */
