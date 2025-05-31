#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include "pcp_mutex.h"
#include "trace.h"
#include "../sched/sched.h"

struct pcp_mutex_wq pcp_wq;

void init_pcp_mutex(void){
    pcp_wq.tasks = RB_ROOT;
    raw_spin_lock_init(&pcp_wq.lock);
    atomic_set(&pcp_wq.flag, 0);
    pcp_wq.holder_original_prio = -1;
    pcp_wq.holder = NULL;
    pcp_wq.ceilinged_priority = MAX_RT_PRIO; //inicializar com o maximo do linux
}

void change_task_prio_pcp(struct task_struct *p, int new_prio)
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

void lock_pcp_mutex(void) {
    struct task_struct *p = current;
    
    raw_spin_lock(&pcp_wq.lock);

    if(p->prio < pcp_wq.ceilinged_priority){
        pcp_wq.ceilinged_priority = p->prio;
    }
    
    raw_spin_unlock(&pcp_wq.lock);

    while (!atomic_add_unless(&pcp_wq.flag, 1, 1)) {
        raw_spin_lock(&pcp_wq.lock);

        set_current_state(TASK_INTERRUPTIBLE);

        enqueue_pcp_mutex_task(p);
        
        raw_spin_unlock(&pcp_wq.lock);

        schedule();
    }

    // Adquiriu o lock: registar como holder
    raw_spin_lock(&pcp_wq.lock);
    pcp_wq.holder = p;
    pcp_wq.holder_original_prio = p->prio;
    if(pcp_wq.ceilinged_priority < pcp_wq.holder->prio){
        change_task_prio_pcp(p, pcp_wq.ceilinged_priority);
        
        raw_spin_unlock(&pcp_wq.lock);
        schedule();        
    }else{
        raw_spin_unlock(&pcp_wq.lock);
    }

    #ifdef CONFIG_MOKER_TRACING
    moker_trace(MUTEX_LOCK, p);
    #endif
}

void unlock_pcp_mutex(void) {
    struct task_struct *p = current;
    struct task_struct *t = NULL;

    raw_spin_lock(&pcp_wq.lock);
    if (pcp_wq.holder_original_prio != -1 && p->prio != pcp_wq.holder_original_prio) {
        change_task_prio_pcp(p, pcp_wq.holder_original_prio);
    }
    pcp_wq.holder_original_prio = -1;
    pcp_wq.holder = NULL;
    raw_spin_unlock(&pcp_wq.lock);

    t = dequeue_pcp_mutex_task();
    if (t){
        if (!wake_up_process(t)) {
            printk(KERN_ERR "BUG: wake up process failed: %d\n", t->pid);
        }
    }
    
    atomic_set(&pcp_wq.flag, 0);

    #ifdef CONFIG_MOKER_TRACING
    moker_trace(MUTEX_UNLOCK, p);
    #endif
}

int enqueue_pcp_mutex_task(struct task_struct *p)
{
    struct rb_node **link = &pcp_wq.tasks.rb_node, *parent = NULL;
    struct pcp_mutex_node *entry;
    struct pcp_mutex_node *new_node;

    new_node = kmalloc(sizeof(*new_node), GFP_KERNEL);
    if (!new_node)
        return -ENOMEM;

    new_node->task = p;
    new_node->priority = p->prio;

    raw_spin_lock(&pcp_wq.lock);

    while (*link) {
        parent = *link;
        entry = rb_entry(parent, struct pcp_mutex_node, node);

        if (new_node->priority < entry->priority)
            link = &(*link)->rb_left;
        else
            link = &(*link)->rb_right;
    }

    rb_link_node(&new_node->node, parent, link);
    rb_insert_color(&new_node->node, &pcp_wq.tasks);

    raw_spin_unlock(&pcp_wq.lock);

    return 0;
}

struct task_struct * dequeue_pcp_mutex_task(void)
{
    struct rb_node *node;
    struct pcp_mutex_node *entry;
    struct task_struct *task = NULL;

    raw_spin_lock(&pcp_wq.lock);

    node = rb_first(&pcp_wq.tasks);
    if (!node) {
        raw_spin_unlock(&pcp_wq.lock);
        return NULL;
    }

    entry = rb_entry(node, struct pcp_mutex_node, node);
    task = entry->task;

    rb_erase(&entry->node, &pcp_wq.tasks);
    kfree(entry);

    raw_spin_unlock(&pcp_wq.lock);

    #ifdef CONFIG_MOKER_TRACING
    moker_trace(DEQUEUE_WQ, task);
    #endif

    return task;
}