#ifndef __RM_TASK_H_
#define __RM_TASK_H_

#include <linux/list.h>

struct sched_rm_entity{
    struct rb_node node;
    u64 period;
};

#endif