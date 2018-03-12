// EepromHandler.h

#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "EepromHandler.h"
#include "Lamp.h"

extern Lamp lamp;

class MqttHandler {
  char mqtt_host[65];
  PubSubClient client;
  bool enabled = true;

public:
  MqttHandler(WiFiClient& espClient) : client(espClient) {
  }

  static void callback(char* topic, byte* payload, unsigned int length) {
    //Serial.print("Message arrived [");
    //Serial.print(topic);
    //Serial.print("] ");
    for (int i = 0; i < length; i++) {
      //Serial.print((char)payload[i]);
    }
    //Serial.println();
     if (length < 1) return;

    // Switch on the LED if an 1 was received as first character
    switch ((char)payload[0]) {
      case '0':
        //lamp.setLevel(0);
        Serial.println("xd");
      break;
          case '1':
        //lamp.setLevel(1);
        Serial.println("xD");
      break;
          case '2':
        //lamp.setLevel(2);
        Serial.println("XD");
      break;
          case '3':
        //lamp.setLevel(3);
        Serial.println("XDDD");
      break;
    }
  }


void reconnect() {
    Serial.print("Attempting MQTT connection to: ");
    Serial.println(mqtt_host);
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("home/xdlamp/set");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }

}

  void handle() {
    if (!enabled) return;

    if (!client.connected()) {
      reconnect();
    } else {
      client.loop();
    }
  }

  void init() {
    eepromHandler.init();
    eepromHandler.readMqttHost();
    eepromHandler.end();

    if (eepromHandler.getMqttHost().length() == 0) {
      enabled = false;
      return;
    }

    strcpy(mqtt_host, eepromHandler.getMqttHost().c_str());

    client.setServer(mqtt_host, 1883);
    client.setCallback(MqttHandler::callback);
     //Serial.println("mqtt init done");
  }

  bool isConnected() {
    return client.connected();
  }

  bool isEnabled() {
    return enabled;
  }
};

#endif // MQTT_HANDLER_H
