#pragma once

#include "app_config.h"
#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <Wire.h>

static constexpr float ERROR_TEMP_READING = -50;
static constexpr float ERROR_HUMIDITY_READING = 0;

class AhtManager {
public:
    Adafruit_AHTX0 aht;
    AhtManager(pin_size_t powerPin);

    float errorTemp     = ERROR_TEMP_READING;
    float errorHumidity = ERROR_HUMIDITY_READING;

    struct Stats {
        float sum     = 0;
        int64_t count = 0;
        float average = 0;
    };

    struct AHTMeasurement {
        float temperature = ERROR_TEMP_READING;
        float humidity    = ERROR_HUMIDITY_READING;
        bool isValid      = false;
    };

    void begin(uint32_t count);
    void update();

    bool hasValidReading() const;
    float getTemperatureC() const;
    float getHumidityPct() const;
    AHTMeasurement getLatestMeasurement() const;

private:
    pin_size_t m_powerPin;
    sensors_event_t m_temp;
    sensors_event_t m_humidity;
    bool m_successfulReading = false;
    uint32_t readCount;

    Stats tempStats;
    Stats humidityStats;
    AHTMeasurement latestMeasurement;

    void clearState();
    bool sensorStart();
    void sensorEnd();
    void getAHTEventStats();
    AHTMeasurement getAHTEvent();
    void updateTempStats(AHTMeasurement measurement);
    void finalizeMeasurement();
    void printReadings();
    float convertCtoF(float C);
};