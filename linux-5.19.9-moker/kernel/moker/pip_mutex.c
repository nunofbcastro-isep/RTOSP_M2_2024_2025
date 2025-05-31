#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sched/types.h> 
#include <uapi/linux/sched/types.h> 
#include <asm/uaccess.h>
#include "../sched/sched.h"

#include "pip_mutex.h"
#include "trace.h"

struct pip_mutex_wq pip_wq;

void init_pip_mutex(void){
    pip_wq.tasks = RB_ROOT;
    raw_spin_lock_init(&pip_wq.lock);
    atomic_set(&pip_wq.flag, 0);
    pip_wq.holder_original_prio = -1;
    pip_wq.holder = NULL;
}

void change_task_prio_pip(struct task_struct *p, int new_prio)
{
    struct rq_flags rf;
    struct rq *rq;
    int old_prio = p->prio;

    p->prio = new_prio;
    p->static_prio = new_prio;

    if (p->policy == SCHED_RM) {
        rq = task_rq_lock(p, &rf);
        
        p->sched_class->prio_changed(rq, p, old_prio);
        
        task_rq_unlock(rq, p, &rf);
    }

    #ifdef CONFIG_MOKER_TRACING
    moker_trace(MUTEX_UPDATE, p);
    #endif
}

void lock_pip_mutex(void) {
    struct task_struct *p = current;

    while (!atomic_add_unless(&pip_wq.flag, 1, 1)) {
        raw_spin_lock(&pip_wq.lock);
        set_current_state(TASK_INTERRUPTIBLE);
        
        enqueue_pip_mutex_task(p);
        
        if (pip_wq.holder && p->prio < pip_wq.holder->prio) {
            // Aumentar prioridade temporariamente
            change_task_prio_pip(pip_wq.holder, p->prio);
        }
        raw_spin_unlock(&pip_wq.lock);

        schedule();
    }

    // Adquiriu o lock: registar como holder
    raw_spin_lock(&pip_wq.lock);
    pip_wq.holder = p;
    pip_wq.holder_original_prio = p->prio;
    raw_spin_unlock(&pip_wq.lock);

    #ifdef CONFIG_MOKER_TRACING
    moker_trace(MUTEX_LOCK, p);
    #endif
}

void unlock_pip_mutex(void) {
    struct task_struct *p = current;
    struct task_struct *t = NULL;

    raw_spin_lock(&pip_wq.lock);
    
    if (p->prio != pip_wq.holder_original_prio) {
        // Restaurar prioridade original
        change_task_prio_pip(p, pip_wq.holder_original_prio);
    }
    
    pip_wq.holder_original_prio = -1;
    pip_wq.holder = NULL;

    t = dequeue_pip_mutex_task();
    raw_spin_unlock(&pip_wq.lock);
    if (t){
        if (!wake_up_process(t)) {
            printk(KERN_ERR "BUG: wake up process failed: %d\n", t->pid);
        }
    }
    
    atomic_set(&pip_wq.flag, 0);

    #ifdef CONFIG_MOKER_TRACING
    moker_trace(MUTEX_UNLOCK, p);
    #endif
}

int enqueue_pip_mutex_task(struct task_struct *p)
{
    struct rb_node **link = &pip_wq.tasks.rb_node, *parent = NULL;
    struct pip_mutex_node *entry;
    struct pip_mutex_node *new_node;

    new_node = kmalloc(sizeof(*new_node), GFP_KERNEL);
    if (!new_node)
        return -ENOMEM;

    new_node->task = p;
    new_node->priority = p->prio;

    raw_spin_lock(&pip_wq.lock);

    while (*link) {
        parent = *link;
        entry = rb_entry(parent, struct pip_mutex_node, node);

        if (new_node->priority < entry->priority)
            link = &(*link)->rb_left;
        else
            link = &(*link)->rb_right;
    }

    rb_link_node(&new_node->node, parent, link);
    rb_insert_color(&new_node->node, &pip_wq.tasks);

    raw_spin_unlock(&pip_wq.lock);

    return 0;
}

struct task_struct * dequeue_pip_mutex_task(void)
{
    struct rb_node *node;
    struct pip_mutex_node *entry;
    struct task_struct *task = NULL;

    node = rb_first(&pip_wq.tasks);
    if (!node) {
        raw_spin_unlock(&pip_wq.lock);
        return NULL;
    }

    entry = rb_entry(node, struct pip_mutex_node, node);
    task = entry->task;

    rb_erase(&entry->node, &pip_wq.tasks);
    kfree(entry);

    raw_spin_unlock(&pip_wq.lock);

    #ifdef CONFIG_MOKER_TRACING
    moker_trace(DEQUEUE_WQ, task);
    #endif

    return task;
}