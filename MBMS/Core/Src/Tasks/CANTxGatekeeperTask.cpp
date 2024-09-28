/*
 * TxGatekeeperTask.cpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas
 */
#include "CANTxGatekeeperTask.hpp"

void CANTxGatekeeperTask(void* arg)
{
    while(1)
    {
    	CANTxGatekeeper();
    }
}

void CANTxGatekeeper(void* arg)
{

}


