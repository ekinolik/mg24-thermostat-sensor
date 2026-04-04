# MG24 thermostat sensor

Firmware for a **Silicon Labs EFR32MG24**–class Arduino board that reads **temperature and relative humidity** from an **AHT20/AHT21-class** sensor (via the Adafruit AHTX0 driver), prints samples over **Serial**, then returns to **deep sleep** to save power. A **wake cycle counter** is stored in deep-sleep retention memory so it survives across sleep.

## Behavior

1. On boot, the sketch waits **2 seconds** (`LowPower.sleep(2000)`), then increments a persisted cycle counter (starts at 1 on cold boot). **Why:** see *Boot delay after deep sleep / EM4* below.
2. Initializes the built-in LED, Serial at **115200 baud**, and the AHT manager.
3. Each loop iteration: powers the sensor, runs I²C, takes several consecutive readings, prints °C, °F, and humidity, drives the LED inactive, logs execution time and cycle number, flushes Serial, then **deep sleeps** for a configurable interval.
4. After deep sleep the MCU resets; `setup()` runs again and the cycle continues.

Sensor power is **only asserted during a read**: the power pin is driven high, `Wire` is started, the AHT is initialized, samples are taken, then `Wire` is stopped and the power pin is driven low.

### Boot delay after deep sleep / EM4

On this platform, waking from **`LowPower.deepSleep()`** (energy mode **EM4**) can leave the system not fully ready when `setup()` first runs. If the firmware touches **Serial**, **I²C**, or other peripherals too soon, the device can fail to recover—effectively **bricked** until a reset or reflash. The **2 s sleep at the very start of `setup()`** is intentional: it gives clocks, buses, and the rest of the runtime time to finish coming up before any device code runs. That delay is separate from the optional wait for USB Serial in the IDE.

## Hardware

| Signal | Default pin | Notes |
|--------|-------------|--------|
| Sensor power | `D2` (`AppConfig::SENSOR_POWER`) | High = sensor powered for the measurement window |
| I²C | Board default (`Wire`) | SDA/SCL per your MG24 Arduino core / board |
| AHT sensor | I²C | AHT20/AHT10 family as supported by Adafruit AHTX0 |

Wire your AHT module to the board’s I²C pins and to the same ground. Power the sensor through the `SENSOR_POWER` switched line if your schematic uses it; adjust `SENSOR_POWER` if you use a different GPIO.

## Arduino setup

1. Install the **Silicon Labs** board support in the Arduino Board Manager (version in use when this was written: **3.x**; pick the current release that targets your MG24 board).
2. Select the correct **board** and **port** for your device.
3. Install **Library Manager** dependencies (see below).

### Libraries

| Library | Role |
|---------|------|
| [Adafruit AHTX0](https://github.com/adafruit/Adafruit_AHTX0) | AHT temperature / humidity |
| **Adafruit BusIO** | Dependency of Adafruit AHTX0 (install if the IDE prompts) |
| **ArduinoLowPower** | Deep sleep and retention RAM helpers (provided by / aligned with the Silicon Labs core) |
| **Wire** | I²C (built into the core) |

## Configuration

Edit `src/app_config.h`:

| Constant | Meaning |
|----------|---------|
| `DEEP_SLEEP_MS` | Milliseconds in `LowPower.deepSleep()` between wake cycles (default **7000**). |
| `COUNT_OF_READS` | Number of back-to-back samples per wake (default **3**). |
| `SENSOR_POWER` | GPIO that switches sensor power (default **`D2`**). |

## Project layout

| Path | Purpose |
|------|---------|
| `mg24-thermostat-sensor.ino` | Sleep/cycle orchestration, Serial, LED |
| `src/app_config.h` | Timing, pin, and sample-count constants |
| `src/aht_manager.h` | `AhtManager` template declaration |
| `src/aht_manager.cpp` | Template definitions (included from the sketch so the compiler sees the instantiation) |

## Serial output

- Wait / startup line, then per sample: temperature (°C and °F) and humidity (%).
- `"Complete, going to sleep"` before sleep.
- Execution time since start of `setup()` and the current **cycle** number.
- On I²C or sensor init failure: `"AHT read failed"`.

Open the serial monitor at **115200** baud after flashing; on some boards USB enumeration can take up to a couple of seconds (the sketch waits briefly for Serial).
