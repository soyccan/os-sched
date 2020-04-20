CC := clang
CFLAGS := -Wall -Wextra -Wconversion -std=gnu17
LDFLAGS := -pthread -lrt
OBJS := main.o sched.o pcntl.o cpures.o child.o
FILES := Makefile main.c sched.c sched.h pcntl.c pcntl.h cpures.c cpures.h child.c common.h utils.h corr_out/ accuracy.py run.sh timeunit.py

ifndef DEBUG
	DEBUG := 1
endif
ifeq ($(DEBUG), 1)
	CFLAGS += -g
else
	CFLAGS += -DNDEBUG

	# dangerous
	CFLAGS += -O2
endif

.PHONY: all upload run clean

all:
ifeq ($(shell hostname), soyccanmac.local)
	$(MAKE) upload
else
	$(MAKE) demo child
endif

upload: $(FILES)
	scp -P 9455 -r $(FILES) soyccan@bravo.nctu.me:/home/soyccan/Documents/osproj1/src

run: demo child
	./demo

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

demo: main.o sched.o pcntl.o cpures.o
	$(CC) $(LDFLAGS) -o $@ $^

child: child.o cpures.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -rf $(OBJS) demo child