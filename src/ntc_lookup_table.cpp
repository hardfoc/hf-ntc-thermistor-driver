/**
 * @file ntc_lookup_table.cpp
 * @brief Lookup table implementation for NTC thermistor temperature conversion
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */
#include "ntc_lookup_table.hpp"
#include "ntc_types.hpp"
#include <algorithm>
#include <array>
#include <cmath>

using NTC::ntc_lookup_entry_t;
using NTC::ntc_lookup_table_t;

//--------------------------------------
//  NTCG163JF103FT1S table data
//--------------------------------------

// Nominal resistance-temperature characteristic of the TDK NTCG163JF103FT1S
// (R25 = 10 kOhm +/- 1 %, B25/85 = 3435 K +/- 1 %), in ohms, from the TDK-EPC
// spec sheet "NTCG163JF103FT1S", Table 3 "Resistance - Temperature
// Characteristics" (Nom column, kOhm x 1000). -40 C to +150 C in 5 C steps.
// Ordered by rising temperature, so the resistance falls along the table (NTC).
static constexpr std::array<ntc_lookup_entry_t, 39> NTCG163JF103FT1S_TABLE = {{
    {188500.0F, -40.0F},
    {144300.0F, -35.0F},
    {111300.0F, -30.0F},
    {86600.0F, -25.0F},
    {67800.0F, -20.0F},
    {53460.0F, -15.0F},
    {42450.0F, -10.0F},
    {33930.0F, -5.0F},
    {27280.0F, 0.0F},
    {22070.0F, 5.0F},
    {17960.0F, 10.0F},
    {14700.0F, 15.0F},
    {12090.0F, 20.0F},
    {10000.0F, 25.0F},
    {8310.0F, 30.0F},
    {6940.0F, 35.0F},
    {5826.0F, 40.0F},
    {4911.0F, 45.0F},
    {4158.0F, 50.0F},
    {3536.0F, 55.0F},
    {3019.0F, 60.0F},
    {2588.0F, 65.0F},
    {2227.0F, 70.0F},
    {1924.0F, 75.0F},
    {1668.0F, 80.0F},
    {1451.0F, 85.0F},
    {1267.0F, 90.0F},
    {1110.0F, 95.0F},
    {975.0F, 100.0F},
    {860.0F, 105.0F},
    {760.0F, 110.0F},
    {674.0F, 115.0F},
    {599.0F, 120.0F},
    {534.0F, 125.0F},
    {478.0F, 130.0F},
    {428.0F, 135.0F},
    {385.0F, 140.0F},
    {346.0F, 145.0F},
    {313.0F, 150.0F}}};

static constexpr float kTableMinResistance = 313.0F;    // at +150 C
static constexpr float kTableMaxResistance = 188500.0F; // at -40 C
static constexpr float kTableMinTemperature = -40.0F;
static constexpr float kTableMaxTemperature = 150.0F;

//--------------------------------------
//  NTCG163JFT103FT1S Lookup Table (same values as NTCG163JF103FT1S)
//--------------------------------------

static const ntc_lookup_table_t NTCG163JFT103FT1S_LOOKUP_TABLE = {
    .entries = NTCG163JF103FT1S_TABLE.data(),
    .entry_count = NTCG163JF103FT1S_TABLE.size(),
    .min_resistance = kTableMinResistance,
    .max_resistance = kTableMaxResistance,
    .min_temperature = kTableMinTemperature,
    .max_temperature = kTableMaxTemperature,
    .resistance_step = 0.0F}; // not uniform in resistance; 5 C in temperature

//--------------------------------------
//  NTCG164JF103FT1S Lookup Table (same values as NTCG163JF103FT1S)
//--------------------------------------

static const ntc_lookup_table_t NTCG164JF103FT1S_LOOKUP_TABLE = {
    .entries = NTCG163JF103FT1S_TABLE.data(), // Same table
    .entry_count = NTCG163JF103FT1S_TABLE.size(),
    .min_resistance = kTableMinResistance,
    .max_resistance = kTableMaxResistance,
    .min_temperature = kTableMinTemperature,
    .max_temperature = kTableMaxTemperature,
    .resistance_step = 0.0F};

//--------------------------------------
//  NTCG163JF103FT1S Lookup Table
//--------------------------------------

static const ntc_lookup_table_t NTCG163JF103FT1S_LOOKUP_TABLE = {
    .entries = NTCG163JF103FT1S_TABLE.data(), // Same table
    .entry_count = NTCG163JF103FT1S_TABLE.size(),
    .min_resistance = kTableMinResistance,
    .max_resistance = kTableMaxResistance,
    .min_temperature = kTableMinTemperature,
    .max_temperature = kTableMaxTemperature,
    .resistance_step = 0.0F};

//--------------------------------------
//  Lookup Table Functions
//--------------------------------------

namespace NTC {

const ntc_lookup_table_t *GetNtcLookupTable(int ntc_type) noexcept {
  switch (ntc_type) {
  case static_cast<int>(NtcType::NtcG163Jft103Ft1S):
    return &NTCG163JFT103FT1S_LOOKUP_TABLE;
  case static_cast<int>(NtcType::NtcG164Jf103Ft1S):
    return &NTCG164JF103FT1S_LOOKUP_TABLE;
  case static_cast<int>(NtcType::NtcG163Jf103Ft1S):
    return &NTCG163JF103FT1S_LOOKUP_TABLE;
  default:
    return nullptr;
  }
}

bool FindTemperatureFromLookupTable(const ntc_lookup_table_t *table,
                                    float resistance_ohms,
                                    float *temperature_celsius) noexcept {
  if (table == nullptr || temperature_celsius == nullptr) {
    return false;
  }

  if (!ValidateLookupTable(table)) {
    return false;
  }

  // Check if resistance is within table range
  if (!(resistance_ohms >= table->min_resistance &&
        resistance_ohms <= table->max_resistance)) {
    return false;
  }

  // Find the two entries that bracket the resistance
  size_t lower_index = 0;
  size_t upper_index = 0;
  if (!BinarySearchLookupTable(table, resistance_ohms, &lower_index,
                               &upper_index)) {
    return false;
  }

  // Interpolate between the two entries
  const ntc_lookup_entry_t &entry1 = table->entries[lower_index];
  const ntc_lookup_entry_t &entry2 = table->entries[upper_index];

  return InterpolateLookupEntries(entry1, entry2, resistance_ohms,
                                  temperature_celsius);
}

bool FindResistanceFromLookupTable(const ntc_lookup_table_t *table,
                                   float temperature_celsius,
                                   float *resistance_ohms) noexcept {
  if (table == nullptr || resistance_ohms == nullptr) {
    return false;
  }

  if (!ValidateLookupTable(table)) {
    return false;
  }

  // Check if temperature is within table range
  if (!(temperature_celsius >= table->min_temperature &&
        temperature_celsius <= table->max_temperature)) {
    return false;
  }

  // Entries are in rising temperature: find the pair that brackets it
  for (size_t i = 0; i + 1 < table->entry_count; i++) {
    const ntc_lookup_entry_t &entry1 = table->entries[i];
    const ntc_lookup_entry_t &entry2 = table->entries[i + 1];
    if (entry1.temperature_celsius <= temperature_celsius &&
        temperature_celsius <= entry2.temperature_celsius) {
      const float temp_diff =
          entry2.temperature_celsius - entry1.temperature_celsius;
      if (std::abs(temp_diff) < 1e-6F) {
        *resistance_ohms = entry1.resistance_ohms;
        return true;
      }
      // ln(R) is close to linear in temperature over one step (NTC
      // characteristic), so interpolate ln(R) rather than R.
      const float ratio =
          (temperature_celsius - entry1.temperature_celsius) / temp_diff;
      const float ln_r1 = std::log(entry1.resistance_ohms);
      const float ln_r2 = std::log(entry2.resistance_ohms);
      *resistance_ohms = std::exp(ln_r1 + (ratio * (ln_r2 - ln_r1)));
      return true;
    }
  }

  return false;
}

bool InterpolateLookupEntries(const ntc_lookup_entry_t &entry1,
                              const ntc_lookup_entry_t &entry2,
                              float resistance_ohms,
                              float *temperature_celsius) noexcept {
  if (temperature_celsius == nullptr) {
    return false;
  }

  if (resistance_ohms <= 0.0F || entry1.resistance_ohms <= 0.0F ||
      entry2.resistance_ohms <= 0.0F) {
    return false;
  }

  // Temperature is close to linear in ln(R) over one table step (NTC
  // characteristic), so interpolate against ln(R) rather than R.
  const float ln_r1 = std::log(entry1.resistance_ohms);
  const float ln_r2 = std::log(entry2.resistance_ohms);
  const float ln_diff = ln_r2 - ln_r1;
  if (std::abs(ln_diff) < 1e-9F) {
    *temperature_celsius = entry1.temperature_celsius;
    return true;
  }

  const float ratio = (std::log(resistance_ohms) - ln_r1) / ln_diff;
  *temperature_celsius =
      entry1.temperature_celsius +
      (ratio * (entry2.temperature_celsius - entry1.temperature_celsius));

  return true;
}

bool BinarySearchLookupTable(const ntc_lookup_table_t *table,
                             float resistance_ohms, size_t *lower_index,
                             size_t *upper_index) noexcept {
  if (table == nullptr || table->entries == nullptr ||
      lower_index == nullptr || upper_index == nullptr) {
    return false;
  }

  if (table->entry_count < 2) {
    return false;
  }

  // NTC: entries are in rising temperature, so resistance falls along the
  // table. Bracket the target: entries[lo] >= R >= entries[lo + 1].
  const ntc_lookup_entry_t *entries = table->entries;
  const size_t last = table->entry_count - 1;
  if (!(resistance_ohms <= entries[0].resistance_ohms &&
        resistance_ohms >= entries[last].resistance_ohms)) {
    return false; // outside the table (or NaN)
  }

  size_t lo = 0;
  size_t hi = last;
  while (hi - lo > 1) {
    const size_t mid = lo + ((hi - lo) / 2);
    if (entries[mid].resistance_ohms >= resistance_ohms) {
      lo = mid;
    } else {
      hi = mid;
    }
  }

  *lower_index = lo;
  *upper_index = hi;
  return true;
}

bool ValidateLookupTable(const ntc_lookup_table_t *table) noexcept {
  if (table == nullptr) {
    return false;
  }

  if (table->entries == nullptr) {
    return false;
  }

  if (table->entry_count < 2) {
    return false;
  }

  // NTC: temperature rises and resistance falls along the table
  for (size_t i = 1; i < table->entry_count; i++) {
    if (!(table->entries[i].resistance_ohms <
          table->entries[i - 1].resistance_ohms) ||
        !(table->entries[i].temperature_celsius >
          table->entries[i - 1].temperature_celsius)) {
      return false;
    }
  }
  if (!(table->entries[table->entry_count - 1].resistance_ohms > 0.0F)) {
    return false;
  }

  return true;
}

void GetLookupTableStats(const ntc_lookup_table_t *table, float *min_resistance,
                         float *max_resistance, float *min_temperature,
                         float *max_temperature, size_t *entry_count) noexcept {
  if (table == nullptr) {
    return;
  }

  if (min_resistance != nullptr) {
    *min_resistance = table->min_resistance;
  }
  if (max_resistance != nullptr) {
    *max_resistance = table->max_resistance;
  }
  if (min_temperature != nullptr) {
    *min_temperature = table->min_temperature;
  }
  if (max_temperature != nullptr) {
    *max_temperature = table->max_temperature;
  }
  if (entry_count != nullptr) {
    *entry_count = table->entry_count;
  }
}

//--------------------------------------
//  Specific NTC Type Functions
//--------------------------------------

const ntc_lookup_table_t *GetNtcG163Jft103Ft1sLookupTable() noexcept {
  return &NTCG163JFT103FT1S_LOOKUP_TABLE;
}

const ntc_lookup_table_t *GetNtcG164Jf103Ft1sLookupTable() noexcept {
  return &NTCG164JF103FT1S_LOOKUP_TABLE;
}

const ntc_lookup_table_t *GetNtcG163Jf103Ft1sLookupTable() noexcept {
  return &NTCG163JF103FT1S_LOOKUP_TABLE;
}

} // namespace NTC