/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "thermal_device.h"
#include "fingerprint.h"
#include "iolinki/crc.h"
#include "iolinki/protocol.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint64_t clock_us=1000;
static uint8_t rx[64],tx[64]; static size_t rn,ri,tn; static int wake;
uint64_t iolink_time_get_us(void) { return clock_us; }
uint32_t iolink_time_get_ms(void) { return clock_us/1000; }
void iolink_critical_enter(void) {}
void iolink_critical_exit(void) {}
int iolink_nvm_read(uint32_t a,uint8_t *b,size_t n) { (void)a;(void)b;(void)n;return -1; }
int iolink_nvm_write(uint32_t a,const uint8_t *b,size_t n) { (void)a;(void)b;(void)n;return -1; }
static int init(void *p) { (void)p;return 0; }
static void mode(void *p,iolink_phy_mode_t m) { (void)p;(void)m; }
static void baud(void *p,iolink_baudrate_t b) { (void)p;(void)b; }
static int recv(void *p,uint8_t *b) { (void)p;if(ri==rn)return 0;*b=rx[ri++];return 1; }
static int send(void *p,const uint8_t *b,size_t n) { (void)p;assert(n<=sizeof(tx));memcpy(tx,b,n);tn=n;return n; }
static int waking(void *p) { (void)p;int v=wake;wake=0;return v; }
static void exchange(iolink_device_ctx_t *d,uint8_t mc,uint8_t type,const uint8_t *od,size_t n) {
 rx[0]=mc;rx[1]=type;if(n)memcpy(rx+2,od,n);rn=n+2;ri=tn=0;
 rx[1]|=iolink_checksum6(rx,rn);clock_us+=7000;iolink_device_process(d);
 assert(ri==rn && tn>0);
 uint8_t saved=tx[tn-1]&0x3f;tx[tn-1]&=0xc0;
 assert(iolink_checksum6(tx,tn)==saved);tx[tn-1]|=saved;
}
int main(void) {
 iolink_device_ctx_t d;
 const iolink_phy_api_t phy={.init=init,.set_mode=mode,.set_baudrate=baud,.send=send,.recv_byte=recv,.detect_wakeup=waking};
 assert(thermal_device_init(&d,&phy)==0);
 assert(d.dll.enforce_timing && d.dll.min_cycle_time_us==6000);
 wake=1;iolink_device_process(&d);clock_us+=200;
 exchange(&d,0xa2,IOLINK_MSEQ_TYPE_0,NULL,0);assert(tn==2 && tx[0]==60);
 exchange(&d,0xa3,IOLINK_MSEQ_TYPE_0,NULL,0);assert(tn==2 && tx[0]==0x0b);
 exchange(&d,0xa5,IOLINK_MSEQ_TYPE_0,NULL,0);assert(tn==2 && tx[0]==0x89);
 exchange(&d,0xa6,IOLINK_MSEQ_TYPE_0,NULL,0);assert(tn==2 && tx[0]==0);
 uint8_t command=IOLINK_CMD_DEVICE_OPERATE;
 exchange(&d,0x20,IOLINK_MSEQ_TYPE_0,&command,1);assert(tn==1);
 tfs_verdict_t v={.valid=true,.hotspot_c=59,.rate_c_s=0,.state=TFS_WARN,.health=92,.time_to_limit_s=0xffff,.event_flags=TFS_EV_WARN};
 uint8_t pd[TFS_PD_BYTES];tfs_pack_pd(&v,pd);assert(iolink_device_pd_input_update(&d,pd,sizeof(pd),true)==0);
 const uint8_t idle[2]={0,0};exchange(&d,0x80,IOLINK_MSEQ_TYPE_2,idle,2);
 assert(iolink_device_get_state(&d)==IOLINK_DLL_STATE_OPERATE);
 assert(tn==13 && memcmp(tx,pd,sizeof(pd))==0 && !(tx[12]&0x40));
 assert(iolink_device_pd_input_update(&d,pd,sizeof(pd),false)==0);
 exchange(&d,0x80,IOLINK_MSEQ_TYPE_2,idle,2);assert(tn==13 && (tx[12]&0x40));
 assert(d.dll.t_cycle_violations==0 && d.dll.t_byte_violations==0);
 puts("PASS: real stack wake/startup/DPP MinCycleTime/capability/PD lengths, DeviceOperate, Type2.V 10-byte response checksum and invalid-status");
}
