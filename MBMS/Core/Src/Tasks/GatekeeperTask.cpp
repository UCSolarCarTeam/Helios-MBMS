/*
 * GatekeeperTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 */

#include "GatekeeperTask.hpp"

void GatekeeperTask(void* arg)
{
    while(1)
    {
    	Gatekeeper();
    }
}

void Gatekeeper(ContactorInfo_t* contactor)
{
	// (switch case) if we want to close the contactor:
		// arg is the contactor we have to close, it will be an enum

		// to safely close the contactor, we would need to first close the precharger
		// by using this function, we're closing the precharger AND the contactor, so we don't need to call the closeContactor function here
		Precharger(contactor);
}



