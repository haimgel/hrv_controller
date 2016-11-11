
#include <Homie.h>
#include <Ticker.h>
#include <WString.h>
#include "utils.hpp"
#include "node_sensor.hpp"
#include "node_hrv.hpp"
#include "nodes_environment.hpp"
#include "relays.hpp"

// Global declarations
#define FW_NAME "hrv_control"
#define FW_VERSION "1.1.0"

// Pins definitions as marked on doit.am ESP-12F (http://www.banggood.com/Mini-NodeMCU-ESP8266-WIFI-Development-Board-Based-On-ESP-12F-p-1054209.html)
// As far as I know, it's the same as "D1 Mini" definition!

// Pins we use
static const uint8_t PIN_LED = LED_BUILTIN;
static const uint8_t PIN_BUTTON = D7;
static const uint8_t PIN_SENSOR = D6;
static const uint8_t PIN_RELAY_HUMIDISTAT = D1;
static const uint8_t PIN_RELAY_LOW = D2;
static const uint8_t PIN_RELAY_HIGH = D3;

/* Just some sensible defaults, which get overriden by configuration settings,
if set (they probably should) */

const int DEFAULT_DEBUG_LEVEL = 5;
const int BUTTON_RESET_PRESS_TIME = 5000; // Keep button pressed for 5 secs to reset
