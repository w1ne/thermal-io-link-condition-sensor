# Vendored: Melexis MLX90640 driver (official)

- Upstream: https://github.com/melexis/mlx90640-library (master)
- License: Apache-2.0 (see `LICENSE`)
- Files vendored verbatim (no source edits):
  - `functions/MLX90640_API.c`
  - `headers/MLX90640_API.h`
  - `headers/MLX90640_I2C_Driver.h`

This is the real, unmodified Melexis MLX90640 API. LabWired's MLX90640 device
model (`crates/core/src/peripherals/components/mlx90640.rs`) is validated by
compiling this driver in a host C test and decoding frames the model serves
over the simulated I²C path. The I²C platform shim (`MLX90640_I2CRead/Write`)
is provided by `crates/core/native/mlx90640_bridge.c`, which pokes the LabWired
model exactly as on-target firmware would poke the ESP32-C3 I²C controller.

In this thermal reference the API is compiled unconditionally into the actual
ESP-IDF firmware. Its I2C hooks are supplied by firmware/src/sensor_i2c.c.
The optional Cargo feature described in the upstream LabWired tree applies
only to its host decoder tests, not this firmware build. See SHA256SUMS for
an exact snapshot identity; the copied upstream note did not record a Melexis
commit hash.
