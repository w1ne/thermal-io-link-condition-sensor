#include "fingerprint.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
static float field[TFS_PIXELS];
static void scene(float hot) { for(int i=0;i<TFS_PIXELS;i++) field[i]=25; field[12*TFS_COLS+16]=hot; }
static void at(tfs_ctx_t *c,tfs_verdict_t *v,float hot,float t) { scene(hot); tfs_update(c,field,t,v); }
int main(void) {
 tfs_ctx_t c; tfs_verdict_t v; uint8_t pd[TFS_PD_BYTES];
 tfs_init(&c); at(&c,&v,30,0); at(&c,&v,40,1); assert(v.fault==TFS_FAULT_HOTSPOT_EMERGENCE);
 at(&c,&v,30,2); assert(v.fault==TFS_FAULT_HOTSPOT_EMERGENCE);
 assert(v.event_flags&TFS_EV_FAULT_LATCH); tfs_pack_pd(&v,pd);
 assert(pd[8]==TFS_FAULT_HOTSPOT_EMERGENCE && (pd[9]&TFS_EV_FAULT_LATCH));
 tfs_init(&c); at(&c,&v,45,100); at(&c,&v,46,104); at(&c,&v,46,112);
 assert(v.state==TFS_STABLE && v.fault==TFS_FAULT_NONE && v.health==100);
 assert(v.hot_row==12 && v.hot_col==16 && v.ambient_c==25);
 tfs_init(&c); at(&c,&v,58,0); at(&c,&v,58,12);
 assert(v.state==TFS_WARN && (v.event_flags&TFS_EV_WARN));
 tfs_init(&c); at(&c,&v,40,0); at(&c,&v,40,12); at(&c,&v,45,16);
 assert(v.fault==TFS_FAULT_COOLING_FAILURE);
 tfs_init(&c); at(&c,&v,70,0); assert(v.fault==TFS_FAULT_OVERTEMP && v.health==0);
 field[5]=NAN; tfs_update(&c,field,1,&v); assert(!v.valid && v.fault==TFS_FAULT_OVERTEMP);
 at(&c,&v,30,4); assert(v.valid && v.fault==TFS_FAULT_OVERTEMP); /* first cause retained */
 tfs_init(&c); at(&c,&v,30,0); field[5]=NAN; tfs_update(&c,field,1,&v);
 assert(v.fault==TFS_FAULT_SENSOR && v.state==TFS_FAULT);
 tfs_init(&c); at(&c,&v,30,5); at(&c,&v,30,4); assert(v.fault==TFS_FAULT_SENSOR);
 tfs_init(&c); at(&c,&v,30,5); at(&c,&v,31,5); assert(v.fault==TFS_FAULT_SENSOR);
 v=(tfs_verdict_t){.hotspot_c=-1.25f,.rate_c_s=-0.5f,.state=TFS_STABLE,.health=99,.time_to_limit_s=0x1234,.fault=TFS_FAULT_NONE,.event_flags=0x1f};
 tfs_pack_pd(&v,pd); assert(pd[0]==0xff && pd[1]==0x83 && pd[2]==0xff && pd[3]==0xce);
 assert(pd[4]==TFS_STABLE && pd[5]==99 && pd[6]==0x12 && pd[7]==0x34 && pd[8]==0 && pd[9]==0x1f);
 v.hotspot_c=400;v.rate_c_s=-400; tfs_pack_pd(&v,pd);
 assert(pd[0]==0x7f && pd[1]==0xff && pd[2]==0x80 && pd[3]==0);
 tfs_init(&c); scene(30); tfs_update(&c,field,16777216.0,&v);
 scene(30); tfs_update(&c,field,16777217.0,&v); assert(v.fault==TFS_FAULT_NONE);
 tfs_init(&c); for(int i=0;i<TFS_PIXELS;i++) field[i]=-40;
 tfs_update(&c,field,0,&v);field[0]=300;tfs_update(&c,field,0.0000011,&v);
 assert(v.health==0 && v.fault==TFS_FAULT_OVERTEMP);
 puts("PASS: thermal normal, warn, overtemperature, cooling, emergence, latched cause, invalid frame/time and 10-byte process data");
}
