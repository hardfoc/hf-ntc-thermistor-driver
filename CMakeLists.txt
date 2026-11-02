# =============================================================================
# hf-ntc-thermistor-driver — Root CMakeLists.txt
# =============================================================================
# Platform-agnostic build for the HardFOC NTC Thermistor driver.
# For ESP-IDF builds use the component wrapper instead.
# =============================================================================
cmake_minimum_required(VERSION 3.16)

project(hf_ntc_thermistor
  VERSION   1.0.0
  LANGUAGES CXX
)

# ── Import shared build settings ─────────────────────────────────────────────
include(${CMAKE_CURRENT_LIST_DIR}/cmake/hf_ntc_thermistor_build_settings.cmake)

# ── Library target ───────────────────────────────────────────────────────────
add_library(${HF_NTC_THERMISTOR_TARGET_NAME} STATIC ${HF_NTC_THERMISTOR_SOURCE_FILES})

target_include_directories(${HF_NTC_THERMISTOR_TARGET_NAME} PUBLIC ${HF_NTC_THERMISTOR_PUBLIC_INCLUDE_DIRS})

target_compile_features(${HF_NTC_THERMISTOR_TARGET_NAME} PUBLIC cxx_std_20)

# ── Optional warnings (off by default, enable with -DHF_NTC_THERMISTOR_ENABLE_WARNINGS=ON)
option(HF_NTC_THERMISTOR_ENABLE_WARNINGS "Enable compiler warnings" OFF)
if(HF_NTC_THERMISTOR_ENABLE_WARNINGS)
  target_compile_options(${HF_NTC_THERMISTOR_TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Host unit tests and the compiled documentation snippets (tests/) ────────
option(HF_NTC_THERMISTOR_BUILD_TESTS "Build the host unit tests in tests/" OFF)
if(HF_NTC_THERMISTOR_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ── CMake package export ─────────────────────────────────────────────────────
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/hf_ntc_thermistorConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/hf_ntc_thermistorConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/hf_ntc_thermistorConfig.cmake"
  INSTALL_DESTINATION lib/cmake/hf_ntc_thermistor
)
