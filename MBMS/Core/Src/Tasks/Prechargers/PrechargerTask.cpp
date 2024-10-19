/*
 * PrechargerTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 *      Role: Safely closes the contactor by first closing a precharger to prevent a voltage spike
 */
#include "PrechargerTask.hpp"



void PrechargerTask(void* arg)
{
	// should I do this???
	uint16_t delay = 500; /* Default delay */
	uint16_t msg = 0;
	osStatus_t status;

    while(1)
    {
    	status = osMessageQueueGet(msgConactorQueueID, &msg, 0, 10);
    	if(status == osOK)
    		Precharger(status);
    	//should we do the delay?
    	osDelay(delay);
    }
}

void Precharger()
{
	// Change this to use to a queue and the queue should contain: 1) the contactor number 2) the desired action (closed/open)
    uint32_t contactorFlags = osEventFlagsWait(contactorControlEventBits, COMMON_CLOSED | COMMON_OPENED, osFlagsWaitAny, osWaitForever);

	// here the input is a pointer to what contactor we want closed, it will be an enum (a number)

	// once the precharger gets the 'closing' command from the gatekeeper which comes from the FreeRTOS queue, it must first close its own switch
	// every corresponding precharger and contactor have the same number (i.e motor precharger and contactor are both 2)
	tryChangingContactor(prechargers[contactor], CLOSING, CLOSED, PRECHARGER_DELAY, prechargerNames, prechargerNumberOfTries[contactor], contactor);

	// if the precharger doesn't close, the function will be put into a recursive loop and will never end. Thus a count check needs to stop it to throw an error


	// secondly, it must close the array the gatekeeper wanted closed (this is so the contactor isn't flooded with high voltage)
	tryChangingContactor(gatekeepers[contactor], CLOSING, CLOSED, TIMETOCLOSE_DELAY, contactorNames, NumberOfTries[contactor], contactor);

	// if the contactor doesn't close, the function will be put into a recursive loop and will never end. Thus a watchdog needs to stop it to throw an error

	// thirdly, once the array is closed and a 'transition voltage' is provided by the precharger, the precharger opens and the state of the contactor is changed from 'closing' to 'closed'
	// this means that there won't be a voltage spike now so thats safer 😌
	tryChangingContactor(prechargers[contactor], CLOSED, OPEN, TIMETOOPEN_DELAY, prechargerNames, prechargerNumberOfTries[contactor], contactor);

	// check the voltage by reading the line and making sure it's what we want (the battery voltage)
	// if it is, we can change the state from 'closing' to 'closed'
	//asks electrical if we still need to do this
}



void tryChangingContactor(ContactorInfo_t* contactor, ContactorState current_state, ContactorState wanted_state, uint32_t delayTime, char* prechargerOrContactor, int numOfTrials, char* contactorNum){
	while(pin_set == 0){
		if (numOfTrials > 3){
			printf("After 3 attempts, %s is still not in the %s state, so stops trying", prechargerOrContactor[contactorNum], stateNames[state]);
			return;
		}
		// added the prechargerOrContactor for error checking, that's it
		// set state to closing for the contactor specified
		contactor.GPIO_State = current_state;

		// try physically closing the contactor
		physciallyChangeContactor(contactor, current_state);

		// Now I think there should some delay before the contactor's closed
		osDelay(delayTime);

		// check if the contactor is closed
		checkState(contactor, current_state, wanted_state, prechargerOrContactor, numOfTrials, contactorNum);
	}
}

void physciallyChangeContactor(ContactorInfo_t* contactor, ContactorState state)
{
	// This function will take in the contactor of interest for the precharger and also either 1 or 0
	// contactor = this function will pass in the contactor we want to change the state of
	// state = the state we want the contactor to be in (open, closed, closing, error)

	// GPIO_PIN_SET = 1
	// GPIO_PIN_RESET = 0

	switch (state){
			case CLOSED: // if it's in 'open' state, open the contactor

				HAL_GPIO_WritePin(contactor->GPIO_Port, contactor->GPIO_Pin, GPIO_PIN_RESET);
				break;
			case CLOSING: // if it's in 'closing' state, close the contactor
				HAL_GPIO_WritePin(contactor->GPIO_Port, contactor->GPIO_Pin, GPIO_PIN_SET);
				break;
			default:
	}
}

void checkState(ContactorInfo_t* contactor, ContactorState current_state, ContactorState wanted_state, char* prechargerOrContactor, int* numOfTrials, char* contactorNum){
	// added the prechargerOrContactor for error checking, that's it
	// check if the contactor is closed
	// problem. this case won't work for opening case
	if (HAL_GPIO_ReadPin(contactor.GPIO_Port, contactor.GPIO_Pin) == wanted_state){
		contactor.GPIO_State = wanted_state;
		if (wanted_state == CLOSED){
			//if (precharger):
			// if we get the signal from the precharger saying it's done:
			//		numOfTrials = 0;
			//		pin_set = 1;
			// else:
			//		numOfTrials++;
			//		pin_set = 0

			//if (gatekeeper):
			// if sensors tell us power is flowing through contactors:
			//		numOfTrials = 0;
			//		pin_set = 1;
			// else:
			//		numOfTrials++;
			//		pin_set = 0;
		}

		else if (wanted_state == OPEN){
			//if (precharger):
			// if we get the signal from the precharger saying no signal is going through:
			//		numOfTrials = 0;
			//		pin_set = 1;
			// else:
			//		numOfTrials++;
			//		pin_set = 0;

			//if (gatekeeper):
			// if sensors tell us power is NOT flowing through contactors:
			//		numOfTrials = 0;
			//		pin_set = 1;
			// else:
			//		numOfTrials++;
			//		pin_set = 0;
		}
	}
	else{
		// increase the checking system
		numOfTrials++;

		// try closing the contactor again
		printf("%s is not in the %s state, so trying again", prechargerOrContactor[contactorNum], stateNames[state]);
//		tryChangingContactor(contactor, current_state, wanted_state, TIMETOCLOSERETRY_DELAY, prechargerOrContactor, numOfTrials, contactorNum);
		// throw an error if a certain amount of time has passed??
	}
}





















//do we need this????? is it repetitive?????
//void PreChargerContactor(ContactorInfo_t* preContactor, ContactorState state)
//{
//	// contactor = this function will pass in the precharger contactor we want to change the state of
//	// state = the state we want the contactor to be in (open, closed, closing, error)
//	// GPIO_PIN_SET = 1
//	// GPIO_PIN_RESET = 0
//	switch (state){
//			case OPEN: // if it's in 'open' state, open the contactor
//				HAL_GPIO_WritePin(preContactor->GPIO_Port, preContactor->GPIO_Pin, GPIO_PIN_RESET);
//				break;
//			case CLOSING: // if it's in 'closing' state, close the contactor
//				HAL_GPIO_WritePin(preContactor->GPIO_Port, preContactor->GPIO_Pin, GPIO_PIN_SET);
//				break;
//			default:
//	}
//}






