// #define MQTT_MAX_PACKET_SIZE 512

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EspEepromSettings.h>
#include <Logger.h>

#include "Parameters.h"
#include "Debug.h"
#include "Lamp.h"
#include "MqttHandler.h"
#include "WifiHandler.h"
#include "Utils.h"

extern "C"
{
#include "user_interface.h"
}
ESP8266WebServer server(HTTP_SERVER_PORT);
WiFiClient espClient;
MqttHandler mqtt;
Logger logger;
WifiHandler wifiHandler;

bool button_pressed_on_boot = false;

Lamp lamp(LAMP_READ_PIN, LAMP_WRITE_PIN);

void setup(void)
{

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  system_update_cpu_freq(SYS_CPU_160MHZ);

  Serial.begin(115200); // TODO remove
  logger.setLevel(logger.DEBUG);

  delay(BOOT_DELAY);
  button_pressed_on_boot = !digitalRead(BUTTON_PIN);
  button_pressed_on_boot = false; // TODO remove

  espEepromSettings.format();
  if (espEepromSettings.read() != ESP_EEPROM_OK)
  {
    logger.error("Formatting needed...");
    espEepromSettings.format();
  }

  if (button_pressed_on_boot || (strlen(espEepromSettings.getWifiSsid()) == 0) || (strlen(espEepromSettings.getWifiPassword()) == 0))
  {
    logger.debug("Begin AP\n");

    wifiHandler.beginAP();
  }
  else
  {
    logger.debug("Begin ST\n");
    wifiHandler.beginST();
  }

  randomSeed(micros());

  MDNS.begin(MDNS_NAME);
  MDNS.addService("http", "tcp", 80); // Announce esp tcp service on port 80

  wifiHandler.init();

  mqtt.init();

  lamp.setCallback(MqttHandler::updateLevel);
}

void loop(void)
{
  server.handleClient();
  mqtt.handle();
  lamp.handle();
}
