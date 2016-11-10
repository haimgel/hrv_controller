
#include "hrv_control.hpp"

extern HomieSetting<unsigned long> debugLevelSetting;

bool str2bool(String value) {
  // Home Assistant passes uppercase ON and OFF to change state
  return value.equalsIgnoreCase("on") || value.equalsIgnoreCase("true");
}

String bool2str(bool value) {
  // Uppercase ON/OFF conforms to Home Assistant defaults
  const String strOn = "ON";
  const String strOff = "OFF";
  return value ? strOn : strOff;
}

bool debugLevelAbove(int lvl) {
  // This function should probably be called debugLevelAboveOrEqual, but it's too long!
  return (lvl <= debugLevelSetting.get());
}

void debugPrint(int lvl, String str) {
  if (debugLevelAbove(lvl)) {
      Serial.println(str);
  }
}
