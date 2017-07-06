
#include "hrv_control.hpp"
#include <PID_v1.h>

HomieNode hrvNode("hrv", "hvac");
Relays relays;

// PID control parameters
const double Kp = 1;
const double Ki = 0.05;
const double Kd = 0.25;

double pidIn, pidOut, pidSetPoint;
PID pid(&pidIn, &pidOut, &pidSetPoint, Kp, Ki, Kd, DIRECT);

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

enum hrvScheduleEnum { schOff, schLow10_50, schLow20_40, schLowFullTime, schHighFullTime };

hrvOccupancyEnum hrvOccupancy = occMedium;
hrvModeEnum hrvMode = modeAuto;
hrvScheduleEnum hrvSchedule = schLow10_50;
bool hrvDehumidistat = false;
float hrvTemperature = NAN; // Unknown
float hrvHumidity = -1; // Auto

Environment envExterior((char *)"exterior");
Environment envInterior((char *)"interior");

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
    hrvTemperature = NAN;
  } else {
    float tmp = value.toFloat();
    if ((tmp >= 5) && (tmp <= 35)) {
      hrvTemperature = tmp;
      if (debugLevelAbove(3)) Serial << "HRV: temperature set to " << hrvTemperature << endl;
    } else {
      hrvTemperature = NAN;
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
      if (debugLevelAbove(3)) Serial << "HRV: humidity set to " << hrvHumidity << endl;
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
  envInterior.setup();
  envExterior.setup();
  pid.SetOutputLimits(0, 60); // Humidistat active 0 to 60 minutes per hour
  pid.SetSampleTime(60 * 1000); // One minute sample time
}

// ***************************************************************************
// Publish all available information
void hrvNodePublishAll() {
  hrvNode.setProperty(PROPERTY_OCCUPANCY).send(hrvOccupancyNames[hrvOccupancy]);
  hrvNode.setProperty(PROPERTY_MODE).send(hrvModeNames[hrvMode]);
  hrvNode.setProperty(PROPERTY_FAN).send(relays.getFanStr().c_str());
  hrvNode.setProperty(PROPERTY_DEHUMIDIFIER).send(relays.getDehumidistatStr().c_str());
  hrvNode.setProperty(PROPERTY_STATUS).send("Not implemented yet");
  envInterior.publishAll();
  envExterior.publishAll();
}

// ***************************************************************************
// Returns "real" level of desired indoor humidity (takes "auto" mode into account)
//
// Based on multiple Internet sources, I decided to set the humidity to the following
// levels, if in "Auto" mode:
//
//     * 50% if exterior temperature is above 15 deg.C
//     * 40% if exterior temperature is below 0 deg.C
//     * 30% if exterior temperature is below -15 deg.C
//
float hrvRealDesirableHumidityLevel() {
  if (hrvHumidity == -1) {
    float t = envExterior.temperature();
    if isnan(t) {
      // Unknown exterior temperature. Err on the side of more humidity
      return 50.0;
    } else {
      if (t > 15)
        return 50.0;
      else if (t < -15)
        return 30.0;
      else
        return 30.0 + (t + 15) * 20.0 / 30.0;
    }
    return -1;
  } else {
    return hrvHumidity;
  }
}

// ***************************************************************************
// Change schedules and humidistat mode based on environmental data
void hrvUpdateSchedule() {
  switch (hrvMode) {
    case modeOff:
      hrvSchedule = schOff;
      debugPrint(4, "HRV: update: 'mode' is off.");
      break;
    case modeMin:
      hrvSchedule = schLow10_50;
      debugPrint(4, "HRV: update: 'mode' is 'min', 10/50 schedule activated.");
      break;
    case modeMax:
      hrvSchedule = schHighFullTime;
      debugPrint(4, "HRV: update: 'mode' is 'max', full-time high speed schedule activated.");
      break;
    default:
      debugPrint(4, "HRV: update: 'mode' is 'auto'");
      float absExtHumi = envExterior.absHumidity();
      float absIntHumi = envInterior.absHumidity();
      if (debugLevelAbove(3)) Serial << "HRV: exterior temperature: " << envExterior.temperature() << endl;
      if (debugLevelAbove(3)) Serial << "HRV: interior temperature: " << envInterior.temperature() << endl;
      if (debugLevelAbove(3)) Serial << "HRV: exterior humidity: rel=" << envExterior.relHumidity() << "%, abs=" << absExtHumi << " g/m3" << endl;
      if (debugLevelAbove(3)) Serial << "HRV: interior humidity: rel=" << envInterior.relHumidity() << "%, abs=" << absIntHumi << " g/m3" << endl;
      if (debugLevelAbove(3)) Serial << "HRV: desirable humidity: " << hrvRealDesirableHumidityLevel() << "%" << endl;

      if (hrvOccupancy == occVacant) {
        hrvSchedule = schLow10_50;
        debugPrint(2, "HRV: update: house is vacant, 10/50 schedule activated.");
      } else if (envExterior.isDataAvailable() && envInterior.isDataAvailable() &&
          ((absExtHumi < absIntHumi) && (envInterior.relHumidity() < hrvRealDesirableHumidityLevel()))) {
        hrvSchedule = schLow10_50;
        debugPrint(2, "HRV: update: outdoor absolute humidity is less than indoor absolute humidity and indoor relative humidity is less than desirable, 10/50 schedule activated.");
      } else if (hrvOccupancy == occHigh) {
        hrvSchedule = schHighFullTime;
        debugPrint(2, "HRV: update: house is in high activity mode, full-time high speed schedule activated.");
      } else if (envExterior.isDataAvailable() && envInterior.isDataAvailable() && !isnan(hrvTemperature) &&
          (sgn(envInterior.temperature() - envExterior.temperature()) * sgn(envInterior.temperature() - hrvTemperature) == 1)) {
        hrvSchedule = schHighFullTime;
        debugPrint(2, "HRV: update: outside air will change the interior temperature in the desirable direction, full-time high speed schedule activated.");
      } else if (hrvOccupancy == occLow) {
        hrvSchedule = schLow20_40;
        debugPrint(2, "HRV: update: house is in low occupancy/activity mode, 20/40 schedule activated.");
      } else if (!envExterior.isDataAvailable() || !envInterior.isDataAvailable()) {
        hrvSchedule = schLow20_40;
        debugPrint(2, "HRV: update: temperature/humidity data not available, 20/40 schedule activated.");
      } else if ((envInterior.temperature() - envExterior.temperature() > 20) || (envInterior.temperature() - envExterior.temperature() < -10)) {
        hrvSchedule = schLow20_40;
        debugPrint(2, "HRV: update: temperature difference between indoors outdoors is too great, 20/40 schedule activated.");
      } else {
        hrvSchedule = schLowFullTime;
        debugPrint(2, "HRV: update: full time low spped schedule activated by default.");
      }
      // Update humidistat mode
      if (envExterior.isDataAvailable() &&
            envInterior.isDataAvailable() &&
            (absExtHumi < absIntHumi) &&
            (envExterior.temperature() < 15)) {
        hrvDehumidistat = true;
        debugPrint(2, "HRV: update: dehumidistat PID is on.");
      } else {
        hrvDehumidistat = false;
        debugPrint(2, "HRV: update: dehumidistat PID is off.");
      }
  }
}

// ***************************************************************************
// Returns "minutes in current hour", where the 0:0 time is started on device boot.
// This is consistent, but of course does not correspond to real wall-clock.
int minutes() {
  return (millis() % (60*60*1000)) / 60000;
}

// ***************************************************************************
// Update internal state and relays based on external information we have
// See HRV-smartcontrol.md for discussion
void hrvUpdate() {
  int mins = minutes();
  // First, control the high-low speed of the fan
  hrvUpdateSchedule();
  switch (hrvSchedule) {
    case schOff:
      relays.setFan(fanOff);
      break;
    case schLow10_50:
      relays.setFan((mins < 10) ? fanLow : fanOff);
      break;
    case schLow20_40:
      relays.setFan((mins < 20) ? fanLow : fanOff);
      break;
    case schLowFullTime:
      relays.setFan(fanLow);
      break;
    case schHighFullTime:
      relays.setFan(fanHigh);
      break;
  }
  // Second, control the dehumidistat
  if (hrvDehumidistat) {
    pidSetPoint = hrvRealDesirableHumidityLevel();
    pidIn = envInterior.relHumidity();
    pid.SetMode(AUTOMATIC);
    pid.Compute();
    if (debugLevelAbove(3)) Serial << "HRV: dehumidistat is set to " << pidOut << " minutes per hour duty" << endl;
    relays.setDehumidistat(mins < pidOut);
  } else {
    pid.SetMode(MANUAL);
    if (debugLevelAbove(3)) Serial << "HRV: dehumidistat is disabled" << endl;
    relays.setDehumidistat(false);
  }
}
