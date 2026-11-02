# Quick Start

This guide will get you up and running with the NTC Thermistor driver in just a few steps.

## Prerequisites

- [Driver installed](installation.md)
- [Hardware wired](hardware_setup.md) (voltage divider circuit)
- [ADC interface implemented](platform_integration.md)

## Minimal Example

Here's a complete working example:

```cpp
#include "ntc_thermistor.hpp"

// 1. Implement the ADC interface
class MyAdc : public ntc::AdcInterface<MyAdc> {
public:
    bool IsInitialized() const { return initialized_; }
    bool EnsureInitialized() { /* your ADC init */ return true; }
    bool IsChannelAvailable(uint8_t channel) const { return channel < 8; }
    ntc::AdcError ReadChannelCount(uint8_t channel, uint32_t* count) {
        // Your ADC read implementation
        *count = /* read ADC */;
        return ntc::AdcError::Success;
    }
    ntc::AdcError ReadChannelV(uint8_t channel, float* voltage_v) {
        // Your ADC read implementation
        *voltage_v = /* read voltage */;
        return ntc::AdcError::Success;
    }
    float GetReferenceVoltage() const { return 3.3f; }
    uint8_t GetResolutionBits() const { return 12; }
private:
    bool initialized_ = false;
};

// 2. Create instances
MyAdc adc;
NtcThermistor<MyAdc> thermistor(NtcType::NtcG163Jft103Ft1S, &adc);

// 3. Initialize
if (!thermistor.Initialize()) {
    // Handle initialization error
    return;
}

// 4. Read temperature
float temp_c;
if (thermistor.ReadTemperatureCelsius(&temp_c) == NtcError::Success) {
    printf("Temperature: %.2f°C\n", temp_c);
}
```

## Step-by-Step Explanation

### Step 1: Include the Header

```cpp
#include "ntc_thermistor.hpp"
```

### Step 2: Implement the ADC Interface

You need to implement the `AdcInterface` for your platform. See [Platform Integration](platform_integration.md) for detailed examples for ESP32, STM32, and Arduino.

The interface requires:
- `IsInitialized()` - Check if ADC is initialized
- `EnsureInitialized()` - Initialize ADC if needed
- `IsChannelAvailable(uint8_t)` - Check if channel is valid
- `ReadChannelCount(uint8_t, uint32_t*)` - Read raw ADC count
- `ReadChannelV(uint8_t, float*)` - Read voltage
- `GetReferenceVoltage()` - Get ADC reference voltage
- `GetResolutionBits()` - Get ADC resolution

### Step 3: Create Driver Instance

```cpp
MyAdc adc;
NtcThermistor<MyAdc> thermistor(NtcType::NtcG163Jft103Ft1S, &adc);
```

The constructor takes:
- NTC type (predefined or custom)
- Pointer to your ADC interface implementation

### Step 4: Initialize

```cpp
if (!thermistor.Initialize()) {
    // Handle initialization failure
}
```

`Initialize()` validates configuration, checks ADC interface, and prepares the driver for use.

### Step 5: Read Temperature

```cpp
float temp_c;
if (thermistor.ReadTemperatureCelsius(&temp_c) == NtcError::Success) {
    // Use temperature value
}
```

## Expected Output

When running this example:
- Temperature reading in Celsius should be displayed
- Value should be within the configured temperature range
- No error messages should appear

## Troubleshooting

If you encounter issues:

- **Compilation errors**: Check that you've implemented all required ADC interface methods
- **Initialization fails**: Verify hardware connections, ADC channel, and configuration
- **Invalid readings**: Check voltage divider circuit, series resistance, and reference voltage
- **See**: [Troubleshooting](troubleshooting.md) for common issues

## Next Steps

- Explore [Examples](examples.md) for more advanced usage
- Review the [API Reference](api_reference.md) for all available methods
- Check [Configuration](configuration.md) for customization options

---

**Navigation**
⬅️ [Installation](installation.md) | [Next: Hardware Setup ➡️](hardware_setup.md) | [Back to Index](index.md)

