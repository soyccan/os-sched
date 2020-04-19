/* procces controller */
#define _GNU_SOURCE

#include "pcntl.h"

#include <sched.h>
#include <unistd.h>

#include "common.h"
#include "utils.h"

#define DBG_ROLE "PCNTL"

int proc_launch(const char *name, int runtime)
{
	int pid;
	G(pid = fork());
	if (pid == 0) {
		proc_assign_cpu(0, CHILD_CPU);
		// proc_block(0); // TODO: call by child or parent?

		char buf[11];
		snprintf(buf, sizeof(buf), "%d", runtime);
		buf[10] = '\0';
		G(execl("./child", "./child", name, buf, NULL));
	}
	return pid;
}

void proc_block(pid_t pid)
{
	DBG("request to block %d", pid == 0 ? getpid() : pid);
	struct sched_param param;
	param.sched_priority = 0;
	G(sched_setscheduler(pid, SCHED_IDLE, &param));
}

void proc_resume(pid_t pid)
{
	DBG("request to resume %d", pid == 0 ? getpid() : pid);
	struct sched_param param;
	param.sched_priority = 0;
	G(sched_setscheduler(pid, SCHED_OTHER, &param));
}

void proc_assign_cpu(pid_t pid, int cpu)
{
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	G(sched_setaffinity(pid, sizeof(cpu_set_t), &mask));
}