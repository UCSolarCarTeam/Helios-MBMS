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


TripScreen convertToTripScreen(const MBMSTrip* t) {
    TripScreen screen = {
        .highCellVoltageTrip     = t->highCellVoltageTrip,
        .lowCellVoltageTrip      = t->lowCellVoltageTrip,
        .commonHighCurrentTrip   = t->commonHighCurrentTrip,
        .motorHighCurrentTrip    = t->motorHighCurrentTrip,
        .arrayHighCurrentTrip    = t->arrayHighCurrentTrip,
        .lvHighCurrentTrip       = t->LVHighCurrentTrip,
        .chargeHighCurrentTrip   = t->chargeHighCurrentTrip,
        .protectionTrip          = t->protectionTrip,
        .orionMsgTimeoutTrip     = t->orionMessageTimeoutTrip,
        .contactorDiscUnexpected = t->contactorDisconnectedUnexpectedlyTrip,
        .contactorConnUnexpected = t->contactorConnectedUnexpectedlyTrip,
        .commonHeartbeatDead     = t->commonHeartbeatDeadTrip,
        .motorHeartbeatDead      = t->motorHeartbeatDeadTrip,
        .arrayHeartbeatDead      = t->arrayHeartbeatDeadTrip,
        .lvHeartbeatDead         = t->LVHeartbeatDeadTrip,
        .chargeHeartbeatDead     = t->chargeHeartbeatDeadTrip,
        .mpsDisabledTrip         = t->MPSDisabledTrip,
        .esdEnabledTrip          = t->ESDEnabledTrip,
        .highTempTrip            = t->highTemperatureTrip,
        .lowTempTrip             = t->lowTemperatureTrip
    };

    return screen;
}

MBMSStatusScreen convertToMBMSStatusScreen(const MBMSStatus* status) {
    MBMSStatusScreen screen = {
        .auxBatteryVoltage     = status->auxilaryBattVoltage, 
        .strobeBMSLight        = status->strobeBMSLight,
        .nChargeEnable         = status->chargeEnable,
        .nChargeSafety         = status->nChargeSafety,
        .nDischargeEnable      = status->dischargeEnable,
        .orionCANRx            = status->orionCANReceived,
        .dischargeShouldTrip   = status->dischargeShouldTrip,
        .chargeShouldTrip      = status->chargeShouldTrip,
        .startupState          = (StartupState)status->startupState,
        .systemState           = (CarState)status->carState
    };

    return screen;
}