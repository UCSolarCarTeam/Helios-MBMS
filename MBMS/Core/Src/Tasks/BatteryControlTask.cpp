/*
 * BatteryControlTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 */

#include "BatteryControlTask.hpp"

void BatteryControlTask(void* arg)
{
    while(1)
    {
    	BatteryControl();
    }
}

// if something goes wrong set event flag, shutdown has a wait event flag, and will shutdown when its set ...

// maybe use a message queue to send a number corresponding to a contactor from STARTUP
// battery control recieves message queue thing, checks what number it is, and will close respective contactor

// nooooo to above

// startup tasks should just GIVE permission to batterycontrol (for specific contactor), then batterycontrol will check
// if things r chill (orion and stuff idk) then itll close contactor for thing it has permissin for
// should i just use a flag for each contactor ???
// maybe flag to see if contactor is already closed, (so u dont close if alreaady close) only close if things r chill
// does startup only just run onceee????

// make a struct to hold contactor states (open, closed, precharging)
// this struct should be a global var, but only batterycontroltask should be able to write to it!!!!
void BatteryControl(void* arg)
{
	// should communicate with startup and shutdown tasks, queue? mutex? flag? ... event flag for shutdown

	// checking everything is ok through orion

	// get the flag, (uint32_t startUpFlag = osEventFlagsGet(eventFlag_id)
	// var = orionCheck(flag) //giving it what the permissions are
	// var1 = 0; // idk
	// if (var says stuff is BAD || MPS is opened || key turned off || external button pressed) // this means u should shut down !!!
		// var1 = 1; // only do this check when closing contactors
		// set flag to something to indicate what has happened (soft batt lim, hard batt lim, external button, MPS, or key off )
		// this should be flag that shutdown procedure is waiting for... ?

	// else // not shutdown procedure !!!! just keep checking stuff,,, and if u need to close any contactors
	// if var1 == 0 && flag startUpFlag indicates startup is not over (not all perms given
		// compare it to see if common contactor permisiion given, if yes..
			// send message through CAN to connect precharger for common contactor
			// send message (CAN) to close/connect common contactor(the one that igs connect to them all)
			// send message (CAN) to disconnect CC precharger

		// compare it to see if LV perm given, if yes..
			// send message through CAN to connect precharger for  LV
			// send message (CAN) to close/connect LV
			// send message (CAN) to disconnect LV precharger

		// compare to see if motor perm given, if yes..
			// send CAN message to close/connect precharger for  motor
			// send CAN message to connect/close motor contactor
			// send CAN message to open precharger for motor

		// compare to see if all array perm given, if yes..
			// send CAN message to close precharge for array
			// send CAN message to close (connect) to array contactor(solar panels so they can charge battery!!!)
			// send CAN message to open/disconnect precharger for array


	// otherwise its already all permissions
	// maybe just check all the things that ae supposed to be closed are closed...

	// checking everything is ok through orion
	// if something is wrong
		// set flag for shutdwon task (in shutdown task have a wait event flag, and itll just do stuff if that flag is set

}

//should return 1, for everything is ok, and 0 for something (anything) has gone wrong...
uint32_t orionCheck(uint32_t permissions) {
	// idk do all the checks???
	// if not good,
		// set flag for shutdown !!!!!
		//return 0
	// if things r fine
	// check if CC closed
	// if not closed

}

// motor and array cont6actors might still need to open or close during.....
// but i think LV and common can just stay closed ... ?
// also should set flag in this function for shutdown procedure in case anything goes wrong


// also should not be closing more contactors if stuff is wrong!!!!!!

// gatekeeper tasks should be letting battery tasks know if it should close/open contactor
// ofc battery task should check w permissions setup from startup AND var1 (if shutdown procedure should
// occur, bc if so, shouldn't be able to close more contactors)



