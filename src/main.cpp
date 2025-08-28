#include <WiFi.h>
#include "esp_wifi.h"

void sendBeacon(const char *ssid);

String targetSSID = "";
const int numClones = 15;   // số mạng ảo spam ra

// Khung beacon giả
uint8_t beaconPacket[128] = {
  0x80, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,   // Destination (broadcast)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Source MAC
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // BSSID
  0x00, 0x00,
  0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00,
  0x31, 0x04
};

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.println("Đang quét WiFi xung quanh...");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("Không tìm thấy WiFi nào.");
    while (true) delay(1000);
  }

  // In danh sách để chọn
  for (int i = 0; i < n; i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.println(WiFi.SSID(i));
  }

  Serial.println("Nhập số thứ tự mạng WiFi bạn muốn spam:");
  while (Serial.available() == 0) {}   // đợi nhập
  int choice = Serial.parseInt();
  if (choice >= 0 && choice < n) {
    targetSSID = WiFi.SSID(choice);
    Serial.print("Đã chọn: ");
    Serial.println(targetSSID);
  } else {
    Serial.println("Lỗi chọn mạng!");
    while (true) delay(1000);
  }

  esp_wifi_set_promiscuous(true);
  Serial.println("Bắt đầu spam...");
}

void loop() {
  if (targetSSID.length() > 0) {
    for (int i = 0; i < numClones; i++) {
      sendBeacon(targetSSID.c_str());
      delay(20);
    }
  }
}

void sendBeacon(const char *ssid) {
  uint8_t packet[128];
  memcpy(packet, beaconPacket, sizeof(beaconPacket));

  // MAC giả random
  for (int i = 10; i < 16; i++) {
    uint8_t r = random(256);
    packet[i] = r;
    packet[i + 6] = r;
  }

  // Thêm SSID
  int ssidLen = strlen(ssid);
  packet[37] = 0x00;         // SSID tag
  packet[38] = ssidLen;
  memcpy(&packet[39], ssid, ssidLen);

  esp_wifi_80211_tx(WIFI_IF_STA, packet, 39 + ssidLen, false);
}