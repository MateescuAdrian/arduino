#include <Wire.h>

const uint8_t ADDR = 0x68;           // your scan found 0x68
const uint8_t REG_WHO_AM_I = 0x75;
const uint8_t REG_PWR_MGMT_1 = 0x6B;
const uint8_t REG_SIGNAL_PATH_RESET = 0x68;

uint8_t i2cRead8(uint8_t reg) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDR, (uint8_t)1);
  return Wire.available() ? Wire.read() : 0xFF;
}

void i2cWrite8(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

int16_t read16(uint8_t reg) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDR, (uint8_t)2);
  int16_t hi = Wire.read();
  int16_t lo = Wire.read();
  return (hi << 8) | lo;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nMPU6050 WHO_AM_I + wake test (Uno)");

  Wire.begin();            // Uno: SDA=A4, SCL=A5
  Wire.setClock(100000);

  // Read WHO_AM_I
  uint8_t who = i2cRead8(REG_WHO_AM_I);
  Serial.print("WHO_AM_I = 0x"); if (who < 16) Serial.print('0'); Serial.println(who, HEX);

  // Hard reset then wake
  Serial.println("Resetting + waking...");
  i2cWrite8(REG_PWR_MGMT_1, 0x80);  // device reset
  delay(100);
  i2cWrite8(REG_SIGNAL_PATH_RESET, 0x07); // reset signal paths
  delay(100);
  i2cWrite8(REG_PWR_MGMT_1, 0x01);  // clock source = X gyro, exit sleep
  delay(100);

  // Quick config (defaults are fine; just ensure awake)
  // Try a read of accel/gyro
  int16_t ax = read16(0x3B);
  int16_t ay = read16(0x3D);
  int16_t az = read16(0x3F);
  int16_t gx = read16(0x43);
  int16_t gy = read16(0x45);
  int16_t gz = read16(0x47);

  Serial.print("Raw Accel: "); Serial.print(ax); Serial.print(", ");
  Serial.print(ay); Serial.print(", "); Serial.println(az);
  Serial.print("Raw Gyro : "); Serial.print(gx); Serial.print(", ");
  Serial.print(gy); Serial.print(", "); Serial.println(gz);
}

void loop() { }
