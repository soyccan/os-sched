/* convinient functions */
#ifndef _CSIE_UTILS_H
#define _CSIE_UTILS_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
// guard syscall error
#define G(expr)                                                                \
	if ((expr) < 0) {                                                      \
		perror("\e[31m" #expr "\e[0m");                                \
		exit(-1);                                                      \
	}

// guard an error by value
#define GG(expr, err_value)                                                    \
	if ((expr) == err_value) {                                             \
		perror("\e[31m" #expr "\e[0m");                                \
		exit(-1);                                                      \
	}

#define DBG(format, ...)                                                       \
	fprintf(stderr, "\e[31m[" DBG_ROLE "] \e[0m" format "\n", ##__VA_ARGS__)
#define DBGN(format, ...)                                                      \
	fprintf(stderr, "\e[31m[" DBG_ROLE "] \e[0m" format, ##__VA_ARGS__)

#else
#define G(expr) (expr)
#define GG(expr, err_value) (expr)
#define DBG(...)
#define DBGN(...)
#endif

#endif