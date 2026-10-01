set(ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
set(IOL "${ROOT}/third_party/iolinki")
set(MLX "${ROOT}/third_party/mlx90640-library")
idf_component_register(SRCS "main.c" "sensor_i2c.c" "fingerprint.c" "thermal_device.c" "publisher.c" "board.c" "tx_guard.c" "rx_guard.c"
    "${MLX}/functions/MLX90640_API.c"
    "${IOL}/src/phy_l6362a.c" "${IOL}/src/crc.c" "${IOL}/src/frame.c"
    "${IOL}/src/dll.c" "${IOL}/src/isdu.c" "${IOL}/src/events.c"
    "${IOL}/src/params.c" "${IOL}/src/data_storage.c"
    "${IOL}/src/device_info.c" "${IOL}/src/device.c"
    INCLUDE_DIRS "." "${IOL}/include" "${MLX}/headers"
    REQUIRES driver esp_timer)
