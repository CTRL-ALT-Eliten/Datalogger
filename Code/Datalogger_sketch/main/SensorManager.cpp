#include "SensorManager.h"
#include "Config.h"
#include <math.h>

SensorManager::SensorManager()
  : ens160(ENS160_I2CADDR_1) {
}

// Initialiserer sensorer og interfaces.
void SensorManager::begin() {

  Wire.begin(I2C_SDA, I2C_SCL);

  initI2S();

  if (!rtc.begin()) {

    Serial.println("RTC ikke fundet");

  } else {

    Serial.println("RTC OK");
 //   rtc.adjust(DateTime(2026, 5, 24, 11(time), 33(minut), 0(sekund)));

  }

  if (!bmp.begin(0x76)) {

    Serial.println("BMP280 ikke fundet på 0x76. Prøver 0x77...");

    if (!bmp.begin(0x77)) {

      Serial.println("BMP280 ikke fundet");

    } else {

      Serial.println("BMP280 OK på 0x77");
    }

  } else {

    Serial.println("BMP280 OK på 0x76");
  }

  if (!aht.begin()) {

    Serial.println("AHT20/AHT21 ikke fundet");

  } else {

    Serial.println("AHT OK");
  }

  if (!ens160.begin()) {

    Serial.println("ENS160 ikke fundet");

  } else {

    ens160.setMode(ENS160_OPMODE_STD);

    Serial.println("ENS160 OK");
  }
}

// Læser alle sensorværdier.
Measurement SensorManager::readMeasurement() {

  Measurement measurement;

  measurement.deviceId = DEVICE_ID;
  measurement.loggerIdEx = LOGGER_ID_EX;

  measurement.light = analogRead(LDR_PIN);

  measurement.sound = readSoundLevel();

  Serial.print("Sound level: ");
  Serial.println(measurement.sound);

  sensors_event_t humidityEvent;
  sensors_event_t tempEvent;

  aht.getEvent(&humidityEvent, &tempEvent);

  measurement.tempInside = tempEvent.temperature;
  measurement.humidityInside = humidityEvent.relative_humidity;

  ens160.set_envdata(
    measurement.tempInside,
    measurement.humidityInside
  );

  ens160.measure();

  measurement.co2 = ens160.geteCO2();

  ens160.getTVOC();

  measurement.tempOutside = bmp.readTemperature();

  measurement.pressureOutside = bmp.readPressure() / 100.0;

  float dewPointInside = calculateDewPoint(
    measurement.tempInside,
    measurement.humidityInside
  );

  if (
    hasValidReading(dewPointInside) &&
    hasValidReading(measurement.tempOutside)
  ) {

    measurement.condensationRisk =
      measurement.tempOutside <= dewPointInside;
  }

  DateTime now = rtc.now();

  measurement.rtcTimestamp = createTimestamp(now);

  return measurement;
}

String SensorManager::createTimestamp(const DateTime& now) const {

  char timestamp[30];

  sprintf(
    timestamp,
    "%04d-%02d-%02dT%02d:%02d:%02dZ",
    now.year(),
    now.month(),
    now.day(),
    now.hour(),
    now.minute(),
    now.second()
  );

  return String(timestamp);
}

float SensorManager::calculateDewPoint(
  float temperature,
  float humidity
) const {

  if (
    !hasValidReading(temperature) ||
    !hasValidReading(humidity) ||
    humidity <= 0.0
  ) {

    return NAN;
  }

  const float a = 17.62;
  const float b = 243.12;

  const float gamma =
    log(humidity / 100.0) +
    ((a * temperature) / (b + temperature));

  return (b * gamma) / (a - gamma);
}

bool SensorManager::hasValidReading(float value) const {

  return !isnan(value) && !isinf(value);
}

void SensorManager::initI2S() {

  // Start interface til I2S kommunikation
  i2s_chan_config_t chan_cfg =
    I2S_CHANNEL_DEFAULT_CONFIG(
      I2S_NUM_0,
      I2S_ROLE_MASTER
    );

  i2s_new_channel(
    &chan_cfg,
    NULL,
    &_rxHandle
  );
  // Konfigurer I2S audio format.
  i2s_std_config_t std_cfg = {
  // 16 kHz sample rate.
    .clk_cfg =
      I2S_STD_CLK_DEFAULT_CONFIG(16000),
  // 32-bit mono input fra INMP441
    .slot_cfg =
        I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),

    .gpio_cfg = {

      .mclk = I2S_GPIO_UNUSED,

      .bclk = (gpio_num_t)I2S_SCK_PIN,

      .ws = (gpio_num_t)I2S_WS_PIN,

      .dout = I2S_GPIO_UNUSED,

      .din = (gpio_num_t)I2S_SD_PIN,

      .invert_flags = {
        .mclk_inv = false,
        .bclk_inv = false,
        .ws_inv = false
      },
    },
  };

  esp_err_t err;

  err = i2s_channel_init_std_mode(
    _rxHandle,
    &std_cfg
  );

  if (err != ESP_OK) {

    Serial.print("I2S init fejl: ");

    Serial.println(err);

    return;
  }

  err = i2s_channel_enable(_rxHandle);

  if (err != ESP_OK) {

    Serial.print("I2S enable fejl: ");

    Serial.println(err);

    return;
  }

  Serial.println("INMP441 klar");
}

int SensorManager::readSoundLevel() {
  const int SAMPLE_COUNT = 1024;
  int32_t samples[SAMPLE_COUNT];
  size_t bytesRead = 0;

  esp_err_t result = i2s_channel_read(
    _rxHandle, samples, sizeof(samples), &bytesRead, pdMS_TO_TICKS(1000)
  );

// tjek ved fejl
  if (result != ESP_OK || bytesRead == 0) {
    Serial.println("FEJL: ingen data");
    return 0;
  }

  int count = bytesRead / sizeof(int32_t);

  // Vis de første 10 rå samples
  for (int i = 0; i < 10 && i < count; i++) {
    Serial.print("  ["); Serial.print(i); Serial.print("] 0x");
    Serial.print((uint32_t)samples[i], HEX);
    Serial.print(" = "); Serial.println(samples[i]);
  }

  // Find max og min
  int32_t maxVal = samples[0];
  int32_t minVal = samples[0];
  for (int i = 1; i < count; i++) {
    if (samples[i] > maxVal) maxVal = samples[i];
    if (samples[i] < minVal) minVal = samples[i];
  }

  // Vis hvad >> 8 giver

  // RMS med >> 8
  int64_t sum8 = 0;
  for (int i = 0; i < count; i++) {
    int32_t s = samples[i] >> 8;
    sum8 += (int64_t)s * s;
  }
  int rms8 = sqrt((double)sum8 / count);

  // RMS med >> 14
  int64_t sum14 = 0;
  for (int i = 0; i < count; i++) {
    int32_t s = samples[i] >> 14;
    sum14 += (int64_t)s * s;
  }
  int rms14 = sqrt((double)sum14 / count);


  return rms8;
}
