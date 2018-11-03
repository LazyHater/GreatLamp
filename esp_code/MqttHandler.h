// EepromHandler.h

#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EspEepromSettings.h>
#include "Lamp.h"
#include "Deviceinfo.h"

extern Lamp lamp;

class MqttHandler
{

public:
  void init();

  void reconnect();

  void handle();

  bool isConnected();

  bool isEnabled();

  bool getState();

  static void updateLevel(int lv);

  static void callback(char *topic, byte *payload, unsigned int length);

private:
  char mqtt_host[65];
  bool enabled = true;
};

#endif // MQTT_HANDLER_H
