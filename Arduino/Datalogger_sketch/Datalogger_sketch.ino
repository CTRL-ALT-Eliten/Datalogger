#include "TempSensor.h"
#include "SD_log.h"
#include "RTC.h"
#include "SensorData.h"
#include "Lys_Sensor.h"
#include "Mic.h"
#include "OLED.h"

// Komponenter & pins
TempSensor sensor1(7);
TempSensor sensor2(6);
LysSensor ldr(A0);
MikrofonSensor mic(A1);
SD_log sdlog(53, "HiveLog1.txt");
RTC clock1; // Dedikerede pins til RTC digital 20 & 21
OledDisplay oled; // SCL & SDA

void setup()
{
    Serial.begin(9600);
    delay(3000);

    // Start SD, RTC & OLED
    oled.begin();
    sdlog.begin();
    clock1.begin();
}

void loop()
{
    SensorData d;

    // Loop for at undgå ugyldige data | ingen log ved fejlmåling
    while (true)
    {
        d.temp1 = sensor1.readTemperature();
        d.hum1  = sensor1.readHumidity();
        d.temp2 = sensor2.readTemperature();
        d.hum2  = sensor2.readHumidity();
        d.light = ldr.readLight();

        if (d.temp1 > -40 && d.temp2 > -40)
            break;

        Serial.println("Fejl i måling, prøver igen");
        delay(2000);
    }

    // Udskriv til Serial
    Serial.print("Sensor 1 - Temperatur: "); Serial.print(d.temp1, 1);
    Serial.print(" °C, Fugtighed: "); Serial.print(d.hum1, 1); Serial.println("%");
    Serial.print("Sensor 2 - Temperatur: "); Serial.print(d.temp2, 1);
    Serial.print(" °C, Fugtighed: "); Serial.print(d.hum2, 1); Serial.println("%");

    int amplitude = mic.readAmplitude(2000);
    String soundLevel = mic.classifyActivity(amplitude);

    String dayOrNight = ldr.getDayOrNight();
    Serial.print("Aktivitet: "); Serial.print(soundLevel); Serial.println(" | " + dayOrNight);

    String timestamp = clock1.getTimestamp();

    // Logging til SD
    sdlog.log(timestamp, d.temp1, d.hum1, d.temp2, d.hum2, soundLevel, dayOrNight, d.light);

    Serial.print("Tidspunkt: "); Serial.println(timestamp);

    String dateStr = timestamp.substring(0, 10);
    String timeStr = timestamp.substring(11, 19);

    // Send SD-status til OLED
    String sdStatus = sdlog.isLastWriteOK() ? "OK" : "FAIL";

    oled.showData(
          dateStr.c_str(),
          timeStr.c_str(),
          d.temp1,
          d.hum1,
          d.temp2,
          d.hum2,
          soundLevel,
          sdStatus);

    delay(600000); // 10 min interval
}
