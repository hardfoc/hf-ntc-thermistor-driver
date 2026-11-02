/**
 * @file my_adc.hpp
 * @brief The ADC port shown on the documentation's Porting page, compiled by the host build.
 *
 * Every platform call goes through the `platform_*` functions declared below; a real port
 * replaces them with its SDK (ESP-IDF adc_oneshot + adc_cali, STM32 HAL, Zephyr, Linux IIO). The class
 * is what the driver needs, nothing more.
 */
#pragma once

#include "ntc_adc_interface.hpp"

#include <cstdint>

// Platform hooks: your SDK's calls. Declared only, so the docs build checks the port compiles.
bool platform_adc_init() noexcept;                                         // unit, attenuation, calibration
bool platform_adc_read(uint8_t channel, uint32_t& raw, uint32_t& mv) noexcept;  // one conversion, calibrated
void platform_delay_ms(uint32_t ms) noexcept;                                   // RTOS sleep (vTaskDelay, k_msleep)

// --8<-- [start:port]
class MyAdc : public ntc::AdcInterface<MyAdc> {
public:
  static constexpr uint8_t kChannels = 7;       // ADC1 channels 0..6 on this MCU
  static constexpr uint8_t kBits = 12;
  static constexpr float kFullScaleV = 3.3F;    // input range at the chosen attenuation

  [[nodiscard]] bool IsInitialized() const { return ready_; }

  // Bring the ADC up once; later calls return true at once.
  bool EnsureInitialized() {
    if (!ready_) {
      ready_ = platform_adc_init();
    }
    return ready_;
  }

  [[nodiscard]] bool IsChannelAvailable(uint8_t channel) const { return channel < kChannels; }

  ntc::AdcError ReadChannelCount(uint8_t channel, uint32_t* count) {
    uint32_t mv = 0;
    return read(channel, count, &mv);
  }

  // The driver reads volts: return the calibrated voltage at the pin, not count * Vref / 4095.
  ntc::AdcError ReadChannelV(uint8_t channel, float* voltage_v) {
    uint32_t raw = 0;
    uint32_t mv = 0;
    const ntc::AdcError err = read(channel, &raw, &mv);
    if (err == ntc::AdcError::Success) {
      *voltage_v = static_cast<float>(mv) / 1000.0F;
    }
    return err;
  }

  [[nodiscard]] float GetReferenceVoltage() const { return kFullScaleV; }
  [[nodiscard]] uint8_t GetResolutionBits() const { return kBits; }

  // Optional: the driver waits sample_delay_ms between averaged samples through this.
  void DelayMs(uint32_t ms) { platform_delay_ms(ms); }

private:
  ntc::AdcError read(uint8_t channel, uint32_t* raw, uint32_t* mv) {
    if (!ready_) {
      return ntc::AdcError::NotInitialized;
    }
    if (!IsChannelAvailable(channel)) {
      return ntc::AdcError::InvalidChannel;
    }
    if (!platform_adc_read(channel, *raw, *mv)) {
      return ntc::AdcError::ReadFailed;
    }
    return ntc::AdcError::Success;
  }

  bool ready_ = false;
};
// --8<-- [end:port]
