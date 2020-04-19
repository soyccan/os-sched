/* common structures and constant */
#ifndef _CSIE_COMMON_H_
#define _CSIE_COMMON_H_ 1

#include <unistd.h>

#define PARENT_CPU 0
#define CHILD_CPU 0

#define SYS_printk 439

#define TIME_UNIT()                                                            \
	{                                                                      \
		volatile unsigned long i;                                      \
		for (i = 0; i < 1000000UL; i++)                                \
			;                                                      \
	}

struct process {
	char name[8];
	int ready_time;
	int runtime;
	pid_t pid; // pid is given when process is ready
};

enum SchedulePolicy { FIFO, RR, SJF, PSJF };

#endif