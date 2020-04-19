#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h> // copy_from_user

#define MAX_LEN 100

SYSCALL_DEFINE2(printk, const char __user *, s, size_t, n)
{
	char t[MAX_LEN + 1];
	if (n > MAX_LEN)
		n = MAX_LEN;
	copy_from_user(t, s, n);
	t[n] = '\0';
	printk(KERN_WARNING "%s", t);
	return n;
}
