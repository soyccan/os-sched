#define _GNU_SOURCE

#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "common.h"
#include "cpures.h"
#include "utils.h"

#define DBG_ROLE "CHILD"

/* this is a wrapper of kernel function getnstimeofday()
 * kernel information is revealed by kernel module "ticks" via sysfs
 * Param: buf: (string) in the format seconds.nanoseconds
 *        n: size of buf
 * Return: number of bytes written to buf, excluing of terminating null byte
 */
static inline int getnstimeofday(char *buf, size_t n)
{
	int fd;
	ssize_t ret;
	G(fd = open("/sys/ticks/ts", O_RDONLY));
	G(ret = read(fd, buf, n));
	G(close(fd));
	if ((size_t)ret < n)
		buf[ret] = '\0';
	return (int)ret;
}

int main(int argc, char **argv)
{
	cpures_open();

	char tm[2][50]; // start time and finish time
	char buf[200];

	char *name = argv[1];
	int runtime = atoi(argv[2]);

	DBG("launch %s %d", name, getpid());

	assert(getnstimeofday(tm[0], sizeof tm[0]) > 0);

	for (int i = 0; i < runtime; i++) {
		cpures_acquire();
		if (i % 50 == 0)
			DBG("%s runnning %d-th unit", name, i);
		TIME_UNIT();
		cpures_release(getppid()); // let scheduler run
	}

	assert(getnstimeofday(tm[1], sizeof tm[1]) > 0);
	int n = 0;
	snprintf(buf, sizeof buf, "[Project1] %d %s %s\n%n", getpid(), tm[0],
		 tm[1], &n);
	G(syscall(SYS_printk, buf, n));

	DBG("%s finish", name);
	return 0;
}