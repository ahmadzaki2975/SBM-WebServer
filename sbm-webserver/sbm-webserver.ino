#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_BMP085.h>

// SSID depends on network used
const char *ssid = "aufarhmn";
const char *password = "infopulang";

// DHT Setup
#define DHTPIN 13
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Barometer Setup
Adafruit_BMP085 bmp;

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Catch error if BMP085 is not connected
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1);
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println(WiFi.localIP());
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
  initWebSocket();

  // Route: "/"
  // Method: GET
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Request received");
    String htmlContent = "<body><main><h1>SBM API</h1><h2>Dibuat oleh:</h2><ul><li>Ahmad Zaki Akmal</li><li>Aufa Nasywa Rahman</li><li>Agustinus Angelo Christian Fernando</li><li>Difta Fitrahul Qihaj</li><li>Giga Hidjrika Aura Adkhy</li></ul></main></body>";
    request->send(200, "text/html", htmlContent);
  });
}

void loop() {
  ws.cleanupClients();

  // Barometer readings
  float pressure = bmp.readPressure();
  float altitude = bmp.readAltitude();

  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  if(isnan(temp)) {
    temp = 0;
  }

  if(isnan(humidity)) {
    humidity = 0;
  }

  int analogValue = analogRead(32);
  // Rescale to potentiometer's voltage (from 0V to 3.3V):
  float voltage = floatMap(analogValue, 0, 4095, 0, 3.3);

  String jsonString = "";
  jsonString += "{";
  jsonString += "\"temperature\" : ";
  jsonString += temp;
  jsonString += ",";
  jsonString += "\"humidity\" : ";
  jsonString += humidity;
  jsonString += ",";
  jsonString += "\"analog\":";
  jsonString += analogValue;
  jsonString += ",";
  jsonString += "\"voltage\":";
  jsonString += voltage;
  jsonString += "\"pressure\":";
  jsonString += pressure;
  jsonString += "\"altitude\":";
  jsonString += altitude;
  jsonString += "}";

  ws.textAll(jsonString);
  Serial.println(jsonString);
  delay(1000);
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
  }
}

float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}