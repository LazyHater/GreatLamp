#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "Utils.h"
#include "Html.h"
String ok_msg = "{\"status\": \"ok\"}";
extern ESP8266WebServer server;
extern MqttHandler mqtt;
extern Lamp lamp;
extern Logger logger;

class WifiHandler
{
  public:
    void init()
    {
        // server.on("/", std::bind(&WifiHandler::handleRoot, this));
        server.on("/", [this]() {
            this->returnHtml(INDEX_HTML);
        });
        server.on("/docs", [this]() {
            this->returnHtml(DOCS_HTML);
        });
        server.on("/docs", std::bind(&WifiHandler::handleDoc, this));
        server.on("/api/lamp", std::bind(&WifiHandler::handleLamp, this));
        server.on("/api/lamp/set", std::bind(&WifiHandler::handleLampSet, this));
        server.on("/api/lamp/toggle", std::bind(&WifiHandler::handleLampToggle, this));
        server.on("/api/mqtt", std::bind(&WifiHandler::handleMqtt, this));
        server.on("/api/wifi", std::bind(&WifiHandler::handleWifi, this));
        server.on("/api/restart", std::bind(&WifiHandler::handleRestart, this));
        server.on("/api/format", std::bind(&WifiHandler::handleFormat, this));

        server.onNotFound(std::bind(&WifiHandler::handleNotFound, this));

        server.begin();
    }

    void returnHtml(const char *html, int code = 200)
    {
        server.sendHeader("Connection", "close");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(code, "text/html", html);
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
                returnJsonError("Level value is not an integer.");
                return;
            }

            int level = server.arg("level").toInt();

            if (level < 0 || level > 3)
            {
                returnJsonError("Level value out of range <0, 3>");
                return;
            }

            lamp.setLevel(level);
        }

        returnLampLevel();
    }

    void handleLampToggle()
    {
        lamp.toggle();
        returnLampLevel();
    }

    void handleLamp()
    {
        returnLampLevel();
    }

    void returnLampLevel()
    {

        StaticJsonBuffer<128> jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();
        root["level"] = lamp.readLevel();
        returnJson(root);
    }

    void handleRestart()
    {
        StaticJsonBuffer<128> jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();
        returnJson(root);

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
            StaticJsonBuffer<512> jsonBuffer;
            JsonObject &root = jsonBuffer.createObject();
            root["ssid"] = espEepromSettings.getWifiSsid();
            root["password"] = espEepromSettings.getWifiPassword();
            root["wifi_status"] = WL_STATUSES[WiFi.status()];
            root["rssi"] = WiFi.RSSI();
            root["localip"] = WiFi.localIP().toString();
            root["subnetmask"] = WiFi.subnetMask().toString();
            root["gatewayip"] = WiFi.gatewayIP().toString();
            root["dnsip"] = WiFi.dnsIP().toString();
            root["hostname"] = WiFi.hostname();
            root["bssid"] = WiFi.BSSIDstr();
            root["macaddress"] = WiFi.macAddress();
            root["channel"] = WiFi.channel();
            root["phymode"] = PHY_MODE_NAMES[wifi_get_phy_mode()];
            root["cpufreq"] = system_get_cpu_freq();
            root["rstreason"] = RST_REASONS[system_get_rst_info()->reason];

            returnJson(root);
        }
        else
        {
            returnJsonError("Failed to set ssid or password.");
        }
    }

    void handleFormat()
    {
        espEepromSettings.format();
        StaticJsonBuffer<128> jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();
        returnJson(root);
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

            StaticJsonBuffer<512> jsonBuffer;
            JsonObject &root = jsonBuffer.createObject();
            root["mqtt_host"] = espEepromSettings.getMqttHostname();
            root["mqtt_port"] = espEepromSettings.getMqttPort();
            root["mqtt_username"] = espEepromSettings.getMqttUsername();
            root["mqtt_password"] = espEepromSettings.getMqttPassword();
            root["mqtt_enabled"] = mqtt.isEnabled() ? "yes" : "no";
            root["mqtt_state"] = MQTT_STATUSES[mqtt.getState() + 4];
            returnJson(root);
        }
        else
        {
            returnJsonError("Failed to set mqtt settings!");
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
};
#endif /* WIFI_HANDLER_H */