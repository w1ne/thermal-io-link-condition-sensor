# ESP32-C3 + MLX90640 + STEVAL-IOD003V1

This reference uses **ESP32-C3-DevKitM-1**, not the unspecified SuperMini in
the old export. Other ESP32 families/boards require a reviewed pin map and build.

## MLX90640

Use a 3.3 V MLX90640 breakout, address 0x33, with 3.3 V I2C pull-ups:

| ESP32-C3 | MLX90640 |
|---|---|
| GPIO0 | SDA |
| GPIO1 | SCL |
| 3V3 | VCC (3.3 V breakout) |
| GND | GND |

The firmware uses I2C0 at 400 kHz. GPIO2/8/9 boot straps are avoided. Check
breakout pull-ups; do not pull SDA/SCL to 5 V. Do not connect the sensor to
STEVAL's CN11 without reviewing its routing and power.

## STEVAL-IOD003V1 / L6362A

| ESP32-C3 signal | STEVAL header/net |
|---|---|
| GPIO4 UART1 TX | CN5 pin 3 / IN2 |
| GPIO5 UART1 RX | CN9 pin 3 / OUT I/Q |
| GPIO6 EN | CN5 pin 2 / EN through existing R10 |
| GPIO7 falling-edge OL interrupt | CN5 pin 5 / OL |
| GPIO10 DIAG | CN8 pin 3 / DIAG |
| GND | CN6 pin 6 or 7 / common L− |

Master connects at CN1: pin 1=L+, pin 3=L−, pin 4=C/Q; pins 2/5 unused.
**C/Q connects only to the shield PHY, never directly to an ESP32 GPIO.**

With both boards unpowered, configure the shield:

| Item | Configuration |
|---|---|
| SW1 | Close 1–4, 2–4, 3–4 (OUTH, receiver, OUTL to C/Q) |
| SW2 | Close 2–3, open 1–2 (3.3 V logic) |
| SW3 | Close 2–3, open 1–2 (MCU EN control) |
| JP1 | Closed |
| JP2 | Open |
| JP4 / JP5 | Open (disconnect L78L12 input and VIN output) |
| JP3 | Closed only after JP4/JP5 are open |
| JP6 | Open |
| R102 | Remove stock 100 ohm IN1-to-3V3 link; wire IN1 to shield GND |

IN1 low makes UART TX/RX noninverting for this driver. Retain the shield's
OL pull-up and EN/DIAG series resistor R10. DIAG is sensed on the IC side,
not the ESP32 output side.

Power the ESP32 via USB and the MLX90640 from the ESP32's 3V3 rail. Power
the STEVAL via the master's L+/L−. Join grounds; **do not join separately
powered 3V3/5V/VIN rails**. The L6362A regulator is not an ESP32 supply.
Before logic connections, verify IN1=0 V and shield logic VDD near 3.3 V.
GPIO4–7 share external JTAG functions; disconnect external JTAG for this wiring.

The SVG is a connection diagram, not a fabricated PCB design or validated
KiCad schematic. The original invalid generated KiCad file has been removed.

Primary references:

- [ST STEVAL schematic](https://www.st.com/resource/en/schematic_pack/steval-iod003v1_schematic.pdf)
- [ST UM2424 jumper and IN1 instructions](https://community.st.com/ysqtg83639/attachments/ysqtg83639/interface-connectivity-ics-forum/10620/2/en.DM00510917.pdf)
- [L6362A datasheet](https://www.st.com/resource/en/datasheet/l6362a.pdf)
- [ESP32-C3-DevKitM-1 pins](https://documentation.espressif.com/esp-dev-kits/en/latest/esp32c3/esp32-c3-devkitm-1/index.html)
