/**
 * @file ntc_thermistor.ipp
 * @brief Hardware-agnostic NTC thermistor driver implementation
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */
#ifndef NTC_THERMISTOR_IMPL
#define NTC_THERMISTOR_IMPL

// When included from header, use relative path; when compiled directly, use
// standard include
#ifdef NTC_THERMISTOR_HEADER_INCLUDED
#include "../inc/ntc_thermistor.hpp"
#include "../inc/ntc_conversion.hpp"
#include "../inc/ntc_lookup_table.hpp"
#else
#include "ntc_conversion.hpp"
#include "ntc_lookup_table.hpp"
#include "ntc_thermistor.hpp"
#endif

#include <algorithm>
#include <cmath>
#include <cstring>

using NTC::Constants::CELSIUS_TO_FAHRENHEIT_MULTIPLIER_;
using NTC::Constants::FAHRENHEIT_OFFSET_;
using NTC::Constants::FAHRENHEIT_TO_CELSIUS_MULTIPLIER_;
using NTC::Constants::KELVIN_OFFSET_;
using NTC::Constants::ONE_FLOAT_;
using NTC::Constants::ZERO_FLOAT_;

//--------------------------------------
//  CONSTRUCTORS AND DESTRUCTOR
//--------------------------------------

template <typename AdcType>
NtcThermistor<AdcType>::NtcThermistor(NtcType ntc_type,
                                      AdcType *adc_interface) noexcept
    : config_(), adc_interface_(adc_interface), initialized_(false),
      filtered_temperature_(ZERO_FLOAT_), filter_initialized_(false) {

  // Initialize configuration for NTC type
  initializeConfigForType(ntc_type, &config_);
}

template <typename AdcType>
NtcThermistor<AdcType>::NtcThermistor(const ntc_config_t &config,
                                      AdcType *adc_interface) noexcept
    : config_(config), adc_interface_(adc_interface), initialized_(false),
      filtered_temperature_(ZERO_FLOAT_), filter_initialized_(false) {}

//--------------------------------------
//  INITIALIZATION AND CONFIGURATION
//--------------------------------------

template <typename AdcType> bool NtcThermistor<AdcType>::Initialize() noexcept {
  if (initialized_) {
    return true;
  }

  // Validate configuration
  NtcError validation_error = validateConfiguration(config_);
  if (validation_error != NtcError::Success) {
    return false;
  }

  // Validate ADC interface
  if (adc_interface_ == nullptr) {
    return false;
  }

  // Initialize ADC interface if needed
  if (!adc_interface_->IsInitialized()) {
    if (!adc_interface_->EnsureInitialized()) {
      return false;
    }
  }

  // Validate ADC channel
  if (!adc_interface_->IsChannelAvailable(config_.adc_channel)) {
    return false;
  }

  // Reset filter
  filter_initialized_ = false;
  filtered_temperature_ = ZERO_FLOAT_;

  initialized_ = true;
  return true;
}

template <typename AdcType>
bool NtcThermistor<AdcType>::Deinitialize() noexcept {
  if (!initialized_) {
    return true;
  }

  initialized_ = false;
  filter_initialized_ = false;
  filtered_temperature_ = ZERO_FLOAT_;

  return true;
}

template <typename AdcType>
bool NtcThermistor<AdcType>::IsInitialized() const noexcept {
  return initialized_;
}

template <typename AdcType>
NtcError
NtcThermistor<AdcType>::GetConfiguration(ntc_config_t *config) const noexcept {
  if (config == nullptr) {
    return NtcError::NullPointer;
  }

  *config = config_;
  return NtcError::Success;
}

template <typename AdcType>
NtcError
NtcThermistor<AdcType>::SetConfiguration(const ntc_config_t &config) noexcept {
  // Validate new configuration
  NtcError validation_error = validateConfiguration(config);
  if (validation_error != NtcError::Success) {
    return validation_error;
  }

  config_ = config;

  // Reset filter when configuration changes
  filter_initialized_ = false;
  filtered_temperature_ = ZERO_FLOAT_;

  return NtcError::Success;
}

//--------------------------------------
//  TEMPERATURE READING
//--------------------------------------

template <typename AdcType>
NtcError NtcThermistor<AdcType>::ReadTemperatureCelsius(
    float *temperature_celsius) noexcept {
  if (temperature_celsius == nullptr) {
    return NtcError::NullPointer;
  }

  if (!initialized_) {
    return NtcError::NotInitialized;
  }

  // Read voltage from ADC
  float voltage_volts = 0.0F;
  NtcError voltage_error = readAdcVoltage(&voltage_volts);
  if (voltage_error != NtcError::Success) {
    return voltage_error;
  }

  // Calculate resistance
  float resistance_ohms = 0.0F;
  NtcError resistance_error =
      calculateResistance(voltage_volts, &resistance_ohms);
  if (resistance_error != NtcError::Success) {
    return resistance_error;
  }

  // Convert resistance to temperature
  float raw_temperature = 0.0F;
  NtcError conversion_error =
      convertResistanceToTemperature(resistance_ohms, &raw_temperature);
  if (conversion_error != NtcError::Success) {
    return conversion_error;
  }

  // Apply calibration offset
  *temperature_celsius = raw_temperature + config_.calibration_offset;

  // Apply filtering if enabled
  if (config_.enable_filtering) {
    *temperature_celsius = applyFiltering(*temperature_celsius);
  }

  // Validate temperature range
  if (!NTC::ValidateTemperature(*temperature_celsius, config_.min_temperature,
                                config_.max_temperature)) {
    return NtcError::TemperatureOutOfRange;
  }

  return NtcError::Success;
}

template <typename AdcType>
NtcError NtcThermistor<AdcType>::ReadTemperatureFahrenheit(
    float *temperature_fahrenheit) noexcept {
  if (temperature_fahrenheit == nullptr) {
    return NtcError::NullPointer;
  }

  float celsius = 0.0F;
  NtcError error = ReadTemperatureCelsius(&celsius);
  if (error == NtcError::Success) {
    *temperature_fahrenheit = CelsiusToFahrenheit(celsius);
  }
  return error;
}

template <typename AdcType>
NtcError NtcThermistor<AdcType>::ReadTemperatureKelvin(
    float *temperature_kelvin) noexcept {
  if (temperature_kelvin == nullptr) {
    return NtcError::NullPointer;
  }

  float celsius = 0.0F;
  NtcError error = ReadTemperatureCelsius(&celsius);
  if (error == NtcError::Success) {
    *temperature_kelvin = CelsiusToKelvin(celsius);
  }
  return error;
}

template <typename AdcType>
NtcError
NtcThermistor<AdcType>::ReadTemperature(ntc_reading_t *reading) noexcept {
  if (reading == nullptr) {
    return NtcError::NullPointer;
  }

  // Initialize reading structure
  reading->error = NtcError::Success;
  reading->is_valid = false;
  reading->timestamp_us = 0;        // Implementer should provide timestamp
  reading->accuracy_celsius = 0.5F; // Estimate based on typical NTC accuracy

  // Read temperature
  float temperature_celsius = 0.0F;
  NtcError error = ReadTemperatureCelsius(&temperature_celsius);

  if (error == NtcError::Success) {
    // Get additional data
    float resistance_ohms = 0.0F;
    GetResistance(&resistance_ohms);

    float voltage_volts = 0.0F;
    GetVoltage(&voltage_volts);

    uint32_t adc_raw = 0U;
    GetRawAdcValue(&adc_raw);

    // Fill reading structure
    reading->temperature_celsius = temperature_celsius;
    reading->temperature_fahrenheit = CelsiusToFahrenheit(temperature_celsius);
    reading->temperature_kelvin = CelsiusToKelvin(temperature_celsius);
    reading->resistance_ohms = resistance_ohms;
    reading->voltage_volts = voltage_volts;
    reading->adc_raw_value = adc_raw;
    reading->is_valid = true;
  }

  reading->error = error;
  return error;
}

//--------------------------------------
//  RESISTANCE AND VOLTAGE
//--------------------------------------

template <typename AdcType>
NtcError
NtcThermistor<AdcType>::GetResistance(float *resistance_ohms) noexcept {
  if (resistance_ohms == nullptr) {
    return NtcError::NullPointer;
  }

  if (!initialized_) {
    return NtcError::NotInitialized;
  }

  float voltage_volts = 0.0F;
  NtcError voltage_error = readAdcVoltage(&voltage_volts);
  if (voltage_error != NtcError::Success) {
    return voltage_error;
  }

  return calculateResistance(voltage_volts, resistance_ohms);
}

template <typename AdcType>
NtcError NtcThermistor<AdcType>::GetVoltage(float *voltage_volts) noexcept {
  if (voltage_volts == nullptr) {
    return NtcError::NullPointer;
  }

  if (!initialized_) {
    return NtcError::NotInitialized;
  }

  return readAdcVoltage(voltage_volts);
}

template <typename AdcType>
NtcError NtcThermistor<AdcType>::GetRawAdcValue(uint32_t *adc_value) noexcept {
  if (adc_value == nullptr) {
    return NtcError::NullPointer;
  }

  if (!initialized_) {
    return NtcError::NotInitialized;
  }

  if (adc_interface_ == nullptr) {
    return NtcError::NullPointer;
  }

  // Convert AdcError to NtcError helper
  auto convert_error = [](ntc::AdcError err) -> NtcError {
    switch (err) {
    case ntc::AdcError::Success:
      return NtcError::Success;
    case ntc::AdcError::NotInitialized:
      return NtcError::NotInitialized;
    case ntc::AdcError::InvalidChannel:
      return NtcError::InvalidParameter;
    case ntc::AdcError::ReadFailed:
      return NtcError::AdcReadFailed;
    case ntc::AdcError::Timeout:
      return NtcError::Timeout;
    case ntc::AdcError::HardwareError:
      return NtcError::HardwareFault;
    default:
      return NtcError::AdcReadFailed;
    }
  };

  // Perform sampling if configured
  if (config_.sample_count > 1) {
    uint64_t sum = 0;
    uint32_t successful_samples = 0;

    for (uint32_t i = 0; i < config_.sample_count; ++i) {
      uint32_t sample_value = 0;
      ntc::AdcError err =
          adc_interface_->ReadChannelCount(config_.adc_channel, &sample_value);
      if (err == ntc::AdcError::Success) {
        sum += sample_value;
        successful_samples++;
      }

      if (i + 1 < config_.sample_count) {
        delayBetweenSamples();
      }
    }

    if (successful_samples == 0) {
      return NtcError::AdcReadFailed;
    }

    *adc_value = static_cast<uint32_t>(sum / successful_samples);
    return NtcError::Success;
  }
  // Single sample
  {
    uint32_t sample_value = 0;
    ntc::AdcError err =
        adc_interface_->ReadChannelCount(config_.adc_channel, &sample_value);
    if (err != ntc::AdcError::Success) {
      return convert_error(err);
    }
    *adc_value = sample_value;
    return NtcError::Success;
  }
}

//--------------------------------------
//  CALIBRATION
//--------------------------------------

template <typename AdcType>
NtcError NtcThermistor<AdcType>::Calibrate(
    float reference_temperature_celsius) noexcept {
  if (!initialized_) {
    return NtcError::NotInitialized;
  }

  // Read current temperature
  float current_temperature = 0.0F;
  NtcError read_error = ReadTemperatureCelsius(&current_temperature);
  if (read_error != NtcError::Success) {
    return read_error;
  }

  // The reading already includes the current offset, so add the remaining
  // error to it rather than replacing it.
  config_.calibration_offset +=
      reference_temperature_celsius - current_temperature;

  return NtcError::Success;
}

template <typename AdcType>
NtcError
NtcThermistor<AdcType>::SetCalibrationOffset(float offset_celsius) noexcept {
  config_.calibration_offset = offset_celsius;
  return NtcError::Success;
}

template <typename AdcType>
NtcError NtcThermistor<AdcType>::GetCalibrationOffset(
    float *offset_celsius) const noexcept {
  if (offset_celsius == nullptr) {
    return NtcError::NullPointer;
  }

  *offset_celsius = config_.calibration_offset;
  return NtcError::Success;
}

template <typename AdcType>
NtcError NtcThermistor<AdcType>::ResetCalibration() noexcept {
  return SetCalibrationOffset(ZERO_FLOAT_);
}

//--------------------------------------
//  CONFIGURATION METHODS
//--------------------------------------

template <typename AdcType>
NtcError NtcThermistor<AdcType>::SetConversionMethod(
    NtcConversionMethod method) noexcept {
  config_.conversion_method = method;
  return NtcError::Success;
}

template <typename AdcType>
NtcError
NtcThermistor<AdcType>::SetVoltageDivider(float series_resistance) noexcept {
  if (series_resistance <= ZERO_FLOAT_) {
    return NtcError::InvalidParameter;
  }

  config_.series_resistance = series_resistance;
  return NtcError::Success;
}

template <typename AdcType>
NtcError
NtcThermistor<AdcType>::SetReferenceVoltage(float reference_voltage) noexcept {
  if (reference_voltage <= ZERO_FLOAT_) {
    return NtcError::InvalidParameter;
  }

  config_.reference_voltage = reference_voltage;
  return NtcError::Success;
}

template <typename AdcType>
NtcError NtcThermistor<AdcType>::SetBetaValue(float beta_value) noexcept {
  if (!NTC::ValidateBetaValue(beta_value)) {
    return NtcError::InvalidParameter;
  }

  config_.beta_value = beta_value;
  return NtcError::Success;
}

template <typename AdcType>
NtcError NtcThermistor<AdcType>::SetAdcChannel(uint8_t adc_channel) noexcept {
  if (adc_interface_ != nullptr &&
      !adc_interface_->IsChannelAvailable(adc_channel)) {
    return NtcError::InvalidParameter;
  }

  config_.adc_channel = adc_channel;
  return NtcError::Success;
}

template <typename AdcType>
NtcError NtcThermistor<AdcType>::SetSamplingParameters(
    uint32_t sample_count, uint32_t sample_delay_ms) noexcept {
  if (sample_count == 0) {
    return NtcError::InvalidParameter;
  }

  config_.sample_count = sample_count;
  config_.sample_delay_ms = sample_delay_ms;
  return NtcError::Success;
}

template <typename AdcType>
NtcError NtcThermistor<AdcType>::SetFiltering(bool enable,
                                              float alpha) noexcept {
  constexpr float MIN_FILTER_ALPHA_ = 0.0F;
  constexpr float MAX_FILTER_ALPHA_ = 1.0F;
  if (alpha < MIN_FILTER_ALPHA_ || alpha > MAX_FILTER_ALPHA_) {
    return NtcError::InvalidParameter;
  }

  config_.enable_filtering = enable;
  config_.filter_alpha = alpha;

  // Reset filter when changing settings
  filter_initialized_ = false;
  filtered_temperature_ = ZERO_FLOAT_;

  return NtcError::Success;
}

//--------------------------------------
//  UTILITY FUNCTIONS
//--------------------------------------

template <typename AdcType>
float NtcThermistor<AdcType>::CelsiusToFahrenheit(float celsius) noexcept {
  return (celsius * CELSIUS_TO_FAHRENHEIT_MULTIPLIER_) + FAHRENHEIT_OFFSET_;
}

template <typename AdcType>
float NtcThermistor<AdcType>::FahrenheitToCelsius(float fahrenheit) noexcept {
  return (fahrenheit - FAHRENHEIT_OFFSET_) * FAHRENHEIT_TO_CELSIUS_MULTIPLIER_;
}

template <typename AdcType>
float NtcThermistor<AdcType>::CelsiusToKelvin(float celsius) noexcept {
  return celsius + KELVIN_OFFSET_;
}

template <typename AdcType>
float NtcThermistor<AdcType>::KelvinToCelsius(float kelvin) noexcept {
  return kelvin - KELVIN_OFFSET_;
}

template <typename AdcType>
const char *NtcThermistor<AdcType>::GetErrorString(NtcError error) noexcept {
  switch (error) {
  case NtcError::Success:
    return "Success";
  case NtcError::Failure:
    return "General failure";
  case NtcError::NotInitialized:
    return "Not initialized";
  case NtcError::AlreadyInitialized:
    return "Already initialized";
  case NtcError::InvalidParameter:
    return "Invalid parameter";
  case NtcError::NullPointer:
    return "Null pointer";
  case NtcError::OutOfMemory:
    return "Out of memory";
  case NtcError::AdcReadFailed:
    return "ADC read failed";
  case NtcError::InvalidResistance:
    return "Invalid resistance value";
  case NtcError::TemperatureOutOfRange:
    return "Temperature out of range";
  case NtcError::LookupTableError:
    return "Lookup table error";
  case NtcError::ConversionFailed:
    return "Temperature conversion failed";
  case NtcError::CalibrationFailed:
    return "Calibration failed";
  case NtcError::UnsupportedOperation:
    return "Operation not supported";
  case NtcError::Timeout:
    return "Operation timeout";
  case NtcError::HardwareFault:
    return "Hardware fault";
  default:
    return "Unknown error";
  }
}

template <typename AdcType>
const char *NtcThermistor<AdcType>::GetTypeString(NtcType type) noexcept {
  switch (type) {
  case NtcType::Unknown:
    return "Unknown";
  case NtcType::NtcG163Jft103Ft1S:
    return "NTCG163JFT103FT1S";
  case NtcType::NtcG164Jf103Ft1S:
    return "NTCG164JF103FT1S";
  case NtcType::NtcG163Jf103Ft1S:
    return "NTCG163JF103FT1S";
  case NtcType::Custom:
    return "Custom";
  default:
    return "Invalid";
  }
}

//--------------------------------------
//  PRIVATE HELPER METHODS
//--------------------------------------

template <typename AdcType>
NtcError NtcThermistor<AdcType>::validateConfiguration(
    const ntc_config_t &config) const noexcept {
  if (config.resistance_at_25c <= ZERO_FLOAT_) {
    return NtcError::InvalidParameter;
  }

  if (!NTC::ValidateBetaValue(config.beta_value)) {
    return NtcError::InvalidParameter;
  }

  if (config.reference_voltage <= ZERO_FLOAT_) {
    return NtcError::InvalidParameter;
  }

  if (config.series_resistance <= ZERO_FLOAT_) {
    return NtcError::InvalidParameter;
  }

  if (config.sample_count == 0) {
    return NtcError::InvalidParameter;
  }

  if (config.min_temperature >= config.max_temperature) {
    return NtcError::InvalidParameter;
  }

  constexpr float MIN_FILTER_ALPHA_ = 0.0F;
  constexpr float MAX_FILTER_ALPHA_ = 1.0F;
  if (config.enable_filtering && (config.filter_alpha < MIN_FILTER_ALPHA_ ||
                                  config.filter_alpha > MAX_FILTER_ALPHA_)) {
    return NtcError::InvalidParameter;
  }

  return NtcError::Success;
}

template <typename AdcType>
NtcError NtcThermistor<AdcType>::readAdcVoltage(float *voltage_volts) noexcept {
  if (voltage_volts == nullptr) {
    return NtcError::NullPointer;
  }

  if (adc_interface_ == nullptr) {
    return NtcError::NullPointer;
  }

  // Convert AdcError to NtcError helper
  auto convert_error = [](ntc::AdcError err) -> NtcError {
    switch (err) {
    case ntc::AdcError::Success:
      return NtcError::Success;
    case ntc::AdcError::NotInitialized:
      return NtcError::NotInitialized;
    case ntc::AdcError::InvalidChannel:
      return NtcError::InvalidParameter;
    case ntc::AdcError::ReadFailed:
      return NtcError::AdcReadFailed;
    case ntc::AdcError::Timeout:
      return NtcError::Timeout;
    case ntc::AdcError::HardwareError:
      return NtcError::HardwareFault;
    default:
      return NtcError::AdcReadFailed;
    }
  };

  // Perform sampling if configured
  if (config_.sample_count > 1) {
    float sum = ZERO_FLOAT_;
    uint32_t successful_samples = 0;

    for (uint32_t i = 0; i < config_.sample_count; ++i) {
      float sample_voltage = ZERO_FLOAT_;
      ntc::AdcError err =
          adc_interface_->ReadChannelV(config_.adc_channel, &sample_voltage);
      if (err == ntc::AdcError::Success) {
        sum += sample_voltage;
        successful_samples++;
      }

      if (i + 1 < config_.sample_count) {
        delayBetweenSamples();
      }
    }

    if (successful_samples == 0) {
      return NtcError::AdcReadFailed;
    }

    *voltage_volts = sum / static_cast<float>(successful_samples);
    return NtcError::Success;
  } // Single sample
  float sample_voltage = 0.0F;
  ntc::AdcError err =
      adc_interface_->ReadChannelV(config_.adc_channel, &sample_voltage);
  if (err != ntc::AdcError::Success) {
    return convert_error(err);
  }
  *voltage_volts = sample_voltage;
  return NtcError::Success;
}

template <typename AdcType>
NtcError
NtcThermistor<AdcType>::calculateResistance(float voltage_volts,
                                            float *resistance_ohms) noexcept {
  if (resistance_ohms == nullptr) {
    return NtcError::NullPointer;
  }

  if (!NTC::CalculateThermistorResistance(
          voltage_volts, config_.reference_voltage, config_.series_resistance,
          resistance_ohms)) {
    return NtcError::ConversionFailed;
  }

  return NtcError::Success;
}

template <typename AdcType>
NtcError NtcThermistor<AdcType>::convertResistanceToTemperature(
    float resistance_ohms, float *temperature_celsius) noexcept {
  if (temperature_celsius == nullptr) {
    return NtcError::NullPointer;
  }

  // Choose conversion method
  switch (config_.conversion_method) {
  case NtcConversionMethod::LookupTable: {
    const NTC::ntc_lookup_table_t *table =
        NTC::GetNtcLookupTable(static_cast<int>(config_.type));
    if (table != nullptr && FindTemperatureFromLookupTable(
                                table, resistance_ohms, temperature_celsius)) {
      return NtcError::Success;
    }
    // Fall back to mathematical conversion if lookup fails
    break;
  }

  case NtcConversionMethod::Mathematical:
  case NtcConversionMethod::Auto:
  default:
    break;
  }

  // Use mathematical conversion (beta parameter)
  if (!NTC::ConvertResistanceToTemperatureBeta(
          resistance_ohms, config_.resistance_at_25c, config_.beta_value,
          temperature_celsius)) {
    return NtcError::ConversionFailed;
  }

  return NtcError::Success;
}

template <typename AdcType>
void NtcThermistor<AdcType>::delayBetweenSamples() noexcept {
  if (config_.sample_delay_ms == 0U || adc_interface_ == nullptr) {
    return;
  }
  // Optional port hook, detected at compile time: an RTOS sleep or a timed
  // wait. Without it the samples are taken back to back.
  if constexpr (requires(AdcType &adc, uint32_t ms) { adc.DelayMs(ms); }) {
    adc_interface_->DelayMs(config_.sample_delay_ms);
  }
}

template <typename AdcType>
float NtcThermistor<AdcType>::applyFiltering(float new_temperature) noexcept {
  if (!config_.enable_filtering) {
    return new_temperature;
  }

  if (!filter_initialized_) {
    filtered_temperature_ = new_temperature;
    filter_initialized_ = true;
    return new_temperature;
  }

  // Apply exponential moving average
  filtered_temperature_ =
      (config_.filter_alpha * new_temperature) +
      ((ONE_FLOAT_ - config_.filter_alpha) * filtered_temperature_);

  return filtered_temperature_;
}

template <typename AdcType>
void NtcThermistor<AdcType>::initializeConfigForType(
    NtcType ntc_type, ntc_config_t *config) noexcept {
  if (config == nullptr) {
    return;
  }

  // Set default values
  *config = GetDefaultNtcConfig();
  config->type = ntc_type;

  // Set type-specific values
  switch (ntc_type) {
  case NtcType::NtcG163Jft103Ft1S:
  case NtcType::NtcG164Jf103Ft1S:
  case NtcType::NtcG163Jf103Ft1S: {
    config->resistance_at_25c = NTC::DefaultConfig::DEFAULT_RESISTANCE_AT_25C_;
    config->beta_value = NTC::DefaultConfig::DEFAULT_BETA_VALUE_;
    break;
  }

  case NtcType::Custom:
  case NtcType::Unknown:
  default:
    // Use default values from GetDefaultNtcConfig()
    break;
  }
}

#endif // NTC_THERMISTOR_IMPL