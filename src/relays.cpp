
#include "hrv_control.hpp"

// ***************************************************************************
Relays::Relays() {
  mDehumidistat = false;
  mFan = fanOff;
  pinMode(PIN_RELAY_HUMIDISTAT, OUTPUT);
  pinMode(PIN_RELAY_LOW, OUTPUT);
  pinMode(PIN_RELAY_HIGH, OUTPUT);
  update();
}

// ***************************************************************************
bool Relays::getDehumidistat() {
  return mDehumidistat;
}

// ***************************************************************************
String Relays::getDehumidistatStr() {
  return mDehumidistat ? "on" : "off";
}

// ***************************************************************************
void Relays::setDehumidistat(bool value) {
  mDehumidistat = value;
  update();
}

// ***************************************************************************
fanEnum Relays::getFan() {
  return mFan;
}

// ***************************************************************************
String Relays::getFanStr() {
  const char *fanNames[] = { "off", "low", "high" };
  return fanNames[mFan];
}

// ***************************************************************************
void Relays::setFan(fanEnum value) {
  mFan = value;
  update();
}

// ***************************************************************************
// Relay inputs are inverted
void Relays::writeRelay(int pin, bool value) {
    digitalWrite(pin, not value);
}

// ***************************************************************************
// Update relays based on internal state
void Relays::update() {
  writeRelay(PIN_RELAY_HUMIDISTAT, mDehumidistat);
  if (debugLevelAbove(3)) Serial << "Dehumidistat set to " << mDehumidistat << endl;
  switch (mFan) {
    case fanOff:
      writeRelay(PIN_RELAY_LOW, false);
      writeRelay(PIN_RELAY_HIGH, false);
      debugPrint(3, "Setting fan off");
      break;
    case fanLow:
      writeRelay(PIN_RELAY_HIGH, false);
      writeRelay(PIN_RELAY_LOW, true);
      debugPrint(3, "Setting fan to low");
      break;
    case fanHigh:
      writeRelay(PIN_RELAY_LOW, false);
      writeRelay(PIN_RELAY_HIGH, true);
      debugPrint(3, "Setting fan to high");
      break;
  }
}
