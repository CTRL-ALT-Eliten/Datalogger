#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <Arduino.h>

class Measurement {
public:
  Measurement();

  String deviceId;
  int loggerIdEx;
  String rtcTimestamp;

  float tempInside;
  float humidityInside;
  int co2;

  float tempOutside;
  float pressureOutside;

  bool condensationRisk;

  int light;
  int sound;

  String toJson() const;

private:
  float round1(float value) const;
};

#endif
