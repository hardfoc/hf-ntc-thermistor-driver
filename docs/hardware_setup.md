# Hardware Setup

![NTC voltage-divider topology and resistance-temperature curve](assets/ntc-divider.svg)

This guide covers the physical connections and hardware requirements for using an NTC thermistor with this driver.

## Voltage Divider Circuit

NTC thermistors are used in a **voltage divider** configuration to measure temperature. The circuit consists of:

1. **NTC Thermistor**: Temperature-sensitive resistor (resistance decreases with temperature)
2. **Series Resistor**: Fixed-value resistor (typically matches NTC resistance at 25°C)
3. **ADC Input**: Microcontroller ADC channel measures the voltage at the divider point

### Circuit Diagram

```
                    Voltage Divider Circuit
                    ┌─────────────────────┐
        3.3V        │                     │
         │          │   [Series Resistor] │
         │          │   (e.g., 10kΩ)      │
         ├──────────┤                     │
         │          │         │            │
         │          │         ├── ADC ────┤ To MCU ADC Channel
         │          │         │            │
         │          │   [NTC Thermistor]   │
         │          │   (e.g., 10kΩ @25°C)│
         │          │         │            │
         │          └─────────┼────────────┘
         │                    │
        GND ──────────────────┴── GND
```

### How It Works

1. **Voltage Divider**: The series resistor and NTC thermistor form a voltage divider
2. **Temperature Change**: As temperature changes, NTC resistance changes
3. **Voltage Change**: The voltage at the divider point (ADC input) changes proportionally
4. **ADC Reading**: The MCU ADC reads this voltage
5. **Temperature Calculation**: The driver converts voltage → resistance → temperature

### Voltage Calculation

The voltage at the ADC input is:

```
V_adc = V_ref × (R_ntc / (R_series + R_ntc))
```

Where:
- `V_ref` = Reference voltage (typically 3.3V)
- `R_series` = Series resistor value (e.g., 10kΩ)
- `R_ntc` = NTC thermistor resistance (varies with temperature)

## Pin Connections

### Basic Connections

```
MCU              Voltage Divider
─────────────────────────────────
3.3V      ────── Series Resistor ────┬─── ADC Channel
                                     │
                                     ├─── NTC Thermistor
                                     │
GND       ────── GND ────────────────┴─── GND
```

### Connection Details

| Component | Connection | Notes |
|-----------|------------|-------|
| Series Resistor | One end to 3.3V, other end to ADC input | Typically 10kΩ for 10kΩ NTC |
| NTC Thermistor | One end to ADC input, other end to GND | Connect to ADC channel |
| ADC Channel | To voltage divider point | Configure in driver |
| Power | 3.3V to series resistor | Must match reference voltage |
| Ground | Common GND | Shared ground reference |

## Component Selection

### NTC Thermistor

**Recommended Types**:
- **NTCG163JFT103FT1S**: 10kΩ @ 25°C, β=3435K, -40°C to +125°C
- **NTCG164JF103FT1S**: 10kΩ @ 25°C, β=3435K, -40°C to +125°C
- **NTCG163JF103FT1S**: 10kΩ @ 25°C, β=3435K, -40°C to +125°C

**Custom NTC**: Any NTC thermistor can be used with custom configuration

### Series Resistor

**Selection Rule**: Series resistor should match NTC resistance at the center of your temperature range (typically 25°C).

**Common Values**:
- **10kΩ**: For 10kΩ NTC thermistors (most common)
- **1kΩ**: For 1kΩ NTC thermistors
- **100kΩ**: For 100kΩ NTC thermistors

**Tolerance**: 1% or better recommended for accuracy

### ADC Requirements

- **Resolution**: 10-bit minimum, 12-bit recommended
- **Reference Voltage**: Must match power supply (typically 3.3V)
- **Input Range**: 0V to reference voltage
- **Channel**: Any available ADC channel

## Physical Layout Recommendations

- **Keep traces short**: Minimize noise pickup on ADC input
- **Ground plane**: Use ground plane for noise reduction
- **Decoupling**: Place 100nF capacitor near ADC reference pin
- **Routing**: Route ADC input away from noise sources (switching regulators, motors)
- **Shielding**: Consider shielding for long ADC traces

## Example Wiring

### ESP32-C6 Example

```
ESP32-C6              Voltage Divider
─────────────────────────────────────
3.3V ────[10kΩ]───┬─── GPIO4 (ADC1_CH0)
                  │
              [NTC 10kΩ]
                  │
GND ───────────────┴─── GND
```

**Configuration**:
- ADC Channel: 0 (GPIO4)
- Series Resistance: 10kΩ
- Reference Voltage: 3.3V
- NTC Type: NTCG163JFT103FT1S (10kΩ @ 25°C)

## Configuration Values

When setting up the driver, configure:

```cpp
ntc_config_t config = {};
config.adc_channel = 0;              // ADC channel number
config.series_resistance = 10000.0f; // 10kΩ series resistor
config.reference_voltage = 3.3f;     // 3.3V reference
config.resistance_at_25c = 10000.0f; // NTC resistance at 25°C
config.beta_value = 3435.0f;         // NTC beta value
```

**Important**: These values must match your actual hardware configuration!

## Next Steps

- Verify connections with a multimeter
- Use an oscilloscope to verify voltage divider output
- Proceed to [Quick Start](quickstart.md) to test the connection
- Review [Platform Integration](platform_integration.md) for software setup

---

**Navigation**
⬅️ [Quick Start](quickstart.md) | [Next: Platform Integration ➡️](platform_integration.md) | [Back to Index](index.md)

