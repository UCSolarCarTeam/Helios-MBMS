/*
 * BatteryControlTask.hpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millaineli
 */

#ifndef INC_TASK_H_FILES_BATTERYCONTROLTASK_H_
#define INC_TASK_H_FILES_BATTERYCONTROLTASK_H_

#include <stdint.h>

void BatteryControlTask(void* arg);

void BatteryControl();

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

// so should probably dequeue CAN messages and update the values in this struct, and do appropriate checks of values


#endif /* INC_TASK_H_FILES_BATTERYCONTROLTASK_H_ */
