#ifndef UTILS_H
#define UTILS_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


extern ESP8266WebServer server;

void returnFailJSON(String& msg) {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "application/json", msg);
}

void returnJSON(String& msg) {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", msg);
}

bool isValidNumber(String& s) {
  for (size_t i = 0; i < s.length(); i++) {
    if (!isDigit(s.charAt(i)))
    return false;
  }
  return true;
}

String addJsonKeyValue(char* key, String value) {
  String res = "\"" + String(key) + "\":\"" + value + "\"";
  return res;
}

String addJsonKeyValue(char* key, int value) {
  String res = "\"" + String(key) + "\":" + String(value);
  return res;
}


const char * const OP_MODE_NAMES[]
{
    "NULL_MODE",
    "STATION_MODE",
    "SOFTAP_MODE",
    "STATIONAP_MODE"
};

const char * const AUTH_MODE_NAMES[]
{
    "AUTH_OPEN",
    "AUTH_WEP",
    "AUTH_WPA_PSK",
    "AUTH_WPA2_PSK",
    "AUTH_WPA_WPA2_PSK",
    "AUTH_MAX"
};

const char * const PHY_MODE_NAMES[]
{
    "",
    "PHY_MODE_11B",
    "PHY_MODE_11G",
    "PHY_MODE_11N"
};

const char * const RST_REASONS[] =
{
    "REASON_DEFAULT_RST",
    "REASON_WDT_RST",
    "REASON_EXCEPTION_RST",
    "REASON_SOFT_WDT_RST",
    "REASON_SOFT_RESTART",
    "REASON_DEEP_SLEEP_AWAKE",
    "REASON_EXT_SYS_RST"
};

const char * const WL_STATUSES[] =
{
    "WL_IDLE_STATUS",
    "WL_NO_SSID_AVAIL",
    "WL_SCAN_COMPLETED",
    "WL_CONNECTED",
    "WL_CONNECT_FAILED",
    "WL_CONNECTION_LOST",
    "WL_DISCONNECTED"
};
#endif // UTILS_H
