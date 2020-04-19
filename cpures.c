/* manage processes' access to CPU
 * shared between child and parent
 */
#define _GNU_SOURCE

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
// #include <semaphore.h>

#include "utils.h"

#define DBG_ROLE "CPURES"

#define SHM_NAME "/osproj1"

struct cpures {
	// sem_t running; // running on CPU is exclusive
	pthread_cond_t is_next; // conditional variable of (getpid() == next)
	pthread_mutex_t mutex; // protect r/w on cpures
	pid_t current; // let no-value be 0
	pid_t next; // set by scheduler, only 'next' is allowed to acquire CPU
};

static struct cpures *cpures;

/* called by parent (scheduler), should be called before calling schedule() */
void cpures_init()
{
	int fd;
	G(fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600));
	G(ftruncate(fd, sizeof(struct cpures)));
	GG(cpures = mmap(NULL, sizeof(struct cpures), PROT_READ | PROT_WRITE,
			 MAP_SHARED, fd, 0),
	   NULL);
	G(close(fd));

	cpures->current = cpures->next = 0;

	pthread_condattr_t at;
	G(pthread_condattr_init(&at));
	G(pthread_condattr_setpshared(&at, PTHREAD_PROCESS_SHARED));
	G(pthread_cond_init(&cpures->is_next, &at));

	pthread_mutexattr_t att;
	G(pthread_mutexattr_init(&att));
	G(pthread_mutexattr_setpshared(&att, PTHREAD_PROCESS_SHARED));
	G(pthread_mutex_init(&cpures->mutex, &att));
}

/* called by child */
void cpures_open()
{
	int fd;
	G(fd = shm_open(SHM_NAME, O_RDWR, 0));
	GG(cpures = mmap(NULL, sizeof(struct cpures), PROT_READ | PROT_WRITE,
			 MAP_SHARED, fd, 0),
	   NULL);
	G(close(fd));
}

/* called by who init */
void cpures_destroy()
{
	G(munmap(cpures, sizeof(struct cpures)));
	G(shm_unlink(SHM_NAME));
}

void cpures_acquire()
{
	G(pthread_mutex_lock(&cpures->mutex));
	while (getpid() != cpures->next) {
		G(pthread_cond_wait(&cpures->is_next, &cpures->mutex));
	}

	G(cpures->current = getpid());
	cpures->next = 0; // ensure mutual exclusion

	G(pthread_mutex_unlock(&cpures->mutex));
}

/* let next = 0 if don't know next */
void cpures_release(pid_t next)
{
	G(pthread_mutex_lock(&cpures->mutex));

	if (getpid() != cpures->current) {
		G(pthread_mutex_unlock(&cpures->mutex));
		return;
	}

	cpures->current = 0;
	cpures->next = next;
	G(pthread_cond_broadcast(&cpures->is_next));

	G(pthread_mutex_unlock(&cpures->mutex));
}

/* can only be called by scheduler */
void cpures_setnext(pid_t next)
{
	G(pthread_mutex_lock(&cpures->mutex));

	cpures->next = next;
	G(pthread_cond_broadcast(&cpures->is_next));

	G(pthread_mutex_unlock(&cpures->mutex));
}