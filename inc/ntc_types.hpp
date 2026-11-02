/**
 * @file ntc_types.hpp
 * @brief Type definitions for NTC thermistor driver
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */
#pragma once
#include <cstdint>

//--------------------------------------
//  NTC Error Codes
//--------------------------------------

/**
 * @brief NTC thermistor error codes
 *
 * Error codes returned by NTC thermistor driver functions. All functions
 * that can fail return an NtcError value indicating success or the specific
 * error condition.
 *
 * @note Always check return values for error conditions before using results.
 *
 * @see NtcThermistor::GetErrorString() for human-readable error messages
 */
enum class NtcError : uint8_t {
  Success = 0,               ///< Operation successful
  Failure = 1,               ///< General failure
  NotInitialized = 2,        ///< Not initialized
  AlreadyInitialized = 3,    ///< Already initialized
  InvalidParameter = 4,      ///< Invalid parameter
  NullPointer = 5,           ///< Null pointer
  OutOfMemory = 6,           ///< Out of memory
  AdcReadFailed = 7,         ///< ADC read failed
  InvalidResistance = 8,     ///< Invalid resistance value
  TemperatureOutOfRange = 9, ///< Temperature out of range
  LookupTableError = 10,     ///< Lookup table error
  ConversionFailed = 11,     ///< Temperature conversion failed
  CalibrationFailed = 12,    ///< Calibration failed
  UnsupportedOperation = 13, ///< Operation not supported
  Timeout = 14,              ///< Operation timeout
  HardwareFault = 15,        ///< Hardware fault
  Max = 16                   ///< Maximum error code
};

//--------------------------------------
//  NTC Types
//--------------------------------------

/**
 * @brief NTC thermistor types
 *
 * Supported NTC thermistor types. Each type has predefined characteristics
 * (resistance at 25°C, beta value) that are automatically configured
 * when using the type-specific constructor.
 *
 * @note Use NtcType::Custom for thermistors not in this list, and configure
 *       resistance_at_25c and beta_value manually.
 */
enum class NtcType : uint8_t {
  Unknown = 0,           ///< Unknown type
  NtcG163Jft103Ft1S = 1, ///< NTCG163JFT103FT1S (10kΩ @ 25°C, β=3435K)
  NtcG164Jf103Ft1S = 2,  ///< NTCG164JF103FT1S (10kΩ @ 25°C, β=3435K)
  NtcG163Jf103Ft1S = 3,  ///< NTCG163JF103FT1S (10kΩ @ 25°C, β=3435K)
  Custom = 4,            ///< Custom NTC type
  Max = 5                ///< Maximum type
};

/**
 * @brief Temperature conversion methods
 *
 * Methods for converting resistance to temperature. The driver supports
 * two conversion approaches with different trade-offs:
 *
 * - **Lookup Table**: Fast, pre-calculated values, slightly less accurate
 * - **Mathematical**: Slower, uses Steinhart-Hart or Beta equations, more
 * accurate
 * - **Auto**: Automatically selects the best method based on configuration
 *
 * @note For real-time applications requiring high update rates, use
 *       NtcConversionMethod::LookupTable. For maximum accuracy, use
 *       NtcConversionMethod::Mathematical.
 */
enum class NtcConversionMethod : uint8_t {
  LookupTable = 0,  ///< Use lookup table (faster, less accurate)
  Mathematical = 1, ///< Use mathematical conversion (slower, more accurate)
  Auto = 2          ///< Auto-select based on accuracy requirements
};

//--------------------------------------
//  NTC Configuration
//--------------------------------------

/**
 * @brief NTC thermistor configuration structure
 *
 * Complete configuration for an NTC thermistor instance. All parameters
 * must be set appropriately for accurate temperature readings.
 *
 * @note Use GetDefaultNtcConfig() or GetDefaultNtcG163Jft103Ft1SConfig()
 *       functions to initialize with sensible defaults, then modify as needed.
 *
 * @warning Invalid configuration values (e.g., negative resistance) will
 *          cause initialization or reading functions to fail.
 *
 * @see NtcThermistor::SetConfiguration()
 * @see NtcThermistor::GetConfiguration()
 */
struct ntc_config_t {
  NtcType type;             ///< NTC type
  float resistance_at_25c;  ///< Resistance at 25°C (ohms)
  float beta_value;         ///< Beta value (K)
  float reference_voltage;  ///< Reference voltage (V)
  float series_resistance;  ///< Series resistance in voltage divider (ohms)
  float calibration_offset; ///< Calibration offset (°C)
  NtcConversionMethod conversion_method; ///< Conversion method
  uint8_t adc_channel;                   ///< ADC channel number
  uint32_t adc_resolution_bits;          ///< ADC resolution in bits
  uint32_t sample_count;                 ///< Number of samples to average
  uint32_t sample_delay_ms;              ///< Delay between samples (ms)
  float min_temperature;                 ///< Minimum temperature (°C)
  float max_temperature;                 ///< Maximum temperature (°C)
  bool enable_filtering;                 ///< Enable temperature filtering
  float filter_alpha;                    ///< Filter alpha value (0.0-1.0)
};

/**
 * @brief NTC thermistor reading structure
 *
 * Complete temperature reading information including all temperature units,
 * raw ADC values, resistance, voltage, and error status.
 *
 * @note Always check the is_valid and error fields before using temperature
 *       values. Invalid readings may contain NaN or invalid data.
 *
 * @see NtcThermistor::ReadTemperature()
 */
struct ntc_reading_t {
  float temperature_celsius;    ///< Temperature in Celsius
  float temperature_fahrenheit; ///< Temperature in Fahrenheit
  float temperature_kelvin;     ///< Temperature in Kelvin
  float resistance_ohms;        ///< Thermistor resistance (ohms)
  float voltage_volts;          ///< Voltage across thermistor (V)
  uint32_t adc_raw_value;       ///< Raw ADC value
  uint64_t timestamp_us;        ///< Timestamp (microseconds)
  NtcError error;               ///< Error code
  bool is_valid;                ///< Whether reading is valid
  float accuracy_celsius;       ///< Estimated accuracy (°C)
};

//--------------------------------------
//  Default Configurations
//--------------------------------------

/**
 * @brief Default NTC configuration constants
 */
namespace NTC::DefaultConfig {
constexpr float DEFAULT_RESISTANCE_AT_25C_ =
    10000.0F; ///< Default resistance at 25°C (10kΩ)
constexpr float DEFAULT_BETA_VALUE_ = 3435.0F; ///< Default beta value (3435K)
constexpr float DEFAULT_REFERENCE_VOLTAGE_ =
    3.3F; ///< Default reference voltage (3.3V)
constexpr float DEFAULT_SERIES_RESISTANCE_ =
    10000.0F; ///< Default series resistance (10kΩ)
constexpr float DEFAULT_CALIBRATION_OFFSET_ =
    0.0F;                                   ///< Default calibration offset
constexpr uint8_t DEFAULT_ADC_CHANNEL_ = 0; ///< Default ADC channel
constexpr uint32_t DEFAULT_ADC_RESOLUTION_BITS_ =
    12U; ///< Default ADC resolution (12-bit)
constexpr uint32_t DEFAULT_SAMPLE_COUNT_ = 1U;    ///< Default sample count
constexpr uint32_t DEFAULT_SAMPLE_DELAY_MS_ = 0U; ///< Default sample delay (ms)
constexpr float DEFAULT_MIN_TEMPERATURE_ =
    -40.0F; ///< Default minimum temperature (°C)
constexpr float DEFAULT_MAX_TEMPERATURE_ =
    125.0F; ///< Default maximum temperature (°C)
constexpr bool DEFAULT_ENABLE_FILTERING_ = false; ///< Default filtering enabled
constexpr float DEFAULT_FILTER_ALPHA_ = 0.1F; ///< Default filter alpha value
} // namespace NTC::DefaultConfig

/**
 * @brief Default NTC configuration for NTCG163JFT103FT1S
 *
 * Initializes a configuration structure with default values for the
 * NTCG163JFT103FT1S thermistor (10kΩ @ 25°C, β=3435K).
 *
 * @return ntc_config_t with default values for NTCG163JFT103FT1S
 *
 * @par Example
 * @code
 * ntc_config_t config = GetDefaultNtcG163Jft103Ft1SConfig();
 * config.adc_channel = 1;  // Override default channel
 * NtcThermistor<MyAdc> thermistor(config, &my_adc);
 * @endcode
 */
[[nodiscard]] constexpr ntc_config_t
GetDefaultNtcG163Jft103Ft1SConfig() noexcept {
  return {.type = NtcType::NtcG163Jft103Ft1S,
          .resistance_at_25c = NTC::DefaultConfig::DEFAULT_RESISTANCE_AT_25C_,
          .beta_value = NTC::DefaultConfig::DEFAULT_BETA_VALUE_,
          .reference_voltage = NTC::DefaultConfig::DEFAULT_REFERENCE_VOLTAGE_,
          .series_resistance = NTC::DefaultConfig::DEFAULT_SERIES_RESISTANCE_,
          .calibration_offset = NTC::DefaultConfig::DEFAULT_CALIBRATION_OFFSET_,
          .conversion_method = NtcConversionMethod::Auto,
          .adc_channel = NTC::DefaultConfig::DEFAULT_ADC_CHANNEL_,
          .adc_resolution_bits =
              NTC::DefaultConfig::DEFAULT_ADC_RESOLUTION_BITS_,
          .sample_count = NTC::DefaultConfig::DEFAULT_SAMPLE_COUNT_,
          .sample_delay_ms = NTC::DefaultConfig::DEFAULT_SAMPLE_DELAY_MS_,
          .min_temperature = NTC::DefaultConfig::DEFAULT_MIN_TEMPERATURE_,
          .max_temperature = NTC::DefaultConfig::DEFAULT_MAX_TEMPERATURE_,
          .enable_filtering = NTC::DefaultConfig::DEFAULT_ENABLE_FILTERING_,
          .filter_alpha = NTC::DefaultConfig::DEFAULT_FILTER_ALPHA_};
}

/**
 * @brief Default NTC configuration
 *
 * Convenience function that defaults to NTCG163JFT103FT1S configuration.
 * Equivalent to GetDefaultNtcG163Jft103Ft1SConfig().
 *
 * @return ntc_config_t with default values
 *
 * @see GetDefaultNtcG163Jft103Ft1SConfig()
 */
[[nodiscard]] constexpr ntc_config_t GetDefaultNtcConfig() noexcept {
  return GetDefaultNtcG163Jft103Ft1SConfig();
}
