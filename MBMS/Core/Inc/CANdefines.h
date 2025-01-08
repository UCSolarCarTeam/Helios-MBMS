/*
 * CANdefines.h
 *
 *  Created on: Dec 30, 2024
 *      Author: m
 */

#ifndef INC_CANDEFINES_H_
#define INC_CANDEFINES_H_

#include <stdint.h>

#define packInfoID 0x302
#define tempInfoID 0x304
#define maxMinVoltagesID 0x30A

typedef struct {
    uint16_t ID;
    uint32_t extendedID;
    uint8_t DLC;
    uint8_t data[8];
} CANMsg;




#endif /* INC_CANDEFINES_H_ */
