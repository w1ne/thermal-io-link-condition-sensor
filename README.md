# Thermal IO-Link Condition Sensor

An ESP32-C3 acquires calibrated 32×24 MLX90640 thermal frames and classifies
STABLE, WARN and FAULT conditions. It publishes the verdict through **STEVAL-IOD003V1 / L6362A** using the iolinki device stack.

The original export contained an empty `main.ino` and an opaque bare-metal ELF.
This repair supplies buildable **ESP-IDF 5.4.0 C source**, not an Arduino sketch.
It retains and repairs the original thermal classifier; it is not a counter demo.

## Build and flash

Requires [PlatformIO Core 6.1.19](https://docs.platformio.org/en/latest/core/installation/index.html).
From the repository root:

```sh
platformio run -d firmware
platformio run -d firmware -t upload --upload-port /dev/ttyUSB0
platformio device monitor --port /dev/ttyUSB0 --baud 115200
```

Target: **ESP32-C3-DevKitM-1**. Platform/framework are pinned in
[platformio.ini](firmware/platformio.ini). Build outputs are in
`firmware/.pio/build/esp32c3/`: firmware.elf, firmware.bin, bootloader.bin,
and partitions.bin. No prebuilt ELF is treated as authoritative.

Read the complete [wiring and power guide](hardware/wiring.md) before connecting
the boards, including the STEVAL **IN1/R102 modification**. See the
[BOM](hardware/bom.csv) and [connection diagram](hardware/wiring.svg).

## Application behavior

The official Melexis decoder reads EEPROM calibration and both chess subpages
through I2C0 at 400 kHz. It continuously processes complete frames, using the
monotonic MCU clock. Faults latch until reset. Demonstrator thresholds:

| Condition | Criterion |
|---|---|
| STABLE | Settled observation ≥12 s, heating rate <0.30 °C/s, hotspot <58 °C |
| WARN | Same settled behavior, hotspot ≥58 °C and <70 °C |
| OVERTEMP | Hotspot ≥70 °C |
| COOLING_FAILURE | Heating rate ≥0.30 °C/s after the 12 s observation grace |
| HOTSPOT_EMERGENCE | Local hotspot rises ≥8 °C between complete frames |
| SENSOR fault | Failed/stalled acquisition, invalid field or timestamp |

IDLE/WARMUP precede settled classification. These application thresholds require
calibration for the real machine; they are not safety-rated diagnoses. Sensor
failures publish invalid PD. A sample older than three seconds is invalidated.
Only the first fault reason is retained; cooling afterward does not rename it.

The higher-priority IO-Link task is notified every 50 µs, allowing the
lower-priority camera task to perform I2C and soft-float calculations. UART1
uses COM2, 8E1 and actual transceiver EN/OL/DIAG. Timing enforcement remains
active. Response latency under camera load still needs emulator and physical
measurement; source compilation does not prove it.

## Process data

The repaired format is **10 bytes**, replacing the old ambiguous nine-byte
format which discarded the fifth event flag. Configure the master for ten
input bytes and zero output bytes; Type 2.V (two-byte OD), minimum cycle 6 ms, COM2.

| Bytes | Encoding |
|---|---|
| 0–1 | Signed hotspot °C ×100, big endian |
| 2–3 | Signed heating rate °C/s ×100, big endian |
| 4 | State: IDLE=0, WARMUP=1, STABLE=2, FAULT=3, WARN=4 |
| 5 | Health 0–100 |
| 6–7 | Seconds to critical temperature; 65535 means unavailable |
| 8 | Fault: NONE=0, OVERTEMP=1, COOLING_FAILURE=2, EMERGENCE=3, SENSOR=4 |
| 9 | Flags: warn bit0, critical bit1, rising rate bit2, emergence bit3, fault latch bit4 |

An application fault raises vendor-specific event 0x8CA0. The master must use
PD validity as well as the fault byte. Product identity and IODD need their own
integration; this example has no certified IODD or IO-Link conformity report.
Embedded nonvolatile parameter storage remains explicitly unsupported.

## Verification status

```sh
tests/run-host.sh
```

CI builds the actual firmware and tests the shared classifier/encoder against
normal, warning, overtemperature, cooling, emergence, latched-cause, invalid
frame/time, signed encoding and flag-retention cases. Its badge covers those
checks only. The old acceptance test checking fixed boot strings has been
removed: it could not establish thermal sensing or master-received PD.

**Full LabWired actual-firmware acquisition, analog C/Q, master-received PD and
physical-master verification remain pending.** See [sim/README.md](sim/README.md).
Do not interpret the source build as complete emulator/hardware validation.

[Repair design](docs/repair-plan.md) and [source/license provenance](docs/provenance.md).
