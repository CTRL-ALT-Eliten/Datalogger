#include "ApiClient.h"
#include "Config.h"
#include <HTTPClient.h>
#include <WiFi.h>

// Upload funktion
bool ApiClient::upload(const String& json) {
    HTTPClient http;

    http.begin(API_URL);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-api-key", API_KEY);

    int code = http.POST(json);

    Serial.print("HTTP code: ");
    Serial.println(code);

    String response = http.getString();
    Serial.println(response);

    http.end();

    return code >= 200 && code < 300;
}

// LED-status funktion som metode
bool ApiClient::getLedStateFromApi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Kan ikke hente LED-status: WiFi ikke forbundet");
        return false;
    }

    HTTPClient http;
    http.begin(LED_STATUS_URL); // URL fra Config.h

    int httpCode = http.GET();
    if (httpCode != 200) {
        Serial.print("LED GET fejl. HTTP code: ");
        Serial.println(httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    Serial.print("LED API svar: ");
    Serial.println(payload);

    if (payload.indexOf("\"led_on\":true") >= 0 || payload.indexOf("\"state\":\"on\"") >= 0) {
        return true;
    }

    return false;
}
