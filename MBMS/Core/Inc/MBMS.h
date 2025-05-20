/*
 * MBMS.h
 *
 *  Created on: Mar 5, 2025
 *      Author: millaine
 */

#ifndef INC_MBMS_H_
#define INC_MBMS_H_

#define UPDATING_MUTEX_TIMEOUT 200
#define READING_MUTEX_TIMEOUT 200


#define FREERTOS_TICK_PERIOD 1/configTICK_RATE_HZ //USE THIS INSTEAD OF SECONFS PER TICK. this is in ticks per second

#define SHUTOFF_FLAG 0x01 // just making the flag an arbitrary number (should be uint32_t,,, this is = 1 in decimal)
#define nMPS_FLAG 0x02 // main power switch disconnected
#define HARD_BL_FLAG 0x04 // hard battery limit hit

#define OPEN_CONTACTOR 0
#define CLOSE_CONTACTOR 1
#define CLOSING_CONTACTOR 1

// Define boolean as an enum or typedef in a header
typedef enum { false = 0, true = 1 } boolean;

/* oops sorry i accidentally changed it back to two separate things */
//// new enum made march 7
//enum ContactorState {
//	OPEN_CONTACTOR = 0, //00
//	CLOSE_CONTACTOR,    //01
//	CLOSING_CONTACTOR   //10
//
//};

enum Contactor {
	COMMON = 0,
	MOTOR,
	ARRAY,
	LOWV,
	CHARGE
};


enum startupStates {
	nMPS_ENABLED = 0, // when u disconnect mps it kind goes back to this state in the startup okay?? check the draw.io diagram
	nMPS_DISABLED,
	ESD_DISABLED,
	CHECKS_PASSED,
	COMMON_CLOSED,
	LV_CLOSED,
	EN1_ON,
	MOTORS_PERMS,
	ARRAY_PERMS,
	COMPLETED
};


enum carStates {
	STARTUP,
	FULLY_OPERATIONAL,
	CHARGING,
	BPS_FAULT,
	MPS_DISCONNECTED,
	SOFT_TRIP
};


typedef struct {
	uint8_t cell_OV;
	uint8_t cell_UV;
} SoftBatteryTrip;


typedef struct {
	uint8_t common;
	uint8_t motor;
	uint8_t array;
	uint8_t lv;
	uint8_t charge;
	uint8_t startupDone;
	uint8_t faulted;
} Permissions;


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
	uint8_t contactorClosed; // march 8: two bits become one variable, 00 = open, 01 = closed, 10 = closing
	uint8_t contactorClosing;
	uint8_t contactorError;
	int16_t lineCurrent; // CHANGED FROM UINT16 ON APRIL 2
	int16_t chargeCurrent;
	uint8_t BPSerror;
	uint16_t heartbeat;
} ContactorInfo;


// add startup state
typedef struct {
	uint8_t auxilaryBattVoltage; // bits 0-4
	uint8_t strobeBMSLight;
	uint8_t allowCharge; // remember ur question may 7: is it straight up gpio, or is it something i decide
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
	uint8_t commonHeartbeatDeadTrip;
	uint8_t motorHeartbeatDeadTrip;
	uint8_t arrayHeartbeatDeadTrip;
	uint8_t LVHeartbeatDeadTrip;
	uint8_t chargeHeartbeatDeadTrip;
	uint8_t MPSDisabledTrip; // this is not a BPS fault !
	uint8_t ESDEnabledTrip;
	uint8_t highTemperatureTrip;
	uint8_t lowTemperatureTrip;

} MBMSTrip;


typedef struct {
	uint8_t highCellVoltageWarning;
	uint8_t lowCellVoltageWarning;
	uint8_t commonHighCurrentWarning;
	uint8_t motorHighCurrentWarning;
	uint8_t arrayHighCurrentWarning;
	uint8_t LVHighCurrentWarning;
	uint8_t chargeHighCurrentWarning;
	uint8_t highBatteryWarning;
	uint8_t highTemperatureWarning;
	uint8_t lowTemperatureWarning;

} MBMSSoftBatteryLimitWarning;


typedef struct {
	uint8_t nMainPowerSwitch;
	uint8_t ExternalShutdown;
	uint8_t EN1;
	uint8_t nDCDC_Fault;
	uint8_t n3A_OC; // changed bc not bandwidth
	uint8_t nDCDC_On;
	uint8_t nCHG_Fault;
	uint8_t nCHG_On;
	uint8_t nCHG_LV_En;
	uint8_t ABATT_Disable;
	uint8_t Key;
} PowerSelectionStatus;


typedef struct{
	uint8_t common;
	uint8_t motor;
	uint8_t array;
	uint8_t LV;
	uint8_t charge;
} ContactorCommand;


#endif /* INC_MBMS_H_ */
