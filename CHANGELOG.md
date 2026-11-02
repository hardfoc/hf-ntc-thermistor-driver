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

### Added

- Host unit tests (`HF_NTC_THERMISTOR_BUILD_TESTS`) that run the driver against a model of the ADC and divider
  and check readings against the NTCG163JF103FT1S resistance table.
