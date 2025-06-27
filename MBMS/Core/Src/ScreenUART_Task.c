#include "ScreenUART_Task.h"

void UART_ScreenTask(void* arg){
    uint8_t txBuffer[SCREEN_DATA_BUFFER_SIZE];
    size_t txLen;

    for (;;) {
        // Serialize screenData into txBuffer
        serializeScreenData(txBuffer, &screenData, &txLen);

        // Transmit over UART4 (blocking)
        HAL_UART_Transmit(&huart4, txBuffer, txLen, HAL_MAX_DELAY);

        // Delay for 1000 ms (1 second)
        osDelay(1000);
    }
}

// Helper: write uint8_t to buffer
static void write_uint8(uint8_t **buf_ptr, uint8_t val) {
    **buf_ptr = val;
    (*buf_ptr)++;
}

// Helper: write uint16_t to buffer (little endian)
static void write_uint16(uint8_t **buf_ptr, uint16_t val) {
    **buf_ptr = val & 0xFF;
    (*buf_ptr)++;
    **buf_ptr = (val >> 8) & 0xFF;
    (*buf_ptr)++;
}

// Helper: write int16_t to buffer (little endian)
static void write_int16(uint8_t **buf_ptr, int16_t val) {
    write_uint16(buf_ptr, (uint16_t)val);
}

// Helper: write float to buffer (4 bytes IEEE 754)
static void write_float(uint8_t **buf_ptr, float val) {
    uint8_t *p = (uint8_t*)&val;
    for (int i = 0; i < 4; i++) {
        **buf_ptr = p[i];
        (*buf_ptr)++;
    }
}

void serializeScreenData(uint8_t *buffer, const ScreenDataDictionary *data, size_t *out_len) {
    uint8_t *p = buffer;

    // Serialize ContactorScreen: 5 x BoardStatus
    #define SERIALIZE_BOARDSTATUS(bs) do { \
        write_uint8(&p, (bs).heartbeat); \
        write_uint8(&p, (uint8_t)(bs).prechargerState); \
        write_uint8(&p, (uint8_t)(bs).contactorState); \
        write_float(&p, (bs).lineCurrent); \
        write_float(&p, (bs).chargeCurrent); \
    } while(0)

    SERIALIZE_BOARDSTATUS(data->contactorScreen.commonBoard);
    SERIALIZE_BOARDSTATUS(data->contactorScreen.motorBoard);
    SERIALIZE_BOARDSTATUS(data->contactorScreen.arrayBoard);
    SERIALIZE_BOARDSTATUS(data->contactorScreen.lvBoard);
    SERIALIZE_BOARDSTATUS(data->contactorScreen.chargeBoard);

    // Serialize TripScreen: 20 uint8_t fields
    write_uint8(&p, data->tripScreen.highCellVoltageTrip);
    write_uint8(&p, data->tripScreen.lowCellVoltageTrip);
    write_uint8(&p, data->tripScreen.commonHighCurrentTrip);
    write_uint8(&p, data->tripScreen.motorHighCurrentTrip);
    write_uint8(&p, data->tripScreen.arrayHighCurrentTrip);
    write_uint8(&p, data->tripScreen.lvHighCurrentTrip);
    write_uint8(&p, data->tripScreen.chargeHighCurrentTrip);
    write_uint8(&p, data->tripScreen.protectionTrip);
    write_uint8(&p, data->tripScreen.orionMsgTimeoutTrip);
    write_uint8(&p, data->tripScreen.contactorDiscUnexpected);
    write_uint8(&p, data->tripScreen.contactorConnUnexpected);
    write_uint8(&p, data->tripScreen.commonHeartbeatDead);
    write_uint8(&p, data->tripScreen.motorHeartbeatDead);
    write_uint8(&p, data->tripScreen.arrayHeartbeatDead);
    write_uint8(&p, data->tripScreen.lvHeartbeatDead);
    write_uint8(&p, data->tripScreen.chargeHeartbeatDead);
    write_uint8(&p, data->tripScreen.mpsDisabledTrip);
    write_uint8(&p, data->tripScreen.esdEnabledTrip);
    write_uint8(&p, data->tripScreen.highTempTrip);
    write_uint8(&p, data->tripScreen.lowTempTrip);

    // Serialize PowerSelectionStatus: 11 uint8_t fields
    write_uint8(&p, data->powerStatus.nMainPowerSwitch);
    write_uint8(&p, data->powerStatus.ExternalShutdown);
    write_uint8(&p, data->powerStatus.EN1);
    write_uint8(&p, data->powerStatus.nDCDC_Fault);
    write_uint8(&p, data->powerStatus.n3A_OC);
    write_uint8(&p, data->powerStatus.nDCDC_On);
    write_uint8(&p, data->powerStatus.nCHG_Fault);
    write_uint8(&p, data->powerStatus.nCHG_On);
    write_uint8(&p, data->powerStatus.nCHG_LV_En);
    write_uint8(&p, data->powerStatus.ABATT_Disable);
    write_uint8(&p, data->powerStatus.Key);

    // Serialize MBMSStatusScreen
    write_float(&p, data->mbmsStatus.auxBatteryVoltage);
    write_uint8(&p, data->mbmsStatus.strobeBMSLight);
    write_uint8(&p, data->mbmsStatus.nChargeEnable);
    write_uint8(&p, data->mbmsStatus.nChargeSafety);
    write_uint8(&p, data->mbmsStatus.nDischargeEnable);
    write_uint8(&p, data->mbmsStatus.orionCANRx);
    write_uint8(&p, data->mbmsStatus.dischargeShouldTrip);
    write_uint8(&p, data->mbmsStatus.chargeShouldTrip);
    write_uint8(&p, (uint8_t)data->mbmsStatus.startupState);
    write_uint8(&p, (uint8_t)data->mbmsStatus.systemState);

    // Serialize BatteryInfoScreen
    write_int16(&p, data->batteryInfo.packCurrent);
    write_uint16(&p, data->batteryInfo.packVoltage);
    write_uint8(&p, data->batteryInfo.packSOC);
    write_uint16(&p, data->batteryInfo.packAmphours);
    write_uint8(&p, data->batteryInfo.packDOD);
    write_uint8(&p, data->batteryInfo.lowTemp);
    write_uint8(&p, data->batteryInfo.avgTemp);
    write_uint8(&p, data->batteryInfo.highTemp);
    write_uint16(&p, data->batteryInfo.lowCellVoltage);
    write_uint16(&p, data->batteryInfo.highCellVoltage);
    write_uint16(&p, data->batteryInfo.maxCellVoltage);

    // Set output length
    *out_len = (size_t)(p - buffer);
}
