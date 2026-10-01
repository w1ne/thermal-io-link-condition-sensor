#ifndef SENSOR_I2C_H
#define SENSOR_I2C_H
#include <stdint.h>
int sensor_i2c_initialize(void);
void sensor_i2c_deadline(int64_t deadline_us);
#endif
