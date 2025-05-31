#ifndef __PIP_MUTEX_H_
#define __PIP_MUTEX_H_

#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/atomic.h>

struct pip_mutex_wq{
    struct rb_root tasks;
    atomic_t flag;
    raw_spinlock_t lock;
    struct task_struct *holder; // Detentor atual do mutex
    int holder_original_prio; // Prioridade original do detentor
};

struct pip_mutex_node{
    struct rb_node node;
    struct task_struct *task;
    int priority;
};

void init_pip_mutex(void);
void lock_pip_mutex(void);
void unlock_pip_mutex(void);
int enqueue_pip_mutex_task(struct task_struct *p);
struct task_struct * dequeue_pip_mutex_task(void);

#endif