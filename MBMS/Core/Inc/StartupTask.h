/*
 * StartupTask.hpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millaine
 */

#ifndef INC_TASK_H_FILES_STARTUPTASK_H_
#define INC_TASK_H_FILES_STARTUPTASK_H_

#include <stdint.h>

#define COMMON_FLAG 0x01U // 00 0001
#define MOTOR1_FLAG 0x02U // 00 0010
#define MOTOR2_FLAG 0x04U // 00 0100
#define ARRAY_FLAG 0x08U // 00 1000
#define LV_FLAG 0x10U // 01 0000
#define CHARGE_FLAG 0x20U // 10 0000
#define ALL_CONTACTORS_FLAG ( COMMON_FLAG | MOTOR1_FLAG | MOTOR2_FLAG | ARRAY_FLAG | LV_FLAG | CHARGE_FLAG)

// also known as EPCOS
#define MPS_ENABLED 0x0
#define MPS_DISABLED 0x1

void StartupTask(void* arg);

void Startup();



#endif /* INC_TASK_H_FILES_STARTUPTASK_H_ */
