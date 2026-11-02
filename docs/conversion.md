# Conversion methods

How the driver turns a voltage into a temperature, which equation each method uses and how accurate each is
for the NTCG163JF103FT1S.

## The pipeline

```mermaid
flowchart LR
    A["ADC: V_NODE"] --> B["Divider: R = R_SERIES · V / (V_REF − V)"]
    B --> C["Beta, or the datasheet table (LookupTable)"]
    C --> D["+ calibration_offset"]
    D --> E["EMA filter (optional)"]
    E --> F["min/max check"]
```

Every reading follows these steps in `ReadTemperatureCelsius()`: average `sample_count` voltages, solve the
divider, convert, add the calibration offset, filter, and check the result against `min_temperature` and
`max_temperature`.

## Beta equation (what the driver uses)

```text
1/T = 1/T25 + ln(R / R25) / B          T and T25 in kelvin, T25 = 298.15 K
```

`NTC::ConvertResistanceToTemperatureBeta()` implements it, and the driver calls it for
`NtcConversionMethod::Mathematical`, for `Auto` (the default) and whenever the lookup table does not cover a
reading. B is a two-point fit: the datasheet's B25/85 makes the equation exact at 25 °C and 85 °C and leaves
an error elsewhere. For the NTCG163JF103FT1S (R25 = 10 kΩ, B25/85 = 3435 K), against the spec sheet's nominal
resistance:

| Temperature (°C) | Beta reads (°C) | Error (°C) |
| ---: | ---: | ---: |
| −40 | −35.56 | +4.44 |
| −20 | −17.47 | +2.53 |
| 0 | 1.11 | +1.11 |
| 25 | 25.00 | 0.00 |
| 50 | 49.58 | −0.42 |
| 85 | 85.01 | +0.01 |
| 100 | 100.50 | +0.50 |
| 125 | 126.68 | +1.68 |
| 150 | 153.19 | +3.19 |

*Derived* from Table 3 of the [spec sheet](https://product.tdk.com/en/search/sensor/ntc/chip-ntc-thermistor/info?part_no=NTCG163JF103FT1S); the host tests check the 0 °C,
25 °C, 85 °C, 125 °C and −40 °C rows. Between 10 °C and 95 °C the error stays under 0.6 °C. Outside it, use the
lookup table or the Steinhart-Hart equation below, or a calibration offset near the temperature that matters to
you.

Using B25/50 instead (3382 K on TDK's product page for the NTCG163JF103FT1) moves the exact points to 25 °C and
50 °C and shifts the error to higher temperatures. Use the B your datasheet gives for the range you care about.

## Steinhart-Hart equation

```text
1/T = A + B · ln(R) + C · ln(R)³
```

Three coefficients fitted at three temperatures follow the curve far better than Beta.
`NTC::CalculateSteinhartHartCoefficients()` fits them through three (temperature, resistance) points, and
`NTC::ConvertResistanceToTemperatureSteinhartHart()` applies them. Fitted to the spec sheet at −40 °C, 25 °C
and 125 °C (A = 0.8767633 × 10⁻³, B = 2.527899 × 10⁻⁴, C = 1.906681 × 10⁻⁷), the equation stays within 0.2 °C of
the nominal table from −40 °C to +125 °C and within 0.41 °C at +150 °C (*derived*; the host tests check every
row):

```cpp
--8<-- "tests/docs/api_tasks.cpp:steinhart-hart"
```

`NTC::ConvertResistanceToTemperatureDefault()` and `NTC::ConvertTemperatureToResistanceDefault()` use that fit,
so they suit the NTCG163JF103FT1S only; pass your own coefficients for another part. The temperature to
resistance direction solves the cubic exactly, so it inverts the equation. The driver's read path does not
call Steinhart-Hart: read the resistance with `GetResistance()` and apply it, as above, or use `LookupTable`.

## Lookup table

`NtcConversionMethod::LookupTable` searches a per-type table and interpolates. The three TDK types share the
NTCG163JF103FT1S table in `src/ntc_lookup_table.cpp`: the nominal column of Table 3 of the
[spec sheet](https://product.tdk.com/en/search/sensor/ntc/chip-ntc-thermistor/info?part_no=NTCG163JF103FT1S), in
ohms, from −40 °C (188.5 kΩ) to +150 °C (313 Ω) in 5 °C steps. The resistance falls as the temperature rises,
and the search brackets the reading between two rows. Between rows the driver interpolates the temperature
linearly in ln(R), which follows the curve to well under 0.1 °C over one 5 °C step.

At every row the table returns the datasheet temperature exactly, so it removes the Beta equation's error at
the ends of the range (+4.4 °C at −40 °C). A reading outside the table (colder than −40 °C or hotter than
+150 °C) falls back to the Beta equation. `Custom` has no table and always uses Beta.

## Other conversions

| Function | Does |
| --- | --- |
| `NTC::ConvertTemperatureToResistanceBeta()` | Beta, the other way: R at a temperature |
| `NTC::CalculateBetaValue()` | B from two (temperature, resistance) points |
| `NTC::CalculateThermistorResistance()` | The divider equation |
| `NTC::CalculateThermistorVoltage()` | The divider, the other way: V<sub>NODE</sub> for a resistance |
| `NtcThermistor<>::CelsiusToFahrenheit()` and friends | Unit conversions |

```cpp
--8<-- "tests/docs/api_tasks.cpp:beta"
```

## Next

[API guide](api-guide.md) shows the calls by task; [Configuration](configuration.md) lists every setting.
