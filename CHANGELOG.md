# Changelog

All notable changes to this driver are listed here. Versions follow
[Semantic Versioning](https://semver.org/). Entries marked **BREAKING** need source changes in
code that used the affected API; together they call for a major version bump at release.

## Unreleased

### Breaking changes

- **BREAKING (behaviour):** `NTC::Constants::STEINHART_HART_A_`, `_B_` and `_C_`, used by
  `NTC::ConvertResistanceToTemperatureDefault()` and `NTC::ConvertTemperatureToResistanceDefault()`, are now
  the NTCG163JF103FT1S fit through spec sheet Table 3 at −40 °C, 25 °C and 125 °C (0.8767633 × 10⁻³,
  2.527899 × 10⁻⁴, 1.906681 × 10⁻⁷). The old generic values read 9.4 °C low at 85 °C for this part. Code that
  relied on the old values for another thermistor must pass its own coefficients.

### Documentation

- The documentation site moved to the HardFOC MkDocs platform: new page set (quickstart, hardware, conversion
  methods, architecture, API guide, configuration, porting, examples, troubleshooting), the Doxygen API
  reference under `/api/`, and every code sample compiled by the host build from `tests/docs/`.
- Added the TDK NTCG163JF103FT1S spec sheet and corrected the part facts: operating range −40 °C to +150 °C,
  B25/85 = 3435 K ± 1 %, 100 mW rated power. The documentation now states the Beta equation's error against the
  datasheet table and that the driver needs C++20.

### Fixed

- The NTCG163JF103FT1S lookup table now holds the spec sheet's Table 3 nominal resistances in ohms, −40 °C to
  +150 °C in 5 °C steps (it held values that were neither ohms nor this part's curve, including negative
  ones). The search and the validation follow the falling resistance of an NTC, and interpolation is linear in
  ln(R). `NtcConversionMethod::LookupTable` now reads the datasheet temperature at every row instead of falling
  back to the Beta equation.
- Steinhart-Hart helpers: `NTC::CalculateSteinhartHartCoefficients()` solved the 3 × 3 system with a wrong
  determinant (in float) and returned no usable fit; it now solves it in closed form in double precision.
  `NTC::ValidateSteinhartHartCoefficients()` rejected C above 1 × 10⁻⁷, which excluded this part's fit
  (C = 1.9 × 10⁻⁷); it now accepts |C| up to 1 × 10⁻⁶ and rejects non-finite values.
  `NTC::ConvertTemperatureToResistanceSteinhartHart()` dropped the C term; it now solves the cubic exactly.
- `Calibrate()` replaced the offset with `reference − reading`, but the reading already included the old
  offset, so calibrating with a non-zero offset left the sensor off by that offset. It now adds the correction
  to the existing offset. Calling `ResetCalibration()` first, as the docs advised, gives the same result as
  before.
- `sample_delay_ms` was a busy-wait loop that the compiler could remove, so it waited an unknown time or none.
  The driver now waits through an optional `DelayMs(uint32_t ms)` hook on the ADC port, detected at compile
  time; a port without it takes the samples back to back. Existing ports compile unchanged.
- `NTC::CalculateOptimalSeriesResistance()` and `NTC::CalculateTemperatureAccuracy()` used B = 3435 K whatever
  the part. New overloads take R25 and B (from `ntc_config_t`); the accuracy estimate is now the worst-case
  error from the R25 and B tolerances, which reproduces the spec sheet's Table 3 Min/Max band (0.26 °C at
  25 °C, 1.1 °C at 85 °C) instead of a fixed-divisor approximation.

### Deprecated

- `NTC::CalculateOptimalSeriesResistance(r25, t_min, t_max, &r)` and
  `NTC::CalculateTemperatureAccuracy(r, tol_r, tol_b, &acc)`: they assume the default part (R25 = 10 kΩ,
  B = 3435 K). Use the overloads that take B (and R25).

### Added

- CMake `install()` rules: the library, the headers with `src/ntc_thermistor.ipp`, the generated version header
  and an exported package, so `find_package(hf_ntc_thermistor)` and `hf::ntc_thermistor` work from an install
  prefix. `hf::ntc_thermistor` is also an alias in the build tree. The package configuration no longer includes
  the build-settings file, which was not installed.

- Host unit tests (`HF_NTC_THERMISTOR_BUILD_TESTS`) that run the driver against a model of the ADC and divider
  and check readings against the NTCG163JF103FT1S resistance table.
