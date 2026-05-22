#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H
#include <driver/i2s_std.h>

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
  i2s_chan_handle_t _rxHandle;

  String createTimestamp(const DateTime& now) const;
  float calculateDewPoint(float temperature, float humidity) const;
  bool hasValidReading(float value) const;
  void initI2S();
int readSoundLevel();
};

#endif
