
#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "Debug.h"

// int enable_debug_messages = DEBUG_EEPROM; // Needed by debug.h

const int LAMP_WRITE_PIN = 2;
const int LAMP_READ_PIN = 3;
const int LAMP_PULSE_IN_TIME = 2000; // microseconds
const int LAMP_LEVEL_1_PWM_VALUE = 40;
const int LAMP_LEVEL_2_PWM_VALUE = 170;
const int LAMP_HANDLE_LEVEL_CHECK_TIME = 5000; // every 5000ms state of lamp would be checked

const int BUTTON_PIN = 1;
const int HTTP_SERVER_PORT = 80;

#define HOSTNAME "xD-Lamp2"
#define SOFT_AP_SSID "xD-Lamp2"
#define MDNS_NAME "xD-Lamp2"

#define BOOT_DELAY 2000
#endif /* PARAMETERS_H */