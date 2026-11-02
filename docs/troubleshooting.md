# Troubleshooting

Symptoms you may see while bringing up an NTC thermistor, their causes and the fix for each.

Start with the voltage: `GetVoltage()` and `GetResistance()` on a failing reading tell wiring and ADC problems
(the voltage is wrong) apart from configuration problems (the resistance is right, the degrees are not).

## Initialize() returns false

| Cause | Check | Fix |
| --- | --- | --- |
| ADC not up | Your port's `EnsureInitialized()` | Fix the SDK call |
| Channel missing | `IsChannelAvailable(adc_channel)` in your port | Use a channel the port accepts |
| Invalid config | R25, supply or series resistor not positive; B outside 1000 K to 5000 K; `min_temperature` ≥ `max_temperature`; `sample_count` 0 | Correct the value; `SetConfiguration()` returns `InvalidParameter` for the same cases |
| No port | `nullptr` passed to the constructor | Pass the port's address |

## Temperature conversion failed

The node reads 0 V or the supply, or the resistance is outside 0.1 Ω to 1 MΩ.

| Cause | Check | Fix |
| --- | --- | --- |
| Thermistor shorted or missing to the node | `GetVoltage()` near 0 V | Wiring, solder joints |
| Thermistor open | `GetVoltage()` near the supply | Wiring, connector, cable |
| Port returns counts, not volts | `GetVoltage()` returns thousands | Return volts from `ReadChannelV()` ([Porting](porting.md)) |
| `reference_voltage` below the real supply | `GetVoltage()` above `reference_voltage` | Set the divider's real supply |

## Temperature out of range

The conversion worked but the result is outside `min_temperature` to `max_temperature` (−40 °C to +125 °C by
default). Read `GetResistance()`: if it matches the thermistor's table at the real temperature, widen the window;
if not, see the next section. A part near a hot spot may be right.

## Off by a few degrees

| Cause | Check | Fix |
| --- | --- | --- |
| Wrong supply value | Measure the divider supply | Set `reference_voltage` to it: 1 % is about 0.5 °C at 25 °C |
| Wrong series resistor | Measure it | Set `series_resistance` |
| Uncalibrated ADC | `GetVoltage()` against a meter | Use your SDK's ADC calibration in `ReadChannelV()` |
| Beta error far from 25 °C to 85 °C | The error grows with distance: +4.4 °C at −40 °C for this part | `LookupTable` for the TDK types, Steinhart-Hart or a calibration offset ([Conversion methods](conversion.md)) |
| Wrong B for the part | B25/50 and B25/85 differ by 1 % to 2 % | Use the B for your range |
| Self-heating | Reading drops when you lower the divider current | Larger resistors, or power the divider only while sampling ([Hardware](hardware.md#self-heating)) |

## Reading moves the wrong way

The temperature falls as the part heats: the thermistor is on the high side of the divider. Swap the two
parts, or return V<sub>REF</sub> − V<sub>NODE</sub> from the port ([Hardware](hardware.md#divider)).

## Noisy readings

| Cause | Fix |
| --- | --- |
| ADC noise | `SetSamplingParameters(16, 0)` |
| Slow drift or ripple | `SetFiltering(true, 0.1F)`; a smaller α smooths more |
| Long wire to the sensor, high source resistance | C_FILT, 10 nF to 100 nF from the node to ground |
| Supply ripple | Feed the divider from a clean rail ([Hardware](hardware.md#supply-and-reference)) |

## Calibration made it worse

`Calibrate()` measures with the current offset applied and replaces it. Call `ResetCalibration()` before
`Calibrate()`, and calibrate at the temperature you care about most.
