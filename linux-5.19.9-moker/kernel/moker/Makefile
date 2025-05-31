# MOKER framework makefile

obj-$(CONFIG_MOKER_TRACING) += trace.o
obj-y += syscalls.o

obj-$(CONFIG_MOKER_SCHED_LIFO_POLICY) += lf_sched.o lf_rq.o

obj-$(CONFIG_MOKER_MUTEX_LIFO) += lf_mutex.o

obj-$(CONFIG_MOKER_SCHED_RM_POLICY) += rm_sched.o rm_rq.o

obj-$(CONFIG_MOKER_MUTEX_PIP) += pip_mutex.o
obj-$(CONFIG_MOKER_MUTEX_PCP) += pcp_mutex.o