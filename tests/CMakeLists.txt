# =============================================================================
# hf-ntc-thermistor-driver: host unit tests
# =============================================================================
# Builds the driver against a model of an ADC reading an NTC divider and checks
# the conversions against the thermistor's datasheet table. No hardware and no
# ESP-IDF needed:
#
#   cmake -S . -B build-host -DHF_NTC_THERMISTOR_BUILD_TESTS=ON
#   cmake --build build-host
#   ctest --test-dir build-host --output-on-failure
# =============================================================================

add_executable(hf_ntc_thermistor_host_tests
  test_main.cpp
  test_driver.cpp
)

target_link_libraries(hf_ntc_thermistor_host_tests PRIVATE ${HF_NTC_THERMISTOR_TARGET_NAME})
target_include_directories(hf_ntc_thermistor_host_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(hf_ntc_thermistor_host_tests PRIVATE cxx_std_20)
target_compile_options(hf_ntc_thermistor_host_tests PRIVATE -Wall -Wextra)

add_test(NAME hf_ntc_thermistor_host_tests COMMAND hf_ntc_thermistor_host_tests)

# The code shown on the documentation site (docs/ includes these files with pymdownx.snippets), built so a
# page cannot show code that does not compile against the current headers. Compiled, never linked or run.
add_library(hf_ntc_thermistor_doc_snippets OBJECT
  docs/quickstart.cpp
  docs/api_tasks.cpp
  docs/ports.cpp
)
target_link_libraries(hf_ntc_thermistor_doc_snippets PRIVATE ${HF_NTC_THERMISTOR_TARGET_NAME})
target_include_directories(hf_ntc_thermistor_doc_snippets PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/docs)
target_compile_features(hf_ntc_thermistor_doc_snippets PRIVATE cxx_std_20)
target_compile_options(hf_ntc_thermistor_doc_snippets PRIVATE -Wall -Wextra)
