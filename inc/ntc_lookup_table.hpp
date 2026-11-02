/**
 * @file ntc_lookup_table.hpp
 * @brief Lookup table implementation for NTC thermistor temperature conversion
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */
#pragma once
#include <cstddef>
#include <cstdint>

namespace NTC {

//--------------------------------------
//  Lookup Table Entry
//--------------------------------------

/**
 * @brief Lookup table entry structure
 */
struct ntc_lookup_entry_t {
  float resistance_ohms;     ///< Resistance value (ohms)
  float temperature_celsius; ///< Corresponding temperature (°C)
};

//--------------------------------------
//  Lookup Table Structure
//--------------------------------------

/**
 * @brief Lookup table structure
 */
struct ntc_lookup_table_t {
  const ntc_lookup_entry_t *entries; ///< Array of lookup entries
  size_t entry_count;                ///< Number of entries
  float min_resistance;              ///< Minimum resistance in table
  float max_resistance;              ///< Maximum resistance in table
  float min_temperature;             ///< Minimum temperature in table
  float max_temperature;             ///< Maximum temperature in table
  float resistance_step;             ///< Resistance step between entries
};

//--------------------------------------
//  Lookup Table Functions
//--------------------------------------

/**
 * @brief Get lookup table for NTC type
 *
 * Retrieves the pre-calculated lookup table for a specific NTC thermistor type.
 * Lookup tables are optimized for fast temperature conversion with minimal
 * computational overhead.
 *
 * @param ntc_type NTC thermistor type (e.g., NtcType::NtcG163Jf103Ft1S)
 * @return Pointer to lookup table (nullptr if not available for this type)
 *
 * @note Not all NTC types have pre-calculated lookup tables. Use
 *       NtcConversionMethod::Mathematical for unsupported types.
 */
const ntc_lookup_table_t *GetNtcLookupTable(int ntc_type) noexcept;

/**
 * @brief Find temperature using lookup table
 * @param table Lookup table
 * @param resistance_ohms Resistance value
 * @param temperature_celsius Pointer to store temperature
 * @return true if found, false otherwise
 */
bool FindTemperatureFromLookupTable(const ntc_lookup_table_t *table,
                                    float resistance_ohms,
                                    float *temperature_celsius) noexcept;

/**
 * @brief Find resistance using lookup table
 * @param table Lookup table
 * @param temperature_celsius Temperature value
 * @param resistance_ohms Pointer to store resistance
 * @return true if found, false otherwise
 */
bool FindResistanceFromLookupTable(const ntc_lookup_table_t *table,
                                   float temperature_celsius,
                                   float *resistance_ohms) noexcept;

/**
 * @brief Interpolate between two lookup table entries
 * @param entry1 First entry
 * @param entry2 Second entry
 * @param resistance_ohms Target resistance
 * @param temperature_celsius Pointer to store interpolated temperature
 * @return true if interpolation successful, false otherwise
 */
bool InterpolateLookupEntries(const ntc_lookup_entry_t &entry1,
                              const ntc_lookup_entry_t &entry2,
                              float resistance_ohms,
                              float *temperature_celsius) noexcept;

/**
 * @brief Binary search in lookup table
 * @param table Lookup table
 * @param resistance_ohms Target resistance
 * @param lower_index Pointer to store lower index
 * @param upper_index Pointer to store upper index
 * @return true if found, false otherwise
 */
bool BinarySearchLookupTable(const ntc_lookup_table_t *table,
                             float resistance_ohms, size_t *lower_index,
                             size_t *upper_index) noexcept;

/**
 * @brief Validate lookup table
 * @param table Lookup table to validate
 * @return true if valid, false otherwise
 */
bool ValidateLookupTable(const ntc_lookup_table_t *table) noexcept;

/**
 * @brief Get lookup table statistics
 * @param table Lookup table
 * @param min_resistance Pointer to store minimum resistance
 * @param max_resistance Pointer to store maximum resistance
 * @param min_temperature Pointer to store minimum temperature
 * @param max_temperature Pointer to store maximum temperature
 * @param entry_count Pointer to store entry count
 */
void GetLookupTableStats(const ntc_lookup_table_t *table, float *min_resistance,
                         float *max_resistance, float *min_temperature,
                         float *max_temperature, size_t *entry_count) noexcept;

//--------------------------------------
//  NTCG163JFT103FT1S Lookup Table
//--------------------------------------

/**
 * @brief Get NTCG163JFT103FT1S lookup table
 * @return Pointer to lookup table
 */
const ntc_lookup_table_t *GetNtcG163Jft103Ft1sLookupTable() noexcept;

//--------------------------------------
//  NTCG164JF103FT1S Lookup Table
//--------------------------------------

/**
 * @brief Get NTCG164JF103FT1S lookup table
 * @return Pointer to lookup table
 */
const ntc_lookup_table_t *GetNtcG164Jf103Ft1sLookupTable() noexcept;

//--------------------------------------
//  NTCG163JF103FT1S Lookup Table
//--------------------------------------

/**
 * @brief Get NTCG163JF103FT1S lookup table
 * @return Pointer to lookup table
 */
const ntc_lookup_table_t *GetNtcG163Jf103Ft1sLookupTable() noexcept;

} // namespace NTC
