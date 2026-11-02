# Platform Integration Guide

This guide explains how to implement the hardware abstraction interface for the NTC Thermistor driver on your platform.

## Understanding CRTP (Curiously Recurring Template Pattern)

The NTC Thermistor driver uses **CRTP** (Curiously Recurring Template Pattern) for hardware abstraction. This design choice provides several critical benefits for embedded systems:

### Why CRTP Instead of Virtual Functions?

#### 1. **Zero Runtime Overhead**
- **Virtual functions**: Require a vtable lookup (indirect call) = ~5-10 CPU cycles overhead per call
- **CRTP**: Direct function calls = 0 overhead, compiler can inline
- **Impact**: In time-critical embedded code, this matters significantly

#### 2. **Compile-Time Polymorphism**
- **Virtual functions**: Runtime dispatch - the compiler cannot optimize across the abstraction boundary
- **CRTP**: Compile-time dispatch - full optimization, dead code elimination, constant propagation
- **Impact**: Smaller code size, faster execution

#### 3. **Memory Efficiency**
- **Virtual functions**: Each object needs a vtable pointer (4-8 bytes)
- **CRTP**: No vtable pointer needed
- **Impact**: Critical in memory-constrained systems (many MCUs have <64KB RAM)

#### 4. **Type Safety**
- **Virtual functions**: Runtime errors if method not implemented
- **CRTP**: Compile-time errors if method not implemented
- **Impact**: Catch bugs at compile time, not in the field

### How CRTP Works

```cpp
// Base template class (from ntc_adc_interface.hpp)
template <typename Derived>
class AdcInterface {
public:
    bool IsInitialized() const {
        // Cast 'this' to Derived* and call the derived implementation
        return static_cast<const Derived*>(this)->IsInitialized();
    }
    
    AdcError ReadChannelV(uint8_t channel, float* voltage_v) {
        return static_cast<Derived*>(this)->ReadChannelV(channel, voltage_v);
    }
    // ... other methods
};

// Your implementation
class MyAdc : public ntc::AdcInterface<MyAdc> {
public:
    // This method is called directly (no virtual overhead)
    bool IsInitialized() const {
        // Your platform-specific ADC code
    }
    
    AdcError ReadChannelV(uint8_t channel, float* voltage_v) {
        // Your platform-specific ADC code
    }
};
```

The key insight: `static_cast<Derived*>(this)` allows the base class to call methods on the derived class **at compile time**, not runtime.

### Performance Comparison

| Aspect | Virtual Functions | CRTP |
|--------|------------------|------|
| Function call overhead | ~5-10 cycles | 0 cycles (inlined) |
| Code size | Larger (vtables) | Smaller (optimized) |
| Memory per object | +4-8 bytes (vptr) | 0 bytes |
| Compile-time checks | No | Yes |
| Optimization | Limited | Full |

## Interface Definition

The NTC Thermistor driver requires you to implement the `AdcInterface` template:

**Location**: [`inc/ntc_adc_interface.hpp#L58`](../inc/ntc_adc_interface.hpp#L58)

```cpp
template <typename Derived>
class AdcInterface {
public:
    // Required methods (implement all of these)
    bool IsInitialized() const;
    bool EnsureInitialized();
    bool IsChannelAvailable(uint8_t channel) const;
    AdcError ReadChannelCount(uint8_t channel, uint32_t* count);
    AdcError ReadChannelV(uint8_t channel, float* voltage_v);
    float GetReferenceVoltage() const;
    uint8_t GetResolutionBits() const;
};
```

**Method Requirements**:
- `IsInitialized()`: Return true if ADC is initialized
- `EnsureInitialized()`: Initialize ADC if needed, return true on success
- `IsChannelAvailable(uint8_t channel)`: Return true if channel is valid
- `ReadChannelCount(uint8_t channel, uint32_t* count)`: Read raw ADC count value
- `ReadChannelV(uint8_t channel, float* voltage_v)`: Read voltage in volts
- `GetReferenceVoltage()`: Return ADC reference voltage (e.g., 3.3V)
- `GetResolutionBits()`: Return ADC resolution in bits (e.g., 12 for 12-bit ADC)

## Implementation Steps

### Step 1: Create Your Implementation Class

```cpp
#include "ntc_adc_interface.hpp"

class MyPlatformAdc : public ntc::AdcInterface<MyPlatformAdc> {
private:
    // Your platform-specific members
    adc_handle_t adc_handle_;
    bool initialized_;
    float reference_voltage_;
    uint8_t resolution_bits_;
    
public:
    // Constructor
    MyPlatformAdc(float ref_voltage, uint8_t resolution) 
        : initialized_(false), reference_voltage_(ref_voltage), 
          resolution_bits_(resolution) {}
    
    // Implement required methods (NO virtual keyword!)
    bool IsInitialized() const {
        return initialized_;
    }
    
    bool EnsureInitialized() {
        if (!initialized_) {
            // Your ADC initialization code
            initialized_ = true;
        }
        return initialized_;
    }
    
    bool IsChannelAvailable(uint8_t channel) const {
        // Check if channel is valid for your platform
        return channel < 8; // Example: 8 channels available
    }
    
    ntc::AdcError ReadChannelCount(uint8_t channel, uint32_t* count) {
        if (!initialized_ || !IsChannelAvailable(channel) || count == nullptr) {
            return ntc::AdcError::NotInitialized;
        }
        // Your ADC read implementation
        *count = /* read ADC count */;
        return ntc::AdcError::Success;
    }
    
    ntc::AdcError ReadChannelV(uint8_t channel, float* voltage_v) {
        if (!initialized_ || !IsChannelAvailable(channel) || voltage_v == nullptr) {
            return ntc::AdcError::NotInitialized;
        }
        // Your ADC read implementation
        *voltage_v = /* read voltage */;
        return ntc::AdcError::Success;
    }
    
    float GetReferenceVoltage() const {
        return reference_voltage_;
    }
    
    uint8_t GetResolutionBits() const {
        return resolution_bits_;
    }
};
```

### Step 2: Platform-Specific Examples

#### ESP32 (ESP-IDF)

**Location**: Based on [`examples/esp32/main/mock_esp32_adc.hpp`](../examples/esp32/main/mock_esp32_adc.hpp)

```cpp
#include "driver/adc.h"
#include "ntc_adc_interface.hpp"

class Esp32Adc : public ntc::AdcInterface<Esp32Adc> {
private:
    adc1_channel_t channel_;
    bool initialized_;
    
public:
    Esp32Adc(adc1_channel_t channel) : channel_(channel), initialized_(false) {}
    
    bool IsInitialized() const {
        return initialized_;
    }
    
    bool EnsureInitialized() {
        if (!initialized_) {
            adc1_config_width(ADC_WIDTH_BIT_12);
            adc1_config_channel_atten(channel_, ADC_ATTEN_DB_11);
            initialized_ = true;
        }
        return initialized_;
    }
    
    bool IsChannelAvailable(uint8_t channel) const {
        return channel < ADC1_CHANNEL_MAX;
    }
    
    ntc::AdcError ReadChannelCount(uint8_t channel, uint32_t* count) {
        if (!initialized_ || count == nullptr) {
            return ntc::AdcError::NotInitialized;
        }
        *count = adc1_get_raw((adc1_channel_t)channel);
        return ntc::AdcError::Success;
    }
    
    ntc::AdcError ReadChannelV(uint8_t channel, float* voltage_v) {
        if (!initialized_ || voltage_v == nullptr) {
            return ntc::AdcError::NotInitialized;
        }
        uint32_t count = 0;
        ntc::AdcError err = ReadChannelCount(channel, &count);
        if (err != ntc::AdcError::Success) return err;
        
        // Convert count to voltage (12-bit ADC, 3.3V reference)
        *voltage_v = (static_cast<float>(count) * 3.3f) / 4095.0f;
        return ntc::AdcError::Success;
    }
    
    float GetReferenceVoltage() const {
        return 3.3f; // ESP32 typical reference
    }
    
    uint8_t GetResolutionBits() const {
        return 12; // ESP32 ADC1 is 12-bit
    }
};
```

#### STM32 (HAL)

```cpp
#include "stm32f4xx_hal.h"
#include "ntc_adc_interface.hpp"

extern ADC_HandleTypeDef hadc1;

class STM32Adc : public ntc::AdcInterface<STM32Adc> {
public:
    bool IsInitialized() const {
        return hadc1.Instance != nullptr;
    }
    
    bool EnsureInitialized() {
        return HAL_ADC_Init(&hadc1) == HAL_OK;
    }
    
    bool IsChannelAvailable(uint8_t channel) const {
        return channel < 16; // STM32 typically has 16 channels
    }
    
    ntc::AdcError ReadChannelCount(uint8_t channel, uint32_t* count) {
        if (!IsInitialized() || count == nullptr) {
            return ntc::AdcError::NotInitialized;
        }
        
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
            *count = HAL_ADC_GetValue(&hadc1);
            return ntc::AdcError::Success;
        }
        return ntc::AdcError::ReadFailed;
    }
    
    ntc::AdcError ReadChannelV(uint8_t channel, float* voltage_v) {
        uint32_t count = 0;
        ntc::AdcError err = ReadChannelCount(channel, &count);
        if (err != ntc::AdcError::Success) return err;
        
        // Convert count to voltage (12-bit ADC, 3.3V reference)
        *voltage_v = (static_cast<float>(count) * 3.3f) / 4095.0f;
        return ntc::AdcError::Success;
    }
    
    float GetReferenceVoltage() const {
        return 3.3f; // STM32 typical reference
    }
    
    uint8_t GetResolutionBits() const {
        return 12; // STM32 ADC typically 12-bit
    }
};
```

#### Arduino

```cpp
#include <Arduino.h>
#include "ntc_adc_interface.hpp"

class ArduinoAdc : public ntc::AdcInterface<ArduinoAdc> {
private:
    uint8_t pin_;
    float reference_voltage_;
    
public:
    ArduinoAdc(uint8_t pin, float ref_voltage = 5.0f) 
        : pin_(pin), reference_voltage_(ref_voltage) {
        pinMode(pin_, INPUT);
    }
    
    bool IsInitialized() const {
        return true; // Arduino ADC is always available
    }
    
    bool EnsureInitialized() {
        analogReference(DEFAULT); // Use default reference
        return true;
    }
    
    bool IsChannelAvailable(uint8_t channel) const {
        // Arduino Uno has 6 analog pins (A0-A5)
        return channel < 6;
    }
    
    ntc::AdcError ReadChannelCount(uint8_t channel, uint32_t* count) {
        if (count == nullptr) {
            return ntc::AdcError::NotInitialized;
        }
        *count = analogRead(channel);
        return ntc::AdcError::Success;
    }
    
    ntc::AdcError ReadChannelV(uint8_t channel, float* voltage_v) {
        if (voltage_v == nullptr) {
            return ntc::AdcError::NotInitialized;
        }
        uint32_t count = 0;
        ntc::AdcError err = ReadChannelCount(channel, &count);
        if (err != ntc::AdcError::Success) return err;
        
        // Convert count to voltage (10-bit ADC on most Arduinos)
        *voltage_v = (static_cast<float>(count) * reference_voltage_) / 1023.0f;
        return ntc::AdcError::Success;
    }
    
    float GetReferenceVoltage() const {
        return reference_voltage_;
    }
    
    uint8_t GetResolutionBits() const {
        return 10; // Most Arduinos have 10-bit ADC
    }
};
```

## Common Pitfalls

### Don't Use Virtual Functions

```cpp
// WRONG - defeats the purpose of CRTP
class MyAdc : public ntc::AdcInterface<MyAdc> {
public:
    virtual bool IsInitialized() const override {  // ❌ Virtual keyword not needed
        // ...
    }
};
```

### Correct CRTP Implementation

```cpp
// CORRECT - no virtual keyword
class MyAdc : public ntc::AdcInterface<MyAdc> {
public:
    bool IsInitialized() const {  // ✅ Direct implementation
        // ...
    }
};
```

### Don't Forget the Template Parameter

```cpp
// WRONG - missing template parameter
class MyAdc : public ntc::AdcInterface {  // ❌ Compiler error
    // ...
};
```

### Correct Template Parameter

```cpp
// CORRECT - pass your class as template parameter
class MyAdc : public ntc::AdcInterface<MyAdc> {  // ✅
    // ...
};
```

## Testing Your Implementation

After implementing the interface, test it:

```cpp
MyPlatformAdc adc(3.3f, 12);
NtcThermistor<MyPlatformAdc> thermistor(NtcType::NtcG163Jft103Ft1S, &adc);

if (thermistor.Initialize()) {
    float temp_c;
    if (thermistor.ReadTemperatureCelsius(&temp_c) == NtcError::Success) {
        // Interface works!
        printf("Temperature: %.2f°C\n", temp_c);
    }
}
```

## Next Steps

- See [Configuration](configuration.md) for driver configuration options
- Check [Examples](examples.md) for complete usage examples
- Review [API Reference](api_reference.md) for all available methods

---

**Navigation**
⬅️ [Hardware Setup](hardware_setup.md) | [Next: Configuration ➡️](configuration.md) | [Back to Index](index.md)

