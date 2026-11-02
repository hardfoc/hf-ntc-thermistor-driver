/**
 * @file ntc_thermistor_comprehensive_test.cpp
 * @brief Comprehensive test suite for NTC Thermistor driver on ESP32-C6
 *
 * Runs the driver on the real ESP32-C6 ADC (Esp32NtcAdc: adc_oneshot +
 * adc_cali) reading a 10 kOhm / NTCG163JF103FT1S divider; wiring in
 * esp32_ntc_test_config.hpp.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "esp32_ntc_adc.hpp"
#include "esp32_ntc_test_config.hpp"
#include "ntc_conversion.hpp"
#include "ntc_thermistor.hpp"
#include "TestFramework.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cmath>
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif
#include "esp_log.h"
#ifdef __cplusplus
}
#endif

static const char *TAG = "NTC_Test";
static TestResults g_test_results;

//=============================================================================
// TEST CONFIGURATION
//=============================================================================
static constexpr bool ENABLE_BASIC_TESTS = true;

using Cfg = NTC_TestConfig::ADCConfig;
using Circuit = NTC_TestConfig::CircuitConfig;
using Sampling = NTC_TestConfig::SamplingConfig;

//=============================================================================
// SHARED TEST RESOURCES
//=============================================================================
// The real ESP32 ADC (adc_oneshot + adc_cali) on the divider node.
static std::unique_ptr<Esp32NtcAdc> g_adc;
static std::unique_ptr<NtcThermistor<Esp32NtcAdc>> g_ntc_driver;

//=============================================================================
// TEST HELPER FUNCTIONS
//=============================================================================

/**
 * @brief Create the ADC port and the driver for the TDK NTCG163JF103FT1S
 */
static bool init_test_resources() noexcept {
  g_adc = std::make_unique<Esp32NtcAdc>(Cfg::CHANNEL);

  // NTCG163JF103FT1S defaults: R25 = 10 kOhm, B25/85 = 3435 K (TDK spec sheet, Table 2).
  ntc_config_t config = GetDefaultNtcConfig();
  config.type = NtcType::NtcG163Jf103Ft1S;
  config.adc_channel = Cfg::CHANNEL;
  config.series_resistance = Circuit::SERIES_RESISTANCE;
  config.reference_voltage = Circuit::SUPPLY_VOLTAGE;
  config.resistance_at_25c = NTC_TestConfig::ThermistorSpecs::RESISTANCE_AT_25C;
  config.beta_value = NTC_TestConfig::ThermistorSpecs::BETA_VALUE;
  config.conversion_method = NtcConversionMethod::LookupTable;
  config.sample_count = Sampling::SAMPLE_COUNT;
  config.sample_delay_ms = Sampling::SAMPLE_DELAY_MS;
  config.min_temperature = static_cast<float>(NTC_TestConfig::Temperature::MIN_TEMPERATURE);
  config.max_temperature = static_cast<float>(NTC_TestConfig::Temperature::MAX_TEMPERATURE);
  config.enable_filtering = false;
  config.calibration_offset = Circuit::CALIBRATION_OFFSET;

  g_ntc_driver = std::make_unique<NtcThermistor<Esp32NtcAdc>>(config, g_adc.get());
  return true;
}

/**
 * @brief Release the driver, then the ADC unit
 */
static void cleanup_test_resources() noexcept {
  g_ntc_driver.reset();
  g_adc.reset();
}

//=============================================================================
// TEST CASES
//=============================================================================

/**
 * @brief Initialize() brings up ADC1 and the calibration and accepts the channel
 */
static bool test_basic_initialization() noexcept {
  if (!g_ntc_driver->Initialize()) {
    ESP_LOGE(TAG, "Initialize() failed: ADC unit, channel %u or configuration",
             static_cast<unsigned>(Cfg::CHANNEL));
    return false;
  }
  return g_adc->IsInitialized() && g_ntc_driver->IsInitialized();
}

/**
 * @brief Read the divider node and the temperature from the real ADC
 *
 * Fails on an ADC error, or when the node sits at a rail (open or shorted
 * thermistor): check the wiring in esp32_ntc_test_config.hpp.
 */
static bool test_read_temperature() noexcept {
  ntc_reading_t r{};
  const NtcError err = g_ntc_driver->ReadTemperature(&r);
  if (err != NtcError::Success || !r.is_valid) {
    ESP_LOGE(TAG, "ReadTemperature: %s", NtcThermistor<Esp32NtcAdc>::GetErrorString(err));
    return false;
  }
  ESP_LOGI(TAG, "node %.3f V (raw %lu), NTC %.0f ohm, %.2f C", r.voltage_volts,
           static_cast<unsigned long>(r.adc_raw_value), r.resistance_ohms, r.temperature_celsius);
  return true;
}

/**
 * @brief The lookup table and the Steinhart-Hart fit agree on the same resistance
 *
 * Both follow the datasheet table (Table 3) to within 0.2 C from -40 C to +125 C.
 */
static bool test_lookup_matches_steinhart_hart() noexcept {
  float ohms = 0.0F;
  if (g_ntc_driver->GetResistance(&ohms) != NtcError::Success) {
    return false;
  }
  float table_c = 0.0F;
  if (g_ntc_driver->ReadTemperatureCelsius(&table_c) != NtcError::Success) {
    return false;
  }
  float sh_c = 0.0F;
  if (!NTC::ConvertResistanceToTemperatureDefault(ohms, &sh_c)) {
    return false;
  }
  ESP_LOGI(TAG, "lookup %.2f C, Steinhart-Hart %.2f C", table_c, sh_c);
  return std::fabs(table_c - sh_c) < 0.5F; // two reads of a settled sensor
}

//=============================================================================
// MAIN TEST RUNNER
//=============================================================================

extern "C" void app_main() {
  ESP_LOGI(TAG, "╔═════════════════════════════════════════════════════════════"
                "═════════════════╗");
  ESP_LOGI(TAG, "║                  ESP32-C6 NTC THERMISTOR COMPREHENSIVE TEST "
                "SUITE             ║");
  ESP_LOGI(TAG, "║                      HardFOC NTC Thermistor Driver Tests    "
                "                  ║");
  ESP_LOGI(TAG, "╚═════════════════════════════════════════════════════════════"
                "═════════════════╝");
  ESP_LOGI(TAG, "Driver version: %s", GetNtcThermistorDriverVersion());

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Report test section configuration
  print_test_section_status(TAG, "NTC Thermistor");

  // Initialize test resources
  if (!init_test_resources()) {
    ESP_LOGE(TAG, "Failed to initialize test resources");
    return;
  }

  // Run all tests based on configuration
  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_BASIC_TESTS, "NTC THERMISTOR BASIC TESTS", 5,
      RUN_TEST_IN_TASK("basic_initialization", test_basic_initialization, 8192,
                       1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("read_temperature", test_read_temperature, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("lookup_matches_steinhart_hart",
                       test_lookup_matches_steinhart_hart, 8192, 1);
      flip_test_progress_indicator(););

  // Cleanup
  cleanup_test_resources();

  // Print results
  print_test_summary(g_test_results, "NTC Thermistor", TAG);

  // Blink GPIO14 to indicate completion
  output_section_indicator(5);

  cleanup_test_progress_indicator();

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
