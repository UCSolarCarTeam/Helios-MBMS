/*
 * ValueUpdater.h
 *
 *  Created on: May 7, 2025
 *      Author: m
 */

#ifndef INC_VALUEUPDATERTASK_H_
#define INC_VALUEUPDATERTASK_H_

#include <stdint.h>

void ValueUpdaterTask(void* arg);
void ValueUpdater();
void updateContactorInfoStruct();
void updateOrionInfoStruct();
void updatePowerSelectionStruct();
void updateContactorInfo(uint8_t contactor, uint8_t prechargerClosed, uint8_t prechargerClosing, uint8_t prechargerError, uint8_t contactorClosed, uint8_t contactorClosing, uint8_t contactorError, int16_t lineCurrent, int16_t chargeCurrent, uint8_t BPSerror);

#endif /* INC_VALUEUPDATERTASK_H_ */
