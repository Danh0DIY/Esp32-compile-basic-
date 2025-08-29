#include <WiFi.h>
#include "esp_wifi.h"

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_AP);

  // Danh sách tên WiFi để troll
  const char* ssids[] = {
    "FreeWifi_100%",
    "Mat_Khau_123456",
    "Wifi_Nha_Hang_Xom",
    "Hack_Phone_OK",
    "Virus_Phone",
    "ESP32_Troll_Mode",
    "Anh_Yeu_Em",
    "TikTok_Free",
    "Zalo_Pro",
    "Game_LienQuan_Lag"
  };

  // Tạo nhiều AP ảo
  for (int i = 0; i < 10; i++) {
    wifi_config_t config;
    memset(&config, 0, sizeof(wifi_config_t));
    strcpy((char*)config.ap.ssid, ssids[i]);  // copy tên WiFi
    config.ap.ssid_len = strlen(ssids[i]);
    config.ap.channel = 1;
    config.ap.authmode = WIFI_AUTH_OPEN;
    config.ap.max_connection = 1;
    config.ap.beacon_interval = 100;

    esp_wifi_set_config(WIFI_IF_AP, &config);
    esp_wifi_start();
    delay(200);
  }
}

void loop() {
  // không cần làm gì thêm, WiFi sẽ hiện ra
}