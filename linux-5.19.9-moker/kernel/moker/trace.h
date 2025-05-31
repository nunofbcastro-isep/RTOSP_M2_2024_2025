#ifndef __TRACE_H_
#define __TRACE_H_

#define TRACE_ENTRY_NAME "moker_trace"
#define TRACE_BUFFER_SIZE 1000
#define TRACE_STRING_BUFFER_SIZE 200
#define TRACE_TASK_COMM_LEN 16

enum evt{
    SCHED_TICK = 0,
    SWITCH_AWAY,
    SWITCH_TO,
    
    ENQUEUE_RQ,
    DEQUEUE_RQ,

    ENQUEUE_WQ,
    DEQUEUE_WQ,
    MUTEX_LOCK,
    MUTEX_UNLOCK,
    MUTEX_UPDATE,

    PRIO_CHANGED_RQ,
    SWITCHED_FROM_RQ,
    SWITCHED_TO_RQ,

};

struct trace_evt{
    enum evt event;
    unsigned long long time;
    pid_t pid;
    int state;
    int prio;
    int policy;
    unsigned long long period;
    char comm[TRACE_TASK_COMM_LEN];
};

struct trace_evt_buffer{
    struct trace_evt events[TRACE_BUFFER_SIZE];
    int write_item;
    int read_item;
    raw_spinlock_t lock;
};

void moker_trace(enum evt event,struct task_struct *p);

void trace_enable (unsigned int e);

#endif