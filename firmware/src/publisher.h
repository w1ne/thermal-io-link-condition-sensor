#ifndef THERMAL_PUBLISHER_H
#define THERMAL_PUBLISHER_H
#include "fingerprint.h"
#define TFS_SAMPLE_TIMEOUT_US 3000000LL
typedef struct {
 tfs_verdict_t verdict;
 int64_t at_us;
 tfs_fault_t first_fault;
 bool valid;
} tfs_publisher_t;
void tfs_publisher_init(tfs_publisher_t *p);
void tfs_publisher_sample(tfs_publisher_t *p,const tfs_verdict_t *v,int64_t at_us);
bool tfs_publisher_expire(tfs_publisher_t *p,int64_t now_us);
#endif
