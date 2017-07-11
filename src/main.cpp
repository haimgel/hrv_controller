
#include "hrv_control.hpp"

// For more info, see: https://homie-esp8266.readme.io/docs/custom-settings
HomieSetting<long> debugLevelSetting("debug_level", "Debug level for console output");

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
        hrvModeSwitch();
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
// Device has booted in normal mode (configuration present)
void normalModeSetup() {
  hrvNodeSetup();
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
  Homie.setSetupFunction(normalModeSetup);
  // Setup the on/off button
  if (PIN_BUTTON != -1) {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    button.attach(PIN_BUTTON);
    button.interval(25);
    if (debugLevelAbove(2)) Serial << "Listening for button presses on input #" << PIN_BUTTON << endl;
  } else {
    if (debugLevelAbove(2)) Serial << "NOT listening for button presses" << endl;
  }
  if (PIN_STATUS_LED != -1) {
    pinMode(PIN_STATUS_LED, OUTPUT);
    if (debugLevelAbove(2)) Serial << "Status LED on pin #" << PIN_STATUS_LED << endl;
  } else {
    if (debugLevelAbove(2)) Serial << "Status LED disabled" << endl;
  }
  // "N"-only mode is significantly less power-hungry on ESP8266, use it!
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);

  // Enable watchdog, 4 sec. timer
  ESP.wdtEnable(4000);

  // Done all housekeeping
  Homie.setup();
}

// ***************************************************************************
void loop() {
  Homie.loop();
  ESP.wdtFeed();
  if (Homie.isConfigured()) {
    static unsigned long lastHrvUpdate = 0;
    if (PIN_BUTTON != -1) {
      processButtonPress();
    }
    if (PIN_STATUS_LED != -1) {
      digitalWrite(PIN_STATUS_LED, hrvGetLed());
    }
    if (millis() - lastHrvUpdate > 1000*60) {
      hrvUpdate();
      lastHrvUpdate = millis();
    }
  }
}
