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

void ShutoffTask(void* arg)
{
    while(1)
    {
    	Shutoff();
    }
}

// read struct of contactor states to check and maybe when u send the message send it with keeo the states as is except for
// the one that ur tryna change

void Shutoff()
{
	uint32_t flags; //
	while (1) {

		CANMsg msg;
		msg.ID = 0x0;          // bc we're using the extended id...
		msg.extendedID = 0x721;  // this is like so the node that receives it checks id to see if this message is for it
		msg.DLC = 3;             // 1 byte data ?????? bc i dont rly need to send that much do i ...?
		msg.data[0] = OPEN_CONTACTOR;      // first byte of data ,,, maybe like 0x01 for open it, 0x00 to close it? idk ...
		// ADD IT TO THE QUEUE FOR CANTX TASK !!
		osMessageQueuePut(TxCANMessageQueueHandle, &msg, 0, osWaitForever); // do i want it to wait forever tho ... idk
		flags = osEventFlagsWait(shutoffFlagHandle, SHUTOFF_FLAG, osFlagsWaitAny, osWaitForever);
		uint16_t BPSFaultSignal = 0;
		// extract the reason for the shut off procedure
		char* cause = "\0";
		if ((flags & MPS_FLAG) == MPS_FLAG) {
			cause = "MPS"; // main power switch
		}
		else if ((flags & KEY_FLAG) == KEY_FLAG) {
			cause = "key";
		}
		else if ((flags & HARD_BL_FLAG) == HARD_BL_FLAG) {
			cause = "hard"; // hard battery limit
		}
		else if ((flags & SOFT_BL_FLAG) == SOFT_BL_FLAG) {
			cause = "soft"; // soft battery limit
		}
		else if ((flags & EPCOS_FLAG) == EPCOS_FLAG) {
			cause = "EPCOS"; // external power cut off switch
		}

		if (strcmp(cause, "soft") == 0 || strcmp(cause, "hard") == 0 || strcmp(cause, "EPCOS") == 0) {
			BPSFaultSignal = 1;
		}

		if (strcmp(cause, "hard") != 0) { // if hard battery limit is not hit
			// update motor control info // no action needed so far!!!


			// um maybe am i actually supposed to receive can message (curr state of contactors, and then just change the specific bit of the contactor i want to close ?

			// MAKING THE CAN MSG
			CANMsg msg;
			msg.ID = 0x0;          // bc we're using the extended id...
			msg.extendedID = 0x00000001;  // this is like so the node that receives it checks id to see if this message is for it
			msg.DLC = 1;             // 1 byte data ?????? bc i dont rly need to send that much do i ...?
			msg.data[0] = OPEN_CONTACTOR;      // first byte of data ,,, maybe like 0x01 for open it, 0x00 to close it? idk ...
			// ADD IT TO THE QUEUE FOR CANTX TASK !!
			osMessageQueuePut(TxCANMessageQueueHandle, &msg, 0, osWaitForever); // do i want it to wait forever tho ... idk
			// send CAN to open HV (high voltage) contactor // I THINK NOW U SHOULD ADD YOUR CAN MESSAGE TO THE QUEUE!!!!!!!!!!
			// check contactor struct to see if that contactor has opened

		}

		if (strcmp(cause, "key") == 0) {
			// send CAN message to open common contactor
			// MAKING THE CAN MSG
						CANMsg msg;
						msg.ID = 0x0;          // bc we're using the extended id...
						msg.extendedID = 0x00000010;  // this is like so the node that receives it checks id to see if this message is for it
						msg.DLC = 1;             // 1 byte data ?????? bc i dont rly need to send that much do i ...?
						msg.data[0] = OPEN_CONTACTOR;      // first byte of data ,,, maybe like 0x01 for open it, 0x00 to close it? idk ...
						// ADD IT TO THE QUEUE FOR CANTX TASK !!
						osMessageQueuePut(TxCANMessageQueueHandle, &msg, 0, osWaitForever); // do i want it to wait forever tho ... idk
			while(1) {
				// basically waiting until its open at which point everything loses power including bms !
			}
			// at this point BMS loses power...
		}
		// maybe we (the mbms) cannot turn on and off DCDC0... so then should i just wait for it to be connect? maybe yes..
		// set gpio to close DCDC0
		// wait for it to be closed (while loop)
		while(readnDCDC0_ON() == 1) { // (0 is that DCDC0 producing a stable power supply, 1 is not)
			// do nothing, just wait for it to be connected
		}

		// send CAN message to open common contactor
		//  wait until CC open.....

		// while (main power switch == closed) {} wait until its open !!!

		if (BPSFaultSignal == 1) { //if the shutdown due to hard or soft battery limit or external button pressed
			// send BPS fault signal through CAN
			// lights are software strobed (DONT NEEED TO WORRY ABT)
			// dashboard shows fault (DONT NEEED TO WORRY ABT)

			// at this point i THINKKK that they should just turn the key off ... shut down car, so bms shuts down ....
			// like thats the only thing you should be able to do at this point right?
			// so maybe i could just add another infinite while(1) loop just waiting for them to turn off key?? and bms lose power?
			while(1) {

			}

		}

		else { //if shutdown is just due to main power switch being opened
			// then 2 options, 1: turn key off (bms loses power), or 2: close main power switch,
			while(readMainPowerSwitch() == 1) { // this is assuming 1 (SET) means that the switch is open.... change as needed
				// do nothing (?)
			}
			// while (main power switch == open) {}
			// so either they turn key to off during this time, or they close main power switch again and it should go to startup ?

		}

	}

}

// NEED TPO CHANGE THESE BECAUSE CHANGED THE PIN CONFIG :(
// just changed all except EPCOS

uint16_t readMainPowerSwitch(void) {
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_4);  // PC4
}


uint16_t readKeySwitch(void) {
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);  // PB1
}

uint16_t readEPCOSwitch(void) {
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8);  // PB8 NOT SURE ABT THIS ONE!!! ASK SOMEBODY!!!!
}

uint16_t readnDCDC0_ON(void) {
    return HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2);  // PE2
}
//GPIO_PIN_RESET or GPIO_PIN_SET


// if soft shutdown failure emergency switch opened, then bms loses power... um
	// i dont think i have to do anything for the above....?

	// wait for shutdown flag from battery control task
	// after flag received, then...
	// shutoff signal was received, need to check shutdown type

	// just declare some var to see if shutdown was from soft or hard battery limit or external push button (uint16_t var = 0)
	// if it is from above,
		// var = 1

	// if hard battery limit NOT hit (so if its hit itll skip over this part!), then
		// update motor control info // no action needed so far!!!
		// send CAN to open HV (high voltage) contactor
		// check contactor struct to see if that contactor has opened

	// if key turned to off
		// send CAN message to open common contactor
		// while(1)
		// at this point BMS loses power...

	// (If it was a hard battery limit it wouldve skipped above code and come to here)

	// set gpio to close DCDC0
	// wait for it to be closed (while loop)

	// send CAN message to open common contactor
	//  wait until CC open.....


	// while (main power switch == closed) {} wait until its open !!!

	// if var == 1 (if the shutdown due to hard or soft battery limit or external button pressed
		// send BPS fault signal through CAN
		// lights are software strobed (DONT NEEED TO WORRY ABT)
		// dashboard shows fault (DONT NEEED TO WORRY ABT)

		// at this point i THINKKK that they should just turn the key off ... shut down car, so bms shuts down ....
		// like thats the only thing you should be able to do at this point right?
		// so maybe i could just add another infinite while(1) loop just waiting for them to turn off key?? and bms lose power?

	// else if shutdown is just due to main power switch being opened
		// then 2 options, 1: turn key off (bms loses power), or 2: close main power switch,
		// while (main power switch == open) {}
		// so either they turn key to off during this time, or they close main power switch again and it should go to startup ?


/*
 * How can i check what caused the shutdown?
 * im not sure but maybe there can be a specific flag made for reason of shutdown so
 * battery control task can set the flag based on whats wrong.... ? or whats happened...
 * then in shutdown task, get flag, compare and see if its due to hard or soft or whatever
 * and then do what it needs to do based on that...
*/

// maybe i could do something with mutexes......... so u cant startup if ur in shutdown idkkkkk tho



