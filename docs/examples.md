# Examples

An ESP32-C6 test application that runs the driver on the chip's ADC, built by CI on every pull request.

The application runs on an ESP32-C6 DevKit with the divider of [Hardware](hardware.md) on GPIO0 (ADC1
channel 0). Its ADC port, [`esp32_ntc_adc.hpp`](../examples/esp32/main/esp32_ntc_adc.hpp), reads the pin with
the ESP-IDF `adc_oneshot` driver and returns the voltage calibrated by `adc_cali`; it also gives the driver a
`DelayMs()` for `sample_delay_ms`. The app initialises the driver, reads the temperature, and checks the lookup
table against the Steinhart-Hart fit on the same reading. The divider and ADC values are in
[`esp32_ntc_test_config.hpp`](../examples/esp32/main/esp32_ntc_test_config.hpp).

## The apps

<!-- generated: examples -->
| App | What it shows | Source | CI |
| --- | --- | --- | --- |
| `ntc_thermistor_comprehensive_test` :material-star:{ title="featured" } | Driver test on the real ESP32-C6 ADC (adc_oneshot + adc_cali), with a pass / fail summary | [`ntc_thermistor_comprehensive_test.cpp`](../examples/esp32/main/ntc_thermistor_comprehensive_test.cpp) | yes |

Build and flash one (from `examples/esp32`, ESP-IDF environment loaded):

```bash
./scripts/build_app.sh ntc_thermistor_comprehensive_test Release
./scripts/flash_app.sh flash_monitor ntc_thermistor_comprehensive_test Release
```

Source of this table: [`examples/esp32/app_config.yml`](../examples/esp32/app_config.yml).
<!-- /generated -->

## Set up once

```bash
git submodule update --init examples/esp32/scripts   # the HardFOC ESP-IDF build scripts
cd examples/esp32
. $IDF_PATH/export.sh                                 # ESP-IDF v5.5
./scripts/build_app.sh list                           # every app and build type
```

The scripts read [`app_config.yml`](../examples/esp32/app_config.yml) (the same file this page's table comes
from) and build each app as its own firmware image.

## What to read in an example

| You want to | Read |
| --- | --- |
| Write an ESP-IDF ADC port | `esp32_ntc_adc.hpp` |
| Build a configuration from constants | `init_test_resources()` in `ntc_thermistor_comprehensive_test.cpp` |
| Run tests in FreeRTOS tasks with a pass / fail summary | `TestFramework.h` |

[Porting](porting.md#reference-implementation) explains the port. The
[example folder's README](../examples/esp32/README.md) has the build commands, the test setup and the expected results.
