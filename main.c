#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "sched.h"
#include "utils.h"

#define DBG_ROLE "MAIN"

// shared by sched.c
extern struct process *procs;
extern int nproc;
extern enum SchedulePolicy policy;

int main()
{
	char buf[10];

	scanf("%9s %d", buf, &nproc);
	if (strncmp(buf, "FIFO", sizeof buf) == 0)
		policy = FIFO;
	else if (strncmp(buf, "RR", sizeof buf) == 0)
		policy = RR;
	else if (strncmp(buf, "SJF", sizeof buf) == 0)
		policy = SJF;
	else if (strncmp(buf, "PSJF", sizeof buf) == 0)
		policy = PSJF;

	GG(procs = malloc((size_t)nproc * sizeof(struct process)), NULL);
	for (int i = 0; i < nproc; i++) {
		scanf("%7s %d %d", procs[i].name, &procs[i].ready_time,
		      &procs[i].runtime);
	}
	scheduler();
	free(procs);
	DBG("main exit");
	return 0;
}