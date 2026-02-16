#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>

const char* ssid     = "DATABURGOS";
const char* password = "257Denierz";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(3000); 
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  WiFi.setSleep(false); 

  // --- WEB SERIAL SETUP ---
  WebSerial.begin(&server);

  // --- NEW: URL ROUTE HANDLER ---
  // This listens for http://192.168.0.54/motor1?speed=XX
  server.on("/motor1", HTTP_GET, [](AsyncWebServerRequest *request) {
    String message;
    
    // Check if the "speed" parameter exists in the URL
    if (request->hasParam("speed")) {
      message = request->getParam("speed")->value();
      
      // Print to BOTH Serial and WebSerial so you can see it working
      Serial.print("Motor Speed Set to: ");
      Serial.println(message);
      WebSerial.print("Command Received! New Speed: ");
      WebSerial.println(message);
      
      request->send(200, "text/plain", "Speed set to " + message);
    } else {
      request->send(400, "text/plain", "Error: No speed parameter found.");
    }
  });

  server.begin();
  Serial.println("\nSUCCESS: Server and Motor Route are Live!");
}

void loop() {
  // We keep the heartbeat just to know the connection is still alive
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 10000) { // Increased to 10s to clear up the log
    WebSerial.println("System Status: OK");
    lastMsg = millis();
  }
  yield(); 
}