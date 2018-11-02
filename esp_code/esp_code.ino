/*
Demonstrate using an http server and an HTML form to control an LED.
The http server runs on the ESP8266.

Connect to "http://esp8266WebForm.local" or "http://<IP address>"
to bring up an HTML form to control the LED connected GPIO#0. This works
for the Adafruit ESP8266 HUZZAH but the LED may be on a different pin on
other breakout boards.

Imperatives to turn the LED on/off using a non-browser http client.
For example, using wget.
$ wget http://esp8266webform.local/ledon
$ wget http://esp8266webform.local/ledoff
*/

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
#include "Utils.h"
#include "Html.h"
extern "C"
{
#include "user_interface.h"
}
ESP8266WebServer server(HTTP_SERVER_PORT);
WiFiClient espClient;
MqttHandler mqtt;
Logger logger;

bool button_pressed_on_boot = false;

Lamp lamp(LAMP_READ_PIN, LAMP_WRITE_PIN);

String ok_msg = "{\"status\": \"ok\"}";

void returnHomepage()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", INDEX_HTML);
}

void handleRoot()
{
  returnHomepage();
}

void handleDoc()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", DOCS_HTML);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleLampSet()
{
  if (server.hasArg("level"))
  {
    String level_s = server.arg("level");
    if (!isValidNumber(level_s))
    {
      String json = "{";
      json += addJsonKeyValue("status", "error") + ",";
      json += addJsonKeyValue("error", "LEVEL VALUE IS NOT A VALID INTEGER:" + level_s);
      json += "}";
      returnFailJSON(json);
      return;
    }

    int level = server.arg("level").toInt();

    if (level < 0 || level > 3)
    {
      String json = "{";
      json += addJsonKeyValue("status", "error") + ",";
      json += addJsonKeyValue("error", "Level value out of range <0, 3>:" + String(level));
      json += "}";
      returnFailJSON(json);
      return;
    }

    lamp.setLevel(level);
  }

  String json = "{";
  json += addJsonKeyValue("status", "ok") + ",";
  json += addJsonKeyValue("level", lamp.readLevel());
  json += "}";
  returnJSON(json);
}

void handleLampToggle()
{
  lamp.toggle();

  String json = "{";
  json += addJsonKeyValue("status", "ok") + ",";
  json += addJsonKeyValue("level", lamp.readLevel());
  json += "}";
  returnJSON(json);
}

void handleLamp()
{
  String json = "{";
  json += addJsonKeyValue("status", "ok") + ",";
  json += addJsonKeyValue("level", lamp.readLevel());
  json += "}";
  returnJSON(json);
}

void handleRestart()
{
  returnJSON(ok_msg);

  ESP.restart();
}

void handleWifi()
{
  bool success = true, changed = false;

  if (server.hasArg("ssid"))
  {
    success = success && (espEepromSettings.setWifiSsid(server.arg("ssid").c_str()) == ESP_EEPROM_OK);
    if (success)
    {
      changed = true;
    }
  }
  if (server.hasArg("password"))
  {
    success = success && (espEepromSettings.setWifiPassword(server.arg("password").c_str()) == ESP_EEPROM_OK);
    if (success)
    {
      changed = true;
    }
  }

  if (changed)
  {
    espEepromSettings.write();
  }

  espEepromSettings.read();
  if (success)
  {
    String json = "{";
    json += addJsonKeyValue("status", "ok") + ",";
    json += addJsonKeyValue("ssid", espEepromSettings.getWifiSsid()) + ",";
    json += addJsonKeyValue("password", espEepromSettings.getWifiPassword()) + ",";
    json += addJsonKeyValue("wifi_status", WL_STATUSES[WiFi.status()]) + ",";
    json += addJsonKeyValue("rssi", WiFi.RSSI()) + ",";
    json += addJsonKeyValue("localip", WiFi.localIP().toString()) + ",";
    json += addJsonKeyValue("subnetmask", WiFi.subnetMask().toString()) + ",";
    json += addJsonKeyValue("gatewayip", WiFi.gatewayIP().toString()) + ",";
    json += addJsonKeyValue("dnsip", WiFi.dnsIP().toString()) + ",";
    json += addJsonKeyValue("hostname", WiFi.hostname()) + ",";
    json += addJsonKeyValue("bssid", WiFi.BSSIDstr()) + ",";
    json += addJsonKeyValue("macaddress", WiFi.macAddress()) + ",";
    json += addJsonKeyValue("channel", WiFi.channel()) + ",";
    json += addJsonKeyValue("cpufreq", system_get_cpu_freq()) + ",";
    json += addJsonKeyValue("rstreason", RST_REASONS[system_get_rst_info()->reason]) + ",";
    json += addJsonKeyValue("phymode", PHY_MODE_NAMES[wifi_get_phy_mode()]);

    json += "}";
    returnJSON(json);
  }
  else
  {
    String json = "{";
    json += addJsonKeyValue("status", "error") + ",";
    json += addJsonKeyValue("error", "Failed to set ssid or password for ssid:" + server.arg("ssid") + " password:" + server.arg("password"));
    json += "}";
    returnFailJSON(json);
  }
}

void handleFormat()
{
  espEepromSettings.format();
  returnJSON(ok_msg);
}

void handleMqtt()
{

  bool success = true, changed = false;

  if (server.hasArg("mqtt_host"))
  {
    success = success && (espEepromSettings.setMqttHostname(server.arg("mqtt_host").c_str()) == ESP_EEPROM_OK);
    if (success)
    {
      changed = true;
    }
  }
  if (server.hasArg("mqtt_port"))
  {
    success = success && (espEepromSettings.setMqttPort(server.arg("mqtt_port").toInt()) == ESP_EEPROM_OK);
    if (success)
    {
      changed = true;
    }
  }

  if (server.hasArg("mqtt_username"))
  {
    success = success && (espEepromSettings.setMqttUsername(server.arg("mqtt_username").c_str()) == ESP_EEPROM_OK);
    if (success)
    {
      changed = true;
    }
  }

  if (server.hasArg("mqtt_password"))
  {
    success = success && (espEepromSettings.setMqttPassword(server.arg("mqtt_password").c_str()) == ESP_EEPROM_OK);
    if (success)
    {
      changed = true;
    }
  }

  if (changed)
  {
    espEepromSettings.write();
  }
  espEepromSettings.read();

  if (success)
  {
    String json = "{";
    json += addJsonKeyValue("status", "ok") + ",";
    json += addJsonKeyValue("mqtt_host", espEepromSettings.getMqttHostname()) + ",";
    json += addJsonKeyValue("mqtt_port", espEepromSettings.getMqttPort()) + ",";
    json += addJsonKeyValue("mqtt_username", espEepromSettings.getMqttUsername()) + ",";
    json += addJsonKeyValue("mqtt_password", espEepromSettings.getMqttPassword()) + ",";
    json += addJsonKeyValue("mqtt_enabled", mqtt.isEnabled() ? "yes" : "no") + ",";
    json += addJsonKeyValue("mqtt_state", MQTT_STATUSES[mqtt.getState() + 4]);
    json += "}";

    returnJSON(json);
  }
  else
  {
    String json = "{";
    json += addJsonKeyValue("status", "error") + ",";
    json += addJsonKeyValue("error", "Failed to set mqtt settings!");
    json += "}";
    returnFailJSON(json);
  }
}

void beginST()
{
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_STA);
  wifi_station_set_hostname(HOSTNAME);
  WiFi.begin(espEepromSettings.getWifiSsid(), espEepromSettings.getWifiPassword());

  logger.debug("Connecting to %s\n", espEepromSettings.getWifiSsid());
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    logger.debug(".");
    delay(500);
  }
  logger.debug("Connected!\n");
}

void beginAP()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.softAP(SOFT_AP_SSID);
}

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

    beginAP();
  }
  else
  {
    logger.debug("Begin ST\n");
    beginST();
  }

  randomSeed(micros());

  MDNS.begin(MDNS_NAME);
  MDNS.addService("http", "tcp", 80); // Announce esp tcp service on port 80

  server.on("/", handleRoot);
  server.on("/docs", handleDoc);
  server.on("/api/lamp", handleLamp);
  server.on("/api/lamp/set", handleLampSet);
  server.on("/api/lamp/toggle", handleLampToggle);
  server.on("/api/mqtt", handleMqtt);
  server.on("/api/wifi", handleWifi);
  server.on("/api/restart", handleRestart);
  server.on("/api/format", handleFormat);

  server.onNotFound(handleNotFound);

  server.begin();

  mqtt.init();

  lamp.setCallback(MqttHandler::updateLevel);
}

void loop(void)
{
  server.handleClient();
  mqtt.handle();
  lamp.handle();
}
