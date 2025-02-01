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


//#define CLOSED 0x0
//#define OPEN 0x1

// can change these later, discuss w khadeeja
#define OPEN_CONTACTOR GPIO_PIN_RESET // 0
#define CLOSE_CONTACTOR GPIO_PIN_SET // 1

// These are random values i made for the batt limit, must be updated
#define HARD_BATTERY_LIMIT 75
#define SOFT_BATTERY_LIMIT 12

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
	uint8_t highVoltageEnableState;
	uint8_t allowDischarge;
	uint8_t orionCANReceived;
	uint8_t dischargeTrip;
	uint8_t chargeTrip;
	uint8_t startupState;

} MBMSStatus;

typedef struct {
	uint8_t highCellVoltageTrip;
	uint8_t lowCellVoltageTrip;
	uint8_t highCommonCurrentTrip;
	uint8_t motorHighTempCurrentTrip;
	uint8_t arrayHighTempCurrentTrip;
	uint8_t LVHighTempCurrentTrip;
	uint8_t chargeHighTempTrip;
	uint8_t protectionTrip;
	uint8_t orionMessageTimeoutTrip;
	uint8_t contactorDisconnectedTrip;
	uint8_t mainPowerSwitchTrip;
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
