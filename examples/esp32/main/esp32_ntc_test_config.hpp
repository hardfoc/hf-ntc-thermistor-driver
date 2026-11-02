/**
 * @file esp32_ntc_test_config.hpp
 * @brief Hardware configuration for NTC thermistor driver on ESP32-C6
 *
 * This file contains the actual hardware configuration that is used by the HAL
 * and example applications. Modify these values to match your hardware setup.
 *
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */

#pragma once

#include <cstdint>

//==============================================================================
// COMPILE-TIME CONFIGURATION FLAGS
//==============================================================================

/**
 * @brief Enable detailed ADC reading logging
 *
 * @details
 * When enabled (set to 1), the mock/real ADC bus will log detailed
 * information about each ADC reading including:
 * - Raw ADC counts
 * - Converted voltage values
 * - Temperature calculation steps
 *
 * When disabled (set to 0), only basic error logging is performed.
 *
 * Default: 0 (disabled) - Set to 1 to enable for debugging
 */
#ifndef ESP32_NTC_ENABLE_DETAILED_ADC_LOGGING
#define ESP32_NTC_ENABLE_DETAILED_ADC_LOGGING 0
#endif

namespace NTC_TestConfig {

/**
 * @brief ADC Configuration for ESP32-C6
 *
 * ADC channel and resolution settings for reading the NTC thermistor.
 * ESP32-C6 has ADC1 with channels 0-6 (GPIO0-6).
 */
struct ADCConfig {
    static constexpr uint8_t CHANNEL = 0;               ///< ADC1 channel 0 (GPIO0)
    static constexpr uint8_t RESOLUTION_BITS = 12;       ///< ADC resolution (12-bit for ESP32-C6)
    static constexpr uint16_t MAX_COUNT = 4095;          ///< Maximum ADC count (2^12 - 1)
    static constexpr float REFERENCE_VOLTAGE = 3.3f;     ///< ADC reference voltage (V)
};

/**
 * @brief Thermistor Specifications
 *
 * Parameters for a standard 10kΩ NTC thermistor (B3950 type).
 * Modify these values to match your specific thermistor part.
 */
struct ThermistorSpecs {
    static constexpr float RESISTANCE_AT_25C = 10000.0f; ///< Nominal resistance at 25°C (Ω)
    static constexpr float BETA_VALUE = 3435.0f;         ///< B25/85 (K), TDK NTCG163JF103FT1S spec sheet, Table 2
    static constexpr float REFERENCE_TEMP_C = 25.0f;     ///< Reference temperature (°C)
    static constexpr float TOLERANCE_PERCENT = 1.0f;     ///< Resistance tolerance (%)
};

/**
 * @brief Voltage Divider Circuit Configuration
 *
 * Configuration for the resistor divider circuit used to read the NTC.
 * Standard configuration: VCC → R_SERIES → ADC_PIN → NTC → GND
 */
struct CircuitConfig {
    static constexpr float SERIES_RESISTANCE = 10000.0f;    ///< Series resistor value (Ω)
    static constexpr float SUPPLY_VOLTAGE = 3.3f;           ///< Supply voltage for divider (V)
    static constexpr float CALIBRATION_OFFSET = 0.0f;       ///< Temperature calibration offset (°C)
};

/**
 * @brief Sampling Configuration
 *
 * Parameters for ADC sampling and filtering.
 */
struct SamplingConfig {
    static constexpr uint8_t SAMPLE_COUNT = 16;          ///< Number of samples to average
    static constexpr uint16_t SAMPLE_DELAY_MS = 5;       ///< Delay between samples (ms)
    static constexpr bool ENABLE_FILTERING = true;        ///< Enable moving average filter
    static constexpr float FILTER_ALPHA = 0.3f;           ///< EMA filter coefficient (0-1)
};

/**
 * @brief Temperature Range Specifications (celsius)
 *
 * Valid operating temperature range for the thermistor.
 */
struct Temperature {
    static constexpr int16_t MIN_TEMPERATURE = -40;     ///< Minimum measurable temperature (°C)
    static constexpr int16_t MAX_TEMPERATURE = 125;     ///< Maximum measurable temperature (°C)
    static constexpr int16_t WARNING_THRESHOLD = 100;   ///< Temperature warning threshold (°C)
    static constexpr int16_t CRITICAL_THRESHOLD = 120;  ///< Critical temperature threshold (°C)
};

/**
 * @brief Timing Parameters
 *
 * Timing configuration for temperature readings.
 */
struct Timing {
    static constexpr uint16_t CONVERSION_DELAY_MS = 10;     ///< ADC conversion delay (ms)
    static constexpr uint16_t POLL_INTERVAL_MS = 1000;      ///< Temperature polling interval (ms)
};

/**
 * @brief Diagnostic Thresholds
 *
 * Thresholds for detecting sensor faults.
 */
struct Diagnostics {
    static constexpr uint16_t OPEN_CIRCUIT_COUNT = 4090;     ///< ADC count for open circuit detection
    static constexpr uint16_t SHORT_CIRCUIT_COUNT = 5;       ///< ADC count for short circuit detection
    static constexpr uint8_t MAX_RETRY_COUNT = 3;            ///< Maximum read retries
};

/**
 * @brief Test Configuration
 *
 * Default parameters for testing.
 */
struct TestConfig {
    static constexpr uint16_t READ_COUNT = 100;              ///< Number of reads per test
    static constexpr uint16_t TEST_DURATION_MS = 5000;       ///< Test duration (ms)
    static constexpr float TEMPERATURE_TOLERANCE = 1.0f;     ///< Temperature tolerance (°C)
};

/**
 * @brief Application-specific Configuration
 *
 * Configuration values that can be adjusted per application.
 */
struct AppConfig {
    // Logging
    static constexpr bool ENABLE_DEBUG_LOGGING = true;     ///< Enable detailed debug logs
    static constexpr bool ENABLE_ADC_LOGGING = false;      ///< Enable ADC reading logs

    // Performance
    static constexpr bool ENABLE_PERFORMANCE_MONITORING = true;  ///< Enable performance metrics
    static constexpr uint16_t STATS_REPORT_INTERVAL_MS = 10000;  ///< Statistics reporting interval

    // Error handling
    static constexpr bool ENABLE_AUTO_RECOVERY = true;     ///< Enable automatic error recovery
    static constexpr uint8_t MAX_ERROR_COUNT = 10;         ///< Maximum errors before failsafe
};

} // namespace NTC_TestConfig

/**
 * @brief Hardware configuration validation
 *
 * Compile-time checks to ensure configuration is valid.
 */
static_assert(NTC_TestConfig::ADCConfig::CHANNEL <= 6,
              "ADC channel exceeds ESP32-C6 maximum of 6");

static_assert(NTC_TestConfig::ADCConfig::RESOLUTION_BITS == 12,
              "ESP32-C6 ADC resolution should be 12 bits");

static_assert(NTC_TestConfig::ThermistorSpecs::BETA_VALUE > 0,
              "Beta value must be positive");

static_assert(NTC_TestConfig::CircuitConfig::SERIES_RESISTANCE > 0,
              "Series resistance must be positive");

/**
 * @brief Helper macro for compile-time GPIO pin validation
 */
#define NTC_VALIDATE_GPIO(pin) \
    static_assert((pin) >= 0 && (pin) < 30, "Invalid GPIO pin number for ESP32-C6")
