/*
 * ContactorState.h
 *
 *  Created on: Sep 18, 2024
 *      Author: khadeejaabbas
 */
// this ensure this file is included only once in the project (not repeated)
#pragma once

// macro for the precharger delay (I just made up 10 for rn)
#define PRECHARGER_DELAY 10

// assigning all the prechargers a number
typedef enum
{
	Precharger,
	CommonContactorPrecharger,
	LVContactorPrecharger,
	MotorContactorPrecharger,
	ChargeContactorPrecharger,
	ArrayContactorPrecharger,
	ContactorLEDsPrecharger
} Prechargers;

// assigning all the contactors a number
typedef enum
{
	Gatekeeper,
	CommonContactor,
	LVContactor,
	MotorContactor,
	ChargeContactor,
	ArrayContactor,
	ContactorLEDs
} Contactors;

// assigning all the states of the contactors a number
typedef enum
{
    OPEN,
    CLOSING, // Intermediate state between open and closed
    CLOSED,
    CONTACTOR_ERROR
} ContactorState;


// a struct for the general info for each contactor

typedef struct
{
	GPIO_TypeDef * GPIO_Port;
	uint16_t GPIO_Pin;
    ContactorState GPIO_State;
} ContactorInfo_t;



