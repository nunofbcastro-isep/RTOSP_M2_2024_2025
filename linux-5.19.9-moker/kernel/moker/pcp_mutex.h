#ifndef __PCP_MUTEX_H_
#define __PCP_MUTEX_H_

#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/atomic.h>

struct pcp_mutex_wq{
    struct rb_root tasks;
    atomic_t flag;
    raw_spinlock_t lock;    
    struct task_struct *holder; // Detentor atual do mutex
    int holder_original_prio; // Prioridade original do detentor
    int ceilinged_priority;
};

struct pcp_mutex_node{
    struct rb_node node;
    struct task_struct *task;
    int priority;
};

void init_pcp_mutex(void);
void lock_pcp_mutex(void);
void unlock_pcp_mutex(void);
int enqueue_pcp_mutex_task(struct task_struct *p);
struct task_struct * dequeue_pcp_mutex_task(void);

#endif