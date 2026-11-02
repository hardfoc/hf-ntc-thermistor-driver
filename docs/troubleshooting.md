# Troubleshooting

This guide helps you diagnose and resolve common issues when using the NTC Thermistor driver.

## Common Error Messages

### Error: Not Initialized

**Symptoms:**
- `Initialize()` returns `false`
- Methods return `NtcError::NotInitialized`

**Solutions:**
1. **Call Initialize() first**:
   ```cpp
   if (!thermistor.Initialize()) {
       // Check ADC interface and configuration
   }
   ```

2. **Verify ADC interface**:
   - Ensure ADC interface is properly implemented
   - Check that `EnsureInitialized()` succeeds

3. **Check configuration**:
   - Verify all configuration parameters are valid
   - Check ADC channel is available

---

### Error: ADC Read Failed

**Symptoms:**
- `ReadTemperatureCelsius()` returns `NtcError::AdcReadFailed`
- Temperature readings fail

**Solutions:**
1. **Verify ADC interface implementation**:
   - Check `ReadChannelV()` implementation
   - Verify ADC is properly initialized

2. **Check hardware connections**:
   - Verify voltage divider circuit is wired correctly
   - Check ADC channel matches physical connection
   - Ensure power supply is stable

3. **Verify ADC configuration**:
   - Check reference voltage matches hardware
   - Verify ADC resolution settings

---

### Error: Temperature Out of Range

**Symptoms:**
- `ReadTemperatureCelsius()` returns `NtcError::TemperatureOutOfRange`
- Temperature values are invalid

**Solutions:**
1. **Check temperature limits**:
   ```cpp
   ntc_config_t config;
   thermistor.GetConfiguration(&config);
   // Adjust min_temperature and max_temperature if needed
   ```

2. **Verify circuit configuration**:
   - Check series resistance matches configuration
   - Verify NTC resistance at 25°C is correct
   - Ensure reference voltage is accurate

3. **Check for hardware issues**:
   - Verify NTC thermistor is functioning
   - Check for loose connections
   - Test with multimeter

---

## Hardware Issues

### Incorrect Temperature Readings

**Symptoms:**
- Temperature readings are consistently wrong
- Readings don't change with temperature

**Checklist:**
- [ ] Verify voltage divider circuit is wired correctly
- [ ] Check series resistor value matches configuration
- [ ] Verify NTC resistance at 25°C matches configuration
- [ ] Check reference voltage matches hardware (3.3V or 5V)
- [ ] Ensure ADC channel matches physical connection
- [ ] Verify NTC thermistor is functioning (test with multimeter)

**Solutions:**
1. **Calibrate the sensor**:
   ```cpp
   // Calibrate at known temperature
   thermistor.Calibrate(25.0f);
   ```

2. **Check configuration values**:
   ```cpp
   ntc_config_t config;
   thermistor.GetConfiguration(&config);
   // Verify all values match hardware
   ```

---

### Noisy Readings

**Symptoms:**
- Temperature readings fluctuate
- Unstable values

**Solutions:**
1. **Enable filtering**:
   ```cpp
   thermistor.SetFiltering(true, 0.1f); // Enable with alpha=0.1
   ```

2. **Use multi-sample averaging**:
   ```cpp
   thermistor.SetSamplingParameters(5, 10); // 5 samples, 10ms delay
   ```

3. **Check hardware**:
   - Verify power supply is stable
   - Check for noise sources near ADC input
   - Use proper grounding

---

## Software Issues

### Compilation Errors

**Error: "No matching function"**

**Solution:**
- Ensure you've implemented all required `AdcInterface` methods
- Check method signatures match exactly

**Error: "Undefined reference"**

**Solution:**
- Verify you're including/linking the driver source
- Check include paths are correct

---

### Runtime Errors

**Initialization Fails**

**Checklist:**
- [ ] ADC interface is properly implemented
- [ ] ADC channel is available
- [ ] Configuration parameters are valid
- [ ] ADC is initialized

**Invalid Readings**

**Checklist:**
- [ ] Driver is initialized
- [ ] ADC interface is working
- [ ] Configuration matches hardware
- [ ] Error codes are checked

---

## Debugging Tips

### Enable Debug Output

Add debug prints to your ADC interface:

```cpp
ntc::AdcError ReadChannelV(uint8_t channel, float* voltage_v) {
    printf("Reading ADC channel %u\n", channel);
    // ... your implementation
    printf("Voltage: %.3fV\n", *voltage_v);
    return ntc::AdcError::Success;
}
```

### Check Configuration

```cpp
ntc_config_t config;
thermistor.GetConfiguration(&config);
printf("Series resistance: %.1fΩ\n", config.series_resistance);
printf("Reference voltage: %.2fV\n", config.reference_voltage);
printf("ADC channel: %u\n", config.adc_channel);
```

### Verify ADC Readings

```cpp
float voltage;
if (adc.ReadChannelV(0, &voltage) == ntc::AdcError::Success) {
    printf("ADC voltage: %.3fV\n", voltage);
    // Should be between 0V and reference voltage
}
```

---

## FAQ

### Q: Why are my temperature readings incorrect?

**A:** Common causes:
1. Series resistance doesn't match configuration
2. Reference voltage is incorrect
3. NTC resistance at 25°C is wrong
4. ADC channel mismatch
5. Need calibration

### Q: How do I calibrate the sensor?

**A:** Use the `Calibrate()` method with a known reference temperature:
```cpp
thermistor.Calibrate(25.0f); // Calibrate at 25°C
```

### Q: What's the difference between lookup table and mathematical conversion?

**A:**
- **Lookup Table**: Faster, slightly less accurate, uses pre-calculated values
- **Mathematical**: Slower, more accurate, uses Steinhart-Hart or Beta equations

### Q: How do I reduce noise in readings?

**A:**
1. Enable filtering: `thermistor.SetFiltering(true, 0.1f)`
2. Use multi-sample averaging: `thermistor.SetSamplingParameters(5, 10)`
3. Check hardware for noise sources

---

## Getting More Help

If you're still experiencing issues:

1. Check the [API Reference](api_reference.md) for method details
2. Review [Examples](examples.md) for working code
3. Search existing issues on GitHub
4. Open a new issue with:
   - Description of the problem
   - Steps to reproduce
   - Hardware setup details
   - Error messages/logs
   - Configuration values

---

**Navigation**
⬅️ [Examples](examples.md) | [Back to Index](index.md)

