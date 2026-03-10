#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
#include <WiFi.h>

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

const char *ssid = "DATABURGOS";
const char *password = "257Denierz";

AsyncWebServer server(80);

// --- Servo angle tracking (estimated, no encoder) ---
const float SERVO_MAX_RPM = 60.0; // calibrate to your servo
const float DEGREES_PER_MS = (SERVO_MAX_RPM * 360.0) / 60000.0;

float totalAngle = 0.0;     // cumulative angle (never resets at 360)
int currentServoSpeed = 90; // current servo command (90 = stop)
unsigned long lastServoUpdate = 0;
unsigned long lastCommandTime = 0; // when the last /drive command arrived
const unsigned long SERVO_TIMEOUT_MS =
    200; // auto-stop if no command in this time

void setup() {
  Serial.begin(115200);
  delay(3000);

  // Configure Motor Pins for PWM
  ledcAttach(IN1, freq, res);
  ledcAttach(IN2, freq, res);
  ledcAttach(IN3, freq, res);
  ledcAttach(IN4, freq, res);

  // Configure Servo Signal using 50Hz PWM
  ledcAttach(SERVO_PIN, 50, 10); // 10-bit resolution (0-1023)

  // 360 continuous servo: 90 = stop, 0 = full CW, 180 = full CCW
  int stopDuty = map(90, 0, 180, 26, 123); // ~74 = 1.5ms pulse = stop
  ledcWrite(SERVO_PIN, stopDuty);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  WiFi.setSleep(false);

  // Allow cross-origin requests so the HTML page can read responses
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  WebSerial.begin(&server);
  Serial.println(WiFi.localIP());

  // UPDATED ROUTE: /drive?m1=XX&servo=YY
  server.on("/drive", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response = "Commands Processed: ";

    // 1. Single Parameter 'm1' controls BOTH motors
    if (request->hasParam("m1")) {
      int speedValue =
          constrain(request->getParam("m1")->value().toInt(), -255, 255);

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

    // 2. Continuous rotation servo: 0=full CW, 90=stop, 180=full CCW
    if (request->hasParam("servo")) {
      int servoSpeed =
          constrain(request->getParam("servo")->value().toInt(), 0, 180);

      // Map 0-180 to PWM duty (26=1ms full CW, 74=1.5ms stop, 123=2ms full CCW)
      int duty = map(servoSpeed, 0, 180, 51, 102);
      ledcWrite(SERVO_PIN, duty);

      // Update tracking state
      currentServoSpeed = servoSpeed;
      lastCommandTime = millis();

      response += "Servo=" + String(servoSpeed);
    }

    WebSerial.println(response);
    request->send(200, "text/plain", response);
  });

  // Endpoint to get servo angle
  server.on("/servo_status", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("reset")) {
      totalAngle = 0.0;
    }

    String json = "{";
    json += "\"angle\":" + String(totalAngle, 1) + ",";
    json += "\"speed\":" + String(currentServoSpeed);
    json += "}";

    request->send(200, "application/json", json);
  });

  server.begin();
  Serial.println("\nSUCCESS: All-in-One Route Live!");
}

void loop() {
  unsigned long now = millis();
  unsigned long elapsed = now - lastServoUpdate;

  // Safety cap: ignore spikes from async race conditions
  if (elapsed > 100)
    elapsed = 100;

  // Auto-stop servo if no command received recently
  if (currentServoSpeed != 90 && (now - lastCommandTime) > SERVO_TIMEOUT_MS) {
    currentServoSpeed = 90;
    int stopDuty = map(90, 0, 180, 26, 123);
    ledcWrite(SERVO_PIN, stopDuty);
  }

  if (elapsed > 0 && currentServoSpeed != 90) {
    float speedFactor = (currentServoSpeed - 90) / 90.0;
    totalAngle += speedFactor * DEGREES_PER_MS * elapsed;
  }

  lastServoUpdate = now;
  yield();
}