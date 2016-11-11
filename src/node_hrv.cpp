
#include "hrv_control.hpp"

HomieNode hrvNode("hrv", "hvac");
Relays relays;

#define PROPERTY_OCCUPANCY "occupancy"
#define PROPERTY_MODE "mode"
#define PROPERTY_TEMPERATURE "temperature"
#define PROPERTY_HUMIDITY "humidity"
#define PROPERTY_FAN "fan"
#define PROPERTY_DEHUMIDIFIER "dehumidifier"
#define PROPERTY_STATUS "status"

enum hrvOccupancyEnum { occVacant, occLow, occMedium, occHigh };
const char *hrvOccupancyNames[] = { "vacant", "low", "medium", "high" };

enum hrvModeEnum { modeOff, modeMin, modeAuto, modeMax };
const char *hrvModeNames[] = { "off", "min", "auto", "max" };

enum hrvScheduleEnum { schLow10_50, schLow20_40, schLowFullTime, schHighFullTime };

hrvOccupancyEnum hrvOccupancy = occMedium;
hrvModeEnum hrvMode = modeAuto;
hrvScheduleEnum hrvSchedule = schLow10_50;
float hrvTemperature = -1; // Unknown
float hrvHumidity = -1; // Auto

// ***************************************************************************
// Handle occupancy change
bool hrvOccupancyHandler(HomieRange range, String value) {
  if (value.equalsIgnoreCase(hrvOccupancyNames[occVacant]))
    hrvOccupancy = occVacant;
  else if (value.equalsIgnoreCase(hrvOccupancyNames[occLow]))
    hrvOccupancy = occLow;
  else if (value.equalsIgnoreCase(hrvOccupancyNames[occMedium]))
    hrvOccupancy = occMedium;
  else if (value.equalsIgnoreCase(hrvOccupancyNames[occHigh]))
    hrvOccupancy = occHigh;
  else {
    if (debugLevelAbove(1)) Serial << "HRV: error: unknown occupancy level: " << value.c_str() << endl;
    return false;
  }
  if (debugLevelAbove(3)) Serial << "HRV: occupancy level set to " << hrvOccupancyNames[hrvOccupancy] << endl;
  hrvUpdate();
  hrvNodePublishAll();
  return true;
}

// ***************************************************************************
// Handle mode change
bool hrvModeHandler(HomieRange range, String value) {
  if (value.equalsIgnoreCase(hrvModeNames[modeOff]))
    hrvMode = modeOff;
  else if (value.equalsIgnoreCase(hrvModeNames[modeMin]))
    hrvMode = modeMin;
  else if (value.equalsIgnoreCase(hrvModeNames[modeAuto]))
    hrvMode = modeAuto;
  else if (value.equalsIgnoreCase(hrvModeNames[modeMax]))
    hrvMode = modeMax;
  else {
    if (debugLevelAbove(1)) Serial << "HRV: error: unknown mode level: " << value.c_str() << endl;
    return false;
  }
  if (debugLevelAbove(3)) Serial << "HRV: mode set to " << hrvModeNames[hrvMode] << endl;
  hrvUpdate();
  hrvNodePublishAll();
  return true;
}

// ***************************************************************************
// Handle desirable temperature set
bool hrvTemperatureHandler(HomieRange range, String value) {
  if (value.equalsIgnoreCase("unknown")) {
    hrvTemperature = -1;
  } else {
    float tmp = value.toFloat();
    if ((tmp >= 5) && (tmp <= 35)) {
      hrvTemperature = tmp;
      if (debugLevelAbove(3)) Serial << "HRV: temperature set to " << hrvTemperature << endl;
    } else {
      hrvTemperature = -1;
      if (debugLevelAbove(1)) Serial << "HRV: error: invalid desirable interior temperature: " << value.c_str() << endl;
    }
  }
}

// ***************************************************************************
// Handle desirable temperature set
bool hrvHumidityHandler(HomieRange range, String value) {
  if (value.equalsIgnoreCase("auto")) {
    hrvHumidity = -1;
  } else {
    float tmp = value.toFloat();
    if ((tmp >= 15) && (tmp <= 85)) {
      hrvHumidity = tmp;
      if (debugLevelAbove(3)) Serial << "HRV: humidity set to " << hrvTemperature << endl;
    } else {
      hrvHumidity = -1;
      if (debugLevelAbove(1)) Serial << "HRV: error: invalid desirable interior humidity: " << value.c_str() << endl;
    }
  }
}

// ***************************************************************************
void hrvNodeSetup() {
  hrvNode.advertise(PROPERTY_OCCUPANCY).settable(hrvOccupancyHandler);
  hrvNode.advertise(PROPERTY_MODE).settable(hrvModeHandler);
  hrvNode.advertise(PROPERTY_TEMPERATURE).settable(hrvTemperatureHandler);
  hrvNode.advertise(PROPERTY_HUMIDITY).settable(hrvHumidityHandler);
  hrvNode.advertise(PROPERTY_FAN);
  hrvNode.advertise(PROPERTY_DEHUMIDIFIER);
  hrvNode.advertise(PROPERTY_STATUS);
  hrvNodePublishAll();
}

// ***************************************************************************
// Publish all available information
void hrvNodePublishAll() {
  hrvNode.setProperty(PROPERTY_OCCUPANCY).send(hrvOccupancyNames[hrvOccupancy]);
  hrvNode.setProperty(PROPERTY_MODE).send(hrvModeNames[hrvMode]);
  hrvNode.setProperty(PROPERTY_FAN).send(relays.getFanStr().c_str());
  hrvNode.setProperty(PROPERTY_DEHUMIDIFIER).send(relays.getDehumidistatStr().c_str());
  hrvNode.setProperty(PROPERTY_STATUS).send("Not implemented yet");
}

// ***************************************************************************
// Update internal state and relays based on external information we have
// See HRV-smartcontrol.md for discussion
void hrvUpdate() {
  float absoluteInteriorHumidity = absoluteHumidity(env_int_temp, env_int_humi);
  float absoluteExteriorHumidity = absoluteHumidity(env_ext_temp, env_ext_humi);
  if ((hrvOccupancy == occVacant) || ())
}
