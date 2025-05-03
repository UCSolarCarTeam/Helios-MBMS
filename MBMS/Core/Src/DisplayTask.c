/*
 * DisplayTask.cpp
 *
 *  Created on: April, 2025
 *      Author: MounikaKompally
 */

#include "DisplayTask.h"
#include "ReadPowerGPIO.h"
#include "MBMS.h"

extern SPI_HandleTypeDef hspi1;
extern BatteryInfo batteryInfo;
extern MBMSStatus mbmsStatus;
extern MBMSTrip mbmsTrip;
extern ContactorInfo contactorInfo[6];
//extern PowerSelectionStatus; Not in BatteryControlTask so can't get info rn

void DisplayTask(void* arg)
{
    while(1)
    {
    	Display();
    	osDelay(1000);

    }
}

void Display()
{
	uint8_t data[138];

	data[0] = (uint8_t)(batteryInfo.packCurrent | 0x00FF);
	data[1] = (uint8_t)(batteryInfo.packCurrent >> 8);
	data[2] = (uint8_t)(batteryInfo.packVoltage | 0x00FF);
	data[3] = (uint8_t)(batteryInfo.packVoltage >> 8);
	data[4] = (uint8_t)(batteryInfo.packSOC);
	data[5] = (uint8_t)(batteryInfo.packAmphours | 0x00FF);
	data[6] = (uint8_t)(batteryInfo.packAmphours >> 8);
	data[7] = (uint8_t)(batteryInfo.packDOD);
	data[8] = (uint8_t)(batteryInfo.highTemp);
	data[9] = (uint8_t)(batteryInfo.lowTemp);
	data[10] = (uint8_t)(batteryInfo.avgTemp);
	data[11] = (uint8_t)(batteryInfo.maxCellVoltage | 0x00FF);
	data[12] = (uint8_t)(batteryInfo.maxCellVoltage >> 8);
	data[13] = (uint8_t)(batteryInfo.minCellVoltage | 0x00FF);
	data[14] = (uint8_t)(batteryInfo.minCellVoltage >> 8);
	data[15] = (uint8_t)(batteryInfo.maxPackVoltage | 0x00FF);
	data[16] = (uint8_t)(batteryInfo.maxPackVoltage >> 8);
	data[17] = (uint8_t)(batteryInfo.minPackVoltage | 0x00FF);
	data[18] = (uint8_t)(batteryInfo.minPackVoltage >> 8);
	data[19];
	data[20] = (uint8_t)(mbmsStatus.auxilaryBattVoltage);
	data[21] = (uint8_t)(mbmsStatus.strobeBMSLight);
	data[22] = (uint8_t)(mbmsStatus.allowCharge);
	data[23] = (uint8_t)(mbmsStatus.chargeSafety);
	data[24] = (uint8_t)(mbmsStatus.highVoltageEnableState);
	data[25] = (uint8_t)(mbmsStatus.allowDischarge);
	data[26] = (uint8_t)(mbmsStatus.orionCANReceived);
	data[27] = (uint8_t)(mbmsStatus.dischargeShouldTrip);
    data[28] = (uint8_t)(mbmsStatus.chargeShouldTrip);
    data[29] = (uint8_t)(mbmsStatus.startupState);
    data[30];
	data[31] = (uint8_t)(mbmsTrip.highCellVoltageTrip);
	data[32] = (uint8_t)(mbmsTrip.lowCellVoltageTrip);
	data[33] = (uint8_t)(mbmsTrip.commonHighCurrentTrip);
	data[34] = (uint8_t)(mbmsTrip.motorHighCurrentTrip);
	data[35] = (uint8_t)(mbmsTrip.arrayHighCurrentTrip);
	data[36] = (uint8_t)(mbmsTrip.LVHighCurrentTrip);
	data[37] = (uint8_t)(mbmsTrip.chargeHighCurrentTrip);
	data[38] = (uint8_t)(mbmsTrip.protectionTrip);
	data[39] = (uint8_t)(mbmsTrip.orionMessageTimeoutTrip);
	data[40] = (uint8_t)(mbmsTrip.contactorDisconnectedUnexpectedlyTrip);
	data[41] = (uint8_t)(mbmsTrip.contactorConnectedUnexpectedlyTrip);
	data[42] = (uint8_t)(mbmsTrip.highBatteryTrip);
	data[43] = (uint8_t)(mbmsTrip.commonHeartbeatDeadTrip);
	data[44] = (uint8_t)(mbmsTrip.motor1HeartbeatDeadTrip);
	data[45] = (uint8_t)(mbmsTrip.motor2HeartbeatDeadTrip);
	data[46] = (uint8_t)(mbmsTrip.arrayHeartbeatDeadTrip);
	data[47] = (uint8_t)(mbmsTrip.LVHeartbeatDeadTrip);
	data[48] = (uint8_t)(mbmsTrip.chargeHeartbeatDeadTrip);
	data[49] = (uint8_t)(mbmsTrip.MPSDisabledTrip);
	data[50] = (uint8_t)(mbmsTrip.keyOffTrip);
	data[52] = (uint8_t)(mbmsTrip.hardBatteryLimitTrip);
	data[52] = (uint8_t)(mbmsTrip.softBatteryLimitTrip);
	data[53];
	data[54] = (uint8_t)(contactorInfo[0].prechargerClosed);
	data[55] = (uint8_t)(contactorInfo[0].prechargerClosing);
	data[56] = (uint8_t)(contactorInfo[0].prechargerError);
	data[57] = (uint8_t)(contactorInfo[0].contactorState);
	data[58] = (uint8_t)(contactorInfo[0].contactorError);
    data[59] = (uint8_t)(contactorInfo[0].voltage | 0x00FF);
    data[60] = (uint8_t)(contactorInfo[0].voltage >> 8);
    data[61] = (uint8_t)(contactorInfo[0].current| 0x00FF);
    data[62] = (uint8_t)(contactorInfo[0].current >> 8);
    data[63] = (uint8_t)(contactorInfo[0].heartbeat| 0x00FF);
    data[64] = (uint8_t)(contactorInfo[0].heartbeat >> 8);
    data[65];
	data[66] = (uint8_t)(contactorInfo[1].prechargerClosed);
	data[67] = (uint8_t)(contactorInfo[1].prechargerClosing);
	data[68] = (uint8_t)(contactorInfo[1].prechargerError);
	data[69] = (uint8_t)(contactorInfo[1].contactorState);
	data[70] = (uint8_t)(contactorInfo[1].contactorError);
    data[71] = (uint8_t)(contactorInfo[1].voltage | 0x00FF);
    data[72] = (uint8_t)(contactorInfo[1].voltage >> 8);
    data[73] = (uint8_t)(contactorInfo[1].current| 0x00FF);
    data[74] = (uint8_t)(contactorInfo[1].current >> 8);
    data[75] = (uint8_t)(contactorInfo[1].heartbeat| 0x00FF);
    data[76] = (uint8_t)(contactorInfo[1].heartbeat >> 8);
    data[77];
	data[78] = (uint8_t)(contactorInfo[2].prechargerClosed);
	data[79] = (uint8_t)(contactorInfo[2].prechargerClosing);
	data[80] = (uint8_t)(contactorInfo[2].prechargerError);
	data[81] = (uint8_t)(contactorInfo[2].contactorState);
	data[82] = (uint8_t)(contactorInfo[2].contactorError);
    data[83] = (uint8_t)(contactorInfo[2].voltage | 0x00FF);
    data[84] = (uint8_t)(contactorInfo[2].voltage >> 8);
    data[85] = (uint8_t)(contactorInfo[2].current| 0x00FF);
    data[86] = (uint8_t)(contactorInfo[2].current >> 8);
    data[87] = (uint8_t)(contactorInfo[2].heartbeat| 0x00FF);
    data[88] = (uint8_t)(contactorInfo[2].heartbeat >> 8);
    data[89];
	data[90] = (uint8_t)(contactorInfo[3].prechargerClosed);
	data[91] = (uint8_t)(contactorInfo[3].prechargerClosing);
	data[92] = (uint8_t)(contactorInfo[3].prechargerError);
	data[93] = (uint8_t)(contactorInfo[3].contactorState);
	data[94] = (uint8_t)(contactorInfo[3].contactorError);
    data[95] = (uint8_t)(contactorInfo[3].voltage | 0x00FF);
    data[96] = (uint8_t)(contactorInfo[3].voltage >> 8);
    data[97] = (uint8_t)(contactorInfo[3].current| 0x00FF);
    data[98] = (uint8_t)(contactorInfo[3].current >> 8);
    data[99] = (uint8_t)(contactorInfo[3].heartbeat| 0x00FF);
    data[100] = (uint8_t)(contactorInfo[3].heartbeat >> 8);
    data[102];
	data[103] = (uint8_t)(contactorInfo[4].prechargerClosed);
	data[104] = (uint8_t)(contactorInfo[4].prechargerClosing);
	data[105] = (uint8_t)(contactorInfo[4].prechargerError);
	data[106] = (uint8_t)(contactorInfo[4].contactorState);
	data[107] = (uint8_t)(contactorInfo[4].contactorError);
    data[108] = (uint8_t)(contactorInfo[4].voltage | 0x00FF);
    data[109] = (uint8_t)(contactorInfo[4].voltage >> 8);
    data[110] = (uint8_t)(contactorInfo[4].current| 0x00FF);
    data[111] = (uint8_t)(contactorInfo[4].current >> 8);
    data[112] = (uint8_t)(contactorInfo[4].heartbeat| 0x00FF);
    data[113] = (uint8_t)(contactorInfo[4].heartbeat >> 8);
    data[114];
	data[115] = (uint8_t)(contactorInfo[5].prechargerClosed);
	data[116] = (uint8_t)(contactorInfo[5].prechargerClosing);
	data[117] = (uint8_t)(contactorInfo[5].prechargerError);
	data[118] = (uint8_t)(contactorInfo[5].contactorState);
	data[119] = (uint8_t)(contactorInfo[5].contactorError);
    data[120] = (uint8_t)(contactorInfo[5].voltage | 0x00FF);
    data[121] = (uint8_t)(contactorInfo[5].voltage >> 8);
    data[122] = (uint8_t)(contactorInfo[5].current| 0x00FF);
    data[123] = (uint8_t)(contactorInfo[5].current >> 8);
    data[124] = (uint8_t)(contactorInfo[5].heartbeat| 0x00FF);
    data[125] = (uint8_t)(contactorInfo[5].heartbeat >> 8);
    data[126];
	data[127] = (uint8_t)(contactorInfo[6].prechargerClosed);
	data[128] = (uint8_t)(contactorInfo[6].prechargerClosing);
	data[129] = (uint8_t)(contactorInfo[6].prechargerError);
	data[130] = (uint8_t)(contactorInfo[6].contactorState);
	data[131] = (uint8_t)(contactorInfo[6].contactorError);
    data[132] = (uint8_t)(contactorInfo[6].voltage | 0x00FF);
    data[133] = (uint8_t)(contactorInfo[6].voltage >> 8);
    data[134] = (uint8_t)(contactorInfo[6].current| 0x00FF);
    data[135] = (uint8_t)(contactorInfo[6].current >> 8);
    data[136] = (uint8_t)(contactorInfo[6].heartbeat| 0x00FF);
    data[137] = (uint8_t)(contactorInfo[6].heartbeat >> 8);

	HAL_StatusTypeDef statusScreen;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

	statusScreen = HAL_SPI_Transmit(&hspi1, data, 19, 100U);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

}
