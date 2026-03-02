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

// Servo pin - Use GP17 (Physical Pin 12)
const int SERVO_PIN = 17; 

// PWM Settings
const int freq = 5000;
const int res = 8; 

const char* ssid     = "DATABURGOS";
const char* password = "257Denierz";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(3000); 

  // Configure Motor Pins for PWM
  ledcAttach(IN1, freq, res);
  ledcAttach(IN2, freq, res);
  ledcAttach(IN3, freq, res);
  ledcAttach(IN4, freq, res);
  
  // Configure Servo Signal using 50Hz for standard servos
  ledcAttach(SERVO_PIN, 50, 10); // 10-bit resolution (0-1023)
  
  // Initial center position (90 degrees)
  int centerDuty = map(90, 0, 180, 26, 123);
  ledcWrite(SERVO_PIN, centerDuty);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  WiFi.setSleep(false); 

  WebSerial.begin(&server);
  Serial.println(WiFi.localIP());

  // UPDATED ROUTE: /drive?m1=XX&servo=YY
  server.on("/drive", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response = "Commands Processed: ";

    // 1. Single Parameter 'm1' controls BOTH motors
    if (request->hasParam("m1")) {
      int speedValue = constrain(request->getParam("m1")->value().toInt(), -255, 255);
      
      if (speedValue >= 0) {
        // Forward: Both motors use high signal on IN1 and IN3
        ledcWrite(IN1, speedValue);
        ledcWrite(IN2, 0);
        ledcWrite(IN3, speedValue);
        ledcWrite(IN4, 0);
      } else {
        // Backward: Both motors use high signal on IN2 and IN4
        ledcWrite(IN1, 0);
        ledcWrite(IN2, abs(speedValue));
        ledcWrite(IN3, 0);
        ledcWrite(IN4, abs(speedValue));
      }
      response += "Motors=" + String(speedValue) + " ";
    }

    // 2. Add steering servo control
    if (request->hasParam("servo")) {
      int servoAngle = constrain(request->getParam("servo")->value().toInt(), 0, 180);
      
      // Map 0-180 angle to standard 50Hz servo duty cycle
      int duty = map(servoAngle, 0, 180, 26, 123);
      ledcWrite(SERVO_PIN, duty);
      
      response += "Servo=" + String(servoAngle);
    }

    WebSerial.println(response);
    request->send(200, "text/plain", response);
  });

  server.begin();
  Serial.println("\nSUCCESS: All-in-One Route Live!");
}

void loop() {
  yield(); 
}