# Source provenance and licenses

- Original checked-in ELF debug records identify LabWired's
  `examples/esp32c3-mlx90640-thermal/firmware` sources. The restored
  `fingerprint.c/.h` come from w1ne/labwired-core commit
  94b6f47a3bff08cb430af4ea0d505b593f1992d5, MIT, copyright 2026 Andrii Shylenko.
  Local repairs add validated input/time, explicit WARN, first-fault retention,
  and ten-byte PD with separate fault and complete flags.
- `third_party/mlx90640-library` contains the official Apache-2.0 Melexis API
  and headers copied unchanged from the same LabWired tree. The original
  upstream snapshot did not record an upstream commit; file SHA-256s in
  `third_party/mlx90640-library/SHA256SUMS` identify these exact bytes.
- `third_party/iolinki` is an unmodified source/header subset from
  w1ne/iolinki commit 5572a628b8befa6a9c74496311da2359868f96d9. Its upstream
  GPL/commercial license files are retained.
- `board.c/.h`, `tx_guard.*`, `rx_guard.*` originate in that iolinki commit's
  ESP32/L6362A example. Board changes remove unused button/LED GPIO0/1
  initialization, reserving those pins for the camera. Adapter code retains
  upstream GPL-3.0-or-later notices.
- Root MIT license applies to original MIT repository material and restored
  classifier. New integration source carries GPL-3.0-or-later notices; the
  public linked firmware includes GPL iolinki. MIT does not supersede the
  included dependency licenses.
