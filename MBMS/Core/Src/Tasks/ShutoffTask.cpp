/*
 * ShutoffTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 */

#include "ShutoffTask.hpp"

void ShutoffTask(void* arg)
{
    while(1)
    {
    	Shutoff();
    }
}

void Shutoff(void* arg)
{
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
		// check contactor struct to see if that contactor has openedr

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

}

/*
 * How can i check what caused the shutdown?
 * im not sure but maybe there can be a specific flag made for reason of shutdown so
 * battery control task can set the flag based on whats wrong.... ? or whats happened...
 * then in shutdown task, get flag, compare and see if its due to hard or soft or whatever
 * and then do what it needs to do based on that...
*/

// maybe i could do something with mutexes......... so u cant startup if ur in shutdown idkkkkk tho



