#===============================================================================
# NTC Thermistor Driver - Build Settings
# Shared variables for target name, includes, sources, and dependencies.
# This file is the SINGLE SOURCE OF TRUTH for the driver version.
#===============================================================================

include_guard(GLOBAL)

# Target name
set(HF_NTC_THERMISTOR_TARGET_NAME "hf_ntc_thermistor")

#===============================================================================
# Versioning (single source of truth)
#===============================================================================
set(HF_NTC_THERMISTOR_VERSION_MAJOR 1)
set(HF_NTC_THERMISTOR_VERSION_MINOR 0)
set(HF_NTC_THERMISTOR_VERSION_PATCH 0)
set(HF_NTC_THERMISTOR_VERSION "${HF_NTC_THERMISTOR_VERSION_MAJOR}.${HF_NTC_THERMISTOR_VERSION_MINOR}.${HF_NTC_THERMISTOR_VERSION_PATCH}")
set(HF_NTC_THERMISTOR_VERSION_STRING "${HF_NTC_THERMISTOR_VERSION}")  # used by ntc_thermistor_version.h.in

#===============================================================================
# Generate version header from template (into build directory)
#===============================================================================
set(HF_NTC_THERMISTOR_VERSION_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/../inc/ntc_thermistor_version.h.in")
set(HF_NTC_THERMISTOR_VERSION_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/hf_ntc_thermistor_generated")
set(HF_NTC_THERMISTOR_VERSION_HEADER     "${HF_NTC_THERMISTOR_VERSION_HEADER_DIR}/ntc_thermistor_version.h")

file(MAKE_DIRECTORY "${HF_NTC_THERMISTOR_VERSION_HEADER_DIR}")

if(EXISTS "${HF_NTC_THERMISTOR_VERSION_TEMPLATE}")
    configure_file(
        "${HF_NTC_THERMISTOR_VERSION_TEMPLATE}"
        "${HF_NTC_THERMISTOR_VERSION_HEADER}"
        @ONLY
    )
    message(STATUS "NTC Thermistor driver v${HF_NTC_THERMISTOR_VERSION} — generated ntc_thermistor_version.h in ${HF_NTC_THERMISTOR_VERSION_HEADER_DIR}")
else()
    message(WARNING "ntc_thermistor_version.h.in not found at ${HF_NTC_THERMISTOR_VERSION_TEMPLATE}")
endif()

#===============================================================================
# Public include directories
#===============================================================================
set(HF_NTC_THERMISTOR_PUBLIC_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../inc"
    "${HF_NTC_THERMISTOR_VERSION_HEADER_DIR}"
)

#===============================================================================
# Source files (lookup tables + conversion logic)
#===============================================================================
set(HF_NTC_THERMISTOR_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/../src/ntc_conversion.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/ntc_lookup_table.cpp"
)

#===============================================================================
# ESP-IDF component dependencies
#===============================================================================
set(HF_NTC_THERMISTOR_IDF_REQUIRES driver)
