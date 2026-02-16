#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Create an Adafruit_MPU6050 object called 'mpu'
Adafruit_MPU6050 mpu;

void setup(void) {
  // Initialize serial communication for output
  Serial.begin(115200);
  
  // Wait a moment for the Serial Monitor to open
  while (!Serial); 
  Serial.println("MPU6050 Test");
  
  // Try to initialize the MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip, check wiring!");
    while (1) {
      delay(10); // Loop indefinitely if initialization fails
    }
  }
  Serial.println("MPU6050 Found and Initialized!");
  
  // Optional: Configure sensor ranges (defaults are often fine)
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  Serial.println("---------------------------------------");
  delay(100);
}

void loop() {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values to the Serial Monitor */
  
  // 1. Acceleration (linear movement)
  Serial.print("Accel X:");
  Serial.print(a.acceleration.x, 2);
  Serial.print(" Y:");
  Serial.print(a.acceleration.y, 2);
  Serial.print(" Z:");
  Serial.print(a.acceleration.z, 2);
  Serial.print(" m/s^2 | ");

  // 2. Rotation (angular velocity)
  Serial.print("Rot X:");
  Serial.print(g.gyro.x, 2);
  Serial.print(" Y:");
  Serial.print(g.gyro.y, 2);
  Serial.print(" Z:");
  Serial.print(g.gyro.z, 2);
  Serial.print(" rad/s | ");
  
  // 3. Temperature
  Serial.print("Temp: ");
  Serial.print(temp.temperature, 2);
  Serial.println(" degC");

  delay(500); // Read every half second
}