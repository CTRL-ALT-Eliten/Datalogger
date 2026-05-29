#include <Arduino.h>
#include "ApiClient.h"
#include "Config.h"

#define SIM_SERIAL Serial2

static void flushInput() {
  while (SIM_SERIAL.available()) {
    SIM_SERIAL.read();
  }
}

// Send en AT-kommando og vent på et terminerings-token ("OK" eller "ERROR").
// Returnerer hele det rå svar.
static String sendAT(const String& cmd, unsigned long timeout = 5000) {
  flushInput();

  SIM_SERIAL.println(cmd);

  unsigned long start = millis();
  String response = "";

  while (millis() - start < timeout) {
    while (SIM_SERIAL.available()) {
      response += (char)SIM_SERIAL.read();
    }
    if (response.indexOf("OK") >= 0 || response.indexOf("ERROR") >= 0) {
      break;
    }
    delay(5);
  }

  return response;
}

// True hvis svaret indeholder "OK".
static bool isOk(const String& resp) {
  return resp.indexOf("OK") >= 0;
}

static int httpAction(int method, unsigned long timeout = 30000) {
  flushInput();

  SIM_SERIAL.println("AT+HTTPACTION=" + String(method));

  unsigned long start = millis();
  String resp = "";

  while (millis() - start < timeout) {
    while (SIM_SERIAL.available()) {
      resp += (char)SIM_SERIAL.read();
    }
    // Vi venter på den URC-linje der kommer ASYNKRONT efter "OK",
    // ikke bare på "OK" selv.
    if (resp.indexOf("+HTTPACTION:") >= 0) {
      // Giv lige modemmet et øjeblik til at skrive resten af linjen.
      delay(50);
      while (SIM_SERIAL.available()) {
        resp += (char)SIM_SERIAL.read();
      }
      break;
    }
    delay(20);
  }

  int idx = resp.indexOf("+HTTPACTION:");
  if (idx < 0) {
    return -1;
  }

  int c1 = resp.indexOf(",", idx);
  int c2 = resp.indexOf(",", c1 + 1);
  if (c1 < 0 || c2 < 0) {
    return -1;
  }

  return resp.substring(c1 + 1, c2).toInt();
}

// Læs response-body fra serveren med AT+HTTPREAD.
static String httpRead(unsigned long timeout = 5000) {
  flushInput();

  SIM_SERIAL.println("AT+HTTPREAD");

  unsigned long start = millis();
  String resp = "";

  while (millis() - start < timeout) {
    while (SIM_SERIAL.available()) {
      resp += (char)SIM_SERIAL.read();
    }
    if (resp.indexOf("OK") >= 0) {
      break;
    }
    delay(5);
  }

  return resp;
}

// Åbn en frisk HTTP-session med de fælles parametre.
// Returnerer true hvis init og URL blev sat OK.
static bool httpBegin(const String& url) {
  // Luk altid en eventuel hængende session først.
  sendAT("AT+HTTPTERM", 2000);

  if (!isOk(sendAT("AT+HTTPINIT"))) {
    Serial.println("HTTPINIT fejlede");
    return false;
  }

  // Brug GPRS-bearer profil 1 (samme som AT+SAPBR=...,1 i jeres opsætning).
  sendAT("AT+HTTPPARA=\"CID\",1");

  if (!isOk(sendAT("AT+HTTPPARA=\"URL\",\"" + url + "\""))) {
    Serial.println("Kunne ikke saette URL");
    return false;
  }

  return true;
}


// -----------------------------------------------------------------------------
// Offentlige metoder
// -----------------------------------------------------------------------------

// Sender måling til API.
bool ApiClient::upload(const String& json) {
  if (!httpBegin(String(API_URL))) {
    sendAT("AT+HTTPTERM", 2000);
    return false;
  }

  sendAT("AT+HTTPPARA=\"CONTENT\",\"application/json\"");

  // API-nøglen sendes som custom header via USERDATA.
  sendAT("AT+HTTPPARA=\"USERDATA\",\"x-api-key: " + String(API_KEY) + "\"");

  // Annoncér hvor mange bytes vi vil sende, og hvor længe modemmet
  // må vente på dem (her 10 sekunder).
  flushInput();
  SIM_SERIAL.println("AT+HTTPDATA=" + String(json.length()) + ",10000");

  // Vent på "DOWNLOAD"-prompten før vi sender selve data.
  unsigned long t = millis();
  String r = "";
  bool gotPrompt = false;
  while (millis() - t < 5000) {
    while (SIM_SERIAL.available()) {
      r += (char)SIM_SERIAL.read();
    }
    if (r.indexOf("DOWNLOAD") >= 0) {
      gotPrompt = true;
      break;
    }
    delay(20);
  }

  if (!gotPrompt) {
    Serial.println("Fik ikke DOWNLOAD-prompt - afbryder upload");
    sendAT("AT+HTTPTERM", 2000);
    return false;
  }

  // Send selve JSON-payloaden. print() (ikke println) – ingen ekstra newline.
  SIM_SERIAL.print(json);

  // Vent på OK efter data er modtaget af modemmet.
  unsigned long td = millis();
  String rd = "";
  while (millis() - td < 5000) {
    while (SIM_SERIAL.available()) {
      rd += (char)SIM_SERIAL.read();
    }
    if (rd.indexOf("OK") >= 0 || rd.indexOf("ERROR") >= 0) {
      break;
    }
    delay(20);
  }

  // Udfør POST.
  int code = httpAction(1);

  // Læs serverens svar (nyttigt til fejlfinding, fx 401 = forkert API-nøgle).
  sendAT("AT+HTTPTERM", 2000);

  Serial.print("HTTP code: ");
  Serial.println(code);

  return code >= 200 && code < 300;
}


// Henter LED-status fra server.
bool ApiClient::getLedStateFromApi() {
  if (!httpBegin(String(LED_STATUS_URL))) {
    sendAT("AT+HTTPTERM", 2000);
    return false;
  }

  sendAT("AT+HTTPPARA=\"USERDATA\",\"x-api-key: " + String(API_KEY) + "\"");

  int code = httpAction(0);  // GET
// Hivs modtaget statuskode ikke er 200, print fejl i serialprint
  if (code != 200) {
    Serial.print("LED GET fejl. HTTP code: ");
    Serial.println(code);
    sendAT("AT+HTTPTERM", 2000);
    return false;
  }

  String payload = httpRead();
  sendAT("AT+HTTPTERM", 2000);

  Serial.print("LED API svar: ");
  Serial.println(payload);

  return payload.indexOf("\"led_on\":true") >= 0 ||
         payload.indexOf("\"state\":\"on\"") >= 0;
}
