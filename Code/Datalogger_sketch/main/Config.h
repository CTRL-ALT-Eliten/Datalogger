#ifndef CONFIG_H
#define CONFIG_H

// -----------------------------
// WIFI / API
// -----------------------------

const char* const WIFI_SSID = "KoegeGuest";
const char* const WIFI_PASSWORD = "";

const char* const API_URL = "http://152.115.77.165:50326/measurements";
const char* const API_KEY = "bee123";

const char* const DEVICE_ID = "logger_001";
const int LOGGER_ID_EX = 1;

// -----------------------------
// LED
// -----------------------------
const char* const LED_STATUS_URL = "http://152.115.77.165:50326/api/led/1";
#define LED_PIN 2
const unsigned long LED_POLL_INTERVAL_MS = 2000;

// -----------------------------
// PINS
// -----------------------------

#define I2C_SDA 21
#define I2C_SCL 22

#define LDR_PIN 34
#define MIC_PIN 35

#define SD_CS 5

// ESP32 default VSPI pins used for SD card
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23

// -----------------------------
// TIMING
// -----------------------------

const unsigned long MEASUREMENT_INTERVAL_MS = 60000;

#endif
