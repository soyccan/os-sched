#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE2(add, long, a, long, b)
{
	printk(KERN_WARNING "add: %ld + %ld = %ld\n", a, b, a + b);
	return a + b;
}
