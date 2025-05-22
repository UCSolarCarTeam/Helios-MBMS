/*
 * CANMessageSenderTask.h
 *
 *  Created on: Mar 15, 2025
 *      Author: m
 */

#ifndef INC_CANMESSAGESENDERTASK_H_
#define INC_CANMESSAGESENDERTASK_H_

#include <stdint.h>

void CANMessageSenderTask(void * arg);
void CANMessageSender();
void sendTripStatusCanMessage();
void sendMBMSStatusCanMessage();
void sendMBMSHeartbeatCanMessage();
void sendContactorsCanMessage();

#endif /* INC_CANMESSAGESENDERTASK_H_ */
