#include <Wire.h>
#include <math.h>

// MPU-6050 I2C address (0x68 when AD0 is low, 0x69 when AD0 is high)
#define MPU6050_ADDR 0x68

// Register addresses
#define PWR_MGMT_1   0x6B
#define ACCEL_XOUT_H 0x3B

// I2C pins on ESP32-S2
#define SDA_PIN 36
#define SCL_PIN 37

// Complementary filter coefficient (0-1, higher = trust gyro more)
#define ALPHA 0.96

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t rawTemp;

float pitch = 0, roll = 0, yaw = 0;
float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;
unsigned long prevTime = 0;

void writeMPURegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void readMPURaw() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);

  uint8_t count = Wire.requestFrom((uint8_t)MPU6050_ADDR, (uint8_t)14);
  if (count < 14) return;

  uint8_t buf[14];
  for (int i = 0; i < 14; i++) {
    buf[i] = Wire.read();
  }

  ax      = (int16_t)((buf[0]  << 8) | buf[1]);
  ay      = (int16_t)((buf[2]  << 8) | buf[3]);
  az      = (int16_t)((buf[4]  << 8) | buf[5]);
  rawTemp = (int16_t)((buf[6]  << 8) | buf[7]);
  gx      = (int16_t)((buf[8]  << 8) | buf[9]);
  gy      = (int16_t)((buf[10] << 8) | buf[11]);
  gz      = (int16_t)((buf[12] << 8) | buf[13]);
}

void calibrateGyro() {
  Serial.println("Calibrating gyro — keep sensor still...");
  long sumX = 0, sumY = 0, sumZ = 0;
  const int samples = 500;

  for (int i = 0; i < samples; i++) {
    readMPURaw();
    sumX += gx;
    sumY += gy;
    sumZ += gz;
    delay(4);
  }

  gyroBiasX = (sumX / (float)samples) / 131.0;
  gyroBiasY = (sumY / (float)samples) / 131.0;
  gyroBiasZ = (sumZ / (float)samples) / 131.0;

  Serial.print("Gyro bias: X=");
  Serial.print(gyroBiasX, 3);
  Serial.print(" Y=");
  Serial.print(gyroBiasY, 3);
  Serial.print(" Z=");
  Serial.println(gyroBiasZ, 3);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Wire.begin(SDA_PIN, SCL_PIN);

  // Reset the MPU-6050
  writeMPURegister(PWR_MGMT_1, 0x80);
  delay(200);

  // Wake up
  writeMPURegister(PWR_MGMT_1, 0x00);
  delay(100);

  // Verify WHO_AM_I
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x75);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU6050_ADDR, (uint8_t)1);
  uint8_t whoAmI = Wire.read();

  if (whoAmI == 0x68) {
    Serial.println("MPU6050_OK");
  } else {
    Serial.println("MPU6050_FAIL");
  }

  // Calibrate gyro (keep sensor still for ~2 seconds)
  calibrateGyro();

  prevTime = millis();
}

void loop() {
  readMPURaw();

  unsigned long now = millis();
  float dt = (now - prevTime) / 1000.0;
  prevTime = now;

  // Convert raw values to physical units
  float accelX = ax / 16384.0;
  float accelY = ay / 16384.0;
  float accelZ = az / 16384.0;
  float gyroX  = gx / 131.0 - gyroBiasX;  // subtract bias
  float gyroY  = gy / 131.0 - gyroBiasY;
  float gyroZ  = gz / 131.0 - gyroBiasZ;
  float tempC  = rawTemp / 340.0 + 36.53;

  // Dead zone — ignore tiny gyro noise
  if (fabs(gyroX) < 0.5) gyroX = 0;
  if (fabs(gyroY) < 0.5) gyroY = 0;
  if (fabs(gyroZ) < 0.5) gyroZ = 0;

  // Pitch and roll from accelerometer
  float accelPitch = atan2(accelY, sqrt(accelX * accelX + accelZ * accelZ)) * 180.0 / M_PI;
  float accelRoll  = atan2(-accelX, accelZ) * 180.0 / M_PI;

  // Wrap-aware complementary filter
  // Gyro prediction
  float gyroPredictPitch = pitch + gyroX * dt;
  float gyroPredictRoll  = roll  + gyroY * dt;

  // Find shortest-path difference between accel angle and gyro prediction
  float pitchDiff = accelPitch - gyroPredictPitch;
  while (pitchDiff > 180) pitchDiff -= 360;
  while (pitchDiff < -180) pitchDiff += 360;

  float rollDiff = accelRoll - gyroPredictRoll;
  while (rollDiff > 180) rollDiff -= 360;
  while (rollDiff < -180) rollDiff += 360;

  // Apply correction via shortest path
  pitch = gyroPredictPitch + (1.0 - ALPHA) * pitchDiff;
  roll  = gyroPredictRoll  + (1.0 - ALPHA) * rollDiff;
  yaw  += gyroZ * dt;

  // Normalize to [-180, 180]
  while (pitch > 180) pitch -= 360;
  while (pitch < -180) pitch += 360;
  while (roll > 180) roll -= 360;
  while (roll < -180) roll += 360;
  while (yaw > 180) yaw -= 360;
  while (yaw < -180) yaw += 360;

  // Output format: P:pitch,R:roll,Y:yaw,T:temp
  Serial.print("P:");
  Serial.print(pitch, 2);
  Serial.print(",R:");
  Serial.print(roll, 2);
  Serial.print(",Y:");
  Serial.print(yaw, 2);
  Serial.print(",T:");
  Serial.println(tempC, 1);

  delay(20);  // ~50Hz update rate for smooth 3D
}