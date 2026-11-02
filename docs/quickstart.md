# Quickstart

Read a temperature from your MCU: wire a divider, write a small ADC port class, call two driver functions.

## 1. Wire it

The thermistor sits between the ADC node and ground; the series resistor between the supply and the node.

![Voltage divider: V_REF through the series resistor to the ADC node, the NTC thermistor from the node to
ground, and an optional filter capacitor from the node to ground](assets/ntc-divider.svg){ .hf-plate }

| From | To | Notes |
| --- | --- | --- |
| 3.3 V | R_SERIES | 10 kΩ, 1 % or better; the supply value goes in `reference_voltage` |
| R_SERIES | ADC input | The divider node |
| ADC input | NTC | NTCG163JF103FT1S or any 10 kΩ NTC |
| NTC | GND | Same ground as the ADC |

Keep the node voltage inside the ADC's input range. With 3.3 V and 10 kΩ / 10 kΩ it is 1.65 V at 25 °C and
moves toward 0 V as the sensor heats. The [Hardware](hardware.md) page covers resistor choice, filtering and
self-heating.

## 2. Add the driver

Add the repository as a submodule and link the `hf_ntc_thermistor` CMake target.
[Installation](installation.md) covers CMake and ESP-IDF.

```bash
git submodule add https://github.com/hardfoc/hf-ntc-thermistor-driver.git external/hf-ntc-thermistor-driver
```

## 3. Write the port

The port is the only platform code. It brings the ADC up and returns a channel's voltage and raw count. Replace
the two `platform_*` calls with your SDK's.

```cpp title="my_adc.hpp"
--8<-- "tests/docs/my_adc.hpp:port"
```

Return the calibrated voltage at the pin from `ReadChannelV()`: the driver computes the resistance from it.
[Porting](porting.md) explains each method.

## 4. Read the temperature

```cpp title="main.cpp"
--8<-- "tests/docs/quickstart.cpp:quickstart"
```

What each step does:

1. `adc_channel` selects the input your port reads.
2. `series_resistance` is the resistor between the supply and the node, in ohms.
3. `reference_voltage` is the divider's supply voltage. The driver solves
   R<sub>NTC</sub> = R<sub>SERIES</sub> · V / (V<sub>REF</sub> − V) with it.
4. `Initialize()` checks the configuration, brings the ADC up and checks the channel.
5. `ReadTemperatureCelsius()` reads the voltage, converts it and checks the result against
   `min_temperature` / `max_temperature` (−40 °C to +125 °C by default).

This example is built by the host CI, so it compiles against the current headers.

## If it does not work

| Symptom | First thing to check |
| --- | --- |
| `Initialize()` returns `false` | `EnsureInitialized()` and `IsChannelAvailable()` in your port; a zero or negative value in the config |
| `Temperature conversion failed` | The node reads 0 V (thermistor shorted) or the supply (thermistor open) |
| A few degrees off at every temperature | `reference_voltage` or `series_resistance` differ from the board |
| Reading moves the wrong way | Thermistor and series resistor swapped ([Hardware](hardware.md#divider)) |

## Next

- [API guide](api-guide.md): averaging, filtering, calibration and error handling.
- [Configuration](configuration.md): every field of `ntc_config_t`, and other thermistors.
- [Conversion methods](conversion.md): how accurate the Beta equation is for this part.
