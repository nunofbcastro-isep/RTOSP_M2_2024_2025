#ifndef __SYSCALLS_H
#define __SYSCALLS_H

int sys_moker_tracing (unsigned int enable);

int sys_moker_lf_mutex_lock(void);
int sys_moker_lf_mutex_unlock(void);

long sys_set_rm_period(pid_t pid, u64 period);

int sys_moker_pip_mutex_lock(void);
int sys_moker_pip_mutex_unlock(void);

int sys_moker_pcp_mutex_lock(void);
int sys_moker_pcp_mutex_unlock(void);


#endif 