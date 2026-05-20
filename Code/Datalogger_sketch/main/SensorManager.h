#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>
#include <ScioSense_ENS160.h>
#include <RTClib.h>
#include "Measurement.h"

class SensorManager {
public:
  SensorManager();

  void begin();
  Measurement readMeasurement();

private:
  Adafruit_BMP280 bmp;
  Adafruit_AHTX0 aht;
  ScioSense_ENS160 ens160;
  RTC_DS3231 rtc;

  String createTimestamp(const DateTime& now) const;
  float calculateDewPoint(float temperature, float humidity) const;
  bool hasValidReading(float value) const;
};

#endif
