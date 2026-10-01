/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "sensor_i2c.h"
#include "MLX90640_I2C_Driver.h"
#include "driver/i2c.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static int64_t deadline;
static uint8_t bytes[1664]; /* sensor task is the only caller */
void sensor_i2c_deadline(int64_t t) { deadline=t; }
int sensor_i2c_initialize(void) {
    i2c_config_t c={.mode=I2C_MODE_MASTER,.sda_io_num=0,.scl_io_num=1,
        .sda_pullup_en=GPIO_PULLUP_ENABLE,.scl_pullup_en=GPIO_PULLUP_ENABLE,
        .master.clk_speed=400000};
    if(i2c_param_config(I2C_NUM_0,&c)!=ESP_OK) return -1;
    return i2c_driver_install(I2C_NUM_0,c.mode,0,0,0)==ESP_OK?0:-1;
}
void MLX90640_I2CInit(void) {}
void MLX90640_I2CFreqSet(int freq) { (void)freq; }
int MLX90640_I2CGeneralReset(void) { return -1; }
int MLX90640_I2CRead(uint8_t addr,uint16_t reg,uint16_t n,uint16_t *data) {
    if(n>832 || esp_timer_get_time()>=deadline) return -1;
    uint8_t r[2]={reg>>8,reg};
    if(i2c_master_write_read_device(I2C_NUM_0,addr,r,2,bytes,n*2,pdMS_TO_TICKS(100))!=ESP_OK) return -1;
    for(unsigned i=0;i<n;i++) data[i]=((uint16_t)bytes[i*2]<<8)|bytes[i*2+1];
    /* GetFrameData's status polling must yield rather than spin forever. */
    if(reg==MLX90640_STATUS_REG && !(data[0]&MLX90640_STAT_DATA_READY_MASK)) vTaskDelay(1);
    return 0;
}
int MLX90640_I2CWrite(uint8_t addr,uint16_t reg,uint16_t data) {
    if(esp_timer_get_time()>=deadline) return -1;
    uint8_t b[4]={reg>>8,reg,data>>8,data};
    return i2c_master_write_to_device(I2C_NUM_0,addr,b,4,pdMS_TO_TICKS(100))==ESP_OK?0:-1;
}
