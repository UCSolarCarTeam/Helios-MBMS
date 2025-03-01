/*
 * CANdefines.h
 *
 *  Created on: Dec 30, 2024
 *      Author: m
 */

#ifndef INC_CANDEFINES_H_
#define INC_CANDEFINES_H_

#include <stdint.h>
#include "cmsis_os.h"


#define PACKINFOID 0x302
#define TEMPINFOID 0x304
#define MAXMINVOLTAGESID 0x30A

#define CONTACTORMASK  0x1fffffe0 // just changed it so it accepts 0x21X and 0x20X
#define CONTACTORIDS 0x210 // was 0x700,, then was 0x200.. just changed to 0x210 feb 6
#define CONTACTOR_HEARTBEATS_IDS 0x200

#define CONTACTORCOMMANDID 0x101 // was 0x721 love it

#define MBMS_TRIP_ID 0x104
#define MBMS_STATUS_ID 0x102




typedef struct {
    uint16_t ID;
    uint32_t extendedID;
    uint8_t DLC;
    uint8_t data[8];
} CANMsg;

//typedef struct {
//	uint8_t common;
//	uint8_t motor1;
//	uint8_t motor2;
//	uint8_t array;
//	uint8_t LV;
//	uint8_t charge;
//} ContactorCommand;

// this struct was og in battery control, i moved it here while doiung extern stuff but i could move it backk doesnt rly matter
// but this struct should be readable by diff tasks !!!! e.g. startup, shutoff
// zero for closed (connected), one for open (disconnected)
//typedef struct{
//	uint8_t common;
//	uint8_t motor1;
//	uint8_t motor2;
//	uint8_t array;
//	uint8_t LV;
//	uint8_t charge;
//} ContactorState;



#endif /* INC_CANDEFINES_H_ */
