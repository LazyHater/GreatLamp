#include "MqttHandler.h"

extern WiFiClient espClient;
PubSubClient mqttClient(espClient);

bool MqttHandler::isConnected() { return mqttClient.connected(); }

bool MqttHandler::isEnabled() { return enabled; }

bool MqttHandler::getState() { return mqttClient.state(); }

void MqttHandler::callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, "/inbox/xdlamp/toggle") == 0) {
    lamp.toggle();
    lamp.handle(true);
    return;
  }

  if (strcmp(topic, "/inbox/xdlamp/deviceInfo") == 0) {
    mqttClient.publish_P("/outbox/xdlamp/deviceInfo", DEVICEINFO_PAYLOAD, DEVICEINFO_PAYLOAD_SIZE, true);
    return;
  }

  if (strcmp(topic, "/inbox/xdlamp/set") == 0) {
    for (int i = 0; i < length; i++) {
      byte level = payload[i] - '0';
      if (level >= 0 && level <= 3 ) {
        lamp.setLevel(level);
        lamp.handle(true);
        break;
      }
    }
    return;
  }
}

void MqttHandler::updateLevel(int lv) {
  if (!mqttClient.connected()) {
    return;
  }

  char buff[64];
  snprintf(buff, 64, "{\"value\": %i}", lv);
  mqttClient.publish("/outbox/xdlamp/set", buff);
}


void MqttHandler::updateLevel2(int lv) {
  if (!mqttClient.connected()) {
    return;
  }

  char buff[64];
  snprintf(buff, 64, "{\"value\": %i}", lv);
  mqttClient.publish("/outbox/xdlamp/cnt", buff);
}

void MqttHandler::reconnect() {
  // Attempt to connect
  static unsigned long reconnectTime = 0;
  if (millis() - reconnectTime > 5000) {
    String clientId = "xD-Lamp-";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str(), "/outbox/xdlamp/lwt", 1, false, "xd")) {
      mqttClient.subscribe("/inbox/xdlamp/set");
      mqttClient.subscribe("/inbox/xdlamp/toggle");
      mqttClient.subscribe("/inbox/xdlamp/deviceInfo");
      mqttClient.publish_P("/outbox/xdlamp/deviceInfo", DEVICEINFO_PAYLOAD, DEVICEINFO_PAYLOAD_SIZE, true);
    } 
    reconnectTime = millis();
  }
}

void MqttHandler::handle() {
  if (!enabled) return;

  if (!mqttClient.connected()) {
    reconnect();
  } else {
  }
    mqttClient.loop();
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

  mqttClient.setServer(mqtt_host, 1883);
  mqttClient.setCallback(MqttHandler::callback);
}
