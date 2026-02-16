#include <Arduino.h>
#include <ESP32Servo.h> //

// Using GP17 (Physical Pin 12) for the Signal wire
const int SERVO_PIN = 17; 

Servo testServo; //

void setup() {
  Serial.begin(115200);
  
  // Attach the servo to pin 17
  testServo.attach(SERVO_PIN);
  
  // Center the servo at the start
  testServo.write(90); 
  
  Serial.println("Servo Test Starting on Pin 17...");
}

void loop() {
  // Sweep from 45 to 135 degrees (Safe range for most linkages)
  Serial.println("Sweeping Right...");
  for (int pos = 45; pos <= 135; pos += 1) {
    testServo.write(pos); //
    delay(15); 
  }

  delay(1000);

  // Sweep back to 45 degrees
  Serial.println("Sweeping Left...");
  for (int pos = 135; pos >= 45; pos -= 1) {
    testServo.write(pos); //
    delay(15);
  }

  delay(1000);
}