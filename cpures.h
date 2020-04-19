#ifndef _CSIE_CPURES_H
#define _CSIE_CPURES_H 1

#include <unistd.h>

/* called by parent (scheduler), should be called before calling schedule() */
void cpures_init();

/* called by child */
void cpures_open();

void cpures_destroy();

void cpures_acquire();

/* let next = 0 if don't know next */
void cpures_release(pid_t next);

/* can only be called by scheduler */
void cpures_setnext(pid_t next);

#endif