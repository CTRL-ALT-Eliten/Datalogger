#include "SDLogger.h"
#include "Config.h"
#include <SPI.h>
#include <SD.h>

void SDLogger::begin() {
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS)) {
    Serial.println("SD-kort ikke fundet");
  } else {
    Serial.println("SD-kort OK");
  }
}

void SDLogger::save(const String& json) {
  File file = SD.open(offlineFilePath, FILE_APPEND);

  if (!file) {
    Serial.println("Kunne ikke åbne offline.txt");
    return;
  }

  file.println(json);
  file.close();

  Serial.println("Måling gemt på SD");
}

void SDLogger::uploadStoredData(ApiClient& apiClient) {
  if (!SD.exists(offlineFilePath)) {
    return;
  }

  File file = SD.open(offlineFilePath, FILE_READ);

  if (!file) {
    Serial.println("Kunne ikke læse offline.txt");
    return;
  }

  bool allUploaded = true;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();

    if (line.length() == 0) {
      continue;
    }

    bool uploaded = apiClient.upload(line);

    if (!uploaded) {
      allUploaded = false;
      break;
    }

    delay(500);
  }

  file.close();

  if (allUploaded) {
    SD.remove(offlineFilePath);
    Serial.println("Alle offline målinger uploadet og fil slettet");
  } else {
    Serial.println("Ikke alle offline målinger kunne uploades");
  }
}
