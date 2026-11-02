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

include(GNUInstallDirs)
set(HF_NTC_THERMISTOR_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/hf_ntc_thermistor")

target_include_directories(${HF_NTC_THERMISTOR_TARGET_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
    $<BUILD_INTERFACE:${HF_NTC_THERMISTOR_VERSION_HEADER_DIR}>
    $<INSTALL_INTERFACE:${HF_NTC_THERMISTOR_INSTALL_INCLUDEDIR}/inc>
)

target_compile_features(${HF_NTC_THERMISTOR_TARGET_NAME} PUBLIC cxx_std_20)

add_library(hf::ntc_thermistor ALIAS ${HF_NTC_THERMISTOR_TARGET_NAME})
set_target_properties(${HF_NTC_THERMISTOR_TARGET_NAME} PROPERTIES
  EXPORT_NAME ntc_thermistor   # installed package target: hf::ntc_thermistor, same as the alias
)

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

# ── Install and CMake package export ─────────────────────────────────────────
# Layout: <prefix>/include/hf_ntc_thermistor/{inc,src}. ntc_thermistor.hpp includes
# "../src/ntc_thermistor.ipp", so the .ipp keeps its place next to inc/.
include(CMakePackageConfigHelpers)

install(TARGETS ${HF_NTC_THERMISTOR_TARGET_NAME} EXPORT hf_ntc_thermistorTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${HF_NTC_THERMISTOR_INSTALL_INCLUDEDIR}/inc
)
install(DIRECTORY inc/ DESTINATION ${HF_NTC_THERMISTOR_INSTALL_INCLUDEDIR}/inc
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
install(DIRECTORY src/ DESTINATION ${HF_NTC_THERMISTOR_INSTALL_INCLUDEDIR}/src
  FILES_MATCHING PATTERN "*.ipp")
install(FILES "${HF_NTC_THERMISTOR_VERSION_HEADER}" DESTINATION ${HF_NTC_THERMISTOR_INSTALL_INCLUDEDIR}/inc)

install(EXPORT hf_ntc_thermistorTargets
  FILE hf_ntc_thermistorTargets.cmake
  NAMESPACE hf::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hf_ntc_thermistor
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/hf_ntc_thermistorConfigVersion.cmake"
  VERSION ${HF_NTC_THERMISTOR_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/hf_ntc_thermistorConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/hf_ntc_thermistorConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hf_ntc_thermistor
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/hf_ntc_thermistorConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/hf_ntc_thermistorConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hf_ntc_thermistor
)
