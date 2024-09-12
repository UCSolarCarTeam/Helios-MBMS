/*
 * main.cpp
 *
 *	Master BMS
 *
 *  Created on: Aug 21, 2024
 *      Author: khadeejaabbas
 *      System: Core
 *      Role: Application Entry Point
 */

//Includes
#include "main.hpp"

#include "boot.hpp"

//main
//Application Entry Point
int main(void) {

	//System Setup
	//Same as saying SystemInitalize() == QA_Fail since a fail is a 1 (a logical true)
	if (SystemInitalize()) {
		//The reason we're chucking the code into an infinite loop if there's an error is because this code needs to run successfully in order for the rest of the system to run successfully (power, etc.)
		while(1) {}
	}

	//Processing Loop
	while(1){


	}

	return 0;
}




