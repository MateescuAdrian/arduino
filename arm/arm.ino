/***********************************************************
File name: UARTOledArmControl.ino
Description: Control the Adeept robotic arm via UART with absolute angles
             and display the current servo positions on the I2C OLED display.

ESP32 should send UART commands in format: "ServoNumber:Angle\n"
Example: "1:90\n" sets Servo 1 (Base) to 90 degrees.
Valid ServoNumbers: 1 to 5.
Valid Angles: 0 to 180.

Author: AI Assistant
Date: 2026/03/15
***********************************************************/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <Wire.h>

// OLED Configuration
#define OLED_RESET -1 // No reset pin
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

// Servo Configuration
// Based on AdeeptArmRobot.ino pin mappings
const int servoPins[5] = {9, 6, 5, 3, 11};
Servo servos[5];
int currentAngles[5] = {90, 90, 90, 90, 90}; // Default positions
bool updateDisplay = true;
String lastRx = "(none)"; // Debug: last raw data received
int rxCount = 0;          // Debug: total messages received

void setup() {

  // 2. Initialize the OLED Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.setTextColor(WHITE);
  display.clearDisplay();

  // 3. Initialize Servos
  for (int i = 0; i < 5; i++) {
    pinMode(servoPins[i], OUTPUT);
    servos[i].attach(servoPins[i]);
    servos[i].write(currentAngles[i]);
    delay(20);
  }

  // Draw initial OLED Screen
  drawScreen();
  display.print("serial started");
  display.display();
  delay(2000);
  // 1. Initialize Serial - receives from Bluetooth/WiFi header via DPDT switch
  Serial.begin(9600);
  display.print("serial started");
  drawScreen();
  display.display();
}

void loop() {
  // Check if serial data is available
  if (Serial.available() > 0) {
    // Read the string until newline '\n'
    String command = Serial.readStringUntil('\n');
    command.trim();   // Remove any extra spaces/carriage returns
    lastRx = command; // Store raw for OLED debug
    rxCount++;
    updateDisplay = true;

    // Process the command if it contains a colon ':'
    int colonPos = command.indexOf(':');
    if (colonPos > 0) {
      // Parse Servo Index (1 to 5) and Angle (0 to 180)
      int servoNum = command.substring(0, colonPos).toInt();
      int angle = command.substring(colonPos + 1).toInt();

      // Constrain inputs for safety
      if (servoNum >= 1 && servoNum <= 5) {
        int index = servoNum - 1;         // Array is 0-indexed (0 to 4)
        angle = constrain(angle, 0, 180); // Keep angles within physical limits

        // Move the servo if angle has changed
        if (currentAngles[index] != angle) {
          servos[index].write(angle);
          currentAngles[index] = angle;
          updateDisplay = true; // Mark display for update
        }
      }
    }
  }

  // Update OLED display if values changed
  if (updateDisplay) {
    drawScreen();
    updateDisplay = false;
  }
}

// Function to draw text UI on the OLED screen
void drawScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  // Title
  display.println("ASTRO NUTS Robot");
  display.drawLine(0, 10, 128, 10, WHITE);

  // Display current angles
  display.setCursor(0, 15);
  display.print("S1:");
  display.print(currentAngles[0]);
  display.print(" S2:");
  display.println(currentAngles[1]);
  display.print("S3:");
  display.print(currentAngles[2]);
  display.print(" S4:");
  display.println(currentAngles[3]);
  display.print("S5:");
  display.println(currentAngles[4]);

  // Debug: show raw received data
  display.drawLine(0, 46, 128, 46, WHITE);
  display.setCursor(0, 50);
  display.print("RX#");
  display.print(rxCount);
  display.print(": ");
  display.println(lastRx);

  display.display();
}
