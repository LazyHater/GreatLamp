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

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "FS.h"

#include "Lamp.h"
#include "EepromHandler.h"
#include "MqttHandler.h"
#include "Utils.h"
#include "Html.h"
extern "C" {
#include "user_interface.h"
}
ESP8266WebServer server(80);
WiFiClient espClient;
MqttHandler mqtt(espClient);

const int LAMP_WRITE_PIN = 2;
const int LAMP_READ_PIN = 3;
const int BUTTON_PIN = 1;
bool button_pressed_on_boot = false;

Lamp lamp(LAMP_READ_PIN, LAMP_WRITE_PIN);

String ok_msg = "{\"status\": \"ok\"}";

void returnHomepage() {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", INDEX_HTML);
}

void handleRoot() {
  returnHomepage();
}

void handleDoc() {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", DOCS_HTML);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleLampSet() {
  if (server.hasArg("level")) {
    String level_s = server.arg("level");
    if (!isValidNumber(level_s)) {
      String json = "{";
      json += addJsonKeyValue("status", "error") + ",";
      json += addJsonKeyValue("error", "LEVEL VALUE IS NOT A VALID INTEGER:" + level_s);
      json += "}";
      returnFailJSON(json);
      return;
    }

    int level = server.arg("level").toInt();

    if (level < 0 || level > 3) {
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

void handleLampToggle() {
  lamp.toggle();

  String json = "{";
  json += addJsonKeyValue("status", "ok") + ",";
  json += addJsonKeyValue("level", lamp.readLevel());
  json += "}";
  returnJSON(json);
}

void handleLamp() {
  String json = "{";
  json += addJsonKeyValue("status", "ok") + ",";
  json += addJsonKeyValue("level", lamp.readLevel());
  json += "}";
  returnJSON(json);
}

void handleRestart() {
  returnJSON(ok_msg);

  ESP.restart();
}


void handleWifi() {
  bool success = true;

  eepromHandler.init();

  if (server.hasArg("ssid") && server.hasArg("password")) {
    success = eepromHandler.writeWifiParameters(server.arg("ssid"), server.arg("password"));
  }

  eepromHandler.readWiFiParameters();
  eepromHandler.end();

  if (success) {
    String json = "{";
    json += addJsonKeyValue("status", "ok") + ",";
    json += addJsonKeyValue("ssid", eepromHandler.getSsid()) + ",";
    json += addJsonKeyValue("password", eepromHandler.getPassword()) + ",";
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
  } else {
    String json = "{";
    json += addJsonKeyValue("status", "error") + ",";
    json += addJsonKeyValue("error", "Failed to set ssid and password for ssid:" + server.arg("ssid") + " password:" + server.arg("password"));
    json += "}";
    returnFailJSON(json);
  }
}

void handleMqtt() {
  bool success = true;
  eepromHandler.init();

  if (server.hasArg("mqtt_host")) {
    success = eepromHandler.writeMqttHost(server.arg("mqtt_host"));
  }

  eepromHandler.readMqttHost();
  eepromHandler.end();

  if (success) {
    String json = "{";
    json += addJsonKeyValue("status", "ok") + ",";
    json += addJsonKeyValue("mqtt_host", eepromHandler.getMqttHost()) + ",";
    json += addJsonKeyValue("mqtt_enabled",  mqtt.isEnabled() ? "yes" : "no") + ",";
    json += addJsonKeyValue("mqtt_state", MQTT_STATUSES[mqtt.getState() + 4]);
    json += "}";

    returnJSON(json);
  } else {
    String json = "{";
    json += addJsonKeyValue("status", "error") + ",";
    json += addJsonKeyValue("error", "Failed to set " + server.arg("mqtt_host") + " as mqtt host.");
    json += "}";
    returnFailJSON(json);
  }
}

void beginST() {
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_STA);
  wifi_station_set_hostname("xD-Lamp");
  WiFi.begin(eepromHandler.getSsid().c_str(), eepromHandler.getPassword().c_str());


  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void beginAP() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.softAP("xd-lamp");
}

void setup(void) {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.begin(115200);
    Serial.println("Start ");
  system_update_cpu_freq(SYS_CPU_160MHZ); 
  
  delay(2000);
  button_pressed_on_boot = !digitalRead(BUTTON_PIN);
  Serial.println(button_pressed_on_boot);
  button_pressed_on_boot = false;
  eepromHandler.init();
  eepromHandler.readWiFiParameters();
  eepromHandler.readMqttHost();
  eepromHandler.end();

  if (button_pressed_on_boot || (eepromHandler.getSsid().length() == 0)) {
    Serial.println("Begin ap");
    beginAP();
  } else {
    Serial.println("Begin st");
    beginST();
  }
  
    Serial.println("Connected");
  randomSeed(micros());

  MDNS.begin("xd-lamp");
  MDNS.addService("http", "tcp", 80); // Announce esp tcp service on port 80

  server.on("/", handleRoot);
  server.on("/lamp", handleLamp);
  server.on("/lamp/set", handleLampSet);
  server.on("/lamp/toggle", handleLampToggle);
  server.on("/mqtt", handleMqtt);
  server.on("/wifi", handleWifi);
  server.on("/restart", handleRestart);
  server.on("/docs", handleDoc);

  server.onNotFound(handleNotFound);

  server.begin();

  mqtt.init();
}

void loop(void) {
  server.handleClient();
  mqtt.handle();
  
}
