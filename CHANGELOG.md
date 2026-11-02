# Changelog

All notable changes to this driver are listed here. Versions follow
[Semantic Versioning](https://semver.org/). Entries marked **BREAKING** need source changes in
code that used the affected API; together they call for a major version bump at release.

## Unreleased

### Documentation

- The documentation site moved to the HardFOC MkDocs platform: new page set (quickstart, hardware, conversion
  methods, architecture, API guide, configuration, porting, examples, troubleshooting), the Doxygen API
  reference under `/api/`, and every code sample compiled by the host build from `tests/docs/`.
- Added the TDK NTCG163JF103FT1S spec sheet and corrected the part facts: operating range −40 °C to +150 °C,
  B25/85 = 3435 K ± 1 %, 100 mW rated power. The documentation now states the Beta equation's error against the
  datasheet table, that the lookup table is not usable, and that the driver needs C++20.

### Added

- Host unit tests (`HF_NTC_THERMISTOR_BUILD_TESTS`) that run the driver against a model of the ADC and divider
  and check readings against the NTCG163JF103FT1S resistance table.
