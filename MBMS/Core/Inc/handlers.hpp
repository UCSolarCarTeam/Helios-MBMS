/*
 * handlers.hpp
 *
 *	Master BMS
 *
 *  Created on: Aug 21, 2024
 *      Author: khadeejaabbas
 *      System: Core
 *      Role: Exception and Interrupt Handlers
 */

//Prevent Recursive Inclusion
#ifndef __HANDLERS_HPP_
#define __HANDLERS_HPP_

//Includes
#include "setup.hpp"

//The reason we're doing this is because the Handler function defined in "startup_stm32f407vgtx.s" won't replace the default function with the ones we defined below unless they're C files. Currently, they're C++ files so we're converting them
extern "C" {


	//---------------------------
	//Exception Handler Functions

void  NMI_Handler(void);
void  HardFault_Handler(void);
void  MemManage_Handler(void);
void  BusFault_Handler(void);
void  UsageFault_Handler(void);
void  SVC_Handler(void);
void  DebugMon_Handler(void);
void  PendSV_Handler(void);
void  SysTick_Handler(void);

	//-------------------------
	//Interrupt Handler Methods

}


//Prevent Recursive Inclusion
#endif /* __HANDLERS_HPP_ */
