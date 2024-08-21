/*
 * ShutoffTask.c
 *
 *  Created on: Jul 20, 2024
 *      Author: khadeejaabbas
 */

#include "ShutoffTask.h"

void ShutoffTask(void* arg)
{
    for (;;)
    {
    	Shutoff();
    }
}
