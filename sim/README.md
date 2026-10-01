# Actual-firmware Twin acceptance

The old test executed an opaque exported ELF and checked six console strings.
That was not evidence that the published source compiled or that the STEVAL
analog shield transported thermal process data. It has been removed.

The authoritative image is built from `firmware/` source. A complete acceptance
run must boot its ROM/second-stage bootloader/partition/application images,
attach the MLX90640 to I2C0, and attach the GPIO4/5/6/7/10 STEVAL L6362A model
with analog C/Q to an IO-Link master. It must establish:

1. Camera I2C acquisition and two-subpage calibrated frames, not canned logs.
2. Normal, warm warning, overheating, cooling-rate and sensor-failure scenes
   change the shared classifier's observable state using real elapsed clocks.
3. Actual master-received ten-byte PD matches the firmware values/checksum
   through the electrical line, with correct PD validity and diagnostic event.
4. Camera computation does not starve protocol service or violate deadlines.
5. C/Q capacitance, receiver thresholds, contention/shorts, supply loss and
   recovery produce the expected real-firmware response.

The LabWired engine integration is being completed separately. There is no
current passing full analog thermal acceptance claim or executable replacement
that quietly falls back to ideal UART bytes. Source-build CI is intentionally
labelled separately from full Twin validation.
