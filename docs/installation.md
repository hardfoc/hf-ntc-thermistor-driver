---
layout: default
title: "🛠️ Installation"
description: "Installation and build instructions for the NTC Thermistor driver"
nav_order: 1
parent: "📚 Documentation"
permalink: /docs/installation/
---

# Installation

This guide covers how to obtain, build, and verify the NTC Thermistor driver library.

## Prerequisites

Before installing the driver, ensure you have:

- **C++11 Compiler**: GCC 4.8+, Clang 3.3+, or MSVC 2013+
- **Build System**: Make or CMake (optional, for building static library)
- **Platform SDK**: Your platform's ADC driver (ESP-IDF, STM32 HAL, Arduino, etc.)

## Obtaining the Source

### Option 1: Git Clone

```bash
git clone https://github.com/hardfoc/hf-ntc-thermistor-driver.git
cd hf-ntc-thermistor-driver
```

### Option 2: Copy Files

Copy the following files into your project:

```
inc/
  ├── ntc_thermistor.hpp
  ├── ntc_adc_interface.hpp
  ├── ntc_types.hpp
  ├── ntc_conversion.hpp
  └── ntc_lookup_table.hpp
src/
  ├── ntc_thermistor.cpp
  ├── ntc_conversion.cpp
  └── ntc_lookup_table.cpp
```

**Note**: The driver uses a header-only template design where implementation files are included by headers. You typically only need to include the header files in your project.

## Building the Library

### Using Make

A simple Makefile is provided:

```bash
make
```

This builds `build/libntc_thermistor.a` which can be linked into your application.

### Using CMake

```cmake
add_subdirectory(ntc-thermistor-driver)
target_link_libraries(your_target PRIVATE ntc_thermistor)
```

Or manually:

```cmake
add_library(ntc_thermistor STATIC
    inc/ntc_thermistor.hpp
    inc/ntc_adc_interface.hpp
    inc/ntc_types.hpp
    inc/ntc_conversion.hpp
    inc/ntc_lookup_table.hpp
    src/ntc_thermistor.cpp
    src/ntc_conversion.cpp
    src/ntc_lookup_table.cpp
)
target_include_directories(ntc_thermistor PUBLIC inc)
```

### Using ESP-IDF Component

The driver can be used as an ESP-IDF component. See the [examples/esp32](../examples/esp32/) directory for component integration examples.

## Running Unit Tests

The library includes unit tests. To run them:

```bash
make test
./build/test
```

Expected output:
```
All tests passed.
```

## Verification

To verify the installation:

1. Include the header in a test file:
   ```cpp
   #include "ntc_thermistor.hpp"
   ```

2. Compile a simple test:
   ```bash
   g++ -std=c++11 -I inc/ -c src/ntc_thermistor.cpp -o test.o
   ```

3. If compilation succeeds, the library is properly installed.

## Next Steps

- Follow the [Quick Start](quickstart.md) guide to create your first application
- Review [Hardware Setup](hardware_setup.md) for voltage divider circuit wiring
- Check [Platform Integration](platform_integration.md) to implement the ADC interface

---

**Navigation**
⬅️ [Back to Index](index.md) | [Next: Quick Start ➡️](quickstart.md)

