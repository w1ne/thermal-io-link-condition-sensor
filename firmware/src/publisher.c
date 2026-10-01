/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "publisher.h"
#include <string.h>
static void latch(tfs_publisher_t *p,tfs_fault_t fault) {
 if(p->first_fault==TFS_FAULT_NONE) p->first_fault=fault;
 if(p->first_fault!=TFS_FAULT_NONE) {
  p->verdict.state=TFS_FAULT;p->verdict.fault=p->first_fault;
  p->verdict.event_flags|=TFS_EV_FAULT_LATCH;
 }
}
void tfs_publisher_init(tfs_publisher_t *p) { memset(p,0,sizeof(*p)); }
void tfs_publisher_sample(tfs_publisher_t *p,const tfs_verdict_t *v,int64_t at_us) {
 p->verdict=*v;p->at_us=at_us;p->valid=v->valid;
 latch(p,v->fault);
}
bool tfs_publisher_expire(tfs_publisher_t *p,int64_t now_us) {
 if(!p->valid || now_us-p->at_us<=TFS_SAMPLE_TIMEOUT_US) return false;
 p->valid=false;p->verdict.valid=false;p->verdict.health=0;
 p->verdict.time_to_limit_s=0xffff;
 latch(p,TFS_FAULT_SENSOR);return true;
}
