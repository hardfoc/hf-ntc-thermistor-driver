/**
 * @file ntc_thermistor.hpp
 * @brief Hardware-agnostic NTC thermistor driver for temperature measurement
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */
#pragma once
#include <cstdint>
#include <memory>

#include "ntc_adc_interface.hpp"
#include "ntc_thermistor_version.h"
#include "ntc_types.hpp"

//--------------------------------------
//  NtcThermistor Class
//--------------------------------------

/**
 * @class NtcThermistor
 * @brief Hardware-agnostic NTC thermistor driver
 *
 * This class provides a comprehensive interface for temperature measurement
 * using NTC thermistors. It supports various NTC types and provides both
 * lookup table and mathematical conversion methods.
 *
 * Key features:
 * - Hardware-agnostic design using CRTP-based ADC interface
 * - Support for multiple NTC types
 * - Dual conversion methods (lookup table and mathematical)
 * - Built-in calibration and filtering
 * - Comprehensive error handling
 *
 * @tparam AdcType The ADC implementation type that inherits from
 * ntc::AdcInterface<AdcType>
 *
 * @note The driver requires an ADC type that implements ntc::AdcInterface
 *
 * @par Basic usage
 * @code
 * // Define your ADC implementation
 * class MyAdc : public ntc::AdcInterface<MyAdc> {
 * public:
 *   bool IsInitialized() const { return initialized_; }
 *   bool EnsureInitialized() { return true; }
 * // ... implement other required methods
 * };
 *
 * // Create ADC instance
 * MyAdc my_adc;
 * my_adc.EnsureInitialized();
 * // Create NTC thermistor with default configuration
 * NtcThermistor<MyAdc> thermistor(NtcType::NtcG163Jf103Ft1S, &my_adc);
 * // Initialize and read temperature
 *if (thermistor.Initialize()) {
 * float temp_c;
 * if (thermistor.ReadTemperatureCelsius(&temp_c) == NtcError::Success) {
 * printf("Temperature: %.2F°C\n", temp_c);
 * }
 * }
 * @endcode
 * @par Custom configuration
 * @code
 * // Create custom configuration
 * ntc_config_t config = GetDefaultNtcG163Jft103Ft1SConfig();
 * config.adc_channel = 1;
 * config.series_resistance = 10000.0F;
 * config.conversion_method = NtcConversionMethod::Mathematical;
 * config.enable_filtering = true;
 * config.filter_alpha = 0.1F;
 *
 * NtcThermistor<MyAdc> thermistor(config, &my_adc);
 * thermistor.Initialize();
 * @endcode
 */
template <typename AdcType> class NtcThermistor {
public:
  //==============================================================//
  // CONSTRUCTORS AND DESTRUCTOR
  //==============================================================//

  /**
   * @brief Constructor with NTC type and ADC interface
   *
   * Creates an NTC thermistor instance with default configuration for the
   * specified NTC type. The configuration is automatically initialized with
   * appropriate values for the thermistor type.
   *
   * @param ntc_type NTC thermistor type (e.g., NtcType::NtcG163Jf103Ft1S)
   * @param adc_interface Pointer to ADC interface (must inherit from
   * ntc::AdcInterface<AdcType>)
   *
   * @note The ADC interface must remain valid for the lifetime of the
   *       NtcThermistor instance. The driver does not take ownership.
   *
   * @warning The ADC interface pointer must not be nullptr.
   */
  NtcThermistor(NtcType ntc_type, AdcType *adc_interface) noexcept;

  /**
   * @brief Constructor with custom configuration
   * @param config NTC configuration structure
   * @param adc_interface Pointer to ADC interface (must inherit from
   * ntc::AdcInterface<AdcType>)
   */
  NtcThermistor(const ntc_config_t &config, AdcType *adc_interface) noexcept;

  /**
   * @brief Copy constructor is deleted
   */
  NtcThermistor(const NtcThermistor &) = delete;

  /**
   * @brief Assignment operator is deleted
   */
  NtcThermistor &operator=(const NtcThermistor &) = delete;

  /**
   * @brief Move constructor
   */
  NtcThermistor(NtcThermistor &&) noexcept = default;

  /**
   * @brief Move assignment operator
   */
  NtcThermistor &operator=(NtcThermistor &&) noexcept = default;

  /**
   * @brief Virtual destructor
   */
  virtual ~NtcThermistor() noexcept = default;

  //==============================================================//
  // INITIALIZATION AND CONFIGURATION
  //==============================================================//

  /**
   * @brief Initialize the NTC thermistor driver
   * @return true if successful, false otherwise
   */
  bool Initialize() noexcept;

  /**
   * @brief Deinitialize the NTC thermistor driver
   * @return true if successful, false otherwise
   */
  bool Deinitialize() noexcept;

  /**
   * @brief Check if initialized
   * @return true if initialized, false otherwise
   */
  [[nodiscard]] bool IsInitialized() const noexcept;

  /**
   * @brief Get current configuration
   * @param config Pointer to store configuration
   * @return Error code
   */
  NtcError GetConfiguration(ntc_config_t *config) const noexcept;

  /**
   * @brief Set configuration
   * @param config New configuration
   * @return Error code
   */
  NtcError SetConfiguration(const ntc_config_t &config) noexcept;

  //==============================================================//
  // TEMPERATURE READING
  //==============================================================//

  /**
   * @brief Read temperature in Celsius
   *
   * Performs a complete temperature reading cycle: reads ADC, calculates
   * resistance, converts to temperature, applies filtering (if enabled),
   * and validates the result.
   *
   * @param temperature_celsius Pointer to store temperature (°C)
   * @return Error code (NtcError::Success on success)
   *
   * @note This function may take several milliseconds depending on sample_count
   *       and sample_delay_ms configuration.
   *
   * @warning The driver must be initialized before calling this function.
   *          Check return value before using the temperature value.
   */
  NtcError ReadTemperatureCelsius(float *temperature_celsius) noexcept;

  /**
   * @brief Read temperature in Fahrenheit
   * @param temperature_fahrenheit Pointer to store temperature
   * @return Error code
   */
  NtcError ReadTemperatureFahrenheit(float *temperature_fahrenheit) noexcept;

  /**
   * @brief Read temperature in Kelvin
   * @param temperature_kelvin Pointer to store temperature
   * @return Error code
   */
  NtcError ReadTemperatureKelvin(float *temperature_kelvin) noexcept;

  /**
   * @brief Read complete temperature information
   * @param reading Pointer to store reading information
   * @return Error code
   */
  NtcError ReadTemperature(ntc_reading_t *reading) noexcept;

  //==============================================================//
  // RESISTANCE AND VOLTAGE
  //==============================================================//

  /**
   * @brief Get thermistor resistance
   * @param resistance_ohms Pointer to store resistance
   * @return Error code
   */
  NtcError GetResistance(float *resistance_ohms) noexcept;

  /**
   * @brief Get voltage across thermistor
   * @param voltage_volts Pointer to store voltage
   * @return Error code
   */
  NtcError GetVoltage(float *voltage_volts) noexcept;

  /**
   * @brief Get raw ADC value
   * @param adc_value Pointer to store ADC value
   * @return Error code
   */
  NtcError GetRawAdcValue(uint32_t *adc_value) noexcept;

  //==============================================================//
  // CALIBRATION
  //==============================================================//

  /**
   * @brief Calibrate the thermistor
   * @param reference_temperature_celsius Known reference temperature
   * @return Error code
   */
  NtcError Calibrate(float reference_temperature_celsius) noexcept;

  /**
   * @brief Set calibration offset
   * @param offset_celsius Calibration offset in Celsius
   * @return Error code
   */
  NtcError SetCalibrationOffset(float offset_celsius) noexcept;

  /**
   * @brief Get calibration offset
   * @param offset_celsius Pointer to store offset
   * @return Error code
   */
  NtcError GetCalibrationOffset(float *offset_celsius) const noexcept;

  /**
   * @brief Reset calibration
   * @return Error code
   */
  NtcError ResetCalibration() noexcept;

  //==============================================================//
  // CONFIGURATION METHODS
  //==============================================================//

  /**
   * @brief Set conversion method
   * @param method Conversion method
   * @return Error code
   */
  NtcError SetConversionMethod(NtcConversionMethod method) noexcept;

  /**
   * @brief Set voltage divider parameters
   * @param series_resistance Series resistance (ohms)
   * @return Error code
   */
  NtcError SetVoltageDivider(float series_resistance) noexcept;

  /**
   * @brief Set reference voltage
   * @param reference_voltage Reference voltage (V)
   * @return Error code
   */
  NtcError SetReferenceVoltage(float reference_voltage) noexcept;

  /**
   * @brief Set beta value
   * @param beta_value Beta value (K)
   * @return Error code
   */
  NtcError SetBetaValue(float beta_value) noexcept;

  /**
   * @brief Set ADC channel
   * @param adc_channel ADC channel number
   * @return Error code
   */
  NtcError SetAdcChannel(uint8_t adc_channel) noexcept;

  /**
   * @brief Set sampling parameters
   * @param sample_count Number of samples
   * @param sample_delay_ms Delay between samples (ms)
   * @return Error code
   */
  NtcError SetSamplingParameters(uint32_t sample_count,
                                 uint32_t sample_delay_ms) noexcept;

  /**
   * @brief Enable/disable filtering
   * @param enable Enable filtering
   * @param alpha Filter alpha value (0.0-1.0)
   * @return Error code
   */
  NtcError SetFiltering(bool enable, float alpha = 0.1F) noexcept;

  //==============================================================//
  // UTILITY FUNCTIONS
  //==============================================================//

  /**
   * @brief Convert Celsius to Fahrenheit
   * @param celsius Temperature in Celsius
   * @return Temperature in Fahrenheit
   */
  static float CelsiusToFahrenheit(float celsius) noexcept;

  /**
   * @brief Convert Fahrenheit to Celsius
   * @param fahrenheit Temperature in Fahrenheit
   * @return Temperature in Celsius
   */
  static float FahrenheitToCelsius(float fahrenheit) noexcept;

  /**
   * @brief Convert Celsius to Kelvin
   * @param celsius Temperature in Celsius
   * @return Temperature in Kelvin
   */
  static float CelsiusToKelvin(float celsius) noexcept;

  /**
   * @brief Convert Kelvin to Celsius
   * @param kelvin Temperature in Kelvin
   * @return Temperature in Celsius
   */
  static float KelvinToCelsius(float kelvin) noexcept;

  /**
   * @brief Get error string
   * @param error Error code
   * @return Error string
   */
  static const char *GetErrorString(NtcError error) noexcept;

  /**
   * @brief Get NTC type string
   * @param type NTC type
   * @return Type string
   */
  static const char *GetTypeString(NtcType type) noexcept;

  // ===========================================================================
  // Driver Version
  // ===========================================================================

  /** @brief Get the compiled driver version string. */
  static constexpr const char* GetDriverVersion() noexcept {
    return HF_NTC_THERMISTOR_VERSION_STRING;
  }

  /** @brief Get the compiled driver major version number. */
  static constexpr uint8_t GetDriverVersionMajor() noexcept {
    return HF_NTC_THERMISTOR_VERSION_MAJOR;
  }

  /** @brief Get the compiled driver minor version number. */
  static constexpr uint8_t GetDriverVersionMinor() noexcept {
    return HF_NTC_THERMISTOR_VERSION_MINOR;
  }

  /** @brief Get the compiled driver patch version number. */
  static constexpr uint8_t GetDriverVersionPatch() noexcept {
    return HF_NTC_THERMISTOR_VERSION_PATCH;
  }

private:
  //==============================================================//
  // PRIVATE MEMBER VARIABLES
  //==============================================================//

  ntc_config_t config_;    ///< NTC configuration
  AdcType *adc_interface_; ///< ADC interface pointer
  bool initialized_;       ///< Initialization status

  // Filtering
  float filtered_temperature_; ///< Filtered temperature
  bool filter_initialized_;    ///< Filter initialization status

  //==============================================================//
  // PRIVATE HELPER METHODS
  //==============================================================//

  /**
   * @brief Validate configuration
   * @param config Configuration to validate
   * @return Error code
   */
  [[nodiscard]] NtcError
  validateConfiguration(const ntc_config_t &config) const noexcept;

  /**
   * @brief Read ADC voltage
   * @param voltage_volts Pointer to store voltage
   * @return Error code
   */
  NtcError readAdcVoltage(float *voltage_volts) noexcept;

  /**
   * @brief Calculate resistance from voltage
   * @param voltage_volts Voltage across thermistor
   * @param resistance_ohms Pointer to store resistance
   * @return Error code
   */
  NtcError calculateResistance(float voltage_volts,
                               float *resistance_ohms) noexcept;

  /**
   * @brief Convert resistance to temperature
   * @param resistance_ohms Thermistor resistance
   * @param temperature_celsius Pointer to store temperature
   * @return Error code
   */
  NtcError convertResistanceToTemperature(float resistance_ohms,
                                          float *temperature_celsius) noexcept;

  /**
   * @brief Apply filtering
   * @param new_temperature New temperature reading
   * @return Filtered temperature
   */
  float applyFiltering(float new_temperature) noexcept;

  /**
   * @brief Initialize configuration for NTC type
   * @param ntc_type NTC type
   * @param config Pointer to configuration to initialize
   */
  static void initializeConfigForType(NtcType ntc_type,
                                      ntc_config_t *config) noexcept;
};

// =============================================================================
// Free-function version accessor (no namespace — class is at global scope)
// =============================================================================

/** @brief Get the NTC Thermistor driver version string (free function). */
inline const char* GetNtcThermistorDriverVersion() noexcept {
  return HF_NTC_THERMISTOR_VERSION_STRING;
}

// Include template implementation
#define NTC_THERMISTOR_HEADER_INCLUDED
// NOLINTNEXTLINE(bugprone-suspicious-include) - Template implementation file
#include "../src/ntc_thermistor.ipp"
#undef NTC_THERMISTOR_HEADER_INCLUDED
