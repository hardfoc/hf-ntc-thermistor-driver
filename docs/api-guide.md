# API guide

How to do the common jobs with the driver: configure, read, smooth, calibrate and handle errors.

Every snippet on this page is compiled by the host CI from
[`tests/docs/api_tasks.cpp`](../tests/docs/api_tasks.cpp), with `Ntc` standing for `NtcThermistor<MyAdc>` and
`MyAdc` the port from [Porting](porting.md). The [API reference](api-reference.md) lists every member.

## The shape of the API

One `NtcThermistor<AdcType>` object per thermistor, holding an `ntc_config_t` and a pointer to your ADC port.
Several objects can share one port, each on its own `adc_channel`.

| Group | Calls |
| --- | --- |
| Lifecycle | `Initialize()`, `Deinitialize()`, `IsInitialized()` |
| Read | `ReadTemperatureCelsius()`, `ReadTemperatureFahrenheit()`, `ReadTemperatureKelvin()`, `ReadTemperature()` |
| Raw values | `GetResistance()`, `GetVoltage()`, `GetRawAdcValue()` |
| Configure | `SetConfiguration()`, `GetConfiguration()`, `SetAdcChannel()`, `SetVoltageDivider()`, `SetReferenceVoltage()`, `SetBetaValue()`, `SetConversionMethod()` |
| Smooth | `SetSamplingParameters()`, `SetFiltering()` |
| Calibrate | `Calibrate()`, `SetCalibrationOffset()`, `GetCalibrationOffset()`, `ResetCalibration()` |
| Utilities | `CelsiusToFahrenheit()` and the other unit conversions, `GetErrorString()`, `GetTypeString()`, `GetDriverVersion()` |

`Initialize()` returns `bool`; every other call that can fail returns an `NtcError` and writes its result
through a pointer. Nothing throws or allocates.

## Configure

From a part type, then adjust the board-specific values:

```cpp
--8<-- "tests/docs/api_tasks.cpp:by-type"
```

Or from a full configuration, for any thermistor:

```cpp
--8<-- "tests/docs/api_tasks.cpp:custom"
```

`Initialize()` returns `false` when a value is invalid (R25, the supplies or the series resistor not positive;
B outside 1000 K to 5000 K; `sample_count` zero; `min_temperature` not below `max_temperature`;
`filter_alpha` outside 0 to 1 with filtering on), when the port's
`EnsureInitialized()` fails, or when `IsChannelAvailable(adc_channel)` is false. `SetConfiguration()` checks the
same values and returns `InvalidParameter`. [Configuration](configuration.md) lists every field.

## Read

```cpp
--8<-- "tests/docs/api_tasks.cpp:read"
```

`ReadTemperatureCelsius()` is one conversion. `ReadTemperature()` fills every unit plus the resistance, voltage
and raw count, but reads the ADC again for each of those: with averaging on, it costs four times the
samples. `timestamp_us` is always 0 and `accuracy_celsius` always 0.5; fill in your own if you need them.

## Smooth

```cpp
--8<-- "tests/docs/api_tasks.cpp:smoothing"
```

| Setting | Effect | Cost |
| --- | --- | --- |
| `sample_count` | Averages that many ADC reads; failed reads are skipped | `sample_count` reads per call |
| `sample_delay_ms` | A wait between samples through the port's `DelayMs()` ([Porting](porting.md#what-you-implement)); none if the port has no `DelayMs()` | The call blocks `(sample_count − 1) × sample_delay_ms` |
| `enable_filtering`, `filter_alpha` | EMA after calibration: new = α · reading + (1 − α) · previous | None; the first reading seeds it |

`sample_delay_ms` blocks the calling task for the whole wait; leave it 0 unless the samples must be spread
out, and give the port a `DelayMs()` that sleeps rather than spins. A smaller α smooths more and responds more
slowly; changing the filter or the configuration resets it.

## Calibrate

```cpp
--8<-- "tests/docs/api_tasks.cpp:calibrate"
```

`Calibrate(t)` reads the sensor, with the current offset applied, and adds `t − reading` to the offset, so the
sensor then reads `t` whatever offset was set before. `ResetCalibration()` first is optional; it starts from
zero, as above. The offset is a constant: it corrects R25 tolerance and
divider errors well, and the Beta equation's error only near the calibration temperature
([Conversion methods](conversion.md)). The offset lives in RAM; store it yourself.

## Handle errors

```cpp
--8<-- "tests/docs/api_tasks.cpp:errors"
```

| `NtcError` | Meaning | Where to look |
| --- | --- | --- |
| `NotInitialized` | `Initialize()` not called or failed | Your port's `EnsureInitialized()` |
| `NullPointer` | A null output pointer, or no ADC port | The call site |
| `AdcReadFailed`, `Timeout`, `HardwareFault` | Your port returned `ReadFailed`, `Timeout` or `HardwareError` | Your port |
| `InvalidParameter` | A setter or `SetConfiguration()` rejected a value; or `InvalidChannel` from the port | The value |
| `ConversionFailed` | The node at 0 V or at V_REF, or a resistance outside 0.1 Ω to 1 MΩ | Wiring: shorted or open sensor |
| `TemperatureOutOfRange` | Converted, but outside `min_temperature` to `max_temperature` | The range, or a real over-temperature |

With averaging on, a single failed sample is skipped; the call fails with `AdcReadFailed` only when every sample
fails. `GetErrorString()` returns a short text for each code.

## Next

[Configuration](configuration.md) lists every field; [Troubleshooting](troubleshooting.md) maps symptoms to
causes.
