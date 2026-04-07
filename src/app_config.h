#pragma once

#include <Arduino.h>
#include <string>

namespace AppConfig {
    static constexpr uint64_t DEEP_SLEEP_MS  = 7000UL; // 7 Seconds
    static constexpr uint32_t COUNT_OF_READS = 3;

    static constexpr pin_size_t SENSOR_POWER = D2;

    // BLE
    static constexpr uint16_t COMPANY_ID = 0xFFFF;
    static constexpr uint32_t ADVERTISE_WINDOW_MS = 3000;
    // 160 units = 100 ms, since BLE advertising interval units are 0.625 ms.
    static constexpr uint16_t ADV_INTERVAL_UNITS = 160;
    static constexpr const char* BLE_LOCAL_NAME = "MG24TS";
};