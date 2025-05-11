/*
 * TxGatekeeperTask.hpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millaineli
 */

#ifndef INC_TASK_H_FILES_CANTXGATEKEEPERTASK_H_
#define INC_TASK_H_FILES_CANTXGATEKEEPERTASK_H_

#include "stm32f4xx_hal.h"
#include "CANdefines.h"

void CANTxGatekeeperTask(void* arg);
void CANTxGatekeeper(CANMsg* msg);

extern CAN_HandleTypeDef hcan1;
extern osMessageQueueId_t TxCANMessageQueueHandle;

#endif /* INC_TASK_H_FILES_CANTXGATEKEEPERTASK_HPP_ */
