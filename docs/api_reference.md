# API Reference

Complete reference documentation for all public methods and types in the NTC Thermistor driver.

## Source Code

- **Main Header**: [`inc/ntc_thermistor.hpp`](../inc/ntc_thermistor.hpp)
- **Implementation**: [`src/ntc_thermistor.ipp`](../src/ntc_thermistor.ipp)
- **ADC Interface**: [`inc/ntc_adc_interface.hpp`](../inc/ntc_adc_interface.hpp)
- **Types**: [`inc/ntc_types.hpp`](../inc/ntc_types.hpp)

## Core Class

### `NtcThermistor<AdcType>`

Main driver class for temperature measurement using NTC thermistors.

**Template Parameter**: `AdcType` - Your ADC interface implementation (must inherit from `ntc::AdcInterface<AdcType>`)

**Location**: [`inc/ntc_thermistor.hpp#L74`](../inc/ntc_thermistor.hpp#L74)

**Constructors:**
```cpp
NtcThermistor(NtcType ntc_type, AdcType* adc_interface);
NtcThermistor(const ntc_config_t& config, AdcType* adc_interface);
```

**Location**: [`inc/ntc_thermistor.hpp#L96`](../inc/ntc_thermistor.hpp#L96), [`inc/ntc_thermistor.hpp#L104`](../inc/ntc_thermistor.hpp#L104)

## Methods

### Initialization

| Method | Signature | Location |
|--------|-----------|----------|
| `Initialize()` | `bool Initialize() noexcept` | [`src/ntc_thermistor.ipp#L57`](../src/ntc_thermistor.ipp#L57) |
| `Deinitialize()` | `bool Deinitialize() noexcept` | [`src/ntc_thermistor.ipp#L94`](../src/ntc_thermistor.ipp#L94) |
| `IsInitialized()` | `bool IsInitialized() const noexcept` | [`inc/ntc_thermistor.hpp#L151`](../inc/ntc_thermistor.hpp#L151) |
| `GetConfiguration()` | `NtcError GetConfiguration(ntc_config_t *config) const noexcept` | [`inc/ntc_thermistor.hpp#L158`](../inc/ntc_thermistor.hpp#L158) |
| `SetConfiguration()` | `NtcError SetConfiguration(const ntc_config_t &config) noexcept` | [`inc/ntc_thermistor.hpp#L165`](../inc/ntc_thermistor.hpp#L165) |

### Temperature Reading

| Method | Signature | Location |
|--------|-----------|----------|
| `ReadTemperatureCelsius()` | `NtcError ReadTemperatureCelsius(float *temperature_celsius) noexcept` | [`src/ntc_thermistor.ipp#L145`](../src/ntc_thermistor.ipp#L145) |
| `ReadTemperatureFahrenheit()` | `NtcError ReadTemperatureFahrenheit(float *temperature_fahrenheit) noexcept` | [`src/ntc_thermistor.ipp#L196`](../src/ntc_thermistor.ipp#L196) |
| `ReadTemperatureKelvin()` | `NtcError ReadTemperatureKelvin(float *temperature_kelvin) noexcept` | [`src/ntc_thermistor.ipp#L211`](../src/ntc_thermistor.ipp#L211) |
| `ReadTemperature()` | `NtcError ReadTemperature(ntc_reading_t *reading) noexcept` | [`src/ntc_thermistor.ipp#L227`](../src/ntc_thermistor.ipp#L227) |

### Resistance and Voltage

| Method | Signature | Location |
|--------|-----------|----------|
| `GetResistance()` | `NtcError GetResistance(float *resistance_ohms) noexcept` | [`src/ntc_thermistor.ipp#L273`](../src/ntc_thermistor.ipp#L273) |
| `GetVoltage()` | `NtcError GetVoltage(float *voltage_volts) noexcept` | [`src/ntc_thermistor.ipp#L292`](../src/ntc_thermistor.ipp#L292) |
| `GetRawAdcValue()` | `NtcError GetRawAdcValue(uint32_t *adc_value) noexcept` | [`src/ntc_thermistor.ipp#L305`](../src/ntc_thermistor.ipp#L305) |

### Calibration

| Method | Signature | Location |
|--------|-----------|----------|
| `Calibrate()` | `NtcError Calibrate(float reference_temperature_celsius) noexcept` | [`src/ntc_thermistor.ipp#L390`](../src/ntc_thermistor.ipp#L390) |
| `SetCalibrationOffset()` | `NtcError SetCalibrationOffset(float offset_celsius) noexcept` | [`src/ntc_thermistor.ipp#L414`](../src/ntc_thermistor.ipp#L414) |
| `GetCalibrationOffset()` | `NtcError GetCalibrationOffset(float *offset_celsius) const noexcept` | [`src/ntc_thermistor.ipp#L420`](../src/ntc_thermistor.ipp#L420) |
| `ResetCalibration()` | `NtcError ResetCalibration() noexcept` | [`src/ntc_thermistor.ipp#L431`](../src/ntc_thermistor.ipp#L431) |

### Configuration

| Method | Signature | Location |
|--------|-----------|----------|
| `SetConversionMethod()` | `NtcError SetConversionMethod(NtcConversionMethod method) noexcept` | [`src/ntc_thermistor.ipp#L440`](../src/ntc_thermistor.ipp#L440) |
| `SetVoltageDivider()` | `NtcError SetVoltageDivider(float series_resistance) noexcept` | [`src/ntc_thermistor.ipp#L448`](../src/ntc_thermistor.ipp#L448) |
| `SetReferenceVoltage()` | `NtcError SetReferenceVoltage(float reference_voltage) noexcept` | [`src/ntc_thermistor.ipp#L459`](../src/ntc_thermistor.ipp#L459) |
| `SetBetaValue()` | `NtcError SetBetaValue(float beta_value) noexcept` | [`src/ntc_thermistor.ipp#L469`](../src/ntc_thermistor.ipp#L469) |
| `SetAdcChannel()` | `NtcError SetAdcChannel(uint8_t adc_channel) noexcept` | [`src/ntc_thermistor.ipp#L479`](../src/ntc_thermistor.ipp#L479) |
| `SetSamplingParameters()` | `NtcError SetSamplingParameters(uint32_t sample_count, uint32_t sample_delay_ms) noexcept` | [`src/ntc_thermistor.ipp#L490`](../src/ntc_thermistor.ipp#L490) |
| `SetFiltering()` | `NtcError SetFiltering(bool enable, float alpha = 0.1F) noexcept` | [`src/ntc_thermistor.ipp#L502`](../src/ntc_thermistor.ipp#L502) |

### Utility Functions

| Method | Signature | Location |
|--------|-----------|----------|
| `CelsiusToFahrenheit()` | `static float CelsiusToFahrenheit(float celsius) noexcept` | [`inc/ntc_thermistor.hpp#L331`](../inc/ntc_thermistor.hpp#L331) |
| `FahrenheitToCelsius()` | `static float FahrenheitToCelsius(float fahrenheit) noexcept` | [`inc/ntc_thermistor.hpp#L338`](../inc/ntc_thermistor.hpp#L338) |
| `CelsiusToKelvin()` | `static float CelsiusToKelvin(float celsius) noexcept` | [`inc/ntc_thermistor.hpp#L345`](../inc/ntc_thermistor.hpp#L345) |
| `KelvinToCelsius()` | `static float KelvinToCelsius(float kelvin) noexcept` | [`inc/ntc_thermistor.hpp#L352`](../inc/ntc_thermistor.hpp#L352) |
| `GetErrorString()` | `static const char *GetErrorString(NtcError error) noexcept` | [`inc/ntc_thermistor.hpp#L359`](../inc/ntc_thermistor.hpp#L359) |
| `GetTypeString()` | `static const char *GetTypeString(NtcType type) noexcept` | [`inc/ntc_thermistor.hpp#L366`](../inc/ntc_thermistor.hpp#L366) |

## Types

### Enumerations

| Type | Values | Location |
|------|--------|----------|
| `NtcError` | `NTC_SUCCESS`, `NTC_ERROR_INIT`, `NTC_ERROR_ADC`, etc. | [`inc/ntc_types.hpp`](../inc/ntc_types.hpp) |
| `NtcType` | `NTC_TYPE_NTCG163JFT103FT1S`, etc. | [`inc/ntc_types.hpp`](../inc/ntc_types.hpp) |
| `NtcConversionMethod` | `NTC_METHOD_STEINHART_HART`, `NTC_METHOD_BETA`, `NTC_METHOD_LOOKUP_TABLE` | [`inc/ntc_types.hpp`](../inc/ntc_types.hpp) |

### Structures

| Type | Description | Location |
|------|-------------|----------|
| `ntc_config_t` | NTC configuration structure | [`inc/ntc_types.hpp`](../inc/ntc_types.hpp) |
| `ntc_reading_t` | Temperature reading structure | [`inc/ntc_types.hpp`](../inc/ntc_types.hpp) |

---

**Navigation**
⬅️ [Configuration](configuration.md) | [Next: Examples ➡️](examples.md) | [Back to Index](index.md)
