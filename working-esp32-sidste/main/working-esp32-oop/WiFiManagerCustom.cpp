#include "WiFiManagerCustom.h"
#include "Config.h"
#include <WiFi.h>

void WiFiManagerCustom::connect() {
  Serial.print("Forbinder til WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi forbundet");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Ingen WiFi");
  }
}

bool WiFiManagerCustom::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}
