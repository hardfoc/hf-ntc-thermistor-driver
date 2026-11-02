# Examples

An ESP32-C6 test application that exercises the driver's initialisation, built by CI on every pull request.

The application runs on an ESP32-C6 DevKit. Its ADC port,
[`mock_esp32_adc.hpp`](../examples/esp32/main/mock_esp32_adc.hpp), is simulated: it returns a fixed mid-scale
reading and does not read a pin, so the app checks the driver's logic, not a thermistor. The divider and ADC
values it uses are in [`esp32_ntc_test_config.hpp`](../examples/esp32/main/esp32_ntc_test_config.hpp).

## The apps

<!-- generated: examples -->
| App | What it shows | Source | CI |
| --- | --- | --- | --- |
| `ntc_thermistor_comprehensive_test` :material-star:{ title="featured" } | Driver initialisation test over a simulated ADC, with a pass / fail summary | [`ntc_thermistor_comprehensive_test.cpp`](../examples/esp32/main/ntc_thermistor_comprehensive_test.cpp) | yes |

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
| See the shape of an ADC port | `mock_esp32_adc.hpp` |
| Build a configuration from constants | `init_test_resources()` in `ntc_thermistor_comprehensive_test.cpp` |
| Run tests in FreeRTOS tasks with a pass / fail summary | `TestFramework.h` |

For a port that reads a real ESP32 ADC, see [Porting](porting.md#reference-implementation). The
[example folder's README](../examples/esp32/README.md) has the build commands, the test setup and the expected results.
