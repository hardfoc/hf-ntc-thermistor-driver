# Configuration

Every field of `ntc_config_t`, its default and the setter that changes it at run time.

`GetDefaultNtcConfig()` returns the defaults below (the NTCG163JF103FT1S on channel 0 of a 3.3 V,
10 kΩ divider). The constructor that takes an `NtcType` starts from the same defaults and loads that type's R25
and B. Change any field before construction, or with `SetConfiguration()` afterwards.

## Fields

| Field | Default | Setter |
| --- | --- | --- |
| `type` | `NtcG163Jft103Ft1S` | constructor |
| `resistance_at_25c` (Ω) | 10000 | `SetConfiguration()` |
| `beta_value` (K) | 3435 | `SetBetaValue()` |
| `reference_voltage` (V) | 3.3 | `SetReferenceVoltage()` |
| `series_resistance` (Ω) | 10000 | `SetVoltageDivider()` |
| `calibration_offset` (°C) | 0 | `SetCalibrationOffset()`, `Calibrate()` |
| `conversion_method` | `Auto` | `SetConversionMethod()` |
| `adc_channel` | 0 | `SetAdcChannel()` |
| `adc_resolution_bits` | 12 | not used in the conversion |
| `sample_count` | 1 | `SetSamplingParameters()` |
| `sample_delay_ms` | 0 | `SetSamplingParameters()` |
| `min_temperature` (°C) | −40 | `SetConfiguration()` |
| `max_temperature` (°C) | 125 | `SetConfiguration()` |
| `enable_filtering` | `false` | `SetFiltering()` |
| `filter_alpha` | 0.1 | `SetFiltering()` |

## What each one means

`resistance_at_25c`, `beta_value`
:   The thermistor's R25 and B from its datasheet. B must be 1000 K to 5000 K. Use B25/85 for a range around
    25 °C to 85 °C, B25/50 for a lower one ([Conversion methods](conversion.md#beta-equation-what-the-driver-uses)).

`reference_voltage`
:   The supply of the divider, at R_SERIES, in volts. Measure it on the board ([Hardware](hardware.md#supply-and-reference)).

`series_resistance`
:   The fixed resistor between the supply and the ADC node, in ohms.

`conversion_method`
:   `Auto` and `Mathematical` use the Beta equation. `LookupTable` uses the type's datasheet table from −40 °C
    to +150 °C and falls back to Beta outside it ([Conversion methods](conversion.md#lookup-table)).

`adc_channel`
:   The channel number passed to your port. `SetAdcChannel()` asks the port's `IsChannelAvailable()` first.

`sample_count`, `sample_delay_ms`
:   Reads averaged per conversion, and the wait between them through the port's optional `DelayMs()`
    ([API guide](api-guide.md#smooth)).

`min_temperature`, `max_temperature`
:   A reading outside this window returns `TemperatureOutOfRange`. Set it to what is physically possible for
    your sensor, so a broken divider shows up as an error.

`enable_filtering`, `filter_alpha`
:   An exponential moving average, α from 0 to 1.

## A different thermistor

```cpp
--8<-- "tests/docs/api_tasks.cpp:custom"
```

`type` only selects the defaults and the lookup table; `Custom` has no table, so it always uses the Beta
equation. If your datasheet gives a resistance table but no B, compute B from two of its points with
`NTC::CalculateBetaValue()`.

## Next

[API guide](api-guide.md) shows the setters in use; [Hardware](hardware.md) helps choose the divider values.
