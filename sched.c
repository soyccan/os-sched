#define _GNU_SOURCE

#include "sched.h"

#include <assert.h>
#include <limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <signal.h>

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
static int last_run;
// this is set after running is done
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
		/* child */
#undef DBG_ROLE
#define DBG_ROLE "CHILD"

		DBG("child just forked %s", name);
		cpures_open();
		cpures_acquire();

		char buf[200];
		long tms[4];

		syscall(SYS_gettime, &tms[0], &tms[1]);
		DBG("launch %s %d, time=%ld.%09ld", name, getpid(), tms[0],
		    tms[1]);

		for (int i = 0; i < runtime; i++) {
			// if (i % 100 == 0)
			// 	DBG("%s runnning %d-th unit", name, i);
			TIME_UNIT();
			cpures_release(getppid()); // let scheduler run
			cpures_acquire();
		}

		syscall(SYS_gettime, &tms[2], &tms[3]);
		int n = 0;
		snprintf(buf, sizeof buf,
			 "[Project1] %d %ld.%09ld %ld.%09ld\n%n", getpid(),
			 tms[0], tms[1], tms[2], tms[3], &n);
		G(syscall(SYS_printk, buf, n));
		DBG("%s finish, time=%ld.%09ld", name, tms[2], tms[3]);
		cpures_release(getppid()); // let scheduler run

		_exit(0);
#undef DBG_ROLE
#define DBG_ROLE "SCHED"
	}
	DBG("forked child %s", name);
	proc_assign_cpu(pid, CHILD_CPU);
	return pid;
}

static inline int is_ready(int i)
{
	return procs[i].pid != 0 && procs[i].runtime > 0;
}

static inline int get_next_proc()
{
	if (policy == FIFO) {
		int a = running == -1 ? 0 : running;
		for (int i = a; i < nproc; i++) {
			if (is_ready(i)) {
				return i;
			}
		}
		return -1;
	} else if (policy == RR) {
		if (running == -1) {
			if (last_run == -1) {
				for (int i = 0; i < nproc; i++) {
					if (is_ready(i)) {
						last_cs_time = current_time;
						return i;
					}
				}
			} else {
				for (int i = (last_run + 1) % nproc;
				     i != last_run; i = (i + 1) % nproc) {
					if (is_ready(i)) {
						last_cs_time = current_time;
						return i;
					}
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
			assert(procs[running].runtime > 0);
			last_cs_time = current_time;
			return running;
		} else {
			assert(procs[running].runtime > 0);
			return running;
		}
	} else if (policy == SJF && running != -1) {
		assert(procs[running].runtime > 0);
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

static void why_the_fucking_sigsuspend_returns_negative_1(int signo)
{
}

void scheduler()
{
	proc_elevate_priority(0, DEFAULT_PRI);
	proc_assign_cpu(0, PARENT_CPU);

	struct sigaction sigact = { 0 }; // TODO: need to initialize?
	sigact.sa_handler = why_the_fucking_sigsuspend_returns_negative_1;
	G(sigaction(SIGUSR1, &sigact, NULL));

	sigset_t sigset;
	G(sigemptyset(&sigset));
	G(sigaddset(&sigset, SIGUSR1));
	G(sigprocmask(SIG_BLOCK, &sigset, NULL));

	cpures_init(getpid());

	// sort by ready time
	qsort(procs, (size_t)nproc, sizeof(procs[0]), cmp);

	for (int i = 0; i < nproc; i++) {
		procs[i].pid = 0;
	}
	dummy_child = proc_launch("dummy", INT_MAX);

	int finished = 0;
	last_run = running = -1;
	current_time = 0;
	last_cs_time = 0;
	while (1) {
		// check finished
		if (running != -1 && procs[running].runtime == 0) {
			DBG("%s finish, time=%d", procs[running].name,
			    current_time);
			cpures_release(procs[running].pid);
			cpures_acquire();

			last_run = running;
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
		int next = get_next_proc(); // may be -1
		if (next != running) {
			// will context switch later
			DBG("context switch from %d %s %d to %d %s %d", running,
			    procs[running].name, procs[running].pid, next,
			    procs[next].name, procs[next].pid);
			last_run = running;
			running = next;
		}

		// run by one time unit
#ifndef NDEBUG
		if (current_time % 100 == 0) {
			if (running != -1)
				DBG("running %s %d, time=%d",
				    procs[running].name, procs[running].pid,
				    current_time);
			else
				DBG("running nothing, time=%d", current_time);
		}
#endif
		if (running != -1) {
			// context switch to run the specified child
			cpures_release(procs[running].pid);
			cpures_acquire();
			procs[running].runtime--;
		} else {
			TIME_UNIT();
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
