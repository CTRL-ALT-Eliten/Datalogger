#include "ApiClient.h"
#include "Config.h"
#include <HTTPClient.h>

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
