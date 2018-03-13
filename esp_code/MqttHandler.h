// EepromHandler.h

#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "EepromHandler.h"
#include "Lamp.h"

extern Lamp lamp;

class MqttHandler {

public:
  MqttHandler(WiFiClient& espClient) : client(espClient) {}

  void init();

  void reconnect();

  void handle();

  inline bool isConnected() { return client.connected(); }

  inline bool isEnabled() { return enabled; }

  static void callback(char* topic, byte* payload, unsigned int length);

private:
  char mqtt_host[65];
  PubSubClient client;
  bool enabled = true;

};

#endif // MQTT_HANDLER_H
