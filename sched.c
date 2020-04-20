#define _GNU_SOURCE

#include "sched.h"

#include <assert.h>
#include <limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#include "common.h"
#include "utils.h"
#include "pcntl.h"
#include "cpures.h"

#define DBG_ROLE "SCHED"

// shared object
struct process *procs;
int nproc;
enum SchedulePolicy policy;

static pid_t dummy_child;
// when CPU has to be idle, run dummy child to ensure the idle time
// has approximate time unit as the others
static int running;
// procs[running] is the running job, -1 for none
static int current_time;
static int last_cs_time;
// last time being context-switched due to time quantum in RR

static int proc_launch(const char *name, int runtime)
{
	int pid;
	G(pid = fork());
	if (pid == 0) {
		proc_elevate_priority(0, DEFAULT_PRI);
		proc_assign_cpu(0, CHILD_CPU);

		char buf[11];
		snprintf(buf, sizeof(buf), "%d", runtime);
		buf[10] = '\0';
		G(execl("./child", "./child", name, buf, NULL));
	}
	return pid;
}

static inline int is_ready(int i)
{
	return procs[i].pid != 0 && procs[i].runtime > 0;
}

static inline int get_next_proc()
{
	if (policy == FIFO) {
		for (int i = 0; i < nproc; i++) {
			if (is_ready(i)) {
				return i;
			}
		}
		return -1;
	} else if (policy == RR) {
		if (running == -1) {
			for (int i = 0; i < nproc; i++) {
				if (is_ready(i)) {
					last_cs_time = current_time;
					return i;
				}
			}
			return -1;
		} else if ((current_time - last_cs_time) % 500 == 0) {
			for (int i = (running + 1) % nproc; i != running;
			     i = (i + 1) % nproc) {
				if (is_ready(i)) {
					last_cs_time = current_time;
					return i;
				}
			}
			if (procs[running].runtime > 0) {
				last_cs_time = current_time;
				return running;
			} else
				return -1;
		} else {
			return running;
		}
	} else if (policy == SJF && running != -1) {
		return running;
	} else if (policy == SJF || policy == PSJF) {
		int ret = -1;
		for (int i = 0; i < nproc; i++) {
			if (is_ready(i) &&
			    (ret == -1 ||
			     procs[i].runtime < procs[ret].runtime)) {
				ret = i;
			}
		}
		return ret;
	} else {
		perror("unsupported scheduling policy");
	}
	return -1;
}

static inline int cmp(const void *x, const void *y)
{
	return ((struct process *)x)->ready_time -
	       ((struct process *)y)->ready_time;
}

void scheduler()
{
	proc_elevate_priority(0, DEFAULT_PRI);
	proc_assign_cpu(0, PARENT_CPU);

	// sort by ready time
	qsort(procs, (size_t)nproc, sizeof(procs[0]), cmp);

	for (int i = 0; i < nproc; i++) {
		procs[i].pid = 0;
	}
	dummy_child = proc_launch("dummy", INT_MAX);

	cpures_init();
	cpures_setnext(getpid());
	cpures_acquire();

	int finished = 0;
	running = -1;
	current_time = 0;
	last_cs_time = 0;
	while (1) {
		if (current_time % 100 == 0) {
			if (running != -1)
				DBG("running %s %d, time=%d",
				    procs[running].name, procs[running].pid,
				    current_time);
			else
				DBG("running nothing, time=%d", current_time);
		}

		// check finished
		if (running != -1 && procs[running].runtime == 0) {
			DBG("%s finish", procs[running].name);
			running = -1;
			finished++;
			if (finished == nproc) {
				break;
			}
		}

		// get process ready
		for (int i = 0; i < nproc; i++) {
			if (procs[i].ready_time == current_time) {
				procs[i].pid = proc_launch(procs[i].name,
							   procs[i].runtime);
				printf("%s %d\n", procs[i].name, procs[i].pid);
				DBG("launched %s %d, ready=%d, current=%d",
				    procs[i].name, procs[i].pid,
				    procs[i].ready_time, current_time);
			}
		}

		// check waiting process
		int next = get_next_proc();
		assert(!(next != running && next == -1));
		if (next != running) {
			// will context switch later
			DBG("context switch from %d %s %d to %d %s %d", running,
			    procs[running].name, procs[running].pid, next,
			    procs[next].name, procs[next].pid);
		}
		running = next;

		// run by one time unit
		if (running != -1) {
			// context switch to run the specified child
			cpures_release(procs[running].pid);
			cpures_acquire();
			procs[running].runtime--;
		} else {
			cpures_release(dummy_child);
			cpures_acquire();
		}
		current_time++;
	}
	for (int i = 0; i < nproc; i++) {
		G(waitpid(procs[i].pid, NULL, 0));
		procs[i].pid = 0;
	}
	kill(dummy_child, SIGTERM);
	G(waitpid(dummy_child, NULL, 0));
	dummy_child = 0;
	cpures_destroy();
}
