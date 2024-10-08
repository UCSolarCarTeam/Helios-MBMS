/*
 * handlers.cpp
 *
 *	Master BMS
 *
 *  Created on: Aug 21, 2024
 *      Author: khadeejaabbas
 *      System: Core
 *      Role:
 */


//Includes
#include "handlers.hpp"


	//---------------------------
	//Exception Handler Functions

//NMI_Handler
//NMI stands for non-maskable interrupts
//Exception Handler Function
void  NMI_Handler(void){

}

//HardFault_Handler
//usage fault handler (example: having a pointer to a class that is currently null)
//Exception Handler Function
void  HardFault_Handler(void){
	while(1) {}
}

//MemManage_Handler
//usage fault handler (example: access wrong memory location)
//Exception Handler Function
void  MemManage_Handler(void){
	while(1) {}
}

//BusFault_Handler
//usage fault handler
//Exception Handler Function
void  BusFault_Handler(void){
	while(1) {}
}

//UsageFault_Handler
//Exception Handler Function
void  UsageFault_Handler(void){
	while(1) {}
}

//SVC_Handler
//Exception Handler Function
void  SVC_Handler(void){

}

//DebugMon_Handler
//Exception Handler Function
void  DebugMon_Handler(void){

}

//PendSV_Handler
//Exception Handler Function
void  PendSV_Handler(void){

}

//SysTick_Handler
//This ensures the system timer is working correctly and with the right priority. We want this a high priority because we want to timer to continue regardless of what happens
//Exception Handler Function
void  SysTick_Handler(void){
	HAL_IncTick();
}


	//-------------------------
	//Interrupt Handler Methods




