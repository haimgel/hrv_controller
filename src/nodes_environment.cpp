
#include "hrv_control.hpp"

HomieNode externalEnvNode("outdoor", "hvac");
HomieNode internalEnvNode("indoor", "hvac");

#define PROPERTY_TEMPERATURE "temperature"
#define PROPERTY_HUMIDITY "humidity"

// Interior / exterior temperature and humidity
float env_ext_temp = NAN;
float env_ext_humi = NAN;
float env_int_temp = NAN;
float env_int_humi = NAN;

// When the data was received last
unsigned long last_ext_temp_update = 0;
unsigned long last_ext_humi_update = 0;
unsigned long last_int_temp_update = 0;
unsigned long last_int_humi_update = 0;

// ***************************************************************************
bool extEnvSetTemp(HomieRange range, String value) {
  env_ext_temp = value.toFloat();
  last_ext_temp_update = millis();
}

// ***************************************************************************
bool extEnvSetHumi(HomieRange range, String value) {
  env_ext_humi = value.toFloat();
  last_ext_humi_update = millis();
}

// ***************************************************************************
bool intEnvSetTemp(HomieRange range, String value) {
  env_int_temp = value.toFloat();
  last_int_temp_update = millis();
}

// ***************************************************************************
bool intEnvSetHumi(HomieRange range, String value) {
  env_int_humi = value.toFloat();
  last_int_humi_update = millis();
}

// ***************************************************************************
// Setup environment data nodes
void envNodesSetup() {
  externalEnvNode.advertise(PROPERTY_TEMPERATURE).settable(extEnvSetTemp);
  externalEnvNode.advertise(PROPERTY_HUMIDITY).settable(extEnvSetHumi);
  internalEnvNode.advertise(PROPERTY_TEMPERATURE).settable(intEnvSetTemp);
  internalEnvNode.advertise(PROPERTY_HUMIDITY).settable(intEnvSetHumi);
  hrvNodePublishAll();
}

// ***************************************************************************
// Publish all available information
void envNodesPublishAll() {
  if (!isnan(env_ext_temp)) {
    externalEnvNode.setProperty(PROPERTY_TEMPERATURE).send(String(env_ext_temp));
  } else {
    externalEnvNode.setProperty(PROPERTY_TEMPERATURE).send("");
  }
  if (!isnan(env_ext_humi)) {
    externalEnvNode.setProperty(PROPERTY_HUMIDITY).send(String(env_ext_humi));
  } else {
    externalEnvNode.setProperty(PROPERTY_HUMIDITY).send("");
  }
  if (!isnan(env_int_temp)) {
    internalEnvNode.setProperty(PROPERTY_TEMPERATURE).send(String(env_int_temp));
  } else {
    internalEnvNode.setProperty(PROPERTY_TEMPERATURE).send("");
  }
  if (!isnan(env_int_humi)) {
    internalEnvNode.setProperty(PROPERTY_HUMIDITY).send(String(env_int_humi));
  } else {
    internalEnvNode.setProperty(PROPERTY_HUMIDITY).send("");
  }
}

// ***************************************************************************
// Expire all data that is more than 2 hours old
void envNodesExpire() {
  bool expired = false;
  unsigned long now = millis();
  if (now = last_ext_temp_update > DATA_LIFETIME_MS) {
     env_ext_temp = NAN;
     expired = true;
  }
  if (now = last_ext_humi_update > DATA_LIFETIME_MS) {
     env_ext_humi = NAN;
     expired = true;
  }
  if (now = last_int_temp_update > DATA_LIFETIME_MS) {
     env_int_temp = NAN;
     expired = true;
  }
  if (now = last_int_humi_update > DATA_LIFETIME_MS) {
     env_int_humi = NAN;
     expired = true;
  }
  if (expired) envNodesPublishAll();
}

// ***************************************************************************
// Returns true of all environmental data is current
bool isEnvironmentalDataCurrent() {
  return
    !isnan(env_ext_temp) &&
    !isnan(env_ext_humi) &&
    !isnan(env_int_temp) &&
    !isnan(env_int_humi);
}

// ***************************************************************************
// Calculates absolute humidity based on relative humidity and temperature
// Based on https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/
//
// Absolute Humidity (grams/m3) = (6.112 x e^[(17.67 x T)/(T+243.5)] x rh x 2.1674) / (273.15+T)
// This formula is accurate to within 0.1% over the temperature range –30°C to +35°C.
//
float absoluteHumidity(float t, float h) {
 float tmp;
 tmp = pow(2.718281828, (17.67 * t) / (t + 243.5));
 return (6.112 * tmp * h * 2.1674) / (273.15 + t);
}
