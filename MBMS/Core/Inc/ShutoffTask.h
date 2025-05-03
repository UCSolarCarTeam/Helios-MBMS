/*
 * ShutoffTask.hpp
 *
 *  Created on: Sep 7, 2024
 *      Author: khadeejaabbas, millaineli
 */

#ifndef INC_TASK_H_FILES_SHUTOFFTASK_H_
#define INC_TASK_H_FILES_SHUTOFFTASK_H_

#include <stdint.h>


#define KEY 0
#define MPS 1
#define HARD 2
#define SOFT 3


// function definitions
void ShutoffTask(void* arg);

void Shutoff();


#endif /* INC_TASK_H_FILES_SHUTOFFTASK_H_ */
