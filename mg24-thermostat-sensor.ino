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
  LowPower.sleep(2000);

  if (LowPower.wokeUpFromDeepSleep()) {
    cycle = LowPower.deepSleepMemoryRead(0);
    cycle++;
  } else {
    cycle = 1;
  }

  LowPower.deepSleepMemoryWrite(0, cycle);

  startMillis = millis();

  setPinsStartup();
  ahtManager.begin();
  Serial.begin(115200);
  unsigned long serialStart = millis();
  while (!Serial && millis() - serialStart < 2000) {
    delay(10);
  }
  
  Serial.printf("Running after waiting %ums...\n", millis() - serialStart);
}

void loop() {
  ahtManager.update();
  setPinsShutdown();

  Serial.printf("Execution time: %ums\n", millis() - startMillis);
  Serial.printf("Cycle: %lu\n", cycle);
  Serial.flush();

  LowPower.deepSleep((int)AppConfig::DEEP_SLEEP_MS);
}

void setPinsStartup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void setPinsShutdown() {
  digitalWrite(LED_BUILTIN, LED_BUILTIN_INACTIVE);
  //digitalWrite(AppConfig::SENSOR_POWER, LOW);
}