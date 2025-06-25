/*
 * BatteryControlTask.hpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millaineli
 */

#ifndef INC_TASK_H_FILES_BATTERYCONTROLTASK_H_
#define INC_TASK_H_FILES_BATTERYCONTROLTASK_H_

#define BLU 0
#define GRN 1
#define RED 2


#include <stdint.h>
#include "main.h"



void BatteryControlTask(void* arg);
void BatteryControl();


void perms_init();
void MBMSStatus_init();

/* "private" helper functions */
void initiateBPSFault();
void checkKeyShutdown();
void updateContactorInfo(uint8_t contactor, uint8_t prechargerClosed, uint8_t prechargerClosing,
		uint8_t prechargerError, uint8_t contactorClosed, uint8_t contactorClosing,
		uint8_t contactorError, float lineCurrent, float chargeCurrent, uint8_t BPSerror);

void startupCheck();
uint8_t waitForFirstHeartbeats();
uint8_t startupBatteryCheck();
uint8_t checkPrechargersOpen();
uint8_t checkContactorsOpen();

void clear_Trips();
void clear_Warnings();



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

void enter_BOOT();



#define ORION_MSG_WAIT_TIMEOUT 5//1000 //ms
#define CONTACTOR_HEARTBEAT_TIMEOUT 2500 //1500 // smilliseconds !!

#define SOC_SAFE_FOR_CHARGE 90 // maybe can do if SOC is less than 90, safe to charge
#define SOC_SAFE_FOR_DISCHARGE 25 // maybe if SOC greater than this, safe to discharge ?

#define NO_CURRENT_THRESHOLD 3 // (AMPS). So if less than this, consider no current, if more than this, consider there is current

// in seconds
#define CONTACTOR_RESPONSE_TIMEOUT 3


// in milliseconds (ticks)
#define LV_OC_TIMEOUT 10000

#define HARD_MAX_CELL_VOLTAGE 4.5F
#define SOFT_MAX_CELL_VOLTAGE 4.2F
#define HARD_MIN_CELL_VOLTAGE 3.5F
#define SOFT_MIN_CELL_VOLTAGE 3.7F
#define HARD_MAX_PACK_VOLTAGE 120 // um yeah idk if these even a trip soo i dont htink so :)
#define SOFT_MAX_PACK_VOLTAGE 115


/* in A */
#define HARD_MAX_COMMON_CONTACTOR_CURRENT 300.0F
#define HARD_MAX_MOTORS_CONTACTOR_CURRENT 300.0F
#define HARD_MAX_ARRAY_CONTACTOR_CURRENT 300.0F
#define HARD_MAX_LV_CONTACTOR_CURRENT 300.0F
#define HARD_MAX_CHARGE_CONTACTOR_CURRENT  300.0F
#define SOFT_MAX_COMMON_CONTACTOR_CURRENT 290.0F
#define SOFT_MAX_MOTORS_CONTACTOR_CURRENT 290.0F
#define SOFT_MAX_ARRAY_CONTACTOR_CURRENT 290.0F
#define SOFT_MAX_LV_CONTACTOR_CURRENT 290.0F
#define SOFT_MAX_CHARGE_CONTACTOR_CURRENT 290.0F


#define	HARD_MAX_TEMP 45
#define	SOFT_MAX_TEMP 40
#define	HARD_MIN_TEMP 0
#define	SOFT_MIN_TEMP 5

// in milliseconds
#define CLOSE_CONTACTOR_TIMEOUT 5000 // so big for testing ok
#define OPEN_CONTACTOR_TIMEOUT 5000





#endif /* INC_TASK_H_FILES_BATTERYCONTROLTASK_H_ */
