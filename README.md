# MQTT HRV controller

This software provides MQTT interface for automatic control of a HRV (Heat Recovery Ventilator).
It was built to control specifically Lifebreath RNC5-TPD, but it could be modified
to control other units if they support dry contact controls (e.g. relay-controllable).

Hardware setup, rationale, and integration with other services is discussed in a series of 
[blog posts](https://haim.dev/posts/2020-04-25-hrv-with-esp8266/)

"Standard" HRV operation and the hardware interface is discussed in `HRV-interface.md`.

## Hardware needed

  1. ESP8266 WiFi SOC board. Any NodeMCU-compatible board would do, as only 4
     GPIO lines are needed.
  2. Relay board to control the HRV (Sunfounder 4-channel relay board, or similar)
  3. 5V power supply to power the ESP8266 and the relays.

## Customizing the software

See `data/homie/config.json.sample` for all customizable settings. Copy to `config.json` and enter
the correct Wi-Fi connection parameters, MQTT server address and username/password if
needed. Also, pin assignments need to be specified in `src/hrv_control.hpp`.
No other files need to be modified.

## Uploading software

  1. Need to download and install [PlatformIO](http://platformio.org/).
  2. Either use the GUI, or do this from the command line:
     `platformio run --target upload`
     `platformio run --target uploadfs`

## Frameworks

This software is built on top of [Homie](http://homieiot.github.io/homie-esp8266/), and Homie is built on top of
[Arduino for ESP8266](https://github.com/esp8266/Arduino).

## MQTT commands and status updates

To set the mode of operation: publish the desired mode to `hrv-control/hrv/schedule/set`. Valid options are:
  * `off` - HRV is turned off.
  * `10_50` - HRV is turned on for 10 minutes, then turned off for 50 minutes (low fan speed).
  * `20_40` - HRV is turned on for 20 minutes, then turned off for 40 minutes (low fan speed).
  * `low` - Constant-on, low fan speed operation.
  * `high` - Constant-on, high fan speed operation.

Current operation mode and fan speed are published to these topics:
  * `hrv-control/hrv/schedule` - currently active schedule (same values as above).
  * `hrv-control/hrv/status` - short human-readable status: `OFF`, `10 / 50`, `20 / 40`, `LOW`, `HIGH`.
  * `hrv-control/hrv/text` - long human-readable status. Example: `HRV is on 10/50 schedule, fan is off now`.
  * `hrv-control/hrv/fan` - current fan mode: `off`, `low`, `high`.
