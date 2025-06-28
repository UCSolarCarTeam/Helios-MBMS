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

static BoardStatus convertToBoardStatus(const ContactorInfo* info) {
    BoardStatus status = {
        .heartbeat      = info->heartbeat,
        .lineCurrent    = info->lineCurrent,
        .chargeCurrent  = info->chargeCurrent,
        .prechargerState =
            info->prechargerError   ? PRECHARGE_ERROR   :
            info->prechargerClosing ? PRECHARGE_CLOSING :
            info->prechargerClosed  ? PRECHARGE_CLOSED  :
                                      PRECHARGE_OPEN,
        .contactorState =
            info->contactorError   ? CONTACTOR_ERROR   :
            info->contactorClosing ? CONTACTOR_CLOSING :
            info->contactorClosed  ? CONTACTOR_CLOSED  :
                                     CONTACTOR_OPEN
    };
    return status;
}

//Hard coded to match contactorInfo array size, flexibility can be added later if needed with enum or size parameter
ContactorScreen convertToContactorScreen(const ContactorInfo contactorInfo[5]) {
    ContactorScreen screen;
    screen.commonBoard = convertToBoardStatus(&contactorInfo[COMMON]);
    screen.motorBoard  = convertToBoardStatus(&contactorInfo[MOTOR]);
    screen.arrayBoard  = convertToBoardStatus(&contactorInfo[ARRAY]);
    screen.lvBoard     = convertToBoardStatus(&contactorInfo[LOWV]);
    screen.chargeBoard = convertToBoardStatus(&contactorInfo[CHARGE]);
    return screen;
}