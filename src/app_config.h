#pragma once

#include <Arduino.h>

namespace AppConfig {
    static constexpr uint64_t DEEP_SLEEP_MS  = 7000UL; // 7 Seconds
    static constexpr uint32_t COUNT_OF_READS = 3;

    static constexpr pin_size_t SENSOR_POWER = D2;
};