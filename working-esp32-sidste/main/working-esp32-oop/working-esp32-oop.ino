#include "Config.h"
#include "SensorManager.h"
#include "WiFiManagerCustom.h"
#include "ApiClient.h"
#include "SDLogger.h"

SensorManager sensors;
WiFiManagerCustom wifi;
ApiClient api;
SDLogger sdLogger;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Starter ESP32 datalogger...");

  sensors.begin();
  sdLogger.begin();
  wifi.connect();
}

void loop() {
  Measurement measurement = sensors.readMeasurement();
  String json = measurement.toJson();

  Serial.println("Ny måling:");
  Serial.println(json);

  if (!wifi.isConnected()) {
    wifi.connect();
  }

  if (wifi.isConnected()) {
    sdLogger.uploadStoredData(api);

    bool uploaded = api.upload(json);

    if (!uploaded) {
      sdLogger.save(json);
    }
  } else {
    sdLogger.save(json);
  }

  delay(MEASUREMENT_INTERVAL_MS);
}
