// super_simple_vib_test.ino
// Vibrator motor test on GPIO 8 (ESP32-S2)

#define VIB_PIN 8

void setup() {
  pinMode(VIB_PIN, OUTPUT);
}

void loop() {
  digitalWrite(VIB_PIN, HIGH);  // motor ON
  delay(1000);

  digitalWrite(VIB_PIN, LOW);   // motor OFF
  delay(1000);
}
