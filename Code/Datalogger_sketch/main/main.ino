#include "Config.h"
#include "SensorManager.h"
#include "WiFiManagerCustom.h"
#include "ApiClient.h"
#include "SDLogger.h"
unsigned long lastLedPoll = 0; // til at tracke polling interval

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
  pinMode(LED_PIN, OUTPUT);
digitalWrite(LED_PIN, LOW); // start slukket
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

  // LED poll
if (wifi.isConnected() && millis() - lastLedPoll >= LED_POLL_INTERVAL_MS) {
    lastLedPoll = millis();

    bool ledOn = api.getLedStateFromApi();

    if (ledOn) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("LED: ON");
    } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println("LED: OFF");
    }
}

  delay(MEASUREMENT_INTERVAL_MS);
}
