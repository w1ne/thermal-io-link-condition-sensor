# Thermal reference repair

The exported main.ino was empty. Its ELF DWARF identifies the original LabWired examples/esp32c3-mlx90640-thermal/firmware C sources. This repair preserves that thermal classifier, with tests correcting latched diagnostic reasons and discarded process-data flag bits.

The hardware firmware is now ESP-IDF 5.4.0, not Arduino: it continuously acquires official MLX90640 calibrated chess subpages on I2C, classifies complete two-subpage frames with monotonic clock timestamps, and publishes ten-byte process data through pinned iolinki and the L6362A adapter. A priority-6 IO-Link task blocks between 50us timer notifications; priority-4 sensor work cannot starve the stack. Sensor transactions have bounded timeouts and failures invalidate process data rather than reporting stale temperatures as valid. Timing enforcement remains enabled.

Classification retains the original application thresholds: warn 58 C, critical 70 C, cooling rise >=0.30 C/s after a 12s observation grace, and localized emergence >=8 C between complete frames. These are demonstrator thresholds requiring product calibration, not a safety-rated diagnosis. Fault reason latches until reset; no silently changing emergence into cooling failure.

Host regressions exercise actual classifier/encoding functions; CI rebuilds firmware source. Emulator evidence must identify input scenes and received wire PD, not fixed boot strings. Full analog C/Q and physical-master validation are separately pending engine/hardware evidence.
