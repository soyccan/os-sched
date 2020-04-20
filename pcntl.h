/* procces controller */

#ifndef _CSIE_PCNTL_H_
#define _CSIE_PCNTL_H_ 1

#include <unistd.h>

void proc_assign_cpu(pid_t pid, int cpu);

void proc_elevate_priority(pid_t pid, int priority);

#endif