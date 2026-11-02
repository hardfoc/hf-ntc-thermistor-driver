# Installation

Add the driver to a CMake or ESP-IDF project. It is a small C++20 library: two source files and a header-only
template, with no dependencies.

## Requirements

- A C++20 compiler (the headers use designated initialisers): GCC 10+, Clang 12+ or a current MSVC.
- The C++ standard library's `<cmath>`. No RTOS, heap or exceptions: every driver call is `noexcept` and
  returns `bool` or an `NtcError`.

## Get the source

A git submodule keeps the driver pinned to a version you chose:

```bash
git submodule add https://github.com/hardfoc/hf-ntc-thermistor-driver.git external/hf-ntc-thermistor-driver
git submodule update --init external/hf-ntc-thermistor-driver
```

Only `inc/`, `src/`, `cmake/` and `CMakeLists.txt` are needed to build. The examples have their own submodule
(`examples/esp32/scripts`); you do not need it to use the driver.

## CMake

```cmake
add_subdirectory(external/hf-ntc-thermistor-driver)
target_link_libraries(my_app PRIVATE hf_ntc_thermistor)
```

The `hf_ntc_thermistor` target is a static library (`src/ntc_conversion.cpp`, `src/ntc_lookup_table.cpp`). It
carries the include directories and requires C++20 (`cxx_std_20`), so your target does not need to set either.

| Option | Default | Effect |
| --- | --- | --- |
| `HF_NTC_THERMISTOR_ENABLE_WARNINGS` | `OFF` | `-Wall -Wextra -Wpedantic` on the driver sources |
| `HF_NTC_THERMISTOR_BUILD_TESTS` | `OFF` | Builds the host unit tests in `tests/` ([Testing and CI](testing.md)) |

## ESP-IDF

The ESP-IDF component wrapper is
[`examples/esp32/components/hf_ntc_thermistor/`](../examples/esp32/components/hf_ntc_thermistor/CMakeLists.txt).
Copy that folder into your project's `components/` (or point `EXTRA_COMPONENT_DIRS` at it) and set the
`HF_NTC_THERMISTOR_ROOT` cache variable to the driver checkout; the wrapper stops with a clear message when the
path is wrong. Your component then declares `REQUIRES hf_ntc_thermistor`.

## How the build is organised

| Layer | File | Role |
| --- | --- | --- |
| Settings | [`cmake/hf_ntc_thermistor_build_settings.cmake`](../cmake/hf_ntc_thermistor_build_settings.cmake) | Sources, include directories, version: the single source of truth |
| Library | [`CMakeLists.txt`](../CMakeLists.txt) | Static library, `install()` rules and the CMake package |
| ESP-IDF | [`examples/esp32/components/hf_ntc_thermistor/CMakeLists.txt`](../examples/esp32/components/hf_ntc_thermistor/CMakeLists.txt) | IDF component wrapper over the same settings |

The version is set once in the settings file and written into `ntc_thermistor_version.h` at configure time.
Read it at run time:

```cpp
const char* v = GetNtcThermistorDriverVersion();   // "1.0.0"
```

## Install and `find_package()`

The root `CMakeLists.txt` installs the library, the headers (with `src/ntc_thermistor.ipp`, which
`ntc_thermistor.hpp` includes), the generated version header and a CMake package:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix /opt/hardfoc
```

```cmake
find_package(hf_ntc_thermistor 1.0 REQUIRED)   # with /opt/hardfoc in CMAKE_PREFIX_PATH
target_link_libraries(my_app PRIVATE hf::ntc_thermistor)
```

`hf::ntc_thermistor` is also defined as an alias by `add_subdirectory()`, so the same `target_link_libraries()`
line works both ways. Headers install under `include/hf_ntc_thermistor/inc/`.

## Next

[Quickstart](quickstart.md) wires a divider and reads a temperature; [Porting](porting.md) writes the ADC
interface for your MCU.
