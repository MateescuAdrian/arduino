#include <ESP32Servo.h>

Servo myServo;

// Define the pin - GPIO 18 is common, but you can use almost any digital pin
const int servoPin = 36; 

void setup() {
  // ESP32 requires specific timer allocation for PWM
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  myServo.setPeriodHertz(50);           // Standard 50Hz servo frequency
  myServo.attach(servoPin, 500, 2400);  // Attaches pin with min/max pulse widths

  // Move to 90 degrees
  myServo.write(90);
  delay(1000);
  myServo.write(0);
  delay(1000);
  myServo.write(0);

}

void loop() {
  myServo.write(0);
  delay(5000);
  myServo.write(180);
  delay(1000);
  myServo.write(90);
  delay(5000);
}