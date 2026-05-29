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

// Gemmer måling lokalt ved manglende forbindelse.
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

  File inFile = SD.open(offlineFilePath, FILE_READ);
  if (!inFile) {
    Serial.println("Kunne ikke læse offline.txt");
    return;
  }

  if (SD.exists(tmpFilePath)) {
    SD.remove(tmpFilePath);
  }

  File tmpFile = SD.open(tmpFilePath, FILE_WRITE);
  if (!tmpFile) {
    inFile.close();
    Serial.println("Kunne ikke oprette tmp-fil");
    return;
  }

  bool anyFailed = false;

  while (inFile.available()) {
    String line = inFile.readStringUntil('\n');
    line.trim();

    if (line.length() == 0) {
      continue;
    }

    if (!anyFailed) {
      bool uploaded = apiClient.upload(line);
      if (!uploaded) {
        anyFailed = true;
        tmpFile.println(line);  // gem den fejlede linje
      } else {
        delay(500);
      }
    } else {
      tmpFile.println(line);  // gem alle resterende linjer
    }
  }

  inFile.close();
  tmpFile.close();

  SD.remove(offlineFilePath);

  // Ved genoprettelse af forbindelse, uploades gemte målinger
  if (!anyFailed) {
    SD.remove(tmpFilePath);
    Serial.println("Alle offline målinger uploadet og fil slettet");

  } else {
    File src = SD.open(tmpFilePath, FILE_READ);
    File dst = SD.open(offlineFilePath, FILE_WRITE);
    if (src && dst) {
      while (src.available()) {
        dst.write(src.read());
      }
    }
    if (src) src.close();
    if (dst) dst.close();
    SD.remove(tmpFilePath);
    Serial.println("Ikke alle offline målinger kunne uploades - resterende gemt");
  }
}
