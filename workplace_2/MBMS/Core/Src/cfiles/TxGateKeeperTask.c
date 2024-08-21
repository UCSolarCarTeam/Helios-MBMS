/*
 * TxGateKeeperTask.c
 *
 *  Created on: Jul 20, 2024
 *      Author: khadeejaabbas
 */
#include "TxGateKeeperTask.h"

void TxGateKeeperTask(void* arg)
{
    for (;;)
    {
    	TxGateKeeper();
    }
}


