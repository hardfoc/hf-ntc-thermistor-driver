# Configuration

This guide covers all configuration options available for the NTC Thermistor driver.

## NTC Types

The driver supports predefined NTC types and custom configurations:

### Predefined Types

| NTC Type | Resistance @ 25°C | Beta Value | Temperature Range |
|----------|-------------------|------------|-------------------|
| `NtcG163Jft103Ft1S` | 10kΩ | 3435K | -40°C to +125°C |
| `NtcG164Jf103Ft1S` | 10kΩ | 3435K | -40°C to +125°C |
| `NtcG163Jf103Ft1S` | 10kΩ | 3435K | -40°C to +125°C |
| `Custom` | Configurable | Configurable | Configurable |

**Location**: [`inc/ntc_types.hpp#L71`](../inc/ntc_types.hpp#L71)

### Using Predefined Types

```cpp
// Create with predefined type
NtcThermistor<MyAdc> thermistor(NtcType::NtcG163Jft103Ft1S, &adc);
```

### Custom Configuration

```cpp
ntc_config_t config = {};
config.type = NtcType::Custom;
config.resistance_at_25c = 10000.0f;  // 10kΩ @ 25°C
config.beta_value = 3950.0f;          // Beta value
config.series_resistance = 10000.0f;   // 10kΩ series resistor
config.reference_voltage = 3.3f;      // 3.3V reference
config.adc_channel = 0;               // ADC channel
// ... set other parameters

NtcThermistor<MyAdc> thermistor(config, &adc);
```

## Conversion Methods

The driver supports three conversion methods:

| Method | Speed | Accuracy | Use Case |
|--------|-------|----------|----------|
| `LookupTable` | Fast | Good | Real-time applications |
| `Mathematical` | Slower | High | Precision applications |
| `Auto` | Balanced | Balanced | General purpose |

**Location**: [`inc/ntc_types.hpp#L95`](../inc/ntc_types.hpp#L95)

### Setting Conversion Method

```cpp
thermistor.SetConversionMethod(NtcConversionMethod::Mathematical);
```

## Configuration Structure

The `ntc_config_t` structure contains all configuration parameters:

**Location**: [`inc/ntc_types.hpp#L120`](../inc/ntc_types.hpp#L120)

```cpp
struct ntc_config_t {
    NtcType type;                      // NTC type
    float resistance_at_25c;           // Resistance at 25°C (ohms)
    float beta_value;                  // Beta value (K)
    float reference_voltage;           // ADC reference voltage (V)
    float series_resistance;           // Series resistor (ohms)
    float calibration_offset;          // Calibration offset (°C)
    NtcConversionMethod conversion_method; // Conversion method
    uint8_t adc_channel;               // ADC channel number
    uint32_t adc_resolution_bits;      // ADC resolution (bits)
    uint32_t sample_count;              // Number of samples to average
    uint32_t sample_delay_ms;           // Delay between samples (ms)
    float min_temperature;             // Minimum temperature (°C)
    float max_temperature;             // Maximum temperature (°C)
    bool enable_filtering;             // Enable temperature filtering
    float filter_alpha;                // Filter alpha (0.0-1.0)
};
```

## Runtime Configuration

### Setting Configuration

```cpp
ntc_config_t config;
thermistor.GetConfiguration(&config);
config.series_resistance = 10000.0f;
config.enable_filtering = true;
thermistor.SetConfiguration(config);
```

### Individual Configuration Methods

```cpp
// Set conversion method
thermistor.SetConversionMethod(NtcConversionMethod::Mathematical);

// Set voltage divider
thermistor.SetVoltageDivider(10000.0f); // 10kΩ series resistor

// Set reference voltage
thermistor.SetReferenceVoltage(3.3f);

// Set beta value
thermistor.SetBetaValue(3950.0f);

// Set ADC channel
thermistor.SetAdcChannel(0);

// Set sampling parameters
thermistor.SetSamplingParameters(5, 10); // 5 samples, 10ms delay

// Enable filtering
thermistor.SetFiltering(true, 0.1f); // Enable with alpha=0.1
```

## Default Values

| Parameter | Default | Description |
|-----------|---------|-------------|
| `resistance_at_25c` | 10000.0f | 10kΩ (for 10kΩ NTC) |
| `beta_value` | 3435.0f | Beta value for NTCG163JFT103FT1S |
| `reference_voltage` | 3.3f | 3.3V (typical MCU reference) |
| `series_resistance` | 10000.0f | 10kΩ (matches NTC at 25°C) |
| `conversion_method` | `Auto` | Auto-select conversion method |
| `sample_count` | 1 | Single sample (no averaging) |
| `sample_delay_ms` | 0 | No delay between samples |
| `enable_filtering` | false | Filtering disabled |
| `filter_alpha` | 0.1f | Filter coefficient |

## Recommended Settings

### For Real-Time Applications

```cpp
ntc_config_t config = {};
config.conversion_method = NtcConversionMethod::LookupTable;
config.sample_count = 1;
config.sample_delay_ms = 0;
config.enable_filtering = false;
```

### For High Accuracy

```cpp
ntc_config_t config = {};
config.conversion_method = NtcConversionMethod::Mathematical;
config.sample_count = 10;
config.sample_delay_ms = 10;
config.enable_filtering = true;
config.filter_alpha = 0.1f;
```

### For Noisy Environments

```cpp
ntc_config_t config = {};
config.sample_count = 5;
config.sample_delay_ms = 10;
config.enable_filtering = true;
config.filter_alpha = 0.2f; // Higher alpha = more filtering
```

## Calibration

### Calibrate Using Reference Temperature

```cpp
// Calibrate at known temperature (e.g., 25°C room temperature)
if (thermistor.Calibrate(25.0f) == NtcError::Success) {
    // Calibration successful
}
```

### Manual Calibration Offset

```cpp
// Set calibration offset manually
thermistor.SetCalibrationOffset(1.5f); // +1.5°C offset

// Get current offset
float offset;
thermistor.GetCalibrationOffset(&offset);

// Reset calibration
thermistor.ResetCalibration();
```

## Next Steps

- See [Examples](examples.md) for configuration examples
- Review [API Reference](api_reference.md) for all configuration methods

---

**Navigation**
⬅️ [Platform Integration](platform_integration.md) | [Next: API Reference ➡️](api_reference.md) | [Back to Index](index.md)

