
#include "hrv_control.hpp"

#define PROPERTY_TEMPERATURE "temperature"
#define PROPERTY_HUMIDITY "humidity"

// ***************************************************************************
void Environment::setup() {
  mNode.advertise(PROPERTY_TEMPERATURE).settable([this](const HomieRange& range, const String& value) {
    float newVal = value.toFloat();
    applyLowPassFilter(mTimestampTemp, mSampleTemp, mEmaTemp, newVal);
    if (debugLevelAbove(3)) Serial << "Environment: " << mType << " temperature set to " << mSampleTemp << ", ema=" << mEmaTemp << endl;
    return true;
  });
  mNode.advertise(PROPERTY_HUMIDITY).settable([this](const HomieRange& range, const String& value) {
    float newVal = value.toFloat();
    applyLowPassFilter(mTimestampHumi, mSampleHumi, mEmaHumi, newVal);
    if (debugLevelAbove(3)) Serial << "Environment: " << mType << " humidity set to " << mSampleHumi << ", ema=" << mEmaHumi << endl;
    return true;
  });
  hrvNodePublishAll();
}

// ***************************************************************************
void Environment::publishAll() {
  float t = temperature();
  float h = relHumidity();
  if (!isnan(t)) {
    mNode.setProperty(PROPERTY_TEMPERATURE).send(String(t));
  }
  if (!isnan(h)) {
    mNode.setProperty(PROPERTY_HUMIDITY).send(String(h));
  }
}

// ***************************************************************************
// Returns true if data is available and not too stale
bool Environment::isDataAvailable() {
  return !isnan(temperature()) && !isnan(relHumidity());
}

// ***************************************************************************
// Get the current data
float Environment::temperature() {
  if (millis() - mTimestampTemp < DATA_LIFETIME_MS)
    return mEmaTemp;
  else
    return NAN;
}

// ***************************************************************************
float Environment::relHumidity() {
  if (millis() - mTimestampHumi < DATA_LIFETIME_MS)
    return mEmaHumi;
  else
    return NAN;
}

// ***************************************************************************
float Environment::absHumidity() {
  float t = temperature();
  float h = relHumidity();
  if (!isnan(t) && !isnan(h))
    return calcAbsoluteHumidity(t, h);
  else
    return NAN;
}

// ***************************************************************************
// Calculates absolute humidity based on relative humidity and temperature
// Based on https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/
//
// Absolute Humidity (grams/m3) = (6.112 x e^[(17.67 x T)/(T+243.5)] x rh x 2.1674) / (273.15+T)
// This formula is accurate to within 0.1% over the temperature range –30°C to +35°C.
//
float Environment::calcAbsoluteHumidity(float t, float h) {
 float tmp;
 tmp = pow(2.718281828, (17.67 * t) / (t + 243.5));
 return (6.112 * tmp * h * 2.1674) / (273.15 + t);
}

// ***************************************************************************
// This basically makes the readings smoother over time. Sudden changes in temperature or humudity
//  won't have as big effect after this filter. It does introduce some latency, of course
void Environment::applyLowPassFilter(unsigned long& timestamp, float& oldSample, float& ema, float newSample) {
  unsigned long now = millis();
  if (isnan(ema) || (timestamp == 0)) {
    // First value assignment, no previous data available
    ema = newSample;
    oldSample = newSample;
    timestamp = now;
  } else {
    // Delta T is number of HOURS passed since the last measurement
    float delta_t = (now - timestamp) * 1.0f / (1.0f * 60 * 60 * 1000);
    ema = exponentialMovingAverageIrregular(EMA_ALPHA, newSample, oldSample, delta_t, ema);
    oldSample = newSample;
    timestamp = now;
  }
}

// ***************************************************************************
// Exponential Moving Average (EMA) for irregular time-series
// Based on:
//   https://oroboro.com/irregular-ema/
//   http://stackoverflow.com/a/1027808/331862
float Environment::exponentialMovingAverageIrregular(float alpha, float sample, float prevSample,
  float deltaTime, float emaPrev) {
  float a = deltaTime / alpha;
  float u = exp(a * -1);
  float v = (1 - u) / a;
  float emaNext = ( u * emaPrev ) + (( v - u ) * prevSample ) + (( 1.0 - v ) * sample );
  return emaNext;
}
