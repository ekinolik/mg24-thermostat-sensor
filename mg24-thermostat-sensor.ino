#include <ArduinoBLE.h>
#include <ArduinoLowPower.h>

#include "src/app_config.h"
#include "src/aht_manager.h"

void setupBLE();
void setPinsStartup();

AhtManager ahtManager(AppConfig::SENSOR_POWER);

uint32_t cycle;

BLEService envService("181A");
BLEShortCharacteristic temperatureCharacteristic("2A6E", BLERead | BLENotify);
BLEUnsignedShortCharacteristic humidityCharacteristic("2A6F", BLERead | BLENotify);

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

  // Keep BLE stack serviced
  BLE.poll();

  ahtManager.update();
  updateBLECharacteristics();

  Serial.printf("Execution time: %ums\n", millis() - cycleStart);
  Serial.printf("Cycle: %lu\n", cycle);
  Serial.flush();

  BLE.poll();

  digitalWrite(LED_BUILTIN, LED_BUILTIN_INACTIVE);

  LowPower.sleep(10); //(int)AppConfig::DEEP_SLEEP_MS);
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

  BLE.setDeviceName("MG24 Thermostat Sensor");
  BLE.setLocalName("MG24 Themostat Sensor");
  BLE.setAdvertisedService(envService);

  envService.addCharacteristic(temperatureCharacteristic);
  envService.addCharacteristic(humidityCharacteristic);

  BLE.addService(envService);

  temperatureCharacteristic.writeValue((int16_t)0);
  humidityCharacteristic.writeValue((int16_t)0);

  BLE.advertise();

  Serial.println("BLE Advertising Started");
}

void updateBLECharacteristics() {
  if (!ahtManager.hasValidReading()) {
    Serial.println("Skipping BLE Update: no valid reading");

    return;
  }

  const float tempC = ahtManager.getTemperatureC();
  const float humidityPct = ahtManager.getHumidityPct();

  // BLE ESS Temperatuer/humidity are commonly sent in 0.01 units
  const int16_t tempBle = (int16_t)lroundf(tempC * 100.0f);
  Serial.printf("---- ROUNDED: %d\n", tempBle);
  const uint16_t humidityBle = (uint16_t)lroundf(humidityPct * 100.0f);

  temperatureCharacteristic.writeValue(tempBle);
  humidityCharacteristic.writeValue(humidityBle);

  Serial.printf("BLE Updated: temp=%d (0.01C), humidity=%u (0.01%%)\n",
    tempBle, humidityBle);

}