#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <ESP8266WebServer.h>
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
        server.on("/", std::bind(&WifiHandler::handleRoot, this));
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
};
#endif /* WIFI_HANDLER_H */