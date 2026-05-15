#ifndef WIFI_MANAGER_CUSTOM_H
#define WIFI_MANAGER_CUSTOM_H

#include <Arduino.h>

class WiFiManagerCustom {
public:
  void connect();
  bool isConnected() const;
};

#endif
