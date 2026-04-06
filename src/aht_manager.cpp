#include "aht_manager.h"
#include "app_config.h"
#include <ArduinoLowPower.h>

template <size_t READCOUNT>
AhtManager<READCOUNT>::AhtManager(pin_size_t powerPin) :
    m_powerPin(powerPin) {}

template <size_t READCOUNT>
void AhtManager<READCOUNT>::begin(uint32_t count) {
    readCount = count;
    clearState();

    pinMode(m_powerPin, OUTPUT);
    digitalWrite(m_powerPin, LOW);
}

template <size_t READCOUNT>
void AhtManager<READCOUNT>::update() {
    clearState();
    getAHTEventStats();
    printReadings();

    Serial.println("Complete, going to sleep");
    Serial.flush();
}

template <size_t READCOUNT>
void AhtManager<READCOUNT>::getAHTEventStats() {
    if (!sensorStart()) {
        return;
    }

    AHTMeasurement measurement ;
    for (uint32_t i = 0; i < readCount; i++) {
        measurement = getAHTEvent();
        if (!measurement.isValid) {
            continue;
        }

        updateTempStats(measurement);
        m_successfulReading = true;
    }

    sensorEnd();

    return;
}

template <size_t READCOUNT>
typename AhtManager<READCOUNT>::AHTMeasurement AhtManager<READCOUNT>::getAHTEvent() {
    if (digitalRead(m_powerPin) != HIGH) {
        return AHTMeasurement{};
    }

    aht.getEvent(&m_humidity, &m_temp);

    AHTMeasurement measurement;
    measurement.temperature = m_temp.temperature;
    measurement.humidity = m_humidity.relative_humidity;
    measurement.isValid = true;

    return measurement;
}

template <size_t READCOUNT>
void AhtManager<READCOUNT>::updateTempStats(AHTMeasurement measurement) {
    if (measurement.temperature != errorTemp) {
        tempStats.count++;
        tempStats.sum += measurement.temperature;
        tempStats.average = tempStats.sum / tempStats.count;
    }

    if (measurement.humidity != errorTemp) {
        humidityStats.count++;
        humidityStats.sum += measurement.humidity;
        humidityStats.average = humidityStats.sum / humidityStats.count;
    }
}

template <size_t READCOUNT>
void AhtManager<READCOUNT>::clearState() {
    tempStats = Stats{};
    humidityStats = Stats{};
    m_successfulReading = false;
}

template <size_t READCOUNT>
bool AhtManager<READCOUNT>::sensorStart() {
  digitalWrite(m_powerPin, HIGH);
  LowPower.sleep(200); // Give time for AHT device to start

  Wire.begin();
  LowPower.sleep(20);

  if (!aht.begin()) {
    Wire.end();
    digitalWrite(m_powerPin, LOW);

    return false;
  }

  return true;
}

template <size_t READCOUNT>
void AhtManager<READCOUNT>::sensorEnd() {
    Wire.end();
    digitalWrite(m_powerPin, LOW);
}

template <size_t READCOUNT>
void AhtManager<READCOUNT>::printReadings() {
    Serial.printf("Temperature: %fC\n", tempStats.average);
    Serial.printf("Temperature: %fF\n", convertCtoF(tempStats.average));
    Serial.printf("Humidity: %f%%\n", humidityStats.average);
}

template <size_t READCOUNT>
float AhtManager<READCOUNT>::convertCtoF(float C) {
    return (C * 9 / 5) + 32;
}