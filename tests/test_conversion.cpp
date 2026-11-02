/**
 * @file test_conversion.cpp
 * @brief Lookup table and Steinhart-Hart helpers checked against the TDK NTCG163JF103FT1S
 *        resistance-temperature table (TDK-EPC spec sheet NTCG163JF103FT1S, Table 3, Nom column).
 */
#include "test_harness.hpp"

#include "ntc_conversion.hpp"
#include "ntc_lookup_table.hpp"
#include "ntc_types.hpp"

#include <array>
#include <cmath>

namespace {
struct Row {
  float celsius;
  float ohms;
};

// TDK NTCG163JF103FT1S spec sheet, Table 3 "Resistance - Temperature Characteristics", Nom (kOhm x
// 1000).
constexpr std::array<Row, 39> kTable3 = {{
    {-40.0F, 188500.0F}, {-35.0F, 144300.0F}, {-30.0F, 111300.0F}, {-25.0F, 86600.0F},
    {-20.0F, 67800.0F},  {-15.0F, 53460.0F},  {-10.0F, 42450.0F},  {-5.0F, 33930.0F},
    {0.0F, 27280.0F},    {5.0F, 22070.0F},    {10.0F, 17960.0F},   {15.0F, 14700.0F},
    {20.0F, 12090.0F},   {25.0F, 10000.0F},   {30.0F, 8310.0F},    {35.0F, 6940.0F},
    {40.0F, 5826.0F},    {45.0F, 4911.0F},    {50.0F, 4158.0F},    {55.0F, 3536.0F},
    {60.0F, 3019.0F},    {65.0F, 2588.0F},    {70.0F, 2227.0F},    {75.0F, 1924.0F},
    {80.0F, 1668.0F},    {85.0F, 1451.0F},    {90.0F, 1267.0F},    {95.0F, 1110.0F},
    {100.0F, 975.0F},    {105.0F, 860.0F},    {110.0F, 760.0F},    {115.0F, 674.0F},
    {120.0F, 599.0F},    {125.0F, 534.0F},    {130.0F, 478.0F},    {135.0F, 428.0F},
    {140.0F, 385.0F},    {145.0F, 346.0F},    {150.0F, 313.0F},
}};

const NTC::ntc_lookup_table_t* tdkTable() {
  return NTC::GetNtcLookupTable(static_cast<int>(NtcType::NtcG163Jf103Ft1S));
}
} // namespace

TEST_CASE("Lookup table holds the datasheet Table 3 in ohms, -40 C to +150 C") {
  const NTC::ntc_lookup_table_t* t = tdkTable();
  REQUIRE(t != nullptr);
  REQUIRE(NTC::ValidateLookupTable(t));
  REQUIRE(t->entry_count == kTable3.size());
  for (size_t i = 0; i < kTable3.size(); ++i) {
    CHECK_NEAR(t->entries[i].temperature_celsius, kTable3[i].celsius, 1e-3);
    CHECK_NEAR(t->entries[i].resistance_ohms, kTable3[i].ohms, 0.5);
  }
  CHECK_NEAR(t->min_resistance, 313.0, 0.5);
  CHECK_NEAR(t->max_resistance, 188500.0, 0.5);
  CHECK_NEAR(t->min_temperature, -40.0, 1e-3);
  CHECK_NEAR(t->max_temperature, 150.0, 1e-3);
  // The other two types share the table.
  CHECK(NTC::GetNtcLookupTable(static_cast<int>(NtcType::NtcG163Jft103Ft1S)) != nullptr);
  CHECK(NTC::GetNtcLookupTable(static_cast<int>(NtcType::NtcG164Jf103Ft1S)) != nullptr);
  CHECK(NTC::GetNtcLookupTable(static_cast<int>(NtcType::Custom)) == nullptr);
}

TEST_CASE("Lookup table returns every datasheet row's temperature from its resistance") {
  const NTC::ntc_lookup_table_t* t = tdkTable();
  REQUIRE(t != nullptr);
  for (const Row& row : kTable3) {
    float c = -999.0F;
    CHECK(NTC::FindTemperatureFromLookupTable(t, row.ohms, &c));
    CHECK_NEAR(c, row.celsius, 0.01);
  }
}

TEST_CASE("Lookup table interpolates between rows on a falling (NTC) resistance") {
  const NTC::ntc_lookup_table_t* t = tdkTable();
  REQUIRE(t != nullptr);
  float c = 0.0F;
  // Geometric mean of the 20 C and 25 C rows: ln(R) half-way, so 22.5 C.
  REQUIRE(NTC::FindTemperatureFromLookupTable(t, std::sqrt(12090.0F * 10000.0F), &c));
  CHECK_NEAR(c, 22.5, 0.01);
  // A reading between rows lies between their temperatures.
  REQUIRE(NTC::FindTemperatureFromLookupTable(t, 1300.0F, &c));
  CHECK(c > 85.0F && c < 90.0F);
  REQUIRE(NTC::FindTemperatureFromLookupTable(t, 150000.0F, &c));
  CHECK(c > -40.0F && c < -35.0F);

  float r = 0.0F;
  REQUIRE(NTC::FindResistanceFromLookupTable(t, 85.0F, &r));
  CHECK_NEAR(r, 1451.0, 0.5);
  REQUIRE(NTC::FindResistanceFromLookupTable(t, 87.5F, &r));
  CHECK_NEAR(r, std::sqrt(1451.0 * 1267.0), 0.5);
}

TEST_CASE("Lookup table rejects resistances and temperatures outside it") {
  const NTC::ntc_lookup_table_t* t = tdkTable();
  REQUIRE(t != nullptr);
  float v = 0.0F;
  CHECK(!NTC::FindTemperatureFromLookupTable(t, 200000.0F, &v)); // colder than -40 C
  CHECK(!NTC::FindTemperatureFromLookupTable(t, 300.0F, &v));    // hotter than 150 C
  CHECK(!NTC::FindTemperatureFromLookupTable(t, std::nanf(""), &v));
  CHECK(!NTC::FindResistanceFromLookupTable(t, -41.0F, &v));
  CHECK(!NTC::FindResistanceFromLookupTable(t, 151.0F, &v));
  size_t lo = 0;
  size_t hi = 0;
  REQUIRE(NTC::BinarySearchLookupTable(t, 10000.0F, &lo, &hi));
  CHECK(t->entries[lo].resistance_ohms >= 10000.0F && t->entries[hi].resistance_ohms <= 10000.0F);
}
