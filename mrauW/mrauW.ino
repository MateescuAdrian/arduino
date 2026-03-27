#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
#include <WiFi.h>

HardwareSerial
    ArmSerial(1); // Serial1 for Adeept Arm (ESP32-S2 only has UART0 + UART1)

// Pins for H-Bridge
const int IN1 = 36;
const int IN2 = 37;
const int IN3 = 39;
const int IN4 = 38;

// Servo pin - Use GP17 (Physical Pin 12)
const int SERVO_PIN = 40;

// PWM Settings
const int freq = 5000;
const int res = 8;

// const char *ssid = "Machuda";
// const char *password = "Deleanu41";

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
    800; // auto-stop if no command in this time

// Helper function to drive a motor with speed -255 to 255
void setMotor(int p1, int p2, int speed) {
  if (speed >= 0) {
    ledcWrite(p1, speed);
    ledcWrite(p2, 0);
  } else {
    ledcWrite(p1, 0);
    ledcWrite(p2, abs(speed));
  }
}

void setup() {
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  ArmSerial.begin(9600, SERIAL_8N1, 3, 2);

  Serial.begin(115200);
  delay(3000);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  // Configure Motor Pins for PWM
  ledcAttach(IN1, freq, res);
  ledcAttach(IN2, freq, res);
  ledcAttach(IN3, freq, res);
  ledcAttach(IN4, freq, res);

  // Configure Servo Signal using 50Hz PWM
  ledcAttach(SERVO_PIN, 50, 10); // 10-bit resolution (0-1023)

  // 360 continuous servo: 90 = stop, 0 = full CW, 180 = full CCW
  int stopDuty = map(90, 0, 180, 51, 102); // 1.5ms pulse = stop
  ledcWrite(SERVO_PIN, stopDuty);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  WiFi.setSleep(false);

  // Allow cross-origin requests so the HTML page can read responses
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  // Initialize UART for Adeept Arm (RX=GP3, TX=GP2)
  // Using the dedicated UART1 pins on the ESP32-S2 Pico

  WebSerial.begin(&server);
  Serial.println(WiFi.localIP());

  // UPDATED ROUTE: /drive?m1=XX&servo=YY
  server.on("/drive", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response = "Commands Processed: ";
    int baseSpeed = 0;
    int angle = 90;
    // 1. Single Parameter 'm1' controls BOTH motors
    if ((request->hasParam("m1")) || (request->hasParam("servo"))) {
      if (request->hasParam("m1"))
        baseSpeed =
            constrain(request->getParam("m1")->value().toInt(), -255, 255);
      if (request->hasParam("servo"))
        angle = constrain(request->getParam("servo")->value().toInt(), 0, 180);
      // --- DIFFERENTIAL STEERING CALCULATION ---
      // If angle < 90, we turn LEFT. Right motor stays full, Left motor slows
      // down. If angle > 90, we turn RIGHT. Left motor stays full, Right motor
      // slows down.

      float leftFactor = 1.0;
      float rightFactor = 1.0;
      if (angle < 85) { // Turning Left
        // Maps angle 0-85 to a multiplier of 0.0 to 1.0
        leftFactor = angle / 85.0;
      } else if (angle > 95) { // Turning Right
        // Maps angle 180-95 to a multiplier of 0.0 to 1.0
        rightFactor = (180.0 - angle) / 85.0;
      }

      int speedL = baseSpeed;
      int speedR = baseSpeed;
      if (baseSpeed != 0) {
        int absSpeed = abs(baseSpeed);
        int sign = (baseSpeed > 0) ? 1 : -1;
        int delta = max(0, absSpeed - 220);
        int baseFloor = min(absSpeed, 220);
        
        if (angle < 85) {
          speedL = sign * (baseFloor + (int)(delta * leftFactor));
        } else if (angle > 95) {
          speedR = sign * (baseFloor + (int)(delta * rightFactor));
        }
      }

      setMotor(IN1, IN2, speedL);
      setMotor(IN3, IN4, speedR);

      response += "Motors= L " + String(speedL) + " , R" + String(speedR);

      // 2. Continuous rotation servo: 0=full CW, 90=stop, 180=full CCW

      // Map 0-180 to PWM duty (26=1ms full CW, 74=1.5ms stop, 123=2ms full CCW)
      int duty = map(angle, 0, 180, 51, 102);
      ledcWrite(SERVO_PIN, duty);

      // Update tracking state
      currentServoSpeed = angle;
      lastCommandTime = millis();

      response += "Servo=" + String(angle);
    }

    // 3. Robotic Arm Control via UART
    if (request->hasParam("arm_s") && request->hasParam("arm_a")) {
      int arm_s = request->getParam("arm_s")->value().toInt();
      int arm_a =
          constrain(request->getParam("arm_a")->value().toInt(), 0, 180);
      ArmSerial.printf("%d:%d\n", arm_s, arm_a);
      response += " ArmS" + String(arm_s) + "=" + String(arm_a);
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

} // end of setup()

unsigned long lastHeartbeat = 0;

void loop() {
  unsigned long now = millis();
  unsigned long elapsed = now - lastServoUpdate;

  // Safety cap: ignore spikes from async race conditions
  if (elapsed > 100)
    elapsed = 100;

  // Auto-stop servo if no command received recently
  // Disabled per user request to hold steering purely based on commands
  /*
  if (currentServoSpeed != 90 && (now - lastCommandTime) > SERVO_TIMEOUT_MS) {
    currentServoSpeed = 90;
    int stopDuty = map(90, 0, 180, 51, 102);
    ledcWrite(SERVO_PIN, stopDuty);
  }
  */

  if (elapsed > 0 && currentServoSpeed != 90) {
    float speedFactor = (currentServoSpeed - 90) / 90.0;
    totalAngle += speedFactor * DEGREES_PER_MS * elapsed;
  }

  lastServoUpdate = now;

  // DEBUG: Send heartbeat to arm every 5 seconds
  if (now - lastHeartbeat > 5000) {

    ArmSerial.println(
        "9:9"); // harmless command (servo 0 doesn't exist, will be ignored)

    ArmSerial.println(WiFi.localIP());

    lastHeartbeat = now;
    Serial.println("ARM TEST: sent ip address on UART1 (GP2)");
  }

  yield();
}