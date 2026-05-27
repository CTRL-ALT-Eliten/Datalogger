#include <Arduino.h>
#include "WiFiManagerCustom.h"
#include "Config.h"

#define SIM_SERIAL Serial2

static String simSendAT(const String& cmd, unsigned long timeout = 3000) {
  SIM_SERIAL.println(cmd);
  unsigned long start = millis();
  String response = "";
  while (millis() - start < timeout) {
    while (SIM_SERIAL.available()) {
      response += (char)SIM_SERIAL.read();
    }
    if (response.indexOf("OK") >= 0 || response.indexOf("ERROR") >= 0) break;
    delay(10);
  }
  return response;
}

// Opretter GPRS-forbindelse.
void WiFiManagerCustom::connect() {
  SIM_SERIAL.begin(9600, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
  delay(3000); // vent på SIM800L boot

  Serial.println("Starter SIM800L...");

  // Test AT-kommunikation
  for (int i = 0; i < 10; i++) {
    if (simSendAT("AT").indexOf("OK") >= 0) break;
    delay(500);
  }

  simSendAT("ATE0"); // slå echo fra

  // Vent på netværksregistrering (max 20 sek)
  Serial.print("Venter på GSM-netværk");
  for (int i = 0; i < 20; i++) {
    String r = simSendAT("AT+CREG?");
    if (r.indexOf(",1") >= 0 || r.indexOf(",5") >= 0) {
      Serial.println(" OK");
      break;
    }
    Serial.print(".");
    delay(1000);
  }

  // Luk eventuel gammel GPRS-session
  simSendAT("AT+SAPBR=0,1", 5000);
  delay(500);

  // Opsæt GPRS bearer
  simSendAT("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  simSendAT("AT+SAPBR=3,1,\"APN\",\"" + String(SIM_APN) + "\"");

  if (strlen(SIM_APN_USER) > 0) {
    simSendAT("AT+SAPBR=3,1,\"USER\",\"" + String(SIM_APN_USER) + "\"");
    simSendAT("AT+SAPBR=3,1,\"PWD\",\"" + String(SIM_APN_PASS) + "\"");
  }

  simSendAT("AT+SAPBR=1,1", 10000); // åbn bearer
  delay(1000);
  simSendAT("AT+SAPBR=1,1", 10000); // åbn bearer
delay(1000);

// Sæt faste DNS-servere til GPRS-forbindelsen.
simSendAT("AT+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\""); // Google DNS
delay(500);

  String ipResp = simSendAT("AT+SAPBR=2,1");
  if (ipResp.indexOf("+SAPBR: 1,1") >= 0) {
    _connected = true;
    Serial.println("SIM800L: GPRS forbundet");
    Serial.println(ipResp);
  } else {
    _connected = false;
    Serial.println("SIM800L: Ingen GPRS-forbindelse");
  }
}

bool WiFiManagerCustom::isConnected() {
  // Spørg modemmet direkte – opdager droppede forbindelser
  SIM_SERIAL.println("AT+SAPBR=2,1");
  unsigned long t = millis();
  String r = "";
  while (millis() - t < 2000) {
    while (SIM_SERIAL.available()) r += (char)SIM_SERIAL.read();
    if (r.indexOf("OK") >= 0 || r.indexOf("ERROR") >= 0) break;
    delay(10);
  }
  _connected = r.indexOf("+SAPBR: 1,1") >= 0;
  return _connected;
}
