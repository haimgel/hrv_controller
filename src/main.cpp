
#include "hrv_control.hpp"

// For more info, see: https://homie-esp8266.readme.io/docs/custom-settings
HomieSetting<unsigned long> debugLevelSetting("debug_level", "Debug level for console output");

// ***************************************************************************
// Device processing loop
void loopHandler() {
 static unsigned long lastHrvUpdate = 0;
  wdt_reset();
  if (PIN_SENSOR != -1) {
    //sensorNodesUpdate();
  }
  if (millis() - lastHrvUpdate > 1000*60) {
    hrvUpdate();
    lastHrvUpdate = millis();
  }
}

// ***************************************************************************
// Device has booted in normal mode (configuration present)
void normalModeSetup() {
  hrvNodePublishAll();
  if (PIN_SENSOR != -1) {
    //sensorNodesPublishGeneral();
  }
}

// ***************************************************************************
void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Serial.println(FW_NAME " " FW_VERSION);

  Homie_setFirmware(FW_NAME, FW_VERSION);
  // Reset is the same as on/off button, so we handle the login ourselves, above.
  Homie.setLedPin(PIN_LED, LOW).disableResetTrigger();
  debugLevelSetting.setDefaultValue(DEFAULT_DEBUG_LEVEL).setValidator([] (unsigned long candidate) {
      return (candidate >= 0) && (candidate <= 10);
  });
  hrvNodeSetup();
  if (PIN_SENSOR != -1) {
    //sensorNodesSetup();
  }
  Homie.setSetupFunction(normalModeSetup).setLoopFunction(loopHandler);

  // "N"-only mode is significantly less power-hungry on ESP8266, use it!
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);

  // Enable watchdog, 8 sec. timer
  wdt_enable(WDTO_8S);

  // Done all housekeeping
  Homie.setup();
}

// ***************************************************************************
void loop() {
  Homie.loop();
}
