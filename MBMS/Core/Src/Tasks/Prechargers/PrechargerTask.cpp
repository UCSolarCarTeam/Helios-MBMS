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

	// once the precharger gets the 'closing' command from the gatekeeper, it must first close its own switch
	// the input is going to be a message saying which contactor to close
	// what I was thinking on how to split the work is each contactor will be a number and
	// the corresponding precharger for each contactor will be the contactor number + some number.
	// Num here represents this number
	closeContactor(arg + num);

	// Now I think there should some delay before the contactor's closed
	delay();

	// secondly, it must close the array the gatekeeper wanted closed (this is so the contactor isn't flooded with high voltage)
	closeContactor(arg);



	// thirdly, once the array is closed and a 'transition voltage' is provided by the precharger, the precharger opens and the state of the contactor is changed from 'closing' to 'closed'
	// this means that there won't be a voltage spike now so thats safer 😌
	openContactor(arg + num);

	// I think i want to put the state change ('closing' -> 'closed') inside the close contactor function

}




