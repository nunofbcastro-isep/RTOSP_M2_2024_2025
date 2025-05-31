
#ifndef DEFS_H_
#define DEFS_H_

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>



#define NR_TASKS		50
#define BUF_SIZE		10000

#define OFFSET			1000000000L

#define NSEC_PER_SEC		1000000000L
#define SCHED_RM		8

#define SYS_MOKER_TRACING_ENABLE 451

#define SYS_SET_RM_PERIOD 454

#define SYS_MOKER_MUTEX_LOCK 457
#define SYS_MOKER_MUTEX_UNLOCK 458

#define CURRENT_CPU 2

struct task {
	int id;
	unsigned long long C1; //exec befero
	unsigned long long C2; //exec while
	unsigned long long C3; //exec after
	unsigned long long T; //period
	unsigned long long O; //first job offset
	unsigned int njobs; //number of jobs
};

#endif
