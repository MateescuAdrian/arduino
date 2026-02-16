#include <WiFi.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>

// Pins for H-Bridge 
const int IN1 = 36; 
const int IN2 = 37; 
const int IN3 = 38;
const int IN4 = 39; 

// PWM Settings
const int freq = 5000;
const int res = 8; 

const char* ssid     = "DATABURGOS";
const char* password = "257Denierz";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(3000); 

  // Attach PWM to all pins
  ledcAttach(IN1, freq, res);
  ledcAttach(IN2, freq, res);
  ledcAttach(IN3, freq, res);
  ledcAttach(IN4, freq, res);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  WiFi.setSleep(false); 

  WebSerial.begin(&server);
  Serial.println(WiFi.localIP());

  // SINGLE ROUTE FOR BOTH MOTORS
  server.on("/drive", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response = "Commands Processed: ";ss

    // Handle Motor 1 (Parameter 'm1')
    if (request->hasParam("m")) {
      int s1 = constrain(request->getParam("m")->value().toInt(), -255, 255);
      if (s1 >= 0) {
        ledcWrite(IN1, s1);
        ledcWrite(IN2, 0);
        ledcWrite(IN3, s1);
        ledcWrite(IN4, 0);
      } else {
        ledcWrite(IN1, 0);
        ledcWrite(IN2, abs(s1));
        ledcWrite(IN3, 0);
        ledcWrite(IN4, abs(s1));
      }
      response += "M=" + String(s1) + " ";
    }

    // Handle Motor 2 (Parameter 'servo')
    if (request->hasParam("servo")) {
    //  int s2 = constrain(request->getParam("servo")->value().toInt(), -255, 255);
    //... do servo magic
     // response += "servo=" + String(s2);
    }

    WebSerial.println(response);
    request->send(200, "text/plain", response);
  });

  server.begin();
  Serial.println("\nSUCCESS: Combined Drive Route Live!");
}

void loop() {
  yield(); 
}