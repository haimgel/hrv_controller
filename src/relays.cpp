
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
// Update relays based on internal state
void Relays::update() {
  digitalWrite(PIN_RELAY_HUMIDISTAT, mDehumidistat);
  switch (mFan) {
    case fanOff:
      digitalWrite(PIN_RELAY_LOW, false);
      digitalWrite(PIN_RELAY_HIGH, false);
      break;
    case fanLow:
      digitalWrite(PIN_RELAY_HIGH, false);
      digitalWrite(PIN_RELAY_LOW, true);
      break;
    case fanHigh:
      digitalWrite(PIN_RELAY_LOW, false);
      digitalWrite(PIN_RELAY_HIGH, true);
      break;
  }
}
