#ifndef INC_SCREEN_DATA_DICTIONARY_H_
#define INC_SCREEN_DATA_DICTIONARY_H_

#include <stdint.h>
#include "MBMS.h"

#ifdef __cplusplus
extern "C" {
#endif

// ENUMS

typedef enum {
  PRECHARGE_OPEN,
  PRECHARGE_CLOSED,
  PRECHARGE_CLOSING,
  PRECHARGE_ERROR
} PrechargeState;

typedef enum {
  CONTACTOR_OPEN,
  CONTACTOR_CLOSED,
  CONTACTOR_CLOSING,
  CONTACTOR_ERROR
} ContactorState;

typedef enum {
  STARTUP_nMPS_ENABLED = 0,
  STARTUP_nMPS_DISABLED,
  STARTUP_ESD_DISABLED,
  STARTUP_CHECKS_PASSED,
  STARTUP_COMMON_CLOSED,
  STARTUP_LV_CLOSED,
  STARTUP_EN1_ON,
  STARTUP_MOTORS_PERMS,
  STARTUP_ARRAY_PERMS,
  STARTUP_COMPLETED
} StartupState;

typedef enum {
  CARSTATE_BOOT = 0,
  CARSTATE_STARTUP,
  CARSTATE_FULLY_OPERATIONAL,
  CARSTATE_CHARGING,
  CARSTATE_BPS_FAULT,
  CARSTATE_MPS_DISCONNECTED,
  CARSTATE_SOFT_TRIP
} CarState;

// COMMON STRUCT FOR BOARD STATUS
typedef struct {
  uint8_t heartbeat;
  PrechargeState prechargerState;
  ContactorState contactorState;
  float lineCurrent;
  float chargeCurrent;
} BoardStatus;

// CONTACTOR SCREEN
typedef struct {
  BoardStatus commonBoard;
  BoardStatus motorBoard;
  BoardStatus arrayBoard;
  BoardStatus lvBoard;
  BoardStatus chargeBoard;
} ContactorScreen;

// TRIP SCREEN
typedef struct {
  uint8_t highCellVoltageTrip;
  uint8_t lowCellVoltageTrip;
  uint8_t commonHighCurrentTrip;
  uint8_t motorHighCurrentTrip;
  uint8_t arrayHighCurrentTrip;
  uint8_t lvHighCurrentTrip;
  uint8_t chargeHighCurrentTrip;
  uint8_t protectionTrip;
  uint8_t orionMsgTimeoutTrip;
  uint8_t contactorDiscUnexpected;
  uint8_t contactorConnUnexpected;
  uint8_t commonHeartbeatDead;
  uint8_t motorHeartbeatDead;
  uint8_t arrayHeartbeatDead;
  uint8_t lvHeartbeatDead;
  uint8_t chargeHeartbeatDead;
  uint8_t mpsDisabledTrip;
  uint8_t esdEnabledTrip;
  uint8_t highTempTrip;
  uint8_t lowTempTrip;
} TripScreen;


// MBMS STATUS SCREEN
typedef struct {
  float auxBatteryVoltage;
  uint8_t strobeBMSLight;
  uint8_t nChargeEnable;
  uint8_t nChargeSafety;
  uint8_t nDischargeEnable;
  uint8_t orionCANRx;
  uint8_t dischargeShouldTrip;
  uint8_t chargeShouldTrip;
  StartupState startupState;
  CarState systemState;
} MBMSStatusScreen;

// BATTERY INFO SCREEN
typedef struct {
  int16_t packCurrent;
  uint16_t packVoltage;
  uint8_t packSOC;
  uint16_t packAmphours;
  uint8_t packDOD;
  uint8_t lowTemp;
  uint8_t avgTemp;
  uint8_t highTemp;
  uint16_t lowCellVoltage;
  uint16_t highCellVoltage;
  uint16_t maxCellVoltage;
} BatteryInfoScreen;

// ROOT DATA STRUCTURE
typedef struct {
  ContactorScreen contactorScreen;
  TripScreen tripScreen;
  PowerSelectionStatus powerStatus;
  MBMSStatusScreen mbmsStatus;
  BatteryInfoScreen batteryInfo;
} ScreenDataDictionary;

extern ScreenDataDictionary screenData;


//* Function Prototypes */
// TODO: Clean up redundant functions and ensure they are used correctly, this was done in a time crunch
BatteryInfoScreen convertToScreenBatteryInfo(const BatteryInfo* b);
TripScreen convertToTripScreen(const MBMSTrip* t);
MBMSStatusScreen convertToMBMSStatusScreen(const MBMSStatus* status);
ContactorScreen convertToContactorScreen(const ContactorInfo contactorInfo[5]);

#ifdef __cplusplus
}
#endif

#endif /* INC_SCREEN_DATA_DICTIONARY_H_ */
