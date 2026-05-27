#ifndef WIFIMANAGERCUSTOM_H
#define WIFIMANAGERCUSTOM_H

class WiFiManagerCustom {
public:
  void connect();
  bool isConnected();
private:
  bool _connected = false;
};

#endif
