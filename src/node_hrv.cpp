
#include "hrv_control.hpp"

HomieNode hrvNode("hrv", "hvac");
Relays relays;

#define PROPERTY_SCHEDULE "schedule"
#define PROPERTY_SCHEDULE_SET "schedule/set"
#define PROPERTY_FAN "fan"
#define PROPERTY_STATUS "status"
#define PROPERTY_TEXT "text"

enum hrvScheduleEnum { schOff, schLow10_50, schLow20_40, schLowFullTime, schHighFullTime };
const char *hrvScheduleNames[] = { "off", "10_50", "20_40", "low", "high" };

hrvScheduleEnum hrvSchedule = schLow20_40;

// ***************************************************************************
// Handle occupancy change
bool hrvScheduleHandler(HomieRange range, String value) {
  if (value.equalsIgnoreCase(hrvScheduleNames[schOff]))
    hrvSchedule = schOff;
  else if (value.equalsIgnoreCase(hrvScheduleNames[schLow10_50]))
    hrvSchedule = schLow10_50;
  else if (value.equalsIgnoreCase(hrvScheduleNames[schLow20_40]))
    hrvSchedule = schLow20_40;
  else if (value.equalsIgnoreCase(hrvScheduleNames[schLowFullTime]))
    hrvSchedule = schLowFullTime;
  else if (value.equalsIgnoreCase(hrvScheduleNames[schHighFullTime]))
    hrvSchedule = schHighFullTime;
  else {
    if (debugLevelAbove(1)) Serial << "HRV: error: unknown schedule name: " << value.c_str() << endl;
    return false;
  }
  if (debugLevelAbove(3)) Serial << "HRV: schedule set to " << hrvScheduleNames[hrvSchedule] << endl;
  hrvUpdate();
  hrvNodePublishAll();
  return true;
}

// ***************************************************************************
void hrvNodeSetup() {
  hrvNode.advertise(PROPERTY_SCHEDULE).settable(hrvScheduleHandler);
  hrvNode.advertise(PROPERTY_FAN);
  hrvNode.advertise(PROPERTY_STATUS);
  hrvNode.advertise(PROPERTY_TEXT);
  hrvNodePublishAll();
}

// ***************************************************************************
// Publish all available information
void hrvNodePublishAll() {
  hrvNode.setProperty(PROPERTY_SCHEDULE).send(hrvScheduleNames[hrvSchedule]);
  hrvNode.setProperty(PROPERTY_FAN).send(relays.getFanStr().c_str());
  String text = "";
  String status = "";
  switch (hrvSchedule) {
    case schOff:
      text += "HRV is off";
      status = "OFF";
      break;
    case schLow10_50:
      text += "HRV is on 10/50 schedule, fan is ";
      text += relays.getFan() == fanOff ? "off" : "on";
      text += " now";
      status = "10 / 50";
      break;
    case schLow20_40:
      text += "HRV is on 20/40 schedule, fan is ";
      text += relays.getFan() == fanOff ? "off" : "on";
      text += " now";
      status = "20 / 40";
      break;
    case schLowFullTime:
      text += "HRV is in always-on low speed mode";
      status = "CONSTANT LOW";
      break;
    case schHighFullTime:
      text += "HRV is in always-on high speed mode";
      status = "CONSTANT HIGH";
      break;
  }
  hrvNode.setProperty(PROPERTY_TEXT).send(text);
  hrvNode.setProperty(PROPERTY_STATUS).send(status);
}

// ***************************************************************************
// Returns "minutes in current hour", where the 0:0 time is started on device boot.
// This is consistent, but of course does not correspond to real wall-clock.
int minutes() {
  return (millis() % (60*60*1000)) / 60000;
}

// ***************************************************************************
// Update relays based on current schedule
// See HRV-smartcontrol.md for discussion
void hrvUpdate() {
  int mins = minutes();
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
}

// ***************************************************************************
// Manually iterate through modes (for hardware button control)
void hrvModeSwitch() {
  switch (hrvSchedule) {
    case schOff:
      hrvSchedule = schLow20_40;
      debugPrint(3, "Setting schedule to 20/40 via button press");
      break;
    case schLow10_50:
    case schLow20_40:
      hrvSchedule = schLowFullTime;
      debugPrint(3, "Setting schedule to LOW via button press");
      break;
    case schLowFullTime:
      hrvSchedule = schHighFullTime;
      debugPrint(3, "Setting schedule to HIGH via button press");
      break;
    case schHighFullTime:
      hrvSchedule = schOff;
      debugPrint(3, "Setting schedule to OFF via button press");
      break;
  }
  hrvUpdate();
  if (Homie.isConnected()) {
    hrvNode.setProperty(PROPERTY_SCHEDULE_SET).send(hrvScheduleNames[hrvSchedule]);
    hrvNodePublishAll();
  }
}

// ***************************************************************************
// Returns "current led tact", it's 50 ms long, total of 60 of these,
// so it repeats every 3 seconds.
int ledTact() {
  return (millis() % (3000)) / 50;
}

// ***************************************************************************
// Get LED indicating current HRV schedule
bool hrvGetLed() {
  int tact = ledTact();
  switch (hrvSchedule) {
    case schOff:
      // Blink for 50ms every 5 seconds, just indication that we're alive.
      return tact == 0;
      break;
    case schLow10_50:
      return tact <= 10;
      break;
    case schLow20_40:
      return tact <= 20;
      break;
    case schLowFullTime:
      // Mostly on, short off period to indicate we're alive.
      return tact < 58;
      break;
    case schHighFullTime:
      // High-speed blink
      return tact % 3 == 0;
      break;
  }
}
