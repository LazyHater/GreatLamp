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


ESP8266WebServer server(80);
WiFiClient espClient;
MqttHandler mqtt(espClient);

const char INDEX_HTML[] = "<!DOCTYPE html><html lang=\"en\"><head> <meta name=\"author\" content=\"\"> <meta name=\"description\" content=\"\"> <meta charset=\"utf-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\"> <link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css\" integrity=\"sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm\" crossorigin=\"anonymous\"> <link href='http://fonts.googleapis.com/css?family=Cookie' rel='stylesheet' type='text/css'> <style>button,h1{color:#ffc107;text-shadow:4px 4px 3px rgba(0,0,0,.1)}.raise:focus,.raise:hover{box-shadow:0 .5em .5em -.4em var(--hover);-webkit-transform:translateY(-.25em);transform:translateY(-.25em)}.raise{--color:#ffa260;--hover:#e5ff60}.xd{background:#555}.no-margin{margin:0}.modal-style{background-color:#444}h5{color:#ffc107}body{color:#fff;background:#000}.top-buffer{margin-top:20px}button{-webkit-transition:.25s;transition:.25s;background:0 0;border:2px solid;line-height:1;margin:.5em;padding:1em 2em;font:400 17px/.8 Cookie,Helvetica,sans-serif}button:focus,button:hover{border-color:var(--hover);color:#fff}h1{font:400 50px/.8 Cookie,Helvetica,sans-serif}</style> <link rel=\"icon\" type=\"image/png\" href=\"https://image.ibb.co/fxD5An/favicon2.png\"> <title>Let's make lampka xD again</title></head><body> <main class=\"container-fluid\"> <div id=\"alert\" class=\"alert top-buffer\" role=\"alert\"> </div><div class=\"col-12 text-center\"> <img id=\"light\" src=\"img/wait.png\" alt=\"light level image\" class=\"img-responsive\"> </div><div class=\"row\"> <div class=\"col text-center top-buffer\"> <h1>Let's make lampka xD again</h1> </div></div><div class=\"row\"> <div class=\"col text-center\"> <button id=\"lv0\" class=\"raise\">xd</button> <button id=\"lv1\" class=\"raise\">xD</button> <button id=\"lv2\" class=\"raise\">XD</button> <button id=\"lv3\" class=\"raise\">XDDD</button> </div></div><div class=\"row\"> <div class=\"col top-buffer text-center\"> <a href=\"#\" data-toggle=\"collapse\" data-target=\"#settings\" class=\"text-muted\">.xX settings Xx.</a> </div></div><row id=\"settings\" class=\"row top-buffer justify-content-center text-center collapse\"> <button type=\"button\" class=\"btn btn-warning btn-lg\" data-toggle=\"modal\" data-target=\"#wifiModal\"> Wifi Settings </button> <button id=\"mqttbtn\" type=\"button\" class=\"btn btn-warning btn-lg\"> Mqtt Settings </button> <button id=\"docsbtn\" type=\"button\" class=\"btn btn-warning btn-lg\" data-toggle=\"modal\" data-target=\"#mqttModal\"> Docs </button> <button id=\"restart\" class=\"btn btn-danger btn-lg\"> Restart </button> </row> <div class=\"modal fade\" id=\"wifiModal\" tabindex=\"-1\" role=\"dialog\" aria-labelledby=\"exampleModalLabel\" aria-hidden=\"true\"> <div class=\"modal-dialog\" role=\"document\" > <div class=\"modal-content\"> <div class=\"modal-body modal-style\"> <h5 class=\"modal-title\" id=\"exampleModalLabel\">Wifi settings</h5> <form id=\"wifiSettingsForm\"> <div class=\"form-group top-buffer\"> <div class=\"form-group\"> <input id=\"ssid\" type=\"text\" class=\"form-control\" placeholder=\"SSID\"> </div><input id=\"password\" type=\"text\" class=\"form-control\" placeholder=\"Password\"> <button id=\"wifisub\" type=\"submit\" class=\"btn btn-primary btn-lg btn-block btn-warning no-margin top-buffer\">Submit</button> </div></form> </div></div></div></div><div class=\"modal fade\" id=\"mqttModal\" tabindex=\"-1\" role=\"dialog\" aria-labelledby=\"exampleModalLabel\" aria-hidden=\"true\"> <div class=\"modal-dialog\" role=\"document\" > <div class=\"modal-content\"> <div class=\"modal-body modal-style\"> <h5 class=\"modal-title\" id=\"exampleModalLabel\">Mqtt settings</h5> <table class=\"table table-dark top-buffer\"> <thead> <tr> <th scope=\"col\">Key</th> <th scope=\"col\">Value</th> </tr></thead> <tbody> <tr> <td>Mqtt host</td><td id=\"mqttHostTable\"></td></tr><tr> <td>Is enabled</td><td id=\"mqttEnableTable\"></td></tr><tr> <td>Is connected</td><td id=\"mqttConnectedTable\"></td></tr></tbody> </table> <form id=\"mqttSettingsForm\" > <div class=\"form-group\"> <input id=\"mqtthost\" type=\"text\" class=\"form-control top-buffer\" placeholder=\"New Mqtt hostname\"> <button id=\"mqttsub\" action=\"#\" type=\"submit\" class=\"btn btn-primary btn-lg btn-block btn-warning no-margin top-buffer\">Submit</button> </div></form> </div></div></div></div></main> <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script> <script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.9/umd/popper.min.js\" integrity=\"sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q\" crossorigin=\"anonymous\"></script> <script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/js/bootstrap.min.js\" integrity=\"sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl\" crossorigin=\"anonymous\"></script> <script>var baseUrl=\"/\",imgUrls=[\"https://image.ibb.co/kWdiwS/lv0.png\",\"https://image.ibb.co/gmuOwS/lv1.png\",\"https://image.ibb.co/c3vkAn/lv2.png\",\"https://image.ibb.co/mSxMi7/lv3.png\",\"https://image.ibb.co/duXyVn/error.png\",\"https://image.ibb.co/kAZ1i7/wait.png\"];$(document).ready(function(){$(\"#alert\").hide(),$(\"#light\").attr(\"src\",imgUrls[5]),$(\"#wifiSettingsForm\").submit(function(){return $(\"#wifiModal\").modal(\"hide\"),setWifiSettings($(\"#ssid\").val(),$(\"#password\").val()),!1}),$(\"#mqttSettingsForm\").submit(function(){return $(\"#mqttModal\").modal(\"hide\"),setMqttHost($(\"#mqtthost\").val()),!1}),$(\"#lv0\").click(function(){setLevel(0)}),$(\"#lv1\").click(function(){setLevel(1)}),$(\"#lv2\").click(function(){setLevel(2)}),$(\"#lv3\").click(function(){setLevel(3)}),$(\"#light\").click(function(){toggleLamp()}),$(\"#restart\").click(function(){restartLamp()}),$(\"#mqttbtn\").click(function(){openMqttModal()}),$.ajax({url:baseUrl+\"lamp\",success:function(a){$(\"#lv\"+a.level).first().focus(),changeImageLevel(a.level)},error:onError})});function successAlert(a){$(\"#alert\").removeClass(\"alert-danger\"),$(\"#alert\").addClass(\"alert-warning\"),$(\"#alert\").html(\"<strong>Success!</strong> \"+a),$(\"#alert\").fadeTo(2e3,500).slideUp(500,function(){$(\"#alert\").slideUp(500)})}function errorAlert(a){$(\"#alert\").removeClass(\"alert-warning\"),$(\"#alert\").addClass(\"alert-danger\"),$(\"#alert\").html(\"<strong>Error!</strong> \"+a),$(\"#alert\").fadeTo(2e3,500).slideUp(500,function(){$(\"#alert\").slideUp(500)})}function getMqttHost(){$.ajax({type:\"POST\",url:baseUrl+\"mqtthost\",success:function(a){console.log(a)},error:onError})}function openMqttModal(){$.ajax({url:baseUrl+\"mqttstatus\",success:function(a){$(\"#mqttHostTable\").html(a.mqtt_host),$(\"#mqttEnableTable\").html(a.mqtt_enabled),$(\"#mqttConnectedTable\").html(a.mqtt_connected),$(\"#mqttModal\").modal(\"show\")},error:onError})}function setLevel(a){$.ajax({type:\"POST\",url:baseUrl+\"lamp\",data:{level:a},success:function(b){$(\"#lv\"+b.level).first().focus(),changeImageLevel(b.level)},error:onError})}function setMqttHost(a){$.ajax({type:\"POST\",url:baseUrl+\"mqtthost\",data:{mqtt_host:a},success:function(b){console.log(b),successAlert(\"Mqtt host has been updated!\")},error:onError})}function setWifiSettings(a,b){console.log({ssid:a,password:b}),$.ajax({type:\"POST\",url:baseUrl+\"wifisettings\",data:{ssid:a,password:b},success:function(c){console.log(c),successAlert(\"Wifi settings have been updated!\")},error:onError})}function restartLamp(){$.ajax({url:baseUrl+\"restart\",success:function(){location.reload()},error:onError})}function toggleLamp(){$.ajax({url:baseUrl+\"toggle\",success:function(a){$(\"#lv\"+a.level).first().focus(),changeImageLevel(a.level)},error:onError})}function changeImageLevel(a){$(\"#light\").fadeOut(200,function(){$(\"#light\").attr(\"src\",imgUrls[a])}).fadeIn(200)}function onError(a){console.log(a),console.log(a.responseJSON.error),errorAlert(a.responseJSON.error),changeImageLevel(4)}\n</script></body></html>\n";
const char DOC_HTML[] = "xDDD";
const int buff_size = 1000;
char buff[buff_size];

/*
  const int LAMP_WRITE_PIN = D0;
  const int LAMP_READ_PIN = D1;
  const int BUTTON_PIN = D2;
*/

const int LAMP_WRITE_PIN = 2;
const int LAMP_READ_PIN = 3;
const int BUTTON_PIN = 1;
bool button_pressed_on_boot = false;

Lamp lamp(LAMP_READ_PIN, LAMP_WRITE_PIN);

String ok_msg = "{\"status\": \"ok\"}";

void returnFailJSON(String& msg) {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "application/json", msg);
}

void returnJSON(String& msg) {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", msg);
}

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

void handleMqttStatus() {
  String s = "{";

  s += "\"mqtt_host\": \"" + eepromHandler.getMqttHost() + "\",";
  s += "\"mqtt_enabled\": ";
  s += (mqtt.isEnabled() ? "\"yes\"," : "\"no\",");
  s += "\"mqtt_connected\": ";
  s += (mqtt.isConnected() ? "\"yes\"}" : "\"no\"}");

  returnJSON(s);
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

bool isValidNumber(String& s) {
  for (size_t i = 0; i < s.length(); i++) {
    if (!isDigit(s.charAt(i)))
      return false;
  }
  return true;
}

void handleLamp() {
  if (server.hasArg("level")) {
    String level_s = server.arg("level");
    if (!isValidNumber(level_s)) {
      return returnFailJSON("{\"status\": \"error\", \"error\": \"LEVEL VALUE IS NOT A VALID INTEGER:" + level_s + "\"}");
    }

    int level = server.arg("level").toInt();

    if (level < 0 || level > 3) {
      return returnFailJSON("{\"status\": \"error\", \"error\": \"LEVEL VALUE OUT OF RANGE <0, 3>:" + String(level) + "\"}");
    }

    lamp.setLevel(level);
  }

  returnJSON("{\"level\": " + String(lamp.readLevel()) + ",\"status\": \"ok\"}");
}

void handleToggle() {
  lamp.toggle();

  returnJSON("{\"level\": " + String(lamp.readLevel()) + ",\"status\": \"ok\"}");
}

void handleRestart() {
  returnJSON(ok_msg);

  ESP.restart();
}

void handleWifiSettings() {
  bool success = false;
  if (server.hasArg("ssid") && server.hasArg("password")) {
    eepromHandler.init();
    success = eepromHandler.writeWifiParameters(server.arg("ssid"), server.arg("password"));
    eepromHandler.readWiFiParameters();
    eepromHandler.end();
  }

  if (success) {
    returnJSON(ok_msg);
  } else {
    returnFailJSON("{\"status\": \"error\", \"error\":\"Failed to set ssid and password for " + server.arg("ssid") + "\"}");
  }
}


void handleMqttHost() {
  bool success = true;
  eepromHandler.init();

  if (server.hasArg("mqtt_host")) {
    success = eepromHandler.writeMqttHost(server.arg("mqtt_host"));
  }

  eepromHandler.readMqttHost();
  eepromHandler.end();

  if (success) {
    returnJSON("{\"status\": \"ok\", \"mqtt_host\": \"" + eepromHandler.getMqttHost() + "\"}");
  } else {
    returnFailJSON("{\"status\": \"error\", \"error\":\"Failed to set " + server.arg("mqtt_host") + " as mqtt host.\"}");
  }
}

void beginST() {
  eepromHandler.init();
  eepromHandler.readWiFiParameters();
  eepromHandler.end();

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
  delay(2000);
  button_pressed_on_boot = !digitalRead(BUTTON_PIN);

  if (button_pressed_on_boot) {
    beginAP();
  } else {
    beginST();
  }


  MDNS.begin("xd-lamp");
  MDNS.addService("http", "tcp", 80); // Announce esp tcp service on port 80

  server.on("/", handleRoot);
  server.on("/lamp", handleLamp);
  server.on("/wifisettings", handleWifiSettings);
  server.on("/mqtthost", handleMqttHost);
  server.on("/toggle", handleToggle);
  server.on("/restart", handleRestart);
  server.on("/doc", handleDoc);
  server.on("/mqttstatus", handleMqttStatus);
  server.onNotFound(handleNotFound);

  server.begin();

  mqtt.init();
}

void loop(void) {
  server.handleClient();
  mqtt.handle();
}
