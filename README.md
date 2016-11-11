# "Smart" HRV controller

This software allows for automatic "smart" control of a HRV (Heat Recovery Ventilator).
It was built to control specifically Lifebreath RNC5-TPD, but it could be modified
to control other units if they support dry contact controls (e.g. relay-controllable).

"Standard" HRV operation and hardware interface is discussed in `HRV-interface.md`.
HRV smart control algorythms and rationale behind this software is discussed
in `HRV-smartcontrol.md`.

## Hardware needed

  1. ESP8266 WiFi SOC board. Any NodeMCU-compatible board would do, as only 4
     GPIO lines are needed.
  2. Relay board to control the HRV (Sunfounder 4-channel relay board, or similar)
  3. 5V power supply to power the ESP8266 and the relays.

## Customizing software

See `data/homie/config.json` for all cusomizable settings. You need to provide
your Wi-Fi connection paramters, MQTT server address and username/password if
needed, as well as MQTT topics that receive temperature/humidity updates.
Also, pin assignments need to be specified in `src/hrv_control.hpp`.
No other files need to be modified.

## Uploading software

  1. Need to download and install [PlatformIO](http://platformio.org/).
  2. Either use the GUI, or do this from the command line:
     `platformio run --target upload`
     `platformio run --target uploadfs`

## Framework

This software is built on top of Homie:
  https://homie-esp8266.readme.io/docs
  https://github.com/marvinroger/homie-esp8266

Homie is built on top of Arduino for ESP8266:
  https://github.com/esp8266/Arduino

## MQTT commands and status updates

This software depends on external sensors to know the indoors and outdoors
temperature and humidity. So the module subscribes to 4 topics to receive
timely updates to that information. Obviously external automation is needed
to supply that information (like, for example, Home Assistant Automation
setting these properties based on thermostat or Z-Wave sensors installed).

 * `hrv_control/outdoor/temperature/set`
 * `hrv_control/outdoor/humidity/set`
 * `hrv_control/indoor/temperature/set`
 * `hrv_control/indoor/humidity/set`

If updates are not received for two hours, HRV controller will switch into
"fail-safe" mode of low ventilation (to prevent excessively dry air,
heating/cooling energy loss).

`hrv_control` is controllable via four topics:

 * `hrv_control/hrv/occupancy/set` - one of the following:
    `vacant`, `low`, `medium`, `high`.
 * `hrv_control/hrv/mode/set` - one of the following:
   `off`, `min`, `auto`, `max`
 * `hrv_control/hrv/humidity/set` - Desirable interior humidity.
    Can only be set to values between 15% and 85%, or `auto` (default). 
    `Auto` means:
     * 50% if exterior temperature is above 15 deg.C
     * 40% if exterior temperature is below 0 deg.C
     * 30% if exterior temperature is below -15 deg.C

 * `hrv_control/hrv/temperature/set` - Desirable interior temperature.
   Defaults to "unknown" -- if not set, will not be taken into account.

See `HRV-smartcontrol.md` for discussion of these parameters and what they do.

### Status

`hrv_control` publishes the following data:

* `hrv_control/hrv/occupancy` - current occupancy mode.
* `hrv_control/hrv/mode` - current operation mode.
* `hrv_control/hrv/fan` - current speed of the fan: `off`, `low`, `high`
* `hrv_control/hrv/dehumidifier` - current dehumidifier mode: `off` or `on`.
* `hrv_control/hrv/status` - human-readable status string (how the unit operates).
   For example:
     "20min on / 40min off low speed ventilation
      15min on / 30min off dehumidifier mode"
