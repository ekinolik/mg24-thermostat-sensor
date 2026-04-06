#include <Wire.h>
#include <ArduinoLowPower.h>
#include "src/app_config.h"
#include "src/aht_manager.cpp"

void setPinsStartup();
void setPinsShutdown();

AhtManager<AppConfig::COUNT_OF_READS> ahtManager(AppConfig::SENSOR_POWER);

unsigned long startMillis;

uint32_t cycle;

void setup() {
  cycle = 0;
  startMillis = millis();

  setPinsStartup();
  Serial.begin(115200);
  ahtManager.begin(AppConfig::COUNT_OF_READS);
  unsigned long serialStart = millis();
  while (!Serial && millis() - serialStart < 2000) {
    LowPower.sleep(10);
  }
  
  Serial.printf("Running after waiting %ums...\n", millis() - serialStart);
}

void loop() {
  uint64_t cycleStart = millis();
  cycle++;
  digitalWrite(LED_BUILTIN, LED_BUILTIN_ACTIVE);
  ahtManager.update();
  //setPinsShutdown();

  Serial.printf("Execution time: %ums\n", millis() - cycleStart);
  Serial.printf("Cycle: %lu\n", cycle);
  Serial.flush();
  digitalWrite(LED_BUILTIN, LED_BUILTIN_INACTIVE);

  //LowPower.deepSleep((int)AppConfig::DEEP_SLEEP_MS);
  LowPower.sleep((int)AppConfig::DEEP_SLEEP_MS);
}

void setPinsStartup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void setPinsShutdown() {
  digitalWrite(LED_BUILTIN, LED_BUILTIN_INACTIVE);
}