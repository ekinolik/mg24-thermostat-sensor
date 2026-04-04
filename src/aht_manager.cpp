#include "aht_manager.h"
#include "app_config.h"

//Adafruit_AHTX0 aht;

template <size_t READCOUNT>
AhtManager<READCOUNT>::AhtManager(pin_size_t powerPin) :
    m_powerPin(powerPin) {}

template <size_t READCOUNT>
void AhtManager<READCOUNT>::begin() {
    clearTempAndHumidityReading();

    pinMode(m_powerPin, OUTPUT);
    digitalWrite(m_powerPin, LOW);
}

template <size_t READCOUNT>
void AhtManager<READCOUNT>::update() {
    getAndPrintTemp();

    Serial.println("Complete, going to sleep");
    Serial.flush();
}

template <size_t READCOUNT>
void AhtManager<READCOUNT>::clearTempAndHumidityReading() {
    for (uint8_t i = 0; i < countOfReads; i++) {
        tempReading[i]     = -50; // Temp shown on error
        humidityReading[i] = 0; // Humidity shown on error
    }
}

template <size_t READCOUNT>
bool AhtManager<READCOUNT>::readAHT() {
  if (!sensorStart()) {
    return false;
  }

  delay(50); // is this needed?

  for (uint32_t i = 0; i < countOfReads; i++) {
    
    aht.getEvent(&m_humidity, &m_temp);

    tempReading[i] = m_temp.temperature;
    humidityReading[i] = m_humidity.relative_humidity;
    delay(10); // Not actually needed
  }

  sensorEnd();
  
  return true;
}

template <size_t READCOUNT>
bool AhtManager<READCOUNT>::sensorStart() {
  digitalWrite(m_powerPin, HIGH);
  delay(200); // Give time for AHT device to start

  Wire.begin();
  delay(20);

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
void AhtManager<READCOUNT>::getAndPrintTemp() {
  if (readAHT()) {
    printReadings();
  } else {
    Serial.println("AHT read failed");
  }
}

template <size_t READCOUNT>
void AhtManager<READCOUNT>::printReadings() {
  for (uint32_t i = 0; i < countOfReads; i++) {
    Serial.printf("Temperature: %fC\n", tempReading[i]);
    Serial.printf("Temperature: %fF\n", convertCtoF(tempReading[i]));
    Serial.printf("Humidity: %f%%\n", humidityReading[i]);
  }
}

template <size_t READCOUNT>
float AhtManager<READCOUNT>::convertCtoF(float C) {
    return (C * 9 / 5) + 32;
}