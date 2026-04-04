#pragma once

#include "app_config.h"
#include <Arduino.h>
#include <Adafruit_AHTX0.h>

template <size_t READCOUNT=3>
class AhtManager {
public:
    Adafruit_AHTX0 aht;
    AhtManager(pin_size_t powerPin);

    static constexpr uint32_t countOfReads = READCOUNT;

    float tempReading[READCOUNT];
    float humidityReading[READCOUNT];

    void begin();
    void update();

    void getAndPrintTemp();

private:
    pin_size_t m_powerPin;
    
    sensors_event_t m_temp;
    sensors_event_t m_humidity;

    void clearTempAndHumidityReading();
    bool readAHT();
    bool sensorStart();
    void sensorEnd();

    void printReadings();

    float convertCtoF(float C);
};