/*
 * MBMS.h
 *
 *  Created on: Mar 5, 2025
 *      Author: millaine
 */

#ifndef INC_MBMS_H_
#define INC_MBMS_H_

// can change these later, discuss w khadeeja
//#define OPEN_CONTACTOR GPIO_PIN_RESET // 0
//#define CLOSE_CONTACTOR GPIO_PIN_SET // 1

#define KEY_OFF 0
#define KEY_ON 1

#define MPS_ENABLED 1
#define MPS_DISABLED 0

#define FREERTOS_TICK_PERIOD 1/configTICK_RATE_HZ //USE THIS INSTEAD OF SECONFS PER TICK

// Define boolean as an enum or typedef in a header
typedef enum { false = 0, true = 1 } boolean;

// new enum made march 7
enum ContactorState {
	OPEN_CONTACTOR = 0,
	CLOSE_CONTACTOR,
	CLOSING_CONTACTOR

};

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
	uint8_t contactorState; // march 8: two bits become one variable, 00 = open, 01 = closed, 10 = closing
	uint8_t contactorError;
	uint16_t voltage;
	uint16_t current;
	uint16_t heartbeat;
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

//typedef struct{
//	uint8_t common;
//	uint8_t motor1;
//	uint8_t motor2;
//	uint8_t array;
//	uint8_t LV;
//	uint8_t charge;
//} ContactorState; // maybe i could use this struct for when its proper closed/proper open.. ? idk i might not even rly need this tsruct anymnore but idk

typedef struct{
	uint8_t common;
	uint8_t motor1;
	uint8_t motor2;
	uint8_t array;
	uint8_t LV;
	uint8_t charge;
} ContactorCommand;


#endif /* INC_MBMS_H_ */
