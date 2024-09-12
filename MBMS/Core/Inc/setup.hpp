/*
 * setup.hpp
 *
 *	Master BMS
 *
 *  Created on: Aug 21, 2024
 *      Author: khadeejaabbas
 *      System: Core
 *      Role: Setup Code
 */

//Prevent Recursive Inclusion
#ifndef __SETUP_HPP_
#define __SETUP_HPP_

//Includes
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"


	//
	//QA stands for Quartz Arc
	//If it's not an error it will be 0. If there is an error, it will be anything other than 0
enum QA_Result : uint8_t {QA_OK = 0, QA_Fail};

//Prevent Recursive Inclusion
#endif /* SETUP_HPP_ */
