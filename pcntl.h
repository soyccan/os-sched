/* procces controller */

#ifndef _CSIE_PCNTL_H_
#define _CSIE_PCNTL_H_ 1

#include <unistd.h>

int proc_launch(const char *name, int runtime);

void proc_block(pid_t pid);

void proc_resume(pid_t pid);

void proc_assign_cpu(pid_t pid, int cpu);

#endif