/*
 * StartupTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 */
#include "StartupTask.hpp"

void StartupTask(void* arg)
{
    while(1)
    {
    	Startup();
    }
}

void Startup(void* arg)
{
	//aux battery has started up and is powering the MBMS now (no code)
	// check if main power switch is closed (connected), if so, then..
	// maybe try using a while loop of while(mainSwitch == open), and when while loop is done u know its closed
	//after while loop, u know main switch is closed ! so next u....
	// check external power switch
	// if external power switch is pressed (OFF/enabled).. then
		// go to shutdown procedure and BPS fault
	// else if external power switch is not pressed (everything is okay still), then continue on.. !

	// set flag to give permission to precharge/close common contactor
	// wait for common contactor to be closed !!! (while CC== open) {}
	// set flag to give permission to precharge/close LV
	// wait until LV contactor closed (maybe another while loop ??)


	// enable DCDC1 (with a GPIO,, ig i choose for now...)
	// wait until DCDC1 on, (COULD do while loop of while DCDC1 == off, then after while loop..)
	// disconnect DCDC0 (no longer connect to aux battery)
	// wait until DCDC0 has been disconnected (while DCDC0 == closed) {}


	// set flag to give permission to precharge/close motor contactor
	// wait until motor contactor closed OR... just check that everything is good still (doesnt HAVE to close motor before moving on to next part)
	// set flag to give permission to precharge/close array contactor
	// wait until array contactor done (same as above, make sure everything okay still, doesnt NEED it to bed closed...)
	// set flag that everything is done (all perms given !!!!)

	// end of startup?



}






