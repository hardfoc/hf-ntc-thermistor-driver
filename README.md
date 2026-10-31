---
layout: default
title: "HardFOC NTC Thermistor Driver"
description: "Hardware-agnostic NTC thermistor driver with lookup table and mathematical conversion methods"
nav_order: 1
permalink: /
---

# HF-NTC-Thermistor Driver
**Hardware-agnostic NTC thermistor driver for temperature measurement**

[![C++](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11)
[![License](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![CI](https://github.com/hardfoc/hf-ntc-thermistor-driver/actions/workflows/esp32-examples-build-ci.yml/badge.svg?branch=main)](https://github.com/hardfoc/hf-ntc-thermistor-driver/actions/workflows/esp32-examples-build-ci.yml)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://hardfoc.github.io/hf-ntc-thermistor-driver/)

## 📚 Table of Contents
1. [Overview](#-overview)
2. [Features](#-features)
3. [Quick Start](#-quick-start)
4. [Installation](#-installation)
5. [API Reference](#-api-reference)
6. [Examples](#-examples)
7. [Documentation](#-documentation)
8. [References](#-references)
9. [Contributing](#-contributing)
10. [License](#-license)

## 📦 Overview

> **📖 [📚🌐 Live Complete Documentation](https://hardfoc.github.io/hf-ntc-thermistor-driver/)** - 
> Interactive guides, examples, and step-by-step tutorials

An **NTC (Negative Temperature Coefficient) thermistor** is a temperature-sensitive resistor whose resistance decreases as temperature increases. This driver provides a comprehensive, hardware-agnostic solution for temperature measurement using NTC thermistors in a voltage divider configuration. The driver supports multiple NTC types (including NTCG163JFT103FT1S, NTCG164JF103FT1S, and custom types), dual conversion methods (lookup table for speed, mathematical for accuracy), built-in calibration, filtering, and multi-sample averaging for precise temperature readings.

The driver uses a **CRTP (Curiously Recurring Template Pattern)** design for zero-overhead hardware abstraction, requiring only an ADC interface implementation for your platform. This makes it ideal for embedded systems where performance and memory efficiency are critical.

![NTC voltage-divider topology and resistance-temperature curve](docs/assets/ntc-divider.svg)

### 🔀 Supported Thermistors

The driver ships with calibrated lookup tables (and Beta-equation parameters) for the
TDK NTCG family. Use `NtcType::Custom` for any other part by supplying β / R₂₅ / R_series
in the `ntc_config_t`.

| NTC Type            | R₂₅      | β (K) | Lookup table | Math (Beta) | Notes |
|---------------------|----------|-------|--------------|-------------|-------|
| `NtcG163Jft103Ft1S` | 10 kΩ    | 3435  | ✅           | ✅          | TDK NTCG163JFT103FT1S, ±1 % at 25 °C |
| `NtcG164Jf103Ft1S`  | 10 kΩ    | 3435  | ✅           | ✅          | TDK NTCG164JF103FT1S, ±1 % at 25 °C |
| `NtcG163Jf103Ft1S`  | 10 kΩ    | 3435  | ✅           | ✅          | TDK NTCG163JF103FT1S, ±1 % at 25 °C |
| `Custom`            | user     | user  | —            | ✅          | Any β/R₂₅/R_series via `ntc_config_t` |

## ✨ Features

- ✅ **Hardware Agnostic**: CRTP-based ADC interface for platform independence
- ✅ **Multiple NTC Types**: Pre-configured support for common NTC thermistors
- ✅ **Dual Conversion Methods**: Lookup table (fast) and mathematical (accurate)
- ✅ **Modern C++**: C++11 compatible with template-based design
- ✅ **Zero Overhead**: CRTP design for compile-time polymorphism
- ✅ **Calibration Support**: Built-in calibration and offset compensation
- ✅ **Filtering**: Optional exponential filtering for noise reduction
- ✅ **Multi-Sample Averaging**: Configurable sampling for improved accuracy
- ✅ **Multiple Units**: Celsius, Fahrenheit, and Kelvin support
- ✅ **Comprehensive Error Handling**: Detailed error codes for debugging

## 🚀 Quick Start

```cpp
#include "ntc_thermistor.hpp"

// 1. Implement the ADC interface (see platform_integration.md)
class MyAdc : public ntc::AdcInterface<MyAdc> {
public:
    bool IsInitialized() const { return initialized_; }
    bool EnsureInitialized() { /* your init code */ return true; }
    bool IsChannelAvailable(uint8_t channel) const { return channel < 8; }
    ntc::AdcError ReadChannelCount(uint8_t channel, uint32_t* count) { /* ... */ }
    ntc::AdcError ReadChannelV(uint8_t channel, float* voltage_v) { /* ... */ }
    float GetReferenceVoltage() const { return 3.3f; }
    uint8_t GetResolutionBits() const { return 12; }
private:
    bool initialized_ = false;
};

// 2. Create instances
MyAdc adc;
NtcThermistor<MyAdc> thermistor(NtcType::NtcG163Jft103Ft1S, &adc);

// 3. Initialize and read temperature
if (thermistor.Initialize()) {
    float temp_c;
    if (thermistor.ReadTemperatureCelsius(&temp_c) == NtcError::Success) {
        printf("Temperature: %.2f°C\n", temp_c);
    }
}
```

For detailed setup, see [Installation](docs/installation.md) and [Quick Start Guide](docs/quickstart.md).

## 🔧 Installation

1. **Clone or copy** the driver files into your project
2. **Implement the ADC interface** for your platform (see [Platform Integration](docs/platform_integration.md))
3. **Include the header** in your code:
   ```cpp
   #include "ntc_thermistor.hpp"
   ```
4. Compile with a **C++11** or newer compiler

For detailed installation instructions, see [docs/installation.md](docs/installation.md).

## 📖 API Reference

| Method | Description |
|--------|-------------|
| `Initialize()` | Initialize the driver |
| `ReadTemperatureCelsius(float*)` | Read temperature in Celsius |
| `ReadTemperatureFahrenheit(float*)` | Read temperature in Fahrenheit |
| `ReadTemperatureKelvin(float*)` | Read temperature in Kelvin |
| `ReadTemperature(ntc_reading_t*)` | Read complete temperature information |
| `Calibrate(float)` | Calibrate using reference temperature |
| `SetFiltering(bool, float)` | Enable/disable filtering |

For complete API documentation with source code links, see [docs/api_reference.md](docs/api_reference.md).

## 📊 Examples

For ESP32 examples, see the [examples/esp32](examples/esp32/) directory.
Additional examples for other platforms are available in the [examples](examples/) directory.

Detailed example walkthroughs are available in [docs/examples.md](docs/examples.md).

## 📚 Documentation

For complete documentation, see the [docs directory](docs/index.md).

## 🔗 References

| Resource | Link |
|----------|------|
| TDK NTCG product family | <https://product.tdk.com/en/search/sensor/ntc/chip-ntc-thermistor> |
| Steinhart–Hart equation | <https://en.wikipedia.org/wiki/Steinhart%E2%80%93Hart_equation> |
| β-parameter equation | <https://en.wikipedia.org/wiki/Thermistor#B_or_%CE%B2_parameter_equation> |
| ESP-IDF ADC API | <https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc.html> |
| C++11 language reference | <https://en.cppreference.com/w/cpp/11> |

## 🤝 Contributing

Pull requests and suggestions are welcome! Please follow the existing code style and include tests for new features.

## 📄 License

This project is licensed under the **GNU General Public License v3.0**.
See the [LICENSE](LICENSE) file for details.

