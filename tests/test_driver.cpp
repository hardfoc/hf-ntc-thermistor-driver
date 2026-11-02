/**
 * @file test_driver.cpp
 * @brief Driver behaviour over a model of the ADC and divider, checked against the TDK NTCG163JF103FT1S
 *        resistance-temperature table (nominal values, TDK NTCG163JF103FT1S spec sheet, Table 3).
 */
#include "mock_adc.hpp"
#include "test_harness.hpp"

#include "ntc_conversion.hpp"
#include "ntc_thermistor.hpp"

using Ntc = NtcThermistor<MockAdc>;

namespace {
// Nominal resistance of the NTCG163JF103FT1S at a few temperatures (TDK spec sheet, Table 3).
constexpr float kR_m40C = 188500.0F;
constexpr float kR_0C = 27280.0F;
constexpr float kR_25C = 10000.0F;
constexpr float kR_85C = 1451.0F;
constexpr float kR_125C = 534.0F;

float readAt(Ntc& ntc, MockAdc& adc, float ohms) {
  adc.thermistor_ohms = ohms;
  float t = -999.0F;
  CHECK(ntc.ReadTemperatureCelsius(&t) == NtcError::Success);
  return t;
}
} // namespace

TEST_CASE("Initialize brings the ADC up once and checks the channel") {
  MockAdc adc;
  Ntc ntc(NtcType::NtcG163Jf103Ft1S, &adc);
  CHECK(!ntc.IsInitialized());
  REQUIRE(ntc.Initialize());
  CHECK(ntc.IsInitialized());
  CHECK_EQ(adc.init_calls, 1);
  REQUIRE(ntc.Initialize());
  CHECK_EQ(adc.init_calls, 1);
}

TEST_CASE("Initialize fails on an ADC that does not come up or a missing channel") {
  MockAdc down;
  down.init_ok = false;
  Ntc a(NtcType::NtcG163Jf103Ft1S, &down);
  CHECK(!a.Initialize());

  MockAdc adc;
  ntc_config_t cfg = GetDefaultNtcConfig();
  cfg.adc_channel = 7; // the model has channels 0..3
  Ntc b(cfg, &adc);
  CHECK(!b.Initialize());

  Ntc c(NtcType::NtcG163Jf103Ft1S, nullptr);
  CHECK(!c.Initialize());
}

TEST_CASE("Beta model is exact at 25 C and 85 C, the B25/85 points") {
  MockAdc adc;
  Ntc ntc(NtcType::NtcG163Jf103Ft1S, &adc);
  REQUIRE(ntc.Initialize());
  CHECK_NEAR(readAt(ntc, adc, kR_25C), 25.0, 0.05);
  CHECK_NEAR(readAt(ntc, adc, kR_85C), 85.0, 0.1);
}

TEST_CASE("Beta model error away from 25-85 C matches the documented values") {
  MockAdc adc;
  Ntc ntc(NtcType::NtcG163Jf103Ft1S, &adc);
  ntc_config_t cfg{};
  REQUIRE(ntc.GetConfiguration(&cfg) == NtcError::Success);
  cfg.min_temperature = -50.0F;                               // the errors push readings past
  cfg.max_temperature = 150.0F;                               // the default -40 .. 125 C window
  REQUIRE(ntc.SetConfiguration(cfg) == NtcError::Success);
  REQUIRE(ntc.Initialize());
  CHECK_NEAR(readAt(ntc, adc, kR_0C), 0.0 + 1.11, 0.05);      // reads 1.1 C high at 0 C
  CHECK_NEAR(readAt(ntc, adc, kR_125C), 125.0 + 1.68, 0.05);  // 1.7 C high at 125 C
  CHECK_NEAR(readAt(ntc, adc, kR_m40C), -40.0 + 4.44, 0.05);  // 4.4 C high at -40 C
}

TEST_CASE("Calls before Initialize and null outputs are rejected") {
  MockAdc adc;
  Ntc ntc(NtcType::NtcG163Jf103Ft1S, &adc);
  float t = 0.0F;
  CHECK(ntc.ReadTemperatureCelsius(&t) == NtcError::NotInitialized);
  CHECK(ntc.GetResistance(&t) == NtcError::NotInitialized);
  REQUIRE(ntc.Initialize());
  CHECK(ntc.ReadTemperatureCelsius(nullptr) == NtcError::NullPointer);
}

TEST_CASE("ADC errors map to NtcError") {
  MockAdc adc;
  Ntc ntc(NtcType::NtcG163Jf103Ft1S, &adc);
  REQUIRE(ntc.Initialize());
  float t = 0.0F;
  adc.fail_reads = 1;
  CHECK(ntc.ReadTemperatureCelsius(&t) == NtcError::AdcReadFailed);
  adc.fail_reads = 1;
  adc.read_error = ntc::AdcError::Timeout;
  CHECK(ntc.ReadTemperatureCelsius(&t) == NtcError::Timeout);
  adc.fail_reads = 1;
  adc.read_error = ntc::AdcError::HardwareError;
  CHECK(ntc.ReadTemperatureCelsius(&t) == NtcError::HardwareFault);
}

TEST_CASE("Averaging reads the ADC sample_count times and skips failed samples") {
  MockAdc adc;
  Ntc ntc(NtcType::NtcG163Jf103Ft1S, &adc);
  REQUIRE(ntc.Initialize());
  REQUIRE(ntc.SetSamplingParameters(8, 0) == NtcError::Success);
  adc.voltage_reads = 0;
  adc.fail_reads = 3;
  CHECK_NEAR(readAt(ntc, adc, kR_25C), 25.0, 0.05);
  CHECK_EQ(adc.voltage_reads, 8);
  CHECK(ntc.SetSamplingParameters(0, 0) == NtcError::InvalidParameter);
}

TEST_CASE("sample_delay_ms waits between samples through the port's DelayMs()") {
  MockAdc adc;
  Ntc ntc(NtcType::NtcG163Jf103Ft1S, &adc);
  REQUIRE(ntc.Initialize());
  REQUIRE(ntc.SetSamplingParameters(4, 5) == NtcError::Success);
  CHECK_NEAR(readAt(ntc, adc, kR_25C), 25.0, 0.05);
  CHECK_EQ(adc.delay_calls, 3); // between samples, not after the last
  CHECK_EQ(adc.delayed_ms, 15U);
  uint32_t raw = 0;
  REQUIRE(ntc.GetRawAdcValue(&raw) == NtcError::Success);
  CHECK_EQ(adc.delay_calls, 6);
  REQUIRE(ntc.SetSamplingParameters(4, 0) == NtcError::Success);
  CHECK_NEAR(readAt(ntc, adc, kR_25C), 25.0, 0.05);
  CHECK_EQ(adc.delay_calls, 6); // no delay configured: no calls
}

TEST_CASE("A port without DelayMs() still averages, back to back") {
  MockAdcNoDelay adc;
  NtcThermistor<MockAdcNoDelay> ntc(NtcType::NtcG163Jf103Ft1S, &adc);
  REQUIRE(ntc.Initialize());
  REQUIRE(ntc.SetSamplingParameters(4, 5) == NtcError::Success);
  adc.inner.thermistor_ohms = kR_25C;
  float t = 0.0F;
  REQUIRE(ntc.ReadTemperatureCelsius(&t) == NtcError::Success);
  CHECK_NEAR(t, 25.0, 0.05);
  CHECK_EQ(adc.inner.voltage_reads, 4);
}

TEST_CASE("Open and shorted thermistors fail the conversion") {
  MockAdc adc;
  Ntc ntc(NtcType::NtcG163Jf103Ft1S, &adc);
  REQUIRE(ntc.Initialize());
  float t = 0.0F;
  adc.thermistor_ohms = 1.0e9F; // open: node at the supply
  CHECK(ntc.ReadTemperatureCelsius(&t) == NtcError::ConversionFailed);
  adc.thermistor_ohms = 0.0F;   // short: node at ground
  CHECK(ntc.ReadTemperatureCelsius(&t) == NtcError::ConversionFailed);
}

TEST_CASE("Readings outside min/max return TemperatureOutOfRange") {
  MockAdc adc;
  ntc_config_t cfg = GetDefaultNtcConfig();
  cfg.max_temperature = 50.0F;
  Ntc ntc(cfg, &adc);
  REQUIRE(ntc.Initialize());
  float t = 0.0F;
  adc.thermistor_ohms = kR_85C;
  CHECK(ntc.ReadTemperatureCelsius(&t) == NtcError::TemperatureOutOfRange);
}

TEST_CASE("Calibration offset is added to every reading") {
  MockAdc adc;
  Ntc ntc(NtcType::NtcG163Jf103Ft1S, &adc);
  REQUIRE(ntc.Initialize());
  REQUIRE(ntc.SetCalibrationOffset(1.5F) == NtcError::Success);
  CHECK_NEAR(readAt(ntc, adc, kR_25C), 26.5, 0.05);
  REQUIRE(ntc.ResetCalibration() == NtcError::Success);
  REQUIRE(ntc.Calibrate(24.0F) == NtcError::Success);
  CHECK_NEAR(readAt(ntc, adc, kR_25C), 24.0, 0.05);
}

TEST_CASE("Calibrate adds the correction to an existing offset") {
  MockAdc adc;
  Ntc ntc(NtcType::NtcG163Jf103Ft1S, &adc);
  REQUIRE(ntc.Initialize());
  REQUIRE(ntc.SetCalibrationOffset(1.5F) == NtcError::Success);
  adc.thermistor_ohms = kR_25C; // reads 26.5 C with the offset
  REQUIRE(ntc.Calibrate(24.0F) == NtcError::Success);
  CHECK_NEAR(readAt(ntc, adc, kR_25C), 24.0, 0.05);
  float offset = 0.0F;
  REQUIRE(ntc.GetCalibrationOffset(&offset) == NtcError::Success);
  CHECK_NEAR(offset, -1.0, 0.05); // 1.5 + (24.0 - 26.5)
  // Calibrating again at the same point changes nothing.
  REQUIRE(ntc.Calibrate(24.0F) == NtcError::Success);
  CHECK_NEAR(readAt(ntc, adc, kR_25C), 24.0, 0.05);
}

TEST_CASE("Filtering is an exponential moving average seeded by the first reading") {
  MockAdc adc;
  Ntc ntc(NtcType::NtcG163Jf103Ft1S, &adc);
  REQUIRE(ntc.Initialize());
  REQUIRE(ntc.SetFiltering(true, 0.5F) == NtcError::Success);
  CHECK_NEAR(readAt(ntc, adc, kR_25C), 25.0, 0.05);
  CHECK_NEAR(readAt(ntc, adc, kR_85C), 55.0, 0.1);
  CHECK(ntc.SetFiltering(true, 1.5F) == NtcError::InvalidParameter);
}

TEST_CASE("ReadTemperature fills every unit, the resistance and the raw count") {
  MockAdc adc;
  Ntc ntc(NtcType::NtcG163Jf103Ft1S, &adc);
  REQUIRE(ntc.Initialize());
  adc.thermistor_ohms = kR_25C;
  ntc_reading_t r{};
  REQUIRE(ntc.ReadTemperature(&r) == NtcError::Success);
  CHECK(r.is_valid);
  CHECK_NEAR(r.temperature_celsius, 25.0, 0.05);
  CHECK_NEAR(r.temperature_fahrenheit, 77.0, 0.1);
  CHECK_NEAR(r.temperature_kelvin, 298.15, 0.05);
  CHECK_NEAR(r.resistance_ohms, kR_25C, 5.0);
  CHECK_NEAR(r.voltage_volts, 1.65, 0.001);
  CHECK_EQ(r.adc_raw_value, 2048U);
}

TEST_CASE("LookupTable reads the datasheet temperatures where Beta is off") {
  MockAdc adc;
  ntc_config_t cfg = GetDefaultNtcConfig();
  cfg.conversion_method = NtcConversionMethod::LookupTable;
  cfg.min_temperature = -40.5F;
  cfg.max_temperature = 150.0F;
  Ntc ntc(cfg, &adc);
  REQUIRE(ntc.Initialize());
  CHECK_NEAR(readAt(ntc, adc, kR_25C), 25.0, 0.05);
  CHECK_NEAR(readAt(ntc, adc, kR_0C), 0.0, 0.05);     // Beta reads +1.1 C
  CHECK_NEAR(readAt(ntc, adc, kR_125C), 125.0, 0.05); // Beta reads +1.7 C
  CHECK_NEAR(readAt(ntc, adc, kR_m40C), -40.0, 0.05); // Beta reads +4.4 C
}

TEST_CASE("LookupTable falls back to the Beta model outside the table") {
  MockAdc adc;
  ntc_config_t cfg = GetDefaultNtcConfig();
  cfg.conversion_method = NtcConversionMethod::LookupTable;
  cfg.min_temperature = -60.0F;
  Ntc ntc(cfg, &adc);
  REQUIRE(ntc.Initialize());
  float beta_c = 0.0F;
  REQUIRE(NTC::ConvertResistanceToTemperatureBeta(250000.0F, 10000.0F, 3435.0F, &beta_c));
  CHECK_NEAR(readAt(ntc, adc, 250000.0F), beta_c, 0.05); // above the -40 C row
}

TEST_CASE("Free functions: Beta in both directions and B from two points") {
  float r = 0.0F;
  REQUIRE(NTC::ConvertTemperatureToResistanceBeta(85.0F, 10000.0F, 3435.0F, &r));
  CHECK_NEAR(r, kR_85C, 2.0);
  float beta = 0.0F;
  REQUIRE(NTC::CalculateBetaValue(25.0F, kR_25C, 85.0F, kR_85C, &beta));
  CHECK_NEAR(beta, 3435.0, 2.0);
  float rt = 0.0F;
  REQUIRE(NTC::CalculateThermistorResistance(1.65F, 3.3F, 10000.0F, &rt));
  CHECK_NEAR(rt, 10000.0, 0.5);
}
