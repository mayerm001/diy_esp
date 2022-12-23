//
// DIY_ESP_Simple (c) Mayer Mihály 2022
// Generic ESP8266, ESP-01, Wemos D1 Mini, NodeMCU, ESP32 ...
// Relay ON = digitalWrite(RELAY_PIN, HIGH)
// Relay OFF = digitalWrite(RELAY_PIN, LOW)
//
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// IMPORTANT:
// Replace the variables below with the values ​​that suit for you!
// -------------------------------------
const int RELAY_PIN       = D7;          
const String RELAY_DEVICE = "FC:F5:C4:89:A6:EB";
const String routername   = "<>";
const String routerpwd    = "<>";
IPAddress ip(192, 168, 0, 82);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8,8,8,8);
// -----------------------------------

const String COMMAND_SWITCH = "zeroconf/switch";
const String COMMAND_INFO = "zeroconf/info";
ESP8266WebServer server(8266);

void setup() {

  Serial.begin(115200);
  WiFi.config(ip, gateway, subnet,dns);
  WiFi.begin(routername, routerpwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.on("/" + COMMAND_SWITCH, HTTP_POST, handleSwitch);
  server.on("/" + COMMAND_INFO, HTTP_POST, handleInfo);
  server.on("/", HTTP_POST, handleRoot);  
  server.onNotFound(handleNotFound);

  server.begin();
  pinMode(RELAY_PIN,OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  Serial.print("ESP Board MAC Address (deviceid):  ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  server.handleClient();
}

String createJsonMessage(String mode)
{
  StaticJsonDocument<256> doc;
  String output;
  
  JsonObject root = doc.to<JsonObject>();  
  if (mode == "") {
       root["deviceid"] = RELAY_DEVICE;   
      JsonObject data = root.createNestedObject("data");
      data["switch"] = "n/a";
  } else {
      root["deviceid"] = RELAY_DEVICE;
      JsonObject data = root.createNestedObject("data");
      data["switch"] = mode;
  }
  serializeJson(root, output);
  return output;  
}

void handleSwitch() {

  DynamicJsonDocument document(256);
  String result;

  String state = (digitalRead(RELAY_PIN) == HIGH) ? "on": "off";
  if (server.hasArg("plain") == true) 
  { 
      String json = server.arg("plain");
      Serial.println("Input: " + json);
      deserializeJson(document, json);
      JsonObject root = document.as<JsonObject>();
      String request_device = root["deviceid"].as<String>();
      if (request_device == RELAY_DEVICE)
      {
        String request_command = root["data"]["switch"].as<String>();
        if (request_command == "on") 
        {
            digitalWrite(RELAY_PIN, HIGH);
            result = createJsonMessage("on");
        } else if (request_command == "off")
        {
            digitalWrite(RELAY_PIN, LOW);
            result = createJsonMessage("off");       
        } 
      } else
            result = createJsonMessage(state);
  } else
    result = createJsonMessage("");
  server.send(200, "text/plain", result);
  Serial.println("Output: " + result);     
}

void handleInfo() {

  DynamicJsonDocument document(256);
  String result;

  if (server.hasArg("plain") == true) {
      String json = server.arg("plain");
      Serial.println("Input: " + json);
      deserializeJson(document, json);
      JsonObject root = document.as<JsonObject>();
      String request_device = root["deviceid"].as<String>();
      if (request_device == RELAY_DEVICE)
      {
        String state = (digitalRead(RELAY_PIN) == HIGH) ? "on": "off";
        result = createJsonMessage(state);
      } else
        result = createJsonMessage("");
  } else
     result = createJsonMessage(""); 
  server.send(200, "text/plain", result);
  Serial.println("Output: " + result); 
}

void handleRoot() {
    server.send(200, "text/plain", createJsonMessage(""));
}

void handleNotFound() {
  server.send(404, "text/plain", createJsonMessage(""));
}
