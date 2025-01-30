/*
 * ShutoffTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millaine li
 */

#include <stdint.h>


#include "../Inc/ShutoffTask.h"
#include "CANdefines.h"
#include "cmsis_os.h"
#include "main.h"
#include "BatteryControlTask.h"
#include "ReadPowerGPIO.h"

extern ContactorState contactorState;
extern uint16_t startupState;

// PRIORITIES FOR SHUTDOWN PROCEDURE
// HIGHEST: HARD BATTERY LIMIT
// THEN ITS MPS/EPCOS, SOFT BATT LIMIT
// LOWEST: KEY

void ShutoffTask(void* arg)
{
    while(1)
    {
    	Shutoff();
    }
}

void Shutoff()
{

	uint32_t flags; //
	while (1) {

		// Making the CAN Message
		CANMsg msg;
		msg.ID = 0x0; // because we're using the extended id
		msg.extendedID = CONTACTORCOMMANDID;  // this is so the node that receives it checks id to see if this message is for it
		msg.DLC = 1;  // 1 byte data

		// wait for shutoff flag
		flags = osEventFlagsWait(shutoffFlagHandle, SHUTOFF_FLAG, osFlagsWaitAny | osFlagsNoClear, osWaitForever);

		// creating the data to send (using bit shifting):
		// make it so all contactors (if common open, all contactors SHOULD BE opened)
		uint8_t openCommon = (OPEN_CONTACTOR << COMMON) + (OPEN_CONTACTOR<< MOTOR1) + (OPEN_CONTACTOR << MOTOR2)+ (OPEN_CONTACTOR << ARRAY) + (OPEN_CONTACTOR << LOWV) + (OPEN_CONTACTOR << CHARGE);
		// assuming all HV stuff opened at same time
		uint8_t openHV = (contactorState.common << COMMON ) + (OPEN_CONTACTOR << MOTOR1) + (OPEN_CONTACTOR << MOTOR2) + (OPEN_CONTACTOR << ARRAY) + (contactorState.LV << LOWV) + (OPEN_CONTACTOR << CHARGE);

		// flag for BPS fault
		uint16_t BPSFaultSignal = 0;

		// extract the reason for the shut off procedure
		uint8_t causes[4] = {0, 0, 0, 0};
		if ((flags & KEY_FLAG) == KEY_FLAG) {
			causes[KEY] = 1;
		}
		if ((flags & HARD_BL_FLAG) == HARD_BL_FLAG) {
			causes[HARD] = 1;
		}

		if ((flags & MPS_FLAG) == MPS_FLAG) {
			causes[MPS] = 1;
		}
		if ((flags & SOFT_BL_FLAG) == SOFT_BL_FLAG) {
			causes[SOFT] = 1;
		}

		if ( causes[SOFT] || causes[HARD] || causes[MPS]) {
			BPSFaultSignal = 1; // if the reason for shut off is one of the above, set BPS Fault Signal
		}

		if (!causes[HARD]) { // if hitting hard battery limit is not the reason for shut off

			if (startupState >= MOTORS_ENABLED) { // check that startup task made it past the poiunt of enabling th motors

				msg.data[0] = openHV; // assign the data to open the HV Contactor Command!
				osMessageQueuePut(TxCANMessageQueueHandle, &msg, 0, osWaitForever); //adding it to the message queue to send CAN messages
				// wait for motor contactors to open (dont care abt array/charge contactors
				while(contactorState.motor1 == CLOSE_CONTACTOR || contactorState.motor2 == CLOSE_CONTACTOR ) {
					 // get shutoff flags to see if hard battery limit has been reached during the time it take foe HV contactors to open
					// if so, change causes, and break out of this loop!
					 uint32_t flags = osEventFlagsGet(shutoffFlagHandle);
					 if ((flags & HARD_BL_FLAG) == HARD_BL_FLAG) {
						 BPSFaultSignal = 1;
						 causes[KEY] = 0;
						 causes[HARD] = 1;
						 break;
					 }
				}
			}

		}

		if (causes[KEY] & !BPSFaultSignal) { // if cause of shut off is the key, and there is no BPS Fault Signal needed
			if (startupState >= COMMON_CLOSED) { // checking that the startup task made it past the point of closing the common contactor

				msg.data[0] = openCommon; // assign the data of the CAN message to open the Common Contactor Command
				osMessageQueuePut(TxCANMessageQueueHandle, &msg, 0, osWaitForever); // do i want it to wait forever tho ... idk
				while(1) {
					// waiting until its open at which point everything loses power including the BMS!
				}
				// at this point BMS has lost power
			}

		}

		// Otherwise, at this point the reason for shut off is either MPS/EPCOS, or hard or soft battery limit hit
		// If the startup task has made it past the point of disconnecting from DCDC0, then connect to it
		if(startupState >= DCDC0_OPEN) {
			// assuming that ABATT_DISABLE = 0 means this line enables/closes DCDC0 connection to aux/small battery
			HAL_GPIO_WritePin(ABATT_DISABLE_GPIO_Port, ABATT_DISABLE_Pin, GPIO_PIN_RESET);
			while(read_nDCDC0_ON() == 1) {
				// wait for it to be connected to the aux battery
			}
		}

		// After connecting to DCDC0, DCDC1 needs to be disconnected
		if(startupState >= DCDC1_CLOSED) {
			HAL_GPIO_WritePin(DCDC1_EN_GPIO_Port, DCDC1_EN_Pin, GPIO_PIN_RESET);
			while(read_nDCDC1_ON() == 0) {
				// wait for disconnection of DCDC1
			}

		}

		// Common contactor needs to be opened
		if (startupState >= COMMON_CLOSED) {
			// assigning the data of the CAN message to open Common Contactor
			msg.data[0] = openCommon;
			osMessageQueuePut(TxCANMessageQueueHandle, &msg, 0, osWaitForever);
			while (contactorState.common == CLOSE_CONTACTOR) {
				// wait for common to open
			}
		}


		// If the shutdown due to hard or soft battery limit or external button pressed (MPS/EPCOS)
		if (BPSFaultSignal == 1) {

			// TO DO: UPDATE THE STRUCT AND PIN !! active low pin, so set it low ,, on startup should be high
			HAL_GPIO_WritePin(STROBE_EN_GPIO_Port, STROBE_EN_Pin, GPIO_PIN_RESET); // enable strobe lights

			while(1) {
				// Wait for driver to turn key to off, to power everything off
				// Is this just a non-software thing??????
				// or does it have to go thru this shutodwn task tooooo ? probably maybe yeah
			}

		}

	}

}
