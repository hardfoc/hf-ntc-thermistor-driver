/**
 * @file esp32_ntc_adc.hpp
 * @brief ESP-IDF ADC port for the NTC thermistor driver (adc_oneshot + adc_cali)
 *
 * Reads one ADC1 channel with the ESP-IDF oneshot driver and returns the
 * calibrated voltage at the pin (adc_cali curve fitting where the chip has it,
 * line fitting otherwise). The driver computes the thermistor resistance from
 * that voltage, so the calibration is what sets the accuracy.
 *
 * Wiring (see esp32_ntc_test_config.hpp): 3.3 V -> R_SERIES -> node -> NTC ->
 * GND, node to the ADC1 channel (ADC1 channel 0 = GPIO0 on the ESP32-C6).
 *
 * @copyright HardFOC
 */

#pragma once

#include "ntc_adc_interface.hpp"

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdint>

/**
 * @brief One ADC1 channel through adc_oneshot, with adc_cali calibration
 *
 * The driver's channel number is the ADC1 channel number. Only the channel
 * given to the constructor is configured and reported available.
 */
class Esp32NtcAdc : public ntc::AdcInterface<Esp32NtcAdc> {
public:
  /**
   * @param channel ADC1 channel (0 to 6 on the ESP32-C6)
   * @param atten Attenuation; 12 dB spans about 0 V to 3.3 V, the whole divider
   */
  explicit Esp32NtcAdc(uint8_t channel, adc_atten_t atten = ADC_ATTEN_DB_12) noexcept
      : channel_(channel), atten_(atten) {}

  Esp32NtcAdc(const Esp32NtcAdc&) = delete;
  Esp32NtcAdc& operator=(const Esp32NtcAdc&) = delete;
  Esp32NtcAdc(Esp32NtcAdc&&) = delete;
  Esp32NtcAdc& operator=(Esp32NtcAdc&&) = delete;

  ~Esp32NtcAdc() noexcept {
    if (cali_handle_ != nullptr) {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
      adc_cali_delete_scheme_curve_fitting(cali_handle_);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
      adc_cali_delete_scheme_line_fitting(cali_handle_);
#endif
      cali_handle_ = nullptr;
    }
    if (unit_handle_ != nullptr) {
      adc_oneshot_del_unit(unit_handle_);
      unit_handle_ = nullptr;
    }
  }

  [[nodiscard]] bool IsInitialized() const noexcept { return ready_; }

  /// Create the ADC1 oneshot unit, configure the channel and the calibration.
  bool EnsureInitialized() noexcept {
    if (ready_) {
      return true;
    }

    adc_oneshot_unit_init_cfg_t unit_cfg = {};
    unit_cfg.unit_id = ADC_UNIT_1;
    esp_err_t err = adc_oneshot_new_unit(&unit_cfg, &unit_handle_);
    if (err != ESP_OK || unit_handle_ == nullptr) {
      ESP_LOGE(kTag, "adc_oneshot_new_unit: %s", esp_err_to_name(err));
      unit_handle_ = nullptr;
      return false;
    }

    adc_oneshot_chan_cfg_t chan_cfg = {};
    chan_cfg.atten = atten_;
    chan_cfg.bitwidth = ADC_BITWIDTH_12;
    err = adc_oneshot_config_channel(unit_handle_, static_cast<adc_channel_t>(channel_), &chan_cfg);
    if (err != ESP_OK) {
      ESP_LOGE(kTag, "adc_oneshot_config_channel(%u): %s", static_cast<unsigned>(channel_),
               esp_err_to_name(err));
      adc_oneshot_del_unit(unit_handle_);
      unit_handle_ = nullptr;
      return false;
    }

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_cfg = {};
    cali_cfg.unit_id = ADC_UNIT_1;
    cali_cfg.chan = static_cast<adc_channel_t>(channel_);
    cali_cfg.atten = atten_;
    cali_cfg.bitwidth = ADC_BITWIDTH_12;
    err = adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali_handle_);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cali_cfg = {};
    cali_cfg.unit_id = ADC_UNIT_1;
    cali_cfg.atten = atten_;
    cali_cfg.bitwidth = ADC_BITWIDTH_12;
    err = adc_cali_create_scheme_line_fitting(&cali_cfg, &cali_handle_);
#else
    err = ESP_ERR_NOT_SUPPORTED;
#endif
    if (err != ESP_OK) {
      // Without calibration the voltage is only as good as the nominal full scale.
      cali_handle_ = nullptr;
      ESP_LOGW(kTag, "ADC calibration unavailable (%s): readings use the nominal scale",
               esp_err_to_name(err));
    }

    ESP_LOGI(kTag, "ADC1 channel %u ready, calibration %s", static_cast<unsigned>(channel_),
             cali_handle_ != nullptr ? "on" : "off");
    ready_ = true;
    return true;
  }

  [[nodiscard]] bool IsChannelAvailable(uint8_t channel) const noexcept { return channel == channel_; }

  /// One conversion, as the raw 12-bit count.
  ntc::AdcError ReadChannelCount(uint8_t channel, uint32_t* count) noexcept {
    int raw = 0;
    const ntc::AdcError err = readRaw(channel, &raw);
    if (err == ntc::AdcError::Success) {
      *count = static_cast<uint32_t>(raw);
    }
    return err;
  }

  /// One conversion, as the calibrated voltage at the pin.
  ntc::AdcError ReadChannelV(uint8_t channel, float* voltage_v) noexcept {
    int raw = 0;
    const ntc::AdcError err = readRaw(channel, &raw);
    if (err != ntc::AdcError::Success) {
      return err;
    }
    int mv = 0;
    if (cali_handle_ != nullptr) {
      if (adc_cali_raw_to_voltage(cali_handle_, raw, &mv) != ESP_OK) {
        return ntc::AdcError::ReadFailed;
      }
    } else {
      mv = static_cast<int>((static_cast<float>(raw) * kNominalFullScaleMv) / kMaxCount);
    }
    *voltage_v = static_cast<float>(mv) / 1000.0F;
    return ntc::AdcError::Success;
  }

  [[nodiscard]] float GetReferenceVoltage() const noexcept { return kNominalFullScaleMv / 1000.0F; }
  [[nodiscard]] uint8_t GetResolutionBits() const noexcept { return 12; }

  /// Optional driver hook: wait between averaged samples, rounded up to whole ticks.
  void DelayMs(uint32_t ms) noexcept {
    const TickType_t ticks = static_cast<TickType_t>((ms + portTICK_PERIOD_MS - 1U) / portTICK_PERIOD_MS);
    vTaskDelay(ticks > 0 ? ticks : 1);
  }

private:
  static constexpr const char* kTag = "Esp32NtcAdc";
  static constexpr float kNominalFullScaleMv = 3300.0F; ///< 12 dB, uncalibrated fallback only
  static constexpr float kMaxCount = 4095.0F;

  ntc::AdcError readRaw(uint8_t channel, int* raw) noexcept {
    if (!ready_ || unit_handle_ == nullptr) {
      return ntc::AdcError::NotInitialized;
    }
    if (!IsChannelAvailable(channel)) {
      return ntc::AdcError::InvalidChannel;
    }
    const esp_err_t err = adc_oneshot_read(unit_handle_, static_cast<adc_channel_t>(channel_), raw);
    if (err == ESP_ERR_TIMEOUT) {
      return ntc::AdcError::Timeout;
    }
    return err == ESP_OK ? ntc::AdcError::Success : ntc::AdcError::ReadFailed;
  }

  uint8_t channel_;
  adc_atten_t atten_;
  adc_oneshot_unit_handle_t unit_handle_ = nullptr;
  adc_cali_handle_t cali_handle_ = nullptr;
  bool ready_ = false;
};
