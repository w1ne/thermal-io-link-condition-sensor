/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "fingerprint.h"
#include "thermal_device.h"
#include "publisher.h"
#include "sensor_i2c.h"
#include "MLX90640_API.h"
#include "board.h"
#include "iolinki/device.h"
#include "iolinki/events.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define MLX_ADDR 0x33
#define SENSOR_TIMEOUT_US 3000000LL
#define PROTOCOL_PERIOD_US 50
static QueueHandle_t samples;
static TaskHandle_t protocol_task;
static iolink_device_ctx_t device;
static iolink_l6362a_ctx_t phy;
static paramsMLX90640 params;
static uint16_t eeprom[832], frame[834];
static float temperatures[TFS_PIXELS];
static tfs_ctx_t classifier;
typedef struct { tfs_verdict_t verdict; int64_t at_us; bool valid; } sample_t;
/* Observable debugger symbols contain genuine acquired application state. */
volatile uint32_t thermal_frame_count, thermal_error_count, protocol_service_count;
volatile uint8_t thermal_pd[TFS_PD_BYTES];
volatile bool thermal_pd_valid;
volatile int thermal_last_error;

static void publish_sample(tfs_verdict_t *v, bool valid) {
    sample_t s={.verdict=*v,.at_us=esp_timer_get_time(),.valid=valid};
    xQueueOverwrite(samples,&s);
}
static void sensor_failure(int error) {
    thermal_last_error=error; thermal_error_count++;
    tfs_verdict_t v; tfs_sensor_fault(&classifier,&v);
    publish_sample(&v,false);
    printf("TFS SENSOR ERROR=%d\n",error);
}
static int sensor_configure(void) {
    sensor_i2c_deadline(esp_timer_get_time()+SENSOR_TIMEOUT_US);
    int e=MLX90640_DumpEE(MLX_ADDR,eeprom); if(e) return e;
    e=MLX90640_ExtractParameters(eeprom,&params); if(e) return e;
    e=MLX90640_SetRefreshRate(MLX_ADDR,2); if(e) return e; /* 2Hz subpages */
    return MLX90640_SetChessMode(MLX_ADDR);
}
static void sensor_worker(void *arg) {
    (void)arg; tfs_init(&classifier);
    if(sensor_i2c_initialize()!=0) { sensor_failure(-100); vTaskDelete(NULL); return; }
    bool configured=false;
    for(;;) {
        if(!configured) {
            int e=sensor_configure();
            if(e) { sensor_failure(e); vTaskDelay(pdMS_TO_TICKS(1000)); continue; }
            configured=true; printf("MLX90640 READY\n");
        }
        unsigned seen=0;
        int64_t start=esp_timer_get_time();
        sensor_i2c_deadline(start+SENSOR_TIMEOUT_US);
        for(unsigned reads=0;seen!=3 && reads<8;reads++) {
            int sub=MLX90640_GetFrameData(MLX_ADDR,frame);
            if(sub<0) { sensor_failure(sub); configured=false; break; }
            float ambient=MLX90640_GetTa(frame,&params);
            if(!isfinite(ambient)) { sensor_failure(-101); configured=false; break; }
            MLX90640_CalculateTo(frame,&params,0.95f,ambient-8.0f,temperatures);
            MLX90640_BadPixelsCorrection(params.brokenPixels,temperatures,1,&params);
            MLX90640_BadPixelsCorrection(params.outlierPixels,temperatures,1,&params);
            seen|=1u<<(sub&1);
        }
        if(!configured) { vTaskDelay(pdMS_TO_TICKS(100)); continue; }
        if(seen!=3 || esp_timer_get_time()-start>SENSOR_TIMEOUT_US) {
            sensor_failure(-102); configured=false; continue;
        }
        tfs_verdict_t v;
        tfs_update(&classifier,temperatures,esp_timer_get_time()/1000000.0,&v);
        thermal_frame_count++;
        publish_sample(&v,v.valid);
        printf("TFS FRAME=%lu state=%s hot=%.2f rate=%.2f fault=%s\n",
            (unsigned long)thermal_frame_count,tfs_state_name(v.state),
            (double)v.hotspot_c,(double)v.rate_c_s,tfs_fault_name(v.fault));
    }
}
static void protocol_tick(void *arg) {
    (void)arg;
    xTaskNotifyGive(protocol_task);
}
static void protocol_worker(void *arg) {
    (void)arg;
    if(iolink_phy_l6362a_init(&phy,board_io())!=0) goto failure;
    if(thermal_device_init(&device,iolink_phy_l6362a_get(&phy))!=0) goto failure;
    uint8_t pd[TFS_PD_BYTES]={0};
    iolink_device_pd_input_update(&device,pd,sizeof(pd),false);
    esp_timer_handle_t timer;
    const esp_timer_create_args_t t={.callback=protocol_tick,.name="iolink-service",.skip_unhandled_events=true};
    if(esp_timer_create(&t,&timer)!=ESP_OK || esp_timer_start_periodic(timer,PROTOCOL_PERIOD_US)!=ESP_OK) goto failure;
    printf("IOLINK INIT OK (timing enabled)\n");
    tfs_publisher_t publisher; tfs_publisher_init(&publisher);
    bool event_raised=false;
    for(;;) {
        bool changed=false;
        sample_t s;
        if(xQueueReceive(samples,&s,0)==pdTRUE) {
            s.verdict.valid=s.valid;
            tfs_publisher_sample(&publisher,&s.verdict,s.at_us);
            changed=true;
        }
        changed |= tfs_publisher_expire(&publisher,esp_timer_get_time());
        if(changed) {
            tfs_pack_pd(&publisher.verdict,pd);
            iolink_device_pd_input_update(&device,pd,sizeof(pd),publisher.valid);
            memcpy((void*)thermal_pd,pd,sizeof(pd)); thermal_pd_valid=publisher.valid;
            if(publisher.first_fault!=TFS_FAULT_NONE && !event_raised) {
                iolink_event_trigger(iolink_device_get_events_ctx(&device),0x8CA0,IOLINK_EVENT_TYPE_ERROR);
                event_raised=true;
            }
        }
        /* Apply queued samples and expiry BEFORE handling any master request. */
        iolink_device_process(&device);
        protocol_service_count++;
        ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
    }

failure:
    printf("IOLINK INIT FAIL\n");
    vTaskDelete(NULL);
}
void app_main(void) {
    printf("TFS BOOT ESP32-C3 MLX90640 STEVAL-IOD003V1\n");
    samples=xQueueCreate(1,sizeof(sample_t));
    if(!samples) { printf("TFS QUEUE FAIL\n"); return; }
    if(xTaskCreate(protocol_worker,"iolink",4096,NULL,6,&protocol_task)!=pdPASS) return;
    if(xTaskCreate(sensor_worker,"thermal",8192,NULL,4,NULL)!=pdPASS) {
        printf("TFS SENSOR TASK FAIL\n");
    }
}
