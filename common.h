/* common structures and constant */
#ifndef _CSIE_COMMON_H_
#define _CSIE_COMMON_H_ 1

#include <unistd.h>

#define PARENT_CPU 0
#define CHILD_CPU 1

// 1-99, higher value for higher priority
// when set to 1 with SCHED_RR, our processes will have pri=-2
// prior to those normal pri=20
#define DEFAULT_PRI 1
#define HIGH_PRI 10

#define SYS_printk 439
#define SYS_gettime 440

#define TIME_UNIT()                                                            \
	{                                                                      \
		volatile unsigned long i;                                      \
		for (i = 0; i < 1000000UL; i++)                                \
			;                                                      \
	}

struct process {
	char name[32];
	int ready_time;
	int runtime;
	pid_t pid; // pid is given when process is ready
};

enum SchedulePolicy { FIFO, RR, SJF, PSJF };

#endif