#include <ArduinoBLE.h>
#include <ArduinoLowPower.h>

#include "src/app_config.h"
#include "src/aht_manager.h"

void setupBLE();
void advertiseMeasurement();
void buildBeaconPayload(uint8_t* payload, size_t& payloadLen);
void setPinsStartup();

AhtManager ahtManager(AppConfig::SENSOR_POWER);

uint32_t cycle;

void setup() {
  cycle = 0;

  setPinsStartup();
  Serial.begin(115200);
  ahtManager.begin(AppConfig::COUNT_OF_READS);

  unsigned long serialStart = millis();
  while (!Serial && millis() - serialStart < 2000) {
    LowPower.sleep(10);
  }
  
  Serial.printf("Running after waiting %ums...\n", millis() - serialStart);

  setupBLE();
}

void loop() {
  uint64_t cycleStart = millis();
  cycle++;
  digitalWrite(LED_BUILTIN, LED_BUILTIN_ACTIVE);

  ahtManager.update();
  advertiseMeasurement();

  Serial.printf("Execution time: %ums\n", millis() - cycleStart);
  Serial.printf("Cycle: %lu\n", cycle);
  Serial.flush();

  BLE.poll();

  digitalWrite(LED_BUILTIN, LED_BUILTIN_INACTIVE);

  LowPower.sleep((int)AppConfig::DEEP_SLEEP_MS);
}

void setPinsStartup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void setupBLE() {
  if (!BLE.begin()) {
    Serial.println("BLE.begin() failed");
    while (true) {
      LowPower.sleep(1000);
    }
  }

  BLE.setLocalName(AppConfig::BLE_LOCAL_NAME);
  BLE.setAdvertisingInterval(AppConfig::ADV_INTERVAL_UNITS);

  Serial.println("BLE beacon mode initiated");
}

void advertiseMeasurement() {
  if (!ahtManager.hasValidReading()) {
    Serial.println("Skipping BLE Beacon: no valid reading");
    return;
  }

  uint8_t payload[8];
  size_t payloadLen = 0;
  buildBeaconPayload(payload, payloadLen);

  BLE.stopAdvertise();
  BLE.setManufacturerData(payload, payloadLen);

  if (!BLE.advertise()) {
    Serial.println("BLE.advertise() failed");
    return;
  }

  Serial.print("Advertising beacon payload: ");
  for (size_t i = 0; i < payloadLen; i++) {
    Serial.printf("%02X", payload[i]);
    if (i + 1 < payloadLen) Serial.print(" ");
  }
  Serial.println();

  uint32_t advStart = millis();
  while (millis() - advStart < AppConfig::ADVERTISE_WINDOW_MS) {
    BLE.poll();
    LowPower.sleep(25);
  }

  BLE.stopAdvertise();
  Serial.println("Stopped Advertising");
}

void buildBeaconPayload(uint8_t* payload, size_t& payloadLen) {
  const float tempC = ahtManager.getTemperatureC();
  const float humidityPct = ahtManager.getHumidityPct();

  const int16_t tempCenti = (uint16_t)lroundf(tempC * 100.0f);
  const uint16_t humidityCenti = (uint16_t)lroundf(humidityPct * 100.0f);

  uint8_t flags = 0x00;
  if (ahtManager.hasValidReading()) {
    flags |= 0x01;
  }

  // Manufacturer data payload
  payload[0] = (uint8_t)(AppConfig::COMPANY_ID & 0xFF);
  payload[1] = (uint8_t)((AppConfig::COMPANY_ID >> 8) & 0xFF);
  payload[2] = 0x01; //payload version
  payload[3] = flags;

  // little-endian int16 temperature
  payload[4] = (uint8_t)(tempCenti & 0xFF);
  payload[5] = (uint8_t)((tempCenti >> 8) & 0xFF);

  // little-endian uint16 humidity
  payload[6] = (uint8_t)(humidityCenti & 0xFF);
  payload[7] = (uint16_t)((humidityCenti >> 8) & 0xFF);

  payloadLen = 8;

  Serial.printf("Beacon values: temp=%.2fC humidity%.2f%%\n", tempC, humidityPct);
}