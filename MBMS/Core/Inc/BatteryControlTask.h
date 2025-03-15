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
void updateContactorInfo(uint8_t contactor, uint8_t prechargerClosed, uint8_t prechargerCLosing, uint8_t prechargerError, uint8_t contactorState, uint8_t contactorError, uint16_t contactorCurrent, uint16_t contactorVoltage);
void updateContactorInfoStruct();

void updatePackInfoStruct();
void updateTripStatus();

void checkIfShutdown();


void checkContactorHeartbeats();


//#define CLOSED 0x0
//#define OPEN 0x1


// These are random values i made for the batt limit, must be updated
#define HARD_BATTERY_LIMIT 75
#define SOFT_BATTERY_LIMIT 73
// wait.... are these the limits for the cell or the pack ????

#define MAX_CELL_VOLTAGE 10
#define MIN_CELL_VOLTAGE 2

/* Add contactor max current (motor1/2 should be the same)*/
#define MAX_COMMON_CONTACTOR_CURRENT 5
#define MAX_MOTORS_CONTACTOR_CURRENT 5
#define MAX_ARRAY_CONTACTOR_CURRENT 5
#define MAX_LV_CONTACTOR_CURRENT 5
#define MAX_CHARGE_CONTACTOR_CURRENT 5

#define ORION_MSG_WAIT_TIMEOUT 200 //ms
#define CONTACTOR_HEARTBEAT_TIMEOUT 1 // seconds !!

#define SOC_SAFE_FOR_CHARGE 90 // maybe can do if SOC is less than 90, safe to charge
#define SOC_SAFE_FOR_DISCHARGE 25 // maybe if SOC greater than this, safe to discharge ?

#define NO_CURRENT_THRESHOLD 3 // (AMPS). So if less than this, consider no current, if more than this, consider there is current

#define MAX_PACK_VOLTAGE 75 // (Volts)



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
