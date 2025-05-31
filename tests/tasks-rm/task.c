#define _GNU_SOURCE
#include "defs.h"
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/syscall.h>

void do_work(unsigned long long exec)
{	
	unsigned long long i,ten_ns;
	ten_ns=exec/10;
	for(i=0; i<ten_ns; i++){
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
//32		
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
//64
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
		asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); asm volatile  ("nop" ::); 
//06
	}
}
int main(int argc, char** argv) 
{
	struct sched_param param;
	unsigned long long C, T, O, time0, release;
	unsigned int task_id,njobs,i=0;

	struct timespec r;

	task_id=atoi(argv[1]);
	C=(unsigned long long)atoll(argv[2]);
	T=(unsigned long long)atoll(argv[3]);
	O=(unsigned long long)atoll(argv[4])+OFFSET;
	time0 = (unsigned long long)atoll(argv[5]);
	njobs=atoi(argv[6]);
	
	cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(0, &cpu_set);

	printf("Task(%d,%d): pinned to CPU %d\n", task_id, getpid(), CURRENT_CPU);
    if (sched_setaffinity(CURRENT_CPU, sizeof(cpu_set_t), &cpu_set) != 0) {
        perror("ERROR: Failed to set CPU affinity");
        exit(EXIT_FAILURE);
    }

	printf("Task(%d,%d): set period %lld\n",task_id,getpid(),T);
	if (syscall(SYS_SET_RM_PERIOD, getpid(), T) == -1) {
		perror("ERROR: syscall set_rm_period failed");
		exit(EXIT_FAILURE);
	}
	
	printf("Task(%d,%d): before SCHED_RM\n",task_id,getpid());
	param.sched_priority = 0;
	if((sched_setscheduler(0,SCHED_RM,&param)) == -1){
		perror("ERROR:sched_setscheduler failed");
		exit(-1);
	}
	printf("Task(%d,%d): after SCHED_RM\n",task_id,getpid());

	release = time0 + O;
	
	for(i=0;i<njobs;i++){
		r.tv_sec = release / NSEC_PER_SEC;
		r.tv_nsec = release % NSEC_PER_SEC;

		
		printf("Task(%d,%d,%d): sleeping until %lld\n",task_id,getpid(),i,release);
		clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME, &r,NULL);
		printf("Task(%d,%d,%d): ready for execution\n",task_id,getpid(),i);
				
		do_work(C); 
		
		//computes the next release
		release += T;
		
	}

	exit(task_id);
	return 0;
}