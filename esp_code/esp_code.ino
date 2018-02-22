/*
 * Demonstrate using an http server and an HTML form to control an LED.
 * The http server runs on the ESP8266.
 *
 * Connect to "http://esp8266WebForm.local" or "http://<IP address>"
 * to bring up an HTML form to control the LED connected GPIO#0. This works
 * for the Adafruit ESP8266 HUZZAH but the LED may be on a different pin on
 * other breakout boards.
 *
 * Imperatives to turn the LED on/off using a non-browser http client.
 * For example, using wget.
 * $ wget http://esp8266webform.local/ledon
 * $ wget http://esp8266webform.local/ledoff
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "Lamp.h"
#include "EepromHandler.h"

MDNSResponder mdns;

ESP8266WebServer server(80);

const char INDEX_HTML[] = "<!DOCTYPE html><html lang=\"en\"><head> <meta name=\"author\" content=\"\"> <meta name=\"description\" content=\"\"> <meta charset=\"utf-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\"> <link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css\" integrity=\"sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm\" crossorigin=\"anonymous\"> <link href='http://fonts.googleapis.com/css?family=Cookie' rel='stylesheet' type='text/css'> <style> .raise:hover,.raise:focus{box-shadow:0 .5em .5em -.4em var(--hover);-webkit-transform:translateY(-0.25em);transform:translateY(-0.25em)}.raise{--color:#ffa260;--hover:#e5ff60}.xd{background:#555}body{color:#fff;background:#000}.top-buffer{margin-top:20px}button{color:var(--color);-webkit-transition:.25s;transition:.25s;background:none;border:2px solid;font:inherit;line-height:1;margin:.5em;padding:1em 2em;font:400 17px/0.8 Cookie,Helvetica,sans-serif;color:#ff0;text-shadow:4px 4px 3px rgba(0,0,0,0.1)}button:hover,button:focus{border-color:var(--hover);color:#fff}h1{font:400 50px/.8 Cookie,Helvetica,sans-serif;color:#ff0;text-shadow:4px 4px 3px rgba(0,0,0,0.1)} </style> <link rel=\"icon\" type=\"image/png\" href=\"https://image.ibb.co/fxD5An/favicon2.png\"> <title>Let's make lampka xD again</title></head><body> <main class=\"container-fluid\"> <div class=\"col-12 text-center\"> <img id=\"light\" src=\"img/wait.png\" alt=\"light level image\" onclick=\"toggleLamp()\"> </div> <div class=\"row\"> <div class=\"col text-center top-buffer\"> <h1>Let's make lampka xD again</h1> </div> </div> <div class=\"row\"> <div class=\"col text-center\"> <button id=\"lv0\" class=\"raise\" onclick=\"setLevel(0)\">xd</button> <button id=\"lv1\" class=\"raise\" onclick=\"setLevel(1)\">xD</button> <button id=\"lv2\" class=\"raise\" onclick=\"setLevel(2)\">XD</button> <button id=\"lv3\" class=\"raise\" onclick=\"setLevel(3)\">XDDD</button> </div> </div> <div class=\"row\"> <div class=\"col top-buffer text-center\"> <a href=\"#\" data-toggle=\"collapse\" data-target=\"#settingsForm\" class=\"text-muted\">.xX settings Xx.</a> </div> </div> <div class=\"row\"> <div id=\"settingsForm\" class=\"col-4 offset-4 top-buffer text-center collapse\"> <input id=\"ssid\" type=\"text\" class=\"form-control\" placeholder=\"SSID\"> <input id=\"password\" type=\"text\" class=\"form-control top-buffer\" placeholder=\"Password\"> <div class=\"row\"> <div class=\"col-md-6 col-sm-12\"> <input type=\"button\" class=\"form-control top-buffer btn btn-danger\" value=\"Restart Lamp\" onclick=\"restartLamp()\"> </div> <div class=\"col-md-6 col-sm-12\"> <input type=\"button\" class=\"form-control top-buffer btn btn-warning\" value=\"Submit\" onclick=\"setSettings($('#ssid').val(), $('#password').val());\"> </div> </div> </div> </div> </main> <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script> <script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.9/umd/popper.min.js\" integrity=\"sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q\" crossorigin=\"anonymous\"></script> <script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/js/bootstrap.min.js\" integrity=\"sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl\" crossorigin=\"anonymous\"></script> <script>let imgUrls=[\"https://image.ibb.co/kWdiwS/lv0.png\",\"https://image.ibb.co/gmuOwS/lv1.png\",\"https://image.ibb.co/c3vkAn/lv2.png\",\"https://image.ibb.co/mSxMi7/lv3.png\",\"https://image.ibb.co/duXyVn/error.png\",\"https://image.ibb.co/kAZ1i7/wait.png\"],baseUrl=\"/\";$(document).ready(function(){$.ajax({url:baseUrl+\"lamp\",success:function(a){$(\"#lv\"+a.level).first().focus(),changeImageLevel(a.level)},error:onError})});function setLevel(a){$.ajax({type:\"POST\",url:baseUrl+\"lamp\",data:{level:a},success:function(b){$(\"#lv\"+b.level).first().focus(),changeImageLevel(b.level)},error:onError})}function setSettings(a,b){console.log({ssid:a,password:b}),$.ajax({type:\"POST\",url:baseUrl+\"settings\",data:{ssid:a,password:b},success:function(c){console.log(c)},error:onError})}function restartLamp(){$.ajax({url:baseUrl+\"restart\",success:function(){location.reload()},error:onError})}function toggleLamp(){$.ajax({url:baseUrl+\"toggle\",success:function(a){$(\"#lv\"+a.level).first().focus(),changeImageLevel(a.level)},error:onError})}function changeImageLevel(a){$(\"#light\").fadeOut(200,function(){$(\"#light\").attr(\"src\",imgUrls[a])}).fadeIn(200)}function onError(a,b,c){console.log(a,b,c),changeImageLevel(4)}</script></body></html>";

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

void returnHomepage() {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", INDEX_HTML);
}

void handleRoot() {
  returnHomepage();
}

void returnFail(String msg) {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "text/plain", msg);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
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
        return returnFail("{\"error\": \"LEVEL VALUE IS NOT A VALID INTEGER:" + level_s + "\"}");
      }

      int level = server.arg("level").toInt();

      if (level < 0 || level > 3) {
        return returnFail("{\"error\": \"LEVEL VALUE OUT OF RANGE <0, 3>:" + String(level) + "\"}");
      }

      lamp.setLevel(level);
  }

  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  String response = "{\"level\": " + String(lamp.readLevel()) + "}";
  server.send(200, "application/json", response);
}

void handleToggle() {
  int next_level = (lamp.readLevel() + 1 ) % 4;
  lamp.setLevel(next_level);
  
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  String response = "{\"level\": " + String(lamp.readLevel()) + "}";
  server.send(200, "application/json", response);
}

void handleRestart() {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "OK");

  ESP.restart();
}


void handleSettings() {
  EepromHandler eeph;
  eeph.init();

  if (server.hasArg("ssid") && server.hasArg("password")) {
     eeph.writeWifiParameters(server.arg("ssid"), server.arg("password"));
  }

  eeph.readWiFiParameters();
  eeph.end();

  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "OK");
}

void beginST() {
  EepromHandler eeph;
  eeph.init();
  eeph.readWiFiParameters();
  eeph.end();

  //Serial.print("Connecting to ");
  //Serial.println(eeph.getSsid());

  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(eeph.getSsid().c_str(), eeph.getPassword().c_str());


  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
  //Serial.println();
  //Serial.print("Connected to ");
  //Serial.println(eeph.getSsid());
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
}

void beginAP() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  //Serial.print("Setting soft-AP ... ");
  //Serial.println() ? "Ready" : "Failed!");
  WiFi.softAP("xDLamp");
}

void setup(void) {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(2000);
  button_pressed_on_boot = !digitalRead(BUTTON_PIN);

  //Serial.begin(115200);
  //Serial.println();

 if (button_pressed_on_boot) {
    beginAP();
  } else {
    beginST();
  }
/*
  if (mdns.begin("xdlamp", WiFi.localIP())) {
    //Serial.println("MDNS responder started");
  }
*/
  server.on("/", handleRoot);
  server.on("/lamp", handleLamp);
  server.on("/settings", handleSettings);
  server.on("/toggle", handleToggle);
  server.on("/restart", handleRestart);
  server.onNotFound(handleNotFound);

  server.begin();
  //Serial.print("Connect to http://xdlamp.local or http://");
 // //Serial.println(WiFi.localIP());
}

void loop(void) {
  server.handleClient();
}
