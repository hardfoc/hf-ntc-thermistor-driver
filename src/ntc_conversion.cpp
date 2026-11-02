/**
 * @file ntc_conversion.cpp
 * @brief Mathematical conversion functions for NTC thermistor temperature
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */
#include "ntc_conversion.hpp"
#include "ntc_types.hpp"
#include <algorithm>
#include <cmath>

using NTC::Constants::EPSILON_DOUBLE_;
using NTC::Constants::EPSILON_FLOAT_;
using NTC::Constants::KELVIN_OFFSET_;
using NTC::Constants::MAX_RESISTANCE_OHMS_;
using NTC::Constants::MAX_TEMPERATURE_CELSIUS_;
using NTC::Constants::MIN_RESISTANCE_OHMS_;
using NTC::Constants::MIN_TEMPERATURE_CELSIUS_;
using NTC::Constants::ONE_FLOAT_;
using NTC::Constants::ZERO_FLOAT_;

//--------------------------------------
//  Conversion Methods
//--------------------------------------

namespace NTC {

bool ConvertResistanceToTemperatureBeta(float resistance_ohms,
                                        float resistance_at_25c,
                                        float beta_value,
                                        float *temperature_celsius) noexcept {
  if (temperature_celsius == nullptr) {
    return false;
  }

  if (!ValidateResistance(resistance_ohms, MIN_RESISTANCE_OHMS_,
                          MAX_RESISTANCE_OHMS_)) {
    return false;
  }

  if (!ValidateBetaValue(beta_value)) {
    return false;
  }

  if (resistance_at_25c <= ZERO_FLOAT_) {
    return false;
  }

  // Beta equation: 1/T = 1/T0 + (1/β) * ln(R/R0)
  // Where: T = temperature in Kelvin, T0 = reference temperature (298.15K =
  // 25°C) R = resistance, R0 = resistance at reference temperature, β = beta
  // value

  constexpr float kReferenceTemperatureKelvin =
      NTC::Constants::REFERENCE_TEMPERATURE_C_ + KELVIN_OFFSET_; // 298.15K
  const float temp_reference_kelvin = kReferenceTemperatureKelvin;
  const float ln_ratio = std::log(resistance_ohms / resistance_at_25c);
  const float inv_temperature =
      (ONE_FLOAT_ / temp_reference_kelvin) + (ln_ratio / beta_value);

  if (inv_temperature <= ZERO_FLOAT_) {
    return false; // Invalid result
  }

  const float temp_kelvin = ONE_FLOAT_ / inv_temperature;
  *temperature_celsius = temp_kelvin - KELVIN_OFFSET_;

  return true;
}

bool ConvertTemperatureToResistanceBeta(float temperature_celsius,
                                        float resistance_at_25c,
                                        float beta_value,
                                        float *resistance_ohms) noexcept {
  if (resistance_ohms == nullptr) {
    return false;
  }

  if (!ValidateTemperature(temperature_celsius, MIN_TEMPERATURE_CELSIUS_,
                           MAX_TEMPERATURE_CELSIUS_)) {
    return false;
  }

  if (!ValidateBetaValue(beta_value)) {
    return false;
  }

  if (resistance_at_25c <= ZERO_FLOAT_) {
    return false;
  }

  // Beta equation: R = R0 * exp(β * (1/T - 1/T0))
  // Where: T = temperature in Kelvin, T0 = reference temperature (298.15K =
  // 25°C) R = resistance, R0 = resistance at reference temperature, β = beta
  // value

  constexpr float kReferenceTemperatureKelvin =
      NTC::Constants::REFERENCE_TEMPERATURE_C_ + KELVIN_OFFSET_; // 298.15K
  const float temp_kelvin = temperature_celsius + KELVIN_OFFSET_;
  const float temp_reference_kelvin = kReferenceTemperatureKelvin;

  if (temp_kelvin <= ZERO_FLOAT_) {
    return false; // Invalid temperature
  }

  const float inv_diff =
      (ONE_FLOAT_ / temp_kelvin) - (ONE_FLOAT_ / temp_reference_kelvin);
  const float exp_term = std::exp(beta_value * inv_diff);
  *resistance_ohms = resistance_at_25c * exp_term;

  return true;
}

bool ConvertResistanceToTemperatureSteinhartHart(
    float resistance_ohms, float coeff_a, float coeff_b, float coeff_c,
    float *temperature_celsius) noexcept {
  if (temperature_celsius == nullptr) {
    return false;
  }

  if (!ValidateResistance(resistance_ohms, MIN_RESISTANCE_OHMS_,
                          MAX_RESISTANCE_OHMS_)) {
    return false;
  }

  if (!ValidateSteinhartHartCoefficients(coeff_a, coeff_b, coeff_c)) {
    return false;
  }

  // Steinhart-Hart equation: 1/T = A + B*ln(R) + C*ln(R)^3
  // Where: T = temperature in Kelvin, R = resistance

  const float ln_R = std::log(resistance_ohms);
  const float ln_R_cubed = ln_R * ln_R * ln_R;
  const float inv_temperature =
      coeff_a + (coeff_b * ln_R) + (coeff_c * ln_R_cubed);

  if (inv_temperature <= ZERO_FLOAT_) {
    return false; // Invalid result
  }

  const float temp_kelvin = ONE_FLOAT_ / inv_temperature;
  *temperature_celsius = temp_kelvin - KELVIN_OFFSET_;

  return true;
}

bool ConvertTemperatureToResistanceSteinhartHart(
    float temperature_celsius, float coeff_a, float coeff_b, float coeff_c,
    float *resistance_ohms) noexcept {
  if (resistance_ohms == nullptr) {
    return false;
  }

  if (!ValidateTemperature(temperature_celsius, MIN_TEMPERATURE_CELSIUS_,
                           MAX_TEMPERATURE_CELSIUS_)) {
    return false;
  }

  if (!ValidateSteinhartHartCoefficients(coeff_a, coeff_b, coeff_c)) {
    return false;
  }

  // Steinhart-Hart: 1/T = A + B*x + C*x^3 with x = ln(R). Solve the cubic
  // C*x^3 + B*x + (A - 1/T) = 0 for x. With B > 0 and C >= 0 it is
  // monotonic and has one real root (Cardano); C == 0 reduces to x = (1/T - A)/B.
  const double temp_kelvin =
      static_cast<double>(temperature_celsius) + KELVIN_OFFSET_;
  if (temp_kelvin <= 0.0) {
    return false; // Invalid temperature
  }

  const double a = coeff_a;
  const double b = coeff_b;
  const double c = coeff_c;
  const double y = (1.0 / temp_kelvin) - a;
  double ln_r = 0.0;
  if (std::abs(c) < 1e-15) {
    ln_r = y / b;
  } else {
    // x^3 + p*x + q = 0 with p = B/C, q = -y/C
    const double p = b / c;
    const double q = -y / c;
    const double disc = ((q * q) / 4.0) + ((p * p * p) / 27.0);
    if (disc < 0.0) {
      return false; // three real roots: not a monotonic NTC curve
    }
    const double s = std::sqrt(disc);
    ln_r = std::cbrt((-q / 2.0) + s) + std::cbrt((-q / 2.0) - s);
  }

  constexpr double kMaxLnResistance = 20.0;
  if (!(ln_r > -kMaxLnResistance && ln_r < kMaxLnResistance)) {
    return false; // Result out of reasonable range (or NaN)
  }

  *resistance_ohms = static_cast<float>(std::exp(ln_r));

  return true;
}

bool ConvertResistanceToTemperatureDefault(
    float resistance_ohms, float *temperature_celsius) noexcept {
  return ConvertResistanceToTemperatureSteinhartHart(
      resistance_ohms, NTC::Constants::STEINHART_HART_A_,
      NTC::Constants::STEINHART_HART_B_, NTC::Constants::STEINHART_HART_C_,
      temperature_celsius);
}

bool ConvertTemperatureToResistanceDefault(float temperature_celsius,
                                           float *resistance_ohms) noexcept {
  return ConvertTemperatureToResistanceSteinhartHart(
      temperature_celsius, NTC::Constants::STEINHART_HART_A_,
      NTC::Constants::STEINHART_HART_B_, NTC::Constants::STEINHART_HART_C_,
      resistance_ohms);
}

//--------------------------------------
//  Voltage Divider Calculations
//--------------------------------------

bool CalculateThermistorResistance(float voltage_thermistor,
                                   float reference_voltage,
                                   float series_resistance,
                                   float *resistance_ohms) noexcept {
  if (resistance_ohms == nullptr) {
    return false;
  }

  if (!ValidateVoltage(voltage_thermistor, ZERO_FLOAT_, reference_voltage)) {
    return false;
  }

  if (reference_voltage <= ZERO_FLOAT_ || series_resistance <= ZERO_FLOAT_) {
    return false;
  }

  // Voltage divider equation: V_thermistor = V_ref * (R_thermistor / (R_series
  // + R_thermistor)) Solving for R_thermistor: R_thermistor = R_series *
  // (V_thermistor / (V_ref - V_thermistor))

  const float voltage_diff = reference_voltage - voltage_thermistor;
  if (std::abs(voltage_diff) < EPSILON_FLOAT_) {
    return false; // Voltage too close to reference voltage
  }

  const float ratio = voltage_thermistor / voltage_diff;
  *resistance_ohms = series_resistance * ratio;

  return true;
}

bool CalculateThermistorVoltage(float resistance_ohms, float reference_voltage,
                                float series_resistance,
                                float *voltage_thermistor) noexcept {
  if (voltage_thermistor == nullptr) {
    return false;
  }

  if (!ValidateResistance(resistance_ohms, MIN_RESISTANCE_OHMS_,
                          MAX_RESISTANCE_OHMS_)) {
    return false;
  }

  if (reference_voltage <= ZERO_FLOAT_ || series_resistance <= ZERO_FLOAT_) {
    return false;
  }

  // Voltage divider equation: V_thermistor = V_ref * (R_thermistor / (R_series
  // + R_thermistor))

  const float total_resistance = series_resistance + resistance_ohms;
  if (total_resistance <= ZERO_FLOAT_) {
    return false;
  }

  *voltage_thermistor =
      reference_voltage * (resistance_ohms / total_resistance);

  return true;
}

bool CalculateVoltageDividerRatio(float resistance_ohms,
                                  float series_resistance,
                                  float *ratio) noexcept {
  if (ratio == nullptr) {
    return false;
  }

  if (!ValidateResistance(resistance_ohms, MIN_RESISTANCE_OHMS_,
                          MAX_RESISTANCE_OHMS_)) {
    return false;
  }

  if (series_resistance <= ZERO_FLOAT_) {
    return false;
  }

  const float total_resistance = series_resistance + resistance_ohms;
  if (total_resistance <= ZERO_FLOAT_) {
    return false;
  }

  *ratio = resistance_ohms / total_resistance;

  return true;
}

//--------------------------------------
//  Validation Functions
//--------------------------------------

bool ValidateResistance(float resistance_ohms, float min_resistance,
                        float max_resistance) noexcept {
  return (resistance_ohms >= min_resistance &&
          resistance_ohms <= max_resistance);
}

bool ValidateTemperature(float temperature_celsius, float min_temperature,
                         float max_temperature) noexcept {
  return (temperature_celsius >= min_temperature &&
          temperature_celsius <= max_temperature);
}

bool ValidateVoltage(float voltage_volts, float min_voltage,
                     float max_voltage) noexcept {
  return (voltage_volts >= min_voltage && voltage_volts <= max_voltage);
}

bool ValidateBetaValue(float beta_value) noexcept {
  // Beta values for NTC thermistors typically range from 2000K to 5000K
  return (beta_value >= NTC::Constants::MIN_BETA_VALUE_ &&
          beta_value <= NTC::Constants::MAX_BETA_VALUE_);
}

bool ValidateSteinhartHartCoefficients(float coeff_a, float coeff_b,
                                       float coeff_c) noexcept {
  // Ranges that cover NTC fits from about 1 kOhm to 1 MOhm. The C term of
  // the NTCG163JF103FT1S fit is 1.9e-7 (spec sheet Table 3), so C must be
  // allowed well above 1e-7.
  constexpr float MAX_ABS_COEFF_A_ = 1e-2F;
  constexpr float MIN_COEFF_B_ = 1e-4F;
  constexpr float MAX_COEFF_B_ = 1e-3F;
  constexpr float MAX_ABS_COEFF_C_ = 1e-6F;
  if (!std::isfinite(coeff_a) || !std::isfinite(coeff_b) ||
      !std::isfinite(coeff_c)) {
    return false;
  }
  return (std::abs(coeff_a) <= MAX_ABS_COEFF_A_ && coeff_b >= MIN_COEFF_B_ &&
          coeff_b <= MAX_COEFF_B_ && std::abs(coeff_c) <= MAX_ABS_COEFF_C_);
}

//--------------------------------------
//  Utility Functions
//--------------------------------------

bool CalculateBetaValue(float temp1_celsius, float resistance1_ohms,
                        float temp2_celsius, float resistance2_ohms,
                        float *beta_value) noexcept {
  if (beta_value == nullptr) {
    return false;
  }

  if (!ValidateTemperature(temp1_celsius, MIN_TEMPERATURE_CELSIUS_,
                           MAX_TEMPERATURE_CELSIUS_) ||
      !ValidateTemperature(temp2_celsius, MIN_TEMPERATURE_CELSIUS_,
                           MAX_TEMPERATURE_CELSIUS_)) {
    return false;
  }

  if (!ValidateResistance(resistance1_ohms, MIN_RESISTANCE_OHMS_,
                          MAX_RESISTANCE_OHMS_) ||
      !ValidateResistance(resistance2_ohms, MIN_RESISTANCE_OHMS_,
                          MAX_RESISTANCE_OHMS_)) {
    return false;
  }

  if (std::abs(temp1_celsius - temp2_celsius) < EPSILON_FLOAT_) {
    return false; // Temperatures too close
  }

  if (resistance1_ohms <= ZERO_FLOAT_ || resistance2_ohms <= ZERO_FLOAT_) {
    return false;
  }

  // Beta equation: β = ln(R1/R2) / (1/T1 - 1/T2)
  // Where: T1, T2 = temperatures in Kelvin, R1, R2 = resistances

  const float temp1_kelvin = temp1_celsius + KELVIN_OFFSET_;
  const float temp2_kelvin = temp2_celsius + KELVIN_OFFSET_;

  const float ln_ratio = std::log(resistance1_ohms / resistance2_ohms);
  const float inv_diff =
      (ONE_FLOAT_ / temp1_kelvin) - (ONE_FLOAT_ / temp2_kelvin);

  if (std::abs(inv_diff) < EPSILON_FLOAT_) {
    return false; // Invalid temperature difference
  }

  *beta_value = ln_ratio / inv_diff;

  return ValidateBetaValue(*beta_value);
}

bool CalculateSteinhartHartCoefficients(
    float temp1_celsius, float resistance1_ohms, float temp2_celsius,
    float resistance2_ohms, float temp3_celsius, float resistance3_ohms,
    float *coeff_a, float *coeff_b, float *coeff_c) noexcept {
  if (coeff_a == nullptr || coeff_b == nullptr || coeff_c == nullptr) {
    return false;
  }

  if (!ValidateTemperature(temp1_celsius, MIN_TEMPERATURE_CELSIUS_,
                           MAX_TEMPERATURE_CELSIUS_) ||
      !ValidateTemperature(temp2_celsius, MIN_TEMPERATURE_CELSIUS_,
                           MAX_TEMPERATURE_CELSIUS_) ||
      !ValidateTemperature(temp3_celsius, MIN_TEMPERATURE_CELSIUS_,
                           MAX_TEMPERATURE_CELSIUS_)) {
    return false;
  }

  if (!ValidateResistance(resistance1_ohms, MIN_RESISTANCE_OHMS_,
                          MAX_RESISTANCE_OHMS_) ||
      !ValidateResistance(resistance2_ohms, MIN_RESISTANCE_OHMS_,
                          MAX_RESISTANCE_OHMS_) ||
      !ValidateResistance(resistance3_ohms, MIN_RESISTANCE_OHMS_,
                          MAX_RESISTANCE_OHMS_)) {
    return false;
  }

  // Check that temperatures are different
  if (std::abs(temp1_celsius - temp2_celsius) < EPSILON_FLOAT_ ||
      std::abs(temp2_celsius - temp3_celsius) < EPSILON_FLOAT_ ||
      std::abs(temp1_celsius - temp3_celsius) < EPSILON_FLOAT_) {
    return false;
  }

  // Solve 1/T_i = A + B*L_i + C*L_i^3 (L_i = ln R_i) for the three points in
  // double precision: the system is badly conditioned in float.
  const double l1 = std::log(static_cast<double>(resistance1_ohms));
  const double l2 = std::log(static_cast<double>(resistance2_ohms));
  const double l3 = std::log(static_cast<double>(resistance3_ohms));
  const double y1 = 1.0 / (static_cast<double>(temp1_celsius) + KELVIN_OFFSET_);
  const double y2 = 1.0 / (static_cast<double>(temp2_celsius) + KELVIN_OFFSET_);
  const double y3 = 1.0 / (static_cast<double>(temp3_celsius) + KELVIN_OFFSET_);

  constexpr double kMinLnDiff = 1e-9;
  if (std::abs(l2 - l1) < kMinLnDiff || std::abs(l3 - l1) < kMinLnDiff ||
      std::abs(l3 - l2) < kMinLnDiff || std::abs(l1 + l2 + l3) < kMinLnDiff) {
    return false; // Singular: two resistances (nearly) equal
  }

  const double g2 = (y2 - y1) / (l2 - l1);
  const double g3 = (y3 - y1) / (l3 - l1);
  const double c = ((g3 - g2) / (l3 - l2)) / (l1 + l2 + l3);
  const double b = g2 - (c * ((l1 * l1) + (l1 * l2) + (l2 * l2)));
  const double a = y1 - ((b + (l1 * l1 * c)) * l1);

  *coeff_a = static_cast<float>(a);
  *coeff_b = static_cast<float>(b);
  *coeff_c = static_cast<float>(c);

  return ValidateSteinhartHartCoefficients(*coeff_a, *coeff_b, *coeff_c);
}

bool CalculateTemperatureAccuracy(float resistance_ohms,
                                  float resistance_at_25c, float beta_value,
                                  float resistance_tolerance,
                                  float beta_tolerance,
                                  float *accuracy_celsius) noexcept {
  if (accuracy_celsius == nullptr) {
    return false;
  }

  constexpr float kMinTolerance = 0.0F;
  constexpr float kMaxTolerance = 1.0F;
  if (!(resistance_tolerance >= kMinTolerance &&
        resistance_tolerance <= kMaxTolerance) ||
      !(beta_tolerance >= kMinTolerance && beta_tolerance <= kMaxTolerance)) {
    return false;
  }

  float temperature_celsius = 0.0F;
  if (!ConvertResistanceToTemperatureBeta(resistance_ohms, resistance_at_25c,
                                          beta_value, &temperature_celsius)) {
    return false;
  }

  // From 1/T = 1/T25 + ln(R/R25)/B:
  //   dT/T = -(T/B) dR/R              (R25 error moves R/R25)
  //   dT   = T (1 - T/T25) dB/B       (B error)
  // Summed as a worst case, like the datasheet's Min/Max columns.
  const float t_kelvin = temperature_celsius + KELVIN_OFFSET_;
  const float t25_kelvin =
      NTC::Constants::REFERENCE_TEMPERATURE_C_ + KELVIN_OFFSET_;
  const float error_r = (t_kelvin * t_kelvin / beta_value) *
                        resistance_tolerance;
  const float error_b =
      std::abs(t_kelvin * (ONE_FLOAT_ - (t_kelvin / t25_kelvin))) *
      beta_tolerance;
  *accuracy_celsius = error_r + error_b;

  return true;
}

bool CalculateTemperatureAccuracy(float resistance_ohms,
                                  float resistance_tolerance,
                                  float beta_tolerance,
                                  float *accuracy_celsius) noexcept {
  return CalculateTemperatureAccuracy(
      resistance_ohms, NTC::DefaultConfig::DEFAULT_RESISTANCE_AT_25C_,
      NTC::DefaultConfig::DEFAULT_BETA_VALUE_, resistance_tolerance,
      beta_tolerance, accuracy_celsius);
}

bool CalculateOptimalSeriesResistance(float thermistor_resistance_at_25c,
                                      float beta_value, float min_temperature,
                                      float max_temperature,
                                      float *optimal_resistance) noexcept {
  if (optimal_resistance == nullptr) {
    return false;
  }

  constexpr float kMinThermistorResistance = 100.0F;
  if (!ValidateResistance(thermistor_resistance_at_25c,
                          kMinThermistorResistance, MAX_RESISTANCE_OHMS_)) {
    return false;
  }

  if (!ValidateBetaValue(beta_value)) {
    return false;
  }

  if (!ValidateTemperature(min_temperature, MIN_TEMPERATURE_CELSIUS_,
                           MAX_TEMPERATURE_CELSIUS_) ||
      !ValidateTemperature(max_temperature, MIN_TEMPERATURE_CELSIUS_,
                           MAX_TEMPERATURE_CELSIUS_)) {
    return false;
  }

  if (min_temperature >= max_temperature) {
    return false;
  }

  // Calculate resistances at min and max temperatures
  float r_min = 0.0F;
  float r_max = 0.0F;
  if (!ConvertTemperatureToResistanceBeta(
          min_temperature, thermistor_resistance_at_25c, beta_value, &r_min) ||
      !ConvertTemperatureToResistanceBeta(
          max_temperature, thermistor_resistance_at_25c, beta_value, &r_max)) {
    return false;
  }

  // Optimal series resistance is the geometric mean of min and max resistances
  // This provides the most linear voltage response over the temperature range
  *optimal_resistance = std::sqrt(r_min * r_max);

  return true;
}

bool CalculateOptimalSeriesResistance(float thermistor_resistance_at_25c,
                                      float min_temperature,
                                      float max_temperature,
                                      float *optimal_resistance) noexcept {
  return CalculateOptimalSeriesResistance(
      thermistor_resistance_at_25c, NTC::DefaultConfig::DEFAULT_BETA_VALUE_,
      min_temperature, max_temperature, optimal_resistance);
}

} // namespace NTC