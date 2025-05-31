/*
* RM scheduling class.
* Implements SCHED_RM
*/

#include "../sched/sched.h"
#include <linux/log2.h>

#define RM_MIN_PRIO 10
#define RM_MAX_PRIO (MAX_RT_PRIO - 1)

static int calculate_rm_priority(u64 period)
{
    int prio;
    
    if (period == 0)
        return RM_MIN_PRIO;
    
    prio = RM_MIN_PRIO + (int)((RM_MAX_PRIO - RM_MIN_PRIO) * 
                               ilog2(period) / ilog2(U64_MAX));
    
    return prio;
}

static void add_task_rm(struct rq *rq, struct task_struct *p){
    struct rb_node **new = &rq->rm.tasks.rb_node;
    struct rb_node *parent = NULL;
    struct sched_rm_entity *entry;
    
    // Traverse the tree to find the insertion point
    while (*new) {
        parent = *new;
        entry = rb_entry(parent, struct sched_rm_entity, node);
        if (p->rm.period >= entry->period) {
            new = &parent->rb_right;
        } else {
            new = &parent->rb_left;
        }
    }

    // Link the new node to the tree
    rb_link_node(&p->rm.node, parent, new);
    rb_insert_color(&p->rm.node, &rq->rm.tasks);
}

static bool remove_task_rm(struct rq *rq, struct task_struct *p)
{
    if (RB_EMPTY_NODE(&p->rm.node)) {
        // The task is not in the tree
        return false;
    }

    // Remove the task from the red-black tree
    rb_erase(&p->rm.node, &rq->rm.tasks);
    RB_CLEAR_NODE(&p->rm.node);
    
    return true;
}

static void enqueue_task_rm(struct rq *rq, struct task_struct *p, int flags)
{
    p->prio = calculate_rm_priority(p->rm.period);
    p->static_prio = p->prio;
    
    raw_spin_lock(&rq->rm.lock);

    add_task_rm(rq, p);

    rq->rm.nr_running++;
    add_nr_running(rq, 1);
    
    raw_spin_unlock(&rq->rm.lock);
    
    #ifdef CONFIG_MOKER_TRACING
    moker_trace(ENQUEUE_RQ, p);
    #endif
}

static void dequeue_task_rm(struct rq *rq, struct task_struct *p, int flags)
{
    raw_spin_lock(&rq->rm.lock);

    if (remove_task_rm(rq, p)) {
        rq->rm.nr_running--;
        sub_nr_running(rq, 1);
    }

    raw_spin_unlock(&rq->rm.lock);

    #ifdef CONFIG_MOKER_TRACING
    moker_trace(DEQUEUE_RQ, p);
    #endif
}

static void yield_task_rm(struct rq *rq)
{}

static bool yield_to_task_rm(struct rq *rq, struct task_struct *p)
{
    return true;
}

static void check_preempt_curr_rm(struct rq *rq, struct task_struct *p, int flags)
{
    switch(rq->curr->policy){
        case SCHED_DEADLINE:
        case SCHED_FIFO:
        case SCHED_RR:
            break;
        case SCHED_NORMAL:
        case SCHED_BATCH:
        case SCHED_IDLE:
        //case SCHED_RESET_ON_FORK:
        case SCHED_RM:
            resched_curr(rq);
            break;
    }
}

static struct task_struct *pick_next_task_rm(struct rq *rq)
{
    struct rb_node *node;
    struct sched_rm_entity *entry;
    struct task_struct *p = NULL;

    if (!rq) {
        printk(KERN_ERR "RM: NULL run queue in pick_next_task_rm\n");
        return NULL;
    }

    raw_spin_lock(&rq->rm.lock);

    node = rb_first(&rq->rm.tasks);
    if (node) {
        entry = rb_entry(node, struct sched_rm_entity, node);
        if (entry) {
            p = container_of(entry, struct task_struct, rm);
        }
    }

    raw_spin_unlock(&rq->rm.lock);

    return p;
}

static void put_prev_task_rm(struct rq *rq, struct task_struct *p)
{}


static void set_next_task_rm(struct rq *rq, struct task_struct *p, bool first)
{}

#ifdef CONFIG_SMP

static int balance_rm(struct rq *rq, struct task_struct *p, struct rq_flags *rf)
{
    return 0;
}

static struct task_struct *pick_task_rm(struct rq *rq)
{
    return NULL;
}

static int select_task_rq_rm(struct task_struct *p, int cpu, int flags)
{
    return cpu;
}

static void migrate_task_rq_rm(struct task_struct *p, int new_cpu __maybe_unused)
{}

static void set_cpus_allowed_rm(struct task_struct *p,const struct cpumask *new_mask, u32 flags)
{}

static void rq_online_rm(struct rq *rq)
{}

static void rq_offline_rm(struct rq *rq)
{}

static void task_woken_rm(struct rq *rq, struct task_struct *p)
{}

static void task_dead_rm(struct task_struct *p)
{}

static struct rq *find_lock_rq_rm(struct task_struct *task, struct rq *rq)
{
    return NULL;
}

#endif

static void task_tick_rm(struct rq *rq, struct task_struct *p, int queued)
{}

static void task_fork_rm(struct task_struct *p)
{}

static void prio_changed_rm(struct rq *rq, struct task_struct *p, int oldprio)
{
    if (!task_on_rq_queued(p)){
        return;
    }

    if (p->prio != oldprio) {
        raw_spin_lock(&rq->rm.lock);
        
        remove_task_rm(rq, p);
        
        add_task_rm(rq, p);

        if (task_running(rq, p)) {
            struct task_struct *next = pick_next_task_rm(rq);
            if (next && next != p && next->prio < p->prio) {
                resched_curr(rq);
            }
        }
    }
    
    #ifdef CONFIG_MOKER_TRACING
    moker_trace(PRIO_CHANGED_RQ, p);
    #endif
}

static void switched_from_rm(struct rq *rq, struct task_struct *p)
{}

static void switched_to_rm(struct rq *rq, struct task_struct *p)
{}

static unsigned int get_rr_interval_rm(struct rq *rq, struct task_struct *task)
{
    return 0;
}

static void update_curr_rm(struct rq *rq)
{}

DEFINE_SCHED_CLASS(rm) = {

    .enqueue_task = enqueue_task_rm,
    .dequeue_task = dequeue_task_rm,
    .yield_task = yield_task_rm,
    .yield_to_task = yield_to_task_rm,

    .check_preempt_curr = check_preempt_curr_rm,

    .pick_next_task = pick_next_task_rm,
    .put_prev_task = put_prev_task_rm,
    .set_next_task = set_next_task_rm,

    #ifdef CONFIG_SMP
    .balance = balance_rm,
    .pick_task = pick_task_rm,
    .select_task_rq = select_task_rq_rm,
    .migrate_task_rq = migrate_task_rq_rm,
    .set_cpus_allowed = set_cpus_allowed_rm,
    .rq_online = rq_online_rm,
    .rq_offline = rq_offline_rm,
    .task_woken = task_woken_rm,
    .task_dead = task_dead_rm,
    .find_lock_rq = find_lock_rq_rm,
    #endif

    .task_tick = task_tick_rm,
    .task_fork = task_fork_rm,

    .prio_changed = prio_changed_rm,
    .switched_from = switched_from_rm,
    .switched_to = switched_to_rm,
    .get_rr_interval = get_rr_interval_rm,

    .update_curr = update_curr_rm,
};
