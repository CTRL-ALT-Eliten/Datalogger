#include "Measurement.h"
#include <ArduinoJson.h>
#include <math.h>

Measurement::Measurement()
  : loggerIdEx(0),
    tempInside(NAN),
    humidityInside(NAN),
    co2(-1),
    tempOutside(NAN),
    pressureOutside(NAN),
    condensationRisk(false),
    light(0),
    sound(0) {
}

float Measurement::round1(float value) const {
  return round(value * 10.0) / 10.0;
}

String Measurement::toJson() const {
  StaticJsonDocument<384> doc;

  doc["device_id"] = deviceId;
  doc["logger_id_ex"] = loggerIdEx;
  doc["rtc_timestamp"] = rtcTimestamp;

  doc["temp_inside"] = round1(tempInside);
  doc["humidity_inside"] = round1(humidityInside);
  doc["co2"] = co2;

  doc["temp_outside"] = round1(tempOutside);
  doc["tryk"] = round1(pressureOutside);

  // Database-kolonnen hedder "condensation", så JSON-feltet skal hedde det samme.
  // Værdien er vores condensation_risk: 0 = ingen risiko, 1 = risiko.
  doc["condensation"] = condensationRisk ? 1 : 0;

  doc["light"] = light;
  doc["sound"] = sound;

  String json;
  serializeJson(doc, json);
  return json;
}
