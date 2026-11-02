/**
 * @file quickstart.cpp
 * @brief The Quickstart page's example, compiled by the host build (never run).
 */
#include "my_adc.hpp"

#include <cstdio>

// --8<-- [start:quickstart]
#include "ntc_thermistor.hpp"

int read_board_temperature(MyAdc& adc, float& celsius) {
  ntc_config_t cfg = GetDefaultNtcConfig();   // NTCG163JF103FT1S: 10 kOhm at 25 C, B 3435 K
  cfg.adc_channel = 2;                        // 1. the ADC input on the divider node
  cfg.series_resistance = 10000.0F;           // 2. the resistor from the supply to the node
  cfg.reference_voltage = 3.3F;               // 3. the supply of the divider

  NtcThermistor<MyAdc> ntc(cfg, &adc);
  if (!ntc.Initialize()) {                    // 4. ADC up, channel present, config valid
    return -1;
  }
  const NtcError err = ntc.ReadTemperatureCelsius(&celsius);  // 5. volts -> ohms -> C
  if (err != NtcError::Success) {
    std::printf("NTC: %s\n", NtcThermistor<MyAdc>::GetErrorString(err));
    return -2;
  }
  return 0;
}
// --8<-- [end:quickstart]
