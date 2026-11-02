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

namespace {
float rowOhms(float celsius) {
  for (const Row& row : kTable3) {
    if (row.celsius == celsius) {
      return row.ohms;
    }
  }
  return 0.0F;
}
} // namespace

TEST_CASE("Steinhart-Hart coefficients from three datasheet points reproduce them") {
  float a = 0.0F;
  float b = 0.0F;
  float c = 0.0F;
  REQUIRE(NTC::CalculateSteinhartHartCoefficients(-40.0F, rowOhms(-40.0F), 25.0F, rowOhms(25.0F), 125.0F,
                                                  rowOhms(125.0F), &a, &b, &c));
  // Reference fit of Table 3 at -40, 25 and 125 C (computed in double precision).
  CHECK_NEAR(a, 8.767633e-4, 1e-7);
  CHECK_NEAR(b, 2.527899e-4, 1e-8);
  CHECK_NEAR(c, 1.906681e-7, 5e-10);
  for (const float t : {-40.0F, 25.0F, 125.0F}) {
    float celsius = 0.0F;
    REQUIRE(NTC::ConvertResistanceToTemperatureSteinhartHart(rowOhms(t), a, b, c, &celsius));
    CHECK_NEAR(celsius, t, 0.02);
  }
}

TEST_CASE("Default Steinhart-Hart follows datasheet Table 3 within 0.2 C to 125 C") {
  for (const Row& row : kTable3) {
    float celsius = 0.0F;
    REQUIRE(NTC::ConvertResistanceToTemperatureDefault(row.ohms, &celsius));
    CHECK_NEAR(celsius, row.celsius, row.celsius <= 125.0F ? 0.2 : 0.45);
  }
}

TEST_CASE("Steinhart-Hart temperature to resistance inverts the equation, C term included") {
  for (const Row& row : kTable3) {
    float ohms = 0.0F;
    REQUIRE(NTC::ConvertTemperatureToResistanceDefault(row.celsius, &ohms));
    float back = 0.0F;
    REQUIRE(NTC::ConvertResistanceToTemperatureDefault(ohms, &back));
    CHECK_NEAR(back, row.celsius, 0.01);
  }
  float ohms = 0.0F;
  REQUIRE(NTC::ConvertTemperatureToResistanceDefault(85.0F, &ohms));
  CHECK_NEAR(ohms, 1451.0, 1451.0 * 0.01); // Table 3 nominal, within the 1 % tolerance band
}

TEST_CASE("Steinhart-Hart validation accepts this part's fit and rejects nonsense") {
  CHECK(NTC::ValidateSteinhartHartCoefficients(8.767633e-4F, 2.527899e-4F, 1.906681e-7F));
  CHECK(!NTC::ValidateSteinhartHartCoefficients(8.767633e-4F, -2.5e-4F, 1.9e-7F));
  CHECK(!NTC::ValidateSteinhartHartCoefficients(std::nanf(""), 2.5e-4F, 1.9e-7F));
}

TEST_CASE("Optimal series resistance uses the configured B, not 3435 K") {
  // A 100 kOhm, B = 4250 K part from 0 C to 100 C.
  ntc_config_t cfg = GetDefaultNtcConfig();
  cfg.resistance_at_25c = 100000.0F;
  cfg.beta_value = 4250.0F;
  float r0 = 0.0F;
  float r100 = 0.0F;
  REQUIRE(NTC::ConvertTemperatureToResistanceBeta(0.0F, cfg.resistance_at_25c, cfg.beta_value, &r0));
  REQUIRE(NTC::ConvertTemperatureToResistanceBeta(100.0F, cfg.resistance_at_25c, cfg.beta_value, &r100));
  float rs = 0.0F;
  REQUIRE(NTC::CalculateOptimalSeriesResistance(cfg.resistance_at_25c, cfg.beta_value, 0.0F, 100.0F, &rs));
  CHECK_NEAR(rs, std::sqrt(r0 * r100), 1.0);
  // This part (B25/85 = 3435 K) against the Table 3 rows at 0 C and 100 C: within the Beta model's own
  // error there (it reads 1.1 C high at 0 C, about 3 % in R).
  REQUIRE(NTC::CalculateOptimalSeriesResistance(10000.0F, 3435.0F, 0.0F, 100.0F, &rs));
  CHECK_NEAR(rs, std::sqrt(rowOhms(0.0F) * rowOhms(100.0F)), 0.04 * rs);
  CHECK(!NTC::CalculateOptimalSeriesResistance(10000.0F, 100.0F, 0.0F, 100.0F, &rs)); // B out of range
}

TEST_CASE("Temperature accuracy from R25 and B tolerances matches the datasheet Min/Max band") {
  // Spec sheet Table 2: R25 +/- 1 %, B25/85 +/- 1 %. Table 3 Min/Max at 25 C and 85 C, converted to
  // degrees with the local slope of the Nom column.
  float acc = 0.0F;
  REQUIRE(NTC::CalculateTemperatureAccuracy(10000.0F, 10000.0F, 3435.0F, 0.01F, 0.01F, &acc));
  // 25 C: 9.90 .. 10.10 kOhm = +/- 1 %, slope -3.9 %/C -> 0.26 C
  CHECK_NEAR(acc, 0.26, 0.05);
  REQUIRE(NTC::CalculateTemperatureAccuracy(1451.0F, 10000.0F, 3435.0F, 0.01F, 0.01F, &acc));
  // 85 C: 1.409 .. 1.494 kOhm = +/- 2.9 %, slope -2.7 %/C -> 1.1 C
  CHECK_NEAR(acc, 1.08, 0.1);
  REQUIRE(NTC::CalculateTemperatureAccuracy(188500.0F, 10000.0F, 3435.0F, 0.01F, 0.01F, &acc));
  // -40 C: 181.2 .. 196.1 kOhm = +/- 3.9 %, slope -5.3 %/C (table) to -6.3 %/C (Beta) -> 0.6 .. 0.7 C
  CHECK_NEAR(acc, 0.66, 0.1);
}
