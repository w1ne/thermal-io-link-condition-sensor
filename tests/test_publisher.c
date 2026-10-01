#include "publisher.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
 tfs_publisher_t p; tfs_verdict_t healthy={.valid=true,.state=TFS_STABLE,.health=100};
 tfs_publisher_init(&p); tfs_publisher_sample(&p,&healthy,100);
 assert(p.valid); assert(tfs_publisher_expire(&p,3000100)==false);
 assert(tfs_publisher_expire(&p,3000101)); assert(!p.valid && p.verdict.fault==TFS_FAULT_SENSOR);
 tfs_publisher_sample(&p,&healthy,4000000);
 assert(p.verdict.state==TFS_FAULT && p.verdict.fault==TFS_FAULT_SENSOR && (p.verdict.event_flags&TFS_EV_FAULT_LATCH));
 tfs_publisher_init(&p);tfs_verdict_t hot={.valid=true,.state=TFS_FAULT,.fault=TFS_FAULT_OVERTEMP};
 tfs_publisher_sample(&p,&hot,100);assert(tfs_publisher_expire(&p,3000101));
 assert(!p.valid && p.verdict.fault==TFS_FAULT_OVERTEMP);
 tfs_publisher_sample(&p,&healthy,4000000);assert(p.verdict.fault==TFS_FAULT_OVERTEMP && p.verdict.state==TFS_FAULT);
 puts("PASS: expiry boundary, invalid stale PD, healthy-timeout-healthy latch and first overtemperature cause retention");
}
