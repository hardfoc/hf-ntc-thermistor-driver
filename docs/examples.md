# Examples

This guide provides complete, working examples demonstrating various use cases for the NTC Thermistor driver.

## Example 1: Basic Temperature Reading

This example shows the minimal setup required to read temperature.

```cpp
#include "ntc_thermistor.hpp"

// Implement ADC interface (see platform_integration.md)
class MyAdc : public ntc::AdcInterface<MyAdc> {
    // ... implement all required methods
};

MyAdc adc;
NtcThermistor<MyAdc> thermistor(NtcType::NtcG163Jft103Ft1S, &adc);

if (thermistor.Initialize()) {
    float temp_c;
    if (thermistor.ReadTemperatureCelsius(&temp_c) == NtcError::Success) {
        printf("Temperature: %.2f°C\n", temp_c);
    }
}
```

### Explanation

1. **ADC Interface**: Implement `AdcInterface` for your platform
2. **Create Driver**: Use predefined NTC type or custom configuration
3. **Initialize**: Call `Initialize()` before reading
4. **Read Temperature**: Use `ReadTemperatureCelsius()` to get temperature

---

## Example 2: Complete Reading with All Units

This example demonstrates reading complete temperature information.

```cpp
ntc_reading_t reading;
if (thermistor.ReadTemperature(&reading) == NtcError::Success && reading.is_valid) {
    printf("Temperature: %.2f°C (%.2f°F, %.2fK)\n",
           reading.temperature_celsius,
           reading.temperature_fahrenheit,
           reading.temperature_kelvin);
    printf("Resistance: %.2fΩ, Voltage: %.3fV\n",
           reading.resistance_ohms,
           reading.voltage_volts);
}
```

---

## Example 3: Custom Configuration

This example shows how to configure the driver for custom NTC thermistors.

```cpp
ntc_config_t config = {};
config.type = NtcType::Custom;
config.resistance_at_25c = 10000.0f;  // 10kΩ @ 25°C
config.beta_value = 3950.0f;          // Beta value
config.series_resistance = 10000.0f;   // 10kΩ series resistor
config.reference_voltage = 3.3f;      // 3.3V reference
config.adc_channel = 0;               // ADC channel 0
config.conversion_method = NtcConversionMethod::Mathematical;
config.enable_filtering = true;
config.filter_alpha = 0.1f;
config.sample_count = 5;
config.sample_delay_ms = 10;

NtcThermistor<MyAdc> thermistor(config, &adc);
thermistor.Initialize();
```

---

## Example 4: Calibration

This example demonstrates calibration for improved accuracy.

```cpp
// Calibrate at known temperature (e.g., 25°C room temperature)
if (thermistor.Calibrate(25.0f) == NtcError::Success) {
    printf("Calibration successful\n");
    
    // Read calibrated temperature
    float temp_c;
    thermistor.ReadTemperatureCelsius(&temp_c);
}
```

---

## Example 5: Error Handling

This example shows proper error handling.

```cpp
float temp_c;
NtcError error = thermistor.ReadTemperatureCelsius(&temp_c);

if (error == NtcError::Success) {
    printf("Temperature: %.2f°C\n", temp_c);
} else {
    switch (error) {
        case NtcError::NotInitialized:
            printf("Driver not initialized\n");
            break;
        case NtcError::AdcReadFailed:
            printf("ADC read failed\n");
            break;
        case NtcError::TemperatureOutOfRange:
            printf("Temperature out of range\n");
            break;
        default:
            printf("Error: %d\n", static_cast<int>(error));
    }
}
```

---

## Running the Examples

### ESP32

The examples are available in the [`examples/esp32`](../examples/esp32/) directory.

```bash
cd examples/esp32
idf.py build flash monitor
```

### Other Platforms

Adapt the ADC interface implementation for your platform (see [Platform Integration](platform_integration.md)) and compile with your platform's toolchain.

## Next Steps

- Review the [API Reference](api_reference.md) for method details
- Check [Troubleshooting](troubleshooting.md) if you encounter issues
- Explore the [examples directory](../examples/) for more examples

---

**Navigation**
⬅️ [API Reference](api_reference.md) | [Next: Troubleshooting ➡️](troubleshooting.md) | [Back to Index](index.md)

