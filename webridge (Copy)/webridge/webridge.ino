#include <WiFi.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>


// Motor pins
const int IN1 = 36; 
const int IN2 = 37; 
const int IN3 = 38;
const int IN4 = 39;



// PWM Settings
const int freq = 5000;
const int res = 8; // 8-bit resolution (0-255)

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
  
  // Attach servo

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  WiFi.setSleep(false); 

  Serial.println("");
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  // --- WEB SERIAL SETUP ---
  WebSerial.begin(&server);

  // --- DRIVE ROUTE HANDLER ---
  // Listens for http://<IP>/drive?m1=XX&m2=XX&servo=XX
  // m1 and m2 set both motors to run at same speed
  // servo sets the steering servo angle (0-180, 90 = center)
  server.on("/drive", HTTP_GET, [](AsyncWebServerRequest *request) {
    int speedValue = 0;

    
    // Get speed parameter (both motors run at same speed)
    if (request->hasParam("m1")) {
      speedValue = request->getParam("m1")->value().toInt();
    }
    

    
    // Print to Serial and WebSerial
    Serial.print("Speed: ");
    Serial.print(speedValue);
   
    
    WebSerial.print("Speed: ");
    WebSerial.print(speedValue);
 

    // Control Motor 1 (IN1/IN2)
    if (speedValue >= 0) {
      ledcWrite(IN1, speedValue);
      ledcWrite(IN2, 0);
    } else {
      ledcWrite(IN2, -speedValue);
      ledcWrite(IN1, 0);
    }
    
    // Control Motor 2 (IN3/IN4) - same speed as Motor 1
    if (speedValue >= 0) {
      ledcWrite(IN3, speedValue);
      ledcWrite(IN4, 0);
    } else {
      ledcWrite(IN4, -speedValue);
      ledcWrite(IN3, 0);
    }
    
    request->send(200, "text/plain", "Speed: " + String(speedValue) );
  });

  server.begin();
  Serial.println("\nSUCCESS: Server and Motor Route are Live!");
}


void loop() {
  // Heartbeat to know connection is still alive
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 10000) {
    WebSerial.println("System Status: OK");
    lastMsg = millis();
  }
  yield(); 
}