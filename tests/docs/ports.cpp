/**
 * @file ports.cpp
 * @brief Instantiates the driver over the documented port, so every member compiles.
 */
#include "my_adc.hpp"

#include "ntc_thermistor.hpp"

template class NtcThermistor<MyAdc>;
