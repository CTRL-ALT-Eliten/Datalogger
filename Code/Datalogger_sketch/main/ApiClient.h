#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <Arduino.h>
 
class ApiClient {
public:
    bool upload(const String& json);

    // Tilføj denne linje
    bool getLedStateFromApi();
};

#endif

// LED-funktion
bool getLedStateFromApi();
