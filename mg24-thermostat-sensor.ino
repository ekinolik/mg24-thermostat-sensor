#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <ArduinoLowPower.h>

#define SENSOR_POWER D2
#define DEEP_SLEEP_MS 7000UL // 7 seconds

#define COUNT_OF_READS 3

bool readAHT(float tempReading[], float humidityReading[]);
void printReadings(float tempReading[], float humidityReading[]);
void getAndPrintTemp();
void setPinsStartup();
void setPinsShutdown();

Adafruit_AHTX0 aht;
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
  Serial.begin(115200);
  unsigned long serialStart = millis();
  while (!Serial && millis() - serialStart < 2000) {
    delay(10);
  }
  
  Serial.printf("Running after waiting %ums...\n", millis() - serialStart);
}

void loop() {
  getAndPrintTemp();

  Serial.println("Complete, going to sleep");
  Serial.printf("Execution time: %ums\n", millis() - startMillis);
  Serial.printf("Cycle: %lu\n", cycle);
    setPinsShutdown();
  Serial.flush();
  delay(50);

  LowPower.deepSleep(DEEP_SLEEP_MS);
}

void setPinsStartup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR_POWER, OUTPUT);
  digitalWrite(SENSOR_POWER, LOW);
}

void setPinsShutdown() {
  digitalWrite(LED_BUILTIN, LED_BUILTIN_INACTIVE);
  digitalWrite(SENSOR_POWER, LOW);
}

void getAndPrintTemp() {
  float tempReading[COUNT_OF_READS];
  float humidityReading[COUNT_OF_READS];

  if (readAHT(tempReading, humidityReading)) {
    printReadings(tempReading, humidityReading);
  } else {
    Serial.println("AHT read failed");
  }

}

bool readAHT(float tempReading[], float humidityReading[]) {
  digitalWrite(SENSOR_POWER, HIGH);
  delay(200);

  Wire.begin();
  delay(20);

  if (!aht.begin()) {
    Wire.end();
    digitalWrite(SENSOR_POWER, LOW);

    return false;
  }

  delay(50);

  sensors_event_t humidity, temp;
  for (uint32_t i = 0; i < COUNT_OF_READS; i++) {
    
    aht.getEvent(&humidity, &temp);

    tempReading[i] = temp.temperature;
    humidityReading[i] = humidity.relative_humidity;
    delay(50);
  }

  Wire.end();
  digitalWrite(SENSOR_POWER, LOW);

  return true;
}

void printReadings(float tempReading[], float humidityReading[]) {
  for (uint32_t i = 0; i < COUNT_OF_READS; i++) {
    Serial.printf("Temperature: %fC\n", tempReading[i]);
    Serial.printf("Temperature: %fF\n", (tempReading[i] * 9 / 5) + 32);
    Serial.printf("Humidity: %f%%\n", humidityReading[i]);
  }
}