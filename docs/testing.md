# Testing and CI

How the driver is checked: host unit tests against a model of the divider, compiled documentation, an ESP32
build and static analysis, on every pull request.

## Host unit tests

The tests build the real driver against a model of an ADC reading the divider node. A test sets the
thermistor's resistance; the model returns the node voltage and count. Readings are checked against the
nominal resistance table of the TDK NTCG163JF103FT1S (25 °C, 85 °C, 0 °C, 125 °C and −40 °C), and ADC failures
are injected.

```bash
cmake -S . -B build-host -DHF_NTC_THERMISTOR_BUILD_TESTS=ON
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

| File | Covers |
| --- | --- |
| [`tests/test_driver.cpp`](../tests/test_driver.cpp) | Initialisation, Beta accuracy against the table, errors, averaging, open and short, range, calibration, filter, free functions |
| [`tests/mock_adc.hpp`](../tests/mock_adc.hpp) | The ADC and divider model |
| [`tests/test_harness.hpp`](../tests/test_harness.hpp) | A small test registry and check macros |

## Documentation snippets

Every C++ block on this site is included from [`tests/docs/`](../tests/docs/my_adc.hpp), which the host build
compiles (target `hf_ntc_thermistor_doc_snippets`) with the driver instantiated over the documented port. A
change to the API that breaks a page breaks the build first.

## CI workflows

| Workflow | What it checks |
| --- | --- |
| Host unit tests | GCC and Clang build and run the tests and the doc snippets |
| ESP32 examples build | Every app in `examples/esp32/app_config.yml`, for each listed ESP-IDF version and build type |
| C++ lint, C++ analysis | clang-format, clang-tidy and cppcheck |
| Docs | Strict site build, figure and generated-table drift, Doxygen, link check, Lighthouse budget |
| Markdown lint, YAML lint, link check | Repository text files |

The workflows call the shared, versioned workflows of
[hf-general-ci-tools](https://github.com/hardfoc/hf-general-ci-tools) and
[hf-espidf-ci-tools](https://github.com/hardfoc/hf-espidf-ci-tools).

## On hardware

The [ESP32 example](examples.md) runs over a simulated ADC. To check a real board, follow
[Porting](porting.md#check-your-port): a precision resistor in place of the thermistor gives a known 25 °C.
