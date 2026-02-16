void setup() {
  pinMode(9, OUTPUT); // onboard LED is usually GPIO2
}

void loop() {
  digitalWrite(9, HIGH);
  delay(500);
  digitalWrite(9, LOW);
  delay(500);
}
