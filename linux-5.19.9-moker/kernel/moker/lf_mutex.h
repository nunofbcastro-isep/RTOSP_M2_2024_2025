#ifndef __LF_MUTEX_H_
#define __LF_MUTEX_H_

#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/atomic.h>

struct lf_mutex_wq{
    struct list_head tasks;
    atomic_t flag;
    raw_spinlock_t lock;
};

struct lf_mutex_node{
    struct list_head node;
    struct task_struct *task;
};

void init_lf_mutex(void);
void lock_lf_mutex(void);
void unlock_lf_mutex(void);
int enqueue_lf_mutex_task(struct task_struct *p);
struct task_struct * dequeue_lf_mutex_task(void);

#endif