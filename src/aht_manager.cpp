#include "aht_manager.h"
#include "app_config.h"
#include <ArduinoLowPower.h>

AhtManager::AhtManager(pin_size_t powerPin) :
    m_powerPin(powerPin) {}

void AhtManager::begin(uint32_t count) {
    // Serial may not be available yet
    readCount = count;
    clearState();

    pinMode(m_powerPin, OUTPUT);
    digitalWrite(m_powerPin, LOW);
}

void AhtManager::update() {
    clearState();
    getAHTEventStats();
    finalizeMeasurement();
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

    if (measurement.humidity != errorHumidity) {
        humidityStats.count++;
        humidityStats.sum += measurement.humidity;
        humidityStats.average = humidityStats.sum / humidityStats.count;
    }
}

void AhtManager::finalizeMeasurement() {
    if (!m_successfulReading || tempStats.count == 0 || humidityStats.count == 0) {
        latestMeasurement = AHTMeasurement{};
    }

    latestMeasurement.temperature = tempStats.average;
    latestMeasurement.humidity    = humidityStats.average;
    latestMeasurement.isValid     = true;
}

void AhtManager::clearState() {
    tempStats = Stats{};
    humidityStats = Stats{};
    m_successfulReading = false;
}

bool AhtManager::sensorStart() {
  digitalWrite(m_powerPin, HIGH);
  LowPower.sleep(10); // Give time for AHT device to start

  Wire.begin();
  LowPower.sleep(5);

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
    if (!latestMeasurement.isValid) {
        Serial.println("Temperature: invalid");
        Serial.println("Humidity: invalid");
    }
    
    Serial.printf("Temperature: %fC\n", latestMeasurement.temperature);
    Serial.printf("Temperature: %fF\n", convertCtoF(latestMeasurement.temperature));
    Serial.printf("Humidity: %f%%\n", latestMeasurement.humidity);
}

float AhtManager::convertCtoF(float C) {
    return (C * 9 / 5) + 32;
}

bool AhtManager::hasValidReading() const {
    return latestMeasurement.isValid;
}

float AhtManager::getTemperatureC() const {
    return latestMeasurement.temperature;
}

float AhtManager::getHumidityPct() const {
    return latestMeasurement.humidity;
}

AhtManager::AHTMeasurement AhtManager::getLatestMeasurement() const {
    return latestMeasurement;
}