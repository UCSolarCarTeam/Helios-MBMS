/*
 * PrechargerTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 *      Role: Safely close the contactor by first closing a precharger to prevent a voltage spike
 */

void PrechargerTask(void* arg)
{
    for (;;)
    {
    	Precharger();
    }
}

void Precharger(void* arg)
{
	// here arg is a pointer to what contactor we want closed

	// once the precharger gets the 'closing' command from the gatekeeper which comes from the FreeRTOS queue, it must first close its own switch
	// the input is going to be a message saying which contactor to close
	// every corresponding precharger and contactor have the same number (i.e motor precharger and contactor are both 2)
	// This function will take in the contactor of interest for the precharger and also either 1 or 0
	// 1 = close the contactor
	// 0 = open the contactor
	PreChargerContactor(arg, 1);

	// Now I think there should some delay before the contactor's closed
	delay();

	// secondly, it must close the array the gatekeeper wanted closed (this is so the contactor isn't flooded with high voltage)
	GatekeeperContactor(arg, 1);



	// thirdly, once the array is closed and a 'transition voltage' is provided by the precharger, the precharger opens and the state of the contactor is changed from 'closing' to 'closed'
	// this means that there won't be a voltage spike now so thats safer 😌
	PreChargerContactor(arg, 0);

	// check the voltage by reading the line and making sure it's what we want (the battery voltage)
	// if it is, we can change the state from 'closing' to 'closed'
}




