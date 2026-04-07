#pragma once

#include "app_config.h"
#include <Arduino.h>
#include <Adafruit_AHTX0.h>

class AhtManager {
public:
    Adafruit_AHTX0 aht;
    AhtManager(pin_size_t powerPin);

    float errorTemp = -50;
    float errorHumidity = 0;

    struct Stats {
        float sum;
        int64_t count;
        float average;
    };

    struct AHTMeasurement {
        float temperature;
        float humidity;
        bool isValid = false;
    };

    Stats tempStats;
    Stats humidityStats;

    void begin(uint32_t count);
    void update();

private:
    pin_size_t m_powerPin;
    
    sensors_event_t m_temp;
    sensors_event_t m_humidity;

    bool m_successfulReading = false;

    uint32_t readCount;

    void clearState();
    bool readAHT();
    bool sensorStart();
    void sensorEnd();

    void getAHTEventStats();
    AHTMeasurement getAHTEvent();
    void updateTempStats(AHTMeasurement measurement);

    void printReadings();

    float convertCtoF(float C);
};