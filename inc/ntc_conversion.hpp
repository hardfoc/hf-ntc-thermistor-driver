/**
 * @file ntc_conversion.hpp
 * @brief Mathematical conversion functions for NTC thermistor temperature
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */
#pragma once
#include <cmath>
#include <cstdint>

//--------------------------------------
//  Conversion Constants
//--------------------------------------

namespace NTC::Constants {
constexpr float ABSOLUTE_ZERO_CELSIUS_ = -273.15F; ///< Absolute zero in Celsius
constexpr float KELVIN_OFFSET_ =
    273.15F; ///< Celsius to Kelvin conversion offset
constexpr float REFERENCE_TEMPERATURE_C_ =
    25.0F; ///< Reference temperature (25°C)
constexpr float STEINHART_HART_A_ =
    1.129241e-3F; ///< Steinhart-Hart coefficient A
constexpr float STEINHART_HART_B_ =
    2.341077e-4F; ///< Steinhart-Hart coefficient B
constexpr float STEINHART_HART_C_ =
    8.775468e-8F; ///< Steinhart-Hart coefficient C

// Validation limits
constexpr float MIN_RESISTANCE_OHMS_ =
    0.1F; ///< Minimum valid resistance (ohms)
constexpr float MAX_RESISTANCE_OHMS_ =
    1000000.0F; ///< Maximum valid resistance (ohms)
constexpr float MIN_TEMPERATURE_CELSIUS_ =
    -273.15F; ///< Minimum valid temperature (°C)
constexpr float MAX_TEMPERATURE_CELSIUS_ =
    1000.0F;                               ///< Maximum valid temperature (°C)
constexpr float MIN_BETA_VALUE_ = 1000.0F; ///< Minimum valid beta value (K)
constexpr float MAX_BETA_VALUE_ = 5000.0F; ///< Maximum valid beta value (K)

// Numerical precision constants
constexpr float EPSILON_FLOAT_ = 1e-6F;   ///< Float comparison epsilon
constexpr float EPSILON_DOUBLE_ = 1e-12F; ///< Double precision epsilon
constexpr float ONE_FLOAT_ = 1.0F;        ///< One as float
constexpr float ZERO_FLOAT_ = 0.0F;       ///< Zero as float

// Time conversion constants
constexpr uint32_t MILLISECONDS_PER_SECOND_ =
    1000U; ///< Milliseconds per second

// Temperature conversion constants
constexpr float CELSIUS_TO_FAHRENHEIT_MULTIPLIER_ =
    9.0F / 5.0F; ///< Celsius to Fahrenheit multiplier
constexpr float FAHRENHEIT_OFFSET_ = 32.0F; ///< Fahrenheit offset
constexpr float FAHRENHEIT_TO_CELSIUS_MULTIPLIER_ =
    5.0F / 9.0F; ///< Fahrenheit to Celsius multiplier
} // namespace NTC::Constants

//--------------------------------------
//  Conversion Methods
//--------------------------------------

namespace NTC {

/**
 * @brief Convert resistance to temperature using beta parameter
 *
 * Uses the beta parameter equation to calculate temperature from resistance.
 * This is a simplified model that assumes a linear relationship in the
 * temperature-resistance curve around the reference temperature.
 *
 * @note The beta parameter method is faster but less accurate than
 *       Steinhart-Hart, especially at temperature extremes.
 *
 * @param resistance_ohms Thermistor resistance (ohms)
 * @param resistance_at_25c Resistance at 25°C (ohms)
 * @param beta_value Beta value (K), typically 3000-4000K for NTC thermistors
 * @param temperature_celsius Pointer to store temperature (°C)
 * @return true if successful, false otherwise
 *
 * @warning Input resistance must be positive and non-zero. Beta value should
 *          be within valid range (1000-5000K).
 */
bool ConvertResistanceToTemperatureBeta(float resistance_ohms,
                                        float resistance_at_25c,
                                        float beta_value,
                                        float *temperature_celsius) noexcept;

/**
 * @brief Convert temperature to resistance using beta parameter
 * @param temperature_celsius Temperature (°C)
 * @param resistance_at_25c Resistance at 25°C (ohms)
 * @param beta_value Beta value (K)
 * @param resistance_ohms Pointer to store resistance (ohms)
 * @return true if successful, false otherwise
 */
bool ConvertTemperatureToResistanceBeta(float temperature_celsius,
                                        float resistance_at_25c,
                                        float beta_value,
                                        float *resistance_ohms) noexcept;

/**
 * @brief Convert resistance to temperature using Steinhart-Hart equation
 *
 * Uses the Steinhart-Hart equation for high-accuracy temperature calculation.
 * This method provides better accuracy across a wider temperature range
 * compared to the beta parameter method.
 *
 * The Steinhart-Hart equation: 1/T = A + B*ln(R) + C*ln(R)³
 *
 * @note This method is more computationally expensive but provides superior
 *       accuracy, especially at temperature extremes.
 *
 * @param resistance_ohms Thermistor resistance (ohms)
 * @param coeff_a Steinhart-Hart coefficient A (typically ~1.129e-3)
 * @param coeff_b Steinhart-Hart coefficient B (typically ~2.341e-4)
 * @param coeff_c Steinhart-Hart coefficient C (typically ~8.775e-8)
 * @param temperature_celsius Pointer to store temperature (°C)
 * @return true if successful, false otherwise
 *
 * @see ValidateSteinhartHartCoefficients() to validate coefficients
 */
bool ConvertResistanceToTemperatureSteinhartHart(
    float resistance_ohms, float coeff_a, float coeff_b, float coeff_c,
    float *temperature_celsius) noexcept;

/**
 * @brief Convert temperature to resistance using Steinhart-Hart equation
 * @param temperature_celsius Temperature (°C)
 * @param coeff_a Steinhart-Hart coefficient A
 * @param coeff_b Steinhart-Hart coefficient B
 * @param coeff_c Steinhart-Hart coefficient C
 * @param resistance_ohms Pointer to store resistance (ohms)
 * @return true if successful, false otherwise
 */
bool ConvertTemperatureToResistanceSteinhartHart(
    float temperature_celsius, float coeff_a, float coeff_b, float coeff_c,
    float *resistance_ohms) noexcept;

/**
 * @brief Convert resistance to temperature using default Steinhart-Hart
 * coefficients
 * @param resistance_ohms Thermistor resistance (ohms)
 * @param temperature_celsius Pointer to store temperature (°C)
 * @return true if successful, false otherwise
 */
bool ConvertResistanceToTemperatureDefault(float resistance_ohms,
                                           float *temperature_celsius) noexcept;

/**
 * @brief Convert temperature to resistance using default Steinhart-Hart
 * coefficients
 * @param temperature_celsius Temperature (°C)
 * @param resistance_ohms Pointer to store resistance (ohms)
 * @return true if successful, false otherwise
 */
bool ConvertTemperatureToResistanceDefault(float temperature_celsius,
                                           float *resistance_ohms) noexcept;

//--------------------------------------
//  Voltage Divider Calculations
//--------------------------------------

/**
 * @brief Calculate thermistor resistance from voltage divider
 * @param voltage_thermistor Voltage across thermistor (V)
 * @param reference_voltage Reference voltage (V)
 * @param series_resistance Series resistance (ohms)
 * @param resistance_ohms Pointer to store thermistor resistance (ohms)
 * @return true if successful, false otherwise
 */
bool CalculateThermistorResistance(float voltage_thermistor,
                                   float reference_voltage,
                                   float series_resistance,
                                   float *resistance_ohms) noexcept;

/**
 * @brief Calculate voltage across thermistor from resistance
 * @param resistance_ohms Thermistor resistance (ohms)
 * @param reference_voltage Reference voltage (V)
 * @param series_resistance Series resistance (ohms)
 * @param voltage_thermistor Pointer to store voltage across thermistor (V)
 * @return true if successful, false otherwise
 */
bool CalculateThermistorVoltage(float resistance_ohms, float reference_voltage,
                                float series_resistance,
                                float *voltage_thermistor) noexcept;

/**
 * @brief Calculate voltage divider ratio
 * @param resistance_ohms Thermistor resistance (ohms)
 * @param series_resistance Series resistance (ohms)
 * @param ratio Pointer to store voltage ratio
 * @return true if successful, false otherwise
 */
bool CalculateVoltageDividerRatio(float resistance_ohms,
                                  float series_resistance,
                                  float *ratio) noexcept;

//--------------------------------------
//  Validation Functions
//--------------------------------------

/**
 * @brief Validate resistance value
 * @param resistance_ohms Resistance value to validate
 * @param min_resistance Minimum valid resistance
 * @param max_resistance Maximum valid resistance
 * @return true if valid, false otherwise
 */
bool ValidateResistance(float resistance_ohms, float min_resistance,
                        float max_resistance) noexcept;

/**
 * @brief Validate temperature value
 * @param temperature_celsius Temperature value to validate
 * @param min_temperature Minimum valid temperature
 * @param max_temperature Maximum valid temperature
 * @return true if valid, false otherwise
 */
bool ValidateTemperature(float temperature_celsius, float min_temperature,
                         float max_temperature) noexcept;

/**
 * @brief Validate voltage value
 * @param voltage_volts Voltage value to validate
 * @param min_voltage Minimum valid voltage
 * @param max_voltage Maximum valid voltage
 * @return true if valid, false otherwise
 */
bool ValidateVoltage(float voltage_volts, float min_voltage,
                     float max_voltage) noexcept;

/**
 * @brief Validate beta value
 * @param beta_value Beta value to validate
 * @return true if valid, false otherwise
 */
bool ValidateBetaValue(float beta_value) noexcept;

/**
 * @brief Validate Steinhart-Hart coefficients
 * @param coeff_a Coefficient A
 * @param coeff_b Coefficient B
 * @param coeff_c Coefficient C
 * @return true if valid, false otherwise
 */
bool ValidateSteinhartHartCoefficients(float coeff_a, float coeff_b,
                                       float coeff_c) noexcept;

//--------------------------------------
//  Utility Functions
//--------------------------------------

/**
 * @brief Calculate beta value from two temperature-resistance pairs
 * @param temp1_celsius First temperature (°C)
 * @param resistance1_ohms First resistance (ohms)
 * @param temp2_celsius Second temperature (°C)
 * @param resistance2_ohms Second resistance (ohms)
 * @param beta_value Pointer to store calculated beta value
 * @return true if successful, false otherwise
 */
bool CalculateBetaValue(float temp1_celsius, float resistance1_ohms,
                        float temp2_celsius, float resistance2_ohms,
                        float *beta_value) noexcept;

/**
 * @brief Calculate Steinhart-Hart coefficients from three
 * temperature-resistance pairs
 * @param temp1_celsius First temperature (°C)
 * @param resistance1_ohms First resistance (ohms)
 * @param temp2_celsius Second temperature (°C)
 * @param resistance2_ohms Second resistance (ohms)
 * @param temp3_celsius Third temperature (°C)
 * @param resistance3_ohms Third resistance (ohms)
 * @param coeff_a Pointer to store coefficient A
 * @param coeff_b Pointer to store coefficient B
 * @param coeff_c Pointer to store coefficient C
 * @return true if successful, false otherwise
 */
bool CalculateSteinhartHartCoefficients(
    float temp1_celsius, float resistance1_ohms, float temp2_celsius,
    float resistance2_ohms, float temp3_celsius, float resistance3_ohms,
    float *coeff_a, float *coeff_b, float *coeff_c) noexcept;

/**
 * @brief Calculate temperature accuracy estimate
 * @param resistance_ohms Thermistor resistance (ohms)
 * @param resistance_tolerance Resistance tolerance (fraction)
 * @param beta_tolerance Beta tolerance (fraction)
 * @param accuracy_celsius Pointer to store accuracy estimate (°C)
 * @return true if successful, false otherwise
 */
bool CalculateTemperatureAccuracy(float resistance_ohms,
                                  float resistance_tolerance,
                                  float beta_tolerance,
                                  float *accuracy_celsius) noexcept;

/**
 * @brief Calculate optimal series resistance for voltage divider
 * @param thermistor_resistance_at_25c Thermistor resistance at 25°C (ohms)
 * @param min_temperature Minimum temperature (°C)
 * @param max_temperature Maximum temperature (°C)
 * @param optimal_resistance Pointer to store optimal series resistance (ohms)
 * @return true if successful, false otherwise
 */
bool CalculateOptimalSeriesResistance(float thermistor_resistance_at_25c,
                                      float min_temperature,
                                      float max_temperature,
                                      float *optimal_resistance) noexcept;

} // namespace NTC
