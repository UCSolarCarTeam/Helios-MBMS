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

void Precharger(ContactorInfo_t* contactor)
{
	// here the input is a pointer to what contactor we want closed, it will be an enum (a number)

	// once the precharger gets the 'closing' command from the gatekeeper which comes from the FreeRTOS queue, it must first close its own switch
	// every corresponding precharger and contactor have the same number (i.e motor precharger and contactor are both 2)
	tryClosingContactor(prechargers[contactor], PRECHARGER_DELAY);

	// if the precharger doesn't close, the function will be put into a recursive loop and will never end. Thus a watchdog needs to stop it to throw an error

	// secondly, it must close the array the gatekeeper wanted closed (this is so the contactor isn't flooded with high voltage)
	tryClosingContactor(gatekeepers[contactor], TIMETOCLOSE_DELAY);

	// if the contactor doesn't close, the function will be put into a recursive loop and will never end. Thus a watchdog needs to stop it to throw an error

	// thirdly, once the array is closed and a 'transition voltage' is provided by the precharger, the precharger opens and the state of the contactor is changed from 'closing' to 'closed'
	// this means that there won't be a voltage spike now so thats safer 😌
	physciallyChangeContactor(prechargers[contactor], OPEN);

	//do we need to check if the precharger is open?

	// check the voltage by reading the line and making sure it's what we want (the battery voltage)
	// if it is, we can change the state from 'closing' to 'closed'
	// we first have to change the state of the corresponding contactor
}

void tryClosingContactor(ContactorInfo_t* contactor, uint32_t delayTime){
	// set state to closing for the contactor specified
	contactor.GPIO_State = CLOSING;

	// try physically closing the contactor
	physciallyChangeContactor(contactor, CLOSING);

	// Now I think there should some delay before the contactor's closed
	HAL_Delay(delayTime);

	// check if the contactor is closed
	checkClosed(contactor);

}

void physciallyChangeContactor(ContactorInfo_t* contactor, ContactorState state)
{
	// This function will take in the contactor of interest for the precharger and also either 1 or 0
	// contactor = this function will pass in the contactor we want to change the state of
	// state = the state we want the contactor to be in (open, closed, closing, error)

	// GPIO_PIN_SET = 1
	// GPIO_PIN_RESET = 0

	switch (state){
			case OPEN: // if it's in 'open' state, open the contactor
				HAL_GPIO_WritePin(contactor->GPIO_Port, contactor->GPIO_Pin, GPIO_PIN_RESET);
				break;
			case CLOSING: // if it's in 'closing' state, close the contactor
				HAL_GPIO_WritePin(contactor->GPIO_Port, contactor->GPIO_Pin, GPIO_PIN_SET);
				break;
			default:
	}
}

void checkClosed(ContactorInfo_t* contactor){
	// check if the contactor is closed
	if (HAL_GPIO_ReadPin(contactor.GPIO_Port, contactor.GPIO_Pin) == 1){
		contactor.GPIO_State = CLOSED;
	}
	else{
		// try closing the contactor again
		printf("Contactor is not closed so trying again");
		tryClosingContactor(contactor);
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






