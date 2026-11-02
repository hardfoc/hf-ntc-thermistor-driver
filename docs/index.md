---
hide:
  - navigation
  - toc
---

<div class="hf-hero" markdown>
<span class="hf-kicker">HardFOC driver</span>

# NTC Thermistor Driver

<p>A C++20 driver for NTC thermistors read through an ADC. It turns the voltage of a resistor divider into the
thermistor's resistance and the resistance into degrees, with averaging, filtering and calibration, on any MCU
with an ADC.</p>

[Quickstart](quickstart.md){ .md-button .md-button--primary }
[API guide](api-guide.md){ .md-button }
[Source on GitHub](https://github.com/hardfoc/hf-ntc-thermistor-driver){ .md-button }
</div>

The driver has no platform code. You give it a small class that brings an ADC up and returns a channel's
voltage; it does the rest: the divider equation, the Beta conversion, range checks, averaging, filtering and
error codes.

<div class="hf-chip" markdown>
<div class="hf-chip__head">
<span class="hf-chip__kicker">NTC chip thermistor, the driver's default part</span>
<span class="hf-chip__name">NTCG163JF103FT1S</span>
<span class="hf-chip__vendor">TDK</span>
</div>

Resistance
:   10 kΩ ± 1 % at 25 °C (R25)

B constant
:   3435 K ± 1 % (B25/85)

Operating range
:   −40 °C to +150 °C

Rated power
:   100 mW at 25 °C, still air

Package
:   0603 (1.6 mm × 0.8 mm × 0.8 mm)

Interface
:   Analog: a resistor divider into any ADC input; the driver reads the node voltage

[Spec sheet](datasheet/ntcg163jf103ft1s.pdf)
</div>

## What you get

- **One object per sensor.** `NtcThermistor<YourAdc>` takes a part type or a full `ntc_config_t`; several
  sensors share one ADC. See [API guide](api-guide.md).
- **Degrees from volts.** The divider equation gives the resistance, the Beta equation the temperature, in °C,
  °F or K. See [Conversion methods](conversion.md).
- **Any NTC.** Set R25, B and the series resistor for a 10 kΩ, 100 kΩ or other part; the TDK NTCG163JF103FT1S
  is the default. See [Configuration](configuration.md).
- **Clean readings.** Multi-sample averaging, an exponential filter, a calibration offset and a valid range,
  with an `NtcError` for every failure. See [API guide](api-guide.md#handle-errors).
- **Zero-overhead portability.** The port is a CRTP class with seven small methods: no virtual calls in the
  ADC path, no heap, no RTOS dependency. See [Porting](porting.md).
- **Tested without hardware.** Host unit tests check the conversions against the TDK resistance table. See
  [Testing and CI](testing.md).

## Where to go next

<div class="grid cards" markdown>

-   :material-rocket-launch-outline: **[Quickstart](quickstart.md)**

    ---

    Wire the divider, write the port, read a temperature in about 15 lines.

-   :material-chip: **[Hardware](hardware.md)**

    ---

    The divider, the series resistor, the ADC, self-heating and the part's limits.

-   :material-layers-outline: **[Architecture](architecture.md)**

    ---

    The three layers and what each one owns.

-   :material-code-braces: **[API guide](api-guide.md)**

    ---

    Configure, read, smooth, calibrate and handle errors, task by task.

-   :material-swap-horizontal: **[Porting](porting.md)**

    ---

    Implement the ADC interface for your MCU.

-   :material-book-open-variant: **[API reference](api-reference.md)**

    ---

    Every class and member, generated from the headers.

</div>
