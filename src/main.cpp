#include <Arduino.h>
#include "USB.h"
#include "USBHIDMouse.h"

USBHIDMouse Mouse;
USBHID HID;

#define BTN 0  // chân nút bấm

void setup() {
  pinMode(BTN, INPUT_PULLUP);

  HID.begin();
  Mouse.begin();
  USB.begin();
}

void loop() {
  if (digitalRead(BTN) == LOW) {
    // Di chuyển chuột
    Mouse.move(616, 586); 
    delay(200);

    // Click chuột trái
    Mouse.click();
    delay(500);
  }
}