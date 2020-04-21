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
// static inline int getnstimeofday(char *buf, size_t n)
// {
// 	int fd;
// 	ssize_t ret;
// 	G(fd = open("/sys/ticks/ts", O_RDONLY));
// 	G(ret = read(fd, buf, n));
// 	G(close(fd));
// 	if ((size_t)ret < n)
// 		buf[ret] = '\0';
// 	return (int)ret;
// }

int main(int argc, char **argv)
{
	cpures_open();
	cpures_acquire();

	char buf[200];
	long tms[4];

	char *name = argv[1];
	int runtime = atoi(argv[2]);

	syscall(SYS_gettime, &tms[0], &tms[1]);
	DBG("launch %s %d, time=%ld.%09ld", name, getpid(), tms[0], tms[1]);
	cpures_release(getppid()); // let scheduler run

	for (int i = 0; i < runtime; i++) {
		cpures_acquire();
		if (i % 100 == 0)
			DBG("%s runnning %d-th unit", name, i);
		TIME_UNIT();
		cpures_release(getppid()); // let scheduler run
	}

	cpures_acquire();
	syscall(SYS_gettime, &tms[2], &tms[3]);
	int n = 0;
	snprintf(buf, sizeof buf, "[Project1] %d %ld.%09ld %ld.%09ld\n%n",
		 getpid(), tms[0], tms[1], tms[2], tms[3], &n);
	G(syscall(SYS_printk, buf, n));
	DBG("%s finish, time=%ld.%09ld", name, tms[2], tms[3]);
	cpures_release(getppid()); // let scheduler run

	return 0;
}