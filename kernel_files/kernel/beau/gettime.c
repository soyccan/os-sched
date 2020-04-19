#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/ktime.h> // ktime_t
#include <linux/time64.h> // time64_t, timespec64
#include <linux/timekeeping.h> // ktime_get_real_ts64
#include <linux/uaccess.h> // put_user

SYSCALL_DEFINE2(gettime, time64_t __user *, tv_sec, long __user *, tv_nsec)
{
	struct timespec64 ts;
	ktime_get_real_ts64(&ts);
	put_user(ts.tv_sec, tv_sec);
	put_user(ts.tv_nsec, tv_nsec);
	printk(KERN_WARNING "gettime called %lld.%ld\n", ts.tv_sec, ts.tv_nsec);
	return 0;
}
