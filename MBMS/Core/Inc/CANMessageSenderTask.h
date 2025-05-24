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

void sendMBMSHeartbeatCanMessage();

void sendContactorsCanMessage();
void sendMBMSStatusCanMessage();
void sendPowerSelectionStatus();
void sendTripStatusCanMessage();
void sendSoftBatteryLimitCanMessage();


void lastSentTime_init();

enum CANMessageToSend {
	HEARTBEAT = 0,
	CONTACTOR_COMMAND,
	MBMS_STATUS,
	POWER_SELECTION_STATUS,
	MBMS_TRIP,
	MBMS_SOFT_BATTERY_LIMIT_WARNING
};

// in Hz
#define HEARTBEAT_FREQ 1.0
#define CONTACTOR_COMMAND_FREQ 10.0
#define MBMS_STATUS_FREQ 10.0
#define POWER_SELECTION_STATUS_FREQ 10.0
#define MBMS_TRIP_FREQ 10.0
#define MBMS_SOFT_BATTERY_LIMIT_WARNING_FREQ 10.0

#endif /* INC_CANMESSAGESENDERTASK_H_ */
