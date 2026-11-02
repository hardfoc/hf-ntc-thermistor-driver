# NTC Thermistor ESP32-C6 Comprehensive Test Suite

This directory contains comprehensive test suites for the NTC Thermistor driver using the ESP32-C6 DevKit-M-1.

## Table of Contents

- [Hardware Overview](#hardware-overview)
- [Test Setup](#test-setup)
- [Building the Tests](#building-the-tests)
- [Running the Tests](#running-the-tests)
- [Test Suites](#test-suites)
- [Troubleshooting](#troubleshooting)

---

## Hardware Overview

### ESP32-C6 DevKit-M-1

The ESP32-C6 DevKit-M-1 serves as the host controller for testing the NTC thermistor driver.

```
┌─────────────────────────────────────────────────┐
│        ESP32-C6 DevKit-M-1                      │
│                                                 │
│  ┌──────────────────────────────────────────┐   │
│  │        ESP32-C6 Microcontroller          │   │
│  │                                          │   │
│  │  GPIO Pins:                              │   │
│  │  • ADC: GPIO0-GPIO5 (ADC1 channels)      │   │
│  │  • Test Indicator: GPIO14                │   │
│  └──────────────────────────────────────────┘   │
│                                                 │
│  USB-C Connector                                │
│  (Power + Serial Communication)                 │
└─────────────────────────────────────────────────┘
```

### NTC Thermistor Circuit

The NTC thermistor is connected in a voltage divider configuration:

```
┌─────────────────────────────────────────────────┐
│      NTC Thermistor Circuit                     │
│                                                 │
│  ┌───────────────────────────────────────────┐  │
│  │        Voltage Divider                    │  │
│  │                                           │  │
│  │  3.3V                                     │  │
│  │   │                                       │  │
│  │   ├── [Series Resistor] ──┬── ADC ────    │  │
│  │                           │  Channel      │  │
│  │                           │               │  │
│  │                    [NTC Thermistor]       │  │
│  │                           │               │  │
│  │                          GND              │  │
│  └───────────────────────────────────────────┘  │
│                                                 │
│  Typical Values:                                │
│  • NTC: 10kΩ @ 25°C (NTCG163JFT103FT1S)         │
│  • Series Resistor: 10kΩ                        │
│  • Reference Voltage: 3.3V                      │
└─────────────────────────────────────────────────┘
```

---

## Test Setup

### ADC port

The test app reads the thermistor divider through `main/esp32_ntc_adc.hpp`, a port over the ESP-IDF
`adc_oneshot` driver with `adc_cali` calibration (ADC1 channel 0 = GPIO0 at 12 dB by default; see
`main/esp32_ntc_test_config.hpp`). It needs the divider below wired to the board: with nothing on the pin the
read test fails with a conversion or range error.

### Hardware

For hardware testing with a real NTC thermistor:

#### Pin Connections

| Component | ESP32-C6 GPIO | Function | Notes |
|-----------|---------------|----------|-------|
| NTC Voltage Divider | GPIO0-GPIO5 | ADC Input | Any ADC1 channel |
| Series Resistor | - | Voltage Divider | 10kΩ typical |
| NTC Thermistor | - | Temperature Sensor | 10kΩ @ 25°C typical |
| Test Indicator | GPIO14 | Visual Progress | Optional |

#### Circuit Configuration

1. **Voltage Divider Setup**:
   - Connect 3.3V to series resistor (10kΩ)
   - Connect series resistor to NTC thermistor
   - Connect NTC thermistor to GND
   - Connect ADC channel to junction between resistor and NTC

2. **Power Connections**:
   - Connect ESP32-C6 3.3V to voltage divider
   - Connect ESP32-C6 GND to circuit ground

3. **ADC Configuration**:
   - Configure ADC channel in driver configuration
   - Set reference voltage (typically 3.3V)
   - Set ADC resolution (12-bit for ESP32-C6)

---

## Building the Tests

### Prerequisites

1. **Install ESP-IDF** (if not already installed):

   ```bash
   # Clone ESP-IDF
   git clone --recursive https://github.com/espressif/esp-idf.git
   cd esp-idf
   ./install.sh esp32c6
   . ./export.sh
   ```

2. **Navigate to examples directory**:

   ```bash
   cd external/hf-ntc-thermistor-driver/examples/esp32
   ```

### Build Commands

```bash
# List available apps
./scripts/build_app.sh list

# Build comprehensive test (Debug)
./scripts/build_app.sh ntc_thermistor_comprehensive_test Debug

# Build comprehensive test (Release)
./scripts/build_app.sh ntc_thermistor_comprehensive_test Release
```

### Build Output

The build process will:
- Compile the NTC thermistor driver
- Compile the test suite
- Link all dependencies
- Generate binary files in `build-app-ntc_thermistor_comprehensive_test-type-{Debug|Release}-target-esp32c6-idf-{version}/`

---

## Running the Tests

### Flash and Monitor

```bash
# Flash and monitor (Debug)
./scripts/flash_app.sh flash_monitor ntc_thermistor_comprehensive_test Debug

# Flash only
./scripts/flash_app.sh flash ntc_thermistor_comprehensive_test Debug

# Monitor only
./scripts/flash_app.sh monitor
```

### Test Execution

The comprehensive test suite runs automatically on boot and provides:
- Real-time test progress via GPIO14 indicator
- Serial output with detailed test results
- Automatic test summary at completion

### GPIO14 Test Indicator

GPIO14 toggles between HIGH/LOW for each completed test, providing visual feedback:
- Use oscilloscope or logic analyzer to monitor
- Useful for automated test verification
- Blinks 5 times at section start/end

---

## Test Suites

### Basic Tests

The comprehensive test suite runs three tests on the board:

1. **basic_initialization**: `Initialize()` creates the ADC1 oneshot unit and the calibration scheme and
   accepts the configured channel.
2. **read_temperature**: `ReadTemperature()` on the real divider; logs the node voltage, raw count,
   thermistor resistance and temperature. Fails on an ADC error or an open or shorted thermistor.
3. **lookup_matches_steinhart_hart**: the driver's lookup-table reading and
   `NTC::ConvertResistanceToTemperatureDefault()` agree within 0.5 °C on the same sensor.

#### Test Results

The test framework provides:
- Automatic pass/fail tracking
- Execution time measurement
- GPIO14 progress indicator (toggles on each test)
- Comprehensive test summary
- Success percentage calculation

---

## Configuration

### ADC port

`main/esp32_ntc_adc.hpp` (class `Esp32NtcAdc`) reads one ADC1 channel at 12 dB attenuation with
`adc_oneshot_read()` and returns the voltage from `adc_cali_raw_to_voltage()`. Its `DelayMs()` gives the driver a
real wait for `sample_delay_ms`.

```cpp
Esp32NtcAdc adc(0);                                   // ADC1 channel 0 = GPIO0 on the ESP32-C6
```

### NTC configuration

`init_test_resources()` builds the configuration from `main/esp32_ntc_test_config.hpp`: the TDK
NTCG163JF103FT1S (R25 = 10 kΩ, B25/85 = 3435 K), a 10 kΩ series resistor on 3.3 V, the lookup-table
method, and 16 samples 5 ms apart.

```cpp
ntc_config_t config = GetDefaultNtcConfig();
config.type = NtcType::NtcG163Jf103Ft1S;
config.adc_channel = 0;
config.series_resistance = 10000.0F;
config.conversion_method = NtcConversionMethod::LookupTable;
config.sample_count = 16;
config.sample_delay_ms = 5;

NtcThermistor<Esp32NtcAdc> thermistor(config, &adc);
```

---

## Troubleshooting

### Build Errors

**Symptoms**: CMake or compilation errors

**Solutions**:
1. **Verify ESP-IDF version**:
   ```bash
   idf.py --version
   # Should show ESP-IDF v5.5 or compatible
   ```

2. **Clean and rebuild**:
   ```bash
   idf.py fullclean
   ./scripts/build_app.sh ntc_thermistor_comprehensive_test Debug
   ```

3. **Check component paths**:
   - Verify component CMakeLists.txt paths
   - Ensure source files are accessible

### Test Failures

**Symptoms**: Specific tests fail

**Solutions**:
1. **Check the divider on the ADC pin**:
   - `read_temperature` logs the node voltage: about 1.65 V at 25 °C
   - 0 V or 3.3 V means a shorted or open thermistor

2. **Review test logs**:
   - Check which specific test failed
   - Review error messages in serial output

3. **Verify driver state**:
   - Ensure driver is properly initialized
   - Check configuration values

### Hardware Testing Issues

**Symptoms**: Incorrect temperature readings with real hardware

**Solutions**:
1. **Check circuit connections**:
   - Verify voltage divider connections
   - Check series resistor value (should match configuration)
   - Ensure proper power connections

2. **Verify ADC configuration**:
   - Check ADC channel matches physical connection
   - Verify reference voltage (typically 3.3V)
   - Confirm ADC resolution settings

3. **Calibrate if needed**:
   - Use known reference temperature
   - Apply calibration offset
   - Verify with multiple temperature points

---

## Additional Resources

- [NTC Driver Documentation](../../README.md) - Complete driver documentation
- [API Reference](../../docs/) - Detailed API documentation
- [NTCG163JF103FT1S spec sheet][tdk-spec] - TDK

[tdk-spec]: https://product.tdk.com/en/search/sensor/ntc/chip-ntc-thermistor/info?part_no=NTCG163JF103FT1S

---

## Quick Reference

### Build Commands

```bash
# List available apps
./scripts/build_app.sh list

# Build comprehensive test
./scripts/build_app.sh ntc_thermistor_comprehensive_test Debug

# Flash and monitor
./scripts/flash_app.sh flash_monitor ntc_thermistor_comprehensive_test Debug
```

### Test Execution

The comprehensive test suite runs automatically on boot and provides:
- Real-time test progress via GPIO14 indicator
- Serial output with detailed test results
- Automatic test summary at completion

### GPIO14 Test Indicator

GPIO14 toggles between HIGH/LOW for each completed test, providing visual feedback:
- Use oscilloscope or logic analyzer to monitor
- Useful for automated test verification
- Blinks 5 times at section start/end

---

## Notes

- **Real ADC**: The app reads the ESP32-C6 ADC; wire the divider before running it
- **CI**: CI builds the app for every pull request but does not run it
- **Test Duration**: Comprehensive test suite takes approximately 1-2 minutes to complete
- **Hardware Requirements**: An NTC thermistor divider on the configured ADC1 channel

---

<div style="text-align: center; margin: 2em 0; padding: 1em; background: #f8f9fa; border-radius: 8px;">
  <strong>🎯 Ready to test the NTC Thermistor driver?</strong><br>
  Start with: <code>./scripts/build_app.sh ntc_thermistor_comprehensive_test Debug</code>
</div>
