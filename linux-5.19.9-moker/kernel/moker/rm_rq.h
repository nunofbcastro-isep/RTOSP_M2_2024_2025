#ifndef __RM_RQ_H_
#define __RM_RQ_H_

#include <linux/sched.h>
#include <linux/list.h>
#include <linux/spinlock.h>

struct rm_rq{
    struct rb_root tasks;
    raw_spinlock_t lock;
    unsigned nr_running;
};

void init_rm_rq(struct rm_rq *rq);

#endif