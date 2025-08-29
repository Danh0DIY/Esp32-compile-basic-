#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

const char *ssid = "FreeWifi";   // Tên WiFi troll
const char *password = "";

WebServer server(80);
DNSServer dns;

void handleRoot() {
  // Captive portal trả về redirect thẳng sang app
  String page = "<!DOCTYPE html><html><head>";
  page += "<meta http-equiv='refresh' content='0;url=intent://youtube.com/#Intent;scheme=https;package=com.google.android.youtube;end'>";
  page += "</head><body></body></html>";
  server.send(200, "text/html", page);
}

void setup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  dns.start(53, "*", WiFi.softAPIP());   // bắt toàn bộ DNS

  server.onNotFound(handleRoot);
  server.begin();
}

void loop() {
  dns.processNextRequest();
  server.handleClient();
}