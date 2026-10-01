/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "thermal_device.h"
#include "fingerprint.h"
#include <string.h>
static iolink_device_config_t config; /* this reference intentionally owns one device */
static const iolink_device_info_t identity={
 .vendor_name="iolinki example",.vendor_text="Experimental identity; replace before shipping",
 .product_name="Thermal condition sensor",.product_id="thermal-mlx90640",
 .product_text="Ten input bytes, no output bytes",.serial_number="EXAMPLE-THERMAL-001",
 .hardware_revision="reference",.firmware_revision="2",
 .vendor_id=0xffff,.device_id=5679,.min_cycle_time=60,.revision_id=0x11
};
int thermal_device_init(iolink_device_ctx_t *device,const iolink_phy_api_t *phy) {
 if(!device || !phy) return -1;
 memset(&config,0,sizeof(config));
 config.phy=*phy; config.device_info=&identity;
 config.stack.m_seq_type=IOLINK_M_SEQ_TYPE_2_V;
 config.stack.min_cycle_time=identity.min_cycle_time;
 config.stack.pd_in_len=TFS_PD_BYTES;
 /* Pinned stack's DPP/ISDU path still reads legacy identity. Align it explicitly
  * for this one-device reference without modifying vendored stack source. */
 iolink_device_info_init(&identity);
 int result=iolink_device_init(device,&config);
 if(result==0) iolink_device_set_timing_enforcement(device,true);
 return result;
}
