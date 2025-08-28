// ESP32 - liên tục quét WiFi và in danh sách SSID ra Serial (spam)
#include <WiFi.h>

unsigned long lastScan = 0;
const unsigned long SCAN_INTERVAL_MS = 2000; // 2 giây giữa 2 lần quét

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("ESP32 WiFi spam scanner starting...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // đảm bảo ở chế độ station, không kết nối AP
  delay(100);
}

void loop() {
  unsigned long now = millis();
  if (now - lastScan >= SCAN_INTERVAL_MS) {
    lastScan = now;

    Serial.println();
    Serial.println("=== Bắt đầu quét WiFi ===");

    int n = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);
    if (n == 0) {
      Serial.println("Không tìm thấy mạng nào.");
    } else {
      for (int i = 0; i < n; ++i) {
        // in thông tin chi tiết; thay đổi format nếu muốn chỉ in tên
        Serial.print(i + 1);
        Serial.print(". SSID: ");
        Serial.print(WiFi.SSID(i));
        Serial.print("  | RSSI: ");
        Serial.print(WiFi.RSSI(i));
        Serial.print(" dBm | BSSID: ");
        Serial.print(WiFi.BSSIDstr(i));
        Serial.print(" | Ch: ");
        Serial.print(WiFi.channel(i));
        Serial.println();
      }
    }
    WiFi.scanDelete(); // giải phóng bộ nhớ scan
    Serial.println("=== Kết thúc quét ===");
  }

  // Nếu muốn spam nhanh hơn, có thể giảm SCAN_INTERVAL_MS; tuy nhiên quá nhanh có thể gây tải cao
  // Thêm một chút delay nhỏ tránh chiếm CPU liên tục
  delay(10);
}