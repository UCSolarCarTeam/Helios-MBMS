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

void Gatekeeper(Contactors contactor, ContactorState WantedState)
{
	// switch case between opening or closing
	switch (WantedState){
		case OPEN:
			HAL_GPIO_WritePin(contactor->GPIO_Port, contactor->GPIO_Pin, GPIO_PIN_RESET);
			break;
		case CLOSED:
			// to safely close the contactor, we would need to first close the precharger
			// by using this function, we're closing the precharger AND the contactor, so we don't need to call a closeContactor function here
			Precharger(contactor);
			break;
	}
}



