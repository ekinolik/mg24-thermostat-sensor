# MG24 thermostat sensor

Firmware for a **Silicon Labs EFR32MG24**–class Arduino board that reads **temperature and relative humidity** from an **AHT20/AHT21-class** sensor (via the Adafruit AHTX0 driver), **averages** several samples per cycle, prints results over **Serial**, and sleeps between cycles with **`LowPower.sleep()`** to reduce duty cycle.

## Behavior

1. **`setup()`** — Resets a **cycle counter** (RAM only), configures the built-in LED, starts **Serial** at **115200 baud**, and calls `ahtManager.begin(AppConfig::COUNT_OF_READS)`. While waiting for USB Serial (up to ~2 s), the wait loop uses **`LowPower.sleep(10)`** instead of a busy delay.
2. **`loop()`** — Increments the cycle counter, turns the **LED on**, runs a measurement (`ahtManager.update()`), prints **execution time for that iteration** and the **cycle** number, turns the **LED off**, then sleeps for **`AppConfig::DEEP_SLEEP_MS`** using **`LowPower.sleep()`**.  
   - **`LowPower.deepSleep()`** is **commented out** in the sketch; with the current sleep mode the MCU typically **does not** fully reset between iterations, so the cycle counter **does not** use retention RAM.
3. **Sensor path** — Power to the AHT is **only asserted during a read**: the power pin goes high, short **`LowPower.sleep()`** delays allow the part and I²C to settle, **`Wire`** is started, the sensor is initialized, then **`readCount`** events are read. Each valid sample updates **running sums and averages** for temperature and humidity. **`Wire`** is stopped and the power pin is driven low when done.

### Averaging and errors

`AhtManager` aggregates successful `getEvent()` samples into **`tempStats`** and **`humidityStats`** (sum, count, average). Samples that fail validation are skipped. If **`aht.begin()`** fails after power-up, no readings are collected for that cycle (averages stay at default/zero).

### Deep sleep / EM4 (optional)

The sketch currently uses **`LowPower.sleep()`**, not EM4 **deep sleep**. If you **re-enable** `LowPower.deepSleep()` for maximum savings, waking from deep sleep can leave the system not fully ready when `setup()` runs; touching **Serial** or **I²C** too early has been observed to leave some boards stuck until reset. A **short `LowPower.sleep()` at the very start of `setup()`** (not present in the tree today) is a common mitigation—evaluate on your hardware.

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
| **ArduinoLowPower** | `LowPower.sleep()` (and optional deep sleep APIs if you enable them) |
| **Wire** | I²C (built into the core) |

## Configuration

Edit `src/app_config.h`:

| Constant | Meaning |
|----------|---------|
| `DEEP_SLEEP_MS` | Milliseconds for **`LowPower.sleep()`** between loop iterations (default **7000**). |
| `COUNT_OF_READS` | Samples per measurement before averaging (passed to `ahtManager.begin()`, default **3**). |
| `SENSOR_POWER` | GPIO that switches sensor power (default **`D2`**). |

## Project layout

| Path | Purpose |
|------|---------|
| `mg24-thermostat-sensor.ino` | Cycle counter, Serial, LED, sleep between iterations |
| `src/app_config.h` | Sleep interval, sample count, sensor power pin |
| `src/aht_manager.h` | `AhtManager` class (sensor power, I²C, averaging) |
| `src/aht_manager.cpp` | `AhtManager` implementation |

## Serial output

- Startup: `Running after waiting …` (USB Serial wait time).
- After each measurement: **average** temperature (°C and °F) and **average** humidity (%).
- `"Complete, going to sleep"` before the sketch returns to the wait/sleep between cycles.
- **Execution time** for the **current loop iteration** (measurement + printing), and the **cycle** number (monotonic while powered).

Open the serial monitor at **115200** baud after flashing; the sketch waits up to ~2 s for USB Serial before continuing.
