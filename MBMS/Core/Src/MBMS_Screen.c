#include "MBMS_Screen.h"

BatteryInfoScreen convertToScreenBatteryInfo(const BatteryInfo* b) {
    BatteryInfoScreen screen;
    screen.packCurrent      = (int16_t)b->packCurrent;
    screen.packVoltage      = (uint16_t)b->packVoltage;
    screen.packSOC          = (uint8_t)b->packSOC;
    screen.packAmphours     = (uint16_t)b->packAmphours;
    screen.packDOD          = (uint8_t)b->packDOD;
    screen.lowTemp          = (uint8_t)b->lowTemp;
    screen.avgTemp          = b->avgTemp;
    screen.highTemp         = b->highTemp;
    screen.lowCellVoltage   = (uint16_t)(b->lowCellVoltage);   // float V → mV
    screen.highCellVoltage  = (uint16_t)(b->highCellVoltage);
    screen.maxCellVoltage   = (uint16_t)(b->highCellVoltage);  // reusing since no `maxCellVoltage` in source
    return screen;
}



