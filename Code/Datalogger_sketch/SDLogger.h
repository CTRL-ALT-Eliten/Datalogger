#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <Arduino.h>
#include "ApiClient.h"

class SDLogger {
public:
  void begin();
  void save(const String& json);
  void uploadStoredData(ApiClient& apiClient);

private:
  const char* offlineFilePath = "/offline.txt";
};

#endif
