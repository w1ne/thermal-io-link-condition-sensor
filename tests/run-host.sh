#!/bin/sh
set -eu
THERMAL_TEST_DIR=$(mktemp -d)
trap 'rm -rf "$THERMAL_TEST_DIR"' EXIT HUP INT TERM
cc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined,float-cast-overflow -fno-sanitize-recover=all -Ifirmware/src \
 tests/test_fingerprint.c firmware/src/fingerprint.c -lm -o "$THERMAL_TEST_DIR/fingerprint"
"$THERMAL_TEST_DIR/fingerprint"
cc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined,float-cast-overflow -fno-sanitize-recover=all -Ifirmware/src -Ithird_party/iolinki/include \
 tests/test_device_wire.c firmware/src/thermal_device.c firmware/src/fingerprint.c \
 third_party/iolinki/src/device.c third_party/iolinki/src/crc.c third_party/iolinki/src/frame.c \
 third_party/iolinki/src/dll.c third_party/iolinki/src/isdu.c third_party/iolinki/src/events.c \
 third_party/iolinki/src/params.c third_party/iolinki/src/data_storage.c third_party/iolinki/src/device_info.c \
 -lm -o "$THERMAL_TEST_DIR/device-wire"
"$THERMAL_TEST_DIR/device-wire"
cc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined,float-cast-overflow -fno-sanitize-recover=all -Ifirmware/src \
 tests/test_publisher.c firmware/src/publisher.c -o "$THERMAL_TEST_DIR/publisher"
"$THERMAL_TEST_DIR/publisher"
