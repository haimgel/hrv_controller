
#include "hrv_control.hpp"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define PROPERTY_MAX_VALUE "max"
#define PROPERTY_MIN_VALUE "min"
#define PROPERTY_RESOLUTION "resolution"
#define PROPERTY_UNITS "units"
#define PROPERTY_VALUE "value"

// Frequent sensor updates are counter-productive. Sensor readings become
// less precise, sensor heats up, etc. Also, the reading has a delay of 250ms!!!
// Bad things could happen because of this: deassociations, other Wi-Fi problems, etc.
// So we read the sensor only every 3 minutes.
#define SECS_BETWEEN_SENSOR_UPDATES 180

DHT_Unified dht(PIN_SENSOR, DHT22);

HomieNode tempSensorNode("temperature", "sensor");
HomieNode humiditySensorNode("humidity", "sensor");

// ***************************************************************************
void sensorNodesSetup() {
  dht.begin();
  tempSensorNode.advertise(PROPERTY_MAX_VALUE);
  tempSensorNode.advertise(PROPERTY_MIN_VALUE);
  tempSensorNode.advertise(PROPERTY_RESOLUTION);
  tempSensorNode.advertise(PROPERTY_UNITS);
  tempSensorNode.advertise(PROPERTY_VALUE);
  humiditySensorNode.advertise(PROPERTY_MAX_VALUE);
  humiditySensorNode.advertise(PROPERTY_MIN_VALUE);
  humiditySensorNode.advertise(PROPERTY_RESOLUTION);
  humiditySensorNode.advertise(PROPERTY_UNITS);
  humiditySensorNode.advertise(PROPERTY_VALUE);
}

// ***************************************************************************
// Publish general information about the sensor (this is done once)
void sensorNodesPublishGeneral() {
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  tempSensorNode.setProperty(PROPERTY_MAX_VALUE).send(String(sensor.max_value));
  tempSensorNode.setProperty(PROPERTY_MIN_VALUE).send(String(sensor.min_value));
  tempSensorNode.setProperty(PROPERTY_RESOLUTION).send(String(sensor.resolution));
  tempSensorNode.setProperty(PROPERTY_UNITS).send("c");
  if (debugLevelAbove(3)) {
    Serial << "Temperature sensor: min=" << sensor.min_value << ", max=" << sensor.max_value << ", resolution=" << sensor.resolution << endl;
  }
  dht.humidity().getSensor(&sensor);
  humiditySensorNode.setProperty(PROPERTY_MAX_VALUE).send(String(sensor.max_value));
  humiditySensorNode.setProperty(PROPERTY_MIN_VALUE).send(String(sensor.min_value));
  humiditySensorNode.setProperty(PROPERTY_RESOLUTION).send(String(sensor.resolution));
  humiditySensorNode.setProperty(PROPERTY_UNITS).send("%");
  if (debugLevelAbove(3)) {
    Serial << "Humidity sensor: min=" << sensor.min_value << ", max=" << sensor.max_value << ", resolution=" << sensor.resolution << endl;
  }
}

// ***************************************************************************
// Read data from the sensor and report it
void sensorNodesUpdate() {
  static unsigned long sensorLastUpdate = 0;
  if (millis() - sensorLastUpdate > SECS_BETWEEN_SENSOR_UPDATES * 1000)  {
    sensorLastUpdate = millis();
    sensors_event_t event;
    // First, temperature
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      debugPrint(1, "Error reading temperature");
    } else {
      if (debugLevelAbove(3)) Serial << "Temperature: " << event.temperature << " deg. C" << endl;
      tempSensorNode.setProperty(PROPERTY_VALUE).setRetained(false).send(String(event.temperature));
    }
    // Second, humidity
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      debugPrint(1, "Error reading humidity");
    } else {
      if (debugLevelAbove(3)) Serial << "Humidity: " << event.relative_humidity << " %" << endl;
      humiditySensorNode.setProperty(PROPERTY_VALUE).setRetained(false).send(String(event.relative_humidity));
    }
  }
}
