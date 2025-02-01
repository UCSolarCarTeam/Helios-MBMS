/*
 * ContactorState.h
 *
 *  Created on: Sep 18, 2024
 *      Author: khadeejaabbas
 */
// this ensure this file is included only once in the project (not repeated)
#pragma once

// macro for the precharger delay (1 second)
#define PRECHARGER_DELAY 1000
// macro for the time to close delay (1 second)
#define TIMETOCLOSE_DELAY 1000
// macro for the time to open delay (1 second)
#define TIMETOOPEN_DELAY 1000
// macro for the time to retry closing delay (10 seconds)
#define TIMETOCLOSERETRY_DELAY 10000

// number of prechargers for the list of structs below
#define NUM_PRECHARGERS 7
// number of contacotors for the list of structs below
#define NUM_CONTACTORS 7

// assigning all the prechargers a number
//typedef enum
//{
//	Precharger,
//	CommonContactorPrecharger,
//	LVContactorPrecharger,
//	MotorContactorPrecharger,
//	ChargeContactorPrecharger,
//	ArrayContactorPrecharger,
//	ContactorLEDsPrecharger
//} Prechargers;

// assigning all the contactors a number
//typedef enum
//{
//	Gatekeeper,
//	CommonContactor,
//	LVContactor,
//	MotorContactor,
//	ChargeContactor,
//	ArrayContactor,
//	ContactorLEDs
//} Contactors;

// assigning all the states of the contactors a number
typedef enum
{
    OPEN,
    CLOSED,
    CLOSING, // Intermediate state between open and closed
    CONTACTOR_ERROR
} ContactorState;


// a struct for the general info for each contactor
typedef struct
{
	GPIO_TypeDef * GPIO_Port;
	uint16_t GPIO_Pin;
	GPIO_TypeDef * GPIO_Port_Sense;
	uint16_t GPIO_Pin_Sense;
    ContactorState GPIO_State;
} ContactorInfo_t;

// a struct for the queue message for each contactor
typedef struct
{
	GPIO_TypeDef * GPIO_Wanted_Port;
	uint16_t GPIO_Wanted_Pin;
    ContactorState GPIO_Wanted_State;
} ContactorQueueMessage_t;

// creating list of structs and initalizing
//ContactorInfo_t prechargers[NUM_PRECHARGERS] = {
//		[0] = {
//				.GPIO_Port = Common_Contactor_Precharger_Output_GPIO_Port,
//				.GPIO_Pin = Common_Contactor_Precharger_Output_Pin,
//				.GPIO_Port_Sense = Common_Contactor_Precharger_Sense_GPIO_Port,
//				.GPIO_Pin_Sense = Common_Contactor_Precharger_Sense_Pin,
//				.GPIO_State = OPEN
//		},
//		[1] = {
//				.GPIO_Port = LV_Contactor_Precharger_Output_GPIO_Port,
//				.GPIO_Pin = LV_Contactor_Precharger_Output_Pin,
//				.GPIO_Port_Sense = LV_Contactor_Precharger_Sense_GPIO_Port,
//				.GPIO_Pin_Sense = LV_Contactor_Precharger_Sense_Pin,
//				.GPIO_State = OPEN
//		},
//		[2] = {
//				.GPIO_Port = Motor_Contactor_Precharger_Output_GPIO_Port,
//				.GPIO_Pin = Motor_Contactor_Precharger_Output_Pin,
//				.GPIO_Port_Sense = Motor_Contactor_Precharger_Output_GPIO_Port,
//				.GPIO_Pin_Sense = Motor_Contactor_Precharger_Output_Pin,
//				.GPIO_State = OPEN
//		},
//		[3] = {
//				.GPIO_Port = Charge_Contactor_Precharger_Output_GPIO_Port,
//				.GPIO_Pin = Charge_Contactor_Precharger_Output_Pin,
//				.GPIO_Port_Sense = Charge_Contactor_Precharger_Sense_GPIO_Port,
//				.GPIO_Pin_Sense = Charge_Contactor_Precharger_Sense_Pin,
//				.GPIO_State = OPEN
//		},
//		[4] = {
//				.GPIO_Port = Array_Contactor_Precharger_GPIO_Port,
//				.GPIO_Pin = Array_Contactor_Precharger_Pin,
//				.GPIO_Port_Sense = Charge_Contactor_Precharger_Sense_GPIO_Port,
//				.GPIO_Pin_Sense = Charge_Contactor_Precharger_Sense_Pin,
//				.GPIO_State = OPEN
//		},
//		[5] = {
//				.GPIO_Port = Contactor_LEDs_Precharger_GPIO_Port,
//				.GPIO_Pin = Contactor_LEDs_Precharger_Pin,
//				.GPIO_State = OPEN
//		}
//};
//ContactorInfo_t gatekeepers[NUM_CONTACTORS] = {
//		[0] = {
//				.GPIO_Port = Common_Contactor_GPIO_Port,
//				.GPIO_Pin = Common_Contactor_Pin,
//				.GPIO_State = CLOSED
//		},
//		[1] = {
//				.GPIO_Port = LV_Contactor_GPIO_Port,
//				.GPIO_Pin = LV_Contactor_Pin,
//				.GPIO_State = OPEN
//		},
//		[2] = {
//				.GPIO_Port = Motor_Contactor_GPIO_Port,
//				.GPIO_Pin = Motor_Contactor_Pin,
//				.GPIO_State = OPEN
//		},
//		[3] = {
//				.GPIO_Port = Charge_Contactor_GPIO_Port,
//				.GPIO_Pin = Charge_Contactor_Pin,
//				.GPIO_State = OPEN
//		},
//		[4] = {
//				.GPIO_Port = Array_Contactor_GPIO_Port,
//				.GPIO_Pin = Array_Contactor_Pin,
//				.GPIO_State = OPEN
//		},
//		[5] = {
//				.GPIO_Port = Contactor_LEDs_GPIO_Port,
//				.GPIO_Pin = Contactor_LEDs_Pin,
//				.GPIO_State = OPEN
//		}
//};
//
//
//
//// listing all the names of the enum for more accurate error checking
//const char* prechargerNames[] = {"CommonContactorPrecharger", "LVContactorPrecharger", "MotorContactorPrecharger", "ChargeContactorPrecharger", "ArrayContactorPrecharger", "ContactorLEDsPrecharger"};
//const char* contactorNames[] = {"CommonContactor", "LVContactor", "MotorContactor", "ChargeContactor", "ArrayContactor", "ContactorLEDs"};

const int* prechargerNumberOfTries[] = {0, 0, 0, 0, 0, 0};
const int* contactorNumberOfTries[] = {0, 0, 0, 0, 0, 0};

const char* stateNames[] = {"OPEN", "CLOSED", "CLOSING", "CONTACTOR_ERROR"};




