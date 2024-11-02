/*
 * RxGatekeeperTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 */
#include "CANRxGatekeeperTask.hpp"

void CANRxGatekeeperTask(void* arg)
{
    while(1)
    {
    	CANRxGatekeeper();
    }
}

void CANRxGatekeeper(void* arg)
{

}


