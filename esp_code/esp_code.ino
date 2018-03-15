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
#include "Lamp.h"
#include "EepromHandler.h"
#include "MqttHandler.h"
#include "Utils.h"

extern "C" {
#include "user_interface.h"
}

ESP8266WebServer server(80);
WiFiClient espClient;
MqttHandler mqtt(espClient);


const char INDEX_HTML[] PROGMEM = "<!DOCTYPE html><html lang=\"en\"><head> <meta name=\"author\" content=\"Dominik Hofman\"> <meta name=\"description\" content=\"Best lampka device\"> <meta charset=\"utf-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\"> <link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css\" integrity=\"sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm\" crossorigin=\"anonymous\"> <link href='http://fonts.googleapis.com/css?family=Cookie' rel='stylesheet' type='text/css'> <style>button,h1{color:#ffc107;text-shadow:4px 4px 3px rgba(0,0,0,.1)}.raise:focus,.raise:hover{box-shadow:0 .5em .5em -.4em var(--hover);-webkit-transform:translateY(-.25em);transform:translateY(-.25em)}.raise{--color:#ffa260;--hover:#e5ff60}.xd{background:#555}.no-margin{margin:0}.modal-style{background-color:#444}h5{color:#ffc107}body{color:#fff;background:#000}.top-buffer{margin-top:20px}button{-webkit-transition:.25s;transition:.25s;background:0 0;border:2px solid;line-height:1;margin:.5em;padding:1em 2em;font:400 17px/.8 Cookie,Helvetica,sans-serif}button:focus,button:hover{border-color:var(--hover);color:#fff}h1{font:400 50px/.8 Cookie,Helvetica,sans-serif}\n</style> <link rel=\"icon\" type=\"image/png\" href=\"https://image.ibb.co/fxD5An/favicon2.png\"> <title>Let's make lampka xD again</title></head><body> <main class=\"container-fluid\"> <div id=\"alert\" class=\"alert top-buffer\" role=\"alert\"> </div><div class=\"col-12 text-center\"> <img id=\"light\" src=\"img/wait.png\" alt=\"light level image\" class=\"img-responsive\"> </div><div class=\"row\"> <div class=\"col text-center top-buffer\"> <h1>Let's make lampka xD again</h1> </div></div><div class=\"row\"> <div class=\"col text-center\"> <button id=\"lv0\" class=\"raise\">xd</button> <button id=\"lv1\" class=\"raise\">xD</button> <button id=\"lv2\" class=\"raise\">XD</button> <button id=\"lv3\" class=\"raise\">XDDD</button> </div></div><div class=\"row\"> <div class=\"col top-buffer text-center\"> <a href=\"#\" data-toggle=\"collapse\" data-target=\"#settings\" class=\"text-muted\">.xX settings Xx.</a> </div></div><row id=\"settings\" class=\"row top-buffer justify-content-center text-center collapse\"> <button id=\"wifibtn\" type=\"button\" class=\"btn btn-warning btn-lg\"> Wifi Settings </button> <button id=\"mqttbtn\" type=\"button\" class=\"btn btn-warning btn-lg\"> Mqtt Settings </button> <button id=\"docsbtn\" type=\"button\" class=\"btn btn-warning btn-lg\"> Docs </button> <button id=\"restart\" class=\"btn btn-danger btn-lg\"> Restart </button> </row> <div class=\"modal fade\" id=\"wifiModal\" tabindex=\"-1\" role=\"dialog\" aria-labelledby=\"exampleModalLabel\" aria-hidden=\"true\"> <div class=\"modal-dialog\" role=\"document\" > <div class=\"modal-content\"> <div class=\"modal-body modal-style\"> <h5 class=\"modal-title\">Wifi settings</h5> <table class=\"table table-dark top-buffer\" id=\"wifiTable\"> <thead> <tr> <th scope=\"col\">Key</th> <th scope=\"col\">Value</th> </tr></thead> <tbody> </tbody> </table> <form id=\"wifiSettingsForm\"> <div class=\"form-group top-buffer\"> <div class=\"form-group\"> <input id=\"ssid\" type=\"text\" class=\"form-control\" placeholder=\"New SSID\"> </div><input id=\"password\" type=\"text\" class=\"form-control\" placeholder=\"New Password\"> <button id=\"wifisub\" type=\"submit\" class=\"btn btn-primary btn-lg btn-block btn-warning no-margin top-buffer\">Submit</button><button type=\"button\" class=\"btn btn-secondary btn-lg btn-block btn-warning no-margin top-buffer\" data-dismiss=\"modal\">Close</button></div></form> </div></div></div></div><div class=\"modal fade\" id=\"mqttModal\" tabindex=\"-1\" role=\"dialog\" aria-labelledby=\"exampleModalLabel\" aria-hidden=\"true\"> <div class=\"modal-dialog\" role=\"document\" > <div class=\"modal-content\"> <div class=\"modal-body modal-style\"> <h5 class=\"modal-title\">Mqtt settings</h5> <table class=\"table table-dark top-buffer\" id=\"mqttTable\"> <thead> <tr> <th scope=\"col\">Key</th> <th scope=\"col\">Value</th> </tr></thead> <tbody> </tbody> </table> <form id=\"mqttSettingsForm\" > <div class=\"form-group\"> <input id=\"mqtthost\" type=\"text\" class=\"form-control top-buffer\" placeholder=\"New Mqtt hostname\"> <button id=\"mqttsub\" action=\"#\" type=\"submit\" class=\"btn btn-primary btn-lg btn-block btn-warning no-margin top-buffer\">Submit</button><button type=\"button\" class=\"btn btn-secondary btn-lg btn-block btn-warning no-margin top-buffer\" data-dismiss=\"modal\">Close</button> </div></form> </div></div></div></div></main> <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script> <script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.9/umd/popper.min.js\" integrity=\"sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q\" crossorigin=\"anonymous\"></script> <script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/js/bootstrap.min.js\" integrity=\"sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl\" crossorigin=\"anonymous\"></script> <script>var baseUrl=\"/\",imgUrls=[\"https://image.ibb.co/kWdiwS/lv0.png\",\"https://image.ibb.co/gmuOwS/lv1.png\",\"https://image.ibb.co/c3vkAn/lv2.png\",\"https://image.ibb.co/mSxMi7/lv3.png\",\"https://image.ibb.co/duXyVn/error.png\",\"https://image.ibb.co/kAZ1i7/wait.png\"];$(document).ready(function(){$(\"#alert\").hide(),$(\"#light\").attr(\"src\",imgUrls[5]),$(\"#wifiSettingsForm\").submit(function(){return $(\"#wifiModal\").modal(\"hide\"),setWifiSettings($(\"#ssid\").val(),$(\"#password\").val()),!1}),$(\"#mqttSettingsForm\").submit(function(){return $(\"#mqttModal\").modal(\"hide\"),setMqttHost($(\"#mqtthost\").val()),!1}),$(\"#lv0\").click(function(){setLevel(0)}),$(\"#lv1\").click(function(){setLevel(1)}),$(\"#lv2\").click(function(){setLevel(2)}),$(\"#lv3\").click(function(){setLevel(3)}),$(\"#light\").click(function(){toggleLamp()}),$(\"#restart\").click(function(){restartLamp()}),$(\"#mqttbtn\").click(function(){openMqttModal()}),$(\"#wifibtn\").click(function(){openWifiModal()}),$(\"#docsbtn\").click(function(){location.href=\"/docs\"}),$.ajax({url:baseUrl+\"lamp\",success:function(a){$(\"#lv\"+a.level).first().focus(),changeImageLevel(a.level)},error:onError})});function successAlert(a){$(\"#alert\").removeClass(\"alert-danger\"),$(\"#alert\").addClass(\"alert-warning\"),$(\"#alert\").html(\"<strong>Success!</strong> \"+a),$(\"#alert\").fadeTo(2e3,500).slideUp(500,function(){$(\"#alert\").slideUp(500)})}function createTable(a,b){for(let c in $(\"#\"+a+\" tbody tr\").remove(),b)if(b.hasOwnProperty(c)){if(\"status\"==c)continue;tableAddRow(a,c,b[c])}}function tableAddRow(a,b,c){$(\"#\"+a+\" > tbody:last-child\").append(\"<tr><td>\"+b+\"</td><td>\"+c+\"</td></tr>\")}function errorAlert(a){$(\"#alert\").removeClass(\"alert-warning\"),$(\"#alert\").addClass(\"alert-danger\"),$(\"#alert\").html(\"<strong>Error!</strong> \"+a),$(\"#alert\").fadeTo(2e3,500).slideUp(500,function(){$(\"#alert\").slideUp(500)})}function openMqttModal(){$.ajax({url:baseUrl+\"mqtt\",success:function(a){createTable(\"mqttTable\",a),$(\"#mqttModal\").modal(\"show\")},error:onError})}function openWifiModal(){$.ajax({url:baseUrl+\"wifi\",success:function(a){createTable(\"wifiTable\",a),$(\"#wifiModal\").modal(\"show\")},error:onError})}function setLevel(a){$.ajax({type:\"POST\",url:baseUrl+\"lamp/set\",data:{level:a},success:function(b){$(\"#lv\"+b.level).first().focus(),changeImageLevel(b.level)},error:onError})}function setMqttHost(a){$.ajax({type:\"POST\",url:baseUrl+\"mqtt\",data:{mqtt_host:a},success:function(){successAlert(\"Mqtt host has been updated!\")},error:onError})}function setWifiSettings(a,b){$.ajax({type:\"POST\",url:baseUrl+\"wifi\",data:{ssid:a,password:b},success:function(){successAlert(\"Wifi settings have been updated!\")},error:onError})}function restartLamp(){$.ajax({url:baseUrl+\"restart\",success:function(){location.reload()},error:onError})}function toggleLamp(){$.ajax({url:baseUrl+\"lamp/toggle\",success:function(a){$(\"#lv\"+a.level).first().focus(),changeImageLevel(a.level)},error:onError})}function changeImageLevel(a){$(\"#light\").fadeOut(200,function(){$(\"#light\").attr(\"src\",imgUrls[a])}).fadeIn(200)}function onError(a){errorAlert(a.responseJSON.error),changeImageLevel(4)}\n</script></body></html>";
const char DOC_HTML[] PROGMEM = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">\n\n  <link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css\" integrity=\"sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm\" crossorigin=\"anonymous\">\n  <link href='http://fonts.googleapis.com/css?family=Cookie' rel='stylesheet' type='text/css'>\n  <style>\n  body {\n    background: #000;\n  }\n  .list-group-item {\n    background: #444;\n    color: #ff0;\n  }\n  .list-group-item.active {\n    background-color: #ffc107;\n    border-color: #aed248;\n    color: #fff;\n  }\n\n  .jumbotron {\n    margin-top:20px;\n    background-color: #222222;\n    border-color: #aed248;\n    color: #ff0;\n  }\n  </style>\n  <link rel=\"icon\" type=\"image/png\" href=\"https://image.ibb.co/fxD5An/favicon2.png\">\n\n  <title>Docs</title>\n</head>\n\n<body>\n  <main class=\"container-fluid\">\n    <div class=\"jumbotron\">\n      <h1>REST API</h1><br>\n      <div class=\"row\">\n        <div class=\"col-4\">\n          <div class=\"list-group\" id=\"list-tab-api\" role=\"tablist\">\n            <a class=\"list-group-item list-group-item-action active\" id=\"list-home-list\" data-toggle=\"list\" href=\"#list-/\" role=\"tab\" aria-controls=\"home\">/</a>\n            <a class=\"list-group-item list-group-item-action\" id=\"list-profile-list\" data-toggle=\"list\" href=\"#list-/lamp\" role=\"tab\" aria-controls=\"profile\">/lamp</a>\n            <a class=\"list-group-item list-group-item-action\" id=\"list-messages-list\" data-toggle=\"list\" href=\"#list-/lamp/set\" role=\"tab\" aria-controls=\"messages\">/lamp/set</a>\n            <a class=\"list-group-item list-group-item-action\" id=\"list-settings-list\" data-toggle=\"list\" href=\"#list-/lamp/toggle\" role=\"tab\" aria-controls=\"settings\">/lamp/toggle</a>\n            <a class=\"list-group-item list-group-item-action\" id=\"list-home-list\" data-toggle=\"list\" href=\"#list-/mqtt\" role=\"tab\" aria-controls=\"home\">/mqtt</a>\n            <a class=\"list-group-item list-group-item-action\" id=\"list-profile-list\" data-toggle=\"list\" href=\"#list-/wifi\" role=\"tab\" aria-controls=\"profile\">/wifi</a>\n            <a class=\"list-group-item list-group-item-action\" id=\"list-messages-list\" data-toggle=\"list\" href=\"#list-/restart\" role=\"tab\" aria-controls=\"messages\">/restart</a>\n            <a class=\"list-group-item list-group-item-action\" id=\"list-settings-list\" data-toggle=\"list\" href=\"#list-/docs\" role=\"tab\" aria-controls=\"settings\">/docs</a>\n          </div>\n        </div>\n        <div class=\"col-8\">\n          <div class=\"tab-content\" id=\"nav-tabContent-api\">\n            <div class=\"tab-pane fade show active\" id=\"list-/\" role=\"tabpanel\" aria-labelledby=\"list-home-list\">displays lamp homepage</div>\n            <div class=\"tab-pane fade\" id=\"list-/lamp\" role=\"tabpanel\" aria-labelledby=\"list-profile-list\">returns json object with status and level property, where level is current lamp level</div>\n            <div class=\"tab-pane fade\" id=\"list-/lamp/set\" role=\"tabpanel\" aria-labelledby=\"list-messages-list\">when arg \"level\" provided, sets lamp to desired level, level has to be integer with value <0, 3></div>\n            <div class=\"tab-pane fade\" id=\"list-/lamp/toggle\" role=\"tabpanel\" aria-labelledby=\"list-settings-list\">levels up lamp, when level excedes 3 then cycles to 0</div>\n            <div class=\"tab-pane fade\" id=\"list-/mqtt\" role=\"tabpanel\" aria-labelledby=\"list-home-list\">when arg \"mqtt_host\" provided, sets mqtt hostname. Hostname has to be no longer than 64 characters. If hostname is set to empty string then mqtt funcionality is disabled. All changes to mqtthost are applied after reboot<br>\n              Returns json object with keys:<br>\n              \"mqtt_host\" - currently used mqtt hostname<br>\n              \"mqtt_enabled\" - if this is set to \"no\" then lamp wouldn't try to connect to mqtt broker <br>\n              \"mqtt_connected\" - returns \"yes\" if is connected to mqtt broker</div>\n              <div class=\"tab-pane fade\" id=\"list-/wifi\" role=\"tabpanel\" aria-labelledby=\"list-profile-list\">when args \"ssid\" and \"password\" are provided, sets it. SSID and password have to be no longer than 64 characters. If SSID is set to empty string then on next boot lamp will start in AP_MODE.  All changes to wifi settings are applied after reboot. Returns JSON object with wifi settings.</div>\n              <div class=\"tab-pane fade\" id=\"list-/restart\" role=\"tabpanel\" aria-labelledby=\"list-messages-list\">restarts lamp</div>\n              <div class=\"tab-pane fade\" id=\"list-/docs\" role=\"tabpanel\" aria-labelledby=\"list-settings-list\">this page</div>\n            </div>\n          </div>\n        </div>\n      </div>\n\n      <div class=\"jumbotron\">\n        <h1>MQTT topics</h1><br>\n        <div class=\"row\">\n          <div class=\"col-4\">\n            <div class=\"list-group\" id=\"list-tab-mqtt\" role=\"tablist\">\n              <a class=\"list-group-item list-group-item-action active\" id=\"list-home-list\" data-toggle=\"list\" href=\"#list-home\" role=\"tab\" aria-controls=\"home\">home/xdlamp/set</a>\n              <a class=\"list-group-item list-group-item-action\" id=\"list-profile-list\" data-toggle=\"list\" href=\"#list-profile\" role=\"tab\" aria-controls=\"profile\">home/xdlamp/toggle</a>\n            </div>\n          </div>\n          <div class=\"col-8\">\n            <div class=\"tab-content\" id=\"nav-tabContent-mqtt\">\n              <div class=\"tab-pane fade show active\" id=\"list-home\" role=\"tabpanel\" aria-labelledby=\"list-home-list\">sets lamp to desired level, in payload should be number between 0 and 3</div>\n              <div class=\"tab-pane fade\" id=\"list-profile\" role=\"tabpanel\" aria-labelledby=\"list-profile-list\">same behavior as in rest api \"/lamp/toggle\", payload is ignored</div>\n            </div>\n          </div>\n        </div>\n      </div><button type=\"button\" class=\"btn btn-secondary btn-lg btn-block btn-warning no-margin top-buffer\" onclick=\"location.href='/'\">Return</button></main>\n    <script src=\"https://code.jquery.com/jquery-3.2.1.slim.min.js\" integrity=\"sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN\" crossorigin=\"anonymous\"></script>\n    <script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.9/umd/popper.min.js\" integrity=\"sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q\" crossorigin=\"anonymous\"></script>\n    <script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/js/bootstrap.min.js\" integrity=\"sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl\" crossorigin=\"anonymous\"></script></body>";

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
  server.send(200, "text/html", DOC_HTML);
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
    json += addJsonKeyValue("mqtt_connected",  mqtt.isConnected() ? "yes" : "no");
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
  
  system_update_cpu_freq(SYS_CPU_160MHZ); 
  
  delay(2000);
  button_pressed_on_boot = !digitalRead(BUTTON_PIN);

  eepromHandler.init();
  eepromHandler.readWiFiParameters();
  eepromHandler.end();

  if (button_pressed_on_boot || (eepromHandler.getSsid().length() == 0)) {
    beginAP();
  } else {
    beginST();
  }

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
