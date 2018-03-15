#include "MqttHandler.h"

void MqttHandler::callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, "home/xdlamp/toggle") == 0) {
    lamp.toggle();
    return;
  }

  // assume topic "home/xdlamp/set"
  if (length < 1) return;

  switch ((char)payload[0]) {
    case '0':
    lamp.setLevel(0);
    break;

    case '1':
    lamp.setLevel(1);
    break;

    case '2':
    lamp.setLevel(2);
    break;

    case '3':
    lamp.setLevel(3);
    break;
  }
}

void MqttHandler::reconnect() {
  // Attempt to connect
  static long reconnectTime = 0;
  if (millis() - reconnectTime > 5000) {
    if (client.connect("xD-Lamp")) {
      client.subscribe("home/xdlamp/set");
      client.subscribe("home/xdlamp/toggle");
    }
    reconnectTime = millis();
  }
}

void MqttHandler::handle() {
  if (!enabled) return;

  if (!client.connected()) {
    reconnect();
  } else {
    client.loop();
  }
}

void MqttHandler::init() {
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
}
