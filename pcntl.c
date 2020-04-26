/* procces controller */
#define _GNU_SOURCE

#include "pcntl.h"

#include <sched.h>
#include <unistd.h>

#include "common.h"
#include "utils.h"

#define DBG_ROLE "PCNTL"

void proc_assign_cpu(pid_t pid, int cpu)
{
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	G(sched_setaffinity(pid, sizeof(cpu_set_t), &mask));
}

/* priority: 1-99, higher value for higher priority
 */
void proc_elevate_priority(pid_t pid, int priority)
{
	struct sched_param param;
	param.sched_priority = priority;
	G(sched_setscheduler(pid, SCHED_FIFO, &param));

	// struct timespec p;
	// sched_rr_get_interval(0, &p);
	// DBG("time slice: %d.%09d", p.tv_sec, p.tv_nsec);
}