/*
 * GatekeeperTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 */

void GatekeeperTask(void* arg)
{
    for (;;)
    {
    	Gatekeeper();
    }
}

void Gatekeeper(void* arg)
{
	// arg is the contactor we have to close

	// set charge state to closing for the contactor specified
	arg.status = CLOSING;

	// to safely close the contactor, we would need to first close the precharger
	// by using this function, we're closing the precharger AND the contactor, so we don't need to call the closeContactor function here
	Precharger(arg);


}



