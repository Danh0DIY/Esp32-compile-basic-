#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 Touch Sensor Test");
}

void loop() {
  int touchValue = touchRead(T4);  // GPIO13 = Touch pin T4
  Serial.println(touchValue);
  delay(500);
}