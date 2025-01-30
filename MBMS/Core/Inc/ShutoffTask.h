/*
 * ShutoffTask.hpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millaineli
 */

#ifndef INC_TASK_H_FILES_SHUTOFFTASK_H_
#define INC_TASK_H_FILES_SHUTOFFTASK_H_

#include <stdint.h>

#define SHUTOFF_FLAG 0b00000001U // just making the flag an arbitrary number (should be uint32_t,,, this is = 1 in decimal)
// what was the cause of the shutdown??
#define EPCOS_FLAG 0b00000010U // external power cut off switch (push button outside car), starts soft shutdown
#define MPS_FLAG 0b00000100U // main power switch is the cause of shutoff
#define KEY_FLAG 0b00001000U // turning car key is cause of shutoff
#define HARD_BL_FLAG 0b00010000U // hard battery limit is cause of shutoff
#define SOFT_BL_FLAG 0b00100000U // soft battery limit is cause of shutoff


#define KEY 0
#define MPS 1
#define HARD 2
#define SOFT 3

// i defined the below in CANdefines.h instead
//#define OPEN_CONTACTOR 0x01
//#define CLOSE_CONTACTOR 0x00

// fucntion definitions
void ShutoffTask(void* arg);

void Shutoff();
//uint16_t readnDCDC0_ON(void);
//uint16_t readEPCOSwitch(void);
//uint16_t readKeySwitch(void);
//uint16_t readMainPowerSwitch(void);


#endif /* INC_TASK_H_FILES_SHUTOFFTASK_H_ */
