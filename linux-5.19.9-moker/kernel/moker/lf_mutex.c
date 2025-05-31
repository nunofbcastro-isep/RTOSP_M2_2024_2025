#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include "lf_mutex.h"
#include "trace.h"

struct lf_mutex_wq wq;

void init_lf_mutex(void){
    INIT_LIST_HEAD(&wq.tasks);
    raw_spin_lock_init(&wq.lock);
    atomic_set(&wq.flag, 0);
}

void lock_lf_mutex(){
    struct task_struct *p = current;
    while(!atomic_add_unless(&wq.flag, 1, 1)){
        set_current_state(TASK_INTERRUPTIBLE);
        enqueue_lf_mutex_task(p);
        schedule();
    }

    #ifdef CONFIG_MOKER_TRACING
    moker_trace(MUTEX_LOCK, p);
    #endif
}

void unlock_lf_mutex(void){
    struct task_struct *p = current;
    struct task_struct *t = NULL;
    t = dequeue_lf_mutex_task();
    if(t){
        if(!wake_up_process(t)){
            printk(KERN_ERR "BUG: wake up process failed: %d\n",t->pid);
        }
    }
    atomic_set(&wq.flag,0);

    #ifdef CONFIG_MOKER_TRACING
    moker_trace(MUTEX_UNLOCK, p);
    #endif
}

int enqueue_lf_mutex_task(struct task_struct *p)
{
    int ret = -1;
    struct lf_mutex_node *t = kmalloc(sizeof(struct lf_mutex_node),GFP_KERNEL);
    if(t){
        t->task = p;
        raw_spin_lock(&wq.lock);
        list_add(&t->node,&wq.tasks);
        raw_spin_unlock(&wq.lock);
        ret = 0;

        #ifdef CONFIG_MOKER_TRACING
        moker_trace(ENQUEUE_WQ, p);
        #endif
    }
    return ret;
}

struct task_struct * dequeue_lf_mutex_task(void)
{
    struct task_struct *p = NULL;
    struct lf_mutex_node *t = NULL;
    raw_spin_lock(&wq.lock);
    if(!list_empty(&wq.tasks)){
        t = list_first_entry(&wq.tasks,struct lf_mutex_node, node);
        p = t->task;
        list_del(&t->node);
    }
    raw_spin_unlock(&wq.lock);
    if(t){
        kfree(t);
    }

    #ifdef CONFIG_MOKER_TRACING
    if(p)
        moker_trace(DEQUEUE_WQ, p);
    #endif

    return p;
}