#include "SensorManager.h"
#include "Config.h"
#include <math.h>

SensorManager::SensorManager()
  : ens160(ENS160_I2CADDR_1) {
}

void SensorManager::begin() {
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!rtc.begin()) {
    Serial.println("RTC ikke fundet");
  } else {
    Serial.println("RTC OK");
  }

  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 ikke fundet på 0x76. Prøver 0x77...");
    if (!bmp.begin(0x77)) {
      Serial.println("BMP280 ikke fundet");
    } else {
      Serial.println("BMP280 OK på 0x77");
    }
  } else {
    Serial.println("BMP280 OK på 0x76");
  }

  if (!aht.begin()) {
    Serial.println("AHT20/AHT21 ikke fundet");
  } else {
    Serial.println("AHT OK");
  }

  if (!ens160.begin()) {
    Serial.println("ENS160 ikke fundet");
  } else {
    ens160.setMode(ENS160_OPMODE_STD);
    Serial.println("ENS160 OK");
  }
}

Measurement SensorManager::readMeasurement() {
  Measurement measurement;

  measurement.deviceId = DEVICE_ID;
  measurement.loggerIdEx = LOGGER_ID_EX;

  measurement.light = analogRead(LDR_PIN);
  measurement.sound = analogRead(MIC_PIN);

  sensors_event_t humidityEvent;
  sensors_event_t tempEvent;
  aht.getEvent(&humidityEvent, &tempEvent);

  measurement.tempInside = tempEvent.temperature;
  measurement.humidityInside = humidityEvent.relative_humidity;

  ens160.set_envdata(measurement.tempInside, measurement.humidityInside);
  ens160.measure();

  measurement.co2 = ens160.geteCO2();
  ens160.getTVOC(); // TVOC læses ikke med i JSON/databasen lige nu.

  measurement.tempOutside = bmp.readTemperature();
  measurement.pressureOutside = bmp.readPressure() / 100.0;

  float dewPointInside = calculateDewPoint(measurement.tempInside, measurement.humidityInside);

  if (hasValidReading(dewPointInside) && hasValidReading(measurement.tempOutside)) {
    measurement.condensationRisk = measurement.tempOutside <= dewPointInside;
  }

  DateTime now = rtc.now();
  measurement.rtcTimestamp = createTimestamp(now);

  return measurement;
}

String SensorManager::createTimestamp(const DateTime& now) const {
  char timestamp[30];

  sprintf(
    timestamp,
    "%04d-%02d-%02dT%02d:%02d:%02dZ",
    now.year(),
    now.month(),
    now.day(),
    now.hour(),
    now.minute(),
    now.second()
  );

  return String(timestamp);
}

float SensorManager::calculateDewPoint(float temperature, float humidity) const {
  if (!hasValidReading(temperature) || !hasValidReading(humidity) || humidity <= 0.0) {
    return NAN;
  }

  // Magnus formula. Resultat i grader Celsius.
  const float a = 17.62;
  const float b = 243.12;
  const float gamma = log(humidity / 100.0) + ((a * temperature) / (b + temperature));

  return (b * gamma) / (a - gamma);
}

bool SensorManager::hasValidReading(float value) const {
  return !isnan(value) && !isinf(value);
}
