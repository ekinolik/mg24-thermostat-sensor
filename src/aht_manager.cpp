#include "aht_manager.h"
#include "app_config.h"
#include <ArduinoLowPower.h>

AhtManager::AhtManager(pin_size_t powerPin) :
    m_powerPin(powerPin) {}

void AhtManager::begin(uint32_t count) {
    readCount = count;
    clearState();

    pinMode(m_powerPin, OUTPUT);
    digitalWrite(m_powerPin, LOW);
}

void AhtManager::update() {
    clearState();
    getAHTEventStats();
    printReadings();

    Serial.println("Complete, going to sleep");
    Serial.flush();
}

void AhtManager::getAHTEventStats() {
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

typename AhtManager::AHTMeasurement AhtManager::getAHTEvent() {
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

void AhtManager::updateTempStats(AHTMeasurement measurement) {
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

void AhtManager::clearState() {
    tempStats = Stats{};
    humidityStats = Stats{};
    m_successfulReading = false;
}

bool AhtManager::sensorStart() {
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

void AhtManager::sensorEnd() {
    Wire.end();
    digitalWrite(m_powerPin, LOW);
}

void AhtManager::printReadings() {
    Serial.printf("Temperature: %fC\n", tempStats.average);
    Serial.printf("Temperature: %fF\n", convertCtoF(tempStats.average));
    Serial.printf("Humidity: %f%%\n", humidityStats.average);
}

float AhtManager::convertCtoF(float C) {
    return (C * 9 / 5) + 32;
}