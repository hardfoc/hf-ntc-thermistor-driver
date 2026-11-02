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

### Mock ADC Testing

The comprehensive test suite uses a **Mock ADC** implementation that simulates ADC readings without requiring actual hardware. This allows for:

- **Unit Testing**: Test driver logic without hardware
- **CI/CD Integration**: Run tests in automated pipelines
- **Rapid Development**: Iterate quickly without physical setup

### Hardware Testing (Optional)

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

The comprehensive test suite includes:

1. **Initialization Tests**:
   - Driver initialization
   - Configuration validation
   - ADC interface verification

2. **Temperature Reading Tests**:
   - Celsius, Fahrenheit, and Kelvin readings
   - Complete reading structure validation
   - Error handling verification

3. **Configuration Tests**:
   - Configuration setting/getting
   - Conversion method selection
   - Filtering configuration

4. **Calibration Tests**:
   - Calibration offset
   - Calibration reset
   - Calibration persistence

#### Test Results

The test framework provides:
- Automatic pass/fail tracking
- Execution time measurement
- GPIO14 progress indicator (toggles on each test)
- Comprehensive test summary
- Success percentage calculation

---

## Configuration

### Mock ADC Configuration

Default mock ADC configuration (in test file):

```cpp
MockEsp32Adc mock_adc(3.3f, 12);  // 3.3V reference, 12-bit resolution
```

### NTC Configuration

Default NTC configuration (in test file):

```cpp
NtcThermistor<MockEsp32Adc> thermistor(
    NTC_TYPE_NTCG163JFT103FT1S,  // NTC type
    &mock_adc                    // ADC interface
);
```

### Custom Configuration

For custom NTC setups:

```cpp
ntc_config_t config = NTC_CONFIG_DEFAULT_NTCG163JFT103FT1S();
config.adc_channel = 0;                              // ADC channel
config.series_resistance = 10000.0f;                 // 10kΩ series resistor
config.conversion_method = NTC_CONVERSION_MATHEMATICAL; // Conversion method
config.enable_filtering = true;                      // Enable filtering
config.filter_alpha = 0.1f;                          // Filter coefficient

NtcThermistor<MockEsp32Adc> thermistor(config, &mock_adc);
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
1. **Check mock ADC configuration**:
   - Verify reference voltage matches expected values
   - Check ADC resolution settings

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
- [Datasheets](../../docs/datasheet/) - NTC thermistor datasheets

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

- **Mock ADC**: Tests use mock ADC for hardware-independent testing
- **Hardware Testing**: Optional - connect real NTC thermistor for hardware validation
- **Test Duration**: Comprehensive test suite takes approximately 1-2 minutes to complete
- **Hardware Requirements**: Basic tests work with mock ADC; hardware tests require NTC thermistor circuit

---

<div style="text-align: center; margin: 2em 0; padding: 1em; background: #f8f9fa; border-radius: 8px;">
  <strong>🎯 Ready to test the NTC Thermistor driver?</strong><br>
  Start with: <code>./scripts/build_app.sh ntc_thermistor_comprehensive_test Debug</code>
</div>
