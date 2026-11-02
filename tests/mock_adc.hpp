/**
 * @file mock_adc.hpp
 * @brief A model of an ADC reading the node of an NTC voltage divider, for the host unit tests.
 *
 * The divider is the one the driver assumes: the series resistor from the supply to the ADC node, the
 * thermistor from the node to ground. Set the thermistor resistance; the ADC returns the node voltage (and
 * the matching count). Failures can be injected.
 */
#pragma once

#include "ntc_adc_interface.hpp"

#include <cmath>
#include <cstdint>

class MockAdc : public ntc::AdcInterface<MockAdc> {
public:
  // Model settings.
  float supply_v = 3.3F;            ///< Divider supply and ADC reference (V)
  float series_ohms = 10000.0F;     ///< Series resistor (ohms)
  float thermistor_ohms = 10000.0F; ///< Thermistor resistance now (ohms)
  uint8_t bits = 12;                ///< ADC resolution
  uint8_t channels = 4;             ///< Channels 0 .. channels-1 exist

  // Fault injection and counters.
  bool init_ok = true;
  int fail_reads = 0;                         ///< Next N reads fail with read_error
  ntc::AdcError read_error = ntc::AdcError::ReadFailed;
  int voltage_reads = 0;
  int count_reads = 0;
  int init_calls = 0;

  [[nodiscard]] bool IsInitialized() const { return initialized_; }

  bool EnsureInitialized() {
    ++init_calls;
    initialized_ = init_ok;
    return initialized_;
  }

  [[nodiscard]] bool IsChannelAvailable(uint8_t channel) const { return channel < channels; }

  ntc::AdcError ReadChannelV(uint8_t channel, float* voltage_v) {
    ++voltage_reads;
    if (const auto err = check(channel); err != ntc::AdcError::Success) {
      return err;
    }
    *voltage_v = nodeVoltage();
    return ntc::AdcError::Success;
  }

  ntc::AdcError ReadChannelCount(uint8_t channel, uint32_t* count) {
    ++count_reads;
    if (const auto err = check(channel); err != ntc::AdcError::Success) {
      return err;
    }
    const float full_scale = static_cast<float>((1U << bits) - 1U);
    *count = static_cast<uint32_t>(std::lround(nodeVoltage() / supply_v * full_scale));
    return ntc::AdcError::Success;
  }

  [[nodiscard]] float GetReferenceVoltage() const { return supply_v; }
  [[nodiscard]] uint8_t GetResolutionBits() const { return bits; }

  /// Voltage at the ADC node for the current thermistor resistance.
  [[nodiscard]] float nodeVoltage() const {
    return supply_v * thermistor_ohms / (series_ohms + thermistor_ohms);
  }

private:
  ntc::AdcError check(uint8_t channel) {
    if (!initialized_) {
      return ntc::AdcError::NotInitialized;
    }
    if (!IsChannelAvailable(channel)) {
      return ntc::AdcError::InvalidChannel;
    }
    if (fail_reads > 0) {
      --fail_reads;
      return read_error;
    }
    return ntc::AdcError::Success;
  }

  bool initialized_ = false;
};
