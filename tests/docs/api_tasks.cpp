/**
 * @file api_tasks.cpp
 * @brief The API guide's and Configuration page's snippets, compiled by the host build (never run).
 */
#include "my_adc.hpp"

#include "ntc_conversion.hpp"
#include "ntc_thermistor.hpp"

#include <cmath>
#include <cstdio>

using Ntc = NtcThermistor<MyAdc>;

bool by_type(MyAdc& adc) {
  // --8<-- [start:by-type]
  Ntc ntc(NtcType::NtcG163Jf103Ft1S, &adc);      // R25 = 10 kOhm, B = 3435 K, defaults for the rest
  if (ntc.SetAdcChannel(2) != NtcError::Success ||
      ntc.SetVoltageDivider(10000.0F) != NtcError::Success ||   // series resistor (ohms)
      ntc.SetReferenceVoltage(3.3F) != NtcError::Success) {     // divider supply (V)
    return false;
  }
  return ntc.Initialize();
  // --8<-- [end:by-type]
}

bool custom_part(MyAdc& adc) {
  // --8<-- [start:custom]
  ntc_config_t cfg = GetDefaultNtcConfig();
  cfg.type = NtcType::Custom;
  cfg.resistance_at_25c = 100000.0F;   // a 100 kOhm part
  cfg.beta_value = 4250.0F;            // its B25/85 (or B25/50) from the datasheet
  cfg.series_resistance = 100000.0F;   // match R25 for the best resolution around 25 C
  cfg.reference_voltage = 3.3F;
  cfg.adc_channel = 0;
  cfg.min_temperature = -20.0F;        // readings outside this window return TemperatureOutOfRange
  cfg.max_temperature = 100.0F;

  Ntc ntc(cfg, &adc);
  return ntc.Initialize();             // false: invalid config, ADC not up, or no such channel
  // --8<-- [end:custom]
}

void read(Ntc& ntc) {
  // --8<-- [start:read]
  ntc_reading_t r{};
  if (ntc.ReadTemperature(&r) == NtcError::Success && r.is_valid) {
    std::printf("%.2f C  %.2f F  %.0f ohm  %.3f V  raw %lu\n", r.temperature_celsius,
                r.temperature_fahrenheit, r.resistance_ohms, r.voltage_volts,
                static_cast<unsigned long>(r.adc_raw_value));
  }

  float ohms = 0.0F;
  if (ntc.GetResistance(&ohms) == NtcError::Success) {
    std::printf("thermistor: %.0f ohm\n", ohms);
  }
  // --8<-- [end:read]
}

void smoothing(Ntc& ntc) {
  // --8<-- [start:smoothing]
  ntc.SetSamplingParameters(16, 0);     // average 16 ADC reads per reading
  ntc.SetFiltering(true, 0.2F);         // then an EMA: new = 0.2 * reading + 0.8 * previous
  // --8<-- [end:smoothing]
}

void calibrate(Ntc& ntc) {
  // --8<-- [start:calibrate]
  // With the sensor at a known temperature (a reference thermometer reads 23.4 C):
  ntc.ResetCalibration();
  if (ntc.Calibrate(23.4F) == NtcError::Success) {
    float offset = 0.0F;
    ntc.GetCalibrationOffset(&offset);  // store it; restore with SetCalibrationOffset(offset)
    std::printf("offset %.2f C\n", offset);
  }
  // --8<-- [end:calibrate]
}

void errors(Ntc& ntc) {
  // --8<-- [start:errors]
  float c = 0.0F;
  switch (const NtcError err = ntc.ReadTemperatureCelsius(&c); err) {
    case NtcError::Success:
      break;
    case NtcError::ConversionFailed:       // node at 0 V or at the supply: shorted or open sensor
    case NtcError::TemperatureOutOfRange:  // converted, but outside min/max_temperature
      std::printf("sensor fault: %s\n", Ntc::GetErrorString(err));
      break;
    default:                               // AdcReadFailed, Timeout, HardwareFault, NotInitialized ...
      std::printf("ADC: %s\n", Ntc::GetErrorString(err));
      break;
  }
  // --8<-- [end:errors]
}

bool steinhart_hart(Ntc& ntc, float& celsius) {
  // --8<-- [start:steinhart-hart]
  // Fit A, B and C through three rows of the datasheet table (-40, 25 and 125 C) ...
  float a = 0.0F;
  float b = 0.0F;
  float c = 0.0F;
  if (!NTC::CalculateSteinhartHartCoefficients(-40.0F, 188500.0F, 25.0F, 10000.0F, 125.0F, 534.0F,
                                               &a, &b, &c)) {
    return false;
  }
  // ... and apply them to the resistance the driver measures.
  float ohms = 0.0F;
  return ntc.GetResistance(&ohms) == NtcError::Success &&
         NTC::ConvertResistanceToTemperatureSteinhartHart(ohms, a, b, c, &celsius);
  // For the NTCG163JF103FT1S, NTC::ConvertResistanceToTemperatureDefault(ohms, &celsius) uses this fit.
  // --8<-- [end:steinhart-hart]
}

bool beta_from_table(float& beta) {
  // --8<-- [start:beta]
  // B25/85 from two datasheet points: 10.00 kOhm at 25 C, 1.451 kOhm at 85 C.
  return NTC::CalculateBetaValue(25.0F, 10000.0F, 85.0F, 1451.0F, &beta);  // beta ~= 3435 K
  // --8<-- [end:beta]
}
