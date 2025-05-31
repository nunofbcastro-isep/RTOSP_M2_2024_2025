#include <linux/syscalls.h>
#include "trace.h"
#include "lf_mutex.h"
#include "pip_mutex.h"
#include "pcp_mutex.h"

SYSCALL_DEFINE1(moker_tracing, unsigned int, enable)
{
    printk("MOKER: moker_tracing:[%d][%d]\n", (int) enable, current->pid);
    return sys_moker_tracing(enable);
}
int sys_moker_tracing (unsigned int enable){
    #ifdef CONFIG_MOKER_TRACING
        printk("MOKER: sys_moker_tracing:[%d][%d]\n", (int) enable, current->pid);
        trace_enable(enable);
    #endif
    return 0;
}

SYSCALL_DEFINE0(moker_lf_mutex_lock)
{
    return sys_moker_lf_mutex_lock();
}
int sys_moker_lf_mutex_lock (){
    #ifdef CONFIG_MOKER_MUTEX_LIFO
    lock_lf_mutex();
    #endif
    return 0;
}

SYSCALL_DEFINE0(moker_lf_mutex_unlock)
{
    return sys_moker_lf_mutex_unlock();
}
int sys_moker_lf_mutex_unlock (){
    #ifdef CONFIG_MOKER_MUTEX_LIFO
    unlock_lf_mutex();
    #endif
    return 0;
}

SYSCALL_DEFINE2(set_rm_period, pid_t, pid, u64, period) {
    return sys_set_rm_period(pid, period);
}
long sys_set_rm_period(pid_t pid, u64 period) {
    #ifdef CONFIG_MOKER_SCHED_RM_POLICY
    struct task_struct *task;

    // Verifica se o período é válido
    if (period <= 0)
        return -EINVAL;

    rcu_read_lock();
    task = find_task_by_vpid(pid);
    if (!task) {
        rcu_read_unlock();
        return -ESRCH;
    }

    get_task_struct(task);
    rcu_read_unlock();

    // Protege o acesso à task com lock apropriado
    task_lock(task);
    task->rm.period = period;
    task_unlock(task);
    
    put_task_struct(task);
    #endif
    return 0;
}

SYSCALL_DEFINE0(moker_pip_mutex_lock)
{
    return sys_moker_pip_mutex_lock();
}
int sys_moker_pip_mutex_lock (){
    #ifdef CONFIG_MOKER_MUTEX_PIP
    lock_pip_mutex();
    #endif
    return 0;
}

SYSCALL_DEFINE0(moker_pip_mutex_unlock)
{
    return sys_moker_pip_mutex_unlock();
}
int sys_moker_pip_mutex_unlock (){
    #ifdef CONFIG_MOKER_MUTEX_PIP
    unlock_pip_mutex();
    #endif
    return 0;
}

SYSCALL_DEFINE0(moker_pcp_mutex_lock)
{
    return sys_moker_pcp_mutex_lock();
}
int sys_moker_pcp_mutex_lock (){
    #ifdef CONFIG_MOKER_MUTEX_PCP
    lock_pcp_mutex();
    #endif
    return 0;
}

SYSCALL_DEFINE0(moker_pcp_mutex_unlock)
{
    return sys_moker_pcp_mutex_unlock();
}
int sys_moker_pcp_mutex_unlock (){
    #ifdef CONFIG_MOKER_MUTEX_PCP
    unlock_pcp_mutex();
    #endif
    return 0;
}