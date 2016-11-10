
#include "hrv_control.hpp"

// For more info, see: https://homie-esp8266.readme.io/docs/custom-settings
HomieSetting<unsigned long> debugLevelSetting("debug_level", "Debug level for console output");

Bounce button = Bounce();

// ***************************************************************************
// Process pressing/depressing hardware button
void processButtonPress() {
  static bool lastButtonValue = 0;
  static unsigned long buttonPressTime = 0;

  if (button.update()) {
    // Because the button is shorted to ground, we invert the reading
    bool buttonValue = !button.read();

    if (buttonValue != lastButtonValue) {
      if (debugLevelAbove(3)) Serial << "ButtonValue=" << buttonValue << endl;
      if (buttonValue) {
        // Button has been pressed, record the time!
        buttonPressTime = millis();
      } else {
        // Act if the button was released after it was pressed
        debugPrint(3, "Button has been released after it was pressed");
        // DO something when the button is pressed!
        // lightNodeSwitch();
      }
      lastButtonValue = buttonValue;
    }
  } else {
    // No change in button state, see if it IS pressed, and WAS pressed for 5 seconds:
    if (lastButtonValue && (millis() - buttonPressTime > BUTTON_RESET_PRESS_TIME)) {
      debugPrint(1, "Button has been pressed for 5 seconds, resetting!");
      Homie.reset();
    }
  }
}

// ***************************************************************************
// Device processing loop
void loopHandler() {
  processButtonPress();
  if (PIN_SENSOR != -1) {
    sensorNodesUpdate();
  }
}

// ***************************************************************************
// Device has booted in normal mode (configuration present)
void normalModeSetup() {
  hrvNodePublishAll();
  if (PIN_SENSOR != -1) {
    sensorNodesPublishGeneral();
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
    sensorNodesSetup();
  }
  Homie.setSetupFunction(normalModeSetup).setLoopFunction(loopHandler);
  // Setup the on/off button
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  button.attach(PIN_BUTTON);
  button.interval(25);
  if (debugLevelAbove(3)) Serial << "Listening for button presses on input #" << PIN_BUTTON << endl;

  // "N"-only mode is significantly less power-hungry on ESP8266, use it!
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  // Done all housekeeping
  Homie.setup();
}

// ***************************************************************************
void loop() {
  Homie.loop();
}
