CC := clang
CFLAGS := -Wall -Wextra -Wconversion -std=gnu17
LDFLAGS := -pthread -lrt
OBJS := main.o sched.o pcntl.o cpures.o
deps := $(OBJS:%.o=.%.o.d)

ifndef DEBUG
	DEBUG := 0
endif
ifeq ($(DEBUG), 1)
	CFLAGS += -g
else
	CFLAGS += -DNDEBUG -O2
endif

.PHONY: all run clean

all: demo

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ -MMD -MF .$@.d $<

demo: main.o sched.o cpures.o pcntl.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -rf $(OBJS) demo

-include $(deps)
