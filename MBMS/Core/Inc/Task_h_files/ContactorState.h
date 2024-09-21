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
// macro for the time to close delay (made up rn)
#define TIMETOCLOSE_DELAY 10

// number of prechargers for the list of structs below
#define NUM_PRECHARGERS 7
// number of contacotors for the list of structs below
#define NUM_CONTACTORS 7

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

// making an array of all the prechargers and contactors as structs
ContactorInfo_t prechargers[NUM_PRECHARGERS];
ContactorInfo_t gatekeepers[NUM_CONTACTORS];






