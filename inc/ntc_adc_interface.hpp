/**
 * @file ntc_adc_interface.hpp
 * @brief CRTP-based template interface for ADC used by NTC thermistor driver
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */
#pragma once
#include <cstdint>

namespace ntc {

/**
 * @brief ADC error codes
 */
enum class AdcError : uint8_t {
  Success = 0,    ///< Operation successful
  NotInitialized, ///< ADC not initialized
  InvalidChannel, ///< Invalid ADC channel
  ReadFailed,     ///< ADC read operation failed
  Timeout,        ///< Operation timeout
  HardwareError   ///< Hardware-level error
};

/**
 * @brief CRTP-based template interface for ADC operations
 *
 * This template class provides a hardware-agnostic interface for ADC operations
 * using the CRTP pattern. Platform-specific implementations should inherit from
 * this template with themselves as the template parameter.
 *
 * Benefits of CRTP:
 * - Compile-time polymorphism (no virtual function overhead)
 * - Static dispatch instead of dynamic dispatch
 * - Better optimization opportunities for the compiler
 *
 * Example usage:
 * @code
 * class Esp32C6Adc : public ntc::AdcInterface<Esp32C6Adc> {
 * public:
 *   bool IsInitialized() const { ... }
 *   bool EnsureInitialized() { ... }
 *   // ... implement other methods
 *   void DelayMs(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }  // optional
 * };
 * @endcode
 *
 * Optional hook: `void DelayMs(uint32_t ms)` in the derived class. When the
 * port provides it, the driver waits `sample_delay_ms` between averaged
 * samples through it (an RTOS sleep or a timed busy-wait). A port without it
 * takes the samples back to back and `sample_delay_ms` has no effect. The hook
 * is detected at compile time; it is deliberately not declared in this base
 * class.
 *
 * @tparam Derived The derived class type (CRTP pattern)
 */
template <typename Derived> class AdcInterface {
public:
  /**
   * @brief Check if ADC is initialized
   * @return true if ADC is initialized, false otherwise
   */
  [[nodiscard]] bool IsInitialized() const {
    return static_cast<const Derived *>(this)->IsInitialized();
  }

  /**
   * @brief Ensure ADC is initialized, initializing if necessary
   * @return true if ADC is initialized (or was successfully initialized), false
   * otherwise
   */
  bool EnsureInitialized() {
    return static_cast<Derived *>(this)->EnsureInitialized();
  }

  /**
   * @brief Check if a specific ADC channel is available
   * @param channel ADC channel number to check
   * @return true if channel is available, false otherwise
   */
  [[nodiscard]] bool IsChannelAvailable(uint8_t channel) const {
    return static_cast<const Derived *>(this)->IsChannelAvailable(channel);
  }

  /**
   * @brief Read raw ADC count value from specified channel
   * @param channel ADC channel to read from
   * @param count Pointer to store the raw count value
   * @return AdcError::SUCCESS on success, error code otherwise
   */
  AdcError ReadChannelCount(uint8_t channel, uint32_t *count) {
    return static_cast<Derived *>(this)->ReadChannelCount(channel, count);
  }

  /**
   * @brief Read voltage value from specified channel
   * @param channel ADC channel to read from
   * @param voltage_v Pointer to store the voltage value in volts
   * @return AdcError::SUCCESS on success, error code otherwise
   */
  AdcError ReadChannelV(uint8_t channel, float *voltage_v) {
    return static_cast<Derived *>(this)->ReadChannelV(channel, voltage_v);
  }

  /**
   * @brief Get the reference voltage of the ADC
   * @return Reference voltage in volts
   */
  [[nodiscard]] float GetReferenceVoltage() const {
    return static_cast<const Derived *>(this)->GetReferenceVoltage();
  }

  /**
   * @brief Get the ADC resolution in bits
   * @return ADC resolution (e.g., 12 for 12-bit ADC)
   */
  [[nodiscard]] uint8_t GetResolutionBits() const {
    return static_cast<const Derived *>(this)->GetResolutionBits();
  }

  // Prevent copying
  AdcInterface(const AdcInterface &) = delete;
  AdcInterface &operator=(const AdcInterface &) = delete;

  // Allow moving
  AdcInterface(AdcInterface &&) = default;
  AdcInterface &operator=(AdcInterface &&) = default;

protected:
  /**
   * @brief Protected constructor to prevent direct instantiation
   */
  AdcInterface() = default;

  /**
   * @brief Protected destructor
   * @note Derived classes can have public destructors
   */
  ~AdcInterface() = default;
};

} // namespace ntc
