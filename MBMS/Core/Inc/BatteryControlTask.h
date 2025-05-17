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


/* "private" helper functions */
void initiateBPSFault();
void checkKeyShutdown();
void updateContactorInfo(uint8_t contactor, uint8_t prechargerClosed, uint8_t prechargerClosing,
		uint8_t prechargerError, uint8_t contactorClosed, uint8_t contactorClosing,
		uint8_t contactorError, int16_t lineCurrent, int16_t chargeCurrent, uint8_t BPSerror);

void startupCheck();
uint8_t waitForFirstHeartbeats();
uint8_t startupBatteryCheck();
uint8_t checkPrechargersOpen();
uint8_t checkContactorsOpen();



/* "public" functions */
void UpdateContactorInfoStruct();
void UpdateOrionInfoStruct();
void UpdatePowerSelectionStruct();

void UpdateTripStatus();
void CheckContactorHeartbeats();
void CheckSoftBatteryLimit();

void SystemStateMachine();
void UpdateContactors();

void UpdateCounter(uint32_t * counter);


#define ORION_MSG_WAIT_TIMEOUT 200 //ms
#define CONTACTOR_HEARTBEAT_TIMEOUT 1 // seconds !!

#define SOC_SAFE_FOR_CHARGE 90 // maybe can do if SOC is less than 90, safe to charge
#define SOC_SAFE_FOR_DISCHARGE 25 // maybe if SOC greater than this, safe to discharge ?

#define NO_CURRENT_THRESHOLD 3 // (AMPS). So if less than this, consider no current, if more than this, consider there is current

#define MAX_PACK_VOLTAGE 75 // (Volts)

enum VoltageLimits {
	HARD_MAX_CELL_VOLTAGE = 10,
	SOFT_MAX_CELL_VOLTAGE = 8,
	HARD_MIN_CELL_VOLTAGE = 2,
	SOFT_MIN_CELL_VOLTAGE = 3,
	HARD_MAX_PACK_VOLTAGE = 76,
	SOFT_MAX_PACK_VOLTAGE = 75

};

enum CurrentLimits {
	HARD_MAX_COMMON_CONTACTOR_CURRENT = 5,
	HARD_MAX_MOTORS_CONTACTOR_CURRENT = 5,
	HARD_MAX_ARRAY_CONTACTOR_CURRENT = 5,
	HARD_MAX_LV_CONTACTOR_CURRENT = 5,
	HARD_MAX_CHARGE_CONTACTOR_CURRENT = 5,
	SOFT_MAX_COMMON_CONTACTOR_CURRENT = 4,
	SOFT_MAX_MOTORS_CONTACTOR_CURRENT = 4,
	SOFT_MAX_ARRAY_CONTACTOR_CURRENT = 4,
	SOFT_MAX_LV_CONTACTOR_CURRENT = 4,
	SOFT_MAX_CHARGE_CONTACTOR_CURRENT = 4

};

enum TemperatureLimits {
	HARD_MAX_TEMP,
	SOFT_MAX_TEMP,
	HARD_MIN_TEMP,
	SOFT_MIN_TEMP
};


#endif /* INC_TASK_H_FILES_BATTERYCONTROLTASK_H_ */
